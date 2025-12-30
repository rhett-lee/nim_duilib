#include "FullscreenBox.h"
#include "duilib/Core/Window.h"
#include "duilib/Control/Button.h"

namespace ui 
{
FullscreenBox::FullscreenBox(Window* pWindow) :
    Box(pWindow),
    m_nOldItemIndex(0),
    m_bWindowMaximized(false),
    m_bWindowFullscreen(false)
{
    //关闭控件自身的内边距
    SetEnableControlPadding(false);

    //背景色默认为白色(如果不设置背景色，在某些情况下会出现窗口透明的现象，比如WebView2控件的网页全屏时)
    SetBkColor(_T("white"));

    //保存窗口原来的状态
    if (pWindow != nullptr) {
        m_bWindowMaximized = pWindow->IsWindowMaximized();
        m_bWindowFullscreen = pWindow->IsWindowFullscreen();
    }
}

FullscreenBox::~FullscreenBox()
{
    //从全屏直接退出窗口时，需要手动释放原来的root
    Box* pOldRoot = m_pOldRoot.get();
    if (pOldRoot != nullptr) {
        delete pOldRoot;
        pOldRoot = nullptr;
    }
}

DString FullscreenBox::GetType() const { return _T("FullscreenBox"); }

bool FullscreenBox::EnterControlFullscreen(Box* pOldRoot, Control* pFullscreenControl, const DString& exitButtonClass)
{
    ASSERT((pOldRoot != nullptr) && (pFullscreenControl != nullptr));
    if ((pOldRoot == nullptr) || (pFullscreenControl == nullptr)) {
        return false;
    }

    ASSERT(m_pFullscreenControl == nullptr);
    if (m_pFullscreenControl != nullptr) {
        return false;
    }
    ASSERT(m_pOldParent == nullptr);
    if (m_pOldParent != nullptr) {
        return false;
    }
    ASSERT(m_pOldRoot == nullptr);
    if (m_pOldRoot != nullptr) {
        return false;
    }

    //将控件从原来的容器中提取出来
    RemoveControlFromBox(pFullscreenControl);

    //原来的控件隐藏
    pOldRoot->SetVisible(false);
    m_pOldRoot = pOldRoot;

    //添加到全屏容器中
    AddItem(pFullscreenControl);
    pFullscreenControl->SetVisible(true);

    //设置"退出全屏"按钮
    UpdateExitFullscreenBtn(exitButtonClass);
    return true;
}

bool FullscreenBox::UpdateControlFullscreen(Control* pFullscreenControl, const DString& exitButtonClass)
{
    if (pFullscreenControl == nullptr) {
        return false;
    }
    //还原旧的全屏控件
    RestoreControlToBox();

    //将控件从原来的容器中提取出来
    RemoveControlFromBox(pFullscreenControl);

    //添加到全屏容器中
    AddItem(pFullscreenControl);
    pFullscreenControl->SetVisible(true);

    //设置"退出全屏"按钮
    UpdateExitFullscreenBtn(exitButtonClass);
    return true;
}

void FullscreenBox::ExitControlFullscreen()
{
    //还原旧的全屏控件
    RestoreControlToBox();

    m_pOldRoot.reset();
    m_pExitFullscreenBtn.reset();
    m_exitButtonClass.clear();
}

void FullscreenBox::RemoveControlFromBox(Control* pFullscreenControl)
{
    if (pFullscreenControl == nullptr) {
        return;
    }
    m_pOldParent = pFullscreenControl->GetParent();
    if (m_pOldParent != nullptr) {
        //记录在父容器中的索引号
        m_nOldItemIndex = m_pOldParent->GetItemIndex(pFullscreenControl);

        //从原来的父容器中移除
        bool bOldAutoDestroyChild = m_pOldParent->IsAutoDestroyChild();
        m_pOldParent->SetAutoDestroyChild(false);
        m_pOldParent->Box::RemoveItem(pFullscreenControl);
        m_pOldParent->SetAutoDestroyChild(bOldAutoDestroyChild);

        //保存外边距
        m_rcOldMargin = pFullscreenControl->GetMargin();
    }
    m_pFullscreenControl = pFullscreenControl;
}

void FullscreenBox::RestoreControlToBox()
{
    if ((m_pOldParent != nullptr) && (m_pFullscreenControl != nullptr)) {
        SetAutoDestroyChild(false);
        RemoveItem(m_pFullscreenControl.get());
        SetAutoDestroyChild(true);

        //恢复原来的外边距
        m_pFullscreenControl->SetMargin(m_rcOldMargin, false);

        //恢复到原容器中
        m_pOldParent->Box::AddItemAt(m_pFullscreenControl.get(), m_nOldItemIndex);
    }
    m_pFullscreenControl.reset();
    m_pOldParent.reset();
    m_nOldItemIndex = 0;
    m_rcOldMargin.Clear();
}

void FullscreenBox::UpdateExitFullscreenBtn(const DString& exitButtonClass)
{
    if (m_exitButtonClass == exitButtonClass) {
        if (m_exitButtonClass.empty()) {
            if (m_pExitFullscreenBtn != nullptr) {
                RemoveItem(m_pExitFullscreenBtn.get());
                m_pExitFullscreenBtn.reset();
            }
            return;
        }
        else {
            if (m_pExitFullscreenBtn != nullptr) {
                return;
            }
        }
    }
    if (m_pExitFullscreenBtn != nullptr) {
        RemoveItem(m_pExitFullscreenBtn.get());
        m_pExitFullscreenBtn.reset();
    }

    m_exitButtonClass = exitButtonClass;
    if (!exitButtonClass.empty()) {
        Button* pButton = new Button(GetWindow());
        m_pExitFullscreenBtn = pButton;
        AddItem(pButton);
        pButton->SetClass(exitButtonClass);

        pButton->AttachClick([this](const EventArgs&) {
            //退出全屏
            if (GetWindow()) {
                GetWindow()->ExitControlFullscreen();
            }
            return true;
            });
    }
}

void FullscreenBox::ProcessFullscreenButtonMouseMove(const UiPoint& pt)
{
    if (m_pExitFullscreenBtn != nullptr) {
        Control* pExitFullscreenBtn = m_pExitFullscreenBtn.get();
        if (pExitFullscreenBtn->GetRect().ContainsPt(pt)) {
            pExitFullscreenBtn->SetFadeVisible(true);
        }
        else if (pExitFullscreenBtn->GetAlpha() > 0) {
            pExitFullscreenBtn->SetFadeVisible(false);
        }
    }
}

Control* FullscreenBox::GetFullscreenControl() const
{
    return m_pFullscreenControl.get();
}

Box* FullscreenBox::GetOldRoot() const
{
    return m_pOldRoot.get();
}

bool FullscreenBox::IsWindowOldMaximized() const
{
    return m_bWindowMaximized;
}

bool FullscreenBox::IsWindowOldFullscreen() const
{
    return m_bWindowFullscreen;
}

} // namespace ui
