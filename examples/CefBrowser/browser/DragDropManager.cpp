#include "DragDropManager.h"
#include "browser/DragForm.h"
#include "browser/BrowserForm.h"
#include "browser/BrowserManager.h"

DragDropManager::DragDropManager():
    m_pDragingBox(nullptr),
    m_pDragForm(nullptr)
{
}

DragDropManager::~DragDropManager()
{
}

DragDropManager* DragDropManager::GetInstance()
{
    static DragDropManager self;
    return &self;
}

bool DragDropManager::IsDragingBorwserBox() const
{
    return (m_pDragingBox != nullptr);
}

bool DragDropManager::StartDragBorwserBox(BrowserBox* browserBox, std::shared_ptr<ui::IBitmap> spIBitmap, const ui::UiPoint& ptOffset)
{
    ASSERT(browserBox != nullptr);
    if (browserBox == nullptr) {
        return false;
    }
    m_pDragingBox = browserBox;
    BrowserForm* dragBrowserForm = dynamic_cast<BrowserForm*>(m_pDragingBox->GetBrowserForm());
    ASSERT(dragBrowserForm != nullptr);
    if (dragBrowserForm == nullptr) {
        m_pDragingBox = nullptr;
        return false;
    }

    // 获取被拖拽浏览器窗口中浏览器盒子的数量
    int32_t box_count = dragBrowserForm->GetBoxCount();
    ASSERT(box_count > 0);

    if (!dragBrowserForm->OnBeforeDragBoxCallback(ui::StringConvert::UTF8ToT(m_pDragingBox->GetBrowserId()))) {
        m_pDragingBox = nullptr;
        return false;
    }

    if ((m_pDragForm != nullptr) && m_pDragForm->IsClosingWnd()) {
        m_pDragForm->Release();
        m_pDragForm = nullptr;
    }
    if (m_pDragForm == nullptr) {
        m_pDragForm = new DragForm;
        m_pDragForm->AddRef();        

        ui::WindowCreateParam createWndParam;
        createWndParam.m_dwStyle = ui::kWS_POPUP;
        createWndParam.m_dwExStyle = ui::kWS_EX_LAYERED | ui::kWS_EX_NOACTIVATE | ui::kWS_EX_TRANSPARENT;
        m_pDragForm->CreateWnd(nullptr, createWndParam);
        ASSERT(m_pDragForm->IsWindow());
        if (m_pDragForm->IsWindow()) {
            m_pDragForm->AddRef();
        }

        DString title = m_pDragingBox->GetTitle();
        ui::Box* pRootBox = m_pDragForm->GetRootBox();
        if (pRootBox != nullptr) {
            if (pRootBox->GetItemCount() > 0) {
                ui::Label* pTitle = dynamic_cast<ui::Label*>(pRootBox->GetItemAt(0));
                if (pTitle != nullptr) {
                    pTitle->SetText(title);
                }
            }
        }
        m_pDragForm->SetDragImage(spIBitmap);
        m_pDragForm->ShowWindow(ui::kSW_SHOW_NA);
        m_pDragForm->AdjustPos();
    }
    return true;
}

