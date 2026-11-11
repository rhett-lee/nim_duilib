#include "RichTextImpl.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/Window.h"

#include <utility>

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
           (name == _T("font")) || (name == _T("br"));
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
    if (m_hAlignType == HorAlignType::kAlignCenter) {
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
    UiRect rc = m_pOwner->GetRect();
    rc.Deflate(m_pOwner->GetControlPadding());
    rc.Deflate(GetTextPadding());

    //检查并更新文本
    CheckParseText();

    //如果设置了对齐方式，需要评估绘制位置
    //（水平方向的对齐在绘制时按行计算偏移，见DrawRichTextCacheData的逐行对齐处理）
    const uint32_t uTextStyle = GetTextStyle();
    if ((uTextStyle & (TEXT_VCENTER | TEXT_BOTTOM))) {
        //计算绘制所占的区域大小
        UiRect rect;
        CalcDestRect(pRender, rc, rect);
        if ((rect.Height() < rc.Height()) && (uTextStyle & (TEXT_VCENTER | TEXT_BOTTOM))) {
            //垂直方向
            int32_t diff = rc.Height() - rect.Height();
            if (uTextStyle & TEXT_VCENTER) {
                rc.Offset(0, diff / 2);
            }
            else if (uTextStyle & TEXT_BOTTOM) {
                rc.Offset(0, diff);
            }
        }
    }

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

        std::vector<RichTextData> richTextData;
        richTextData.reserve(m_textData.size());
        for (const RichTextDataEx& textDataEx : m_textData) {
            if (!textDataEx.m_linkUrl.empty()) {
                //对于超级链接，设置默认文本格式
                RichTextData textData = textDataEx;
                textData.m_pFontInfo.reset(new UiFontEx);
                ASSERT(textDataEx.m_pFontInfo != nullptr);
                if (textDataEx.m_pFontInfo != nullptr) {
                    ASSERT(!textDataEx.m_pFontInfo->m_fontName.empty());
                    textData.m_pFontInfo->CopyFrom(*textDataEx.m_pFontInfo);
                }                
                if (m_bLinkUnderlineFont) {
                    //是否显示下划线字体（常态与Hover状态保持一致）
                    textData.m_pFontInfo->m_bUnderline = true;
                }
                if (!m_linkNormalTextColor.empty()) {                    
                    if (!normalLinkTextColor.IsEmpty()) {
                        textData.m_textColor = normalLinkTextColor;//标准状态的字体颜色
                    }
                }
                if (textDataEx.m_bMouseDown && !m_linkPressedTextColor.empty()) {                    
                    if (!mouseDownLinkTextColor.IsEmpty()) {
                        textData.m_textColor = mouseDownLinkTextColor;//鼠标按下时的字体颜色
                    }
                }
                else if (textDataEx.m_bMouseHover && !m_linkHoveredTextColor.empty()) {                    
                    if (!linkHoverTextColor.IsEmpty()) {
                        textData.m_textColor = linkHoverTextColor;//鼠标Hover时的字体颜色
                    }
                }
                richTextData.push_back(textData);
            }
            else {
                richTextData.push_back(textDataEx);
            }
        }
        std::vector<std::vector<UiRect>> richTextRects;
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        const UiRect& textRect = rc;
        UiSize szScrollOffset;
        if (m_spDrawRichTextCache != nullptr) {
            //校验缓存，如果失效则重新生成
            if (!pRender->IsValidDrawRichTextCache(textRect, richTextData, m_spDrawRichTextCache)) {
                m_spDrawRichTextCache.reset();
            }
        }
        if (m_spDrawRichTextCache == nullptr) {
            bool bRet = pRender->CreateDrawRichTextCache(textRect, szScrollOffset, pRenderFactory, richTextData, m_spDrawRichTextCache);
            if (!bRet) {
                m_spDrawRichTextCache.reset();
            }
        }
        if (m_spDrawRichTextCache != nullptr) {
            //使用绘制缓存
            std::vector<int32_t> rowXOffset;//对齐方式为居中或者靠右时使用
            pRender->DrawRichTextCacheData(m_spDrawRichTextCache, textRect, szScrollOffset, rowXOffset, m_pOwner->GetAlpha(), &richTextRects);

#if 0
            //测试代码结束(比较使用缓存绘制和不使用缓存绘制时结果是否相同)
            std::vector<std::vector<UiRect>> richTextRectsOld;
            pRender->DrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, GetAlpha(), &richTextRectsOld);
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
            pRender->DrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, m_pOwner->GetAlpha(), &richTextRects);
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
    }

    if (m_textData.empty()) {
        ParseText(m_textData);
        m_nTextDataDisplayScaleFactor = m_pOwner->Dpi().GetDisplayScaleFactor();
        m_spDrawRichTextCache.reset();
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

    //文本中可以包含换行符和TAB（绘制时由DrawRichText负责处理）
    currentTextData.m_linkUrl = textSlice.m_linkUrl;
    if (currentTextData.m_linkUrl.empty()) {
        //如果是链接的子节点，则继承父节点的链接地址
        currentTextData.m_linkUrl = parentTextData.m_linkUrl;
    }
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
    if (!currentTextData.m_textView.empty()) {
        //空文本节点不参与排版（链接的文本由子节点承载）
        textData.push_back(currentTextData);
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
            DString formatedText = _T("<RichText>") + EscapeXmlText(richText) + _T("</RichText>");
            bResult = WindowBuilder::ParseRichTextXmlText(formatedText, m_pOwner);
        }
    }
    return bResult;
}

