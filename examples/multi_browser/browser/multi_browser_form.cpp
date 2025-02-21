#include "multi_browser_form.h"
#include "browser/browser_box.h"
#include "browser/multi_browser_manager.h"
#include "control/browser_tab_item.h"
#include "custom_layout.h"
#include <chrono>

using namespace ui;
using namespace std;
using namespace ui;

namespace
{
    // 注册这个消息，收到这个消息后表示窗口对应的任务栏按钮被系统创建，这时候初始化ITaskbarList4接口
    UINT WM_TASKBARBUTTONCREATED = ::RegisterWindowMessage(TEXT("TaskbarButtonCreated"));

    // 窗口收到WM_CLOSE消息的原因
    enum CloseReason
    {
        kDefaultClose    = 10,  // 在任务栏右击关闭窗口，按Alt+F4等常规原因
        kBrowserBoxClose = 11   // 关闭了最后一个浏览器盒子导致窗口关闭
    };
}

MultiBrowserForm::MultiBrowserForm()
{
    lbl_title_ = nullptr;
    btn_max_restore_ = nullptr;
    edit_url_ = nullptr;
    tab_list_ = nullptr;
    borwser_box_tab_ = nullptr;
    active_browser_box_ = nullptr;
    drop_helper_ = nullptr;
    is_drag_state_ = false;
    old_drag_point_ = {0, 0}; 

}

MultiBrowserForm::~MultiBrowserForm()
{
    lbl_title_ = nullptr;
    btn_max_restore_ = nullptr;
    edit_url_ = nullptr;
    tab_list_ = nullptr;
    borwser_box_tab_ = nullptr;
    active_browser_box_ = nullptr;
    drop_helper_ = nullptr;
}

DString MultiBrowserForm::GetSkinFolder()
{
    return _T("multi_browser");
}

DString MultiBrowserForm::GetSkinFile()
{
    return _T("multi_browser.xml");
}

ui::Control* MultiBrowserForm::CreateControl(const DString& pstrClass)
{
    if (pstrClass == _T("CustomTabBox")) {
        if (IsLayeredWindow()) {
            return new TabBox(this, new CustomLayout);
        }
        else {
            return new TabBox(this, new Layout);
        }
    }
    return nullptr;
}

void MultiBrowserForm::OnInitWindow()
{
    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MultiBrowserForm::OnClicked, this, std::placeholders::_1));
    btn_max_restore_ = static_cast<Button*>(FindControl(_T("btn_max_restore")));

    edit_url_ = static_cast<RichEdit*>(FindControl(_T("edit_url")));
    lbl_title_ = static_cast<Label*>(FindControl(_T("title")));
    edit_url_->AttachReturn(UiBind(&MultiBrowserForm::OnReturn, this, std::placeholders::_1));

    tab_list_ = static_cast<ListBox*>(FindControl(_T("tab_list")));
    borwser_box_tab_ = static_cast<TabBox*>(FindControl(_T("browser_box_tab")));

    tab_list_->AttachSelect(UiBind(&MultiBrowserForm::OnTabItemSelected, this, std::placeholders::_1));

    InitDragDrop();
}

void MultiBrowserForm::OnCloseWindow()
{
    // 使用tab_list_来判断浏览器盒子总数，browser_box_tab_获取的总数不准确
    int browser_box_count = GetBoxCount();
    for (int i = 0; i < browser_box_count; i++)
    {
        Control* box_item = borwser_box_tab_->GetItemAt(i);
        ASSERT(NULL != box_item);
        if (NULL == box_item)
            continue;;

        BrowserBox* browser_box = dynamic_cast<BrowserBox*>(box_item);
        if (NULL != browser_box)
            browser_box->UninitBrowserBox();
    }

    UnInitDragDrop();
}

