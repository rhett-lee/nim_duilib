#include "RichTextUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"

#include <algorithm>

namespace ui
{

DString EscapeRichTextXmlText(const DString& text)
{
    DString result;
    result.reserve(text.size());
    for (DString::value_type ch : text) {
        switch (ch) {
        case _T('&'): result += _T("&amp;"); break;
        case _T('<'): result += _T("&lt;"); break;
        case _T('>'): result += _T("&gt;"); break;
        default: result += ch; break;
        }
    }
    return result;
}

DString EscapeRichTextXmlAttribute(const DString& text)
{
    DString result;
    result.reserve(text.size());
    for (DString::value_type ch : text) {
        switch (ch) {
        case _T('&'): result += _T("&amp;"); break;
        case _T('<'): result += _T("&lt;"); break;
        case _T('>'): result += _T("&gt;"); break;
        case _T('\"'): result += _T("&quot;"); break;
        default: result += ch; break;
        }
    }
    return result;
}

bool ContainsRichTextAlignValue(const DString& text, const DString::value_type* value)
{
    if (value == nullptr) {
        return false;
    }
    size_t nStart = 0;
    while (nStart <= text.size()) {
        size_t nEnd = text.find(_T(','), nStart);
        if (nEnd == DString::npos) {
            nEnd = text.size();
        }
        DString item = text.substr(nStart, nEnd - nStart);
        StringUtil::Trim(item);
        if (item == value) {
            return true;
        }
        if (nEnd == text.size()) {
            break;
        }
        nStart = nEnd + 1;
    }
    return false;
}

RichTextTextAlign ParseRichTextTextAlign(const DString& text)
{
    RichTextTextAlign alignValue;
    //水平方向："center"作为"hcenter"的别名
    if (ContainsRichTextAlignValue(text, _T("left"))) {
        alignValue.m_nHAlign = 0;
    }
    else if (ContainsRichTextAlignValue(text, _T("hcenter")) ||
             ContainsRichTextAlignValue(text, _T("center"))) {
        alignValue.m_nHAlign = 1;
    }
    else if (ContainsRichTextAlignValue(text, _T("right"))) {
        alignValue.m_nHAlign = 2;
    }

    //垂直方向
    if (ContainsRichTextAlignValue(text, _T("top"))) {
        alignValue.m_nVAlign = 0;
    }
    else if (ContainsRichTextAlignValue(text, _T("vcenter"))) {
        alignValue.m_nVAlign = 1;
    }
    else if (ContainsRichTextAlignValue(text, _T("bottom"))) {
        alignValue.m_nVAlign = 2;
    }

    //两端对齐（justify）：不属于水平对齐方式的枚举值，单独记录
    alignValue.m_bJustify = ContainsRichTextAlignValue(text, _T("justify"));
    return alignValue;
}

bool IsSupportedRichTextTagName(const DString& tagName)
{
    DString name = StringUtil::MakeLowerString(tagName);
    return (name == _T("a")) || (name == _T("b")) || (name == _T("i")) ||
           (name == _T("u")) || (name == _T("s")) || (name == _T("del")) ||
           (name == _T("strike")) || (name == _T("ins")) || (name == _T("bgcolor")) ||
           (name == _T("font")) || (name == _T("br")) || (name == _T("img")) ||
           (name == _T("span")) || (name == _T("p")) || (name == _T("ul")) ||
           (name == _T("ol")) || (name == _T("li")) || (name == _T("sup")) ||
           (name == _T("sub")) || (name == _T("hr")) ||
           (name == _T("h1")) || (name == _T("h2")) || (name == _T("h3")) ||
           (name == _T("h4")) || (name == _T("h5")) || (name == _T("h6"));
}

bool IsSupportedRichTextTags(const DString& text)
{
    size_t nPos = 0;
    while ((nPos = text.find(_T('<'), nPos)) != DString::npos) {
        size_t nEnd = text.find(_T('>'), nPos + 1);
        if (nEnd == DString::npos) {
            return false;
        }
        size_t nNameBegin = nPos + 1;
        if ((nNameBegin < nEnd) && (text[nNameBegin] == _T('/'))) {
            ++nNameBegin;
        }
        size_t nNameEnd = nNameBegin;
        while (nNameEnd < nEnd) {
            const DString::value_type ch = text[nNameEnd];
            if (((ch >= _T('a')) && (ch <= _T('z'))) ||
                ((ch >= _T('A')) && (ch <= _T('Z'))) ||
                ((ch >= _T('0')) && (ch <= _T('9')))) {
                ++nNameEnd;
            }
            else {
                break;
            }
        }
        if ((nNameEnd == nNameBegin) || !IsSupportedRichTextTagName(text.substr(nNameBegin, nNameEnd - nNameBegin))) {
            return false;
        }
        nPos = nEnd + 1;
    }
    return true;
}

bool IsRichTextBlockTagName(const DString& tagName)
{
    //标签名称不区分大小写
    const DString lowerTagName = StringUtil::MakeLowerString(tagName);
    return (lowerTagName == _T("p")) ||
           (lowerTagName == _T("h1")) || (lowerTagName == _T("h2")) || (lowerTagName == _T("h3")) ||
           (lowerTagName == _T("h4")) || (lowerTagName == _T("h5")) || (lowerTagName == _T("h6")) ||
           (lowerTagName == _T("ul")) || (lowerTagName == _T("ol")) || (lowerTagName == _T("li")) ||
           (lowerTagName == _T("hr"));
}

bool HasRichTextVisibleText(const DString& text)
{
    for (DString::value_type ch : text) {
        if ((ch != _T(' ')) && (ch != _T('\t')) && (ch != _T('\r')) && (ch != _T('\n'))) {
            return true;
        }
    }
    return false;
}

int8_t ParseRichTextHAlignValue(const DString& alignValue)
{
    DString align = alignValue;
    StringUtil::Trim(align);
    //取值不区分大小写（兼容 align="Center" 等写法）
    StringUtil::LowerString(align);
    if (align == _T("center")) {
        return 1;
    }
    if ((align == _T("right")) || (align == _T("end"))) {
        return 2;
    }
    if (align == _T("justify")) {
        return 3;
    }
    if ((align == _T("left")) || (align == _T("start"))) {
        return 0;
    }
    return -1;
}

DString NormalizeRichTextLinkUrl(const DString& url)
{
    DString text = url;
    StringUtil::Trim(text);
    StringUtil::LowerString(text);
    for (;;) {
        if (text.rfind(_T("https://"), 0) == 0) {
            text = text.substr(8);
        }
        else if (text.rfind(_T("http://"), 0) == 0) {
            text = text.substr(7);
        }
        else if (text.rfind(_T("www."), 0) == 0) {
            text = text.substr(4);
        }
        else {
            break;
        }
    }
    while (!text.empty() && (text.back() == _T('/'))) {
        text.pop_back();
    }
    return text;
}

bool IsRichTextLinkUrlDifferentFromText(const std::wstring_view& linkText, const DString& linkUrl)
{
    if (linkUrl.empty() || linkText.empty()) {
        return false;
    }
    const DStringW linkTextW(linkText.data(), linkText.size());
    return NormalizeRichTextLinkUrl(StringConvert::WStringToT(linkTextW)) !=
           NormalizeRichTextLinkUrl(DString(linkUrl.c_str()));
}

DString TrimRichText(const DString& text, RichTextTrimPolicy policy)
{
    if (policy == RichTextTrimPolicy::kNone) {
        //不处理
        return text;
    }

    //回车/换行：替换为1个空格（TAB保留，用于绘制制表位）
    DString tempText = text;
    for (DString::value_type& ch : tempText) {
        if ((ch == _T('\r')) || (ch == _T('\n'))) {
            ch = _T(' ');
        }
    }

    if (policy == RichTextTrimPolicy::kKeepOne) {
        //把连续的空白压缩为一个空格
        DString result;
        result.reserve(tempText.size());
        for (DString::value_type ch : tempText) {
            if (ch == _T(' ')) {
                if (result.empty() || (result.back() != _T(' '))) {
                    result += ch;
                }
            }
            else {
                result += ch;
            }
        }
        tempText.swap(result);
    }
    else {
        //去掉左右两侧的空格，但保留跨文本节点的词间空格
        const bool bFirstSpace = !tempText.empty() && (tempText.front() == _T(' '));
        const bool bLastSpace = !tempText.empty() && (tempText.back() == _T(' '));
        StringUtil::Trim(tempText);
        if (tempText.empty()) {
            if (bFirstSpace || bLastSpace) {
                tempText = DString(_T(" "));
            }
        }
        else {
            if (bFirstSpace) {
                tempText = DString(_T(" ")) + tempText;
            }
            if (bLastSpace) {
                tempText += DString(_T(" "));
            }
        }
    }
    return tempText;
}

void TrimRichTextSlices(std::vector<std::wstring_view>& textViews)
{
    //跨文本节点的空格处理：保留词间的一个空格，去掉整段文本开头和结尾的空格
    bool bPrevSpace = true;
    for (std::wstring_view& textView : textViews) {
        if (textView.empty()) {
            continue;
        }
        if ((textView.size() == 1) && (textView[0] == L'\n')) {
            //换行后，下一行开头的空格需要去掉
            bPrevSpace = true;
            continue;
        }
        if (bPrevSpace) {
            //上一个文本片段已经以空格结尾，本片段开头的空格全部去掉
            size_t nCount = 0;
            while ((nCount < textView.size()) && (textView[nCount] == L' ')) {
                ++nCount;
            }
            textView.remove_prefix(nCount);
        }
        else {
            //本片段开头的连续空格最多保留一个
            size_t nCount = 0;
            while ((nCount < textView.size()) && (textView[nCount] == L' ')) {
                ++nCount;
            }
            if (nCount > 1) {
                textView.remove_prefix(nCount - 1);
            }
        }
        //本片段结尾的连续空格最多保留一个
        if (textView.size() > 1) {
            size_t nCount = 0;
            while ((nCount + 1 < textView.size()) &&
                   (textView[textView.size() - 1 - nCount] == L' ') &&
                   (textView[textView.size() - 2 - nCount] == L' ')) {
                ++nCount;
            }
            if (nCount > 0) {
                textView.remove_suffix(nCount);
            }
        }
        if (!textView.empty()) {
            bPrevSpace = (textView.back() == L' ');
        }
    }

    bool bNextSpace = true;
    for (auto it = textViews.rbegin(); it != textViews.rend(); ++it) {
        std::wstring_view& textView = *it;
        if (textView.empty()) {
            continue;
        }
        if ((textView.size() == 1) && (textView[0] == L'\n')) {
            bNextSpace = true;
            continue;
        }
        if (bNextSpace) {
            size_t nCount = 0;
            while ((nCount < textView.size()) && (textView[textView.size() - 1 - nCount] == L' ')) {
                ++nCount;
            }
            textView.remove_suffix(nCount);
        }
        if (!textView.empty()) {
            bNextSpace = (textView.front() == L' ');
        }
    }
}

std::wstring NormalizeRichTextSelectedText(const std::wstring_view& text)
{
    std::wstring selectedText;
    selectedText.reserve(text.size() + 8);
    for (wchar_t ch : text) {
        if (ch == L'\r') {
            continue;
        }
        if (ch == L'\n') {
            selectedText += L"\r\n";
            continue;
        }
        selectedText += ch;
    }
    return selectedText;
}

}//namespace ui
