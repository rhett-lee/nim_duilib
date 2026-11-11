#include "RichTextImpl.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/Clipboard.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Core/Window.h"

#include <utility>
#include <algorithm>

namespace ui 
{

namespace
{
/** 判断文本中是否包含XML特殊字符
*/
DString EscapeXmlText(const DString& text)
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

/** 转义XML属性值中的特殊字符
*/
DString EscapeXmlAttribute(const DString& text)
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

/** 判断以逗号分隔的对齐值中，是否包含指定的值（兼容"center"作为"hcenter"的别名）
*/
bool ContainsAlignValue(const DString& text, const DString::value_type* value)
{
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

/** 是否为RichText支持的标签名称
*/
bool IsSupportedTagName(const DString& tagName)
{
    DString name = tagName;
    StringUtil::LowerString(name);
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

/** 检查替换花括号后的文本中，所有标签是否都是RichText支持的标签
*/
bool IsSupportedTagsText(const DString& text)
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
        if ((nNameEnd == nNameBegin) || !IsSupportedTagName(text.substr(nNameBegin, nNameEnd - nNameBegin))) {
            return false;
        }
        nPos = nEnd + 1;
    }
    return true;
}
} // namespace

RichTextImpl::RichTextImpl(Control* pOwner) :
    m_pOwner(pOwner),
    m_hAlignType(HorAlignType::kAlignLeft),
    m_vAlignType(VerAlignType::kAlignTop),
    m_fRowSpacingMul(1.0f),
    m_fRowSpacingAdd(0.0f),
    m_bLinkUnderlineFont(true),
    m_nTextDataDisplayScaleFactor(0),
    m_bWordWrap(true),
    m_bReplaceBrace(true),
    m_bEnableRedraw(true)
{
}

RichTextImpl::~RichTextImpl()
{
    m_spDrawRichTextCache.reset();
}

bool RichTextImpl::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("text_align")) {
        //水平方向对齐方式（"center"作为"hcenter"的别名，兼容以逗号分隔的多个取值）
        if (ContainsAlignValue(strValue, _T("left"))) {
            SetTextHAlignType(HorAlignType::kAlignLeft);
        }
        else if (ContainsAlignValue(strValue, _T("hcenter")) || ContainsAlignValue(strValue, _T("center"))) {
            SetTextHAlignType(HorAlignType::kAlignCenter);
        }
        else if (ContainsAlignValue(strValue, _T("right"))) {
            SetTextHAlignType(HorAlignType::kAlignRight);
        }

        //垂直方向对齐方式
        if (ContainsAlignValue(strValue, _T("top"))) {
            SetTextVAlignType(VerAlignType::kAlignTop);
        }
        else if (ContainsAlignValue(strValue, _T("vcenter"))) {
            SetTextVAlignType(VerAlignType::kAlignCenter);
        }
        else if (ContainsAlignValue(strValue, _T("bottom"))) {
            SetTextVAlignType(VerAlignType::kAlignBottom);
        }
        m_textData.clear();
        m_spDrawRichTextCache.reset();
        ++m_nLayoutVersion;
    }    
    else if (strName == _T("font")) {
        SetFontId(strValue);
    }
    else if ((strName == _T("text_color")) || (strName == _T("normal_text_color"))) {
        SetTextColor(strValue);
    }    
    else if ((strName == _T("text_padding")) || (strName == _T("textpadding"))) {
        UiPadding rcTextPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcTextPadding);
        SetTextPadding(rcTextPadding);
    }
    else if (strName == _T("row_spacing_mul")) {
        SetRowSpacingMul(StringUtil::StringToFloat(strValue.c_str(), nullptr));
    }
    else if (strName == _T("row_spacing_add")) {
        SetRowSpacingAdd(StringUtil::StringToFloat(strValue.c_str(), nullptr));
    }
    else if (strName == _T("default_link_font_color")) {
        //超级链接：常规文本颜色值
        m_linkNormalTextColor = strValue;
    }
    else if ((strName == _T("hovered_link_font_color")) || (strName == _T("hover_link_font_color"))) {
        //超级链接：Hover状态文本颜色值
        m_linkHoveredTextColor = strValue;
    }
    else if ((strName == _T("pressed_link_font_color")) || (strName == _T("mouse_down_link_font_color"))) {
        //超级链接：鼠标按下状态文本颜色值
        m_linkPressedTextColor = strValue;
    }
    else if (strName == _T("link_font_underline")) {
        //超级链接：是否使用带下划线的字体
        m_bLinkUnderlineFont = (StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("visited_link_font_color")) {
        //超级链接：访问过（点击过）的文本颜色
        m_linkVisitedTextColor = strValue;
    }
    else if (strName == _T("letter_spacing")) {
        //字间距（像素）
        float fLetterSpacing = StringUtil::StringToFloat(strValue.c_str(), nullptr);
        if (fLetterSpacing < 0.0f) {
            fLetterSpacing = 0.0f;
        }
        if (m_fLetterSpacing != fLetterSpacing) {
            m_fLetterSpacing = fLetterSpacing;
            Redraw();
        }
    }
    else if (strName == _T("text_indent")) {
        //首行缩进（像素）
        int32_t nTextIndent = StringUtil::StringToInt32(strValue.c_str());
        if (nTextIndent < 0) {
            nTextIndent = 0;
        }
        if (m_nTextIndent != nTextIndent) {
            m_nTextIndent = nTextIndent;
            Redraw();
        }
    }
    else if (strName == _T("paragraph_spacing")) {
        //段间距（像素）：用于 <p>/<h1>~<h6> 等块级标签
        int32_t nParagraphSpacing = StringUtil::StringToInt32(strValue.c_str());
        if (nParagraphSpacing < 0) {
            nParagraphSpacing = 0;
        }
        if (m_nParagraphSpacing != nParagraphSpacing) {
            m_nParagraphSpacing = nParagraphSpacing;
        }
    }
    else if (strName == _T("max_lines")) {
        //最多显示的行数
        int32_t nMaxLines = StringUtil::StringToInt32(strValue.c_str());
        if (nMaxLines < 0) {
            nMaxLines = 0;
        }
        if (m_nMaxLines != nMaxLines) {
            m_nMaxLines = nMaxLines;
            Redraw();
        }
    }
    else if (strName == _T("text_overflow")) {
        //超出最大行数时的处理方式：ellipsis（省略号）或 clip（裁剪）
        const bool bEllipsis = (strValue != _T("clip"));
        if (m_bTextOverflowEllipsis != bEllipsis) {
            m_bTextOverflowEllipsis = bEllipsis;
            Redraw();
        }
    }
    else if ((strName == _T("text_align")) || (strName == _T("textalign"))) {
        //文本水平对齐方式：justify 表示两端对齐
        if (strValue == _T("justify")) {
            if (!m_bHAlignJustify) {
                m_bHAlignJustify = true;
                Redraw();
            }
        }
        else {
            if (m_bHAlignJustify) {
                m_bHAlignJustify = false;
                Redraw();
            }
            if (strValue == _T("center")) {
                SetTextHAlignType(HorAlignType::kAlignCenter);
            }
            else if ((strValue == _T("right")) || (strValue == _T("end"))) {
                SetTextHAlignType(HorAlignType::kAlignRight);
            }
            else if ((strValue == _T("left")) || (strValue == _T("start"))) {
                SetTextHAlignType(HorAlignType::kAlignLeft);
            }
        }
    }
    else if (strName == _T("replace_brace")) {
        //对text属性，是否允许替换花括号
        m_bReplaceBrace = StringUtil::IsValueTrue(strValue);
    }
    else if (strName == _T("text")) {
        //允许使用'{'代替'<'，'}'代替'>' (m_bReplaceBrace变量为开关)
        if (m_bReplaceBrace &&
            ((strValue.find(_T('<')) == DString::npos) && (strValue.find(_T('>')) == DString::npos)) &&
            ((strValue.find(_T('{')) != DString::npos) && (strValue.find(_T('}')) != DString::npos))) {
            DString richText(strValue);
            StringUtil::ReplaceAll(_T("{"), _T("<"), richText);
            StringUtil::ReplaceAll(_T("}"), _T(">"), richText);
            if (IsSupportedTagsText(richText)) {
                //仅当替换后是合法的RichText标签时，才按富文本解析
                SetText(richText);
            }
            else {
                //否则视为普通文本，避免内容被误解析或丢失
                SetText(strValue);
            }
        }
        else {
            SetText(strValue);
        }        
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
        SetTextId(strValue);
    }
    else if (strName == _T("trim_policy")) {
        if (strValue == _T("all")) {
            m_trimPolicy = TrimPolicy::kAll;
        }
        else if (strValue == _T("none")) {
            m_trimPolicy = TrimPolicy::kNone;
        }
        else if (strValue == _T("keep_one")) {
            m_trimPolicy = TrimPolicy::kKeepOne;
        }
        else {
            m_trimPolicy = TrimPolicy::kAll;
        }
    }
    else if (strName == _T("word_wrap")) {
        SetWordWrap(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("selection_bg_color")) {
        //文本选择区域的背景颜色
        m_selectBgColor = strValue;
        Invalidate();
    }
    else if ((strName == _T("enable_text_select")) || (strName == _T("text_select"))) {
        //是否允许鼠标选择文本
        SetEnableTextSelect(StringUtil::IsValueTrue(strValue));
    }
    else {
        return false;
    }
    return true;
}

void RichTextImpl::OnWindowChanged()
{
    //绑定的窗口变化时，DPI可能发生变化，需要重绘
    Redraw();
}

void RichTextImpl::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!m_pOwner->Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    UiPadding rcTextPadding = GetTextPadding();
    rcTextPadding = m_pOwner->Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    SetTextPadding(rcTextPadding, false);
    //DPI变化时，需要重新绘制（字体大小等需要缩放）
    Redraw();
}

void RichTextImpl::Redraw()
{
    //重新绘制
    m_textData.clear();
    m_spDrawRichTextCache.reset();
    m_bCachedTextRectValid = false;
    ++m_nLayoutVersion;
    //排版属性变化后，文本选择的几何信息需要重新计算
    InvalidateSelectLayout();
    if (IsEnableRedraw()) {
        m_pOwner->Invalidate();
        m_pOwner->RelayoutOrRedraw();
    }    
}