LRESULT MultiBrowserForm::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    bHandled = true;
    if (uMsg == WM_SIZE)
    {
        if (wParam == SIZE_RESTORED)
            OnWndSizeMax(false);
        else if (wParam == SIZE_MAXIMIZED)
            OnWndSizeMax(true);
    }
    else if (uMsg == WM_KEYDOWN)
    {
        // 处理Ctrl+Tab快捷键
        if (wParam == VK_TAB && ::GetKeyState(VK_CONTROL) < 0)
        {
            int next = (int)tab_list_->GetCurSel();
            next = (next + 1) % GetBoxCount();
            tab_list_->SelectItem(next, true, true);
            return 0;
        }
        // 处理ESC快捷键
        else if (wParam == VK_ESCAPE)
        {
            BOOL bHandled = FALSE;
            if (!MultiBrowserManager::GetInstance()->IsDragingBorwserBox() && NULL != active_browser_box_)
            {
                this->CloseBox(active_browser_box_->GetId());
            }

            return 0;
        }
    }
    else if (uMsg == WM_TASKBARBUTTONCREATED)
    {
        taskbar_manager_.Init(this);

        // 因为窗口刚创建时，浏览器盒子已经创建但是那时还没有收到WM_TASKBARBUTTONCREATED消息，导致RegisterTab函数没有被调用，所以收到消息后重新遍历一下没有被注册的Tab
        for (size_t i = 0; i < borwser_box_tab_->GetItemCount(); ++i) {
            Control *box_item = borwser_box_tab_->GetItemAt(i);
            ASSERT(nullptr != box_item);
            if (nullptr == box_item) {
                continue;
            }

            BrowserBox* browser_box = dynamic_cast<BrowserBox*>(box_item);
            if (nullptr == browser_box) {
                continue;
            }

            TaskbarTabItem* taskbar_item = browser_box->GetTaskbarItem();
            if (nullptr == taskbar_item) {
                continue;
            }
            taskbar_manager_.RegisterTab(*taskbar_item);
        }

        return TRUE;
    }

    return BaseClass::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

LRESULT MultiBrowserForm::OnWindowCloseMsg(uint32_t wParam, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    int browser_count = GetBoxCount();
    if (browser_count > 0 && nullptr != active_browser_box_)
    {
        // 如果只有一个浏览器盒子，就直接关闭
        if (1 == browser_count)    {
            CloseBox(active_browser_box_->GetId());
        }        
        else {
            // 如果包含多个浏览器盒子
            while (GetBoxCount() > 0) {
                Control* tab_item = tab_list_->GetItemAt(0);
                ASSERT(nullptr != tab_item);
                if (nullptr == tab_item) {
                    break;
                }
                CloseBox(tab_item->GetUTF8Name());
            }
        }
    }
    return BaseClass::OnWindowCloseMsg(wParam, nativeMsg, bHandled);
}

void MultiBrowserForm::OnWndSizeMax(bool max)
{
    if (btn_max_restore_)
        btn_max_restore_->SetClass(max ? _T("btn_wnd_restore") : _T("btn_wnd_max"));
}

bool MultiBrowserForm::OnClicked(const ui::EventArgs& arg )
{
    DString name = arg.GetSender()->GetName();
    if (name == _T("btn_max_restore"))
    {
        if (IsWindowMaximized()) {
            ShowWindow(ui::kSW_RESTORE);
        }
        else {
            ShowWindow(ui::kSW_SHOW_MAXIMIZED);
        }
    }
    else if (name == _T("btn_close"))
    {
        if (NULL == active_browser_box_)
        {
            ASSERT(0);
            return true;
        }

        CloseBox(active_browser_box_->GetId());
    }
    else if (name == _T("btn_min"))
    {
        ShowWindow(ui::kSW_MINIMIZE);
    }
    else if (name == _T("btn_add"))
    {
        uint64_t nTimeMS = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
        std::string timeStamp = ui::StringUtil::Printf("%I64u", nTimeMS);
        MultiBrowserManager::GetInstance()->CreateBorwserBox(this, timeStamp, _T(""));
    }
    else if (active_browser_box_)
    {
        auto cef_control = active_browser_box_->GetCefControl();
        if (!cef_control)
            return true;

        if (name == _T("btn_back"))
        {
            cef_control->GoBack();
        }
        else if (name == _T("btn_forward"))
        {
            cef_control->GoForward();
        }
        else if (name == _T("btn_refresh"))
        {
            cef_control->Refresh();
        }
    }

    return true;
}

