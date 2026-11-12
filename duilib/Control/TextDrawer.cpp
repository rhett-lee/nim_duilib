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

/** RichText模式使用的实现与缓存：仅在RichText模式下创建，普通文本模式不占用这部分内存
*/
struct TextDrawer::RichTextState
{
    /** RichText实现接口
    */
    std::unique_ptr<RichText> m_pRichText;

    /** RichText当前解析的文本内容（作为解析缓存的键：文本内容不变时无需重新解析）
    */
    DString m_sRichTextText;

    /** 纵向文本降级显示时，纯文本缓存的源文本
    */
    DString m_sPlainTextSource;

    /** 纵向文本降级显示时，去标记后的纯文本
    */
    DString m_sPlainText;
};

TextDrawer::TextDrawer()
{
}

TextDrawer::~TextDrawer()
{
}

TextDrawer::RichTextState& TextDrawer::EnsureRichTextState()
{
    if (m_pRichTextState == nullptr) {
        m_pRichTextState = std::make_unique<RichTextState>();
    }
    return *m_pRichTextState;
}

void TextDrawer::SetTextChanged()
{
    //清除缓存键，使下次测量/绘制时重新解析文本
    if (m_pRichTextState != nullptr) {
        m_pRichTextState->m_sRichTextText.clear();
    }
}

void TextDrawer::OnWindowChanged(Control* pOwner)
{
    SetTextChanged();
    if ((m_pRichTextState != nullptr) && (m_pRichTextState->m_pRichText != nullptr) && (pOwner != nullptr)) {
        Window* pWindow = pOwner->GetWindow();
        if (m_pRichTextState->m_pRichText->GetWindow() != pWindow) {
            //切换窗口时RichText会重新解析；此处禁止产生Invalidate调用
            m_pRichTextState->m_pRichText->SetEnableRedraw(false);
            m_pRichTextState->m_pRichText->SetWindow(pWindow);
            m_pRichTextState->m_pRichText->SetEnableRedraw(true);
        }
    }
}

void TextDrawer::SyncWindow(Control* pOwner)
{
    if ((m_pRichTextState == nullptr) || (m_pRichTextState->m_pRichText == nullptr)) {
        return;
    }
    Window* pWindow = pOwner->GetWindow();
    if (m_pRichTextState->m_pRichText->GetWindow() != pWindow) {
        //仅在窗口真正变化时才切换窗口，避免每次测量/绘制都清空RichText的解析缓存
        m_pRichTextState->m_pRichText->SetWindow(pWindow);
    }
}

DString TextDrawer::ToPlainText(const DString& strRichText)
{
    return RemoveRichTextTags(strRichText);
}

const DString& TextDrawer::GetPlainText(const DString& strText)
{
    RichTextState& richTextState = EnsureRichTextState();
    if (richTextState.m_sPlainTextSource != strText) {
        richTextState.m_sPlainTextSource = strText;
        richTextState.m_sPlainText = RemoveRichTextTags(strText);
    }
    return richTextState.m_sPlainText;
}

void TextDrawer::UpdateTextDrawProps(uint32_t uFormat, float fSpacingMul, float fSpacingAdd, float fWordSpacing, const DString& fontId)
{
    if ((m_pRichTextState == nullptr) || (m_pRichTextState->m_pRichText == nullptr)) {
        return;
    }
    RichText& richText = *(m_pRichTextState->m_pRichText);
    richText.SetRowSpacingMul(fSpacingMul);
    richText.SetRowSpacingAdd(fSpacingAdd);
    //两端对齐：RichText自身支持，映射后与其他控件保持一致
    //（含hjustify时，LabelImpl已保证不会同时存在left/hcenter/right，此处按靠左对齐处理）
    richText.SetTextHAlignJustify((uFormat & TEXT_HJUSTIFY) != 0);
    if (uFormat & TEXT_HCENTER) {
        richText.SetTextHAlignType(HorAlignType::kAlignCenter);
    }
    else if (uFormat & TEXT_RIGHT) {
        richText.SetTextHAlignType(HorAlignType::kAlignRight);
    }
    else {
        richText.SetTextHAlignType(HorAlignType::kAlignLeft);
    }

    if (uFormat & TEXT_VCENTER) {
        richText.SetTextVAlignType(VerAlignType::kAlignCenter);
    }
    else if (uFormat & TEXT_BOTTOM) {
        richText.SetTextVAlignType(VerAlignType::kAlignBottom);
    }
    else {
        richText.SetTextVAlignType(VerAlignType::kAlignTop);
    }
    richText.SetWordWrap(uFormat & TEXT_SINGLELINE ? false : true);
    //字间距与结尾省略号：与其他控件保持一致
    richText.SetLetterSpacing(fWordSpacing);
    richText.SetTextOverflowEllipsis((uFormat & TEXT_END_ELLIPSIS) != 0);
    richText.SetFontId(fontId);
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
        RichTextState& richTextState = EnsureRichTextState();
        if (richTextState.m_pRichText == nullptr) {
            richTextState.m_pRichText = std::make_unique<RichText>(pOwner->GetWindow());
            richTextState.m_pRichText->SetAttribute(_T("trim_policy"), _T("none")); //对空格不执行trim操作
        }
        richTextState.m_pRichText->SetEnableRedraw(false);
        SyncWindow(pOwner); //仅在窗口变化时切换，避免每次测量都触发RichText重新解析
        UpdateTextDrawProps(measureParam.uFormat, measureParam.fSpacingMul, measureParam.fSpacingAdd, measureParam.fWordSpacing, fontId);

        //以实际文本内容作为缓存键：测量文本与绘制文本不同时，也能正确重新解析
        if (richTextState.m_sRichTextText != strText) {
            richTextState.m_pRichText->SetText(strText, false);
            richTextState.m_sRichTextText = strText;
        }
        richTextState.m_pRichText->SetEnableRedraw(true);

        UiSize szAvailable;
        szAvailable.cx = measureParam.rectSize;
        szAvailable.cy = 0;
        UiSize szEstimateSize = richTextState.m_pRichText->EstimateText(szAvailable);
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
        RichTextState& richTextState = EnsureRichTextState();
        if (richTextState.m_pRichText == nullptr) {
            richTextState.m_pRichText = std::make_unique<RichText>(pOwner->GetWindow());
            richTextState.m_pRichText->SetAttribute(_T("trim_policy"), _T("none")); //对空格不执行trim操作
        }
        richTextState.m_pRichText->SetEnableRedraw(false);
        SyncWindow(pOwner); //仅在窗口变化时切换，避免每次绘制都触发RichText重新解析
        richTextState.m_pRichText->SetRect(drawParam.textRect);//不调用SetPos函数，避免产生回调等事件
        richTextState.m_pRichText->SetPaintAlpha(drawParam.uFade);//仅设置绘制透明度，避免在绘制过程中触发窗口重绘
        richTextState.m_pRichText->SetTextColor(pOwner->GetColorString(drawParam.dwTextColor));
        UpdateTextDrawProps(drawParam.uFormat, drawParam.fSpacingMul, drawParam.fSpacingAdd, drawParam.fWordSpacing, fontId);

        //以实际文本内容作为缓存键：测量文本与绘制文本不同时，也能正确重新解析
        if (richTextState.m_sRichTextText != strText) {
            richTextState.m_pRichText->SetText(strText, false);
            richTextState.m_sRichTextText = strText;
        }
        richTextState.m_pRichText->SetEnableRedraw(true);
        richTextState.m_pRichText->PaintText(pRender);
    }
    else {
        //普通文本
        pRender->DrawString(*pText, drawParam);
    }
}

}//namespace ui