void DragDropManager::EndDragBorwserBox(bool bSuccess)
{
    if (m_pDragForm != nullptr) {
        if (!m_pDragForm->IsClosingWnd()) {
            m_pDragForm->SetDragImage(nullptr);
            m_pDragForm->CloseWnd();
        }
        m_pDragForm->Release();
        m_pDragForm = nullptr;
    }

    if (m_pDragingBox == nullptr) {
        return;
    }

    // 获取当前被拖拽的浏览器盒子所属的浏览器窗口
    BrowserForm* dragBrowserForm = dynamic_cast<BrowserForm*>(m_pDragingBox->GetBrowserForm());
    ASSERT(dragBrowserForm != nullptr);
    if (dragBrowserForm == nullptr) {
        m_pDragingBox = nullptr;
        return;
    }

    if (!bSuccess) {
        //操作失败
        dragBrowserForm->OnAfterDragBoxCallback(false);
        m_pDragingBox = nullptr;
        return;
    }

    //确定目标窗口
    BrowserForm* dropBrowserForm = nullptr;
    ui::UiPoint screenPt;
    dragBrowserForm->GetCursorPos(screenPt);
    ui::Window* pWindow = dragBrowserForm->WindowFromPoint(screenPt, true);
    if (pWindow != nullptr) {
        dropBrowserForm = dynamic_cast<BrowserForm*>(pWindow);
    }

    // 获取被拖拽浏览器窗口中浏览器盒子的数量
    int box_count = dragBrowserForm->GetBoxCount();
    ASSERT(box_count > 0);

    if (dragBrowserForm == dropBrowserForm) {
        dropBrowserForm = nullptr;
    }

    if (dropBrowserForm != nullptr) {
        //被拖拽的浏览器盒子放入到另外一个浏览器窗口里
        dragBrowserForm->OnAfterDragBoxCallback(true);
        if (dragBrowserForm->DetachBox(m_pDragingBox)) {
            dropBrowserForm->AttachBox(m_pDragingBox);
        }
    }
    else {
        // 如果被拖拽的浏览器窗口里只有一个浏览器盒子,则拖拽失败
        if (1 == box_count) {
            dragBrowserForm->OnAfterDragBoxCallback(false);
        }
        // 如果有多个浏览器盒子, 就把浏览器盒子脱离原浏览器窗口，附加到新的浏览器窗口，拖拽成功
        else {
            dragBrowserForm->OnAfterDragBoxCallback(true);
            if (dragBrowserForm->DetachBox(m_pDragingBox)) {
                BrowserForm* newBrowserForm = BrowserManager::GetInstance()->CreateBrowserForm();
                if (newBrowserForm->CreateWnd(nullptr, ui::WindowCreateParam(_T("CefBrowser")))) {
                    if (newBrowserForm->AttachBox(m_pDragingBox)) {
                        // 这里设置新浏览器窗口的位置，设置到偏移鼠标坐标100,20的位置
                        ui::UiPoint pt_mouse;
                        newBrowserForm->GetCursorPos(pt_mouse);

                        const int kDragFormXOffset = -100;   //拖拽出新浏览器窗口后的相对鼠标的x偏移坐标
                        const int kDragFormYOffset = -20;    //拖拽出新浏览器窗口后的相对鼠标的y偏移坐标
                        ui::UiRect rect(pt_mouse.x + newBrowserForm->Dpi().GetScaleInt(kDragFormXOffset),
                                        pt_mouse.y + newBrowserForm->Dpi().GetScaleInt(kDragFormYOffset),
                                        0, 0);
                        newBrowserForm->SetWindowPos(ui::InsertAfterWnd(), rect.left, rect.top, rect.Width(), rect.Height(), ui::kSWP_NOSIZE);
                        newBrowserForm->ShowWindow(ui::kSW_SHOW_NORMAL);
                    }
                }
            }
        }
    }
    m_pDragingBox = nullptr;
}

void DragDropManager::UpdateDragFormPos()
{
    if (m_pDragingBox != nullptr) {
        //将目标窗口，置为前端窗口
        BrowserForm* dragBrowserForm = dynamic_cast<BrowserForm*>(m_pDragingBox->GetBrowserForm());
        ASSERT(dragBrowserForm != nullptr);
        if (dragBrowserForm != nullptr) {
            BrowserForm* dropBrowserForm = nullptr;
            ui::UiPoint screenPt;
            dragBrowserForm->GetCursorPos(screenPt);
            ui::Window* pWindow = dragBrowserForm->WindowFromPoint(screenPt, true);
            if (pWindow != nullptr) {
                dropBrowserForm = dynamic_cast<BrowserForm*>(pWindow);
            }
            if (dropBrowserForm != nullptr) {
                ui::InsertAfterWnd insertAfterWnd;
                if ((m_pDragForm != nullptr) && !m_pDragForm->IsClosingWnd()) {
                    insertAfterWnd.m_pWindow = m_pDragForm;
                }
                dropBrowserForm->SetWindowPos(insertAfterWnd, 0, 0, 0, 0, ui::kSWP_NOSIZE | ui::kSWP_NOMOVE | ui::kSWP_NOACTIVATE);
            }
        }
    }

    if (m_pDragForm != nullptr) {
        if (IsDragingBorwserBox()) {
            if (!m_pDragForm->IsClosingWnd()) {
                m_pDragForm->AdjustPos();
            }
        }
        else {
            if (!m_pDragForm->IsClosingWnd()) {
                m_pDragForm->SetDragImage(nullptr);
                m_pDragForm->CloseWnd();
            }
            m_pDragForm->Release();
            m_pDragForm = nullptr;
        }
    }
}
