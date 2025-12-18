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
    m_sFontId(),
    m_uTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_SINGLELINE),
    m_bSingleLine(true),
    m_bAutoShowToolTipEnabled(false),
    m_bAutoShowTooltip(false),
    m_bReplaceNewline(false),
    m_fSpacingMul(1.0f),
    m_fSpacingAdd(0),
    m_fWordSpacing(0),
    m_bVerticalText(false),
    m_bUseFontHeight(true),
    m_bRotate90ForAscii(true),
    m_rcTextPadding(),
    m_bRichText(false)
{
    Box* pBox = dynamic_cast<Box*>(pOwner);
    if (pBox != nullptr) {
        pOwner->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
        pOwner->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    }
    else {
        pOwner->SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
        pOwner->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
    }
    m_pTextDrawer = std::make_unique<TextDrawer>();
}

LabelImpl::~LabelImpl()
{
    m_pTextDrawer.reset();
}

bool LabelImpl::SetAttribute(const DString& strName, const DString& strValue)
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
            //垂直对齐：靠下
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_VJUSTIFY;
        }
    }
    else if ((strName == _T("end_ellipsis")) || (strName == _T("endellipsis"))) {
        if (strValue == _T("true")) {
            m_uTextStyle |= TEXT_END_ELLIPSIS;
        }
        else {
            m_uTextStyle &= ~TEXT_END_ELLIPSIS;
        }
    }
    else if ((strName == _T("path_ellipsis")) || (strName == _T("pathellipsis"))) {
        if (strValue == _T("true")) {
            m_uTextStyle |= TEXT_PATH_ELLIPSIS;
        }
        else {
            m_uTextStyle &= ~TEXT_PATH_ELLIPSIS;
        }
    }
    else if ((strName == _T("single_line")) || (strName == _T("singleline"))) {
        SetSingleLine(strValue == _T("true"));
    }
    else if ((strName == _T("multi_line")) || (strName == _T("multiline"))) {
        SetSingleLine(strValue != _T("true"));
    }
    else if (strName == _T("text")) {
        SetText(strValue);
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))){
        SetTextId(strValue);
    }
    else if ((strName == _T("auto_tooltip")) || (strName == _T("autotooltip"))) {
        SetAutoShowToolTipEnabled(strValue == _T("true"));
    }
    else if (strName == _T("font")) {
        SetFontId(strValue);
    }
    else if ((strName == _T("normal_text_color")) || (strName == _T("normaltextcolor"))) {
        SetStateTextColor(kControlStateNormal, strValue);
    }
    else if ((strName == _T("hot_text_color")) || (strName == _T("hottextcolor"))) {
        SetStateTextColor(kControlStateHot, strValue);
    }
    else if ((strName == _T("pushed_text_color")) || (strName == _T("pushedtextcolor"))) {
        SetStateTextColor(kControlStatePushed, strValue);
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
        SetReplaceNewline(strValue == _T("true"));
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
        SetVerticalText(strValue == _T("true"));
    }
    else if (strName == _T("word_spacing")) {
        // 设置两个相邻的字符之间的间隔（像素）
        SetWordSpacing(StringUtil::StringToFloat(strValue.c_str(), nullptr), true);
    }
    else if (strName == _T("use_font_height")) {
        // 设置当纵向绘制文本时，使用字体的默认高度，而不是每个字体的高度（显示时所有字体等高）
        SetUseFontHeight(strValue == _T("true"));
    }
    else if (strName == _T("ascii_rotate_90")) {
        // 设置当纵向绘制文本时，对于字母数字等，顺时针旋转90度显示
        SetRotate90ForAscii(strValue == _T("true"));
    }
    else if (strName == _T("rich_text")) {
        // 设置文本内容是否为RichText
        SetRichText(strValue == _T("true"));
    }
    else {
        return false;
    }
    return true;
}

void LabelImpl::OnWindowChanged()
{
    //RichText模式时，绘制缓存需要重新生成
    m_pTextDrawer->SetTextChanged();
}

void LabelImpl::OnLanguageChanged()
{
    CheckShowToolTip();

    //RichText模式时，绘制缓存需要重新生成
    m_pTextDrawer->SetTextChanged();
}

void LabelImpl::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t /*nNewDpiScale*/)
{
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
    m_pTextDrawer->SetTextChanged();
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
    DString strText = m_sText.c_str();
    if (strText.empty() && !m_sTextId.empty()) {
        strText = GlobalManager::Instance().Lang().GetStringViaID(m_sTextId.c_str());
    }

    if (IsReplaceNewline()) {
        //将反斜杠+n这两个字符替换成换行符
        StringUtil::ReplaceAll(_T("\\n"), _T("\n"), strText);
    }
    return strText;
}