void RichTextImpl::Invalidate()
{
    if (IsEnableRedraw()) {
        m_pOwner->Invalidate();
    }
}

uint16_t RichTextImpl::GetTextStyle() const
{
    uint32_t uTextStyle = 0;
    if (m_bHAlignJustify) {
        uTextStyle |= TEXT_HJUSTIFY;
    }
    else if (m_hAlignType == HorAlignType::kAlignCenter) {
        uTextStyle |= TEXT_HCENTER;
    }
    else if (m_hAlignType == HorAlignType::kAlignRight) {
        uTextStyle |= TEXT_RIGHT;
    }
    else {
        uTextStyle |= TEXT_LEFT;
    }

    if (m_vAlignType == VerAlignType::kAlignCenter) {
        uTextStyle |= TEXT_VCENTER;
    }
    else if (m_vAlignType == VerAlignType::kAlignBottom) {
        uTextStyle |= TEXT_BOTTOM;
    }
    else {
        uTextStyle |= TEXT_TOP;
    }

    if (IsWordWrap()) {
        uTextStyle |= TEXT_WORD_WRAP;
    }
    else {
        uTextStyle &= ~TEXT_WORD_WRAP;
    }

    //不应包含单行属性
    uTextStyle &= ~TEXT_SINGLELINE;

    return ui::TruncateToUInt16(uTextStyle);
}

void RichTextImpl::CalcDestRect(IRender* pRender, const UiRect& rc, UiRect& rect)
{
    if (pRender == nullptr) {
        return;
    }
    //测量结果缓存：可用大小未变化时直接复用，避免重复测量
    if (m_bCachedTextRectValid &&
        (m_cachedAvailableSize.cx == rc.Width()) &&
        (m_cachedAvailableSize.cy == rc.Height())) {
        rect = m_cachedTextRect;
        return;
    }
    rect.Clear();
    if (!m_textData.empty()) {
        std::vector<RichTextData> richTextData;
        richTextData.reserve(m_textData.size());
        const uint16_t nTextStyle = GetTextStyle();
        for (const RichTextData& textData : m_textData) {
            richTextData.push_back(textData);
            //计算时需要带上绘制文字的属性信息
            richTextData[richTextData.size() - 1].m_textStyle = nTextStyle;
        }
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        std::vector<std::vector<UiRect>> richTextRects;
        pRender->MeasureRichText(rc, UiSize(), pRenderFactory, richTextData, &richTextRects);
        ASSERT(richTextRects.size() == m_textData.size());
        if (richTextRects.size() == m_textData.size()) {
            for (size_t index = 0; index < m_textData.size(); ++index) {
                m_textData[index].m_textRects.swap(richTextRects[index]);
            }
        }
    }
    for (const RichTextDataEx& textData : m_textData) {
        for (const UiRect& textRect : textData.m_textRects) {
            rect.Union(textRect);
        }
    }
    //更新测量结果缓存
    m_cachedTextRect = rect;
    m_cachedAvailableSize.cx = rc.Width();
    m_cachedAvailableSize.cy = rc.Height();
    m_bCachedTextRectValid = true;
}

UiSize RichTextImpl::EstimateText(UiSize szAvailable)
{
    UiSize fixedSize;
    IRender* pRender = nullptr;
    if (m_pOwner->GetWindow() != nullptr) {
        pRender = m_pOwner->GetWindow()->GetRender();
    }
    if (pRender == nullptr) {
        return fixedSize;
    }

    int32_t nWidth = szAvailable.cx;
    if (m_pOwner->GetFixedWidth().IsStretch()) {
        //如果是拉伸类型，使用外部宽度
        nWidth = CalcStretchValue(m_pOwner->GetFixedWidth(), szAvailable.cx);
    }
    else if (m_pOwner->GetFixedWidth().IsInt32()) {
        nWidth = m_pOwner->GetFixedWidth().GetInt32();
    }
    else if (m_pOwner->GetFixedWidth().IsAuto()) {
        //宽度为自动时，不限制宽度
        nWidth = m_pOwner->GetMaxWidth();
    }

    //最大高度，不限制
    int32_t nHeight = INT_MAX;

    UiRect rc;
    rc.left = 0;
    rc.right = rc.left + nWidth;
    rc.top = 0;
    rc.bottom = rc.top + nHeight;

    const UiPadding rcTextPadding = GetTextPadding();
    const UiPadding rcPadding = m_pOwner->GetControlPadding();
    if (nWidth != INT_MAX) {
        rc.left += (rcPadding.left + rcTextPadding.left);
        rc.right -= (rcPadding.right + rcTextPadding.right);
    }
    if (nHeight != INT_MAX) {
        rc.top += (rcPadding.top + rcTextPadding.top);
        rc.bottom -= (rcPadding.bottom + rcTextPadding.bottom);
    }
    if (rc.IsEmpty()) {
        return fixedSize;
    }

    //检查并更新文本
    CheckParseText();

    //计算绘制所占的区域大小
    UiRect rect;
    CalcDestRect(pRender, rc, rect);

    fixedSize.cx = rect.Width();
    fixedSize.cx += (rcTextPadding.left + rcTextPadding.right);
    fixedSize.cx += (rcPadding.left + rcPadding.right);

    fixedSize.cy = rect.Height();
    fixedSize.cy += (rcTextPadding.top + rcTextPadding.bottom);
    fixedSize.cy += (rcPadding.top + rcPadding.bottom);

    return fixedSize;
}

void RichTextImpl::PaintText(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    //检查并更新文本
    CheckParseText();

    //文本绘制的目标区域（水平方向的对齐在绘制时按行计算偏移）
    const UiRect rc = GetTextDrawRect(pRender);
    if (rc.IsEmpty()) {
        return;
    }
    const uint32_t uTextStyle = GetTextStyle();

    if (!m_textData.empty()) {
        UiColor normalLinkTextColor;
        if (!m_linkNormalTextColor.empty()) {
            normalLinkTextColor = m_pOwner->GetUiColor(m_linkNormalTextColor.c_str());
        }
        UiColor mouseDownLinkTextColor;
        if (!m_linkPressedTextColor.empty()) {
            mouseDownLinkTextColor = m_pOwner->GetUiColor(m_linkPressedTextColor.c_str());
        }
        UiColor linkHoverTextColor;
        if (!m_linkHoveredTextColor.empty()) {
            linkHoverTextColor = m_pOwner->GetUiColor(m_linkHoveredTextColor.c_str());
        }
        UiColor linkVisitedTextColor;
        if (!m_linkVisitedTextColor.empty()) {
            linkVisitedTextColor = m_pOwner->GetUiColor(m_linkVisitedTextColor.c_str());
        }

        //基础绘制数据只与解析结果相关（链接字体、基础颜色），解析后只重建一次；
        //悬停/按下/访问等状态变化只影响颜色，通过textColors覆盖，不触发重新排版
        if ((m_nRichTextLayoutDataVersion != m_nLayoutVersion) ||
            (m_richTextLayoutData.size() != m_textData.size())) {
            m_richTextLayoutData.clear();
            m_richTextLayoutData.reserve(m_textData.size());
            for (const RichTextDataEx& textDataEx : m_textData) {
                RichTextData textData = textDataEx;
                if (!textDataEx.m_linkUrl.empty()) {
                    //对于超级链接，设置默认文本格式
                    if (textDataEx.m_pLinkFontInfo != nullptr) {
                        //链接字体在解析时已生成，此处直接共享，避免每帧创建字体对象
                        textData.m_pFontInfo = textDataEx.m_pLinkFontInfo;
                    }
                    else {
                        ASSERT(textDataEx.m_pFontInfo != nullptr);
                        textData.m_pFontInfo = textDataEx.m_pFontInfo;
                    }
                    if (!normalLinkTextColor.IsEmpty()) {
                        textData.m_textColor = normalLinkTextColor;//标准状态的字体颜色
                    }
                }
                m_richTextLayoutData.emplace_back(std::move(textData));
            }
            m_nRichTextLayoutDataVersion = m_nLayoutVersion;
        }

        //逐片段计算本帧使用的颜色（只有颜色会随交互状态变化）
        std::vector<UiColor>& textColors = m_richTextTextColorBuffer;
        textColors.clear();
        textColors.reserve(m_textData.size());
        int32_t nLinkGroup = -1;
        const UiString* pLastLinkUrl = nullptr;
        for (size_t nIndex = 0; nIndex < m_textData.size(); ++nIndex) {
            const RichTextDataEx& textDataEx = m_textData[nIndex];
            UiColor textColor = m_richTextLayoutData[nIndex].m_textColor;
            if (!textDataEx.m_linkUrl.empty()) {
                if ((pLastLinkUrl == nullptr) || (*pLastLinkUrl != textDataEx.m_linkUrl)) {
                    ++nLinkGroup;
                    pLastLinkUrl = &textDataEx.m_linkUrl;
                }
                if (textDataEx.m_bMouseDown && !mouseDownLinkTextColor.IsEmpty()) {
                    textColor = mouseDownLinkTextColor;//鼠标按下时的字体颜色
                }
                else if (textDataEx.m_bMouseHover && !linkHoverTextColor.IsEmpty()) {
                    textColor = linkHoverTextColor;//鼠标Hover时的字体颜色
                }
                else if ((nLinkGroup == m_nFocusedLinkGroup) && !linkHoverTextColor.IsEmpty()) {
                    textColor = linkHoverTextColor;//键盘聚焦时的字体颜色
                }
                else if (!linkVisitedTextColor.IsEmpty() &&
                         (m_visitedLinks.find(DString(textDataEx.m_linkUrl.c_str())) != m_visitedLinks.end())) {
                    textColor = linkVisitedTextColor;//访问过的字体颜色
                }
            }
            textColors.push_back(textColor);
        }
        std::vector<std::vector<UiRect>> richTextRects;
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        const UiRect& textRect = rc;
        UiSize szScrollOffset;
        if (m_spDrawRichTextCache != nullptr) {
            //校验缓存，如果失效则重新生成
            //文字颜色不影响排版，悬停/按下时通过pTextColors覆盖，避免重新排版
            if ((m_nDrawCacheLayoutVersion != m_nLayoutVersion) || (m_drawCacheTextRect != textRect)) {
                //布局版本或绘制区域变化时，才需要逐项比较内容
                if (!pRender->IsValidDrawRichTextCache(textRect, m_richTextLayoutData, m_spDrawRichTextCache, false)) {
                    m_spDrawRichTextCache.reset();
                }
            }
        }
        if (m_spDrawRichTextCache == nullptr) {
            bool bRet = pRender->CreateDrawRichTextCache(textRect, szScrollOffset, pRenderFactory,
                                                         m_richTextLayoutData, m_spDrawRichTextCache);
            if (bRet) {
                m_nDrawCacheLayoutVersion = m_nLayoutVersion;
                m_drawCacheTextRect = textRect;
            }
            else {
                m_spDrawRichTextCache.reset();
            }
        }

        //绘制文本选择的背景（在绘制文字之前，避免遮挡文字）
        if (HasSelection() && UpdateTextSelectLayout(pRender)) {
            std::vector<UiRect> selectionRects;
            GetSelectionRects(selectionRects);
            if (!selectionRects.empty()) {
                UiColor selectBgColor;
                if (!m_selectBgColor.empty()) {
                    selectBgColor = m_pOwner->GetUiColor(m_selectBgColor.c_str());
                }
                if (selectBgColor.IsEmpty()) {
                    selectBgColor = UiColor(0x66, 0x33, 0x99, 0xFF);
                }
                for (UiRect& rcSelect : selectionRects) {
                    rcSelect.Offset(textRect.left, textRect.top);
                    UiRect rcDraw;
                    if (UiRect::Intersect(rcDraw, rcSelect, textRect)) {
                        pRender->FillRect(UiRectF::MakeFromRect(rcDraw), selectBgColor, m_pOwner->GetAlpha());
                    }
                }
            }
        }

        if (m_spDrawRichTextCache != nullptr) {
            //使用绘制缓存
            std::vector<int32_t> rowXOffset;//对齐方式为居中或者靠右时使用
            pRender->DrawRichTextCacheData(m_spDrawRichTextCache, textRect, szScrollOffset,
                                           rowXOffset, m_pOwner->GetAlpha(), &richTextRects, &textColors);

#if 0
            //测试代码结束(比较使用缓存绘制和不使用缓存绘制时结果是否相同)
            std::vector<std::vector<UiRect>> richTextRectsOld;
            pRender->DrawRichText(textRect, szScrollOffset, pRenderFactory, m_richTextLayoutData, GetAlpha(), &richTextRectsOld);
            ASSERT(richTextRects.size() == m_textData.size());
            ASSERT(richTextRectsOld.size() == m_textData.size());
            if (richTextRectsOld.size() == richTextRects.size()) {
                for (size_t i = 0; i < richTextRectsOld.size(); ++i) {
                    const std::vector<UiRect>& rectOld = richTextRectsOld[i];
                    const std::vector<UiRect>& rectNew = richTextRects[i];
                    ASSERT(rectOld.size() == rectNew.size());
                    for (size_t j = 0; j < rectOld.size(); ++j) {
                        const UiRect& oldRect = rectOld[j];
                        const UiRect& newRect = rectNew[j];
                        ASSERT(oldRect == newRect);
                    }
                }

            }
#endif      //测试代码结束
        }
        else {
            //不使用绘制缓存            
            pRender->DrawRichText(textRect, szScrollOffset, pRenderFactory, m_richTextLayoutData, m_pOwner->GetAlpha(), &richTextRects);
        }
        ASSERT(richTextRects.size() == m_textData.size());
        if (richTextRects.size() == m_textData.size()) {
            for (size_t index = 0; index < m_textData.size(); ++index) {
                m_textData[index].m_textRects.swap(richTextRects[index]);
            }
        }
    }
}