bool MultiBrowserForm::OnReturn(const ui::EventArgs& arg)
{
    DString name = arg.GetSender()->GetName();
    if (name == _T("edit_url"))
    {
#if 0
         // 在当前页面跳转
         auto cef_control = active_browser_box_->GetCefControl();
         if (cef_control)
             cef_control->LoadURL(edit_url_->GetText());
#endif
         uint64_t nTimeMS = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
        std::string timeStamp = ui::StringUtil::Printf("%I64u", nTimeMS);
        // 新建标签页
        MultiBrowserManager::GetInstance()->CreateBorwserBox(this, timeStamp, edit_url_->GetText());
    }

    return false;
}

BrowserBox* MultiBrowserForm::CreateBox(const std::string &browser_id, DString url)
{
    DString id = ui::StringConvert::UTF8ToT(browser_id);
    if (NULL != FindTabItem(id))
    {
        ASSERT(0);
        return NULL;
    }
    if (NULL != FindBox(id))
    {
        ASSERT(0);
        return NULL;
    }

    BrowserTabItem *tab_item = new BrowserTabItem(tab_list_->GetWindow());
    GlobalManager::Instance().FillBoxWithCache(tab_item, ui::FilePath(_T("multi_browser/tab_item.xml")));
    tab_list_->AddItemAt(tab_item, GetBoxCount());
    tab_item->AttachAllEvents(UiBind(&MultiBrowserForm::OnProcessTabItemDrag, this, std::placeholders::_1));
    tab_item->AttachButtonDown(UiBind(&MultiBrowserForm::OnTabItemSelected, this, std::placeholders::_1));
    tab_item->InitControl(browser_id);
    Button *btn_item_close = (Button*)tab_item->FindSubControl(_T("tab_item_close"));
    btn_item_close->AttachClick(UiBind(&MultiBrowserForm::OnTabItemClose, this, std::placeholders::_1, browser_id));

    BrowserBox* browser_box = new BrowserBox(borwser_box_tab_->GetWindow(), browser_id);
    borwser_box_tab_->AddItem(browser_box);
    GlobalManager::Instance().FillBoxWithCache(browser_box, ui::FilePath(_T("multi_browser/browser_box.xml")), UiBind(&BrowserBox::CreateControl, browser_box, std::placeholders::_1));
    browser_box->SetName(id);
    browser_box->InitBrowserBox(url);
    auto taskbar_item = browser_box->GetTaskbarItem();
    if (taskbar_item)
        taskbar_manager_.RegisterTab(*taskbar_item);

    if (GetBoxCount() <= 1)
        active_browser_box_ = browser_box;

    // 切换到新的浏览器盒子
    // 如果tab_item处于隐藏状态，则无法顺利触发选择事件，所以这里直接切换到目标浏览器盒子
    tab_item->Selected(true, false);
    ChangeToBox(id);

    return browser_box;
}

