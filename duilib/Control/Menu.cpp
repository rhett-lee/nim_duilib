#include "Menu.h"
#include "MenuListBox.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Core/WindowCreateParam.h"

namespace ui {

//TODO: 静态对象集中管理
ContextMenuObserver& Menu::GetMenuObserver()
{
    static ContextMenuObserver s_context_menu_observer;
    return s_context_menu_observer;
}

//二级或者多级子菜单的托管类
class SubMenu: public ui::ListBoxItem
{
public:
    explicit SubMenu(Window* pWindow):
        ListBoxItem(pWindow)
    {
    }
};

ui::Control* Menu::CreateControl(const DString& pstrClass)
{
    if (pstrClass == DUI_CTR_MENU_ITEM){
        return new MenuItem(this);
    }
    else if (pstrClass == DUI_CTR_SUB_MENU) {
        return new SubMenu(this);
    }
    else if (pstrClass == DUI_CTR_MENU_LISTBOX) {
        return new MenuListBox(this);
    }
    return nullptr;
}

bool Menu::Receive(ContextMenuParam param)
{
    switch (param.wParam)
    {
    case MenuCloseType::eMenuCloseAll:
        CloseMenu();
        break;
        case MenuCloseType::eMenuCloseThis:
        {
            Window* pParentWindow = GetParentWindow();
            while (pParentWindow != nullptr) {
                if (pParentWindow == param.pWindow) {
                    CloseMenu();
                    break;
                }
                pParentWindow = pParentWindow->GetParentWindow();
            }
        }
        break;
    default:
        break;
    }

    return true;
}

Menu::Menu(Window* pParentWindow, Control* pRelatedControl):
    m_pParentWindow(pParentWindow),
    m_pRelatedControl(pRelatedControl),
    m_menuPoint({ 0, 0 }),
    m_popupPosType(MenuPopupPosType::RIGHT_TOP),
    m_noFocus(false),
    m_pOwner(nullptr),
    m_pListBox(nullptr)
{
    m_skinFolder = DString(_T("public/menu/"));
    m_submenuXml = DString(_T("submenu.xml"));
    m_submenuNodeName = DString(_T("submenu"));
}

void Menu::SetSkinFolder(const DString& skinFolder)
{
    m_skinFolder = skinFolder;
}

void Menu::SetSubMenuXml(const DString& submenuXml, const DString& submenuNodeName)
{
    m_submenuXml = submenuXml;
    m_submenuNodeName = submenuNodeName;
}

void Menu::ShowMenu(const DString& xml, const UiPoint& point, MenuPopupPosType popupPosType, bool noFocus, MenuItem* pOwner)
{
    m_menuPoint = point;
    m_popupPosType = popupPosType;

    m_xml = xml;
    m_noFocus = noFocus;
    m_pOwner = pOwner;

    Menu::GetMenuObserver().AddReceiver(this);
    WindowCreateParam createWndParam;
    createWndParam.m_dwStyle = kWS_POPUP;
    createWndParam.m_dwExStyle = kWS_EX_TOPMOST | kWS_EX_LAYERED;
    //设置初始位置，避免菜单初次显示时出现黑屏现象
    createWndParam.m_nX = point.x;
    createWndParam.m_nY = point.y;
    CreateWnd(m_pParentWindow, createWndParam);
    
    bool bShown = false;
    if (m_pOwner) {
        bShown = ResizeSubMenu();
    }
    else {
        bShown = ResizeMenu();
    }
    if (!bShown) {
        if (noFocus) {
            ShowWindow(kSW_SHOW_NA);
        }
        else {
            ShowWindow(kSW_SHOW_NORMAL);
        }
    }
    KeepParentActive();
    //修正菜单项的宽度，保持一致
    UpdateWindow();
    ListBox* pLayoutListBox = Menu::GetLayoutListBox();
    if (pLayoutListBox != nullptr) {
        std::vector<MenuItem*> allMenuItems;
        const size_t nItemCount = pLayoutListBox->GetItemCount();
        for (size_t i = 0; i < nItemCount; ++i) {
            MenuItem* pMenuItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(i));
            if (pMenuItem != nullptr) {
                allMenuItems.push_back(pMenuItem);
            }
        }
        int32_t nMaxWidth = 0;
        for (auto pMenuItem : allMenuItems) {
            if (pMenuItem == nullptr) {
                continue;
            }
            if (pMenuItem->GetFixedWidth().IsInt32()) {
                nMaxWidth = std::max(nMaxWidth, pMenuItem->GetFixedWidth().GetInt32());
            }
            else if (pMenuItem->GetFixedWidth().IsAuto()) {
                nMaxWidth = std::max(nMaxWidth, pMenuItem->GetWidth());
            }
        }
        if (nMaxWidth > 0) {
            for (auto pMenuItem : allMenuItems) {
                if (pMenuItem == nullptr) {
                    continue;
                }
                if (pMenuItem->GetFixedWidth().IsAuto() || pMenuItem->GetFixedWidth().IsInt32()) {
                    pMenuItem->SetFixedWidth(UiFixedInt(nMaxWidth), true, false);
                }
            }
        }        
    }
}

