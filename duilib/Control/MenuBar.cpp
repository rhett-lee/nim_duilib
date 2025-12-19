#include "MenuBar.h"
#include "duilib/Control/Button.h"

namespace ui
{
class MenuBarButton : public ButtonBox
{
    typedef ButtonBox BaseClass;
public:
    explicit MenuBarButton(MenuBar* pMenuBar);
public:
    //鼠标事件
    virtual bool MouseEnter(const EventArgs& msg) override;
    virtual bool MouseLeave(const EventArgs& msg) override;
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;

private:
    MenuBar* m_pMenuBar;
};

MenuBarButton::MenuBarButton(MenuBar* pMenuBar) :
    ButtonBox(pMenuBar->GetWindow()),
    m_pMenuBar(pMenuBar)
{
    //使用RichText模式
    //SetRichText(true);
}

bool MenuBarButton::MouseEnter(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseEnter(msg);
    m_pMenuBar->OnMenuMouseEnter(this, msg);
    return bRet;
}

bool MenuBarButton::MouseLeave(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseLeave(msg);
    m_pMenuBar->OnMenuMouseLeave(this, msg);
    return bRet;
}

bool MenuBarButton::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    m_pMenuBar->OnMenuMouseButtonDown(this, msg);
    return bRet;
}

bool MenuBarButton::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    m_pMenuBar->OnMenuMouseButtonUp(this, msg);
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
MenuBar::MenuBar(Window* pWindow):
    HBox(pWindow),
    m_nItemDataId(0),
    m_bActiveState(false),
    m_bEnableBtnActive(false)
{
}

DString MenuBar::GetType() const { return DUI_CTR_MENU_BAR; }

void MenuBar::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //初始化
    for (TopMenuData& menuData : m_topMenuList) {
        menuData.m_nItemDataId = ++m_nItemDataId;
        AddTopMenuToUI(menuData, GetItemCount());
    }
}

void MenuBar::AddTopMenuToUI(const TopMenuData& menuData, size_t nInsertItem)
{
    MenuBarButton* pNewItem = new MenuBarButton(this);
    if (!menuData.m_menuTextId.empty()) {
        pNewItem->SetTextId(menuData.m_menuTextId.c_str());
    }
    else {
        pNewItem->SetText(menuData.m_menuText.c_str());
    }
    bool bAdded = AddItemAt(pNewItem, nInsertItem);
    ASSERT(bAdded);
    if (bAdded) {
        pNewItem->SetUserDataID(menuData.m_nItemDataId);
        if (!menuData.m_menuTextButtonClass.empty()) {
            pNewItem->SetClass(menuData.m_menuTextButtonClass.c_str());
        }
        else {
            //使用默认值
            pNewItem->SetClass(_T("menu_bar_button"));
        }
        if (!menuData.m_menuTextButtonAttributes.empty()) {
            pNewItem->ApplyAttributeList(menuData.m_menuTextButtonAttributes.c_str());
        }
    }

    pNewItem->AttachClick([this, pNewItem](const EventArgs&) {
        //按钮点击时
        if (m_bEnableBtnActive) {
            ShowPopupMenu(pNewItem);
        }        
        return true;
        });
}

void MenuBar::RemoveTopMenuFromUI(const TopMenuData& menuData)
{
    const size_t nItemCount = GetItemCount();
    for (size_t nItem = 0; nItem < nItemCount; ++nItem) {
        Control* pItem = GetItemAt(nItem);
        if ((pItem != nullptr) && (pItem->GetUserDataID() == menuData.m_nItemDataId)) {
            RemoveItem(pItem);
            break;
        }
    }
}

int32_t MenuBar::AddTopMenu(const DString& menuItemId,
                            const DString& menuText,
                            const DString& menuTextId,
                            const DString& menuXmlPath,
                            const DString& menuTextButtonClass,
                            const DString& menuTextButtonAttributes)
{
    return InsertTopMenu((int32_t)m_topMenuList.size(), menuItemId, menuText, menuTextId, menuXmlPath, menuTextButtonClass, menuTextButtonAttributes);
}

int32_t MenuBar::AddTopMenu(const MenuBarItem& menuBarItem)
{
    return InsertTopMenu((int32_t)m_topMenuList.size(), menuBarItem);
}

int32_t MenuBar::InsertTopMenu(int32_t nMenuIndex, const MenuBarItem& menuBarItem)
{
    return InsertTopMenu(nMenuIndex,
                         menuBarItem.m_menuItemId,
                         menuBarItem.m_menuText,
                         menuBarItem.m_menuTextId,
                         menuBarItem.m_menuXmlPath,
                         menuBarItem.m_menuTextButtonClass,
                         menuBarItem.m_menuTextButtonAttributes);
}