void RichTextImpl::CheckParseText()
{
    if (!m_richTextId.empty() && (m_langFileName != GlobalManager::Instance().GetLanguageFileName())) {
        //多语言版：当语言发生变化时，更新文本内容
        DoSetText(GlobalManager::Instance().Lang().GetStringByID(m_richTextId.c_str()));
        m_langFileName = GlobalManager::Instance().GetLanguageFileName();
    }

    //当DPI变化时，需要重新解析文本，更新字体大小
    if (m_nTextDataDisplayScaleFactor != m_pOwner->Dpi().GetDisplayScaleFactor()) {
        m_textData.clear();
        m_spDrawRichTextCache.reset();
        m_bCachedTextRectValid = false;
        ++m_nLayoutVersion;
        //文本的排版结果变化后，已选择的文本失效
        ResetTextSelect();
    }

    if (m_textData.empty()) {
        ParseText(m_textData);
        m_nTextDataDisplayScaleFactor = m_pOwner->Dpi().GetDisplayScaleFactor();
        m_spDrawRichTextCache.reset();
        m_bCachedTextRectValid = false;
        ++m_nLayoutVersion;
    }
}

bool RichTextImpl::ParseText(std::vector<RichTextDataEx>& outTextData) const
{
    //默认字体
    DString sFontId = GetFontId();
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(sFontId, m_pOwner->Dpi());
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return false;
    }

    RichTextDataEx parentTextData;
    //默认文本颜色
    parentTextData.m_textColor = m_pOwner->GetUiColor(GetTextColor());
    if (parentTextData.m_textColor.IsEmpty()) {
        parentTextData.m_textColor = UiColor(UiColors::Black);
    }
    parentTextData.m_pFontInfo.reset(new UiFontEx);
    parentTextData.m_pFontInfo->m_fontName = pFont->FontName();
    parentTextData.m_pFontInfo->m_fontSize = pFont->FontSize();
    parentTextData.m_pFontInfo->m_bBold = pFont->IsBold();
    parentTextData.m_pFontInfo->m_bUnderline = pFont->IsUnderline();
    parentTextData.m_pFontInfo->m_bItalic = pFont->IsItalic();
    parentTextData.m_pFontInfo->m_bStrikeOut = pFont->IsStrikeOut();
    parentTextData.m_fRowSpacingMul = m_fRowSpacingMul;
    parentTextData.m_fRowSpacingAdd = m_fRowSpacingAdd;
    parentTextData.m_textStyle = GetTextStyle();
    //字间距与首行缩进：按当前DPI缩放
    parentTextData.m_fLetterSpacing = m_pOwner->Dpi().GetScaleFloat(m_fLetterSpacing);
    parentTextData.m_nFirstLineIndent = m_pOwner->Dpi().GetScaleInt(m_nTextIndent);
    parentTextData.m_nMaxLines = m_nMaxLines;
    parentTextData.m_bTextOverflowEllipsis = m_bTextOverflowEllipsis;

    std::vector<RichTextDataEx> textData;

    for (const RichTextSlice& textSlice : m_textSlice) {
        if (!ParseTextSlice(textSlice, parentTextData, textData)) {
            return false;
        }
    }

    if (m_trimPolicy != TrimPolicy::kNone) {
        //跨文本节点的空格处理：保留词间的一个空格，去掉整段文本开头和结尾的空格
        bool bPrevSpace = true;
        for (RichTextDataEx& textDataItem : textData) {
            std::wstring_view& textView = textDataItem.m_textView;
            if (textView.empty()) {
                continue;
            }
            if ((textView.size() == 1) && (textView[0] == L'\n')) {
                //换行后，下一行开头的空格需要去掉
                bPrevSpace = true;
                continue;
            }
            if (bPrevSpace) {
                size_t nCount = 0;
                while ((nCount < textView.size()) && (textView[nCount] == L' ')) {
                    ++nCount;
                }
                textView.remove_prefix(nCount);
            }
            if (!textView.empty()) {
                bPrevSpace = (textView.back() == L' ');
            }
        }

        bool bNextSpace = true;
        for (auto it = textData.rbegin(); it != textData.rend(); ++it) {
            std::wstring_view& textView = it->m_textView;
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

    outTextData.swap(textData);
    return true;
}

bool RichTextImpl::ParseTextSlice(const RichTextSlice& textSlice, 
                                  const RichTextDataEx& parentTextData, 
                                  std::vector<RichTextDataEx>& textData) const
{
    //当前节点
    RichTextDataEx currentTextData;
    currentTextData.m_fRowSpacingMul = parentTextData.m_fRowSpacingMul;
    currentTextData.m_fRowSpacingAdd = parentTextData.m_fRowSpacingAdd;
    currentTextData.m_textStyle = parentTextData.m_textStyle;
    currentTextData.m_textView = std::wstring_view(textSlice.m_text.c_str(), textSlice.m_text.size());
    //继承父节点的排版属性
    currentTextData.m_fLetterSpacing = parentTextData.m_fLetterSpacing;
    currentTextData.m_nFirstLineIndent = parentTextData.m_nFirstLineIndent;
    currentTextData.m_nMaxLines = parentTextData.m_nMaxLines;
    currentTextData.m_bTextOverflowEllipsis = parentTextData.m_bTextOverflowEllipsis;
    currentTextData.m_bHorizontalRule = textSlice.m_bHorizontalRule;
    if (textSlice.m_fRowSpacingAdd != 0.0f) {
        //段间距：由所属的文本片段增加行间距附加量
        currentTextData.m_fRowSpacingAdd += m_pOwner->Dpi().GetScaleFloat(textSlice.m_fRowSpacingAdd);
    }
    if (textSlice.m_nHAlign >= 0) {
        //逐段对齐：覆盖水平对齐方式
        currentTextData.m_textStyle = static_cast<uint16_t>(currentTextData.m_textStyle &
                                                            static_cast<uint16_t>(~DrawStringFormat::TEXT_HALIGN_ALL));
        if (textSlice.m_nHAlign == 1) {
            currentTextData.m_textStyle |= DrawStringFormat::TEXT_HCENTER;
        }
        else if (textSlice.m_nHAlign == 2) {
            currentTextData.m_textStyle |= DrawStringFormat::TEXT_RIGHT;
        }
        else if (textSlice.m_nHAlign == 3) {
            currentTextData.m_textStyle |= DrawStringFormat::TEXT_HJUSTIFY;
        }
        else {
            currentTextData.m_textStyle |= DrawStringFormat::TEXT_LEFT;
        }
    }

    //文本中可以包含换行符和TAB（绘制时由DrawRichText负责处理）
    currentTextData.m_linkUrl = textSlice.m_linkUrl;
    if (currentTextData.m_linkUrl.empty()) {
        //如果是链接的子节点，则继承父节点的链接地址
        currentTextData.m_linkUrl = parentTextData.m_linkUrl;
    }
    currentTextData.m_linkTitle = textSlice.m_linkTitle;
    if (currentTextData.m_linkTitle.empty()) {
        currentTextData.m_linkTitle = parentTextData.m_linkTitle;
    }
    //基线偏移：用于上标/下标
    currentTextData.m_nBaselineOffset = parentTextData.m_nBaselineOffset +
        m_pOwner->Dpi().GetScaleInt(textSlice.m_nBaselineOffset);
    if (!textSlice.m_textColor.empty()) {
        currentTextData.m_textColor = m_pOwner->GetUiColor(textSlice.m_textColor.c_str());
    }
    if (textSlice.m_textColor.empty()) {
        currentTextData.m_textColor = parentTextData.m_textColor;
    }

    if (!textSlice.m_bgColor.empty()) {
        currentTextData.m_bgColor = m_pOwner->GetUiColor(textSlice.m_bgColor.c_str());
    }
    else {
        currentTextData.m_bgColor = parentTextData.m_bgColor;
    }

    //字体信息：没有字体属性时共享父节点的字体，避免每个文本节点都创建字体对象
    const bool bHasFontAttribute = !textSlice.m_fontInfo.m_fontName.empty() ||
                                   (textSlice.m_fontInfo.m_fontSize > 0) ||
                                   textSlice.m_fontInfo.m_bBold ||
                                   textSlice.m_fontInfo.m_bUnderline ||
                                   textSlice.m_fontInfo.m_bItalic ||
                                   textSlice.m_fontInfo.m_bStrikeOut ||
                                   (textSlice.m_fFontScale != 1.0f);
    if (!bHasFontAttribute && (parentTextData.m_pFontInfo != nullptr)) {
        currentTextData.m_pFontInfo = parentTextData.m_pFontInfo;
    }
    else {
        currentTextData.m_pFontInfo.reset(new UiFontEx);
        if (parentTextData.m_pFontInfo != nullptr) {
            currentTextData.m_pFontInfo->CopyFrom(*parentTextData.m_pFontInfo);
        }
        if (!textSlice.m_fontInfo.m_fontName.empty()) {
            currentTextData.m_pFontInfo->m_fontName = textSlice.m_fontInfo.m_fontName;
        }
        if (textSlice.m_fontInfo.m_fontSize > 0) {
            currentTextData.m_pFontInfo->m_fontSize = m_pOwner->Dpi().GetScaleInt(textSlice.m_fontInfo.m_fontSize); //字体大小，需要DPI缩放
        }
        if (textSlice.m_fontInfo.m_bBold) {
            currentTextData.m_pFontInfo->m_bBold = textSlice.m_fontInfo.m_bBold;
        }
        if (textSlice.m_fontInfo.m_bUnderline) {
            currentTextData.m_pFontInfo->m_bUnderline = textSlice.m_fontInfo.m_bUnderline;
        }
        if (textSlice.m_fontInfo.m_bItalic) {
            currentTextData.m_pFontInfo->m_bItalic = textSlice.m_fontInfo.m_bItalic;
        }
        if (textSlice.m_fontInfo.m_bStrikeOut) {
            currentTextData.m_pFontInfo->m_bStrikeOut = textSlice.m_fontInfo.m_bStrikeOut;
        }
        if ((textSlice.m_fFontScale > 0.0f) && (textSlice.m_fFontScale != 1.0f) &&
            (currentTextData.m_pFontInfo->m_fontSize > 0)) {
            //字体缩放：用于 <h1>~<h6>、<sup>、<sub>
            const float fFontSize = (float)currentTextData.m_pFontInfo->m_fontSize * textSlice.m_fFontScale;
            currentTextData.m_pFontInfo->m_fontSize = std::max(1, (int32_t)(fFontSize + 0.5f));
        }
    }

    if (!currentTextData.m_linkUrl.empty() && m_bLinkUnderlineFont &&
        ((currentTextData.m_pFontInfo == nullptr) || !currentTextData.m_pFontInfo->m_bUnderline)) {
        //链接的需要显示下划线：在解析时生成字体对象，绘制时共享，避免每帧创建
        currentTextData.m_pLinkFontInfo.reset(new UiFontEx);
        if (currentTextData.m_pFontInfo != nullptr) {
            currentTextData.m_pLinkFontInfo->CopyFrom(*currentTextData.m_pFontInfo);
        }
        currentTextData.m_pLinkFontInfo->m_bUnderline = true;
    }

    if (!textSlice.m_imageSrc.empty()) {
        //内联图片：解析时加载并解码位图，绘制时直接复用（避免每帧加载）
        FilePath windowResPath;
        if (m_pOwner->GetWindow() != nullptr) {
            windowResPath = m_pOwner->GetWindow()->GetResourcePath();
        }
        FilePath imageFileFullPath;
        std::vector<uint8_t> imageFileData;
        const ThemeManager& themeMgr = GlobalManager::Instance().Theme();
        if (themeMgr.GetResFile(FilePath(textSlice.m_imageSrc.c_str()), windowResPath, imageFileFullPath, imageFileData)) {
            ImageDecodeParam decodeParam;
            decodeParam.m_imageFilePath = imageFileFullPath;
            decodeParam.m_pFileData = std::make_shared<std::vector<uint8_t>>();
            decodeParam.m_fImageSizeScale = m_pOwner->Dpi().GetDisplayScale();
            decodeParam.m_bAssertEnabled = false;
            if (imageFileData.empty()) {
                FileUtil::ReadFileData(decodeParam.m_imageFilePath, *decodeParam.m_pFileData);
            }
            else {
                decodeParam.m_pFileData->swap(imageFileData);
            }
            currentTextData.m_pImage = GlobalManager::Instance().ImageDecoders().DecodeImageData(decodeParam);
        }
        if (currentTextData.m_pImage != nullptr) {
            //显示尺寸：指定值时按96 DPI进行缩放，未指定时使用位图的原始尺寸（位图已按DPI缩放）
            const int32_t nBitmapWidth = (int32_t)currentTextData.m_pImage->GetWidth();
            const int32_t nBitmapHeight = (int32_t)currentTextData.m_pImage->GetHeight();
            currentTextData.m_nImageWidth = (textSlice.m_nImageWidth > 0) ?
                m_pOwner->Dpi().GetScaleInt(textSlice.m_nImageWidth) : nBitmapWidth;
            currentTextData.m_nImageHeight = (textSlice.m_nImageHeight > 0) ?
                m_pOwner->Dpi().GetScaleInt(textSlice.m_nImageHeight) : nBitmapHeight;
        }
    }

    if (!currentTextData.m_textView.empty() || currentTextData.m_bHorizontalRule ||
        (currentTextData.m_pImage != nullptr)) {
        //空文本节点不参与排版（链接的文本由子节点承载）；水平分割线和内联图片没有文本，需要参与排版
        if (textSlice.m_children.empty()) {
            //叶子节点：直接移动，避免解析结果拷贝
            textData.emplace_back(std::move(currentTextData));
        }
        else {
            //有子节点时，子节点需要继承当前节点的属性，不能移动
            textData.push_back(currentTextData);
        }
    }
        
    if (!textSlice.m_children.empty()) {
        //子节点
        for (const RichTextSlice& childSlice : textSlice.m_children) {
            if (!ParseTextSlice(childSlice, currentTextData, textData)) {
                return false;
            }
        }
    }
    return true;
}

UiPadding RichTextImpl::GetTextPadding() const
{
    return UiPadding(m_rcTextPadding.left, m_rcTextPadding.top, m_rcTextPadding.right, m_rcTextPadding.bottom);
}

void RichTextImpl::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
{
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    if ((padding.left < 0) || (padding.top < 0) ||
        (padding.right < 0) || (padding.bottom < 0)) {
        return;
    }
    if (bNeedDpiScale) {
        m_pOwner->Dpi().ScalePadding(padding);
    }
    if (!GetTextPadding().Equals(padding)) {
        m_rcTextPadding.left = TruncateToUInt16(padding.left);
        m_rcTextPadding.top = TruncateToUInt16(padding.top);
        m_rcTextPadding.right = TruncateToUInt16(padding.right);
        m_rcTextPadding.bottom = TruncateToUInt16(padding.bottom);
        //文本区域变化后，文本选择的几何信息需要重新计算
        InvalidateSelectLayout();
        if (IsEnableRedraw()) {
            m_pOwner->RelayoutOrRedraw();
        }        
    }
}

const DString& RichTextImpl::TrimText(DString& text)
{
    if (m_trimPolicy == TrimPolicy::kNone) {
        //不处理
        return text;
    }

    //回车/换行/TAB键：替换为1个空格
    DString tempText;
    tempText.swap(text);
    for (DString::value_type& ch : tempText) {
        if ((ch == _T('\r')) || (ch == _T('\n')) || (ch == _T('\t'))) {
            ch = _T(' ');
        }
    }

    if (m_trimPolicy == TrimPolicy::kKeepOne) {
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
    tempText.swap(text);
    return text;
}

DString RichTextImpl::TrimText(const DString::value_type* text)
{
    DString retText;
    if (text != nullptr) {
        retText = text;
    }
    return TrimText(retText);
}

void RichTextImpl::SetEnableRedraw(bool bEnable)
{
    m_bEnableRedraw = bEnable;
}

bool RichTextImpl::IsEnableRedraw() const
{
    return m_bEnableRedraw;
}

bool RichTextImpl::DoSetText(const DString& richText)
{
    Clear();
    //XML解析的内容，全部封装在WindowBuilder这个类中，以避免到处使用XML解析器，从而降低代码维护复杂度
    bool bResult = true;
    if (!richText.empty()) {
        DString trimmedText = richText;
        StringUtil::Trim(trimmedText);
        if (trimmedText.find(_T("<RichText")) == 0) {
            bResult = WindowBuilder::ParseRichTextXmlText(trimmedText, m_pOwner);
        }
        else {
            DString formatedText = _T("<RichText>") + richText + _T("</RichText>");
            bResult = WindowBuilder::ParseRichTextXmlText(formatedText, m_pOwner);
        }
        if (!bResult) {
            //解析失败时，按纯文本降级显示，避免文本内容丢失
            m_textSlice.clear();
            m_textData.clear();
            m_spDrawRichTextCache.reset();
            ++m_nLayoutVersion;
            DString formatedText = _T("<RichText>") + EscapeXmlText(richText) + _T("</RichText>");
            bResult = WindowBuilder::ParseRichTextXmlText(formatedText, m_pOwner);
        }
    }
    return bResult;
}

bool RichTextImpl::SetText(const DString& richText, bool bRedraw)
{
    //直接设置文本时，清除通过文本ID设置的记录（SetTextId在调用SetText后会重新设置）
    m_richTextId.clear();
    m_langFileName.clear();
    bool bResult = DoSetText(richText);
    if (bResult && bRedraw) {
        if (IsEnableRedraw()) {
            m_pOwner->RelayoutOrRedraw();
        }
    }
    return bResult;
}

bool RichTextImpl::SetTextId(const DString& richTextId, bool bRedraw)
{
    bool bRet = SetText(GlobalManager::Instance().Lang().GetStringByID(richTextId), bRedraw);
    m_richTextId = richTextId;
    if (!m_richTextId.empty()) {
        m_langFileName = GlobalManager::Instance().GetLanguageFileName();
    }
    else {
        m_langFileName.clear();
    }
    return bRet;
}

void RichTextImpl::Clear()
{
    m_textData.clear();
    m_spDrawRichTextCache.reset();
    m_bCachedTextRectValid = false;
    ++m_nLayoutVersion;
    //文本内容变化后，已选择的文本失效
    ResetTextSelect();
    if (!m_textSlice.empty()) {
        m_textSlice.clear();
        Invalidate();
    }
}

DString RichTextImpl::GetFontId() const
{
    return m_sFontId.c_str();
}

void RichTextImpl::SetFontId(const DString& strFontId)
{
    if (m_sFontId != strFontId) {
        m_sFontId = strFontId;
        Redraw();
    }
}

DString RichTextImpl::GetTextColor() const
{
    return m_sTextColor.c_str();
}

void RichTextImpl::SetTextColor(const DString& sTextColor)
{
    if (m_sTextColor != sTextColor) {
        m_sTextColor = sTextColor;
        Redraw();
    }
}

float RichTextImpl::GetRowSpacingMul() const
{
    return m_fRowSpacingMul;
}

void RichTextImpl::SetRowSpacingMul(float fRowSpacingMul)
{
    if (m_fRowSpacingMul != fRowSpacingMul) {
        m_fRowSpacingMul = fRowSpacingMul;
        if (m_fRowSpacingMul <= 0.01f) {
            m_fRowSpacingMul = 1.0f;
        }
        Redraw();
    }
}

float RichTextImpl::GetRowSpacingAdd() const
{
    return m_fRowSpacingAdd;
}

void RichTextImpl::SetRowSpacingAdd(float fRowSpacingAdd)
{
    if (m_fRowSpacingAdd != fRowSpacingAdd) {
        m_fRowSpacingAdd = fRowSpacingAdd;
        if (m_fRowSpacingAdd <= 0.0001f) {
            m_fRowSpacingAdd = 0.0f;
        }
        Redraw();
    }
}

bool RichTextImpl::IsWordWrap() const
{
    return m_bWordWrap;
}

void RichTextImpl::SetWordWrap(bool bWordWrap)
{
    if (m_bWordWrap != bWordWrap) {
        m_bWordWrap = bWordWrap;
        Redraw();
    }
}

void RichTextImpl::SetTextHAlignType(HorAlignType alignType)
{
    if (m_hAlignType != alignType) {
        m_hAlignType = alignType;
        Redraw();
    }
}

HorAlignType RichTextImpl::GetHAlignType() const
{
    return m_hAlignType;
}

void RichTextImpl::SetTextVAlignType(VerAlignType alignType)
{
    if (m_vAlignType != alignType) {
        m_vAlignType = alignType;
        Redraw();
    }
}

VerAlignType RichTextImpl::GetVAlignType() const
{
    return m_vAlignType;
}


void RichTextImpl::AppendTextSlice(RichTextSlice&& textSlice, bool bRedraw)
{
    m_textSlice.emplace_back(std::move(textSlice));
    m_textData.clear();
    m_spDrawRichTextCache.reset();
    ++m_nLayoutVersion;
    ResetTextSelect();
    if (bRedraw) {
        //追加内容后，需要重新布局与绘制
        Redraw();
    }
}

void RichTextImpl::AppendTextSlice(const RichTextSlice& textSlice, bool bRedraw)
{
    m_textSlice.emplace_back(textSlice);
    m_textData.clear();
    m_spDrawRichTextCache.reset();
    ++m_nLayoutVersion;
    ResetTextSelect();
    if (bRedraw) {
        //追加内容后，需要重新布局与绘制
        Redraw();
    }
}

DString RichTextImpl::ToString() const
{
    const DString indentValue = _T("    ");
    const DString lineBreak = _T("\r\n");
    DString richText = _T("<RichText>");
    richText += lineBreak;
    for (const RichTextSlice& textSlice : m_textSlice) {
        richText += ToString(textSlice, indentValue);
    }
    richText += _T("</RichText>");
    return richText;
}

DString RichTextImpl::ToString(const RichTextSlice& textSlice, const DString& indent) const
{
    // 支持的标签列表(兼容HTML的标签):
    // 
    // 超级链接：   <a href="URL">文本</a>
    // 粗体字:      <b> </b>
    // 斜体字:      <i> </i>
    // 删除字:      <s> </s> 或 <del> </del> 或者 <strike> </strike>
    // 下划线字:    <u> </u>
    // 设置背景色:  <bgcolor color="#000000"> </bgcolor>
    // 设置字体:    <font face="宋体" size="12" color="#000000">
    // 换行标签：   <br/>
    const DString indentValue = _T("    ");
    const DString lineBreak = _T("\r\n");
    DString richText;
    if (textSlice.m_nodeName.empty()) {
        if (!textSlice.m_text.empty()) {
            richText += indent;
            richText += EscapeXmlText(StringConvert::WStringToT(textSlice.m_text));
            richText += lineBreak;
        }
        //没有节点名称的情况下，就没有属性和子节点，直接返回
        return richText;
    }

    //生成属性列表（多个属性之间使用空格分隔，属性值中的特殊字符需要转义）
    DString attrList;    
    if (!textSlice.m_linkUrl.empty()) {
        if (!attrList.empty()) {
            attrList += _T(" ");
        }
        attrList += _T("href=\"");
        attrList += EscapeXmlAttribute(textSlice.m_linkUrl.c_str());
        attrList += _T("\"");
    }
    if (!textSlice.m_bgColor.empty()) {
        if (!attrList.empty()) {
            attrList += _T(" ");
        }
        attrList += _T("color=\"");
        attrList += EscapeXmlAttribute(textSlice.m_bgColor.c_str());
        attrList += _T("\"");
    }
    if (!textSlice.m_textColor.empty()) {
        if (!attrList.empty()) {
            attrList += _T(" ");
        }
        attrList += _T("color=\"");
        attrList += EscapeXmlAttribute(textSlice.m_textColor.c_str());
        attrList += _T("\"");
    }
    if (!textSlice.m_fontInfo.m_fontName.empty()) {
        if (!attrList.empty()) {
            attrList += _T(" ");
        }
        attrList += _T("face=\"");
        attrList += EscapeXmlAttribute(textSlice.m_fontInfo.m_fontName.c_str());
        attrList += _T("\"");
    }
    if (textSlice.m_fontInfo.m_fontSize != 0) {
        if (!attrList.empty()) {
            attrList += _T(" ");
        }
        attrList += _T("size=\"");
        attrList += StringUtil::Printf(_T("%d"), textSlice.m_fontInfo.m_fontSize);
        attrList += _T("\"");
    }
    if (!textSlice.m_imageSrc.empty()) {
        //内联图片：输出资源路径与显示尺寸
        if (!attrList.empty()) {
            attrList += _T(" ");
        }
        attrList += _T("src=\"");
        attrList += EscapeXmlAttribute(textSlice.m_imageSrc.c_str());
        attrList += _T("\"");
        if (textSlice.m_nImageWidth > 0) {
            attrList += StringUtil::Printf(_T(" width=\"%d\""), textSlice.m_nImageWidth);
        }
        if (textSlice.m_nImageHeight > 0) {
            attrList += StringUtil::Printf(_T(" height=\"%d\""), textSlice.m_nImageHeight);
        }
    }

    if(!textSlice.m_children.empty()) {
        //有子节点：节点开始
        richText += indent;
        richText += _T("<");
        richText += textSlice.m_nodeName.c_str();
        if (!attrList.empty()) {
            richText += _T(" ");
            richText += attrList;
        }
        richText += _T(">");
        richText += lineBreak;

        //添加子节点
        for (const RichTextSlice& childSlice : textSlice.m_children) {
            richText += ToString(childSlice, indent + indentValue);
        }

        //节点结束
        richText += indent;
        richText += _T("</");
        richText += textSlice.m_nodeName.c_str();
        richText += _T(">");
        richText += lineBreak;
    }
    else if (!textSlice.m_linkUrl.empty()) {
        //超级链接节点：需要特殊处理
        richText += indent;
        richText += _T("<");
        richText += textSlice.m_nodeName.c_str();
        if (!attrList.empty()) {
            richText += _T(" ");
            richText += attrList;
        }
        richText += _T(">");

        //添加超链接的文本
        richText += EscapeXmlText(StringConvert::WStringToT(textSlice.m_text));

        //节点结束
        richText += _T("</");
        richText += textSlice.m_nodeName.c_str();
        richText += _T(">");
        richText += lineBreak;
    }
    else {
        //没有子节点：放一行中表示
        richText += indent;
        richText += _T("<");
        richText += textSlice.m_nodeName.c_str();
        if (!attrList.empty()) {
            richText += _T(" ");
            richText += attrList;
        }
        richText += _T("/>");
        richText += lineBreak;
    }
    return richText;
}

void RichTextImpl::ButtonDown(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return;
    }
    bool bInvalidate = false;
    bool bOnLink = false;
    //GetRect()返回的是布局坐标（不包含祖先滚动容器的滚动偏移），需要将鼠标坐标转换为布局坐标
    UiPoint ptMouse = msg.ptMouse;
    ptMouse.Offset(m_pOwner->GetScrollOffsetInScrollBox());
    for (RichTextDataEx& textData : m_textData) {
        bool bMouseDown = false;
        if (!textData.m_linkUrl.empty()) {
            for (const UiRect& textRect : textData.m_textRects) {
                if (textRect.ContainsPt(ptMouse)) {
                    //在超级链接上
                    bMouseDown = true;
                    break;
                }
            }
        }
        if (bMouseDown) {
            bOnLink = true;
        }
        if (textData.m_bMouseDown != bMouseDown) {
            textData.m_bMouseDown = bMouseDown;
            bInvalidate = true;
        }
    }

    //在非链接区域按下鼠标左键：开始选择文本
    if (!bOnLink && m_bEnableTextSelect) {
        IRender* pRender = nullptr;
        Window* pWindow = m_pOwner->GetWindow();
        if (pWindow != nullptr) {
            pRender = pWindow->GetRender();
        }
        if (UpdateTextSelectLayout(pRender)) {
            if (!m_pOwner->IsFocused()) {
                //获取键盘焦点，以便接收Ctrl+A、Ctrl+C等快捷键
                m_pOwner->SetFocus();
            }
            const size_t nPos = HitTestTextPos(ptMouse);
            if ((m_nSelectAnchor != nPos) || (m_nSelectCaret != nPos)) {
                bInvalidate = true;
            }
            m_nSelectAnchor = nPos;
            m_nSelectCaret = nPos;
            m_bSelecting = true;
        }
    }
    if (bInvalidate) {
        //状态变化时，只重绘一次
        Invalidate();
    }
}

void RichTextImpl::ButtonUp(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return;
    }
    //GetRect()返回的是布局坐标（不包含祖先滚动容器的滚动偏移），需要将鼠标坐标转换为布局坐标
    UiPoint ptMouse = msg.ptMouse;
    ptMouse.Offset(m_pOwner->GetScrollOffsetInScrollBox());
    if (m_bSelecting) {
        //结束选择文本
        m_bSelecting = false;
        const size_t nPos = HitTestTextPos(ptMouse);
        if (nPos != m_nSelectCaret) {
            m_nSelectCaret = nPos;
            Invalidate();
        }
    }
    for (RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            continue;
        }
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(ptMouse)) {
                //在超级链接上, 并且与按下鼠标时的相同，则触发点击事件
                if (textData.m_bMouseDown) {
                    textData.m_bMouseDown = false;
                    //记录为已访问的链接（用于visited状态的颜色）
                    m_visitedLinks.insert(DString(textData.m_linkUrl.c_str()));
                    Invalidate();
                    //直接使用成员中的URL，避免使用局部变量导致回调中指针失效
                    m_pOwner->SendEvent(kEventLinkClick, (WPARAM)textData.m_linkUrl.c_str());
                    return;
                }                
            }
        }
    }
    bool bInvalidate = false;
    for (RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            continue;
        }
        if (textData.m_bMouseDown) {
            textData.m_bMouseDown = false;
            bInvalidate = true;
        }
    }
    if (bInvalidate) {
        Invalidate();
    }
}