bool RichTextImpl::SetText(const DString& richText, bool bRedraw)
{
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
    for (size_t index = 0; index < m_textData.size(); ++index) {
        RichTextDataEx& textData = m_textData[index];
        textData.m_bMouseDown = false;
        if (textData.m_linkUrl.empty()) {
            continue;
        }        
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(msg.ptMouse)) {
                //在超级链接上
                textData.m_bMouseDown = true;
                Invalidate();
            }
        }
    }
}

void RichTextImpl::ButtonUp(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return;
    }
    for (size_t index = 0; index < m_textData.size(); ++index) {
        RichTextDataEx& textData = m_textData[index];
        if (textData.m_linkUrl.empty()) {
            continue;
        }
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(msg.ptMouse)) {
                //在超级链接上, 并且与按下鼠标时的相同，则触发点击事件
                if (textData.m_bMouseDown) {
                    textData.m_bMouseDown = false;
                    Invalidate();
                    DString url = textData.m_linkUrl.c_str();
                    m_pOwner->SendEvent(kEventLinkClick, (WPARAM)url.c_str());
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
            if (textRect.ContainsPt(msg.ptMouse)) {
                //在超级链接上
                bMouseHover = true;
                if (textData.m_linkUrl == m_pOwner->GetToolTipText()) {
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
    if (!bOnLinkUrl) {
        m_pOwner->SetToolTipText(_T(""));
    }
}

void RichTextImpl::MouseHover(const EventArgs& msg)
{
    bool hasHover = false;
    for (const RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            continue;
        }
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(msg.ptMouse)) {
                //超级链接, 显示ToolTip
                m_pOwner->SetToolTipText(textData.m_linkUrl.c_str());
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
}

bool RichTextImpl::OnSetCursor(const EventArgs& msg)
{
    if (m_pOwner->IsEnabled() && m_pOwner->IsVisible()) {
        for (const RichTextDataEx& textData : m_textData) {
            if (textData.m_linkUrl.empty()) {
                continue;
            }
            for (const UiRect& textRect : textData.m_textRects) {
                if (textRect.ContainsPt(msg.ptMouse)) {
                    //超级链接，光标变成手型
                    m_pOwner->SetCursor(CursorType::kCursorHand);
                    return true;
                }
            }
        }
    }
    return false;
}

} // namespace ui
