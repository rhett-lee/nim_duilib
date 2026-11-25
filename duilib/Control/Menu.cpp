#include "Menu.h"
#include "MenuListBox.h"
#include "duilib/Control/MenuBar.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Core/WindowCreateAttributes.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Animation/PopupAnimation.h"
#include "duilib/Animation/AnimationPlayer.h"

#if defined (DUILIB_BUILD_FOR_SDL)
    #include <SDL3/SDL.h>
#endif

namespace ui {

//菜单Observer对象：函数局部静态对象，进程内唯一，随进程生命周期管理
ContextMenuObserver& Menu::GetMenuObserver()
{
    static ContextMenuObserver s_context_menu_observer;
    return s_context_menu_observer;
}

namespace
{
    /** 获取窗口所属菜单树的根菜单（窗口不是菜单时返回nullptr）
    *   子菜单窗口的父窗口就是上级菜单窗口，因此沿父窗口链向上即可找到根菜单
    */
    Menu* GetRootMenu(WindowBase* pWindow)
    {
        Menu* pMenu = dynamic_cast<Menu*>(pWindow);
        if (pMenu == nullptr) {
            return nullptr;
        }
        WindowBase* pParentWindow = pMenu->GetParentWindow();
        while (pParentWindow != nullptr) {
            Menu* pParentMenu = dynamic_cast<Menu*>(pParentWindow);
            if (pParentMenu == nullptr) {
                break;
            }
            pMenu = pParentMenu;
            pParentWindow = pMenu->GetParentWindow();
        }
        return pMenu;
    }

    /** 全局"点击菜单外部关闭菜单"检测的轮询周期(毫秒)
    */
    const int32_t kMenuOutsideCheckIntervalMs = 50;

    /** 全局"点击菜单外部关闭菜单"检测的状态（仅UI线程访问）
    */
    struct TMenuOutsideCheckState
    {
        size_t nTaskId = 0;             //轮询任务ID，0表示检测未启动
        bool bPointerDownLast = false;  //上次轮询时鼠标是否处于按下状态，用于识别按下边沿
    };

    TMenuOutsideCheckState& GetMenuOutsideCheckState()
    {
        static TMenuOutsideCheckState s_state;
        return s_state;
    }

    /** 判断鼠标的任意键是否处于按下状态（轮询使用，跨平台）
    *   SDL 模式下需要使用全局鼠标状态：菜单窗口可能没有焦点，点击可能落在菜单之外的窗口上，
    *   此时 SDL_GetMouseState 无法反映按键状态
    */
    bool IsMouseButtonDownAny()
    {
#if defined (DUILIB_BUILD_FOR_SDL)
        const SDL_MouseButtonFlags nButtonState = SDL_GetGlobalMouseState(nullptr, nullptr);
        return (nButtonState & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK | SDL_BUTTON_MMASK)) != 0;
#else
        return ((::GetAsyncKeyState(VK_LBUTTON) |
                 ::GetAsyncKeyState(VK_RBUTTON) |
                 ::GetAsyncKeyState(VK_MBUTTON)) & 0x8000) != 0;
#endif
    }
}