void RichTextImpl::MouseMove(const EventArgs& msg)
{
    bool bOnLinkUrl = false;
    bool bInvalidate = false;
    //GetRect()返回的是布局坐标（不包含祖先滚动容器的滚动偏移），需要将鼠标坐标转换为布局坐标
    UiPoint ptMouse = msg.ptMouse;
    ptMouse.Offset(m_pOwner->GetScrollOffsetInScrollBox());
    //当前ToolTip文本：判断它是否由悬停的链接设置，避免鼠标移动时误清除
    const DString toolTipText = m_pOwner->GetToolTipText();
    for (RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            if (textData.m_bMouseHover) {
                //从链接上移开时，需要取消Hover状态
                textData.m_bMouseHover = false;
                bInvalidate = true;
            }
            continue;
        }
        bool bMouseHover = false;
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(ptMouse)) {
                //在超级链接上
                bMouseHover = true;
                //ToolTip可能是链接地址，也可能是<a>的title属性（自定义提示文本）
                if ((textData.m_linkUrl == toolTipText) ||
                    (!textData.m_linkTitle.empty() && (textData.m_linkTitle == toolTipText))) {
                    bOnLinkUrl = true;
                }
            }
        }
        if (textData.m_bMouseHover != bMouseHover) {
            textData.m_bMouseHover = bMouseHover;
            bInvalidate = true;
        }
    }
    if (bInvalidate) {
        //只有在链接的Hover状态发生变化时，才需要重绘
        Invalidate();
    }
    if (!bOnLinkUrl && !toolTipText.empty()) {
        m_pOwner->SetToolTipText(_T(""));
    }
    //更新悬停的链接，并派发Hover/Leave事件
    UpdateHoveredLink(HitTestLinkGroup(ptMouse));

    //拖拽选择文本
    if (m_bSelecting) {
        const size_t nPos = HitTestTextPos(ptMouse);
        if (nPos != m_nSelectCaret) {
            m_nSelectCaret = nPos;
            Invalidate();
        }
    }
}

