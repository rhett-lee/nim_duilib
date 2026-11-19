#include "MenuBar.h"
#include "duilib/Control/Button.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

namespace ui
{
namespace
{
    //鼠标在有展开菜单的菜单栏上悬停切换的延迟时间（毫秒），避免快速划过时频繁重建弹层
    constexpr uint32_t kHoverSwitchDelayMs = 150;
}

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

    /** 消息处理：回车键、空格键或者向下键激活时，打开或者收起顶级菜单
    */
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 设置是否为键盘操作获得焦点（只有键盘操作时才绘制焦点框）
    */
    void SetKeyboardFocused(bool bKeyboardFocused)
    {
        if (m_bKeyboardFocus != bKeyboardFocused) {
            m_bKeyboardFocus = bKeyboardFocused;
            Invalidate();
        }
    }

    //是否由键盘操作获得焦点
    bool IsKeyboardFocused() const { return m_bKeyboardFocus; }

protected:
    /** 是否绘制焦点框：仅键盘操作获得焦点，并且窗口处于激活状态时绘制
    */
    virtual bool ShouldPaintFocusRect() const override;

    /** 绘制焦点框：使用圆角强调色描边，替代默认的虚线矩形
    */
    virtual void DoPaintFocusRect(IRender* pRender) override;

private:
    MenuBar* m_pMenuBar;
    bool m_bKeyboardFocus;  //是否由键盘操作获得焦点（鼠标点击时为false）
};

MenuBarButton::MenuBarButton(MenuBar* pMenuBar) :
    ButtonBox(pMenuBar->GetWindow()),
    m_pMenuBar(pMenuBar),
    m_bKeyboardFocus(false)
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
    //鼠标操作获得焦点时，不显示焦点框
    m_bKeyboardFocus = false;
    bool bRet = BaseClass::ButtonDown(msg);
    if (IsActivatable()) {
        //鼠标按下时切换菜单（此时菜单窗口可能刚因失焦而关闭，需要按关联关系判断）
        m_pMenuBar->TogglePopupMenu(this);
    }
    return bRet;
}

bool MenuBarButton::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if (m_pMenuBar->IsPopupMenuOf(this)) {
        //菜单处于展开状态时，按钮保持按下状态
        SetState(kControlStatePressed);
    }
    return bRet;
}

void MenuBarButton::HandleEvent(const EventArgs& msg)
{
    if ((msg.eventType == kEventKeyDown) && IsEnabled() && IsKeyboardEnabled()) {
        //键盘操作获得焦点时，显示焦点框
        m_bKeyboardFocus = true;
        if ((msg.vkCode == kVK_SPACE) || (msg.vkCode == kVK_RETURN)) {
            //键盘回车/空格激活，与鼠标点击走同一入口
            m_pMenuBar->TogglePopupMenu(this);
            return;
        }
        else if (msg.vkCode == kVK_DOWN) {
            //向下键展开菜单（不收起已展开的菜单）
            m_pMenuBar->ShowPopupMenu(this);
            return;
        }
        else if ((msg.vkCode == kVK_LEFT) || (msg.vkCode == kVK_RIGHT)) {
            //左右方向键：在各个顶级菜单按钮之间切换键盘焦点
            m_pMenuBar->FocusAdjacentTopMenuButton(this, msg.vkCode == kVK_RIGHT);
            return;
        }
    }
    BaseClass::HandleEvent(msg);
}

bool MenuBarButton::ShouldPaintFocusRect() const
{
    //只有通过键盘操作(如F10、Alt、回车等)获得焦点时才显示焦点框
    return m_bKeyboardFocus && BaseClass::ShouldPaintFocusRect();
}

