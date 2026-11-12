#include "LabelImpl.h"
#include "TextDrawer.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/StateColorMap.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"

namespace ui
{
LabelImpl::LabelImpl(Control* pOwner):
    m_pOwner(pOwner),
    m_pLabelOwner(dynamic_cast<LabelOwner*>(pOwner)),
    m_sFontId(),
    m_sTextCache(),
    m_pTextColorMap(),
    m_pFont(nullptr),
    m_pExtras(),
    m_uTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_SINGLELINE),
    m_nFontCacheVersion(0),
    m_flags(),
    m_fSpacingMul(1.0f),
    m_fSpacingAdd(0),
    m_fWordSpacing(0),
    m_rcTextPadding()
{
    //布尔标志的默认值（其余标志由m_flags()初始化为false）
    m_flags.m_bSingleLine = true;
    m_flags.m_bUseFontHeight = true;
    m_flags.m_bRotate90ForAscii = true;

    Box* pBox = dynamic_cast<Box*>(pOwner);
    if (pBox != nullptr) {
        pOwner->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
        pOwner->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    }
    else {
        pOwner->SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
        pOwner->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
    }
}

LabelImpl::~LabelImpl()
{
    m_pExtras.reset();
}

bool LabelImpl::OnSetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("text_align")) {
        bool bHCenter = false;        
        size_t centerPos = strValue.find(_T("center"));
        if (centerPos != DString::npos) {
            //"center"这个属性有歧义，保留以保持兼容性，新的属性是"hcenter"
            bHCenter = true;
            size_t vCenterPos = strValue.find(_T("vcenter"));
            if (vCenterPos != DString::npos) {
                if ((vCenterPos + 1) == centerPos) {
                    bHCenter = false;
                }
            }
        }

        //水平对齐方式
        if (strValue.find(_T("hcenter")) != DString::npos) {            
            bHCenter = true;
        }
        if (bHCenter) {
            //水平对齐：居中
            m_uTextStyle &= ~TEXT_HALIGN_ALL;
            m_uTextStyle |= TEXT_HCENTER;
        }
        else if (strValue.find(_T("right")) != DString::npos) {
            //水平对齐：靠右
            m_uTextStyle &= ~TEXT_HALIGN_ALL;
            m_uTextStyle |= TEXT_RIGHT;
        }
        else if (strValue.find(_T("left")) != DString::npos) {
            //水平对齐：靠左
            m_uTextStyle &= ~TEXT_HALIGN_ALL;
            m_uTextStyle |= TEXT_LEFT;
        }
        else if (strValue.find(_T("hjustify")) != DString::npos) {
            //水平对齐：两端对齐
            m_uTextStyle &= ~TEXT_HALIGN_ALL;
            m_uTextStyle |= TEXT_HJUSTIFY;
        }

        //垂直对齐方式
        if (strValue.find(_T("top")) != DString::npos) {
            //垂直对齐：靠上
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_TOP;
        }
        else if (strValue.find(_T("vcenter")) != DString::npos) {
            //垂直对齐：居中
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_VCENTER;
        }
        else if (strValue.find(_T("bottom")) != DString::npos) {
            //垂直对齐：靠下
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_BOTTOM;
        }
        else if (strValue.find(_T("vjustify")) != DString::npos) {
            //垂直对齐：两端对齐
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_VJUSTIFY;
        }
    }
    else if ((strName == _T("end_ellipsis")) || (strName == _T("endellipsis"))) {
        if (StringUtil::IsValueTrue(strValue)) {
            m_uTextStyle |= TEXT_END_ELLIPSIS;
        }
        else {
            m_uTextStyle &= ~TEXT_END_ELLIPSIS;
        }
    }
    else if ((strName == _T("path_ellipsis")) || (strName == _T("pathellipsis"))) {
        if (StringUtil::IsValueTrue(strValue)) {
            m_uTextStyle |= TEXT_PATH_ELLIPSIS;
        }
        else {
            m_uTextStyle &= ~TEXT_PATH_ELLIPSIS;
        }
    }
    else if ((strName == _T("single_line")) || (strName == _T("singleline"))) {
        SetSingleLine(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("multi_line")) || (strName == _T("multiline"))) {
        //与其他布尔属性保持一致：支持 "true"/"1"
        SetSingleLine(!StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("text")) {
        SetText(strValue);
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))){
        SetTextId(strValue);
    }
    else if ((strName == _T("auto_tooltip")) || (strName == _T("autotooltip"))) {
        SetAutoShowToolTipEnabled(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("font")) {
        SetFontId(strValue);
    }
    else if ((strName == _T("text_color")) || (strName == _T("normal_text_color")) || (strName == _T("normaltextcolor"))) {
        SetStateTextColor(kControlStateNormal, strValue);
    }
    else if ((strName == _T("hovered_text_color")) || (strName == _T("hot_text_color")) || (strName == _T("hottextcolor"))) {
        SetStateTextColor(kControlStateHovered, strValue);
    }
    else if ((strName == _T("pressed_text_color")) || (strName == _T("pushed_text_color")) || (strName == _T("pushedtextcolor"))) {
        SetStateTextColor(kControlStatePressed, strValue);
    }
    else if ((strName == _T("disabled_text_color")) || (strName == _T("disabledtextcolor"))) {
        SetStateTextColor(kControlStateDisabled, strValue);
    }
    else if ((strName == _T("text_padding")) || (strName == _T("textpadding"))) {
        UiPadding rcTextPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcTextPadding);
        SetTextPadding(rcTextPadding, true);
    }
    else if (strName == _T("replace_newline")) {
        // 设置是否替换换行符(将字符串"\\n"替换为换行符"\n"
        SetReplaceNewline(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("spacing_mul")) {
        // 设置行间距倍数
        float mul = 1.0f;
        float add = 0;
        GetLineSpacing(&mul, &add);
        mul = StringUtil::StringToFloat(strValue.c_str(), nullptr);
        SetLineSpacing(mul, add, false);
    }
    else if (strName == _T("spacing_add")) {
        // 设置行间距固定的附加像素值
        float mul = 1.0f;
        float add = 0;
        GetLineSpacing(&mul, &add);
        add = StringUtil::StringToFloat(strValue.c_str(), nullptr);
        SetLineSpacing(mul, add, true);
    }
    else if (strName == _T("vertical_text")) {
        // 设置是否为纵向文本
        SetVerticalText(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("word_spacing")) {
        // 设置两个相邻的字符之间的间隔（像素）
        SetWordSpacing(StringUtil::StringToFloat(strValue.c_str(), nullptr), true);
    }
    else if (strName == _T("use_font_height")) {
        // 设置当纵向绘制文本时，使用字体的默认高度，而不是每个字体的高度（显示时所有字体等高）
        SetUseFontHeight(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("ascii_rotate_90")) {
        // 设置当纵向绘制文本时，对于字母数字等，顺时针旋转90度显示
        SetRotate90ForAscii(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("rich_text")) {
        // 设置文本内容是否为RichText
        SetRichText(StringUtil::IsValueTrue(strValue));
    }
    else {
        return false;
    }
    return true;
}

void LabelImpl::OnWindowChanged()
{
    //RichText模式时，绘制缓存需要重新生成；同时同步RichText关联的窗口
    if ((m_pExtras != nullptr) && (m_pExtras->m_pTextDrawer != nullptr)) {
        m_pExtras->m_pTextDrawer->OnWindowChanged(m_pOwner);
    }
    InvalidateTextCache();
    InvalidateMeasureCache();
}

void LabelImpl::OnLanguageChanged()
{
    //RichText模式时，绘制缓存需要重新生成；必须先失效缓存，再重新测量
    InvalidateRichTextCache();
    InvalidateTextCache();
    InvalidateMeasureCache();
    CheckShowToolTip();
}

void LabelImpl::OnThemeChanged()
{
    //RichText模式时，文本颜色在解析时已按主题色解析为颜色值，需要使解析缓存失效
    InvalidateRichTextCache();
}

void LabelImpl::OnDpiScaleChanged(uint32_t nOldDpiScale, uint32_t /*nNewDpiScale*/)
{
    //DPI变化后，字体缓存需要重新查询
    m_pFont = nullptr;

    UiPadding rcTextPadding = GetTextPadding();
    rcTextPadding = m_pOwner->Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    SetTextPadding(rcTextPadding, false);

    float mul = 1.0f;
    float add = 0;
    GetLineSpacing(&mul, &add);
    add = m_pOwner->Dpi().GetScaleFloat(add, nOldDpiScale);
    SetLineSpacing(mul, add, false);

    float fWordSpacing = GetWordSpacing();
    fWordSpacing = m_pOwner->Dpi().GetScaleFloat(fWordSpacing, nOldDpiScale);
    SetWordSpacing(fWordSpacing, false);

    //RichText模式时，绘制缓存需要重新生成
    InvalidateRichTextCache();
}

uint32_t LabelImpl::GetValidTextStyle(uint32_t nTextFormat)
{
    uint32_t nValidTextFormat = 0;
    if (nTextFormat & TEXT_HCENTER) {
        nValidTextFormat |= TEXT_HCENTER;
    }
    else if (nTextFormat & TEXT_RIGHT) {
        nValidTextFormat |= TEXT_RIGHT;
    }
    else if (nTextFormat & TEXT_HJUSTIFY) {
        nValidTextFormat |= TEXT_HJUSTIFY;
    }
    else {
        nValidTextFormat |= TEXT_LEFT;
    }

    if (nTextFormat & TEXT_VCENTER) {
        nValidTextFormat |= TEXT_VCENTER;
    }
    else if (nTextFormat & TEXT_BOTTOM) {
        nValidTextFormat |= TEXT_BOTTOM;
    }
    else if (nTextFormat & TEXT_VJUSTIFY) {
        nValidTextFormat |= TEXT_VJUSTIFY;
    }
    else {
        nValidTextFormat |= TEXT_TOP;
    }

    if (nTextFormat & TEXT_SINGLELINE) {
        nValidTextFormat |= TEXT_SINGLELINE;
    }
    if (nTextFormat & TEXT_END_ELLIPSIS) {
        nValidTextFormat |= TEXT_END_ELLIPSIS;
    }
    if (nTextFormat & TEXT_PATH_ELLIPSIS) {
        nValidTextFormat |= TEXT_PATH_ELLIPSIS;
    }
    if (nTextFormat & TEXT_NOCLIP) {
        nValidTextFormat |= TEXT_NOCLIP;
    }

    if (nTextFormat & TEXT_WORD_WRAP) {
        nValidTextFormat |= TEXT_WORD_WRAP;
    }

    if (nTextFormat & TEXT_VERTICAL) {
        nValidTextFormat |= TEXT_VERTICAL;
    }

    return nValidTextFormat;
}

DString LabelImpl::GetText() const
{
    //快速路径：有字面文本且无需换行符替换时无需缓存，避免额外占用内存
    if (!m_flags.m_bReplaceNewline && !m_sText.empty()) {
        return m_sText.c_str();
    }
    if (!m_flags.m_bTextCacheValid) {
        DString strText = m_sText.c_str();
        if (strText.empty() && !m_sTextId.empty()) {
            strText = GlobalManager::Instance().Lang().GetStringByID(m_sTextId.c_str());
        }

        if (m_flags.m_bReplaceNewline) {
            //将反斜杠+n这两个字符替换成换行符
            StringUtil::ReplaceAll(_T("\\n"), _T("\n"), strText);
        }
        m_sTextCache = strText.c_str();
        m_flags.m_bTextCacheValid = true;
    }
    return m_sTextCache.c_str();
}

void LabelImpl::SetText(const DString& strText)
{
    if (m_sText == strText) {
        return;
    }
    m_sText = strText;
    InvalidateTextCache();
    InvalidateMeasureCache();
    //必须先使RichText的解析缓存失效，再重新布局：
    //重新布局时会重新测量文本（OnEstimateText），若缓存未失效，自动宽高会按旧文本计算
    InvalidateRichTextCache();
    m_pOwner->RelayoutOrRedraw();
    CheckShowToolTip();
}

DString LabelImpl::GetTextId() const
{
    return m_sTextId.c_str();
}

void LabelImpl::SetTextId(const DString& strTextId)
{
    if (m_sTextId == strTextId) {
        return;
    }
    m_sTextId = strTextId;
    InvalidateTextCache();
    InvalidateMeasureCache();
    //先使RichText的解析缓存失效，再重新布局，避免自动宽高使用旧语言的文本计算
    InvalidateRichTextCache();
    m_pOwner->RelayoutOrRedraw();
    CheckShowToolTip();
}

std::string LabelImpl::GetUTF8Text() const
{
    DString strIn = GetOwnerText();
    return StringConvert::TToUTF8(strIn);
}

void LabelImpl::SetUTF8Text(const std::string& strText)
{
    DString strOut = StringConvert::UTF8ToT(strText);
    if (m_pLabelOwner != nullptr) {
        m_pLabelOwner->SetText(strOut);
    }
    else {
        SetText(strOut);
    }    
}

void LabelImpl::SetUTF8TextId(const std::string& strTextId)
{
    DString strOut = StringConvert::UTF8ToT(strTextId);
    if (m_pLabelOwner != nullptr) {
        m_pLabelOwner->SetTextId(strOut);
    }
    else {
        SetTextId(strOut);
    }    
}

std::string LabelImpl::GetUTF8TextId() const
{
    DString textId;
    if (m_pLabelOwner != nullptr) {
        textId = m_pLabelOwner->GetTextId();
    }
    else {
        textId = GetTextId();
    }
    return StringConvert::TToUTF8(textId);
}

void LabelImpl::SetRichText(bool bRichText)
{
    if (m_flags.m_bRichText != bRichText) {
        m_flags.m_bRichText = bRichText;
        //普通文本与富文本的解析、测量结果不同，需要使绘制缓存失效并重新布局
        InvalidateRichTextCache();
        InvalidateMeasureCache();
        m_pOwner->RelayoutOrRedraw();
        CheckShowToolTip();
    }
}

bool LabelImpl::IsRichText() const
{
    return m_flags.m_bRichText;
}

bool LabelImpl::IsTextEquals(const DString& text) const
{
    //与GetText()保持一致：文本ID、换行符替换等处理结果参与比较
    return StringUtil::StringCompare(GetText().c_str(), text.c_str()) == 0;
}

DString LabelImpl::GetOwnerText() const
{
    if (m_pLabelOwner != nullptr) {
        return m_pLabelOwner->GetText();
    }
    else {
        return GetText();
    }    
}

void LabelImpl::SetAutoShowToolTipEnabled(bool bAutoShow)
{
    m_flags.m_bAutoShowToolTipEnabled = bAutoShow;
    CheckShowToolTip();
}

bool LabelImpl::IsAutoShowToolTipEnabled() const
{
    return m_flags.m_bAutoShowToolTipEnabled;
}

void LabelImpl::SetReplaceNewline(bool bReplaceNewline)
{
    if (m_flags.m_bReplaceNewline != bReplaceNewline) {
        m_flags.m_bReplaceNewline = bReplaceNewline;
        //换行符替换会改变实际显示的文本内容，需要使解析缓存失效并重新布局
        InvalidateTextCache();
        InvalidateMeasureCache();
        InvalidateRichTextCache();
        m_pOwner->RelayoutOrRedraw();
        CheckShowToolTip();
    }
}

bool LabelImpl::IsReplaceNewline() const
{
    return m_flags.m_bReplaceNewline;
}

void LabelImpl::SetVerticalText(bool bVerticalText)
{
    if (m_flags.m_bVerticalText != bVerticalText) {
        m_flags.m_bVerticalText = bVerticalText;
        if (m_flags.m_bVerticalText) {
            m_uTextStyle |= TEXT_VERTICAL;
        }
        else {
            m_uTextStyle &= ~TEXT_VERTICAL;
        }
        //文本方向变化会改变测量宽高，需要重新布局
        InvalidateMeasureCache();
        m_pOwner->RelayoutOrRedraw();
        CheckShowToolTip();
    }
}

bool LabelImpl::IsVerticalText() const
{
    return m_flags.m_bVerticalText;
}

void LabelImpl::SetLineSpacing(float mul, float add, bool bNeedDpiScale)
{
    if (mul < 0) {
        mul = 1.0f;
    }
    if (add < 0) {
        add = 0;
    }
    if (bNeedDpiScale) {
        add = m_pOwner->Dpi().GetScaleFloat(add);
    }
    if ((mul != m_fSpacingMul) || (add != m_fSpacingAdd)) {
        m_fSpacingMul = mul;
        m_fSpacingAdd = add;
        //行间距影响文本的测量高度，需要重新布局
        InvalidateMeasureCache();
        m_pOwner->RelayoutOrRedraw();
        CheckShowToolTip();
    }
}

void LabelImpl::GetLineSpacing(float* mul, float* add) const
{
    if (mul != nullptr) {
        *mul = m_fSpacingMul;
    }
    if (add != nullptr) {
        *add = m_fSpacingAdd;
    }
}

void LabelImpl::SetWordSpacing(float fWordSpacing, bool bNeedDpiScale)
{
    if (fWordSpacing < 0) {
        fWordSpacing = 0;
    }
    if (bNeedDpiScale) {
        fWordSpacing = m_pOwner->Dpi().GetScaleFloat(fWordSpacing);
    }
    if (m_fWordSpacing != fWordSpacing) {
        m_fWordSpacing = fWordSpacing;
        //字间距对横向和纵向文本的测量宽度均有影响，需要重新布局
        InvalidateMeasureCache();
        m_pOwner->RelayoutOrRedraw();
        CheckShowToolTip();
    }
}

float LabelImpl::GetWordSpacing() const
{
    return m_fWordSpacing;
}

void LabelImpl::SetUseFontHeight(bool bUseFontHeight)
{
    if (m_flags.m_bUseFontHeight != bUseFontHeight) {
        m_flags.m_bUseFontHeight = bUseFontHeight;
        InvalidateMeasureCache();
        if (IsVerticalText()) {
            //纵向文本的测量结果会变化，需要重新布局
            m_pOwner->RelayoutOrRedraw();
            CheckShowToolTip();
        }
    }
}

bool LabelImpl::IsUseFontHeight() const
{
    return m_flags.m_bUseFontHeight;
}

void LabelImpl::SetRotate90ForAscii(bool bRotate90ForAscii)
{
    if (m_flags.m_bRotate90ForAscii != bRotate90ForAscii) {
        m_flags.m_bRotate90ForAscii = bRotate90ForAscii;
        InvalidateMeasureCache();
        if (IsVerticalText()) {
            //纵向文本的测量结果会变化，需要重新布局
            m_pOwner->RelayoutOrRedraw();
            CheckShowToolTip();
        }
    }
}

bool LabelImpl::IsRotate90ForAscii() const
{
    return m_flags.m_bRotate90ForAscii;
}

MeasureStringParam LabelImpl::GetMeasureParam() const
{
    MeasureStringParam measureParam;
    measureParam.pFont = GetFont();
    measureParam.uFormat = m_uTextStyle;
    measureParam.fSpacingMul = m_fSpacingMul;
    measureParam.fSpacingAdd = m_fSpacingAdd;
    measureParam.fWordSpacing = m_fWordSpacing;
    measureParam.bUseFontHeight = m_flags.m_bUseFontHeight;
    measureParam.bRotate90ForAscii = m_flags.m_bRotate90ForAscii;
    return measureParam;
}

DrawStringParam LabelImpl::GetDrawParam() const
{
    DrawStringParam drawParam;
    drawParam.pFont = GetFont();
    drawParam.uFormat = m_uTextStyle;
    drawParam.fSpacingMul = m_fSpacingMul;
    drawParam.fSpacingAdd = m_fSpacingAdd;
    drawParam.fWordSpacing = m_fWordSpacing;
    drawParam.bUseFontHeight = m_flags.m_bUseFontHeight;
    drawParam.bRotate90ForAscii = m_flags.m_bRotate90ForAscii;
    return drawParam;
}

DString LabelImpl::GetAutoToolTipText() const
{
    DString toolTip;
    if (m_flags.m_bAutoShowToolTipEnabled && m_flags.m_bAutoShowTooltip) {
        toolTip = GetOwnerText();
        if (IsRichText()) {
            //富文本模式下，提示内容需要去掉标记，避免把<b>、<font>等标记显示给用户
            toolTip = TextDrawer::ToPlainText(toolTip);
        }
    }
    return toolTip;
}

void LabelImpl::CheckShowToolTip()
{
    m_flags.m_bAutoShowTooltip = false;
    if (!m_flags.m_bAutoShowToolTipEnabled || (m_pOwner->GetWindow() == nullptr)) {
        return;
    }
    auto pRender = m_pOwner->GetWindow()->GetRender();
    if (pRender == nullptr) {
        return;
    }    
    const DString sText = GetOwnerText();
    if (sText.empty()) {
        return;
    }
    UiRect rc = m_pOwner->GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    UiPadding rcPadding = m_pOwner->GetControlPadding();
    rc.Deflate(rcPadding);
    rc.Deflate(this->GetTextPadding());

    //单行文本不限制测量宽度（高度），这样可以得到完整的文本尺寸，用于判断是否需要显示提示；
    //多行文本必须使用文本区域的宽（高）度，否则换行结果与实际绘制不一致
    int32_t rectSize = INT32_MAX;
    if (!m_flags.m_bSingleLine) {
        rectSize = !IsVerticalText() ? rc.Width() : rc.Height();
    }

    //测量结果缓存：字体、测量限制尺寸、可用宽高和DPI均未变化时，直接复用上次结果
    //（缓存对象仅在启用auto_tooltip时创建，未使用该功能的控件不占用这部分内存）
    const uint32_t nDpiScale = m_pOwner->Dpi().GetDisplayScaleFactor();
    const uint32_t nFontVersion = GlobalManager::Instance().Font().GetCacheVersion();
    ToolTipCache& cache = EnsureToolTipCache();
    if (cache.m_bValid &&
        (cache.m_nFontVersion == nFontVersion) &&
        (cache.m_nRectSize == rectSize) &&
        (cache.m_nWidth == rc.Width()) &&
        (cache.m_nHeight == rc.Height()) &&
        (cache.m_nDpiScale == nDpiScale)) {
        m_flags.m_bAutoShowTooltip = cache.m_bOverflow;
        return;
    }

    MeasureStringParam measureParam = GetMeasureParam();
    measureParam.rectSize = rectSize;
    UiRect rcMessure = MeasureText(pRender, sText, measureParam);
    const bool bOverflow = (rc.Width() < rcMessure.Width()) || (rc.Height() < rcMessure.Height());

    cache.m_bValid = true;
    cache.m_nFontVersion = nFontVersion;
    cache.m_nRectSize = rectSize;
    cache.m_nWidth = rc.Width();
    cache.m_nHeight = rc.Height();
    cache.m_nDpiScale = nDpiScale;
    cache.m_bOverflow = bOverflow;
    m_flags.m_bAutoShowTooltip = bOverflow;
}

void LabelImpl::InvalidateTextCache()
{
    m_flags.m_bTextCacheValid = false;
    m_sTextCache.clear();
}

void LabelImpl::InvalidateMeasureCache()
{
    //ToolTip缓存对象未创建时无需处理
    if (m_pExtras != nullptr) {
        m_pExtras->m_toolTipCache.m_bValid = false;
    }
}

IFont* LabelImpl::GetFont() const
{
    const uint32_t nFontCacheVersion = GlobalManager::Instance().Font().GetCacheVersion();
    if ((m_pFont == nullptr) || (m_nFontCacheVersion != nFontCacheVersion)) {
        m_nFontCacheVersion = nFontCacheVersion;
        m_pFont = m_pOwner->GetIFontById(m_sFontId.c_str());
    }
    return m_pFont;
}

TextDrawer* LabelImpl::GetTextDrawer()
{
    if (!m_flags.m_bRichText) {
        //普通文本不需要TextDrawer，直接调用渲染接口，避免每个控件都分配该对象
        return nullptr;
    }
    if (m_pExtras == nullptr) {
        m_pExtras = std::make_unique<Extras>();
    }
    if (m_pExtras->m_pTextDrawer == nullptr) {
        m_pExtras->m_pTextDrawer = std::make_unique<TextDrawer>();
    }
    return m_pExtras->m_pTextDrawer.get();
}

void LabelImpl::InvalidateRichTextCache()
{
    if ((m_pExtras != nullptr) && (m_pExtras->m_pTextDrawer != nullptr)) {
        m_pExtras->m_pTextDrawer->SetTextChanged();
    }
}

LabelImpl::ToolTipCache& LabelImpl::EnsureToolTipCache()
{
    if (m_pExtras == nullptr) {
        m_pExtras = std::make_unique<Extras>();
    }
    return m_pExtras->m_toolTipCache;
}

UiRect LabelImpl::MeasureText(IRender* pRender, const DString& strText, const MeasureStringParam& measureParam)
{
    if (strText.empty()) {
        return UiRect();
    }
    TextDrawer* pTextDrawer = GetTextDrawer();
    if (pTextDrawer != nullptr) {
        return pTextDrawer->MeasureString(pRender, strText, measureParam, GetFontId(), IsRichText(), m_pOwner);
    }
    return pRender->MeasureString(strText, measureParam);
}

void LabelImpl::DrawLabelText(IRender* pRender, const DString& strText, const DrawStringParam& drawParam)
{
    if (strText.empty()) {
        return;
    }
    TextDrawer* pTextDrawer = GetTextDrawer();
    if (pTextDrawer != nullptr) {
        pTextDrawer->DrawString(pRender, strText, drawParam, GetFontId(), IsRichText(), m_pOwner);
    }
    else {
        pRender->DrawString(strText, drawParam);
    }
}

bool LabelImpl::HasHoveredStateColor()
{
    if (m_pTextColorMap != nullptr) {
        return m_pTextColorMap->HasHoveredColor();
    }
    return false;
}

UiSize LabelImpl::OnEstimateText(UiSize szAvailable)
{
    return OnEstimateTextWith(GetOwnerText(), szAvailable);
}

UiSize LabelImpl::OnEstimateTextWith(const DString& textValue, UiSize szAvailable)
{
    UiSize fixedSize;
    if (textValue.empty()) {
        //文本为空时，宽度和高度估算结果均为0
        return fixedSize;
    }
    IRender* pRender = nullptr;
    if (m_pOwner->GetWindow() != nullptr) {
        pRender = m_pOwner->GetWindow()->GetRender();
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        //无法估算
        return fixedSize;
    }

    int32_t nWidth = szAvailable.cx;    //最终计算结果为最大宽度
    int32_t nHeight = szAvailable.cy;   //最终计算结果为最大高度
    const UiPadding rcTextPadding = this->GetTextPadding();
    const UiPadding rcPadding = m_pOwner->GetControlPadding();
    if (!m_flags.m_bVerticalText) {
        //文本方向：横向
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
            if (nWidth != INT32_MAX) {
                nWidth -= (rcPadding.left + rcPadding.right);
                nWidth -= (rcTextPadding.left + rcTextPadding.right);
            }
            if (nWidth <= 0) {
                nWidth = INT32_MAX;
            }
        }
        if (!m_pOwner->GetFixedWidth().IsAuto()) {
            if (nWidth != INT32_MAX) {
                nWidth -= (rcPadding.left + rcPadding.right);
                nWidth -= (rcTextPadding.left + rcTextPadding.right);
            }
        }
        if (nWidth < 0) {
            nWidth = 0;
        }

        //横向模式下，若高度为 auto，需以 GetMaxHeight()/INT32_MAX 兜底，
        //否则 nHeight 沿用 szAvailable.cy，多行文本被父容器估算上限截断。
        if (m_pOwner->GetFixedHeight().IsAuto()) {
            nHeight = m_pOwner->GetMaxHeight();
            if (nHeight != INT32_MAX) {
                nHeight -= (rcPadding.top + rcPadding.bottom);
                nHeight -= (rcTextPadding.top + rcTextPadding.bottom);
            }
            if (nHeight <= 0) {
                nHeight = INT32_MAX;
            }
        }
    }
    else {
        //文本方向：纵向
        if (m_pOwner->GetFixedHeight().IsStretch()) {
            //如果是拉伸类型，使用外部高度
            nHeight = CalcStretchValue(m_pOwner->GetFixedHeight(), szAvailable.cy);
        }
        else if (m_pOwner->GetFixedHeight().IsInt32()) {
            nHeight = m_pOwner->GetFixedHeight().GetInt32();
        }
        else if (m_pOwner->GetFixedHeight().IsAuto()) {
            //高度为自动时，不限制高度
            nHeight = m_pOwner->GetMaxHeight();
            if (nHeight != INT32_MAX) {
                nHeight -= (rcPadding.top + rcPadding.bottom);
                nHeight -= (rcTextPadding.top + rcTextPadding.bottom);
            }
            if (nHeight <= 0) {
                nHeight = INT32_MAX;
            }
        }
        if (!m_pOwner->GetFixedHeight().IsAuto()) {
            if (nHeight != INT32_MAX) {
                nHeight -= (rcPadding.top + rcPadding.bottom);
                nHeight -= (rcTextPadding.top + rcTextPadding.bottom);
            }
        }
        if (nHeight < 0) {
            nHeight = 0;
        }

        //纵向模式下，若宽度为 auto，需以 GetMaxWidth()/INT32_MAX 兜底，
        //否则 nWidth 沿用 szAvailable.cx，会被传入的估算上限截断。
        if (m_pOwner->GetFixedWidth().IsAuto()) {
            nWidth = m_pOwner->GetMaxWidth();
            if (nWidth != INT32_MAX) {
                nWidth -= (rcPadding.left + rcPadding.right);
                nWidth -= (rcTextPadding.left + rcTextPadding.right);
            }
            if (nWidth <= 0) {
                nWidth = INT32_MAX;
            }
        }
    }
    
    MeasureStringParam measureParam = GetMeasureParam();
    measureParam.rectSize = !m_flags.m_bVerticalText ? nWidth : nHeight;
    UiRect rect = MeasureText(pRender, textValue, measureParam);
    fixedSize.cx = std::min(rect.Width(), nWidth);
    fixedSize.cx = std::max(fixedSize.cx, 0);
    if (fixedSize.cx > 0) {
        fixedSize.cx += (rcTextPadding.left + rcTextPadding.right);
        fixedSize.cx += (rcPadding.left + rcPadding.right);
    }

    fixedSize.cy = std::min(rect.Height(), nHeight);
    fixedSize.cy = std::max(fixedSize.cy, 0);
    if (fixedSize.cy > 0) {
        fixedSize.cy += (rcTextPadding.top + rcTextPadding.bottom);
        fixedSize.cy += (rcPadding.top + rcPadding.bottom);
    }
    return fixedSize;
}

void LabelImpl::OnPaintText(IRender* pRender)
{
    UiRect rc = m_pOwner->GetRect();
    rc.Deflate(m_pOwner->GetControlPadding());
    rc.Deflate(this->GetTextPadding());
    DoPaintText(rc, pRender);
}

void LabelImpl::DoPaintText(const UiRect& rc, IRender* pRender)
{
    DString textValue = GetOwnerText();
    if (textValue.empty() || (pRender == nullptr)) {
        return;
    }

    ControlStateType stateType = m_pOwner->GetState();
    UiColor dwClrColor = m_pOwner->GetUiColor(GetPaintStateTextColor(m_pOwner->GetState(), stateType));

    DrawStringParam drawParam = GetDrawParam();//绘制参数
    drawParam.textRect = rc;

    if (m_pOwner->IsAnimationPlayerPlaying(AnimationType::kAnimationHovered)) {
        if ((stateType == kControlStateNormal || stateType == kControlStateHovered) && 
            !GetStateTextColor(kControlStateHovered).empty()) {
            //先绘制默认的文本
            const uint8_t nHoveredAlpha = m_pOwner->GetHoveredAlpha();
            bool bPainted = false;
            DString clrColor = GetStateTextColor(kControlStateNormal);
            if (!clrColor.empty()) {                
                drawParam.dwTextColor = m_pOwner->GetUiColor(clrColor);
                drawParam.uFade = 255 - nHoveredAlpha;
                DrawLabelText(pRender, textValue, drawParam);
                bPainted = true;
            }
            //绘制Hovered状态的文本（半透明）
            DString textColor = GetStateTextColor(kControlStateHovered);
            if (!textColor.empty()) {
                drawParam.dwTextColor = m_pOwner->GetUiColor(textColor);
                drawParam.uFade = nHoveredAlpha;
                DrawLabelText(pRender, textValue, drawParam);
                bPainted = true;
            }
            if (bPainted) {
                return;
            }
        }
    }

    drawParam.dwTextColor = dwClrColor;
    drawParam.uFade = 255;
    DrawLabelText(pRender, textValue, drawParam);
}

void LabelImpl::SetTextStyle(uint32_t uStyle, bool bRedraw)
{
    //旧状态
    uint32_t uOldStyle = m_uTextStyle;
    bool bOldSingleLine = m_flags.m_bSingleLine;
    bool bOldVerticalText = m_flags.m_bVerticalText;

    m_uTextStyle = GetValidTextStyle(uStyle);
    if (m_uTextStyle & TEXT_SINGLELINE) {
        m_flags.m_bSingleLine = true;
    }
    else {
        m_flags.m_bSingleLine = false;
    }
    if (m_uTextStyle & TEXT_VERTICAL) {
        m_flags.m_bVerticalText = true;
    }
    else {
        m_flags.m_bVerticalText = false;
    }
    //仅在状态变化时使测量缓存失效（对齐、单行、竖向等属性会改变测量结果）
    const bool bChanged = (uOldStyle != m_uTextStyle) || (bOldSingleLine != m_flags.m_bSingleLine) || (bOldVerticalText != m_flags.m_bVerticalText);
    if (bChanged) {
        //即使不立即重绘，也必须使缓存失效，避免后续布局沿用旧的测量结果
        InvalidateMeasureCache();
        if (bRedraw) {
            m_pOwner->RelayoutOrRedraw();
            CheckShowToolTip();
        }
    }
}

void LabelImpl::SetDefaultTextStyle(bool bRedraw)
{
    //与构造函数中的默认文本风格保持一致（不含TEXT_NOCLIP，避免文本绘制到控件区域之外）
    SetTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_SINGLELINE, bRedraw);
}

uint32_t LabelImpl::GetTextStyle() const
{
    return m_uTextStyle;
}

DString LabelImpl::GetStateTextColor(ControlStateType stateType) const
{
    DString stateColor;
    if (m_pTextColorMap != nullptr) {
        stateColor = m_pTextColorMap->GetStateColor(stateType);
    }
    if (stateColor.empty() && (stateType == kControlStateNormal)) {
        if (m_pOwner->GetWindow() != nullptr) {
            stateColor = m_pOwner->GetWindow()->GetDefaultTextColor();
        }
        else {
            stateColor = GlobalManager::Instance().Color().GetDefaultTextColor();
        }
    }
    if (stateColor.empty() && (stateType == kControlStateDisabled)) {
        if (m_pOwner->GetWindow() != nullptr) {
            stateColor = m_pOwner->GetWindow()->GetDefaultDisabledTextColor();
        }
        else {
            stateColor = GlobalManager::Instance().Color().GetDefaultDisabledTextColor();
        }
    }
    return stateColor;
}

void LabelImpl::SetStateTextColor(ControlStateType stateType, const DString& dwTextColor)
{
    if (m_pTextColorMap != nullptr) {
        if (m_pTextColorMap->EqualsStateColor(stateType, dwTextColor)) {
            //颜色未变化：无需重复设置（避免列表/表格滚动重填时反复写入颜色并触发重绘）
            return;
        }
    }
    if ((stateType == kControlStateHovered) && !dwTextColor.empty()) {
        //未设置悬停颜色时不启用淡入淡出，避免创建无用的动画播放器
        m_pOwner->SetFadeHovered(true);
    }
    if (m_pTextColorMap == nullptr) {
        m_pTextColorMap = std::make_unique<StateColorMap>(m_pOwner);
    }
    m_pTextColorMap->SetStateColor(stateType, dwTextColor);
    m_pOwner->Invalidate();
}

DString LabelImpl::GetPaintStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType)
{
    stateType = buttonStateType;
    if (stateType == kControlStatePressed && GetStateTextColor(kControlStatePressed).empty()) {
        stateType = kControlStateHovered;
    }
    if (stateType == kControlStateHovered && GetStateTextColor(kControlStateHovered).empty()) {
        stateType = kControlStateNormal;
    }
    if (stateType == kControlStateDisabled && GetStateTextColor(kControlStateDisabled).empty()) {
        stateType = kControlStateNormal;
    }
    return GetStateTextColor(stateType);
}