DString LabelImpl::GetTextId() const
{
    return m_sTextId.c_str();
}

void LabelImpl::SetAutoShowToolTipEnabled(bool bAutoShow)
{
    m_bAutoShowToolTipEnabled = bAutoShow;
    CheckShowToolTip();
}

bool LabelImpl::IsAutoShowToolTipEnabled() const
{
    return m_bAutoShowToolTipEnabled;
}

void LabelImpl::SetReplaceNewline(bool bReplaceNewline)
{
    m_bReplaceNewline = bReplaceNewline;
}

bool LabelImpl::IsReplaceNewline() const
{
    return m_bReplaceNewline;
}

void LabelImpl::SetVerticalText(bool bVerticalText)
{
    if (m_bVerticalText != bVerticalText) {
        m_bVerticalText = bVerticalText;
        if (m_bVerticalText) {
            m_uTextStyle |= TEXT_VERTICAL;
        }
        else {
            m_uTextStyle &= ~TEXT_VERTICAL;
        }
        m_pOwner->Invalidate();
    }
}

bool LabelImpl::IsVerticalText() const
{
    return m_bVerticalText;
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
        m_pOwner->Invalidate();
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
        if (IsVerticalText()) {
            m_pOwner->Invalidate();
        }
    }
}

float LabelImpl::GetWordSpacing() const
{
    return m_fWordSpacing;
}

void LabelImpl::SetUseFontHeight(bool bUseFontHeight)
{
    if (m_bUseFontHeight != bUseFontHeight) {
        m_bUseFontHeight = bUseFontHeight;
        if (IsVerticalText()) {
            m_pOwner->Invalidate();
        }
    }
}

bool LabelImpl::IsUseFontHeight() const
{
    return m_bUseFontHeight;
}

void LabelImpl::SetRotate90ForAscii(bool bRotate90ForAscii)
{
    if (m_bRotate90ForAscii != bRotate90ForAscii) {
        m_bRotate90ForAscii = bRotate90ForAscii;
        if (IsVerticalText()) {
            m_pOwner->Invalidate();
        }
    }
}

bool LabelImpl::IsRotate90ForAscii() const
{
    return m_bRotate90ForAscii;
}

void LabelImpl::SetRichText(bool bRichText)
{
    if (m_bRichText != bRichText) {
        m_bRichText = bRichText;
        m_pOwner->Invalidate();
    }    
}

bool LabelImpl::IsRichText() const
{
    return m_bRichText;
}

MeasureStringParam LabelImpl::GetMeasureParam() const
{
    MeasureStringParam measureParam;
    measureParam.pFont = m_pOwner->GetIFontById(this->GetFontId());
    measureParam.uFormat = m_uTextStyle;
    measureParam.fSpacingMul = m_fSpacingMul;
    measureParam.fSpacingAdd = m_fSpacingAdd;
    measureParam.fWordSpacing = m_fWordSpacing;
    measureParam.bUseFontHeight = m_bUseFontHeight;
    measureParam.bRotate90ForAscii = m_bRotate90ForAscii;
    return measureParam;
}

DrawStringParam LabelImpl::GetDrawParam() const
{
    DrawStringParam drawParam;
    drawParam.pFont = m_pOwner->GetIFontById(this->GetFontId());
    drawParam.uFormat = m_uTextStyle;
    drawParam.fSpacingMul = m_fSpacingMul;
    drawParam.fSpacingAdd = m_fSpacingAdd;
    drawParam.fWordSpacing = m_fWordSpacing;
    drawParam.bUseFontHeight = m_bUseFontHeight;
    drawParam.bRotate90ForAscii = m_bRotate90ForAscii;
    return drawParam;
}

DString LabelImpl::GetAutoToolTipText() const
{
    DString toolTip;
    if (m_bAutoShowToolTipEnabled && m_bAutoShowTooltip) {
        toolTip = GetText();
    }
    return toolTip;
}

