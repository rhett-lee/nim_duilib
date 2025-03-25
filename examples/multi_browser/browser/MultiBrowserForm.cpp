#include "MultiBrowserForm.h"
#include "browser/BrowserBox.h"
#include "browser/MultiBrowserManager.h"
#include "CustomLayout.h"
#include <chrono>

using namespace ui;
using namespace std;

namespace
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    // 注册这个消息，收到这个消息后表示窗口对应的任务栏按钮被系统创建，这时候初始化ITaskbarList4接口
    UINT WM_TASKBARBUTTONCREATED = ::RegisterWindowMessage(TEXT("TaskbarButtonCreated"));
#endif

    // 窗口收到WM_CLOSE消息的原因
    enum CloseReason
    {
        kDefaultClose    = 10,  // 在任务栏右击关闭窗口，按Alt+F4等常规原因
        kBrowserBoxClose = 11   // 关闭了最后一个浏览器盒子导致窗口关闭
    };
}

MultiBrowserForm::MultiBrowserForm()
{
    m_pEditUrl = nullptr;
    m_pTabCtrl = nullptr;
    m_pBorwserBoxTab = nullptr;
    m_pActiveBrowserBox = nullptr;
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_pDropHelper = nullptr;
    m_bDragState = false;
    m_oldDragPoint = {0, 0};
#endif
}

