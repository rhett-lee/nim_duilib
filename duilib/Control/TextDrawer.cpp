#include "TextDrawer.h"

namespace ui
{

TextDrawer::TextDrawer():
    m_bRichTextChanged(true)
{
}

TextDrawer::~TextDrawer()
{
}

void TextDrawer::SetTextChanged()
{
    m_bRichTextChanged = true;
}

void TextDrawer::UpdateTextDrawProps(uint32_t uFormat, float fSpacingMul, float fSpacingAdd, const DString& fontId)
{
    ASSERT(m_pRichText != nullptr);
    if (m_pRichText == nullptr) {
        return;
    }
    m_pRichText->SetRowSpacingMul(fSpacingMul);
    m_pRichText->SetRowSpacingAdd(fSpacingAdd);
    if (uFormat & TEXT_HCENTER) {
        m_pRichText->SetTextHAlignType(HorAlignType::kAlignCenter);
    }
    else if (uFormat & TEXT_RIGHT) {
        m_pRichText->SetTextHAlignType(HorAlignType::kAlignRight);
    }
    else {
        m_pRichText->SetTextHAlignType(HorAlignType::kAlignLeft);
    }

    if (uFormat & TEXT_VCENTER) {
        m_pRichText->SetTextVAlignType(VerAlignType::kAlignCenter);
    }
    else if (uFormat & TEXT_BOTTOM) {
        m_pRichText->SetTextVAlignType(VerAlignType::kAlignBottom);
    }
    else {
        m_pRichText->SetTextVAlignType(VerAlignType::kAlignTop);
    }
    m_pRichText->SetWordWrap(uFormat & TEXT_SINGLELINE ? false : true);
    m_pRichText->SetFontId(fontId);
}

UiRect TextDrawer::MeasureString(IRender* pRender,
                                 const DString& strText,
                                 const MeasureStringParam& measureParam,
                                 const DString& fontId,
                                 bool bRichText,
                                 Control* pOwner)
{
    ASSERT(pRender != nullptr);
    if ((pRender == nullptr) || strText.empty()) {
        return UiRect();
    }
    if (measureParam.uFormat & TEXT_VERTICAL) {
        //纵向文本，强制不启用RichText
        bRichText = false;
    }
    UiRect rcMessure;
    if (bRichText && (pOwner != nullptr)) {
        //RichText文本
        if (m_pRichText == nullptr) {
            m_pRichText = std::make_unique<RichText>(pOwner->GetWindow());
            m_pRichText->SetAttribute(_T("trim_policy"), _T("none")); //对空格不执行trim操作
        }
        m_pRichText->SetWindow(nullptr); //不设置关联窗口，避免产生Invalidate调用
        UpdateTextDrawProps(measureParam.uFormat, measureParam.fSpacingMul, measureParam.fSpacingAdd, fontId);

        m_pRichText->SetWindow(pOwner->GetWindow()); //绘制过程中和解析RichText的时候，需要关联窗口（需要Dpi缩放等操作）
        if (m_bRichTextChanged) {
            m_pRichText->SetText(strText);
            m_bRichTextChanged = false;
        }

        UiSize szAvailable;
        szAvailable.cx = measureParam.rectSize;
        szAvailable.cy = 0;
        UiSize szEstimateSize = m_pRichText->EstimateText(szAvailable);
        rcMessure.right = rcMessure.left + szEstimateSize.cx;
        rcMessure.bottom = rcMessure.top + szEstimateSize.cy;
    }
    else {
        //普通文本
        rcMessure = pRender->MeasureString(strText, measureParam);
    }
    return rcMessure;
}

void TextDrawer::DrawString(IRender* pRender,
                            const DString& strText,
                            const DrawStringParam& drawParam,
                            const DString& fontId,
                            bool bRichText,
                            Control* pOwner)
{
    ASSERT(pRender != nullptr);
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    if (drawParam.uFormat & TEXT_VERTICAL) {
        //纵向文本，强制不启用RichText
        bRichText = false;
    }
    if (bRichText && (pOwner != nullptr)) {
        //RichText文本
        if (m_pRichText == nullptr) {
            m_pRichText = std::make_unique<RichText>(pOwner->GetWindow());
            m_pRichText->SetAttribute(_T("trim_policy"), _T("none")); //对空格不执行trim操作
        }
        m_pRichText->SetWindow(nullptr); //不设置关联窗口，避免产生Invalidate调用
        m_pRichText->SetRect(drawParam.textRect);//不调用SetPos函数，避免产生回调等事件
        m_pRichText->SetAlpha(drawParam.uFade);
        m_pRichText->SetTextColor(pOwner->GetColorString(drawParam.dwTextColor));
        UpdateTextDrawProps(drawParam.uFormat, drawParam.fSpacingMul, drawParam.fSpacingAdd, fontId);

        m_pRichText->SetWindow(pOwner->GetWindow()); //绘制过程中和解析RichText的时候，需要关联窗口（需要Dpi缩放等操作）
        if (m_bRichTextChanged) {
            m_pRichText->SetText(strText);
            m_bRichTextChanged = false;
        }        
        m_pRichText->PaintText(pRender);
    }
    else {
        //普通文本
        pRender->DrawString(strText, drawParam);
    }
}

}//namespace ui

