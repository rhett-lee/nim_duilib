#include "RichText.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/Window.h"

namespace ui 
{

RichText::RichText(Window* pWindow) :
    Control(pWindow),
    m_hAlignType(HorAlignType::kHorAlignLeft),
    m_vAlignType(VerAlignType::kVerAlignTop),
    m_fRowSpacingMul(1.0f),
    m_bLinkUnderlineFont(true),
    m_nTextDataDPI(0),
    m_bWordWrap(true)
{
}

RichText::~RichText()
{
    m_spDrawRichTextCache.reset();
}

DString RichText::GetType() const { return DUI_CTR_RICHTEXT; }

void RichText::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("text_align")) {
        if (strValue.find(_T("left")) != DString::npos) {
            SetHAlignType(HorAlignType::kHorAlignLeft);
        }
        else if (strValue.find(_T("hcenter")) != DString::npos) {
            SetHAlignType(HorAlignType::kHorAlignCenter);
        }
        else if (strValue.find(_T("right")) != DString::npos) {
            SetHAlignType(HorAlignType::kHorAlignRight);
        }

        if (strValue.find(_T("top")) != DString::npos) {
            SetVAlignType(VerAlignType::kVerAlignTop);
        }
        else if (strValue.find(_T("vcenter")) != DString::npos) {
            SetVAlignType(VerAlignType::kVerAlignCenter);
        }
        else if (strValue.find(_T("bottom")) != DString::npos) {
            SetVAlignType(VerAlignType::kVerAlignBottom);
        }
        m_textData.clear();
        m_spDrawRichTextCache.reset();
    }    
    else if (strName == _T("font")) {
        SetFontId(strValue);
    }
    else if (strName == _T("text_color")) {
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
    else if (strName == _T("default_link_font_color")) {
        //超级链接：常规文本颜色值
        m_linkNormalTextColor = strValue;
    }
    else if (strName == _T("hover_link_font_color")) {
        //超级链接：Hover状态文本颜色值
        m_linkHoverTextColor = strValue;
    }
    else if (strName == _T("mouse_down_link_font_color")) {
        //超级链接：鼠标按下状态文本颜色值
        m_linkMouseDownTextColor = strValue;
    }
    else if (strName == _T("link_font_underline")) {
        //超级链接：是否使用带下划线的字体
        m_bLinkUnderlineFont = (strValue == _T("true"));
    }
    else if (strName == _T("text")) {
        //允许使用'{'代替'<'，'}'代替'>'
        if (((strValue.find(_T('<')) == DString::npos) && (strValue.find(_T('>')) == DString::npos)) &&
            ((strValue.find(_T('{')) != DString::npos) && (strValue.find(_T('}')) != DString::npos))) {
            DString richText(strValue);
            StringUtil::ReplaceAll(_T("{"), _T("<"), richText);
            StringUtil::ReplaceAll(_T("}"), _T(">"), richText);
            SetText(richText);
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
        SetWordWrap(strValue == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void RichText::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    UiPadding rcTextPadding = GetTextPadding();
    rcTextPadding = Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    SetTextPadding(rcTextPadding, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void RichText::Redraw()
{
    //重新绘制
    m_textData.clear();
    m_spDrawRichTextCache.reset();
    Invalidate();
}

uint16_t RichText::GetTextStyle() const
{
    uint32_t uTextStyle = 0;
    if (m_hAlignType == HorAlignType::kHorAlignCenter) {
        uTextStyle |= TEXT_CENTER;
    }
    else if (m_hAlignType == HorAlignType::kHorAlignRight) {
        uTextStyle |= TEXT_RIGHT;
    }
    else {
        uTextStyle |= TEXT_LEFT;
    }

    if (m_vAlignType == VerAlignType::kVerAlignCenter) {
        uTextStyle |= TEXT_VCENTER;
    }
    else if (m_vAlignType == VerAlignType::kVerAlignBottom) {
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

void RichText::CalcDestRect(IRender* pRender, const UiRect& rc, UiRect& rect)
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

UiSize RichText::EstimateText(UiSize szAvailable)
{
    UiSize fixedSize;
    IRender* pRender = nullptr;
    if (GetWindow() != nullptr) {
        pRender = GetWindow()->GetRender();
    }
    if (pRender == nullptr) {
        return fixedSize;
    }

    int32_t nWidth = szAvailable.cx;
    if (GetFixedWidth().IsStretch()) {
        //如果是拉伸类型，使用外部宽度
        nWidth = CalcStretchValue(GetFixedWidth(), szAvailable.cx);
    }
    else if (GetFixedWidth().IsInt32()) {
        nWidth = GetFixedWidth().GetInt32();
    }
    else if (GetFixedWidth().IsAuto()) {
        //宽度为自动时，不限制宽度
        nWidth = INT_MAX;
    }

    //最大高度，不限制
    int32_t nHeight = INT_MAX;

    UiRect rc;
    rc.left = 0;
    rc.right = rc.left + nWidth;
    rc.top = 0;
    rc.bottom = rc.top + nHeight;

    const UiPadding rcTextPadding = GetTextPadding();
    const UiPadding rcPadding = GetControlPadding();
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

void RichText::PaintText(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    rc.Deflate(GetTextPadding());

    //检查并更新文本
    CheckParseText();

    //如果设置了对齐方式，需要评估绘制位置
    const uint32_t uTextStyle = GetTextStyle();
    if ((uTextStyle & (TEXT_CENTER | TEXT_RIGHT | TEXT_VCENTER | TEXT_BOTTOM))) {
        //计算绘制所占的区域大小
        UiRect rect;
        CalcDestRect(pRender, rc, rect);
        if ((rect.Width() < rc.Width()) && (uTextStyle & (TEXT_CENTER | TEXT_RIGHT))) {
            //水平方向
            int32_t diff = rc.Width() - rect.Width();
            if (uTextStyle & TEXT_CENTER) {
                rc.Offset(diff / 2, 0);
            }
            else if (uTextStyle & TEXT_RIGHT) {
                rc.Offset(diff, 0);
            }
        }
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
            normalLinkTextColor = GetUiColor(m_linkNormalTextColor.c_str());
        }
        UiColor mouseDownLinkTextColor;
        if (!m_linkMouseDownTextColor.empty()) {
            mouseDownLinkTextColor = GetUiColor(m_linkMouseDownTextColor.c_str());
        }
        UiColor linkHoverTextColor;
        if (!m_linkHoverTextColor.empty()) {
            linkHoverTextColor = GetUiColor(m_linkHoverTextColor.c_str());
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
                if (textDataEx.m_bMouseDown || textDataEx.m_bMouseHover) {
                    textData.m_pFontInfo->m_bUnderline = m_bLinkUnderlineFont;//是否显示下划线字体
                }
                if (!m_linkNormalTextColor.empty()) {                    
                    if (!normalLinkTextColor.IsEmpty()) {
                        textData.m_textColor = normalLinkTextColor;//标准状态的字体颜色
                    }
                }
                if (textDataEx.m_bMouseDown && !m_linkMouseDownTextColor.empty()) {                    
                    if (!mouseDownLinkTextColor.IsEmpty()) {
                        textData.m_textColor = mouseDownLinkTextColor;//鼠标按下时的字体颜色
                    }
                }
                else if (textDataEx.m_bMouseHover && !m_linkHoverTextColor.empty()) {                    
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
            pRender->DrawRichTextCacheData(m_spDrawRichTextCache, textRect, szScrollOffset, rowXOffset, (uint8_t)GetAlpha(), &richTextRects);

#if 0
            //测试代码结束(比较使用缓存绘制和不使用缓存绘制时结果是否相同)
            std::vector<std::vector<UiRect>> richTextRectsOld;
            pRender->DrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, (uint8_t)GetAlpha(), &richTextRectsOld);
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
            pRender->DrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, (uint8_t)GetAlpha(), &richTextRects);
        }
        ASSERT(richTextRects.size() == m_textData.size());
        if (richTextRects.size() == m_textData.size()) {
            for (size_t index = 0; index < m_textData.size(); ++index) {
                m_textData[index].m_textRects.swap(richTextRects[index]);
            }
        }
    }
}

void RichText::CheckParseText()
{
    if (!m_richTextId.empty() && (m_langFileName != GlobalManager::Instance().GetLanguageFileName())) {
        //多语言版：当语言发生变化时，更新文本内容
        DoSetText(GlobalManager::Instance().Lang().GetStringViaID(m_richTextId.c_str()));
        m_langFileName = GlobalManager::Instance().GetLanguageFileName();
    }

    //当DPI变化时，需要重新解析文本，更新字体大小
    if (m_nTextDataDPI != Dpi().GetDPI()) {
        m_textData.clear();
        m_spDrawRichTextCache.reset();
    }

    if (m_textData.empty()) {
        ParseText(m_textData);
        m_nTextDataDPI = Dpi().GetDPI();
        m_spDrawRichTextCache.reset();
    }
}

bool RichText::ParseText(std::vector<RichTextDataEx>& outTextData) const
{
    //默认字体
    DString sFontId = GetFontId();
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(sFontId, Dpi());
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return false;
    }

    RichTextDataEx parentTextData;
    //默认文本颜色
    parentTextData.m_textColor = GetUiColor(GetTextColor());
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
    parentTextData.m_textStyle = GetTextStyle();

    std::vector<RichTextDataEx> textData;

    for (const RichTextSlice& textSlice : m_textSlice) {
        if (!ParseTextSlice(textSlice, parentTextData, textData)) {
            return false;
        }
    }
    outTextData.swap(textData);
    return true;
}

bool RichText::ParseTextSlice(const RichTextSlice& textSlice, 
                              const RichTextDataEx& parentTextData, 
                              std::vector<RichTextDataEx>& textData) const
{
    //当前节点
    RichTextDataEx currentTextData;
    currentTextData.m_fRowSpacingMul = parentTextData.m_fRowSpacingMul;
    currentTextData.m_textStyle = parentTextData.m_textStyle;
    currentTextData.m_textView = std::wstring_view(textSlice.m_text.c_str(), textSlice.m_text.size());

    //不应包含回车和换行（由外部调用方负责处理）
    ASSERT(currentTextData.m_textView.find(L"\r") == DStringW::npos);    
    ASSERT(currentTextData.m_textView.find(L"\t") == DStringW::npos);
    if (currentTextData.m_textView.size() != 1) {
        ASSERT(currentTextData.m_textView.find(L"\n") == DStringW::npos);
    }

    currentTextData.m_linkUrl = textSlice.m_linkUrl;
    if (!textSlice.m_textColor.empty()) {
        currentTextData.m_textColor = GetUiColor(textSlice.m_textColor.c_str());
    }
    if (textSlice.m_textColor.empty()) {
        currentTextData.m_textColor = parentTextData.m_textColor;
    }

    if (!textSlice.m_bgColor.empty()) {
        currentTextData.m_bgColor = GetUiColor(textSlice.m_bgColor.c_str());
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
        currentTextData.m_pFontInfo->m_fontSize = Dpi().GetScaleInt(textSlice.m_fontInfo.m_fontSize); //字体大小，需要DPI缩放
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
    if (!currentTextData.m_textView.empty() || !currentTextData.m_linkUrl.empty()) {
        textData.push_back(currentTextData);
    }
        
    if (!textSlice.m_childs.empty()) {
        //子节点
        for (const RichTextSlice& childSlice : textSlice.m_childs) {
            if (!ParseTextSlice(childSlice, currentTextData, textData)) {
                return false;
            }
        }
    }
    return true;
}

UiPadding RichText::GetTextPadding() const
{
    return UiPadding(m_rcTextPadding.left, m_rcTextPadding.top, m_rcTextPadding.right, m_rcTextPadding.bottom);
}

void RichText::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
{
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    if ((padding.left < 0) || (padding.top < 0) ||
        (padding.right < 0) || (padding.bottom < 0)) {
        return;
    }
    if (bNeedDpiScale) {
        Dpi().ScalePadding(padding);
    }
    if (!GetTextPadding().Equals(padding)) {
        m_rcTextPadding.left = TruncateToUInt16(padding.left);
        m_rcTextPadding.top = TruncateToUInt16(padding.top);
        m_rcTextPadding.right = TruncateToUInt16(padding.right);
        m_rcTextPadding.bottom = TruncateToUInt16(padding.bottom);
        RelayoutOrRedraw();
    }
}

const DString& RichText::TrimText(DString& text)
{
    //回车/换行TAB键：替换为1个空格
    DString tempText;
    tempText.swap(text);
    size_t nCount = tempText.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if ((tempText[nIndex] == _T('\r')) || (tempText[nIndex] == _T('\n')) || (tempText[nIndex] == _T('\t'))) {
            tempText[nIndex] = _T(' ');
        }
    }

    if (m_trimPolicy == TrimPolicy::kNone) {
        //不处理
    }
    else if (m_trimPolicy == TrimPolicy::kKeepOne) {
        //只保留一个空格
        if (!tempText.empty()) {
            bool bFirst = (tempText.front() == _T(' '));
            bool bLast = tempText[tempText.size() - 1] == _T(' ');
            StringUtil::Trim(tempText);
            if (tempText.empty()) {
                tempText = DString(_T(" "));
            }
            else {
                if (bFirst) {
                    tempText = DString(_T(" ")) + tempText;
                }
                else if (bLast) {
                    tempText += DString(_T(" "));
                }
            }
        }
    }
    else {
        //去掉左右两侧的空格
        StringUtil::Trim(tempText);
    }
    tempText.swap(text);
    return text;
}

DString RichText::TrimText(const DString::value_type* text)
{
    DString retText;
    if (text != nullptr) {
        retText = text;
    }
    return TrimText(retText);
}

bool RichText::DoSetText(const DString& richText)
{
    Clear();
    //XML解析的内容，全部封装在WindowBuilder这个类中，以避免到处使用XML解析器，从而降低代码维护复杂度
    bool bResult = true;
    if (!richText.empty()) {
        if (richText.find(_T("<RichText")) == DString::npos) {
            DString formatedText = _T("<RichText>") + richText + _T("</RichText>");
            bResult = WindowBuilder::ParseRichTextXmlText(formatedText, this);
        }
        else {
            bResult = WindowBuilder::ParseRichTextXmlText(richText, this);
        }
    }
    return bResult;
}

bool RichText::SetText(const DString& richText)
{
    bool bResult = DoSetText(richText);
    if (bResult) {
        RelayoutOrRedraw();
    }
    return bResult;
}

bool RichText::SetTextId(const DString& richTextId)
{
    bool bRet = SetText(GlobalManager::Instance().Lang().GetStringViaID(richTextId));
    m_richTextId = richTextId;
    if (!m_richTextId.empty()) {
        m_langFileName = GlobalManager::Instance().GetLanguageFileName();
    }
    else {
        m_langFileName.clear();
    }
    return bRet;
}

void RichText::Clear()
{
    m_textData.clear();
    m_spDrawRichTextCache.reset();
    if (!m_textSlice.empty()) {
        m_textSlice.clear();
        Invalidate();
    }
}

DString RichText::GetFontId() const
{
    return m_sFontId.c_str();
}

void RichText::SetFontId(const DString& strFontId)
{
    if (m_sFontId != strFontId) {
        m_sFontId = strFontId;
        Redraw();
    }
}

DString RichText::GetTextColor() const
{
    return m_sTextColor.c_str();
}

void RichText::SetTextColor(const DString& sTextColor)
{
    if (m_sTextColor != sTextColor) {
        m_sTextColor = sTextColor;
        Redraw();
    }
}

float RichText::GetRowSpacingMul() const
{
    return m_fRowSpacingMul;
}

void RichText::SetRowSpacingMul(float fRowSpacingMul)
{
    if (m_fRowSpacingMul != fRowSpacingMul) {
        m_fRowSpacingMul = fRowSpacingMul;
        if (m_fRowSpacingMul <= 0.01f) {
            m_fRowSpacingMul = 1.0f;
        }
        Redraw();
    }
}

bool RichText::IsWordWrap() const
{
    return m_bWordWrap;
}

void RichText::SetWordWrap(bool bWordWrap)
{
    if (m_bWordWrap != bWordWrap) {
        m_bWordWrap = bWordWrap;
        Redraw();
    }
}

void RichText::SetHAlignType(HorAlignType alignType)
{
    if (m_hAlignType != alignType) {
        m_hAlignType = alignType;
        Redraw();
    }
}

HorAlignType RichText::GetHAlignType() const
{
    return m_hAlignType;
}

void RichText::SetVAlignType(VerAlignType alignType)
{
    if (m_vAlignType != alignType) {
        m_vAlignType = alignType;
        Redraw();
    }
}

VerAlignType RichText::GetVAlignType() const
{
    return m_vAlignType;
}


void RichText::AppendTextSlice(const RichTextSlice&& textSlice)
{
    m_textSlice.emplace_back(textSlice);
    m_textData.clear();
    m_spDrawRichTextCache.reset();
}

void RichText::AppendTextSlice(const RichTextSlice& textSlice)
{
    m_textSlice.emplace_back(textSlice);
    m_textData.clear();
    m_spDrawRichTextCache.reset();
}

DString RichText::ToString() const
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

DString RichText::ToString(const RichTextSlice& textSlice, const DString& indent) const
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
            richText += StringConvert::WStringToT(textSlice.m_text);
            richText += lineBreak;
        }
        //没有节点名称的情况下，就没有属性和子节点，直接返回
        return richText;
    }

    //生成属性列表
    DString attrList;    
    if (!textSlice.m_linkUrl.empty()) {
        attrList += _T("href=\"");
        DString url = textSlice.m_linkUrl.c_str();
        attrList += textSlice.m_linkUrl.c_str();
        attrList += _T("\"");
    }
    if (!textSlice.m_bgColor.empty()) {
        attrList += _T("color=\"");
        attrList += textSlice.m_bgColor.c_str();
        attrList += _T("\"");
    }
    if (!textSlice.m_textColor.empty()) {
        attrList += _T("color=\"");
        attrList += textSlice.m_textColor.c_str();
        attrList += _T("\"");
    }
    if (!textSlice.m_fontInfo.m_fontName.empty()) {
        attrList += _T("face=\"");
        attrList += textSlice.m_fontInfo.m_fontName.c_str();
        attrList += _T("\"");
    }
    if (textSlice.m_fontInfo.m_fontSize != 0) {
        attrList += _T("size=\"");
        attrList += StringUtil::Printf(_T("%d"), textSlice.m_fontInfo.m_fontSize);
        attrList += _T("\"");
    }

    if(!textSlice.m_childs.empty()) {
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
        for (const RichTextSlice& childSlice : textSlice.m_childs) {
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
        richText += StringConvert::WStringToT(textSlice.m_text);

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
        richText += _T(" ");
        if (!attrList.empty()) {            
            richText += attrList;
        }
        richText += _T("/>");
        richText += lineBreak;
    }
    return richText;
}

bool RichText::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
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
    return bRet;
}

bool RichText::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
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
                    SendEvent(kEventLinkClick, (WPARAM)url.c_str());
                    return bRet;
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
    return bRet;
}

bool RichText::MouseMove(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseMove(msg);
    bool bOnLinkUrl = false;
    for (RichTextDataEx& textData : m_textData) {
        textData.m_bMouseHover = false;
        if (textData.m_linkUrl.empty()) {
            continue;
        }
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(msg.ptMouse)) {
                //在超级链接上
                textData.m_bMouseHover = true;
                Invalidate();
                if (textData.m_linkUrl == GetToolTipText()) {
                    bOnLinkUrl = true;
                }
            }
        }
    }
    if (!bOnLinkUrl) {
        SetToolTipText(_T(""));
    }
    return bRet;
}

bool RichText::MouseHover(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseHover(msg);
    bool hasHover = false;
    for (const RichTextDataEx& textData : m_textData) {
        if (textData.m_linkUrl.empty()) {
            continue;
        }
        for (const UiRect& textRect : textData.m_textRects) {
            if (textRect.ContainsPt(msg.ptMouse)) {
                //超级链接, 显示ToolTip
                SetToolTipText(textData.m_linkUrl.c_str());
                hasHover = true;
            }
        }        
    }
    if (!hasHover) {
        SetToolTipText(_T(""));
    }
    return bRet;
}

bool RichText::MouseLeave(const EventArgs& msg)
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
    return BaseClass::MouseLeave(msg);
}

bool RichText::OnSetCursor(const EventArgs& msg)
{
    if (IsEnabled() && IsVisible()) {
        for (const RichTextDataEx& textData : m_textData) {
            if (textData.m_linkUrl.empty()) {
                continue;
            }
            for (const UiRect& textRect : textData.m_textRects) {
                if (textRect.ContainsPt(msg.ptMouse)) {
                    //超级链接，光标变成手型
                    SetCursor(CursorType::kCursorHand);
                    return true;
                }
            }
        }
    }
    return BaseClass::OnSetCursor(msg);
}

} // namespace ui