void RichTextImpl::MouseHover(const EventArgs& msg)
{
    bool hasHover = false;
    //GetRect()返回的是布局坐标（不包含祖先滚动容器的滚动偏移），需要将鼠标坐标转换为布局坐标
    UiPoint ptMouse = msg.ptMouse;
    ptMouse.Offset(m_pOwner->GetScrollOffsetInScrollBox());
    for (const RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            continue;
        }
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(ptMouse)) {
                //超级链接, 显示ToolTip（优先使用title属性，未设置时使用URL）
                if (!textData.m_linkTitle.empty()) {
                    m_pOwner->SetToolTipText(textData.m_linkTitle.c_str());
                }
                else {
                    m_pOwner->SetToolTipText(textData.m_linkUrl.c_str());
                }
                hasHover = true;
            }
        }        
    }
    if (!hasHover) {
        m_pOwner->SetToolTipText(_T(""));
    }
}

void RichTextImpl::MouseLeave(const EventArgs& /*msg*/)
{
    bool bInvalidate = false;
    for (RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            continue;
        }
        if (textData.m_bMouseHover) {
            textData.m_bMouseHover = false;
            bInvalidate = true;
        }
        if (textData.m_bMouseDown) {
            textData.m_bMouseDown = false;
            bInvalidate = true;
        }
    }
    if (bInvalidate) {
        Invalidate();
    }
    //鼠标离开控件：派发链接的Leave事件
    UpdateHoveredLink(-1);
}