int32_t MenuBar::InsertTopMenu(int32_t nMenuIndex,
                               const DString& menuItemId,
                               const DString& menuText,
                               const DString& menuTextId,
                               const DString& menuXmlPath,
                               const DString& menuTextButtonClass,
                               const DString& menuTextButtonAttributes)
{
    ASSERT(!(menuText.empty() && menuTextId.empty()) && !menuXmlPath.empty());
    if ((menuText.empty() && menuTextId.empty()) || menuXmlPath.empty()) {
        return -1;
    }
    if ((nMenuIndex < 0) || (nMenuIndex > (int32_t)m_topMenuList.size())) {
        nMenuIndex = (int32_t)m_topMenuList.size();
    }

    TopMenuData menuData;
    menuData.m_menuItemId = menuItemId;
    menuData.m_menuText = menuText;
    menuData.m_menuTextId = menuTextId;
    menuData.m_menuXmlPath = menuXmlPath;
    menuData.m_menuTextButtonClass = menuTextButtonClass;
    menuData.m_menuTextButtonAttributes = menuTextButtonAttributes;
    if (IsInited()) {
        menuData.m_nItemDataId = ++m_nItemDataId;
    }
    int32_t nRetIndex = 0;
    if (nMenuIndex < (int32_t)m_topMenuList.size()) {
        m_topMenuList.insert(m_topMenuList.begin() + nMenuIndex, menuData);
        nRetIndex = nMenuIndex;
    }
    else {
        m_topMenuList.push_back(menuData);
        nRetIndex = (int32_t)m_topMenuList.size() - 1;
    }
    if (IsInited()) {
        //添加到界面
        AddTopMenuToUI(menuData, nMenuIndex);
    }
    return nRetIndex;
}

bool MenuBar::GetTopMenu(int32_t nMenuIndex, MenuBarItem& menuBarItem)
{
    if ((nMenuIndex >= 0) && (nMenuIndex < (int32_t)m_topMenuList.size())) {
        const TopMenuData& menuData = m_topMenuList[nMenuIndex];
        menuBarItem.m_menuItemId = menuData.m_menuItemId.c_str();
        menuBarItem.m_menuText = menuData.m_menuText.c_str();
        menuBarItem.m_menuTextId = menuData.m_menuTextId.c_str();
        menuBarItem.m_menuXmlPath = menuData.m_menuXmlPath.c_str();
        menuBarItem.m_menuTextButtonClass = menuData.m_menuTextButtonClass.c_str();
        menuBarItem.m_menuTextButtonAttributes = menuData.m_menuTextButtonAttributes.c_str();
        return true;
    }
    return false;
}

bool MenuBar::RemoveTopMenu(int32_t nMenuIndex)
{
    if ((nMenuIndex >= 0) && (nMenuIndex < (int32_t)m_topMenuList.size())) {
        TopMenuData menuData = m_topMenuList[nMenuIndex];        
        m_topMenuList.erase(m_topMenuList.begin() + nMenuIndex);
        //从界面中删除
        RemoveTopMenuFromUI(menuData);
        return true;
    }
    return false;
}

bool MenuBar::SetActiveTopMenuIndex(int32_t nMenuIndex)
{
    ASSERT((nMenuIndex >= 0) && (nMenuIndex < (int32_t)m_topMenuList.size()));
    if ((nMenuIndex >= 0) && (nMenuIndex < (int32_t)m_topMenuList.size())) {
        const TopMenuData& menuData = m_topMenuList[nMenuIndex];
        for (size_t nIndex = 0; nIndex < GetItemCount(); ++nIndex) {
            MenuBarButton* pItem = dynamic_cast<MenuBarButton*>(GetItemAt(nIndex));
            if ((pItem != nullptr) && pItem->IsVisible() && pItem->IsEnabled()) {
                if (menuData.m_nItemDataId == pItem->GetUserDataID()) {
                    m_bEnableBtnActive = true;
                    pItem->Activate(nullptr);
                    m_bEnableBtnActive = false;
                    return true;
                }
            }
        }
    }
    return false;
}