MultiBrowserForm::~MultiBrowserForm()
{
    m_pEditUrl = nullptr;
    m_pTabCtrl = nullptr;
    m_pBorwserBoxTab = nullptr;
    m_pActiveBrowserBox = nullptr;

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_pDropHelper = nullptr;
#endif
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

/** 标题栏区域的布局管理
*/
class TitleBarHLayout : public HLayout
{
public:
    TitleBarHLayout() = default;
    virtual ~TitleBarHLayout() override = default;

public:
    /** 调整内部所有控件的位置信息
     * @param[in] items 控件列表
     * @param[in] rc 当前容器位置信息, 包含内边距，但不包含外边距
     * @return 返回排列后最终盒子的宽度和高度信息，包含Owner Box的内边距，不包含外边距
     */
    virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override
    {
        UiSize64 szSize = HLayout::ArrangeChild(items, rc);
        ASSERT(items.empty() || items.size() == 5);
        if (items.size() != 5) {
            return szSize;
        }
        //固定结构, 校验后修改标签栏的宽度
        int64_t nTotalWidth = 0;
        for (Control* pControl : items) {
            if (pControl == nullptr) {
                continue;
            }
            ui::UiMargin margin = pControl->GetMargin();
            nTotalWidth += (pControl->GetRect().Width() + margin.left + margin.right);
        }
        ASSERT(szSize.cx == nTotalWidth);
        if (szSize.cx != nTotalWidth) {
            return szSize;
        }
        ASSERT(rc.Width() == nTotalWidth);
        if (rc.Width() != nTotalWidth) {
            return szSize;
        }

        //校验标签栏的TabCtrl控件
        ui::TabCtrl* pTabCtrl = nullptr;
        ui::Control* pItem = items[2];
        if (pItem != nullptr) {
            pTabCtrl = dynamic_cast<ui::TabCtrl*>(pItem);            
        }
        ASSERT(pTabCtrl != nullptr);
        if (pTabCtrl == nullptr) {
            return szSize;
        }
        ASSERT(pTabCtrl->GetFixedWidth().IsStretch());
        if (!pTabCtrl->GetFixedWidth().IsStretch()) {
            return szSize;
        }

        //等于总宽度：计算标签控件的实际总宽度
        int32_t nTabItemTotalWidth = 0;
        UiRect rcTabCtrl = pItem->GetPos();
        const size_t nTabItemCount = pTabCtrl->GetItemCount();
        for (size_t nTabItem = 0; nTabItem < nTabItemCount; ++nTabItem) {
            Control* pTabItem = pTabCtrl->GetItemAt(nTabItem);
            if ((pTabItem == nullptr) || !pTabItem->IsVisible()) {
                continue;
            }
            ui::UiMargin rcTabItemMargin = pTabItem->GetMargin();
            nTabItemTotalWidth += (pTabItem->GetPos().Width() + rcTabItemMargin.left + rcTabItemMargin.right);
        }
        if (nTabItemTotalWidth < rcTabCtrl.Width()) {
            int32_t nItemDiff = rcTabCtrl.Width() - nTabItemTotalWidth;
            //标签控件：长度缩短
            rcTabCtrl.right -= nItemDiff;
            pTabCtrl->SetPos(rcTabCtrl);

            //新建按钮控件：向左移动
            Control* pItem = items[items.size() - 2];
            if (pItem != nullptr) {
                UiRect rcItem = pItem->GetPos();
                rcItem.Offset(-nItemDiff, 0);
                pItem->SetPos(rcItem);
            }
            //最后的控件：宽度增加
            pItem = items[items.size() - 1];
            if (pItem != nullptr) {
                UiRect rcItem = pItem->GetPos();
                rcItem.left -= nItemDiff;
                pItem->SetPos(rcItem);
            }
        }

        //最后再校验
        nTotalWidth = 0;
        for (Control* pControl : items) {
            if (pControl == nullptr) {
                continue;
            }
            ui::UiMargin margin = pControl->GetMargin();
            nTotalWidth += (pControl->GetRect().Width() + margin.left + margin.right);
        }
        ASSERT(rc.Width() == nTotalWidth);
        return szSize;
    }
};

void MultiBrowserForm::OnInitWindow()
{
    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MultiBrowserForm::OnClicked, this, std::placeholders::_1));

    m_pEditUrl = static_cast<RichEdit*>(FindControl(_T("edit_url")));
    m_pEditUrl->AttachReturn(UiBind(&MultiBrowserForm::OnReturn, this, std::placeholders::_1));

    //替换标题栏的布局
    HBox* pTitleBar = static_cast<HBox*>(FindControl(_T("title_bar")));
    if (pTitleBar != nullptr) {
        TitleBarHLayout* pNewLayout = new TitleBarHLayout;
        Layout* pOldLayout = pTitleBar->ResetLayout(pNewLayout);
        if (pOldLayout != nullptr) {
            //复制布局属性
            pNewLayout->SetChildMarginX(pOldLayout->GetChildMarginX());
            pNewLayout->SetChildMarginY(pOldLayout->GetChildMarginY());
            pTitleBar->FreeLayout(pOldLayout);
        }
    }

    m_pTabCtrl = static_cast<TabCtrl*>(FindControl(_T("tab_ctrl")));
    m_pBorwserBoxTab = static_cast<TabBox*>(FindControl(_T("browser_box_tab")));

    if (m_pTabCtrl != nullptr) {
        m_pTabCtrl->AttachSelect(UiBind(&MultiBrowserForm::OnTabItemSelected, this, std::placeholders::_1));
    }

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    InitDragDrop();
#endif

    //设置按钮的状态
    Control* pButton = FindControl(_T("btn_back"));
    if (pButton != nullptr) {
        pButton->SetEnabled(false);
    }
    pButton = FindControl(_T("btn_forward"));
    if (pButton != nullptr) {
        pButton->SetEnabled(false);
    }
    pButton = FindControl(_T("btn_refresh"));
    if (pButton != nullptr) {
        pButton->SetVisible(true);
    }
    pButton = FindControl(_T("btn_stop"));
    if (pButton != nullptr) {
        pButton->SetVisible(false);
    }

    //设置控制主进程单例的回调函数
    ui::CefManager::GetInstance()->SetAlreadyRunningAppRelaunch(UiBind(&MultiBrowserForm::OnAlreadyRunningAppRelaunch, this, std::placeholders::_1));
}

void MultiBrowserForm::OnCloseWindow()
{
    // 使用m_pTabCtrl来判断浏览器盒子总数，browser_box_tab_获取的总数不准确
    int browser_box_count = GetBoxCount();
    for (int i = 0; i < browser_box_count; i++) {
        Control* box_item = m_pBorwserBoxTab->GetItemAt(i);
        ASSERT(nullptr != box_item);
        if (nullptr == box_item) {
            continue;;
        }

        BrowserBox* browser_box = dynamic_cast<BrowserBox*>(box_item);
        if (nullptr != browser_box) {
            browser_box->UninitBrowserBox();
        }
    }
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    UnInitDragDrop();
#endif
}