bool RichTextImpl::OnSetCursor(const EventArgs& msg)
{
    if (m_pOwner->IsEnabled() && m_pOwner->IsVisible()) {
        //GetRect()返回的是布局坐标（不包含祖先滚动容器的滚动偏移），需要将鼠标坐标转换为布局坐标
        UiPoint ptMouse = msg.ptMouse;
        ptMouse.Offset(m_pOwner->GetScrollOffsetInScrollBox());
        for (const RichTextDataEx& textData : m_textData) {
            if (textData.m_linkUrl.empty()) {
                continue;
            }
            for (const UiRect& textRect : textData.m_textRects) {
                if (textRect.ContainsPt(ptMouse)) {
                    //超级链接，光标变成手型
                    m_pOwner->SetCursor(CursorType::kCursorHand);
                    return true;
                }
            }
        }
    }
    return false;
}

void RichTextImpl::GetLinkGroups(std::vector<size_t>& groupStartIndexes, std::vector<DString>& groupUrls) const
{
    groupStartIndexes.clear();
    groupUrls.clear();
    DString lastLinkUrl;
    for (size_t nIndex = 0; nIndex < m_textData.size(); ++nIndex) {
        const RichTextDataEx& textData = m_textData[nIndex];
        if (textData.m_linkUrl.empty()) {
            lastLinkUrl.clear();
            continue;
        }
        if (textData.m_linkUrl != lastLinkUrl) {
            //连续出现并且URL相同的文本片段，属于同一个链接
            groupStartIndexes.push_back(nIndex);
            groupUrls.push_back(textData.m_linkUrl.c_str());
            lastLinkUrl = textData.m_linkUrl.c_str();
        }
    }
}

int32_t RichTextImpl::HitTestLinkGroup(const UiPoint& ptMouse) const
{
    //直接遍历文本片段累计链接分组序号，避免每次鼠标移动都构建分组数组和复制URL
    int32_t nGroup = -1;
    const UiString* pLastLinkUrl = nullptr;
    for (const RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            pLastLinkUrl = nullptr;
            continue;
        }
        if ((pLastLinkUrl == nullptr) || (*pLastLinkUrl != textData.m_linkUrl)) {
            //连续出现并且URL相同的文本片段，属于同一个链接
            ++nGroup;
            pLastLinkUrl = &textData.m_linkUrl;
        }
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(ptMouse)) {
                return nGroup;
            }
        }
    }
    return -1;
}

DString RichTextImpl::GetLinkGroupUrl(int32_t nLinkGroupIndex) const
{
    if (nLinkGroupIndex < 0) {
        return DString();
    }
    int32_t nGroup = -1;
    const UiString* pLastLinkUrl = nullptr;
    for (const RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            pLastLinkUrl = nullptr;
            continue;
        }
        if ((pLastLinkUrl == nullptr) || (*pLastLinkUrl != textData.m_linkUrl)) {
            ++nGroup;
            pLastLinkUrl = &textData.m_linkUrl;
            if (nGroup == nLinkGroupIndex) {
                return DString(textData.m_linkUrl.c_str());
            }
            if (nGroup > nLinkGroupIndex) {
                break;
            }
        }
    }
    return DString();
}