void MultiBrowserForm::CloseBox(const std::string &browser_id)
{
    if (browser_id.empty()) {
        return;
    }

    DString id = ui::StringConvert::UTF8ToT(browser_id);

    // 从左侧会话列表项移除对应item
    BrowserTabItem *tab_item = FindTabItem(id);
    if (nullptr != tab_item) {
        tab_list_->RemoveItem(tab_item);
    }

    // 在浏览器列表中找到浏览器盒子并且移除盒子
    BrowserBox *browser_box = FindBox(id);
    ASSERT(nullptr != browser_box);
    if (nullptr != browser_box) {
        auto taskbar_item = browser_box->GetTaskbarItem();
        if (taskbar_item) {
            taskbar_manager_.UnregisterTab(*taskbar_item);
        }
        browser_box->UninitBrowserBox();
        // 如果浏览器盒子的数量大于1就立马移除盒子，否则不移除
        // 如果最后一个浏览器盒子在这里立马移除，在窗口关闭时界面会因为没有控件而变成黑色
        // 窗口关闭时，会自动的移除这个浏览器盒子
        if (borwser_box_tab_->GetItemCount() > 1) {
            borwser_box_tab_->RemoveItem(browser_box);
            if (active_browser_box_ == browser_box) {
                active_browser_box_ = nullptr;
                size_t nSelItem = borwser_box_tab_->GetCurSel();
                if (nSelItem != Box::InvalidIndex) {
                    active_browser_box_ = dynamic_cast<BrowserBox*>(borwser_box_tab_->GetItemAt(nSelItem));
                }                
            }
        }
        else {
            active_browser_box_ = nullptr;
        }
    }

    // 当浏览器盒子清空时，关闭浏览器窗口
    if (GetBoxCount() == 0) {
        this->CloseWnd(kBrowserBoxClose);
    }
}

bool MultiBrowserForm::AttachBox(BrowserBox *browser_box)
{
    if (NULL == browser_box)
        return false;

    DString id = ui::StringConvert::UTF8ToT(browser_box->GetId());
    if (NULL != FindTabItem(id))
    {
        ASSERT(0);
        return false;
    }
    if (NULL != FindBox(id))
    {
        ASSERT(0);
        return false;
    }

    BrowserTabItem *tab_item = new BrowserTabItem(tab_list_->GetWindow());
    GlobalManager::Instance().FillBoxWithCache(tab_item, ui::FilePath(_T("multi_browser/tab_item.xml")));
    tab_list_->AddItemAt(tab_item, GetBoxCount());
    tab_item->AttachAllEvents(UiBind(&MultiBrowserForm::OnProcessTabItemDrag, this, std::placeholders::_1));
    tab_item->AttachButtonDown(UiBind(&MultiBrowserForm::OnTabItemSelected, this, std::placeholders::_1));
    tab_item->InitControl(browser_box->GetId());
    tab_item->SetTitle(browser_box->GetTitle());
    Button *btn_item_close = (Button*)tab_item->FindSubControl(_T("tab_item_close"));
    btn_item_close->AttachClick(UiBind(&MultiBrowserForm::OnTabItemClose, this, std::placeholders::_1, browser_box->GetId()));

    // 当另一个窗体创建的browser_box浏览器盒子控件添加到另一个窗体内的容器控件时
    // AddItem函数会重新的修改browser_box内所有子控件的m_pWindow为新的窗体指针
    borwser_box_tab_->AddItem(browser_box);
    auto taskbar_item = browser_box->GetTaskbarItem();
    if (taskbar_item)
        taskbar_manager_.RegisterTab(*taskbar_item);

    if (GetBoxCount() <= 1)
        active_browser_box_ = browser_box;

    // 切换到新的浏览器盒子
    // 如果tab_item处于隐藏状态，则无法顺利触发选择事件，所以这里直接切换到目标浏览器盒子
    tab_item->Selected(true, false);
    ChangeToBox(id);

    return true;
}

