#include "MainForm.h"
#include "ChildWindowPaint.h"
#include "MyChildWindowEvents.h"

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
    if (!m_childWindowEvents.empty()) {
        CloseChildWindows();
    }    
}

DString MainForm::GetSkinFolder()
{
    return _T("child_window");
}

DString MainForm::GetSkinFile()
{
    return _T("child_window.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();
    //创建子窗口，并关联处理接口
    CreateChildWindows();
}

void MainForm::OnPreCloseWindow()
{
    //销毁子窗口，取消关联关系
    CloseChildWindows();
    BaseClass::OnPreCloseWindow();
}

void MainForm::OnLayeredWindowChanged()
{
    BaseClass::OnLayeredWindowChanged();
    //将层窗口属性同步到子窗口
    for (MyChildWindowEvents* pChildWindowEvents : m_childWindowEvents) {
        if (pChildWindowEvents != nullptr) {
            ui::ChildWindow* pChildWindow = pChildWindowEvents->GetChildWindow();
            if (pChildWindow != nullptr) {
                pChildWindow->SetChildWindowLayered(IsLayeredWindow());
            }
        }
    }
}

void MainForm::CreateChildWindows()
{
    ui::GridBox* pChildWindowBox = dynamic_cast<ui::GridBox*>(FindControl(_T("child_window_box")));
    if (pChildWindowBox != nullptr) {
        size_t nCount = pChildWindowBox->GetItemCount();
        for (size_t nItem = 0; nItem < nCount; ++nItem) {
            ui::ChildWindow* pChildWindow = dynamic_cast<ui::ChildWindow*>(pChildWindowBox->GetItemAt(nItem));
            if (pChildWindow != nullptr) {
                MyChildWindowEvents* pMyChildWindowEvents = new MyChildWindowEvents(pChildWindow, nItem, this);
                pChildWindow->CreateChildWindow(pMyChildWindowEvents);
                m_childWindowEvents.push_back(pMyChildWindowEvents);
            }
        }
    }
}

void MainForm::CloseChildWindows()
{
    std::vector<MyChildWindowEvents*> childWindowEvents;
    childWindowEvents.swap(m_childWindowEvents);
    for (MyChildWindowEvents* pChildWindowEvents : childWindowEvents) {
        if (pChildWindowEvents != nullptr) {
            ui::ChildWindow* pChildWindow = pChildWindowEvents->GetChildWindow();
            if (pChildWindow != nullptr) {
                //关闭子窗口（同步关闭）
                pChildWindow->SetChildWindowEvents(nullptr);
                pChildWindow->CloseChildWindow();
            }
            delete pChildWindowEvents;
            pChildWindowEvents = nullptr;
        }
    }
}

bool MainForm::PaintChildWindow(ui::ChildWindow* pChildWindow)
{
    if (pChildWindow == nullptr) {
        return false;
    }
    std::weak_ptr<ui::WeakFlag> weakFlag = pChildWindow->GetWeakFlag();
    ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, [weakFlag, pChildWindow]() {
        if (!weakFlag.expired()) {
            pChildWindow->InvalidateChildWindow();
        }
        }, 10);

    return true;
    //////////////////////////////////////////////
    if (pChildWindow != nullptr) {
        pChildWindow->InvalidateChildWindow();
        return true;
    }
    return false;
}

bool MainForm::PaintNextChildWindow(ui::ChildWindow* pChildWindow)
{
    if (pChildWindow == nullptr) {
        return false;
    }
    if (m_childWindowEvents.empty()) {
        return false;
    }
    size_t nStartItemIndex = 0;
    const size_t nItemCount = m_childWindowEvents.size();
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        MyChildWindowEvents* pChildWindowEvents = m_childWindowEvents[nItemIndex];
        if ((pChildWindowEvents != nullptr) && (pChildWindowEvents->GetChildWindow() == pChildWindow)) {
            nStartItemIndex = nItemIndex;
            break;
        }
    }
    for (size_t nItemIndex = nStartItemIndex + 1; nItemIndex < nItemCount; ++nItemIndex) {
        MyChildWindowEvents* pChildWindowEvents = m_childWindowEvents[nItemIndex];
        if ((pChildWindowEvents != nullptr) &&
            (pChildWindowEvents->GetChildWindow() != nullptr) &&
            pChildWindowEvents->GetChildWindow()->IsVisible() &&
            pChildWindowEvents->IsPaintFps()) {
            //确定绘制该窗口
            PaintChildWindow(pChildWindowEvents->GetChildWindow());
            return true;
        }
    }
    if (nStartItemIndex >= m_childWindowEvents.size()) {
        nStartItemIndex = m_childWindowEvents.size() - 1;
    }
    for (size_t nItemIndex = 0; nItemIndex <= nStartItemIndex; ++nItemIndex) {
        MyChildWindowEvents* pChildWindowEvents = m_childWindowEvents[nItemIndex];
        if ((pChildWindowEvents != nullptr) &&
            (pChildWindowEvents->GetChildWindow() != nullptr) &&
            pChildWindowEvents->GetChildWindow()->IsVisible() &&
            pChildWindowEvents->IsPaintFps()) {
            //确定绘制该窗口
            PaintChildWindow(pChildWindowEvents->GetChildWindow());
            return true;
        }
    }
    return PaintChildWindow(pChildWindow);
}