void MultiBrowserForm::OnLoadingStateChange(BrowserBox* pBrowserBox)
{
    if (m_pActiveBrowserBox != pBrowserBox) {
        return;
    }
    ui::CefControl* pCefCcontrol = m_pActiveBrowserBox->GetCefControl();
    if ((pCefCcontrol == nullptr) || (pCefCcontrol->GetCefBrowser() == nullptr)) {
        return;
    }
    bool isLoading = pCefCcontrol->GetCefBrowser()->IsLoading();
    bool canGoBack = pCefCcontrol->GetCefBrowser()->CanGoBack();
    bool canGoForward = pCefCcontrol->GetCefBrowser()->CanGoForward();
    Control* pButton = FindControl(_T("btn_back"));
    if (pButton != nullptr) {
        pButton->SetEnabled(canGoBack);
    }
    pButton = FindControl(_T("btn_forward"));
    if (pButton != nullptr) {
        pButton->SetEnabled(canGoForward);
    }
    pButton = FindControl(_T("btn_refresh"));
    if (pButton != nullptr) {
        pButton->SetVisible(!isLoading);
    }
    pButton = FindControl(_T("btn_stop"));
    if (pButton != nullptr) {
        pButton->SetVisible(isLoading);
    }
}

LRESULT MultiBrowserForm::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (uMsg == WM_TASKBARBUTTONCREATED) {
        bHandled = true;
        m_taskbarManager.Init(this);

        // 因为窗口刚创建时，浏览器盒子已经创建但是那时还没有收到WM_TASKBARBUTTONCREATED消息，导致RegisterTab函数没有被调用，所以收到消息后重新遍历一下没有被注册的Tab
        for (size_t i = 0; i < m_pBorwserBoxTab->GetItemCount(); ++i) {
            Control *box_item = m_pBorwserBoxTab->GetItemAt(i);
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
            m_taskbarManager.RegisterTab(*taskbar_item);
        }

        return TRUE;
    }
#endif

    return BaseClass::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

LRESULT MultiBrowserForm::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if ((vkCode == VirtualKeyCode::kVK_TAB) && ui::Keyboard::IsKeyDown(VirtualKeyCode::kVK_CONTROL)) {
        // 处理Ctrl+Tab快捷键: 切换标签
        bHandled = true;
        size_t nNextItem = m_pTabCtrl->GetCurSel();
        nNextItem = (nNextItem + 1) % m_pTabCtrl->GetItemCount();
        m_pTabCtrl->SelectItem(nNextItem, true, true);
        return 0;
    }
    return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