bool MultiBrowserForm::DetachBox(BrowserBox *browser_box)
{
    if (NULL == browser_box)
        return false;

    DString id = ui::StringConvert::UTF8ToT(browser_box->GetId());

    // 从顶部标签页移除对应item
    BrowserTabItem *tab_item = FindTabItem(id);
    if (NULL == tab_item)
        return false;

    tab_list_->RemoveItem(tab_item);

    auto taskbar_item = browser_box->GetTaskbarItem();
    if (taskbar_item)
        taskbar_manager_.UnregisterTab(*taskbar_item);
    // 在右侧Tab浏览器盒子列表中找到浏览器盒子并且移除盒子
    // 在这里不能delete browser_box
    bool auto_destroy = borwser_box_tab_->IsAutoDestroyChild();
    borwser_box_tab_->SetAutoDestroyChild(false);
    if (!borwser_box_tab_->RemoveItem(browser_box))
    {
        borwser_box_tab_->SetAutoDestroyChild(auto_destroy);
        return false;
    }
    borwser_box_tab_->SetAutoDestroyChild(auto_destroy);
    if (active_browser_box_ == browser_box) {
        active_browser_box_ = nullptr;
    }

    // 当浏览器盒子清空时，关闭浏览器窗口
    if (GetBoxCount() == 0)
    {
        this->CloseWnd(kBrowserBoxClose);
    }

    return true;
}

BrowserBox* MultiBrowserForm::GetSelectedBox()
{
    return active_browser_box_;
}

void MultiBrowserForm::SetActiveBox(const std::string &browser_id)
{
    if (browser_id.empty())
        return;

    if (IsWindowMinimized()) {
        ShowWindow(kSW_RESTORE);
    }
    else {
        ShowWindow(kSW_SHOW);
    }

    // 从窗口左侧会话列表找到要激活的浏览器盒子项
    DString id = ui::StringConvert::UTF8ToT(browser_id);
    BrowserTabItem *tab_item = FindTabItem(id);
    if (NULL == tab_item)
        return;

    // 如果tab_item处于隐藏状态，则无法顺利触发选择事件，所以这里直接切换到目标浏览器盒子
    tab_item->Selected(true, false);
    ChangeToBox(id);
}

bool MultiBrowserForm::IsActiveBox(const BrowserBox *browser_box)
{
    ASSERT(NULL != browser_box);
    return (browser_box == active_browser_box_ && IsWindowForeground() && !IsWindowMinimized() && IsWindowVisible());
}

bool MultiBrowserForm::IsActiveBox(const DString &browser_id)
{
    ASSERT(!browser_id.empty());
    return (IsWindowForeground() && !IsWindowMinimized() && IsWindowVisible() && FindBox(browser_id) == active_browser_box_);
}

int MultiBrowserForm::GetBoxCount() const
{
    int nBoxCount = (int)tab_list_->GetItemCount();
    if (nBoxCount > 0) {
        nBoxCount -= 1; // 减去右边那个加号的数量
    }
    return nBoxCount;
}

void MultiBrowserForm::OnBeforeDragBoxCallback(const DString &browser_id)
{
    // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，则在拖拽时隐藏浏览器窗口
    int box_count = this->GetBoxCount();
    if (1 == box_count)
    {
        this->ShowWindow(kSW_HIDE);
    }
    // 否则隐藏被拖拽的浏览器盒子和标签
    else
    {
        BrowserBox *browser_box = FindBox(browser_id);
        if (NULL != browser_box)
            browser_box->SetFadeVisible(false);

        BrowserTabItem *tab_item = FindTabItem(browser_id);
        if (NULL != tab_item)
            tab_item->SetFadeVisible(false);

        // 找到新的被显示的浏览器盒子
        int index = tab_item->GetListBoxIndex();
        if (index > 0)
            index--;
        else
            index++;
        BrowserTabItem *new_tab_item = static_cast<BrowserTabItem*>(tab_list_->GetItemAt(index));
        if (NULL != new_tab_item)
        {
            new_tab_item->Selected(true, false);
            ChangeToBox(new_tab_item->GetName());
        }

        draging_browser_id_ = browser_id;
    }
}