void MenuBarButton::DoPaintFocusRect(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    //圆角描边样式的焦点框：贴合按钮的圆角，使用强调色，比默认虚线矩形更美观
    UiRect rcFocusRect = GetRect();
    const int32_t nInset = Dpi().GetScaleInt(1);
    rcFocusRect.Deflate(nInset, nInset);
    if (rcFocusRect.IsEmpty()) {
        return;
    }
    UiColor borderColor = GetUiColor(GetFocusedRectColor());
    if (borderColor.IsEmpty()) {
        borderColor = GetUiColor(_T("color_accent"));
    }
    if (borderColor.IsEmpty()) {
        borderColor = GetUiColor(_T("border_focus_ring"));
    }
    if (borderColor.IsEmpty()) {
        borderColor = GetUiColor(_T("text_menu_bar"));
    }
    if (borderColor.IsEmpty()) {
        borderColor = UiColor(UiColors::Gray);
    }
    float fWidth = Dpi().GetScaleFloat(1.5f);
    if (fWidth < 1.0f) {
        fWidth = 1.0f;
    }
    const float fRadius = (float)Dpi().GetScaleInt(5);
    pRender->DrawRoundRect(UiRectF(rcFocusRect.left, rcFocusRect.top, rcFocusRect.right, rcFocusRect.bottom),
                           fRadius, fRadius, borderColor, fWidth);
}

////////////////////////////////////////////////////////////////////////////////
MenuBar::MenuBar(Window* pWindow):
    HBox(pWindow)
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
        AddTopMenuToUI(menuData, GetItemCount());
    }

    //主窗口移动时，关闭已经展开的菜单（弹层不跟随窗口移动）
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        std::weak_ptr<WeakFlag> menuBarFlag = GetWeakFlag();
        MenuBar* pThis = this;
        pWindow->AttachWindowPosChangedMsg([pThis, menuBarFlag](const EventArgs& /*args*/) {
            if (!menuBarFlag.expired()) {
                pThis->HidePopupMenu(nullptr);
            }
            return true;
            });

        //F10或者Alt键按下时，切换菜单栏的键盘焦点状态（与Windows行为一致：再按一次取消焦点）
        pWindow->AttachWindowKeyDownMsg([pThis, menuBarFlag](const EventArgs& msg) {
            if (menuBarFlag.expired()) {
                return true;
            }
            if (msg.vkCode == kVK_F10) {
                //使用按键状态过滤键盘自动重复产生的消息
                if (!pThis->m_bF10KeyDown) {
                    pThis->m_bF10KeyDown = true;
                    pThis->ToggleFocusTopMenuButton();
                }
            }
            else if (msg.vkCode == kVK_MENU) {
                //使用按键状态过滤键盘自动重复产生的消息
                if (!pThis->m_bAltKeyDown) {
                    pThis->m_bAltKeyDown = true;
                    pThis->ToggleFocusTopMenuButton();
                }
            }
            return true;
            });

        //Alt键抬起时，复位按键状态
        pWindow->AttachWindowKeyUpMsg([pThis, menuBarFlag](const EventArgs& msg) {
            if (!menuBarFlag.expired() && (msg.vkCode == kVK_MENU)) {
                pThis->m_bAltKeyDown = false;
            }
            if (!menuBarFlag.expired() && (msg.vkCode == kVK_F10)) {
                pThis->m_bF10KeyDown = false;
            }
            return true;
            });
    }
}

bool MenuBar::AddTopMenuToUI(TopMenuData& menuData, size_t nInsertItem)
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
    if (!bAdded) {
        delete pNewItem;
        return false;
    }
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
    menuData.m_pButton = pNewItem;
    return true;
}

size_t MenuBar::GetInsertChildIndex(size_t nMenuIndex) const
{
    //若目标位置已有菜单按钮，则插入到该按钮之前
    if (nMenuIndex < m_topMenuList.size()) {
        MenuBarButton* pNextItem = m_topMenuList[nMenuIndex].m_pButton.get();
        if (pNextItem != nullptr) {
            size_t nChildIndex = GetItemIndex(pNextItem);
            if (nChildIndex != Box::InvalidIndex) {
                return nChildIndex;
            }
        }
    }
    //否则追加到最后一个菜单按钮之后
    for (size_t nIndex = m_topMenuList.size(); nIndex > 0; --nIndex) {
        MenuBarButton* pPrevItem = m_topMenuList[nIndex - 1].m_pButton.get();
        if (pPrevItem != nullptr) {
            size_t nChildIndex = GetItemIndex(pPrevItem);
            if (nChildIndex != Box::InvalidIndex) {
                return nChildIndex + 1;
            }
        }
    }
    return GetItemCount();
}