LRESULT MultiBrowserForm::OnWindowCloseMsg(uint32_t wParam, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    int browser_count = GetBoxCount();
    if (browser_count > 0 && nullptr != m_pActiveBrowserBox) {
        // 如果只有一个浏览器盒子，就直接关闭
        if (1 == browser_count) {
            CloseBox(m_pActiveBrowserBox->GetId());
        }        
        else {
            // 如果包含多个浏览器盒子
            while (GetBoxCount() > 0) {
                Control* tab_item = m_pTabCtrl->GetItemAt(0);
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

bool MultiBrowserForm::OnClicked(const ui::EventArgs& arg )
{
    DString name = arg.GetSender()->GetName();
    if (name == _T("btn_close")) {
        if (m_pActiveBrowserBox != nullptr) {
            CloseBox(m_pActiveBrowserBox->GetId());
        }
    }
    else if (name == _T("btn_add")) {
        uint64_t nTimeMS = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
        std::string timeStamp = ui::StringUtil::UInt64ToStringA(nTimeMS);
        MultiBrowserManager::GetInstance()->CreateBorwserBox(this, timeStamp, _T(""));
    }
    else if (m_pActiveBrowserBox) {
        auto cef_control = m_pActiveBrowserBox->GetCefControl();
        if (!cef_control) {
            return true;
        }

        if (name == _T("btn_back")) {
            cef_control->GoBack();
        }
        else if (name == _T("btn_forward")) {
            cef_control->GoForward();
        }
        else if (name == _T("btn_refresh")) {
            cef_control->Refresh();
        }
        else if (name == _T("btn_stop")) {
            cef_control->StopLoad();
        }
    }

    return true;
}

bool MultiBrowserForm::OnReturn(const ui::EventArgs& arg)
{
    if (m_pEditUrl != nullptr) {
        //新建标签页
        DString url = m_pEditUrl->GetText();         
        if (!url.empty()) {
            CreateNewTabPage(url);
        }        
    }
    return true;
}

void MultiBrowserForm::CreateNewTabPage(const DString& url)
{
    if (!url.empty()) {
        uint64_t nTimeMS = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
        std::string timeStamp = ui::StringUtil::UInt64ToStringA(nTimeMS);        
        MultiBrowserManager::GetInstance()->CreateBorwserBox(this, timeStamp, url);
    }
}

void MultiBrowserForm::OnAlreadyRunningAppRelaunch(const std::vector<DString>& argumentList)
{
    if (ui::GlobalManager::Instance().IsInUIThread()) {
        //CEF 133版本会调用此接口
        SetWindowForeground();
        if (!argumentList.empty()) {
            //只处理第一个参数
            DString url = argumentList[0];
            CreateNewTabPage(url);
        }
    }
    else {
        //转发到UI线程处理
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&MultiBrowserForm::OnAlreadyRunningAppRelaunch, this, argumentList));
    }
}

BrowserBox* MultiBrowserForm::CreateBox(const std::string &browser_id, DString url)
{
    DString id = ui::StringConvert::UTF8ToT(browser_id);
    if (nullptr != FindTabItem(id)) {
        ASSERT(0);
        return nullptr;
    }
    if (nullptr != FindBox(id)) {
        ASSERT(0);
        return nullptr;
    }

    TabCtrlItem* tab_item = new TabCtrlItem(m_pTabCtrl->GetWindow());
    GlobalManager::Instance().FillBoxWithCache(tab_item, ui::FilePath(_T("multi_browser/tab_item.xml")));
    m_pTabCtrl->AddItemAt(tab_item, GetBoxCount());
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    tab_item->AttachAllEvents(UiBind(&MultiBrowserForm::OnProcessTabItemDrag, this, std::placeholders::_1));
#endif
    tab_item->AttachButtonDown(UiBind(&MultiBrowserForm::OnTabItemSelected, this, std::placeholders::_1));
    tab_item->SetUTF8Name(browser_id);
    Button* btn_item_close = tab_item->GetCloseButton();
    ASSERT(btn_item_close != nullptr);
    if (btn_item_close != nullptr) {
        btn_item_close->AttachClick(UiBind(&MultiBrowserForm::OnTabItemClose, this, std::placeholders::_1, browser_id));
    }

    BrowserBox* browser_box = new BrowserBox(m_pBorwserBoxTab->GetWindow(), browser_id);
    m_pBorwserBoxTab->AddItem(browser_box);
    GlobalManager::Instance().FillBoxWithCache(browser_box, ui::FilePath(_T("multi_browser/browser_box.xml")), nullptr);
    browser_box->SetName(id);
    browser_box->InitBrowserBox(url);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    auto taskbar_item = browser_box->GetTaskbarItem();
    if (taskbar_item) {
        m_taskbarManager.RegisterTab(*taskbar_item);
    }
#endif

    if (GetBoxCount() <= 1) {
        m_pActiveBrowserBox = browser_box;
    }

    // 切换到新的浏览器盒子
    // 如果tab_item处于隐藏状态，则无法顺利触发选择事件，所以这里直接切换到目标浏览器盒子
    tab_item->Selected(true, false);
    ChangeToBox(id);

    return browser_box;
}

bool MultiBrowserForm::CloseBox(const std::string& browser_id)
{
    if (browser_id.empty()) {
        return false;
    }

    DString id = ui::StringConvert::UTF8ToT(browser_id);

    bool bRet = false;
    // 从左侧会话列表项移除对应item
    TabCtrlItem* tab_item = FindTabItem(id);
    if (nullptr != tab_item) {
        m_pTabCtrl->RemoveItem(tab_item);
        bRet = true;
    }

    // 在浏览器列表中找到浏览器盒子并且移除盒子
    BrowserBox* browser_box = FindBox(id);
    ASSERT(nullptr != browser_box);
    if (nullptr != browser_box) {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        auto taskbar_item = browser_box->GetTaskbarItem();
        if (taskbar_item) {
            m_taskbarManager.UnregisterTab(*taskbar_item);
        }
#endif
        browser_box->UninitBrowserBox();
        // 如果浏览器盒子的数量大于1就立马移除盒子，否则不移除
        // 如果最后一个浏览器盒子在这里立马移除，在窗口关闭时界面会因为没有控件而变成黑色
        // 窗口关闭时，会自动的移除这个浏览器盒子
        if (m_pBorwserBoxTab->GetItemCount() > 1) {
            m_pBorwserBoxTab->RemoveItem(browser_box);
            if (m_pActiveBrowserBox == browser_box) {
                m_pActiveBrowserBox = nullptr;
                size_t nSelItem = m_pBorwserBoxTab->GetCurSel();
                if (nSelItem != Box::InvalidIndex) {
                    m_pActiveBrowserBox = dynamic_cast<BrowserBox*>(m_pBorwserBoxTab->GetItemAt(nSelItem));
                }                
            }
        }
        else {
            m_pActiveBrowserBox = nullptr;
        }
    }

    // 当浏览器盒子清空时，关闭浏览器窗口
    if (GetBoxCount() == 0) {
        this->CloseWnd(kBrowserBoxClose);
    }
    return bRet;
}

bool MultiBrowserForm::AttachBox(BrowserBox *browser_box)
{
    if (nullptr == browser_box) {
        return false;
    }

    DString id = ui::StringConvert::UTF8ToT(browser_box->GetId());
    if (nullptr != FindTabItem(id)) {
        ASSERT(0);
        return false;
    }
    if (nullptr != FindBox(id)) {
        ASSERT(0);
        return false;
    }

    TabCtrlItem* tab_item = new TabCtrlItem(m_pTabCtrl->GetWindow());
    GlobalManager::Instance().FillBoxWithCache(tab_item, ui::FilePath(_T("multi_browser/tab_item.xml")));
    m_pTabCtrl->AddItemAt(tab_item, GetBoxCount());
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    tab_item->AttachAllEvents(UiBind(&MultiBrowserForm::OnProcessTabItemDrag, this, std::placeholders::_1));
#endif
    tab_item->AttachButtonDown(UiBind(&MultiBrowserForm::OnTabItemSelected, this, std::placeholders::_1));
    tab_item->SetUTF8Name(browser_box->GetId());
    tab_item->SetTitle(browser_box->GetTitle());
    Button* btn_item_close = tab_item->GetCloseButton();
    ASSERT(btn_item_close != nullptr);
    if (btn_item_close != nullptr) {
        btn_item_close->AttachClick(UiBind(&MultiBrowserForm::OnTabItemClose, this, std::placeholders::_1, browser_box->GetId()));
    }

    // 当另一个窗体创建的browser_box浏览器盒子控件添加到另一个窗体内的容器控件时
    // AddItem函数会重新的修改browser_box内所有子控件的m_pWindow为新的窗体指针
    m_pBorwserBoxTab->AddItem(browser_box);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    auto taskbar_item = browser_box->GetTaskbarItem();
    if (taskbar_item) {
        m_taskbarManager.RegisterTab(*taskbar_item);
    }
#endif

    if (GetBoxCount() <= 1) {
        m_pActiveBrowserBox = browser_box;
    }

    // 切换到新的浏览器盒子
    // 如果tab_item处于隐藏状态，则无法顺利触发选择事件，所以这里直接切换到目标浏览器盒子
    tab_item->Selected(true, false);
    ChangeToBox(id);

    return true;
}

bool MultiBrowserForm::DetachBox(BrowserBox *browser_box)
{
    if (nullptr == browser_box) {
        return false;
    }

    DString id = ui::StringConvert::UTF8ToT(browser_box->GetId());

    // 从顶部标签页移除对应item
    TabCtrlItem* tab_item = FindTabItem(id);
    if (nullptr == tab_item) {
        return false;
    }

    m_pTabCtrl->RemoveItem(tab_item);

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    auto taskbar_item = browser_box->GetTaskbarItem();
    if (taskbar_item) {
        m_taskbarManager.UnregisterTab(*taskbar_item);
    }
#endif

    // 在右侧Tab浏览器盒子列表中找到浏览器盒子并且移除盒子
    // 在这里不能delete browser_box
    bool auto_destroy = m_pBorwserBoxTab->IsAutoDestroyChild();
    m_pBorwserBoxTab->SetAutoDestroyChild(false);
    if (!m_pBorwserBoxTab->RemoveItem(browser_box)) {
        m_pBorwserBoxTab->SetAutoDestroyChild(auto_destroy);
        return false;
    }
    m_pBorwserBoxTab->SetAutoDestroyChild(auto_destroy);
    if (m_pActiveBrowserBox == browser_box) {
        m_pActiveBrowserBox = nullptr;
    }

    // 当浏览器盒子清空时，关闭浏览器窗口
    if (GetBoxCount() == 0) {
        this->CloseWnd(kBrowserBoxClose);
    }
    return true;
}

BrowserBox* MultiBrowserForm::GetSelectedBox()
{
    return m_pActiveBrowserBox;
}

void MultiBrowserForm::SetActiveBox(const std::string &browser_id)
{
    if (browser_id.empty()) {
        return;
    }

    if (IsWindowMinimized()) {
        ShowWindow(kSW_RESTORE);
    }
    else {
        ShowWindow(kSW_SHOW);
    }

    // 从窗口左侧会话列表找到要激活的浏览器盒子项
    DString id = ui::StringConvert::UTF8ToT(browser_id);
    TabCtrlItem* tab_item = FindTabItem(id);
    if (nullptr == tab_item) {
        return;
    }

    // 如果tab_item处于隐藏状态，则无法顺利触发选择事件，所以这里直接切换到目标浏览器盒子
    tab_item->Selected(true, false);
    ChangeToBox(id);
}

bool MultiBrowserForm::IsActiveBox(const BrowserBox *browser_box)
{
    ASSERT(nullptr != browser_box);
    return (browser_box == m_pActiveBrowserBox && IsWindowForeground() && !IsWindowMinimized() && IsWindowVisible());
}

bool MultiBrowserForm::IsActiveBox(const DString &browser_id)
{
    ASSERT(!browser_id.empty());
    return (IsWindowForeground() && !IsWindowMinimized() && IsWindowVisible() && FindBox(browser_id) == m_pActiveBrowserBox);
}

int MultiBrowserForm::GetBoxCount() const
{
    int nBoxCount = (int)m_pTabCtrl->GetItemCount();
    return nBoxCount;
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
void MultiBrowserForm::OnBeforeDragBoxCallback(const DString &browser_id)
{
    // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，则在拖拽时隐藏浏览器窗口
    int box_count = this->GetBoxCount();
    if (1 == box_count) {
        this->ShowWindow(kSW_HIDE);
    }
    // 否则隐藏被拖拽的浏览器盒子和标签
    else {
        BrowserBox *browser_box = FindBox(browser_id);
        if (nullptr != browser_box) {
            browser_box->SetFadeVisible(false);
        }

        TabCtrlItem* tab_item = FindTabItem(browser_id);
        if (nullptr != tab_item) {
            tab_item->SetFadeVisible(false);
        }

        // 找到新的被显示的浏览器盒子
        int index = tab_item->GetListBoxIndex();
        if (index > 0) {
            index--;
        }
        else {
            index++;
        }
        TabCtrlItem* new_tab_item = static_cast<TabCtrlItem*>(m_pTabCtrl->GetItemAt(index));
        if (nullptr != new_tab_item) {
            new_tab_item->Selected(true, false);
            ChangeToBox(new_tab_item->GetName());
        }
        m_dragingBrowserId = browser_id;
    }
}

void MultiBrowserForm::OnAfterDragBoxCallback(bool drop_succeed)
{
    m_bDragState = false;

    if (drop_succeed) {
        int box_count = this->GetBoxCount();
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，并且拖拽到新的浏览器窗口里，这个浏览器窗口就会关闭
        if (1 == box_count) {

        }
        else {
        }
    }
    else {
        int box_count = this->GetBoxCount();
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，并且没有拖拽到新的浏览器窗口里
        // 就显示浏览器窗口
        if (1 == box_count) {
            this->ShowWindow(kSW_SHOW_NORMAL);
        }
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口有多个浏览器盒子，并且没有拖拽到新的浏览器窗口里
        // 就显示之前被隐藏的浏览器盒子和标签
        else {
            BrowserBox *browser_box = FindBox(m_dragingBrowserId);
            if (nullptr != browser_box) {
                browser_box->SetFadeVisible(true);
            }

            TabCtrlItem* tab_item = FindTabItem(m_dragingBrowserId);
            if (nullptr != tab_item) {
                tab_item->SetFadeVisible(true);
                tab_item->Selected(true, false);
                ChangeToBox(m_dragingBrowserId);
            }
            m_dragingBrowserId.clear();
        }
    }
}
#endif

bool MultiBrowserForm::OnTabItemSelected(const ui::EventArgs& param)
{
    if (kEventSelect == param.eventType) {
        ASSERT(param.GetSender() == m_pTabCtrl);
        if (m_pTabCtrl != nullptr) {
            // 如果单击了顶部的标签，则找到下方Tab里对应的浏览器盒子并选中
            Control* pSelectedItem = m_pTabCtrl->GetItemAt(m_pTabCtrl->GetCurSel());
            ASSERT(pSelectedItem != nullptr);
            if (pSelectedItem != nullptr) {
                DString session_id = pSelectedItem->GetName();
                ChangeToBox(session_id);
            }
        }
    }
    else if (kEventMouseButtonDown == param.eventType) {
        TabCtrlItem* tab_item = dynamic_cast<TabCtrlItem*>(param.GetSender());
        if (tab_item) {
            DString browser_id = tab_item->GetName();
            ChangeToBox(browser_id);
        }
    }
    return false;
}

bool MultiBrowserForm::OnTabItemClose(const ui::EventArgs& param, const std::string& browser_id)
{
    CloseBox(browser_id);
    return true;
}

BrowserBox* MultiBrowserForm::FindBox(const DString &browser_id)
{
    for (int i = 0; i < (int)m_pBorwserBoxTab->GetItemCount(); i++) {
        Control *box_item = m_pBorwserBoxTab->GetItemAt(i);
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

TabCtrlItem* MultiBrowserForm::FindTabItem(const DString &browser_id)
{
    for (int i = 0; i < GetBoxCount(); i++) {
        Control *tab_item = m_pTabCtrl->GetItemAt(i);
        ASSERT(nullptr != tab_item);
        if (nullptr == tab_item) {
            return nullptr;
        }

        if (tab_item->GetName() == browser_id) {
            return static_cast<TabCtrlItem*>(tab_item);
        }
    }
    return nullptr;
}

void MultiBrowserForm::SetTabItemName(const DString &browser_id, const DString &name)
{
    TabCtrlItem* tab_item = FindTabItem(browser_id);
    if (nullptr != tab_item) {
        tab_item->SetTitle(name);
    }
}

void MultiBrowserForm::SetURL(const std::string &browser_id, const DString &url)
{
    if (m_pActiveBrowserBox && m_pActiveBrowserBox->GetId() == browser_id) {
        m_pEditUrl->SetText(url);
    }
}

bool MultiBrowserForm::ChangeToBox(const DString &browser_id)
{
    if (browser_id.empty()) {
        return false;
    }

    BrowserBox* box_item = FindBox(browser_id);
    if (nullptr == box_item) {
        return false;
    }
    box_item->SetPos(box_item->GetPos());
    m_pBorwserBoxTab->SelectItem(box_item);
    box_item->SetFocus();
    m_pActiveBrowserBox = box_item;

    // 根据当前激活的浏览器盒子，更新任务栏的图标和标题
    m_pEditUrl->SetText(m_pActiveBrowserBox->GetCefControl()->GetURL().ToWString());    
    OnLoadingStateChange(m_pActiveBrowserBox);
    return true;
}

void MultiBrowserForm::NotifyFavicon(const BrowserBox* browser_box, CefRefPtr<CefImage> image)
{
    if ((browser_box == nullptr) || (image == nullptr)) {
        return;
    }

    DString id = ui::StringConvert::UTF8ToT(browser_box->GetId());
    TabCtrlItem* tab_item = FindTabItem(id);
    if (tab_item == nullptr) {
        return;
    }
    Control* pIconControl = tab_item->GetIconControl();

}