void RichTextImpl::UpdateHoveredLink(int32_t nLinkGroupIndex)
{
    if (m_nHoveredLinkGroup == nLinkGroupIndex) {
        return;
    }
    const int32_t nOldGroupIndex = m_nHoveredLinkGroup;
    //先取出新旧链接地址，再更新状态，保证事件参数在派发期间有效
    DString oldLinkUrl;
    if (nOldGroupIndex >= 0) {
        oldLinkUrl = GetLinkGroupUrl(nOldGroupIndex);
    }
    DString newLinkUrl;
    if (nLinkGroupIndex >= 0) {
        newLinkUrl = GetLinkGroupUrl(nLinkGroupIndex);
    }
    m_nHoveredLinkGroup = nLinkGroupIndex;
    if (!oldLinkUrl.empty()) {
        //鼠标离开链接
        m_pOwner->SendEvent(kEventLinkLeave, (WPARAM)oldLinkUrl.c_str());
    }
    if (!newLinkUrl.empty()) {
        //鼠标进入链接
        m_pOwner->SendEvent(kEventLinkHover, (WPARAM)newLinkUrl.c_str());
    }
}

void RichTextImpl::SetLinkColor(const DString& normalColor, const DString& hoveredColor,
                                const DString& pressedColor, const DString& visitedColor)
{
    bool bChanged = false;
    if (!normalColor.empty() && (m_linkNormalTextColor != normalColor)) {
        m_linkNormalTextColor = normalColor;
        bChanged = true;
    }
    if (!hoveredColor.empty() && (m_linkHoveredTextColor != hoveredColor)) {
        m_linkHoveredTextColor = hoveredColor;
        bChanged = true;
    }
    if (!pressedColor.empty() && (m_linkPressedTextColor != pressedColor)) {
        m_linkPressedTextColor = pressedColor;
        bChanged = true;
    }
    if (!visitedColor.empty() && (m_linkVisitedTextColor != visitedColor)) {
        m_linkVisitedTextColor = visitedColor;
        bChanged = true;
    }
    if (bChanged) {
        Redraw();
    }
}

size_t RichTextImpl::GetLinkCount() const
{
    std::vector<size_t> groupStartIndexes;
    std::vector<DString> groupUrls;
    GetLinkGroups(groupStartIndexes, groupUrls);
    return groupUrls.size();
}

DString RichTextImpl::GetLinkUrl(size_t nLinkIndex) const
{
    std::vector<size_t> groupStartIndexes;
    std::vector<DString> groupUrls;
    GetLinkGroups(groupStartIndexes, groupUrls);
    if (nLinkIndex < groupUrls.size()) {
        return groupUrls[nLinkIndex];
    }
    return DString();
}

bool RichTextImpl::GetLinkRect(size_t nLinkIndex, UiRect& rcLink) const
{
    rcLink.Clear();
    std::vector<size_t> groupStartIndexes;
    std::vector<DString> groupUrls;
    GetLinkGroups(groupStartIndexes, groupUrls);
    if (nLinkIndex >= groupStartIndexes.size()) {
        return false;
    }
    const DString linkUrl = groupUrls[nLinkIndex];
    bool bFound = false;
    for (size_t nIndex = groupStartIndexes[nLinkIndex]; nIndex < m_textData.size(); ++nIndex) {
        const RichTextDataEx& textData = m_textData[nIndex];
        if (textData.m_linkUrl.empty() || (textData.m_linkUrl != linkUrl)) {
            break;
        }
        for (const UiRect& textRect : textData.m_textRects) {
            if (!bFound) {
                rcLink = textRect;
                bFound = true;
            }
            else {
                rcLink.left = std::min(rcLink.left, textRect.left);
                rcLink.top = std::min(rcLink.top, textRect.top);
                rcLink.right = std::max(rcLink.right, textRect.right);
                rcLink.bottom = std::max(rcLink.bottom, textRect.bottom);
            }
        }
    }
    return bFound;
}

DString RichTextImpl::GetLinkTitle(size_t nLinkIndex) const
{
    std::vector<size_t> groupStartIndexes;
    std::vector<DString> groupUrls;
    GetLinkGroups(groupStartIndexes, groupUrls);
    if (nLinkIndex >= groupStartIndexes.size()) {
        return DString();
    }
    //取该链接分组中第一个文本片段的提示文本
    const RichTextDataEx& textData = m_textData[groupStartIndexes[nLinkIndex]];
    if (!textData.m_linkTitle.empty()) {
        return textData.m_linkTitle.c_str();
    }
    return groupUrls[nLinkIndex];
}

void RichTextImpl::SetEnableTextSelect(bool bEnable)
{
    if (m_bEnableTextSelect != bEnable) {
        m_bEnableTextSelect = bEnable;
        if (!bEnable) {
            ClearSelection();
        }
    }
}

bool RichTextImpl::HasSelection() const
{
    return m_nSelectAnchor != m_nSelectCaret;
}

void RichTextImpl::GetSelectionRange(size_t& nBegin, size_t& nEnd) const
{
    if (m_nSelectAnchor <= m_nSelectCaret) {
        nBegin = m_nSelectAnchor;
        nEnd = m_nSelectCaret;
    }
    else {
        nBegin = m_nSelectCaret;
        nEnd = m_nSelectAnchor;
    }
}

DString RichTextImpl::GetSelectedText() const
{
    size_t nBegin = 0;
    size_t nEnd = 0;
    GetSelectionRange(nBegin, nEnd);
    if (nBegin >= nEnd) {
        return DString();
    }
    BuildSelectText();
    if (nBegin >= m_selectText.size()) {
        return DString();
    }
    if (nEnd > m_selectText.size()) {
        nEnd = m_selectText.size();
    }
    //去除回车符，换行符统一转换为"\r\n"（Windows剪贴板格式）
    std::wstring selectedText;
    selectedText.reserve(nEnd - nBegin + 8);
    for (size_t nIndex = nBegin; nIndex < nEnd; ++nIndex) {
        const wchar_t ch = m_selectText[nIndex];
        if (ch == L'\r') {
            continue;
        }
        if (ch == L'\n') {
            selectedText += L"\r\n";
            continue;
        }
        selectedText += ch;
    }
    return StringConvert::WStringToT(selectedText);
}

bool RichTextImpl::CopySelectedText()
{
    const DString selectedText = GetSelectedText();
    if (selectedText.empty()) {
        return false;
    }
    return Clipboard::SetClipboardText(StringConvert::TToWString(selectedText));
}

void RichTextImpl::SelectAll()
{
    if (!m_bEnableTextSelect) {
        return;
    }
    //确保文本已经解析，才能获取文本内容
    CheckParseText();
    BuildSelectText();
    if (m_selectText.empty()) {
        ClearSelection();
        return;
    }
    m_bSelecting = false;
    if ((m_nSelectAnchor == 0) && (m_nSelectCaret == m_selectText.size())) {
        return;
    }
    m_nSelectAnchor = 0;
    m_nSelectCaret = m_selectText.size();
    Invalidate();
}

void RichTextImpl::ClearSelection()
{
    m_bSelecting = false;
    if ((m_nSelectAnchor == 0) && (m_nSelectCaret == 0)) {
        return;
    }
    m_nSelectAnchor = 0;
    m_nSelectCaret = 0;
    Invalidate();
}

void RichTextImpl::BuildSelectText() const
{
    m_selectText.clear();
    for (const RichTextDataEx& textData : m_textData) {
        if (textData.m_textView.empty()) {
            continue;
        }
        m_selectText.append(textData.m_textView.data(), textData.m_textView.size());
    }
}

void RichTextImpl::ResetTextSelect()
{
    m_nSelectAnchor = 0;
    m_nSelectCaret = 0;
    m_bSelecting = false;
    InvalidateSelectLayout();
}

void RichTextImpl::InvalidateSelectLayout()
{
    m_selectRows.clear();
    m_selectText.clear();
    m_bSelectLayoutValid = false;
    m_selectLayoutRect.Clear();
}

UiRect RichTextImpl::GetTextDrawRect(IRender* pRender)
{
    UiRect rc = m_pOwner->GetRect();
    rc.Deflate(m_pOwner->GetControlPadding());
    rc.Deflate(GetTextPadding());
    if (rc.IsEmpty() || (pRender == nullptr)) {
        return rc;
    }

    //如果设置了对齐方式，需要评估绘制位置
    const uint32_t uTextStyle = GetTextStyle();
    if ((uTextStyle & (TEXT_VCENTER | TEXT_BOTTOM))) {
        //计算绘制所占的区域大小
        UiRect rect;
        CalcDestRect(pRender, rc, rect);
        if (rect.Height() < rc.Height()) {
            //垂直方向
            const int32_t diff = rc.Height() - rect.Height();
            if (uTextStyle & TEXT_VCENTER) {
                rc.Offset(0, diff / 2);
            }
            else if (uTextStyle & TEXT_BOTTOM) {
                rc.Offset(0, diff);
            }
        }
    }
    return rc;
}