void MenuBar::RemoveTopMenuFromUI(TopMenuData& menuData)
{
    MenuBarButton* pButton = menuData.m_pButton.get();
    menuData.m_pButton = nullptr;
    if (pButton == nullptr) {
        return;
    }
    if (IsPopupMenuOf(pButton)) {
        //先关闭该菜单的弹层，避免弹层关联的控件被销毁
        HidePopupMenu(pButton);
    }
    RemoveItem(pButton);
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

    //在修改列表前先计算子控件的插入位置
    size_t nChildIndex = 0;
    if (IsInited()) {
        nChildIndex = GetInsertChildIndex((size_t)nMenuIndex);
    }

    TopMenuData menuData;
    menuData.m_menuItemId = menuItemId;
    menuData.m_menuText = menuText;
    menuData.m_menuTextId = menuTextId;
    menuData.m_menuXmlPath = menuXmlPath;
    menuData.m_menuTextButtonClass = menuTextButtonClass;
    menuData.m_menuTextButtonAttributes = menuTextButtonAttributes;
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
        if (!AddTopMenuToUI(m_topMenuList[nRetIndex], nChildIndex)) {
            m_topMenuList.erase(m_topMenuList.begin() + nRetIndex);
            return -1;
        }
    }
    return nRetIndex;
}

bool MenuBar::GetTopMenu(int32_t nMenuIndex, MenuBarItem& menuBarItem) const
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
        //先从界面中删除（内部会按需关闭弹层）
        RemoveTopMenuFromUI(m_topMenuList[nMenuIndex]);
        m_topMenuList.erase(m_topMenuList.begin() + nMenuIndex);
        return true;
    }
    return false;
}

bool MenuBar::SetActiveTopMenuIndex(int32_t nMenuIndex)
{
    ASSERT((nMenuIndex >= 0) && (nMenuIndex < (int32_t)m_topMenuList.size()));
    if ((nMenuIndex >= 0) && (nMenuIndex < (int32_t)m_topMenuList.size())) {
        MenuBarButton* pButton = m_topMenuList[nMenuIndex].m_pButton.get();
        if ((pButton != nullptr) && pButton->IsVisible() && pButton->IsEnabled()) {
            ShowPopupMenu(pButton);
            return true;
        }
    }
    return false;
}