void Menu::CloseMenu()
{
    //立即关闭，避免连续操作时相互干扰
    CloseWnd();
}

void Menu::DetachOwner()
{
    if (m_pOwner != nullptr) {
        ListBox* pLayoutListBox = Menu::GetLayoutListBox();
        if (pLayoutListBox != nullptr) {
            pLayoutListBox->SelectItem(Box::InvalidIndex, false, false);
        }

        //将在OnInitWindow中，添加到Layout上的节点，解除关联关系
        std::vector<Control*> submenuControls;
        MenuItem::GetAllSubMenuControls(m_pOwner, submenuControls);
        for (auto pItem : submenuControls) {
            if (pItem != nullptr) {
                pItem->SetWindow(nullptr);
                pItem->SetParent(nullptr);
            }
        }

        if (pLayoutListBox != nullptr) {
            pLayoutListBox->RemoveAllItems();
        }
        m_pOwner->m_pSubWindow = nullptr;
        m_pOwner->Invalidate();
        m_pOwner = nullptr;
    }
}

DString Menu::GetSkinFolder()
{
    return m_skinFolder.c_str();
}

DString Menu::GetSkinFile() 
{
    return m_xml.c_str();
}

LRESULT Menu::OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKillFocusMsg(pSetFocusWindow, nativeMsg, bHandled);
    bHandled = true;
    bool bInMenuWindowList = false;
    if (pSetFocusWindow != nullptr) {
        ContextMenuObserver::Iterator<bool, ContextMenuParam> iterator(GetMenuObserver());
        ReceiverImplBase<bool, ContextMenuParam>* pReceiver = iterator.next();
        while (pReceiver != nullptr) {
            Menu* pContextMenu = dynamic_cast<Menu*>(pReceiver);
            if ((pContextMenu != nullptr) && (pContextMenu == pSetFocusWindow)) {
                bInMenuWindowList = true;
                break;
            }
            pReceiver = iterator.next();
        }
    }
    if (!bInMenuWindowList) {
        ContextMenuParam param;
        param.pWindow = this;
        param.wParam = MenuCloseType::eMenuCloseAll;
        GetMenuObserver().RBroadcast(param);
        return 0;
    }
    return lResult;
}