bool RichTextImpl::UpdateTextSelectLayout(IRender* pRender)
{
    if (pRender == nullptr) {
        return false;
    }
    //确保文本已经解析
    CheckParseText();
    const UiRect rcText = GetTextDrawRect(pRender);
    if (rcText.IsEmpty()) {
        InvalidateSelectLayout();
        return false;
    }
    if (m_bSelectLayoutValid &&
        (m_selectLayoutRect.Width() == rcText.Width()) &&
        (m_selectLayoutRect.Height() == rcText.Height())) {
        //文本区域的大小未变化，只需更新位置（控件移动后坐标系原点会变化）
        m_selectLayoutRect = rcText;
        return !m_selectRows.empty();
    }
    InvalidateSelectLayout();
    BuildSelectText();
    m_bSelectLayoutValid = true;
    m_selectLayoutRect = rcText;
    if (m_selectText.empty() || m_textData.empty()) {
        return false;
    }

    //为每个物理行预分配数据结构（文本中的换行符及<hr>都会产生新的物理行）
    uint32_t nLineCount = 1;
    for (const wchar_t ch : m_selectText) {
        if (ch == L'\n') {
            ++nLineCount;
        }
    }
    for (const RichTextDataEx& textData : m_textData) {
        if (textData.m_bHorizontalRule) {
            nLineCount += 2;
        }
    }
    nLineCount += 2; //预留余量，避免行数估算不足
    RichTextLineInfoList lineInfoList;
    lineInfoList.reserve(nLineCount);
    for (uint32_t nLine = 0; nLine < nLineCount; ++nLine) {
        lineInfoList.emplace_back(RichTextLineInfoPtr(new RichTextLineInfo));
    }
    RichTextLineInfoParam lineInfoParam;
    lineInfoParam.m_pLineInfoList = &lineInfoList;

    //测量用的数据（与绘制时保持一致，颜色不影响排版结果）
    std::vector<RichTextData> measureTextData;
    measureTextData.reserve(m_textData.size());
    for (const RichTextDataEx& textData : m_textData) {
        measureTextData.push_back(textData);
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    pRender->MeasureRichText2(rcText, UiSize(), pRenderFactory, measureTextData, &lineInfoParam, nullptr);

    //逐行构建字符的几何信息（行内坐标是相对文本区域左上角的坐标）
    const int32_t nAvailableWidth = rcText.Width();
    const uint32_t uTextStyle = GetTextStyle();
    size_t nCharOffset = 0;
    m_selectRows.reserve(lineInfoList.size() * 2);
    for (const RichTextLineInfoPtr& lineInfoPtr : lineInfoList) {
        if (lineInfoPtr == nullptr) {
            continue;
        }
        for (const RichTextRowInfoPtr& rowInfoPtr : lineInfoPtr->m_rowInfo) {
            if ((rowInfoPtr == nullptr) || rowInfoPtr->m_charInfo.empty()) {
                continue;
            }
            const RichTextRowInfo& rowInfo = *rowInfoPtr;
            TextSelectRow selectRow;
            selectRow.m_fLeft = rowInfo.m_rowRect.left;
            selectRow.m_fRight = rowInfo.m_rowRect.right;
            selectRow.m_nTop = (int32_t)rowInfo.m_rowRect.top;
            selectRow.m_nBottom = (int32_t)rowInfo.m_rowRect.bottom;
            selectRow.m_nFirstChar = nCharOffset;

            //逐行水平对齐：测量结果中的坐标未包含对齐产生的偏移
            const int32_t nRowWidth = (int32_t)rowInfo.m_rowRect.Width();
            int32_t nXOffset = 0;
            if ((nRowWidth > 0) && (nRowWidth < nAvailableWidth)) {
                if (uTextStyle & DrawStringFormat::TEXT_HCENTER) {
                    nXOffset = (nAvailableWidth - nRowWidth) / 2;
                }
                else if (uTextStyle & DrawStringFormat::TEXT_RIGHT) {
                    nXOffset = nAvailableWidth - nRowWidth;
                }
            }
            selectRow.m_fLeft += (float)nXOffset;
            selectRow.m_fRight += (float)nXOffset;

            selectRow.m_charWidths.reserve(rowInfo.m_charInfo.size());
            for (const RichTextCharInfo& charInfo : rowInfo.m_charInfo) {
                selectRow.m_charWidths.push_back(charInfo.CharWidth());
            }
            nCharOffset += rowInfo.m_charInfo.size();
            m_selectRows.push_back(std::move(selectRow));
        }
    }
    return !m_selectRows.empty();
}

size_t RichTextImpl::HitTestTextPos(const UiPoint& ptMouse) const
{
    if (m_selectRows.empty()) {
        return 0;
    }
    //转换为相对文本区域的坐标
    const int32_t nX = ptMouse.x - m_selectLayoutRect.left;
    const int32_t nY = ptMouse.y - m_selectLayoutRect.top;

    for (const TextSelectRow& selectRow : m_selectRows) {
        if (nY < selectRow.m_nTop) {
            //在第一行内文本的上方
            return selectRow.m_nFirstChar;
        }
        if ((nY >= selectRow.m_nTop) && (nY < selectRow.m_nBottom)) {
            //在本行内：按字符宽度查找插入位置
            float fX = selectRow.m_fLeft;
            const size_t nCharCount = selectRow.m_charWidths.size();
            for (size_t nIndex = 0; nIndex < nCharCount; ++nIndex) {
                const float fCharWidth = selectRow.m_charWidths[nIndex];
                if (nX < (fX + fCharWidth * 0.5f)) {
                    return selectRow.m_nFirstChar + nIndex;
                }
                fX += fCharWidth;
            }
            return selectRow.m_nFirstChar + nCharCount;
        }
    }
    //在最后一行文本的下方
    const TextSelectRow& lastRow = m_selectRows.back();
    return lastRow.m_nFirstChar + lastRow.m_charWidths.size();
}

void RichTextImpl::GetSelectionRects(std::vector<UiRect>& selectionRects) const
{
    selectionRects.clear();
    size_t nBegin = 0;
    size_t nEnd = 0;
    GetSelectionRange(nBegin, nEnd);
    if ((nBegin >= nEnd) || m_selectRows.empty()) {
        return;
    }
    for (const TextSelectRow& selectRow : m_selectRows) {
        const size_t nRowCharCount = selectRow.m_charWidths.size();
        if (nRowCharCount == 0) {
            continue;
        }
        const size_t nRowBegin = selectRow.m_nFirstChar;
        const size_t nRowEnd = nRowBegin + nRowCharCount;
        if ((nEnd <= nRowBegin) || (nBegin >= nRowEnd)) {
            continue;
        }
        const size_t nStart = (nBegin > nRowBegin) ? (nBegin - nRowBegin) : 0;
        const size_t nStop = (nEnd < nRowEnd) ? (nEnd - nRowBegin) : nRowCharCount;
        float fLeft = selectRow.m_fLeft;
        for (size_t nIndex = 0; nIndex < nStart; ++nIndex) {
            fLeft += selectRow.m_charWidths[nIndex];
        }
        float fRight = fLeft;
        for (size_t nIndex = nStart; nIndex < nStop; ++nIndex) {
            fRight += selectRow.m_charWidths[nIndex];
        }
        if (fRight <= fLeft) {
            //仅选中了换行符等零宽字符
            continue;
        }
        UiRect rcSelect;
        rcSelect.left = (int32_t)fLeft;
        rcSelect.top = selectRow.m_nTop;
        rcSelect.right = (int32_t)CEILF(fRight);
        rcSelect.bottom = selectRow.m_nBottom;
        selectionRects.push_back(rcSelect);
    }
}

bool RichTextImpl::AppendText(const DString& richText, bool bRedraw)
{
    bool bResult = true;
    if (!richText.empty()) {
        DString trimmedText = richText;
        StringUtil::Trim(trimmedText);
        if (trimmedText.find(_T("<RichText")) == 0) {
            bResult = WindowBuilder::ParseRichTextXmlText(trimmedText, m_pOwner);
        }
        else {
            bResult = WindowBuilder::ParseRichTextXmlText(_T("<RichText>") + richText + _T("</RichText>"), m_pOwner);
        }
        if (!bResult) {
            //追加的内容解析失败时，按纯文本追加
            bResult = WindowBuilder::ParseRichTextXmlText(_T("<RichText>") + EscapeXmlText(richText) + _T("</RichText>"), m_pOwner);
        }
    }
    //文本数据需要重新解析
    m_textData.clear();
    m_spDrawRichTextCache.reset();
    m_bCachedTextRectValid = false;
    ++m_nLayoutVersion;
    m_nHoveredLinkGroup = -1;
    ResetTextSelect();
    if (bRedraw) {
        Redraw();
    }
    return bResult;
}

bool RichTextImpl::OnKeyDown(const EventArgs& msg)
{
    if (!m_pOwner->IsFocused()) {
        return false;
    }
    //文本选择的快捷键：Ctrl+A（全选）、Ctrl+C（复制）
    if (m_bEnableTextSelect && ((msg.modifierKey & kControl) != 0)) {
        if (msg.vkCode == kVK_A) {
            SelectAll();
            return true;
        }
        if (msg.vkCode == kVK_C) {
            return CopySelectedText();
        }
    }

    std::vector<size_t> groupStartIndexes;
    std::vector<DString> groupUrls;
    GetLinkGroups(groupStartIndexes, groupUrls);
    if (groupUrls.empty()) {
        return false;
    }
    const int32_t nGroupCount = (int32_t)groupUrls.size();
    if (msg.vkCode == kVK_TAB) {
        //Tab/Shift+Tab：在链接之间切换键盘焦点
        const bool bShiftKey = ((msg.modifierKey & kShift) != 0);
        int32_t nGroupIndex = m_nFocusedLinkGroup;
        if (bShiftKey) {
            nGroupIndex = (nGroupIndex <= 0) ? (nGroupCount - 1) : (nGroupIndex - 1);
        }
        else {
            nGroupIndex = ((nGroupIndex < 0) || (nGroupIndex >= (nGroupCount - 1))) ? 0 : (nGroupIndex + 1);
        }
        if (nGroupIndex != m_nFocusedLinkGroup) {
            m_nFocusedLinkGroup = nGroupIndex;
            Invalidate();
        }
        return true;
    }
    if ((msg.vkCode == kVK_RETURN) && (m_nFocusedLinkGroup >= 0) && (m_nFocusedLinkGroup < nGroupCount)) {
        //Enter：触发当前聚焦的链接
        DString linkUrl = groupUrls[(size_t)m_nFocusedLinkGroup];
        m_visitedLinks.insert(linkUrl);
        Invalidate();
        m_pOwner->SendEvent(kEventLinkClick, (WPARAM)linkUrl.c_str());
        return true;
    }
    return false;
}

void RichTextImpl::RButtonUp(const EventArgs& msg)
{
    //鼠标右键：复制链接地址到剪贴板
    //GetRect()返回的是布局坐标（不包含祖先滚动容器的滚动偏移），需要将鼠标坐标转换为布局坐标
    UiPoint ptMouse = msg.ptMouse;
    ptMouse.Offset(m_pOwner->GetScrollOffsetInScrollBox());
    const int32_t nGroupIndex = HitTestLinkGroup(ptMouse);
    if (nGroupIndex < 0) {
        return;
    }
    std::vector<size_t> groupStartIndexes;
    std::vector<DString> groupUrls;
    GetLinkGroups(groupStartIndexes, groupUrls);
    if ((size_t)nGroupIndex < groupUrls.size()) {
        Clipboard::SetClipboardText(StringConvert::TToWString(groupUrls[(size_t)nGroupIndex]));
    }
}

} // namespace ui