int32_t MenuBar::GetActiveTopMenuIndex() const
{
    if ((m_pActiveMenu != nullptr) && m_pActiveMenu->IsWindow() && !m_pActiveMenu->IsClosingWnd()) {
        Control* pRelatedControl = m_pActiveMenu->GetRelatedControl();
        if (pRelatedControl != nullptr) {
            MenuBarButton* pItem = dynamic_cast<MenuBarButton*>(pRelatedControl);
            if (pItem != nullptr) {
                for (int32_t nMenuIndex = 0; nMenuIndex < (int32_t)m_topMenuList.size(); ++nMenuIndex) {
                    const TopMenuData& menuData = m_topMenuList[nMenuIndex];
                    if (menuData.m_nItemDataId == pItem->GetUserDataID()) {
                        return nMenuIndex;
                    }
                }
            }
        }
    }
    return -1;
}

void MenuBar::AttachMenuBarItemActivated(MenuBarItemActivatedEvent callback)
{
    if (callback != nullptr) {
        m_callbackList.push_back(callback);
    }
}

void MenuBar::ClearMenuBarItemActivated()
{
    m_callbackList.clear();
}

void MenuBar::CheckShowPopupMenu(MenuBarButton* pButton)
{
    if (HasActivePopupMenu()) {
        bool bPopup = true;
        if ((m_pActiveMenu != nullptr) && (m_pActiveMenu->GetRelatedControl() == pButton)) {
            //已经弹出，不需要重复弹出
            bPopup = false;
            if ((pButton != nullptr) && (pButton->GetState() != ui::kControlStatePushed)) {
                pButton->SetState(ui::kControlStatePushed);
            }
        }
        if (bPopup) {
            ShowPopupMenu(pButton);
        }
    }
}

void MenuBar::OnMenuMouseEnter(MenuBarButton* pButton, const EventArgs& /*msg*/)
{
    if (pButton == nullptr) {
        return;
    }
    size_t nItemDataId = pButton->GetUserDataID();
    for (TopMenuData& menuData : m_topMenuList) {
        if (menuData.m_nItemDataId == nItemDataId) {
            if (menuData.m_bMouseEnter) {
                //已经执行过，不重复执行弹出菜单的操作
                return;
            }
            menuData.m_bMouseEnter = true;
        }
        else {
            menuData.m_bMouseEnter = false;
        }
    }

    //检查并弹出菜单
    CheckShowPopupMenu(pButton);
}

void MenuBar::OnMenuMouseLeave(MenuBarButton* pButton, const EventArgs& /*msg*/)
{
    if (HasActivePopupMenu()) {
        if ((m_pActiveMenu != nullptr) && (m_pActiveMenu->GetRelatedControl() == pButton)) {
            //已经弹出菜单，保持状态不变
            if ((pButton != nullptr) && (pButton->GetState() != ui::kControlStatePushed)) {
                pButton->SetState(ui::kControlStatePushed);
            }
        }
    }
}

void MenuBar::OnMenuMouseButtonDown(MenuBarButton* pButton, const EventArgs& /*msg*/)
{
    m_bActiveState = !m_bActiveState;//鼠标按下时，开关效果
    if (m_bActiveState) {
        //弹出菜单
        ShowPopupMenu(pButton);
    }
    else {
        //已经弹出，收起
        HidePopupMenu(pButton);
    }
}

void MenuBar::OnMenuMouseButtonUp(MenuBarButton* /*pButton*/, const EventArgs& /*msg*/)
{
}

void MenuBar::ShowPopupMenu(MenuBarButton* pButton)
{
    ASSERT(pButton != nullptr);
    if (pButton == nullptr) {
        return;
    }
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return;
    }
    TopMenuData topMenuData;
    bool bFoundMenu = false;
    size_t nItemDataId = pButton->GetUserDataID();
    for (const TopMenuData& menuData : m_topMenuList) {
        if (menuData.m_nItemDataId == nItemDataId) {
            bFoundMenu = true;
            topMenuData = menuData;
        }
    }
    ASSERT(bFoundMenu);
    if (!bFoundMenu) {
        return;
    }

    UiPoint point(pButton->GetRect().left, pButton->GetRect().top);
    point.Offset(0, pButton->GetRect().Height());
    ClientToScreen(point);

    //如果已经展开则关闭
    HidePopupMenu(pButton);

    Menu* pMenu = new ui::Menu(pWindow, pButton, this);//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    m_pActiveMenu = pMenu;

    pMenu->SetSkinFolder(pWindow->GetResourcePath().ToString());
    pMenu->ShowMenu(topMenuData.m_menuXmlPath.c_str(), point);
    pMenu->SetWindowFocus();

    std::weak_ptr<WeakFlag> menuBarFlag = GetWeakFlag();
    //菜单命令事件响应
    DString menuItemId = topMenuData.m_menuItemId.c_str();
    MenuItemActivatedEvent callback = [this, menuBarFlag, menuItemId](const DString& menuName, int32_t nMenuLevel,
                                                                      const DString& itemName, size_t nItemIndex) {
            //菜单命令激活，通知应用层
            if (!menuBarFlag.expired()) {
                DString activeMenuName = menuName;
                int32_t activeMenuLevel = nMenuLevel;
                DString activeItemName = itemName;
                size_t activeItemIndex = nItemIndex;
                std::vector<MenuBarItemActivatedEvent> callbackList(m_callbackList);
                for (MenuBarItemActivatedEvent callback : callbackList) {
                    if (callback) {
                        callback(menuItemId,
                            activeMenuName, activeMenuLevel,
                            activeItemName, activeItemIndex);
                    }
                }
            }
        };
    pMenu->AttachMenuItemActivated(callback);

    //菜单关闭时，复位激活状态
    pMenu->AttachWindowClose([this, pMenu, menuBarFlag](const EventArgs& /*args*/) {
            if (!menuBarFlag.expired() && (m_pActiveMenu == pMenu)) {
                m_bActiveState = false;
            }
            return true;
        });
}

