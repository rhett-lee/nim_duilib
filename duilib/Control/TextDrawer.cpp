#include "TextDrawer.h"

namespace ui
{

namespace
{
/** 去除富文本标记，返回纯文本
*   只跳过"<...>"形式的标记，其余字符原样保留；
*   若"<"之后没有对应的">"，则按普通字符处理。
*/
DString RemoveRichTextTags(const DString& strText)
{
    DString strPlainText;
    strPlainText.reserve(strText.size());
    const size_t nTextCount = strText.size();
    bool bInTag = false;
    for (size_t nIndex = 0; nIndex < nTextCount; ++nIndex) {
        const DString::value_type ch = strText[nIndex];
        if (bInTag) {
            if (ch == _T('>')) {
                bInTag = false;
            }
        }
        else if ((ch == _T('<')) && (strText.find(_T('>'), nIndex + 1) != DString::npos)) {
            bInTag = true;
        }
        else {
            strPlainText.push_back(ch);
        }
    }
    return strPlainText;
}
}//namespace

TextDrawer::TextDrawer()
{
}

TextDrawer::~TextDrawer()
{
}

void TextDrawer::SetTextChanged()
{
    //清除缓存键，使下次测量/绘制时重新解析文本
    m_sRichTextText.clear();
}

DString TextDrawer::ToPlainText(const DString& strRichText)
{
    return RemoveRichTextTags(strRichText);
}

const DString& TextDrawer::GetPlainText(const DString& strText)
{
    if (m_sPlainTextSource != strText) {
        m_sPlainTextSource = strText;
        m_sPlainText = RemoveRichTextTags(strText);
    }
    return m_sPlainText;
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
    const DString* pText = &strText;
    if (measureParam.uFormat & TEXT_VERTICAL) {
        //纵向文本不支持RichText：降级为去掉标记的纯文本，避免把标记当成文本显示
        if (bRichText) {
            pText = &GetPlainText(strText);
        }
        bRichText = false;
    }
    UiRect rcMessure;
    if (bRichText && (pOwner != nullptr)) {
        //RichText文本
        if (m_pRichText == nullptr) {
            m_pRichText = std::make_unique<RichText>(pOwner->GetWindow());
            m_pRichText->SetAttribute(_T("trim_policy"), _T("none")); //对空格不执行trim操作
        }
        m_pRichText->SetEnableRedraw(false);
        m_pRichText->SetWindow(nullptr); //不设置关联窗口，避免产生Invalidate调用
        UpdateTextDrawProps(measureParam.uFormat, measureParam.fSpacingMul, measureParam.fSpacingAdd, fontId);

        m_pRichText->SetWindow(pOwner->GetWindow()); //绘制过程中和解析RichText的时候，需要关联窗口（需要Dpi缩放等操作）
        //以实际文本内容作为缓存键：测量文本与绘制文本不同时，也能正确重新解析
        if (m_sRichTextText != strText) {
            m_pRichText->SetText(strText, false);
            m_sRichTextText = strText;
        }
        m_pRichText->SetEnableRedraw(true);

        UiSize szAvailable;
        szAvailable.cx = measureParam.rectSize;
        szAvailable.cy = 0;
        UiSize szEstimateSize = m_pRichText->EstimateText(szAvailable);
        rcMessure.right = rcMessure.left + szEstimateSize.cx;
        rcMessure.bottom = rcMessure.top + szEstimateSize.cy;
    }
    else {
        //普通文本
        rcMessure = pRender->MeasureString(*pText, measureParam);
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
    const DString* pText = &strText;
    if (drawParam.uFormat & TEXT_VERTICAL) {
        //纵向文本不支持RichText：降级为去掉标记的纯文本，避免把标记当成文本显示
        if (bRichText) {
            pText = &GetPlainText(strText);
        }
        bRichText = false;
    }
    if (bRichText && (pOwner != nullptr)) {
        //RichText文本
        if (m_pRichText == nullptr) {
            m_pRichText = std::make_unique<RichText>(pOwner->GetWindow());
            m_pRichText->SetAttribute(_T("trim_policy"), _T("none")); //对空格不执行trim操作
        }
        m_pRichText->SetEnableRedraw(false);
        m_pRichText->SetWindow(nullptr); //不设置关联窗口，避免产生Invalidate调用
        m_pRichText->SetRect(drawParam.textRect);//不调用SetPos函数，避免产生回调等事件
        m_pRichText->SetAlpha(drawParam.uFade);
        m_pRichText->SetTextColor(pOwner->GetColorString(drawParam.dwTextColor));
        UpdateTextDrawProps(drawParam.uFormat, drawParam.fSpacingMul, drawParam.fSpacingAdd, fontId);

        m_pRichText->SetWindow(pOwner->GetWindow()); //绘制过程中和解析RichText的时候，需要关联窗口（需要Dpi缩放等操作）
        //以实际文本内容作为缓存键：测量文本与绘制文本不同时，也能正确重新解析
        if (m_sRichTextText != strText) {
            m_pRichText->SetText(strText, false);
            m_sRichTextText = strText;
        }
        m_pRichText->SetEnableRedraw(true);
        m_pRichText->PaintText(pRender);
    }
    else {
        //普通文本
        pRender->DrawString(*pText, drawParam);
    }
}

}//namespace ui