//二级或者多级子菜单的托管类
class SubMenu: public ui::ListBoxItem
{
public:
    explicit SubMenu(Window* pWindow):
        ListBoxItem(pWindow)
    {
        //默认宽度和高度均设置为auto类型
        SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
        SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
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
                if (param.pWindow.get() == pParentWindow) {
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

Menu::Menu(Window* pParentWindow, Control* pRelatedControl, MenuBar* pMenuBar):
    m_pParentWindow(pParentWindow),
    m_pRelatedControl(pRelatedControl),
    m_pMenuBar(pMenuBar),
    m_menuPoint({ 0, 0 }),
    m_popupPosType(MenuPopupPosType::RIGHT_TOP),
    m_noFocus(false),
    m_pOwner(nullptr),
    m_pListBox(nullptr)
{
    m_skinFolder = DString(DUILIB_PUBLIC_RES_DIR) + DString(_T("/menu/"));
    m_submenuXml = DString(_T("submenu.xml"));
    m_submenuNodeName = DString(_T("submenu"));
}

Menu::~Menu()
{
    if (m_pFadeInPlayer != nullptr) {
        m_pFadeInPlayer->Stop();
        m_pFadeInPlayer.reset();
    }
    //从菜单Observer中注销：窗口创建失败等场景下不会触发 OnCloseWindow，
    //若不在这里注销，全局Observer中会残留指向已释放对象的指针
    RemoveObserver();
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
    if (m_pParentWindow == nullptr) {
        //父窗口为空时，设置ToolWindow属性，避免在状态栏出现
        createWndParam.m_dwExStyle |= kWS_EX_TOOLWINDOW;
    }
    //计算创建窗口时的初始位置：先按弹出的对齐方式预估目标位置，直接以目标位置创建窗口。
    //macOS(Cocoa)在窗口显示时可能先把窗口摆到"创建位置"，再应用之后的定位；
    //若创建位置不是目标位置，就会出现"先在错误位置显示一帧，再跳转到正常位置"的抖动现象。
    //注：
    //  1. 子菜单的创建位置已经由父菜单按子菜单的锚点预计算好(point)，此处只对顶级菜单预计算；
    //  2. 菜单XML一般不需要设置size属性，窗口大小预估不到时(为0)，按锚点与阴影边距计算位置，
    //     尺寸相关的偏移与智能吸附会被跳过，窗口显示前会再按实际大小校正位置。
    UiPoint ptInitPos = point;
    if (m_pOwner == nullptr) {
        UiSize szInitWindow;
        UiPadding rcInitShadow;
        GetPrefetchWindowSize(szInitWindow, rcInitShadow);

        //坐标换算统一使用父窗口的DPI：菜单与父窗口必然在同一显示器上，
        //以父窗口为基准可避免两个窗口的DPI状态不一致导致的坐标偏差
        const DpiManager& dpi = (m_pParentWindow != nullptr) ? m_pParentWindow->Dpi() : Dpi();
        UiRect rcWork;
        if (GetMonitorWorkRect(point, rcWork) && !rcWork.IsEmpty()) {
            dpi.WindowSizeToClientSize(rcWork);
            UiPoint ptAnchor(point);
            dpi.WindowSizeToClientSize(ptAnchor);
            UiRect rcInitWnd = CalcMenuWindowRect(szInitWindow, rcInitShadow, rcWork, ptAnchor);
            dpi.ClientSizeToWindowSize(rcInitWnd);
            ptInitPos = UiPoint(rcInitWnd.left, rcInitWnd.top);
        }
    }
    createWndParam.m_nX = ptInitPos.x;
    createWndParam.m_nY = ptInitPos.y;
    CreateWnd(m_pParentWindow, createWndParam);

    //菜单淡入动画：在显示前把 Alpha 置 0，显示时由动画渐变到完全不透明
    PopupAnimation::PlayFadeIn(this, m_pFadeInPlayer);

    if (m_pMenuBar != nullptr) {
        //如果显示在MenuBar中，避免上方阴影影响鼠标滑动切换菜单功能，需要设置padding.top为0
        UiPadding rcShadowCorner = GetShadowCorner();
        rcShadowCorner.top = 0;
        SetShadowCorner(rcShadowCorner);
    }
    
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

    //启动"点击菜单外部关闭菜单"的全局检测：作为失焦关闭的兜底措施
    //（进程内只需启动一次，菜单全部关闭后自动停止）
    StartOutsideClickCheck();
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

bool Menu::OnShadowAreaMouseDown(const UiPoint& /*pt*/)
{
    //点击菜单的自绘阴影区域时，视为点击菜单外部：关闭整棵菜单树。
    //返回true，由窗口消费该鼠标消息，避免点击传递到后面的窗口，导致前台窗口被切换
    CloseMenuTree();
    return true;
}

void Menu::CloseMenuTree()
{
    //1. 关闭所有下级菜单
    ContextMenuParam param;
    param.pWindow = this;
    param.wParam = MenuCloseType::eMenuCloseThis;
    GetMenuObserver().RBroadcast(param);

    //2. 关闭自身
    CloseMenu();

    //3. 关闭所有上级菜单（上级菜单不会收到本次失焦消息，需要显式关闭）
    WindowBase* pParentWindow = GetParentWindow();
    while (pParentWindow != nullptr) {
        Menu* pParentMenu = dynamic_cast<Menu*>(pParentWindow);
        if (pParentMenu == nullptr) {
            break;
        }
        pParentMenu->CloseMenu();
        pParentWindow = pParentMenu->GetParentWindow();
    }
}

void Menu::StartOutsideClickCheck()
{
    TMenuOutsideCheckState& state = GetMenuOutsideCheckState();
    if (state.nTaskId != 0) {
        //已经在检测中
        return;
    }
    //以当前鼠标按键状态为初始值，避免把触发菜单弹出的那次点击误判为"点击菜单外部"
    //（例如菜单栏的菜单是在鼠标按下时弹出的）
    state.bPointerDownLast = IsMouseButtonDownAny();
    state.nTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        kThreadUI,
        []() { Menu::OnOutsideClickCheckTick(); },
        kMenuOutsideCheckIntervalMs);
}

void Menu::OnOutsideClickCheckTick()
{
    TMenuOutsideCheckState& state = GetMenuOutsideCheckState();
    if (state.nTaskId == 0) {
        //检测已停止
        return;
    }
    state.nTaskId = 0;

    //收集当前打开中的菜单窗口
    std::vector<Menu*> menuList;
    ContextMenuObserver::Iterator<bool, ContextMenuParam> iterator(GetMenuObserver());
    ReceiverImplBase<bool, ContextMenuParam>* pReceiver = iterator.next();
    while (pReceiver != nullptr) {
        Menu* pMenu = dynamic_cast<Menu*>(pReceiver);
        if ((pMenu != nullptr) && pMenu->IsWindow() && pMenu->IsWindowVisible()) {
            menuList.push_back(pMenu);
        }
        pReceiver = iterator.next();
    }
    if (menuList.empty()) {
        //菜单已全部关闭，停止检测
        state.bPointerDownLast = false;
        return;
    }

    //鼠标按键的按下边沿检测
    const bool bPointerDown = IsMouseButtonDownAny();
    const bool bPointerPressed = bPointerDown && !state.bPointerDownLast;
    state.bPointerDownLast = bPointerDown;
    if (bPointerPressed) {
        //获取光标位置（屏幕坐标，与GetWindowRect的坐标系一致）
        UiPoint ptCursor;
        menuList[0]->GetCursorPos(ptCursor);

        //判断点击位置是否落在某个菜单窗口内（含窗口的阴影区域）
        bool bInMenuWindow = false;
        for (Menu* pMenu : menuList) {
            UiRect rcWindow;
            pMenu->GetWindowRect(rcWindow);
            if (!rcWindow.IsEmpty() && rcWindow.ContainsPt(ptCursor)) {
                bInMenuWindow = true;
                break;
            }
        }
        if (!bInMenuWindow) {
            //点击落在所有菜单窗口之外：关闭所有菜单。
            //这是"点击菜单外部关闭菜单"的兜底措施：菜单窗口没有获得焦点(noFocus)、
            //焦点变化未被菜单感知（激活失败），或者点击被其它窗口/原生子窗口消费时，
            //仍然能够通过检测鼠标按下的位置来关闭菜单
            ContextMenuParam param;
            param.pWindow = nullptr;
            param.wParam = MenuCloseType::eMenuCloseAll;
            GetMenuObserver().RBroadcast(param);
        }
    }

    //继续下一轮检测
    state.nTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        kThreadUI,
        []() { Menu::OnOutsideClickCheckTick(); },
        kMenuOutsideCheckIntervalMs);
}

bool Menu::IsPtInBlankArea(const UiPoint& pt) const
{
    //点击菜单窗口内未命中任何控件的位置，统一视为“点击菜单外部”：
    //自绘阴影边距的判定可能因DPI、贴边、根节点边距与布局不一致而出现死区，
    //这里不再依赖阴影边距，避免出现点击菜单空白处却无法关闭菜单的问题
    UiRect rcClient;
    GetClientRect(rcClient);
    if (rcClient.IsEmpty() || !rcClient.ContainsPt(pt)) {
        return false;
    }
    return FindControl(pt) == nullptr;
}

LRESULT Menu::OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKillFocusMsg(pSetFocusWindow, nativeMsg, bHandled);
    bHandled = true;

    //当前菜单窗口正在关闭时（例如鼠标移动到同级菜单项，子菜单被关闭后正在销毁），
    //窗口销毁引起的失焦不能传播到上级菜单：
    //子菜单销毁时，系统会把焦点转移给其它窗口（可能是上级菜单，也可能是主窗口或空窗口），
    //若按"焦点转移到菜单树之外"处理，会把上级菜单甚至根菜单一起关闭
    if (IsClosingWnd()) {
        return lResult;
    }

    //焦点在同一棵菜单树内转移时（上级菜单与下级菜单之间切换），不关闭菜单；
    //注意：必须按“菜单树”判断，不能只判断新焦点窗口是否为某个菜单窗口，
    //否则另一个窗口或另一套菜单弹出时，本菜单会误认为焦点只是转给了自己的子菜单而一直不关闭
    if ((pSetFocusWindow != nullptr) && (GetRootMenu(pSetFocusWindow) == GetRootMenu(this))) {
        return lResult;
    }

    //焦点转移到菜单树之外：关闭整棵菜单树
    CloseMenuTree();
    return lResult;
}

LRESULT Menu::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (vkCode == kVK_ESCAPE) {
        bHandled = true;
        if ((m_pOwner == nullptr) && (m_pMenuBar != nullptr)) {
            //通知菜单栏，把键盘焦点还给对应的顶级菜单按钮
            m_pMenuBar->OnMenuKeyDownMsg(this, vkCode, modifierKey);
        }
        CloseMenu();
    }
    else if (vkCode == kVK_LEFT) {
        if (m_pOwner != nullptr) {
            //关闭子菜单
            bHandled = true;
            CloseMenu();
        }
        else {
            //拦截该事件，并通知MenuBar
            bHandled = true;
            if (m_pMenuBar != nullptr) {
                m_pMenuBar->OnMenuKeyDownMsg(this, vkCode, modifierKey);
            }
        }
    }
    else if (vkCode == kVK_RIGHT) {        
        ListBox* pLayoutListBox = Menu::GetLayoutListBox();
        if (pLayoutListBox != nullptr) {
            size_t index = pLayoutListBox->GetCurSel();
            MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(index));
            if (pItem != nullptr) {
                if (pItem->CheckSubMenuItem()) {
                    //展开了子菜单
                    bHandled = true;
                }
            }
        }
        if (!bHandled) {
            //拦截该事件，并通知MenuBar
            bHandled = true;
            if (m_pMenuBar != nullptr) {
                m_pMenuBar->OnMenuKeyDownMsg(this, vkCode, modifierKey);
            }
        }
    }
    else if (vkCode == kVK_RETURN || vkCode == kVK_SPACE) {
        bHandled = true;
        ListBox* pLayoutListBox = Menu::GetLayoutListBox();
        if (pLayoutListBox != nullptr) {
            size_t index = pLayoutListBox->GetCurSel();
            MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(index));
            if (pItem != nullptr) {
                if (!pItem->CheckSubMenuItem()) {
                    //回车时，激活当前选择的菜单项
                    pItem->Activate(nullptr);
                    //eMenuCloseAll 不使用 pWindow；
                    //且Activate()可能触发应用的回调（甚至销毁当前菜单窗口），
                    //因此这里不保存this，避免出现悬空指针
                    ContextMenuParam param;
                    param.pWindow = nullptr;
                    param.wParam = MenuCloseType::eMenuCloseAll;
                    Menu::GetMenuObserver().RBroadcast(param);
                }
            }
        }
    }
    else if (vkCode == kVK_DOWN || vkCode == kVK_UP) {
        bHandled = true;
        //支持键盘上下键切换当前菜单项
        ListBox* pLayoutListBox = Menu::GetLayoutListBox();
        if ((pLayoutListBox != nullptr) && (pLayoutListBox->GetItemCount() > 0)) {
            //默认选中当前处于hot状态的菜单项，以支持键盘操作
            size_t nCurSel = pLayoutListBox->GetCurSel();
            if (!Box::IsValidItemIndex(nCurSel)) {
                bool bFoundItem = false;
                for (size_t nIndex = 0; nIndex < pLayoutListBox->GetItemCount(); ++nIndex) {
                    MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(nIndex));
                    if ((pItem != nullptr) && pItem->IsVisible() && pItem->IsEnabled()) {
                        if (pItem->GetState() == ControlStateType::kControlStateHovered) {
                            pLayoutListBox->SelectItem(nIndex, false, false);
                            bFoundItem = true;
                            break;
                        }
                    }
                }
                if (!bFoundItem) {
                    //如果未找到Hovered状态的菜单项
                    if (vkCode == kVK_DOWN) {
                        //选中第一个
                        for (size_t nIndex = 0; nIndex < pLayoutListBox->GetItemCount(); ++nIndex) {
                            MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(nIndex));
                            if ((pItem != nullptr) && pItem->IsVisible() && pItem->IsEnabled()) {
                                pLayoutListBox->SelectItem(nIndex, false, false);
                                break;
                            }
                        }
                    }
                    else {
                        //选中最后一个
                        int32_t nIndex = (int32_t)pLayoutListBox->GetItemCount() - 1;
                        for (; nIndex >= 0; --nIndex) {
                            MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(nIndex));
                            if ((pItem != nullptr) && pItem->IsVisible() && pItem->IsEnabled()) {
                                pLayoutListBox->SelectItem(nIndex, false, false);
                                break;
                            }
                        }
                    }
                }
            }
            else {
                //控制选中下一个菜单项
                size_t nStartItemIndex = 0;
                std::vector<size_t> validMenuItemIndexList;
                for (size_t nIndex = 0; nIndex < pLayoutListBox->GetItemCount(); ++nIndex) {
                    MenuItem* pItem = dynamic_cast<MenuItem*>(pLayoutListBox->GetItemAt(nIndex));
                    if ((pItem != nullptr) && pItem->IsVisible() && pItem->IsEnabled()) {                        
                        if (nCurSel == nIndex) {
                            nStartItemIndex = validMenuItemIndexList.size();
                        }
                        validMenuItemIndexList.push_back(nIndex);
                    }
                }

                if (vkCode == kVK_DOWN) {
                    size_t nNextMenuItemIndex = nStartItemIndex + 1;
                    if (nNextMenuItemIndex >= validMenuItemIndexList.size()) {
                        nNextMenuItemIndex = 0;
                    }
                    if (nNextMenuItemIndex < validMenuItemIndexList.size()) {
                        pLayoutListBox->SelectItem(validMenuItemIndexList[nNextMenuItemIndex], false, false);
                    }
                }
                else {
                    size_t nNextMenuItemIndex = nStartItemIndex - 1;
                    if (nStartItemIndex == 0) {
                        nNextMenuItemIndex = validMenuItemIndexList.size() - 1;
                    }
                    if (nNextMenuItemIndex >= validMenuItemIndexList.size()) {
                        nNextMenuItemIndex = 0;
                    }
                    if (nNextMenuItemIndex < validMenuItemIndexList.size()) {
                        pLayoutListBox->SelectItem(validMenuItemIndexList[nNextMenuItemIndex], false, false);
                    }
                }
            }
        }
    }
    if (bHandled) {
        //已经处理
        return 0;
    }
    else {
        return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    }
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