void MenuBar::HidePopupMenu(MenuBarButton* /*pButton*/)
{
    if ((m_pActiveMenu != nullptr) && m_pActiveMenu->IsWindow() && !m_pActiveMenu->IsClosingWnd()) {
        m_pActiveMenu->CloseMenu();        
    }
    m_pActiveMenu = nullptr;
}

bool MenuBar::HasActivePopupMenu() const
{
    if ((m_pActiveMenu != nullptr) && m_pActiveMenu->IsWindow() && !m_pActiveMenu->IsClosingWnd()) {
        return true;
    }
    return false;
}

void MenuBar::OnMenuKeyDownMsg(Menu* pMenu, VirtualKeyCode vkCode, uint32_t /*modifierKey*/)
{
    if ((pMenu == nullptr) || !HasActivePopupMenu()) {
        return;
    }
    size_t nStartItemIndex = 0;
    std::vector<size_t> itemIndexList;
    for (size_t nIndex = 0; nIndex < GetItemCount(); ++nIndex) {
        MenuBarButton* pItem = dynamic_cast<MenuBarButton*>(GetItemAt(nIndex));
        if ((pItem != nullptr) && pItem->IsVisible() && pItem->IsEnabled()) {
            itemIndexList.push_back(nIndex);
            if (pMenu->GetRelatedControl() == pItem) {
                nStartItemIndex = nIndex;
            }
        }
    }
    if (vkCode == kVK_RIGHT) {

        //TODO: 测试代码，完成
        //InsertTopMenu(100, _T("  1...1  "), _T("public/menu/rich_edit_menu.xml"));

        ////TODO: 测试代码，完成
        //DString menuText;
        //DString menuXmlPath;
        //DString menuTextButtonClass;
        //DString menuTextButtonAttributes;
        //bool bRet = GetTopMenu(5, menuText, menuXmlPath, menuTextButtonClass, menuTextButtonAttributes);

        //TODO: 测试代码，完成
        //RemoveTopMenu(3);
        //return;

        //TODO: 测试代码，完成
        //SetActiveTopMenuIndex(3);
        //int32_t n = GetActiveTopMenuIndex();
        //ASSERT(n == 3);
        //return;

        size_t nNextMenuIndex = nStartItemIndex + 1;
        if (nNextMenuIndex >= itemIndexList.size()) {
            nNextMenuIndex = 0;
        }
        if (nNextMenuIndex < itemIndexList.size()) {
            MenuBarButton* pItem = dynamic_cast<MenuBarButton*>(GetItemAt(nNextMenuIndex));
            if (pItem != nullptr) {
                HidePopupMenu(nullptr);
                m_bEnableBtnActive = true;
                pItem->Activate(nullptr);
                m_bEnableBtnActive = false;
            }
        }
    }
    else if (vkCode == kVK_LEFT) {
        size_t nNextMenuIndex = nStartItemIndex - 1;
        if (nStartItemIndex == 0) {
            nNextMenuIndex = itemIndexList.size() - 1;
        }
        if (nNextMenuIndex >= itemIndexList.size()) {
            nNextMenuIndex = 0;
        }
        if (nNextMenuIndex < itemIndexList.size()) {
            MenuBarButton* pItem = dynamic_cast<MenuBarButton*>(GetItemAt(nNextMenuIndex));
            if (pItem != nullptr) {
                HidePopupMenu(nullptr);
                m_bEnableBtnActive = true;
                pItem->Activate(nullptr);
                m_bEnableBtnActive = false;
            }
        }
    }
}

} // namespace ui