void MultiBrowserForm::OnAfterDragBoxCallback(bool drop_succeed)
{
    is_drag_state_ = false;

    if (drop_succeed)
    {
        int box_count = this->GetBoxCount();
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，并且拖拽到新的浏览器窗口里，这个浏览器窗口就会关闭
        if (1 == box_count)
        {

        }
        else
        {

        }
    }
    else
    {
        int box_count = this->GetBoxCount();
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，并且没有拖拽到新的浏览器窗口里
        // 就显示浏览器窗口
        if (1 == box_count)
        {
            this->ShowWindow(kSW_SHOW_NORMAL);
        }
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口有多个浏览器盒子，并且没有拖拽到新的浏览器窗口里
        // 就显示之前被隐藏的浏览器盒子和标签
        else
        {
            BrowserBox *browser_box = FindBox(draging_browser_id_);
            if (NULL != browser_box)
                browser_box->SetFadeVisible(true);

            BrowserTabItem *tab_item = FindTabItem(draging_browser_id_);
            if (NULL != tab_item)
            {
                tab_item->SetFadeVisible(true);
                tab_item->Selected(true, false);
                ChangeToBox(draging_browser_id_);
            }

            draging_browser_id_.clear();
        }
    }
}

bool MultiBrowserForm::OnTabItemSelected(const ui::EventArgs& param)
{
    if (kEventSelect == param.eventType)
    {
        DString name = param.GetSender()->GetName();

        if (name == _T("tab_list"))
        {
            // 如果单击了顶部的标签，则找到下方Tab里对应的浏览器盒子并选中
            Control *select_item = tab_list_->GetItemAt(tab_list_->GetCurSel());
            ASSERT(NULL != select_item);
            if (NULL == select_item)
                return true;

            DString session_id = select_item->GetName();
            ChangeToBox(session_id);
        }
    }
    else if (kEventMouseButtonDown == param.eventType)
    {
        BrowserTabItem *tab_item = dynamic_cast<BrowserTabItem*>(param.GetSender());
        if (tab_item)
        {
            DString browser_id = tab_item->GetName();
            ChangeToBox(browser_id);
        }
    }
    return false;
}

bool MultiBrowserForm::OnTabItemClose(const ui::EventArgs& param, const std::string& browser_id)
{
    if (param.GetSender()->GetName() == _T("tab_item_close"))
    {
        CloseBox(browser_id);
    }

    return false;
}

BrowserBox* MultiBrowserForm::FindBox(const DString &browser_id)
{
    for (int i = 0; i < (int)borwser_box_tab_->GetItemCount(); i++)    {
        Control *box_item = borwser_box_tab_->GetItemAt(i);
        ASSERT(nullptr != box_item);
        if (nullptr == box_item) {
            return nullptr;
        }

        if (box_item->GetName() == browser_id) {
            return dynamic_cast<BrowserBox*>(box_item);
        }
    }
    return nullptr;
}

BrowserTabItem* MultiBrowserForm::FindTabItem(const DString &browser_id)
{
    for (int i = 0; i < GetBoxCount(); i++)
    {
        Control *tab_item = tab_list_->GetItemAt(i);
        ASSERT(NULL != tab_item);
        if (NULL == tab_item)
            return NULL;

        if (tab_item->GetName() == browser_id)
            return static_cast<BrowserTabItem*>(tab_item);
    }

    return NULL;
}

void MultiBrowserForm::SetTabItemName(const DString &browser_id, const DString &name)
{
    BrowserTabItem *tab_item = FindTabItem(browser_id);
    if (NULL != tab_item)
    {
        tab_item->SetTitle(name);
    }
}

void MultiBrowserForm::SetURL(const std::string &browser_id, const DString &url)
{
    if (active_browser_box_ && active_browser_box_->GetId() == browser_id)
        edit_url_->SetText(url);
}

bool MultiBrowserForm::ChangeToBox(const DString &browser_id)
{
    if (browser_id.empty())
        return false;

    BrowserBox *box_item = FindBox(browser_id);
    if (NULL == box_item)
        return false;

    borwser_box_tab_->SelectItem(box_item);

    box_item->SetFocus();
    active_browser_box_ = box_item;
    edit_url_->SetText(active_browser_box_->GetCefControl()->GetURL().ToWString());
    // 根据当前激活的浏览器盒子，更新任务栏的图标和标题
    return true;
}