LRESULT Menu::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    bHandled = true;
    if (vkCode == kVK_ESCAPE || vkCode == kVK_LEFT) {
        CloseMenu();
    }
    else if (vkCode == kVK_RIGHT) {
        ListBox* pLayoutListBox = Menu::GetLayoutListBox();
        if (pLayoutListBox != nullptr) {
            size_t index = pLayoutListBox->GetCurSel();
            MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(index));
            if (pItem != nullptr) {
                pItem->CheckSubMenuItem();
            }
        }
    }
    else if (vkCode == kVK_RETURN || vkCode == kVK_SPACE)
    {
        ListBox* pLayoutListBox = Menu::GetLayoutListBox();
        if (pLayoutListBox != nullptr) {
            size_t index = pLayoutListBox->GetCurSel();
            MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(index));
            if (pItem != nullptr) {
                if (!pItem->CheckSubMenuItem()) {
                    ContextMenuParam param;
                    param.pWindow = this;
                    param.wParam = MenuCloseType::eMenuCloseAll;
                    //回车时，激活当前选择的菜单项
                    pItem->Activate(nullptr);
                    Menu::GetMenuObserver().RBroadcast(param);
                }
            }
        }
    }
    else if (vkCode == kVK_DOWN || vkCode == kVK_UP) {
        //支持键盘上下键切换当前菜单项
        ListBox* pLayoutListBox = Menu::GetLayoutListBox();
        if (pLayoutListBox != nullptr) {
            //默认选中当前处于hot状态的菜单项，以支持键盘操作            
            if (!Box::IsValidItemIndex(pLayoutListBox->GetCurSel())) {
                for (size_t nIndex = 0; nIndex < pLayoutListBox->GetItemCount(); ++nIndex) {
                    MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(nIndex));
                    if (pItem != nullptr) {
                        if (pItem->GetState() == ControlStateType::kControlStateHot) {
                            pLayoutListBox->SetCurSel(nIndex);
                            break;
                        }
                    }
                }                
            }            
        }
        BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT Menu::OnContextMenuMsg(const UiPoint& /*pt*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = true;
    return 0;
}

LRESULT Menu::OnMouseRButtonDownMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = true;
    return 0;
}

LRESULT Menu::OnMouseRButtonUpMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = true;
    return 0;
}

LRESULT Menu::OnMouseRButtonDbClickMsg(const UiPoint& /*pt*/, uint32_t /*modifierKey*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = true;
    return 0;
}

bool Menu::ResizeMenu()
{
    ui::Control* pRoot = GetRoot();
    ASSERT(pRoot != nullptr);
    if (pRoot == nullptr) {
        return false;
    }
    //点击在哪里，以哪里的屏幕为主
    ui::UiRect rcWork;
    GetMonitorWorkRect(m_menuPoint, rcWork);

    ui::UiSize szMenuWindow = { rcWork.Width(), rcWork.Height()};
    UiEstSize estSize = pRoot->EstimateSize(szMenuWindow);   //这里返回的大小包含了阴影的大小
    if (estSize.cx.IsInt32()) {
        szMenuWindow.cx = estSize.cx.GetInt32();
    }
    if (estSize.cy.IsInt32()) {
        szMenuWindow.cy = estSize.cy.GetInt32();
    }

    UiPadding rcShadowCorner = pRoot->GetPadding(); //窗口阴影所占区域
    ui::UiSize szMenuClient = szMenuWindow;
    szMenuClient.cx -= rcShadowCorner.left + rcShadowCorner.right;
    szMenuClient.cy -= rcShadowCorner.top + rcShadowCorner.bottom; //这里去掉阴影窗口，即用户的视觉有效面积

    ui::UiPoint point(m_menuPoint);  //这里有个bug，由于坐标点与包含在窗口内，会直接出发mouseenter导致出来子菜单，偏移1个像素
    if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Right)) {
        point.x += -szMenuWindow.cx + rcShadowCorner.right + rcShadowCorner.left;
        point.x -= 1;
    }
    else if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Left)) {
        point.x += 1;
    }
    if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Bottom))    {
        point.y += -szMenuWindow.cy + rcShadowCorner.bottom + rcShadowCorner.top;
        point.y += 1;
    }
    else if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Top)) {
        point.y += 1;
    }
    if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Intelligent)) {
        if (point.x < rcWork.left) {
            point.x = rcWork.left;
        }
        else if (point.x + szMenuClient.cx> rcWork.right) {
            point.x = rcWork.right - szMenuClient.cx;
        }
        if (point.y < rcWork.top) {
            point.y = rcWork.top ;
        }
        else if (point.y + szMenuClient.cy > rcWork.bottom) {
            point.y = rcWork.bottom - szMenuClient.cy;
        }
    }
   
    SetWindowPos(InsertAfterWnd(InsertAfterFlag::kHWND_TOPMOST),
                 point.x - rcShadowCorner.left, point.y - rcShadowCorner.top,
                 szMenuWindow.cx, szMenuWindow.cy,
                 kSWP_SHOWWINDOW | (m_noFocus ? kSWP_NOACTIVATE : 0));

    if (!m_noFocus) {
        SetWindowForeground();
        ListBox* pLayoutListBox = Menu::GetLayoutListBox();
        SetFocusControl(pLayoutListBox);
    }
    return true;
}

bool Menu::ResizeSubMenu()
{
    ASSERT(m_pOwner != nullptr);
    if (m_pOwner == nullptr) {
        return false;
    }
    ASSERT(m_pOwner->GetWindow() != nullptr);

    // Position the popup window in absolute space
    UiRect rcOwner = m_pOwner->GetPos();
    UiRect rc = rcOwner;
   
    UiPadding rcCorner = GetCurrentShadowCorner();
    UiRect rcWindow;
    m_pOwner->GetWindow()->GetWindowRect(rcWindow);

    UiRect rcClient;
    GetClientRect(rcClient);
    rcClient.Deflate(rcCorner);
    int32_t cxFixed = rcClient.Width();
    int32_t cyFixed = rcClient.Height();
    rcClient.Inflate(rcCorner);
    if (rcClient.Width() < (rcCorner.left + rcCorner.right)) {
        //窗口大小还没有生效，需要估算
        Box* pRoot = GetRoot();
        if (pRoot != nullptr) {
            UiSize maxSize(999999, 999999);
            UiEstSize estSize = pRoot->EstimateSize(maxSize);
            if (!estSize.cx.IsStretch() && !estSize.cy.IsStretch()) {
                UiSize needSize = MakeSize(estSize);
                if (needSize.cx < pRoot->GetMinWidth()) {
                    needSize.cx = pRoot->GetMinWidth();
                }
                if (needSize.cx > pRoot->GetMaxWidth()) {
                    needSize.cx = pRoot->GetMaxWidth();
                }
                if (needSize.cy < pRoot->GetMinHeight()) {
                    needSize.cy = pRoot->GetMinHeight();
                }
                if (needSize.cy > pRoot->GetMaxHeight()) {
                    needSize.cy = pRoot->GetMaxHeight();
                }
                cxFixed = needSize.cx - rcCorner.left - rcCorner.right;
                cyFixed = needSize.cy - rcCorner.top - rcCorner.bottom;
            }
        }
    }

    //去阴影
    rcWindow.Deflate(rcCorner);

    m_pOwner->GetWindow()->ClientToScreen(rc);
   
    rc.left = rcWindow.right;
    rc.right = rc.left + cxFixed;
    rc.bottom = rc.top + cyFixed;

    bool bReachBottom = false;
    bool bReachRight = false;

    UiRect rcPreWindow;
    ContextMenuObserver::Iterator<bool, ContextMenuParam> iterator(GetMenuObserver());
    ReceiverImplBase<bool, ContextMenuParam>* pReceiver = iterator.next();
    while (pReceiver != nullptr) {
        Menu* pContextMenu = dynamic_cast<Menu*>(pReceiver);
        if (pContextMenu != nullptr) {
            pContextMenu->GetWindowRect(rcPreWindow);  //需要减掉阴影

            bReachRight = (rcPreWindow.left + rcCorner.left) >= rcWindow.right;
            bReachBottom = (rcPreWindow.top + rcCorner.top) >= rcWindow.bottom;
            if (pContextMenu->GetWindowHandle() == m_pOwner->GetWindow()->GetWindowHandle()
                || bReachBottom || bReachRight) {
                break;
            }
        }
        pReceiver = iterator.next();
    }
    if (bReachBottom) {
        rc.bottom = rcWindow.top;
        rc.top = rc.bottom - cyFixed;
    }

    if (bReachRight) {
        rc.right = rcWindow.left;
        rc.left = rc.right - cxFixed;
    }

    UiRect rcWork;
    GetMonitorWorkRect(m_menuPoint, rcWork);

    if (rc.bottom > rcWork.bottom) {
        rc.bottom = rc.top;
        rc.top = rc.bottom - cyFixed;
    }

    if (rc.right > rcWork.right) {
        rc.right = rcWindow.left;
        rc.left = rc.right - cxFixed;
    }

    if (rc.top < rcWork.top) {
        rc.top = rcOwner.top;
        rc.bottom = rc.top + cyFixed;
    }

    if (rc.left < rcWork.left) {
        rc.left = rcWindow.right;
        rc.right = rc.left + cxFixed;
    }

    //调整窗口位置，显示窗口，但不调整窗口的大小
    int32_t nNewWidth = rc.Width() + rcCorner.left + rcCorner.right;
    int32_t nNewHeight = rc.Height() + rcCorner.top + rcCorner.bottom;
    ASSERT(nNewWidth == rcClient.Width());
    ASSERT(nNewHeight == rcClient.Height());
    SetWindowPos(InsertAfterWnd(InsertAfterFlag::kHWND_TOPMOST),
                 rc.left - rcCorner.left, rc.top - rcCorner.top,
                 nNewWidth, nNewHeight,
                 kSWP_SHOWWINDOW | kSWP_NOSIZE | (m_noFocus ? kSWP_NOACTIVATE : 0));

    if (!m_noFocus) {
        SetWindowForeground();
        SetFocusControl(Menu::GetLayoutListBox());
    }
    return true;
}

void Menu::PostInitWindow()
{
    ASSERT(m_pListBox == nullptr);
    if (m_pOwner != nullptr) {
        m_pListBox = dynamic_cast<ui::ListBox*>(FindControl(m_submenuNodeName.c_str()));
        ASSERT(m_pListBox != nullptr);
        if (m_pListBox == nullptr) {
            return;
        }
        //设置不自动销毁Child对象（因为是从owner复制过来的，资源公用，由Owner管理生命对象的周期）
        m_pListBox->SetAutoDestroyChild(false);

        //获取子菜单项需要绘制的控件，并添加到Layout
        std::vector<Control*> submenuControls;
        MenuItem::GetAllSubMenuControls(m_pOwner, submenuControls);
        for (auto pControl : submenuControls) {
            if (pControl != nullptr) {
                m_pListBox->AddItem(pControl);
                continue;
            }
        }
    }
    else {
        m_pListBox = dynamic_cast<ui::ListBox*>(GetRoot());
        if (m_pListBox == nullptr) {
            //允许外面套层阴影
            if ((GetRoot() != nullptr) && (GetRoot()->GetItemCount() > 0)) {
                m_pListBox = dynamic_cast<ui::ListBox*>(GetRoot()->GetItemAt(0));
            }
        }
        ASSERT(m_pListBox != nullptr);
    }

    //菜单显示后，让关联控件处于Push状态(异步)
    if (m_pRelatedControl != nullptr) {
        m_pRelatedControl->SetState(kControlStatePushed);
    }

    //需要在最后才调用基类的实现函数
    BaseClass::PostInitWindow();
}

ListBox* Menu::GetLayoutListBox() const
{
    return m_pListBox.get();
}

void Menu::OnMenuItemActivated(const DString& menuName, int32_t nMenuLevel,
                               const DString& itemName, size_t nItemIndex)
{
    Menu* pParentMenu = nullptr;
    if (GetParentWindow() != nullptr) {
        pParentMenu = dynamic_cast<Menu*>(GetParentWindow());
    }
    if (pParentMenu != nullptr) {
        pParentMenu->OnMenuItemActivated(menuName, nMenuLevel + 1, itemName, nItemIndex);
    }
    else {
        //已经是顶级菜单
        m_pActiveMenuItem = std::make_unique<ActiveMenuItem>();
        m_pActiveMenuItem->m_itemIndex = nItemIndex;
        m_pActiveMenuItem->m_itemName = itemName;
        m_pActiveMenuItem->m_menuLevel = nMenuLevel;
        m_pActiveMenuItem->m_menuName = menuName;
    }
}

void Menu::AttachMenuItemActivated(MenuItemActivatedEvent callback)
{
    if (callback != nullptr) {
        m_callbackList.push_back(callback);
    }
}

void Menu::OnFinalMessage()
{
    //发送回调，通知已经选择的事件
    if ((m_pActiveMenuItem != nullptr) && !m_callbackList.empty()) {
        ActiveMenuItem activeData = *m_pActiveMenuItem;
        std::vector<MenuItemActivatedEvent> callbackList(m_callbackList);
        for (MenuItemActivatedEvent callback : callbackList) {
            if (callback) {
                callback(activeData.m_menuName, activeData.m_menuLevel,
                         activeData.m_itemName, activeData.m_itemIndex);
            }
        }
    }
    BaseClass::OnFinalMessage();
}

void Menu::OnCloseWindow()
{
    RemoveObserver();
    DetachOwner();
    BaseClass::OnCloseWindow();
}

bool Menu::AddMenuItem(MenuItem* pMenuItem)
{
    //目前只有一级菜单可以访问这个接口
    ASSERT(m_pOwner == nullptr);
    ListBox* pLayoutListBox = Menu::GetLayoutListBox();
    ASSERT(pLayoutListBox != nullptr);
    if (pLayoutListBox != nullptr) {
        return pLayoutListBox->AddItem(pMenuItem);
    }
    return false;
}

bool Menu::AddMenuItemAt(MenuItem* pMenuItem, size_t iIndex)
{
    //目前只有一级菜单可以访问这个接口
    ASSERT(m_pOwner == nullptr);
    ListBox* pLayoutListBox = Menu::GetLayoutListBox();
    ASSERT(pLayoutListBox != nullptr);
    if (pLayoutListBox == nullptr) {
        return false;
    }
    
    size_t itemIndex = 0;
    MenuItem* pElementUI = nullptr;
    const size_t count = pLayoutListBox->GetItemCount();
    for (size_t i = 0; i < count; ++i) {
        Control* pControl = pLayoutListBox->GetItemAt(i);
        pElementUI = dynamic_cast<MenuItem*>(pControl);
        if (pElementUI != nullptr) {
            if (itemIndex == iIndex) {
                return pLayoutListBox->AddItemAt(pMenuItem, i);
            }
            ++itemIndex;
        }
        pElementUI = nullptr;
    }
    return false;
}

bool Menu::RemoveMenuItem(MenuItem* pMenuItem)
{
    //目前只有一级菜单可以访问这个接口
    ASSERT(m_pOwner == nullptr);
    ListBox* pLayoutListBox = Menu::GetLayoutListBox();
    ASSERT(pLayoutListBox != nullptr);
    MenuItem* pElementUI = nullptr;
    if (pLayoutListBox != nullptr) {
        const size_t count = pLayoutListBox->GetItemCount();
        for (size_t i = 0; i < count; ++i) {
            pElementUI = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(i));
            if (pMenuItem == pElementUI) {
                pLayoutListBox->RemoveItemAt(i);
            }
            pElementUI = nullptr;
        }
    }
    return false;
}

bool Menu::RemoveMenuItemAt(size_t iIndex)
{
    //目前只有一级菜单可以访问这个接口
    ASSERT(m_pOwner == nullptr);
    MenuItem* pMenuElementUI = GetMenuItemAt(iIndex);
    if (pMenuElementUI != nullptr) {
        return RemoveMenuItem(pMenuElementUI);
    }
    return false;
}

size_t Menu::GetMenuItemCount() const
{
    //目前只有一级菜单可以访问这个接口
    ASSERT(m_pOwner == nullptr);
    ListBox* pLayoutListBox = Menu::GetLayoutListBox();
    if (pLayoutListBox == nullptr) {
        return 0;
    }
    size_t itemCount = 0;
    const size_t count = pLayoutListBox->GetItemCount();
    for (size_t i = 0; i < count; ++i) {
        if (dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(i)) != nullptr) {
            ++itemCount;
        }
    }
    return itemCount;
}

MenuItem* Menu::GetMenuItemAt(size_t iIndex) const
{
    //目前只有一级菜单可以访问这个接口
    ASSERT(m_pOwner == nullptr);
    ListBox* pLayoutListBox = Menu::GetLayoutListBox();
    ASSERT(pLayoutListBox != nullptr);
    if (pLayoutListBox == nullptr) {
        return nullptr;
    }
    size_t itemIndex = 0;
    MenuItem* pElementUI = nullptr;
    const size_t count = pLayoutListBox->GetItemCount();
    for (size_t i = 0; i < count; ++i) {
        Control* pControl = pLayoutListBox->GetItemAt(i);
        pElementUI = dynamic_cast<MenuItem*>(pControl);
        if (pElementUI != nullptr) {
            if (itemIndex == iIndex) {
                break;
            }
            ++itemIndex;
        }
        pElementUI = nullptr;
    }
    return pElementUI;
}

MenuItem* Menu::GetMenuItemByName(const DString& name) const
{
    //目前只有一级菜单可以访问这个接口
    ASSERT(m_pOwner == nullptr);
    ListBox* pLayoutListBox = Menu::GetLayoutListBox();
    ASSERT(pLayoutListBox != nullptr);
    MenuItem* pElementUI = nullptr;
    if (pLayoutListBox != nullptr) {
        const size_t count = pLayoutListBox->GetItemCount();
        for (size_t i = 0; i < count; ++i) {
            pElementUI = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(i));
            if ((pElementUI != nullptr) && (pElementUI->IsNameEquals(name))) {
                break;
            }
            pElementUI = nullptr;
        }
    }
    return pElementUI;
}

MenuItem::MenuItem(Window* pWindow):
    ListBoxItem(pWindow),
    m_pSubWindow(nullptr)
{
    //在菜单元素上，不让子控件响应鼠标消息
    SetMouseChildEnabled(false);
}

void MenuItem::GetAllSubMenuItem(const MenuItem* pParentElementUI,
                                       std::vector<MenuItem*>& submenuItems)
{
    submenuItems.clear();
    ASSERT(pParentElementUI != nullptr);
    if (pParentElementUI == nullptr) {
        return;
    }
    const size_t itemCount = pParentElementUI->GetItemCount();
    for (size_t i = 0; i < itemCount; ++i) {
        Control* pControl = pParentElementUI->GetItemAt(i);
        MenuItem* menuElementUI = dynamic_cast<MenuItem*>(pControl);
        if (menuElementUI != nullptr) {
            submenuItems.push_back(menuElementUI);
            continue;
        }

        menuElementUI = nullptr;
        SubMenu* subMenu = dynamic_cast<SubMenu*>(pControl);
        if (subMenu != nullptr) {
            const size_t count = subMenu->GetItemCount();
            for (size_t j = 0; j < count; ++j) {
                menuElementUI = dynamic_cast<MenuItem*>(subMenu->GetItemAt(j));
                if (menuElementUI != nullptr) {
                    submenuItems.push_back(menuElementUI);
                    continue;
                }
            }
        }
    }
}

void MenuItem::GetAllSubMenuControls(const MenuItem* pParentElementUI,
                                           std::vector<Control*>& submenuControls)
{
    submenuControls.clear();
    ASSERT(pParentElementUI != nullptr);
    if (pParentElementUI == nullptr) {
        return;
    }
    const size_t itemCount = pParentElementUI->GetItemCount();
    for (size_t i = 0; i < itemCount; ++i) {
        Control* pControl = pParentElementUI->GetItemAt(i);
        MenuItem* menuElementUI = dynamic_cast<MenuItem*>(pControl);
        if (menuElementUI != nullptr) {
            submenuControls.push_back(menuElementUI);
            continue;
        }

        SubMenu* subMenu = dynamic_cast<SubMenu*>(pControl);
        if (subMenu != nullptr) {
            const size_t count = subMenu->GetItemCount();
            for (size_t j = 0; j < count; ++j) {
                Control* pSubControl = subMenu->GetItemAt(j);
                if (pSubControl != nullptr) {
                    submenuControls.push_back(pSubControl);
                }
            }
        }
    }
}

bool MenuItem::AddSubMenuItem(MenuItem* pMenuItem)
{
    return AddItem(pMenuItem);
}

bool MenuItem::AddSubMenuItemAt(MenuItem* pMenuItem, size_t iIndex)
{
    const size_t subMenuCount = GetSubMenuItemCount();
    ASSERT(iIndex <= subMenuCount);
    if (iIndex > subMenuCount) {
        return false;
    }
    
    size_t itemIndex = 0;
    const size_t itemCount = GetItemCount();
    for (size_t i = 0; i < itemCount; ++i) {
        Control* pControl = GetItemAt(i);
        MenuItem* menuElementUI = dynamic_cast<MenuItem*>(pControl);
        if (menuElementUI != nullptr) {
            if (itemIndex == iIndex) {
                //在当前节点下匹配到
                return AddItemAt(pMenuItem, i);
            }
            ++itemIndex;
            continue;
        }

        menuElementUI = nullptr;
        SubMenu* subMenu = dynamic_cast<SubMenu*>(pControl);
        if (subMenu != nullptr) {
            const size_t count = subMenu->GetItemCount();
            for (size_t j = 0; j < count; ++j) {
                menuElementUI = dynamic_cast<MenuItem*>(subMenu->GetItemAt(j));
                if (menuElementUI != nullptr) {
                    if (itemIndex == iIndex) {
                        //在当前节点下的SubMenu中匹配到
                        return subMenu->AddItemAt(pMenuItem, j);
                    }
                    ++itemIndex;
                    continue;
                }
            }
        }
    }
    //如果匹配不到，则增加到最后面
    return AddItem(pMenuItem);
}

bool MenuItem::RemoveSubMenuItem(MenuItem* pMenuItem)
{
    const size_t itemCount = GetItemCount();
    for (size_t i = 0; i < itemCount; ++i) {
        Control* pControl = GetItemAt(i);
        MenuItem* menuElementUI = dynamic_cast<MenuItem*>(pControl);
        if (menuElementUI != nullptr) {
            if (pMenuItem == menuElementUI) {
                //在当前节点下匹配到
                return RemoveItemAt(i);
            }
            continue;
        }

        menuElementUI = nullptr;
        SubMenu* subMenu = dynamic_cast<SubMenu*>(pControl);
        if (subMenu != nullptr) {
            const size_t count = subMenu->GetItemCount();
            for (size_t j = 0; j < count; ++j) {
                menuElementUI = dynamic_cast<MenuItem*>(subMenu->GetItemAt(j));
                if (menuElementUI != nullptr) {
                    if (menuElementUI == pMenuItem) {
                        //在当前节点下的SubMenu中匹配到
                        return subMenu->RemoveItemAt(j);
                    }
                    continue;
                }
            }
        }
    }
    return false;
}
bool MenuItem::RemoveSubMenuItemAt(size_t iIndex)
{
    size_t itemIndex = 0;
    const size_t itemCount = GetItemCount();
    for (size_t i = 0; i < itemCount; ++i) {
        Control* pControl = GetItemAt(i);
        MenuItem* menuElementUI = dynamic_cast<MenuItem*>(pControl);
        if (menuElementUI != nullptr) {
            if (itemIndex == iIndex) {
                //在当前节点下匹配到
                return RemoveItemAt(i);
            }
            ++itemIndex;
            continue;
        }

        menuElementUI = nullptr;
        SubMenu* subMenu = dynamic_cast<SubMenu*>(pControl);
        if (subMenu != nullptr) {
            const size_t count = subMenu->GetItemCount();
            for (size_t j = 0; j < count; ++j) {
                menuElementUI = dynamic_cast<MenuItem*>(subMenu->GetItemAt(j));
                if (menuElementUI != nullptr) {
                    if (itemIndex == iIndex) {
                        //在当前节点下的SubMenu中匹配到
                        return subMenu->RemoveItemAt(j);
                    }
                    ++itemIndex;
                    continue;
                }
            }
        }
    }
    return false;
}

void MenuItem::RemoveAllSubMenuItem()
{
    RemoveAllItems();
}

size_t MenuItem::GetSubMenuItemCount() const
{
    std::vector<MenuItem*> submenuItems;
    GetAllSubMenuItem(this, submenuItems);
    return submenuItems.size();
};

MenuItem* MenuItem::GetSubMenuItemAt(size_t iIndex) const
{
    MenuItem* foundItem = nullptr;
    std::vector<MenuItem*> submenuItems;
    GetAllSubMenuItem(this, submenuItems);
    if (iIndex < submenuItems.size()) {
        foundItem = submenuItems.at(iIndex);
    }
    return foundItem;
}

MenuItem* MenuItem::GetSubMenuItemByName(const DString& name) const
{
    std::vector<MenuItem*> submenuItems;
    GetAllSubMenuItem(this, submenuItems);
    MenuItem* subMenuItem = nullptr;
    for (auto item : submenuItems) {
        if ((item != nullptr) && (item->GetName() == name)) {
            subMenuItem = item;
            break;
        }
    }
    return subMenuItem;
}

bool MenuItem::ButtonUp(const ui::EventArgs& msg)
{
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return false;
    }
    std::weak_ptr<WeakFlag> weakFlag = pWindow->GetWeakFlag();
    bool ret = BaseClass::ButtonUp(msg);
    if (ret && !weakFlag.expired() && !msg.IsSenderExpired()) {
        //这里处理下如果有子菜单则显示子菜单
        if (!CheckSubMenuItem()){
            ContextMenuParam param;
            param.pWindow = pWindow;
            param.wParam = MenuCloseType::eMenuCloseAll;
            Menu::GetMenuObserver().RBroadcast(param);
        }
    }
    return ret;
}

bool MenuItem::MouseEnter(const ui::EventArgs& msg)
{
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return BaseClass::MouseEnter(msg);
    }
    std::weak_ptr<WeakFlag> weakFlag = pWindow->GetWeakFlag();
    bool ret = BaseClass::MouseEnter(msg);
    if (!weakFlag.expired() && IsHotState() && !msg.IsSenderExpired()) {
        //这里处理下如果有子菜单则显示子菜单
        if (!CheckSubMenuItem()) {
            ContextMenuParam param;
            param.pWindow = pWindow;
            param.wParam = MenuCloseType::eMenuCloseThis;
            Menu::GetMenuObserver().RBroadcast(param);
            //这里得把之前选中的置为未选中
            if (!weakFlag.expired() && (GetOwner() != nullptr)) {
                GetOwner()->SelectItem(Box::InvalidIndex, false, false);
            }
        }
    }
    return ret;
}

void MenuItem::PaintChild(ui::IRender* pRender, const ui::UiRect& rcPaint)
{
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        return;
    }

    for (auto item : m_items) {
        Control* pControl = item;
        if (pControl == nullptr) {
            continue;
        }

        //对于多级菜单项的内容，不绘制
        MenuItem* menuElementUI = dynamic_cast<MenuItem*>(pControl);
        if (menuElementUI != nullptr){
            continue;
        }
        SubMenu* subMenu = dynamic_cast<SubMenu*>(pControl);
        if (subMenu != nullptr) {
            continue;
        }
        
        if (!pControl->IsVisible()) {
            continue;
        }
        pControl->AlphaPaint(pRender, rcPaint);
    }
}

bool MenuItem::CheckSubMenuItem()
{
    bool hasSubMenu = false;
    for (auto item : m_items) {
        MenuItem* subMenuItem = dynamic_cast<MenuItem*>(item);
        if (subMenuItem != nullptr) {
            hasSubMenu = true;
            break;
        }
    }
    if (hasSubMenu) {
        if (GetOwner() != nullptr) {
            GetOwner()->SelectItem(GetListBoxIndex(), true, true);
        }
        if (m_pSubWindow == nullptr) {
            CreateMenuWnd();
        }
        else {
            //上次展示的子菜单窗口，尚未消失，不再展示
            hasSubMenu = false;
        }
    }
    return hasSubMenu;
}

void MenuItem::CreateMenuWnd()
{
    ASSERT(m_pSubWindow == nullptr);
    if (m_pSubWindow != nullptr) {
        return;
    }

    Window* pWindow = GetWindow();
    m_pSubWindow = new Menu(pWindow, nullptr);
    ContextMenuParam param;
    param.pWindow = pWindow;
    param.wParam = MenuCloseType::eMenuCloseThis;
    Menu::GetMenuObserver().RBroadcast(param);

    //上级级菜单窗口接口，用于同步配置信息
    Menu* pParentWindow = dynamic_cast<Menu*>(pWindow);
    ASSERT(pParentWindow != nullptr);
    if (pParentWindow != nullptr) {
        const DString skinFolder = pParentWindow->GetSkinFolder();
        m_pSubWindow->SetSkinFolder(skinFolder);
        FilePath xmlPath = pParentWindow->GetXmlPath();
        FilePath subXmlFile = FilePath(pParentWindow->m_submenuXml.c_str());
        //约定：子菜单的XML与父菜单的XML文件，在相同的目录中
        if (!xmlPath.IsEmpty()) {
            subXmlFile = FilePathUtil::JoinFilePath(xmlPath, subXmlFile);
        }
        m_pSubWindow->SetSubMenuXml(pParentWindow->m_submenuXml.c_str(), pParentWindow->m_submenuNodeName.c_str());

        //设置子菜单窗口的左上角坐标(避免子菜单弹出时出现闪黑屏现象)
        UiPoint subMenuPt;
        if (pWindow != nullptr) {
            UiRect rcOwner = GetPos();
            UiRect rc = rcOwner;
            UiPadding rcCorner = pWindow->GetCurrentShadowCorner();
            UiRect rcWindow;
            GetWindow()->GetWindowRect(rcWindow);
            //去阴影
            rcWindow.Deflate(rcCorner);
            GetWindow()->ClientToScreen(rc);
            rc.left = rcWindow.right;
            subMenuPt.x = rc.left - rcCorner.left;
            subMenuPt.y = rc.top - rcCorner.top;
        }
        m_pSubWindow->ShowMenu(subXmlFile.ToString(), subMenuPt, MenuPopupPosType::RIGHT_BOTTOM, false, this);
    }
}

void MenuItem::Activate(const EventArgs* pMsg)
{
    std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
    BaseClass::Activate(pMsg);
    if (weakFlag.expired()) {
        //在响应事件过程中，控件已经失效
        return;
    }
    DString itemName = GetName();
    size_t nItemIndex = GetListBoxIndex();
    Menu* pMenu = dynamic_cast<Menu*>(GetWindow());
    if (pMenu != nullptr) {
        DString menuName;
        if (pMenu->GetLayoutListBox() != nullptr) {
            menuName = pMenu->GetLayoutListBox()->GetName();
        }
        pMenu->OnMenuItemActivated(menuName, 0, itemName, nItemIndex);
    }
}

} // namespace ui
