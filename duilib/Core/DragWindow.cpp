#include "DragWindow.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{
/** 位图显示控件
*/
class DragWindowBitmap: public Control
{
public:
    /** 绘制函数
    */
    virtual void Paint(ui::IRender* pRender, const ui::UiRect& rcPaint) override
    {
        __super::Paint(pRender, rcPaint);
        if ((pRender != nullptr) && (m_pBitmap != nullptr)) {
            UiRect rcCorners;
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
            pRender->DrawImage(rcPaint, m_pBitmap.get(), destRect, rcCorners, srcRect, rcCorners);
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

std::wstring DragWindow::GetSkinFolder()
{
    return L"";
}

std::wstring DragWindow::GetSkinFile()
{
    return L"<?xml version = \"1.0\" encoding=\"utf-8\"?>"
           L"<Window size=\"90,90\" >"
           L"    <VBox width=\"stretch\" height=\"stretch\" visible=\"true\" bkcolor=\"white\"/>"
           L"</Window>";
}

std::wstring DragWindow::GetWindowClassName() const
{
    return L"DragWindow";
}

void DragWindow::OnFinalMessage(HWND hWnd)
{
    Window::OnFinalMessage(hWnd);
    Release();
}

/** 设置显示的图片
* @param [in] pBitmap 图片资源的接口
*/
void DragWindow::SetDragImage(const std::shared_ptr<IBitmap>& pBitmap)
{
    Box* pBox = GetRoot();
    ASSERT(pBox != nullptr);
    if (pBox == nullptr) {
        return;
    }
    if (pBitmap == nullptr) {
        size_t nCount = pBox->GetItemCount();
        if (nCount > 0) {
            DragWindowBitmap* pBitmapControl = dynamic_cast<DragWindowBitmap*>(pBox->GetItemAt(nCount - 1));
            if (pBitmapControl != nullptr) {
                pBitmapControl->SetBitmap(nullptr);
            }
        }
        return;
    }
    const int32_t nImageWidth = pBitmap->GetWidth();
    const int32_t nImageHeight = pBitmap->GetHeight();

    DragWindowBitmap* pBitmapControl = new DragWindowBitmap();
    pBitmapControl->SetBitmap(pBitmap);
    pBitmapControl->SetAttribute(L"width", L"100%");
    pBitmapControl->SetAttribute(L"height", L"100%");
    pBitmapControl->SetAttribute(L"bkcolor", L"white");
    pBox->AddItem(pBitmapControl);

    //根据位图的大小，调整窗口大小
    Resize(nImageWidth, nImageHeight, false, false);
}

void DragWindow::AdjustPos()
{
    UiPadding rcCorner = GetShadowCorner();
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
    SetWindowPos(rc, false, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE, nullptr, true);
}

}
