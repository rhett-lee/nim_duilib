#include "MainForm.h"
#include "ChildWindowPaint.h"
#include "MyChildWindowEvents.h"

MainForm::MainForm():
    m_pChildWindow(nullptr)
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
    UpdateChildWindowsLayered();
}

void MainForm::OnWindowShadowTypeChanged()
{
    BaseClass::OnWindowShadowTypeChanged();
    //阴影类型变化后，子窗口的分层窗口属性可能需要同步调整
    UpdateChildWindowsLayered();
}

bool MainForm::IsChildWindowNeedLayered() const
{
    if (IsLayeredWindow()) {
        //父窗口是分层窗口时，子窗口也必须带有分层窗口属性，否则绘制流程有问题
        return true;
    }
    //父窗口使用系统阴影时，客户区为 DWM 合成区域，普通GDI子窗口无法正常显示，需要分层窗口属性
    return IsShadowAttached() && ui::Shadow::IsSystemShadowType(GetShadowType());
}

void MainForm::UpdateChildWindowsLayered()
{
    if (m_childWindowEvents.empty()) {
        //尚未创建子窗口（此时可能窗口的阴影属性还未初始化），无需同步
        return;
    }
    const bool bLayered = IsChildWindowNeedLayered();
    for (MyChildWindowEvents* pChildWindowEvents : m_childWindowEvents) {
        if (pChildWindowEvents != nullptr) {
            ui::ChildWindow* pChildWindow = pChildWindowEvents->GetChildWindow();
            if (pChildWindow != nullptr) {
                pChildWindow->SetChildWindowLayered(bLayered);
            }
        }
    }
}

void MainForm::CreateChildWindows()
{
    if (!m_childWindowEvents.empty()) {
        //已经创建过，避免重复创建
        return;
    }
    ui::GridBox* pChildWindowBox = dynamic_cast<ui::GridBox*>(FindControl(_T("child_window_box")));
    if (pChildWindowBox == nullptr) {
        return;
    }
    size_t nCount = pChildWindowBox->GetItemCount();
    for (size_t nItem = 0; nItem < nCount; ++nItem) {
        ui::ChildWindow* pChildWindow = dynamic_cast<ui::ChildWindow*>(pChildWindowBox->GetItemAt(nItem));
        if (pChildWindow == nullptr) {
            continue;
        }
        MyChildWindowEvents* pMyChildWindowEvents = new MyChildWindowEvents(pChildWindow, nItem, this);
        if (!pChildWindow->CreateChildWindow(pMyChildWindowEvents)) {
            //创建失败时，释放事件对象，不保存关联关系
            delete pMyChildWindowEvents;
            continue;
        }
        m_childWindowEvents.push_back(pMyChildWindowEvents);
        //子窗口的分层窗口属性需要与主窗口的阴影/分层属性保持一致
        pChildWindow->SetChildWindowLayered(IsChildWindowNeedLayered());
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
        }
    }
    //子窗口已经销毁，避免消息循环中继续引用
    m_pChildWindow = nullptr;
}

bool MainForm::PaintChildWindow(ui::ChildWindow* pChildWindow)
{
    if (pChildWindow != nullptr) {
        pChildWindow->InvalidateChildWindow();
        return true;
    }
    return false;
}

bool MainForm::PaintNextChildWindow(ui::ChildWindow* pChildWindow)
{
    //需要在idle函数中触发连续绘制，否则界面会卡死
    m_pChildWindow = pChildWindow;
    return true;
}

bool MainForm::PaintNextChildWindow()
{
    return DoPaintNextChildWindow(m_pChildWindow);
}

bool MainForm::DoPaintNextChildWindow(ui::ChildWindow * pChildWindow)
{
    if ((pChildWindow == nullptr) || m_childWindowEvents.empty()) {
        return false;
    }
    //默认从最后一个开始，保证未找到目标窗口时，可以回绕并覆盖所有窗口
    size_t nStartItemIndex = m_childWindowEvents.size() - 1;
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
            return PaintChildWindow(pChildWindowEvents->GetChildWindow());
        }
    }
    for (size_t nItemIndex = 0; nItemIndex <= nStartItemIndex; ++nItemIndex) {
        MyChildWindowEvents* pChildWindowEvents = m_childWindowEvents[nItemIndex];
        if ((pChildWindowEvents != nullptr) &&
            (pChildWindowEvents->GetChildWindow() != nullptr) &&
            pChildWindowEvents->GetChildWindow()->IsVisible() &&
            pChildWindowEvents->IsPaintFps()) {
            //确定绘制该窗口
            return PaintChildWindow(pChildWindowEvents->GetChildWindow());
        }
    }
    //所有窗口都未处于动态绘制状态，停止刷新
    return false;
}
