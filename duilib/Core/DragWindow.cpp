#include "DragWindow.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowCreateAttributes.h"
#include "duilib/Core/DpiManager.h"

namespace ui
{
/** 位图显示控件
*/
class DragWindowBitmap: public Control
{
    typedef Control BaseClass;
public:
    explicit DragWindowBitmap(Window* pWindow):
        Control(pWindow)
    {
    }

    /** 绘制函数
    */
    virtual void Paint(ui::IRender* pRender, const ui::UiRect& rcPaint) override
    {
        BaseClass::Paint(pRender, rcPaint);
        if ((pRender != nullptr) && (m_pBitmap != nullptr)) {
            UiRect destRect = GetRect();
            UiRect srcRect;
            srcRect.right = m_pBitmap->GetWidth();
            srcRect.bottom = m_pBitmap->GetHeight();
            //居中对齐
            if (srcRect.Height() < destRect.Height()) {
                destRect.top = destRect.CenterY() - srcRect.Height() / 2;
                destRect.bottom = destRect.top + srcRect.Height();
            }
            if (srcRect.Width() < destRect.Width()) {
                destRect.left = destRect.CenterX() - srcRect.Width() / 2;
                destRect.right = destRect.left + srcRect.Width();
            }
            ImageDrawParam drawParam;
            drawParam.rcPaint = rcPaint;
            pRender->DrawImage(m_pBitmap.get(), destRect, srcRect, drawParam);
        }
    }