void LabelImpl::CheckShowToolTip()
{
    m_bAutoShowTooltip = false;
    if (!m_bAutoShowToolTipEnabled || (m_pOwner->GetWindow() == nullptr)) {
        return;
    }
    auto pRender = m_pOwner->GetWindow()->GetRender();
    if (pRender == nullptr) {
        return;
    }    
    const DString sText = this->GetText();
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

    int32_t rectSize = 0;
    if (!IsVerticalText()) {
        int32_t width = m_pOwner->GetFixedWidth().GetInt32();
        if (m_pOwner->GetFixedWidth().IsStretch()) {
            width = 0;
        }
        if (width < 0) {
            width = 0;
        }
        if (!m_bSingleLine && (width == 0)) {
            //多行文本评估宽高的时候，必须指定宽度
            width = rc.Width();
        }
        rectSize = width;
    }
    else {
        int32_t height = m_pOwner->GetFixedHeight().GetInt32();
        if (m_pOwner->GetFixedHeight().IsStretch()) {
            height = 0;
        }
        if (height < 0) {
            height = 0;
        }
        if (!m_bSingleLine && (height == 0)) {
            //多行文本评估宽高的时候，必须指定高度
            height = rc.Height();
        }
        rectSize = height;
    }

    MeasureStringParam measureParam = GetMeasureParam();
    measureParam.rectSize = rectSize;
    UiRect rcMessure = m_pTextDrawer->MeasureString(pRender, sText, measureParam, GetFontId(), IsRichText(), m_pOwner);
    if (rc.Width() < rcMessure.Width() || rc.Height() < rcMessure.Height()) {
        m_bAutoShowTooltip = true;
    }
}

std::string LabelImpl::GetUTF8Text() const
{
    DString strIn = GetText();
    std::string strOut = StringConvert::TToUTF8(strIn);
    return strOut;
}

void LabelImpl::SetText(const DString& strText)
{
    if (m_sText == strText) {
        return;
    }
    m_sText = strText;
    m_pOwner->RelayoutOrRedraw();
    m_pTextDrawer->SetTextChanged();
    CheckShowToolTip();
}

void LabelImpl::SetUTF8Text(const std::string& strText)
{
    DString strOut = StringConvert::UTF8ToT(strText);
    SetText(strOut);
}

void LabelImpl::SetTextId(const DString& strTextId)
{
    if (m_sTextId == strTextId) {
        return;
    }
    m_sTextId = strTextId;
    m_pOwner->RelayoutOrRedraw();
    m_pTextDrawer->SetTextChanged();
    CheckShowToolTip();
}

void LabelImpl::SetUTF8TextId(const std::string& strTextId)
{
    DString strOut = StringConvert::UTF8ToT(strTextId);
    SetTextId(strOut);
}

bool LabelImpl::HasHotColorState()
{
    if (m_pTextColorMap != nullptr) {
        return m_pTextColorMap->HasHotColor();
    }
    return false;
}

UiSize LabelImpl::EstimateText(UiSize szAvailable)
{
    UiSize fixedSize;
    const DString textValue = GetText();
    if (textValue.empty()) {
        //文本为空时，宽度和高度估算结果均为0
        return fixedSize;
    }
    int32_t nWidth = szAvailable.cx;    //最终计算结果为最大宽度
    int32_t nHeight = szAvailable.cy;   //最终计算结果为最大高度
    const UiPadding rcTextPadding = this->GetTextPadding();
    const UiPadding rcPadding = m_pOwner->GetControlPadding();
    if (!m_bVerticalText) {
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
            nWidth -= (rcPadding.left + rcPadding.right);
            nWidth -= (rcTextPadding.left + rcTextPadding.right);
        }
        if (nWidth < 0) {
            nWidth = 0;
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
            //宽度为自动时，不限制宽度
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
            nHeight -= (rcPadding.top + rcPadding.bottom);
            nHeight -= (rcTextPadding.top + rcTextPadding.bottom);
        }
        if (nHeight < 0) {
            nHeight = 0;
        }
    }
    
    if (!textValue.empty() && (m_pOwner->GetWindow() != nullptr)) {
        auto pRender = m_pOwner->GetWindow()->GetRender();
        if (pRender != nullptr) {
            MeasureStringParam measureParam = GetMeasureParam();
            measureParam.rectSize = !m_bVerticalText ? nWidth : nHeight;
            UiRect rect = m_pTextDrawer->MeasureString(pRender, textValue, measureParam, GetFontId(), IsRichText(), m_pOwner);
            fixedSize.cx = std::min(rect.Width(), nWidth);
            if (fixedSize.cx > 0) {
                fixedSize.cx += (rcTextPadding.left + rcTextPadding.right);
                fixedSize.cx += (rcPadding.left + rcPadding.right);
            }

            fixedSize.cy = std::min(rect.Height(), nHeight);
            if (fixedSize.cy) {
                fixedSize.cy += (rcTextPadding.top + rcTextPadding.bottom);
                fixedSize.cy += (rcPadding.top + rcPadding.bottom);
            }
        }
    }
    return fixedSize;
}