void Menu::GetPrefetchWindowSize(UiSize& szWindow, UiPadding& rcShadow)
{
    szWindow.Clear();
    rcShadow.Clear();

    //解析窗口XML属性（解析结果在窗口内部缓存，CreateWnd时会复用，不会重复解析）
    WindowCreateAttributes createAttributes;
    GetCreateWindowAttributes(createAttributes);

    //此时窗口尚未创建，坐标换算使用父窗口的DPI
    const DpiManager& dpi = (m_pParentWindow != nullptr) ? m_pParentWindow->Dpi() : Dpi();

    //阴影边距为DIP值，需要换算为布局坐标
    rcShadow = createAttributes.m_rcShadowCorner;
    dpi.ScalePadding(rcShadow);

    //菜单XML一般不需要设置size属性，因此窗口大小是可选的：
    //取不到时返回(0,0)表示大小未知，由调用方跳过与尺寸相关的计算
    if ((createAttributes.m_szInitSize.cx > 0) && (createAttributes.m_szInitSize.cy > 0)) {
        //窗口大小（含阴影区域）为窗口坐标，需要换算为布局坐标
        szWindow = createAttributes.m_szInitSize;
        dpi.WindowSizeToClientSize(szWindow);
    }
}

UiRect Menu::CalcMenuWindowRect(const UiSize& szMenuWindow, const UiPadding& rcShadowCorner,
                                const UiRect& rcWork, const UiPoint& ptAnchor) const
{
    //窗口大小可能未知(菜单XML未定义size时为0)：此时只能按锚点与阴影边距估算位置，
    //需要跳过与尺寸相关的偏移(右侧/底部对齐)和智能吸附，避免计算出无效位置
    const bool bWidthKnown = szMenuWindow.cx > 0;
    const bool bHeightKnown = szMenuWindow.cy > 0;

    UiSize szMenuClient = szMenuWindow;
    szMenuClient.cx -= rcShadowCorner.left + rcShadowCorner.right;
    szMenuClient.cy -= rcShadowCorner.top + rcShadowCorner.bottom; //这里去掉阴影窗口，即用户的视觉有效面积

    UiPoint point(ptAnchor);  //这里有个bug，由于坐标点与包含在窗口内，会直接触发mouseenter导致出来子菜单，偏移1个像素
    if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Right)) {
        if (bWidthKnown) {
            point.x += -szMenuWindow.cx + rcShadowCorner.right + rcShadowCorner.left;
            point.x -= 1;
        }
    }
    else if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Left)) {
        point.x += 1;
    }
    if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Bottom)) {
        if (bHeightKnown) {
            point.y += -szMenuWindow.cy + rcShadowCorner.bottom + rcShadowCorner.top;
            point.y += 1;
        }
    }
    else if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Top)) {
        point.y += 1;
    }
    if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Intelligent)) {
        if (bWidthKnown) {
            if (point.x < rcWork.left) {
                point.x = rcWork.left;
            }
            else if (point.x + szMenuClient.cx > rcWork.right) {
                point.x = rcWork.right - szMenuClient.cx;
            }
        }
        if (bHeightKnown) {
            if (point.y < rcWork.top) {
                point.y = rcWork.top;
            }
            else if (point.y + szMenuClient.cy > rcWork.bottom) {
                point.y = rcWork.bottom - szMenuClient.cy;
            }
        }
    }

    const int32_t x = point.x - rcShadowCorner.left;
    const int32_t y = point.y - rcShadowCorner.top;
    return UiRect(x, y, x + szMenuWindow.cx, y + szMenuWindow.cy);
}