    /** 设置绘制的位图
    */
    void SetBitmap(const std::shared_ptr<IBitmap>& pBitmap)
    {
        m_pBitmap = pBitmap;
    }

private:
    /** 显示的位图
    */
    std::shared_ptr<IBitmap> m_pBitmap;
};

DragWindow::DragWindow():
    m_nRefCount(0)
{
}

DragWindow::~DragWindow()
{
}

void DragWindow::AddRef()
{
    ASSERT(m_nRefCount >= 0);
    ++m_nRefCount;
}

void DragWindow::Release()
{
    ASSERT(m_nRefCount > 0);
    --m_nRefCount;
    if (m_nRefCount == 0) {
        delete this;
    }
}

DString DragWindow::GetSkinFolder()
{
    return _T("");
}

DString DragWindow::GetSkinFile()
{
    return _T("<?xml version = \"1.0\" encoding=\"utf-8\"?>")
           _T("<Window class=\"popup_window\" size=\"90,90\" shadow_snap=\"false\">")
           _T("    <VBox width=\"stretch\" height=\"stretch\" visible=\"true\" bkcolor=\"bg_window_card\"/>")
           _T("</Window>");
}

void DragWindow::OnFinalMessage()
{
    Release();
}

void DragWindow::SetDragImage(const std::shared_ptr<IBitmap>& pBitmap)
{
    Box* pBox = GetXmlRoot();
    ASSERT(pBox != nullptr);
    if (pBox == nullptr) {
        return;
    }

    int32_t nExtraHeight = 0;
    if (pBitmap == nullptr) {
        const size_t nCount = pBox->GetItemCount();
        if (nCount > 0) {
            DragWindowBitmap* pBitmapControl = dynamic_cast<DragWindowBitmap*>(pBox->GetItemAt(nCount - 1));
            if (pBitmapControl != nullptr) {
                pBitmapControl->SetBitmap(nullptr);
            }
        }
        return;
    }
    else {
        const size_t nCount = pBox->GetItemCount();
        for (size_t nItem = 0; nItem < nCount; ++nItem) {
            Control* pControl = pBox->GetItemAt(nItem);
            if (pControl != nullptr) {
                if (pControl->GetFixedHeight().IsAuto()) {
                    UiRect rcClient;
                    GetClientRect(rcClient);
                    nExtraHeight = pControl->EstimateSize(UiSize(rcClient.Width(), rcClient.Height())).cy.GetInt32();
                }
                else {
                    nExtraHeight = pControl->GetFixedHeight().GetInt32();
                }
            }
        }
    }
    const int32_t nImageWidth = pBitmap->GetWidth();
    const int32_t nImageHeight = pBitmap->GetHeight();

    DragWindowBitmap* pBitmapControl = new DragWindowBitmap(this);
    pBitmapControl->SetBitmap(pBitmap);
    pBitmapControl->SetAttribute(_T("width"), _T("100%"));
    pBitmapControl->SetAttribute(_T("height"), _T("100%"));
    pBitmapControl->SetAttribute(_T("bkcolor"), _T("white"));
    pBox->AddItem(pBitmapControl);

    //根据位图的大小，调整窗口大小
    int32_t cx = nImageWidth;
    int32_t cy = nImageHeight + nExtraHeight;
    Dpi().ClientSizeToWindowSize(cx);
    Dpi().ClientSizeToWindowSize(cy);
    Resize(cx, cy, false, false);
}

UiRect DragWindow::CalcWindowRect(const IBitmap* pBitmap, const UiPoint& ptCursor, const DpiManager& dpi)
{
    //预取窗口XML中的初始大小与阴影边距（阴影边距为DIP，需要换算）
    WindowCreateAttributes createAttributes;
    GetCreateWindowAttributes(createAttributes);

    //阴影边距：DIP -> 屏幕坐标
    UiPadding rcShadow;
    rcShadow.left   = dpi.GetScaleWindowSize(createAttributes.m_rcShadowCorner.left);
    rcShadow.top    = dpi.GetScaleWindowSize(createAttributes.m_rcShadowCorner.top);
    rcShadow.right  = dpi.GetScaleWindowSize(createAttributes.m_rcShadowCorner.right);
    rcShadow.bottom = dpi.GetScaleWindowSize(createAttributes.m_rcShadowCorner.bottom);

    //客户区大小：优先使用位图大小（物理像素 -> 屏幕坐标），否则使用XML中配置的初始大小
    int32_t nClientWidth = 0;
    int32_t nClientHeight = 0;
    if (pBitmap != nullptr) {
        nClientWidth = pBitmap->GetWidth();
        nClientHeight = pBitmap->GetHeight();
        dpi.ClientSizeToWindowSize(nClientWidth);
        dpi.ClientSizeToWindowSize(nClientHeight);
    }
    if ((nClientWidth <= 0) || (nClientHeight <= 0)) {
        //XML中配置的初始大小已包含阴影，需要扣除阴影得到客户区大小
        nClientWidth = createAttributes.m_szInitSize.cx;
        nClientHeight = createAttributes.m_szInitSize.cy;
        if (createAttributes.m_bShadowAttached) {
            nClientWidth -= (rcShadow.left + rcShadow.right);
            nClientHeight -= (rcShadow.top + rcShadow.bottom);
        }
        if (nClientWidth < 0) {
            nClientWidth = 0;
        }
        if (nClientHeight < 0) {
            nClientHeight = 0;
        }
    }

    //与AdjustPos的定位规则保持一致：鼠标位置对准客户区的顶部中点
    UiRect rcWindow;
    rcWindow.left = ptCursor.x - rcShadow.left - nClientWidth / 2;
    rcWindow.top = ptCursor.y - rcShadow.top;
    rcWindow.right = rcWindow.left + nClientWidth + rcShadow.left + rcShadow.right;
    rcWindow.bottom = rcWindow.top + nClientHeight + rcShadow.top + rcShadow.bottom;
    return rcWindow;
}

void DragWindow::AdjustPos()
{
    UiPadding rcCorner = GetCurrentShadowCorner();
    Dpi().ClientSizeToWindowSize(rcCorner);
    UiPoint ptCursor;
    GetCursorPos(ptCursor);
    ptCursor.x -= rcCorner.left;
    ptCursor.y -= rcCorner.top;
    UiRect rc;
    GetWindowRect(rc);
    ptCursor.x -= (rc.right - rc.left - rcCorner.left - rcCorner.right) / 2;
   
    rc.left = ptCursor.x;
    rc.top = ptCursor.y;
    rc.right = rc.left;
    rc.bottom = rc.top;
    SetWindowPos(InsertAfterWnd(), rc.left, rc.top, rc.Width(), rc.Height(), kSWP_NOSIZE | kSWP_SHOWWINDOW | kSWP_NOACTIVATE);
}

}