void LabelImpl::PaintText(IRender* pRender)
{
    UiRect rc = m_pOwner->GetRect();
    rc.Deflate(m_pOwner->GetControlPadding());
    rc.Deflate(this->GetTextPadding());
    DoPaintText(rc, pRender);
}

void LabelImpl::DoPaintText(const UiRect& rc, IRender* pRender)
{
    DString textValue = this->GetText();
    if (textValue.empty() || (pRender == nullptr)) {
        return;
    }

    ControlStateType stateType = m_pOwner->GetState();
    UiColor dwClrColor = m_pOwner->GetUiColor(GetPaintStateTextColor(m_pOwner->GetState(), stateType));

    DrawStringParam drawParam = GetDrawParam();//绘制参数
    drawParam.textRect = rc;

    if (m_pOwner->GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot)) {
        if ((stateType == kControlStateNormal || stateType == kControlStateHot) && 
            !GetStateTextColor(kControlStateHot).empty()) {
            DString clrColor = GetStateTextColor(kControlStateNormal);
            if (!clrColor.empty()) {                
                drawParam.dwTextColor = m_pOwner->GetUiColor(clrColor);
                drawParam.uFade = 255;
                m_pTextDrawer->DrawString(pRender, textValue, drawParam, GetFontId(), IsRichText(), m_pOwner);
            }

            if (m_pOwner->GetHotAlpha() > 0) {
                DString textColor = GetStateTextColor(kControlStateHot);
                if (!textColor.empty()) {
                    drawParam.dwTextColor = m_pOwner->GetUiColor(textColor);
                    drawParam.uFade = (uint8_t)m_pOwner->GetHotAlpha();
                    m_pTextDrawer->DrawString(pRender, textValue, drawParam, GetFontId(), IsRichText(), m_pOwner);
                }
            }
            return;
        }
    }

    drawParam.dwTextColor = dwClrColor;
    drawParam.uFade = 255;
    m_pTextDrawer->DrawString(pRender, textValue, drawParam, GetFontId(), IsRichText(), m_pOwner);
}

void LabelImpl::SetTextStyle(uint32_t uStyle, bool bRedraw)
{
    m_uTextStyle = GetValidTextStyle(uStyle);
    if (m_uTextStyle & TEXT_SINGLELINE) {
        m_bSingleLine = true;
    }
    else {
        m_bSingleLine = false;
    }
    if (m_uTextStyle & TEXT_VERTICAL) {
        m_bVerticalText = true;
    }
    else {
        m_bVerticalText = false;
    }
    if (bRedraw) {
        m_pOwner->Invalidate();
    }
}

void LabelImpl::SetDefaultTextStyle(bool bRedraw)
{
    SetTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE, bRedraw);
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
        stateColor = GlobalManager::Instance().Color().GetDefaultTextColor();
    }
    if (stateColor.empty() && (stateType == kControlStateDisabled)) {
        stateColor = GlobalManager::Instance().Color().GetDefaultDisabledTextColor();
    }
    return stateColor;
}

void LabelImpl::SetStateTextColor(ControlStateType stateType, const DString& dwTextColor)
{
    if (stateType == kControlStateHot) {
        m_pOwner->GetAnimationManager().SetFadeHot(true);
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
    if (stateType == kControlStatePushed && GetStateTextColor(kControlStatePushed).empty()) {
        stateType = kControlStateHot;
    }
    if (stateType == kControlStateHot && GetStateTextColor(kControlStateHot).empty()) {
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
    m_sFontId = strFontId;
    m_pOwner->Invalidate();
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
        m_pOwner->RelayoutOrRedraw();
    }    
}

bool LabelImpl::IsSingleLine() const
{
    return m_bSingleLine;
}

void LabelImpl::SetSingleLine(bool bSingleLine)
{
    if (m_bSingleLine != bSingleLine) {
        m_bSingleLine = bSingleLine;
        if (m_bSingleLine) {
            m_uTextStyle |= TEXT_SINGLELINE;
        }
        else {
            m_uTextStyle &= ~TEXT_SINGLELINE;
        }
        m_pOwner->Invalidate();
    }   
}

}