DString LabelImpl::GetFontId() const
{
    return m_sFontId.c_str();
}

void LabelImpl::SetFontId(const DString& strFontId)
{
    if (m_sFontId == strFontId) {
        //字体未变化时不触发重绘（列表/表格滚动重填时会反复设置字体）
        return;
    }
    m_sFontId = strFontId;
    //字体ID变化，字体缓存中的IFont*需要重新查询
    m_pFont = nullptr;
    //字体变化会影响文本的测量结果，需要重新布局
    InvalidateMeasureCache();
    m_pOwner->RelayoutOrRedraw();
    CheckShowToolTip();
}

UiPadding LabelImpl::GetTextPadding() const
{
    return UiPadding(m_rcTextPadding.left, m_rcTextPadding.top, m_rcTextPadding.right, m_rcTextPadding.bottom);
}

void LabelImpl::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
{
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    if ((padding.left < 0) || (padding.top < 0) ||
        (padding.right < 0) || (padding.bottom < 0)) {
        return;
    }
    if (bNeedDpiScale) {
        m_pOwner->Dpi().ScalePadding(padding);
    }    
    if (!this->GetTextPadding().Equals(padding)) {
        m_rcTextPadding.left = TruncateToUInt16(padding.left);
        m_rcTextPadding.top = TruncateToUInt16(padding.top);
        m_rcTextPadding.right = TruncateToUInt16(padding.right);
        m_rcTextPadding.bottom = TruncateToUInt16(padding.bottom);
        InvalidateMeasureCache();
        m_pOwner->RelayoutOrRedraw();
    }    
}

bool LabelImpl::IsSingleLine() const
{
    return m_flags.m_bSingleLine;
}

void LabelImpl::SetSingleLine(bool bSingleLine)
{
    if (m_flags.m_bSingleLine != bSingleLine) {
        m_flags.m_bSingleLine = bSingleLine;
        if (m_flags.m_bSingleLine) {
            m_uTextStyle |= TEXT_SINGLELINE;
        }
        else {
            m_uTextStyle &= ~TEXT_SINGLELINE;
        }
        //单行/多行模式会影响文本的测量结果，需要重新布局
        InvalidateMeasureCache();
        m_pOwner->RelayoutOrRedraw();
        CheckShowToolTip();
    }   
}

}