UiRect Menu::CalcSubMenuWindowRect(const UiSize& szSubMenuWindow, const UiPadding& rcCorner,
                                   const UiRect& rcWork, const DpiManager& dpi) const
{
    ASSERT(m_pOwner != nullptr);
    if (m_pOwner == nullptr) {
        return UiRect();
    }
    Window* pOwnerWindow = m_pOwner->GetWindow();
    if (pOwnerWindow == nullptr) {
        return UiRect();
    }

    // Position the popup window in absolute space
    UiRect rcOwner = m_pOwner->GetPos();
    UiRect rc = rcOwner;

    UiRect rcWindow;
    pOwnerWindow->GetWindowRect(rcWindow);
    dpi.WindowSizeToClientSize(rcWindow);

    int32_t cxFixed = szSubMenuWindow.cx - rcCorner.left - rcCorner.right;
    int32_t cyFixed = szSubMenuWindow.cy - rcCorner.top - rcCorner.bottom;
    if (cxFixed < 0) {
        cxFixed = 0;
    }
    if (cyFixed < 0) {
        cyFixed = 0;
    }
    if (szSubMenuWindow.cx < (rcCorner.left + rcCorner.right)) {
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

    pOwnerWindow->ClientToScreen(rc);
    dpi.WindowSizeToClientSize(rc);

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
            dpi.WindowSizeToClientSize(rcPreWindow);

            bReachRight = (rcPreWindow.left + rcCorner.left) >= rcWindow.right;
            bReachBottom = (rcPreWindow.top + rcCorner.top) >= rcWindow.bottom;
            if (pContextMenu->GetWindowHandle() == pOwnerWindow->GetWindowHandle()
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

    const int32_t nNewWidth = rc.Width() + rcCorner.left + rcCorner.right;
    const int32_t nNewHeight = rc.Height() + rcCorner.top + rcCorner.bottom;
    const int32_t x = rc.left - rcCorner.left;
    const int32_t y = rc.top - rcCorner.top;
    return UiRect(x, y, x + nNewWidth, y + nNewHeight);
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
    Dpi().WindowSizeToClientSize(rcWork);
    Dpi().WindowSizeToClientSize(m_menuPoint);

    ui::UiSize szMenuWindow = { rcWork.Width(), rcWork.Height()};
    UiEstSize estSize = pRoot->EstimateSize(szMenuWindow);   //这里返回的大小包含了阴影的大小
    if (estSize.cx.IsInt32()) {
        szMenuWindow.cx = estSize.cx.GetInt32();
    }
    if (estSize.cy.IsInt32()) {
        szMenuWindow.cy = estSize.cy.GetInt32();
    }

    UiPadding rcShadowCorner = pRoot->GetPadding(); //窗口阴影所占区域
    UiRect rcWindow = CalcMenuWindowRect(szMenuWindow, rcShadowCorner, rcWork, m_menuPoint);
    Dpi().ClientSizeToWindowSize(rcWindow);
    Dpi().ClientSizeToWindowSize(m_menuPoint);

    SetWindowPos(InsertAfterWnd(InsertAfterFlag::kHWND_TOPMOST),
                 rcWindow.left, rcWindow.top,
                 rcWindow.Width(), rcWindow.Height(),
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
    if (m_pOwner->GetWindow() == nullptr) {
        return false;
    }

    //获取子菜单窗口的当前大小（含阴影区域）
    UiRect rcClient;
    GetClientRect(rcClient);
    const UiSize szSubMenuWindow(rcClient.Width(), rcClient.Height());
    const UiPadding rcCorner = GetCurrentShadowCorner();

    UiRect rcWork;
    GetMonitorWorkRect(m_menuPoint, rcWork);
    Dpi().WindowSizeToClientSize(rcWork);

    //计算子菜单窗口的显示位置
    UiRect rcWindow = CalcSubMenuWindowRect(szSubMenuWindow, rcCorner, rcWork, Dpi());
    Dpi().ClientSizeToWindowSize(rcWindow);

    //调整窗口位置，显示窗口，但不调整窗口的大小
    SetWindowPos(InsertAfterWnd(InsertAfterFlag::kHWND_TOPMOST),
                 rcWindow.left, rcWindow.top,
                 rcWindow.Width(), rcWindow.Height(),
                 kSWP_SHOWWINDOW | kSWP_NOSIZE | (m_noFocus ? kSWP_NOACTIVATE : 0));

    if (!m_noFocus) {
        SetWindowForeground();
        SetFocusControl(Menu::GetLayoutListBox());
    }
    return true;
}

void Menu::PreInitWindow()
{
    //需要先调用基类的实现函数
    BaseClass::PreInitWindow();

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
        m_pRelatedControl->SetState(kControlStatePressed);
    }
}

Control* Menu::GetRelatedControl() const
{
    return m_pRelatedControl.get();
}

void Menu::RefreshMenuLayout()
{
    if (m_pOwner != nullptr) {
        ResizeSubMenu();
    }
    else {
        ResizeMenu();
    }
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

void Menu::ClearMenuItemActivated()
{
    m_callbackList.clear();
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

    if (m_pRelatedControl != nullptr) {
        //恢复关联控件的状态
        UiPoint pt;
        GetCursorPos(pt);
        m_pRelatedControl->ScreenToClient(pt);
        pt.Offset(m_pRelatedControl->GetScrollOffsetInScrollBox());
        if (m_pRelatedControl->GetRect().ContainsPt(pt)) {
            if (m_pRelatedControl->GetState() != ui::kControlStateHovered) {
                m_pRelatedControl->SetState(ui::kControlStateHovered);
            }            
        }
        else {
            if (m_pRelatedControl->GetState() != ui::kControlStateNormal) {
                m_pRelatedControl->SetState(ui::kControlStateNormal);
            }
        }
    }
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
                return true;
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
    if (!weakFlag.expired() && IsHoveredState() && !msg.IsSenderExpired()) {
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
        if ((dynamic_cast<MenuItem*>(item) != nullptr) ||
            (dynamic_cast<SubMenu*>(item) != nullptr)) {
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
        bool bSubMenuXmlChecked = false;
        if (!xmlPath.IsEmpty()) {
            if (GlobalManager::Instance().Theme().IsResFileExists(FilePathUtil::JoinFilePath(xmlPath, subXmlFile), FilePath(skinFolder))) {
                subXmlFile = FilePathUtil::JoinFilePath(xmlPath, subXmlFile);
                bSubMenuXmlChecked = true;
            }
        }
        if (!bSubMenuXmlChecked) {
            if (!GlobalManager::Instance().Theme().IsResFileExists(subXmlFile, FilePath(skinFolder))) {
                //在公共资源目录的public/menu目录中查找submenu.xml文件
                const FilePath publicMenuPath(DString(DUILIB_PUBLIC_RES_DIR) + _T("/menu")); //"public/menu"
                if (GlobalManager::Instance().Theme().IsResFileExists(FilePathUtil::JoinFilePath(publicMenuPath, subXmlFile), FilePath())) {
                    subXmlFile = FilePathUtil::JoinFilePath(publicMenuPath, subXmlFile);
                    bSubMenuXmlChecked = true;
                }
            }
        }
        m_pSubWindow->SetSubMenuXml(pParentWindow->m_submenuXml.c_str(), pParentWindow->m_submenuNodeName.c_str());

        //设置子菜单窗口的左上角坐标(避免子菜单弹出时出现闪黑屏现象)
        UiPoint subMenuPt;
        if (pWindow != nullptr) {
            UiRect rcOwner = GetPos();
            UiRect rc = rcOwner;
            //GetWindowRect返回的是屏幕坐标(窗口坐标)，阴影边距是布局坐标(物理像素)，
            //需要先把阴影边距换算到窗口坐标，否则像素密度>1时子菜单初始位置会偏移
            UiPadding rcCorner = pWindow->GetCurrentShadowCorner();
            UiPadding rcCornerWindow = rcCorner;
            pWindow->Dpi().ClientSizeToWindowSize(rcCornerWindow);
            UiRect rcWindow;
            GetWindow()->GetWindowRect(rcWindow);
            //去阴影
            rcWindow.Deflate(rcCornerWindow);
            GetWindow()->ClientToScreen(rc);
            rc.left = rcWindow.right;
            subMenuPt.x = rc.left - rcCornerWindow.left;
            subMenuPt.y = rc.top - rcCornerWindow.top;
        }
        m_pSubWindow->ShowMenu(subXmlFile.ToString(), subMenuPt, MenuPopupPosType::RIGHT_BOTTOM, false, this);
        if (!m_pSubWindow->IsWindow()) {
            //子菜单XML不存在或创建失败：此时不会触发 OnFinalMessage，
            //需要注销Observer并主动释放对象，否则全局Observer中残留悬空指针，且该菜单项之后无法再展开子菜单
            m_pSubWindow->RemoveObserver();
            delete m_pSubWindow;
            m_pSubWindow = nullptr;
        }
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

UiEstSize MenuItem::EstimateSize(UiSize szAvailable)
{
    //代码与Box::EstimateSize函数一致，但将m_items里面的SubMenu过滤掉
    bool bRemoved = false;
    std::vector<Control*> items = m_items;
    auto iter = m_items.begin();
    while (iter != m_items.end()) {
        if (dynamic_cast<SubMenu*>(*iter) != nullptr) {
            iter = m_items.erase(iter);
            bRemoved = true;
        }
        else {
            ++iter;
        }
    }
    UiEstSize estSize = BaseClass::EstimateSize(szAvailable);
    if (bRemoved) {
        m_items.swap(items);
    }
    return estSize;
}

} // namespace ui