int32_t MenuBar::GetActiveTopMenuIndex() const
{
    if (HasActivePopupMenu()) {
        Control* pRelatedControl = m_pActiveMenu->GetRelatedControl();
        if (pRelatedControl != nullptr) {
            for (int32_t nMenuIndex = 0; nMenuIndex < (int32_t)m_topMenuList.size(); ++nMenuIndex) {
                if (m_topMenuList[nMenuIndex].m_pButton.get() == pRelatedControl) {
                    return nMenuIndex;
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

bool MenuBar::SetTopMenuText(int32_t nMenuIndex, const DString& menuText, const DString& menuTextId)
{
    if ((nMenuIndex < 0) || (nMenuIndex >= (int32_t)m_topMenuList.size())) {
        return false;
    }
    TopMenuData& menuData = m_topMenuList[nMenuIndex];
    menuData.m_menuText = menuText;
    menuData.m_menuTextId = menuTextId;
    MenuBarButton* pButton = menuData.m_pButton.get();
    if (pButton != nullptr) {
        if (!menuTextId.empty()) {
            pButton->SetTextId(menuTextId.c_str());
        }
        else {
            pButton->SetText(menuText.c_str());
        }
    }
    return true;
}

bool MenuBar::SetTopMenuEnabled(int32_t nMenuIndex, bool bEnable)
{
    if ((nMenuIndex < 0) || (nMenuIndex >= (int32_t)m_topMenuList.size())) {
        return false;
    }
    MenuBarButton* pButton = m_topMenuList[nMenuIndex].m_pButton.get();
    if (pButton == nullptr) {
        return false;
    }
    if (!bEnable && IsPopupMenuOf(pButton)) {
        //禁用前先关闭其弹层
        HidePopupMenu(pButton);
    }
    pButton->SetEnabled(bEnable);
    return true;
}

bool MenuBar::SetTopMenuVisible(int32_t nMenuIndex, bool bVisible)
{
    if ((nMenuIndex < 0) || (nMenuIndex >= (int32_t)m_topMenuList.size())) {
        return false;
    }
    MenuBarButton* pButton = m_topMenuList[nMenuIndex].m_pButton.get();
    if (pButton == nullptr) {
        return false;
    }
    if (!bVisible && IsPopupMenuOf(pButton)) {
        //隐藏前先关闭其弹层
        HidePopupMenu(pButton);
    }
    pButton->SetVisible(bVisible);
    return true;
}

void MenuBar::ClosePopupMenu()
{
    HidePopupMenu(nullptr);
}

void MenuBar::AttachMenuBarActiveChanged(MenuBarActiveChangedEvent callback)
{
    if (callback != nullptr) {
        m_activeChangedList.push_back(callback);
    }
}

void MenuBar::ClearMenuBarActiveChanged()
{
    m_activeChangedList.clear();
}

int32_t MenuBar::GetTopMenuIndex(MenuBarButton* pButton) const
{
    if (pButton == nullptr) {
        return -1;
    }
    for (int32_t nMenuIndex = 0; nMenuIndex < (int32_t)m_topMenuList.size(); ++nMenuIndex) {
        if (m_topMenuList[nMenuIndex].m_pButton.get() == pButton) {
            return nMenuIndex;
        }
    }
    return -1;
}

void MenuBar::NotifyActiveChanged(int32_t nOldIndex, int32_t nNewIndex)
{
    if (nOldIndex == nNewIndex) {
        return;
    }
    std::vector<MenuBarActiveChangedEvent> callbackList(m_activeChangedList);
    for (MenuBarActiveChangedEvent callback : callbackList) {
        if (callback) {
            callback(nOldIndex, nNewIndex);
        }
    }
    Invalidate();
}

void MenuBar::FocusTopMenuButton()
{
    int32_t nActiveIndex = GetActiveTopMenuIndex();
    if ((nActiveIndex >= 0) && (nActiveIndex < (int32_t)m_topMenuList.size())) {
        MenuBarButton* pButton = m_topMenuList[nActiveIndex].m_pButton.get();
        if ((pButton != nullptr) && pButton->IsVisible() && pButton->IsEnabled()) {
            pButton->SetKeyboardFocused(true);
            pButton->SetFocus();
            return;
        }
    }
    for (TopMenuData& menuData : m_topMenuList) {
        MenuBarButton* pButton = menuData.m_pButton.get();
        if ((pButton != nullptr) && pButton->IsVisible() && pButton->IsEnabled()) {
            pButton->SetKeyboardFocused(true);
            pButton->SetFocus();
            return;
        }
    }
}

void MenuBar::ToggleFocusTopMenuButton()
{
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    MenuBarButton* pFocusButton = dynamic_cast<MenuBarButton*>(pWindow->GetFocusControl());
    if ((pFocusButton != nullptr) && pFocusButton->IsKeyboardFocused() &&
        (GetTopMenuIndex(pFocusButton) >= 0)) {
        //当前焦点已经通过键盘设置在菜单栏按钮上，再次按下F10/Alt时取消焦点状态
        pFocusButton->SetKeyboardFocused(false);
        pWindow->SetFocusControl(nullptr);
        return;
    }
    FocusTopMenuButton();
}

bool MenuBar::FocusAdjacentTopMenuButton(MenuBarButton* pButton, bool bNext)
{
    if (pButton == nullptr) {
        return false;
    }
    //收集当前可见且可用的顶级菜单按钮
    std::vector<MenuBarButton*> buttonList;
    for (TopMenuData& menuData : m_topMenuList) {
        MenuBarButton* pItem = menuData.m_pButton.get();
        if ((pItem != nullptr) && pItem->IsVisible() && pItem->IsEnabled()) {
            buttonList.push_back(pItem);
        }
    }
    int32_t nIndex = -1;
    for (int32_t i = 0; i < (int32_t)buttonList.size(); ++i) {
        if (buttonList[i] == pButton) {
            nIndex = i;
            break;
        }
    }
    if (nIndex < 0) {
        return false;
    }
    int32_t nNewIndex = bNext ? (nIndex + 1) : (nIndex - 1);
    if ((nNewIndex < 0) || (nNewIndex >= (int32_t)buttonList.size())) {
        //到达边界，焦点保持在当前按钮
        return false;
    }
    MenuBarButton* pNewButton = buttonList[nNewIndex];
    pNewButton->SetKeyboardFocused(true);
    pNewButton->SetFocus();
    return true;
}

void MenuBar::CancelHoverTimer()
{
    if (m_nHoverTimerId != 0) {
        GlobalManager::Instance().Timer().RemoveTimer(m_nHoverTimerId);
        m_nHoverTimerId = 0;
    }
    m_pHoverButton = nullptr;
}

bool MenuBar::IsPopupMenuOf(MenuBarButton* pButton) const
{
    if (pButton == nullptr) {
        return false;
    }
    //这里只判断关联关系：菜单窗口可能已经因为失去焦点进入关闭流程，
    //鼠标按下时需要据此判断为“收起”，不能再次弹出
    Menu* pMenu = m_pActiveMenu.get();
    return (pMenu != nullptr) && (pMenu->GetRelatedControl() == pButton);
}

void MenuBar::TogglePopupMenu(MenuBarButton* pButton)
{
    if (pButton == nullptr) {
        return;
    }
    if (IsPopupMenuOf(pButton)) {
        //已经展开，收起
        HidePopupMenu(pButton);
    }
    else {
        //未展开，或者展开了其他菜单，切换
        ShowPopupMenu(pButton);
    }
}

void MenuBar::CheckShowPopupMenu(MenuBarButton* pButton)
{
    if (!HasActivePopupMenu()) {
        //没有展开的菜单时，悬停不弹出菜单
        return;
    }
    if (IsPopupMenuOf(pButton)) {
        //已经弹出，不需要重复弹出
        if ((pButton != nullptr) && (pButton->GetState() != ui::kControlStatePressed)) {
            pButton->SetState(ui::kControlStatePressed);
        }
        return;
    }
    //已有展开的菜单时，悬停切换
    ShowPopupMenu(pButton);
}

void MenuBar::OnMenuMouseEnter(MenuBarButton* pButton, const EventArgs& /*msg*/)
{
    if (pButton == nullptr) {
        return;
    }
    if (!HasActivePopupMenu() || IsPopupMenuOf(pButton)) {
        //没有展开的菜单，或者悬停的就是当前菜单
        CheckShowPopupMenu(pButton);
        return;
    }

    //已有其他菜单展开：延迟切换，避免快速划过时频繁重建弹层
    CancelHoverTimer();
    m_pHoverButton = pButton;
    std::weak_ptr<WeakFlag> menuBarFlag = GetWeakFlag();
    MenuBarButton* pMenuButton = pButton;
    m_nHoverTimerId = GlobalManager::Instance().Timer().AddTimer(menuBarFlag,
        [this, menuBarFlag, pMenuButton]() {
            if (menuBarFlag.expired()) {
                //控件已销毁时，不能再访问成员变量
                return;
            }
            m_nHoverTimerId = 0;
            if (m_pHoverButton.get() != pMenuButton) {
                return;
            }
            m_pHoverButton = nullptr;
            if (HasActivePopupMenu() && !IsPopupMenuOf(pMenuButton) &&
                (pMenuButton->GetState() == kControlStateHovered)) {
                ShowPopupMenu(pMenuButton);
            }
        }, kHoverSwitchDelayMs, 1);
}

void MenuBar::OnMenuMouseLeave(MenuBarButton* pButton, const EventArgs& /*msg*/)
{
    if (m_pHoverButton.get() == pButton) {
        //鼠标离开等待切换的按钮，取消切换
        CancelHoverTimer();
    }
    if (HasActivePopupMenu() && IsPopupMenuOf(pButton)) {
        //已经弹出菜单，保持状态不变
        if ((pButton != nullptr) && (pButton->GetState() != ui::kControlStatePressed)) {
            pButton->SetState(ui::kControlStatePressed);
        }
    }
}

void MenuBar::ShowPopupMenu(MenuBarButton* pButton)
{
    if ((pButton == nullptr) || !pButton->IsVisible() || !pButton->IsEnabled()) {
        return;
    }
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return;
    }
    if (HasActivePopupMenu() && IsPopupMenuOf(pButton)) {
        //已经展开，不需要重复弹出
        return;
    }
    TopMenuData* pTopMenuData = nullptr;
    for (TopMenuData& menuData : m_topMenuList) {
        if (menuData.m_pButton.get() == pButton) {
            pTopMenuData = &menuData;
            break;
        }
    }
    ASSERT(pTopMenuData != nullptr);
    if (pTopMenuData == nullptr) {
        return;
    }

    UiPoint point(pButton->GetRect().left, pButton->GetRect().top);
    UiPoint scrollOfset = pButton->GetScrollOffsetInScrollBox();
    point.Offset(-scrollOfset.x, -scrollOfset.y);
    point.Offset(0, pButton->GetRect().Height());
    ClientToScreen(point);

    const int32_t nOldIndex = GetActiveTopMenuIndex();
    const int32_t nNewIndex = GetTopMenuIndex(pButton);

    //如果已经展开则关闭
    HidePopupMenu(pButton, false);

    Menu* pMenu = new ui::Menu(pWindow, pButton, this);//需要设置父窗口，否则菜单弹出时程序状态栏会变成非激活状态
    m_pActiveMenu = pMenu;

    pMenu->SetSkinFolder(pWindow->GetResourcePath().ToString());
    pMenu->ShowMenu(pTopMenuData->m_menuXmlPath.c_str(), point);

    if (!pMenu->IsWindow()) {
        //菜单窗口创建失败：解除菜单观察者关联，并释放窗口对象
        m_pActiveMenu = nullptr;
        pMenu->RemoveObserver();
        delete pMenu;
        NotifyActiveChanged(nOldIndex, -1);
        return;
    }
    if (pMenu->GetMenuItemCount() == 0) {
        //菜单XML加载失败或者没有菜单项：关闭菜单窗口
        m_pActiveMenu = nullptr;
        pMenu->CloseMenu();
        NotifyActiveChanged(nOldIndex, -1);
        return;
    }
    pMenu->SetWindowFocus();

    std::weak_ptr<WeakFlag> menuBarFlag = GetWeakFlag();
    //菜单命令事件响应
    DString menuItemId = pTopMenuData->m_menuItemId.c_str();
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
    pMenu->AttachWindowCloseMsg([this, pMenu, menuBarFlag](const EventArgs& /*args*/) {
            if (!menuBarFlag.expired() && (m_pActiveMenu == pMenu)) {
                int32_t nOldIndex = GetTopMenuIndex(dynamic_cast<MenuBarButton*>(pMenu->GetRelatedControl()));
                m_pActiveMenu = nullptr;
                if (nOldIndex >= 0) {
                    NotifyActiveChanged(nOldIndex, -1);
                }
            }
            return true;
        });
    NotifyActiveChanged(nOldIndex, nNewIndex);
}

void MenuBar::HidePopupMenu(MenuBarButton* /*pButton*/, bool bNotify)
{
    const int32_t nOldIndex = bNotify ? GetActiveTopMenuIndex() : -1;
    Menu* pMenu = m_pActiveMenu.get();
    m_pActiveMenu = nullptr;
    if ((pMenu != nullptr) && pMenu->IsWindow() && !pMenu->IsClosingWnd()) {
        pMenu->CloseMenu();
    }
    if (nOldIndex >= 0) {
        NotifyActiveChanged(nOldIndex, -1);
    }
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
    if (pMenu == nullptr) {
        return;
    }
    if (vkCode == kVK_ESCAPE) {
        //关闭一级菜单后，把键盘焦点还给对应的顶级菜单按钮
        MenuBarButton* pButton = dynamic_cast<MenuBarButton*>(pMenu->GetRelatedControl());
        if (pButton != nullptr) {
            pButton->SetKeyboardFocused(true);
            pButton->SetFocus();
        }
        return;
    }
    if (!HasActivePopupMenu()) {
        return;
    }
    if ((vkCode != kVK_LEFT) && (vkCode != kVK_RIGHT)) {
        return;
    }

    //可见且可用的顶级菜单按钮列表
    std::vector<MenuBarButton*> itemList;
    size_t nCurrentPos = Box::InvalidIndex;
    Control* pRelatedControl = pMenu->GetRelatedControl();
    for (size_t nIndex = 0; nIndex < GetItemCount(); ++nIndex) {
        MenuBarButton* pItem = dynamic_cast<MenuBarButton*>(GetItemAt(nIndex));
        if ((pItem != nullptr) && pItem->IsVisible() && pItem->IsEnabled()) {
            if (pRelatedControl == pItem) {
                nCurrentPos = itemList.size();
            }
            itemList.push_back(pItem);
        }
    }
    if (itemList.empty() || (nCurrentPos == Box::InvalidIndex) || (itemList.size() <= 1)) {
        return;
    }

    size_t nNextPos = 0;
    if (vkCode == kVK_RIGHT) {
        nNextPos = (nCurrentPos + 1) % itemList.size();
    }
    else {
        nNextPos = (nCurrentPos + itemList.size() - 1) % itemList.size();
    }
    if (itemList[nNextPos] != nullptr) {
        ShowPopupMenu(itemList[nNextPos]);
    }
}

void MenuBar::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }
    MenuBarButton* pActiveButton = nullptr;
    if (HasActivePopupMenu()) {
        pActiveButton = dynamic_cast<MenuBarButton*>(m_pActiveMenu->GetRelatedControl());
    }
    if ((pActiveButton == nullptr) || !pActiveButton->IsVisible()) {
        return;
    }

    //在激活的顶级菜单下方绘制强调色指示条
    const UiRect rcButton = pActiveButton->GetRect();
    int32_t nLineHeight = Dpi().GetScaleInt(2);
    if (nLineHeight < 1) {
        nLineHeight = 1;
    }
    const int32_t nHMargin = Dpi().GetScaleInt(2);
    UiRect rcLine(rcButton.left + nHMargin, rcButton.bottom - nLineHeight,
                  rcButton.right - nHMargin, rcButton.bottom);
    if (rcLine.Width() <= 0) {
        return;
    }
    UiColor lineColor = GetUiColor(_T("color_accent"));
    if (lineColor.IsEmpty()) {
        lineColor = GetUiColor(_T("text_menu_bar"));
    }
    if (!lineColor.IsEmpty()) {
        pRender->FillRect(UiRectF(rcLine.left, rcLine.top, rcLine.right, rcLine.bottom), lineColor);
    }
}

void MenuBar::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);
    if (bChanged && !IsVisible()) {
        //菜单栏被隐藏时，关闭弹层并取消待执行的悬停切换
        CancelHoverTimer();
        HidePopupMenu(nullptr);
    }
}

void MenuBar::OnSetEnabled(bool bChanged)
{
    BaseClass::OnSetEnabled(bChanged);
    if (bChanged && !IsEnabled()) {
        //菜单栏被禁用时，关闭弹层并取消待执行的悬停切换
        CancelHoverTimer();
        HidePopupMenu(nullptr);
    }
}

} // namespace ui
