#include "CheckCombo.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/WindowAnchorFollower.h"

namespace ui
{
class CCheckComboWnd : public Window
{
    typedef Window BaseClass;
public:
    /** 创建并显示下拉窗口
    */
    void InitComboWnd(CheckCombo* pOwner);

    /** 更新下拉窗口的位置和大小
    */
    void UpdateComboWnd();

    /** 关闭下拉窗口
    */
    void CloseComboWnd();

    /** 宿主控件析构时关闭下拉窗口（不回调宿主，避免访问正在析构的对象）
    */
    void CloseComboWndOnOwnerDestroy();

    //基类虚函数的重写
    virtual void PreInitWindow() override;
    virtual void OnInitWindow() override;
    virtual void OnCloseWindow() override;
    virtual void OnFinalMessage() override;
    virtual void OnWindowShadowTypeChanged() override;

    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override;

private:
    /** 计算下拉框的显示矩形
    */
    UiRect GetComboWndRect() const;

    /** 设置键盘导航时的当前项（更新高亮状态，并保证该项可见）
    */
    void SetKeyIndex(size_t nIndex);

    /** 切换键盘导航当前项的选中状态
    */
    void ToggleKeyIndexItem();

    /** 清除键盘导航的当前项高亮
    */
    void ResetKeyIndex();

private:
    //关联的宿主控件（弱引用，宿主析构后自动失效）
    ControlPtrT<CheckCombo> m_pOwner;
    //是否已经关闭
    bool m_bIsClosed = false;
    //键盘导航的当前项索引（Box::InvalidIndex 表示未定位）
    size_t m_nKeyIndex = Box::InvalidIndex;

    //跟随宿主控件的位置变化（宿主窗口移动、所在滚动容器滚动）
    WindowAnchorFollower m_anchorFollower;
};

void CCheckComboWnd::InitComboWnd(CheckCombo* pOwner)
{
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return;
    }
    m_pOwner = pOwner;
    m_bIsClosed = false;

    //设置下拉框的显示位置和大小，避免弹出界面的时候出现黑屏现象
    UiRect rcWnd = GetComboWndRect();
    WindowCreateParam createWndParam;
    createWndParam.m_dwStyle = kWS_POPUP;
    createWndParam.m_dwExStyle = kWS_EX_LAYERED;
    createWndParam.m_nX = rcWnd.left;
    createWndParam.m_nY = rcWnd.top;
    createWndParam.m_nWidth = rcWnd.Width();
    createWndParam.m_nHeight = rcWnd.Height();
    CreateWnd(pOwner->GetWindow(), createWndParam);
    UpdateComboWnd();

    ShowWindow(ui::kSW_SHOW_NORMAL);
    KeepParentActive();

    //初始化键盘导航的当前项
    if (pOwner->GetListBox()->GetItemCount() > 0) {
        SetKeyIndex(0);
    }

    //跟随宿主控件：窗口移动或滚动容器滚动时，下拉窗口同步更新位置
    m_anchorFollower.Start(
        [this]() -> UiRect {
            CheckCombo* pOwner = m_pOwner.get();
            if ((pOwner == nullptr) || (pOwner->GetWindow() == nullptr)) {
                return UiRect();
            }
            UiRect rcOwner = pOwner->GetPos();
            UiPoint scrollOffset = pOwner->GetScrollOffsetInScrollBox();
            rcOwner.Offset(-scrollOffset.x, -scrollOffset.y);
            pOwner->GetWindow()->ClientToScreen(rcOwner);
            return rcOwner;
        },
        [this]() {
            if (!m_bIsClosed) {
                UpdateComboWnd();
            }
        });

    //发送一个事件
    pOwner->SendEvent(kEventWindowCreate);
}

UiRect CCheckComboWnd::GetComboWndRect() const
{
    CheckCombo* pOwner = m_pOwner.get();
    if (pOwner == nullptr) {
        return UiRect();
    }
    //阴影的大小
    ui::UiPadding rcPadding;
    if (IsWindow()) {
        rcPadding = GetCurrentShadowCorner();
    }

    // Position the popup window in absolute space
    UiSize szDrop = pOwner->GetDropBoxSize();
    UiRect rcOwner = pOwner->GetPos();
    UiPoint scrollBoxOffset = pOwner->GetScrollOffsetInScrollBox();
    rcOwner.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);

    UiRect rc = rcOwner;
    rc.top = rc.bottom + Dpi().GetScaleInt(1);  // 父窗口left、bottom位置作为弹出窗口起点
    rc.bottom = rc.top + szDrop.cy;             // 计算弹出窗口高度
    if (szDrop.cx > 0) {
        rc.right = rc.left + szDrop.cx;         // 计算弹出窗口宽度
    }

    //如果子容器里面的都是拉伸类型，就不需要估算大小（会报错，无法估算），而是按照下拉框的设置大小来显示
    bool bCanEstimateSize = true;
    if (pOwner->GetListBox()->GetFixedHeight().IsStretch() && pOwner->GetListBox()->GetFixedWidth().IsStretch()) {
        size_t nItemCount = pOwner->GetListBox()->GetItemCount();
        if (nItemCount > 0) {
            bCanEstimateSize = false;
            for (size_t nItemIndex = 0; nItemIndex < nItemCount; nItemIndex++) {
                Control* pControl = pOwner->GetListBox()->GetItemAt(nItemIndex);
                if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
                    continue;
                }
                if (!pControl->GetFixedHeight().IsStretch() || !pControl->GetFixedWidth().IsStretch()) {
                    bCanEstimateSize = true;
                    break;
                }
            }
        }
    }

    int32_t cyFixed = 0;
    if (bCanEstimateSize && (pOwner->GetListBox()->GetItemCount() > 0)) {
        UiSize szAvailable(rc.Width(), rc.Height());
        UiFixedInt oldFixedHeight = pOwner->GetListBox()->GetFixedHeight();
        pOwner->GetListBox()->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
        UiEstSize estSize = pOwner->GetListBox()->EstimateSize(szAvailable);
        pOwner->GetListBox()->SetFixedHeight(oldFixedHeight, false, false);
        cyFixed = estSize.cy.GetInt32();
    }
    if (cyFixed == 0) {
        cyFixed = szDrop.cy;
    }
    rc.bottom = rc.top + std::min(cyFixed, szDrop.cy);
    rc.Inflate(rcPadding);
    pOwner->GetWindow()->ClientToScreen(rc);

    UiRect rcWork;
    pOwner->GetWindow()->GetMonitorWorkRect(rcWork);
    if (rc.bottom > rcWork.bottom || pOwner->IsPopupTop()) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if (szDrop.cx > 0) {
            rc.right = rc.left + szDrop.cx;
        }
        rc.top = rcOwner.top - std::min(cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
        rc.Inflate(rcPadding);
        pOwner->GetWindow()->ClientToScreen(rc);
    }
    //限制在显示器工作区内，避免下拉窗口在屏幕边缘溢出
    if (rc.right > rcWork.right) {
        rc.Offset(-(rc.right - rcWork.right), 0);
    }
    if (rc.left < rcWork.left) {
        rc.Offset(rcWork.left - rc.left, 0);
    }
    return rc;
}

void CCheckComboWnd::SetKeyIndex(size_t nIndex)
{
    CheckCombo* pOwner = m_pOwner.get();
    if (pOwner == nullptr) {
        return;
    }
    ScrollBox* pListBox = pOwner->GetListBox();
    if (pListBox == nullptr) {
        return;
    }
    ListBox* pDropListBox = dynamic_cast<ListBox*>(pListBox);
    if (m_nKeyIndex != nIndex) {
        if (Box::IsValidItemIndex(m_nKeyIndex)) {
            Control* pOldItem = pListBox->GetItemAt(m_nKeyIndex);
            if (pOldItem != nullptr) {
                pOldItem->SetState(kControlStateNormal);
            }
        }
        m_nKeyIndex = nIndex;
        Control* pNewItem = pListBox->GetItemAt(m_nKeyIndex);
        if (pNewItem != nullptr) {
            pNewItem->SetState(kControlStateHovered);
        }
    }
    if (Box::IsValidItemIndex(m_nKeyIndex) && (pDropListBox != nullptr)) {
        pDropListBox->EnsureVisible(m_nKeyIndex);
    }
}

void CCheckComboWnd::ToggleKeyIndexItem()
{
    CheckCombo* pOwner = m_pOwner.get();
    if ((pOwner == nullptr) || !Box::IsValidItemIndex(m_nKeyIndex)) {
        return;
    }
    ScrollBox* pListBox = pOwner->GetListBox();
    if (pListBox == nullptr) {
        return;
    }
    Control* pItem = pListBox->GetItemAt(m_nKeyIndex);
    if (pItem == nullptr) {
        return;
    }
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pItem);
    if (pCheckBox != nullptr) {
        pCheckBox->Selected(!pCheckBox->IsSelected(), true);
        return;
    }
    CheckBoxBox* pCheckBoxBox = dynamic_cast<CheckBoxBox*>(pItem);
    if (pCheckBoxBox != nullptr) {
        pCheckBoxBox->Selected(!pCheckBoxBox->IsSelected(), true);
    }
}

void CCheckComboWnd::ResetKeyIndex()
{
    CheckCombo* pOwner = m_pOwner.get();
    if ((pOwner != nullptr) && Box::IsValidItemIndex(m_nKeyIndex)) {
        ScrollBox* pListBox = pOwner->GetListBox();
        if (pListBox != nullptr) {
            Control* pItem = pListBox->GetItemAt(m_nKeyIndex);
            if (pItem != nullptr) {
                pItem->SetState(kControlStateNormal);
            }
        }
    }
    m_nKeyIndex = Box::InvalidIndex;
}

void CCheckComboWnd::UpdateComboWnd()
{
    UiRect rc = GetComboWndRect();
    if (rc.IsEmpty()) {
        return;
    }
    SetWindowPos(InsertAfterWnd(), rc.left, rc.top, rc.Width(), rc.Height(), kSWP_NOZORDER | kSWP_NOACTIVATE);
}

void CCheckComboWnd::CloseComboWnd()
{
    if (m_bIsClosed) {
        return;
    }
    m_bIsClosed = true;
    m_anchorFollower.Stop();
    ResetKeyIndex();
    Box* pRootBox = GetRoot();
    CheckCombo* pOwner = m_pOwner.get();
    if ((pOwner != nullptr) && (pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
        pOwner->GetListBox()->SetWindow(nullptr);
        pOwner->GetListBox()->SetParent(nullptr);
        pRootBox->RemoveAllItems();
    }
    //先将前端窗口切换为父窗口，避免前端窗口关闭后，切换到其他窗口
    if ((pOwner != nullptr) && (pOwner->GetWindow() != nullptr)) {
        if (IsWindowForeground()) {
            pOwner->GetWindow()->SetWindowForeground();
        }
    }
    CloseWnd();
}

void CCheckComboWnd::CloseComboWndOnOwnerDestroy()
{
    if (m_bIsClosed) {
        return;
    }
    m_bIsClosed = true;
    m_anchorFollower.Stop();
    ResetKeyIndex();
    Box* pRootBox = GetRoot();
    CheckCombo* pOwner = m_pOwner.get();
    if (pRootBox != nullptr) {
        if (pOwner != nullptr) {
            pOwner->GetListBox()->SetWindow(nullptr);
            pOwner->GetListBox()->SetParent(nullptr);
        }
        pRootBox->RemoveAllItems();
    }
    //宿主正在析构，后续不再访问宿主
    m_pOwner.reset();
    CloseWnd();
}

void CCheckComboWnd::OnFinalMessage()
{
    m_anchorFollower.Stop();
    CheckCombo* pOwner = m_pOwner.get();
    if (pOwner != nullptr) {
        if (pOwner->GetListBox()->GetWindow() == this) {
            pOwner->GetListBox()->SetWindow(nullptr);
            pOwner->GetListBox()->SetParent(nullptr);
        }
        if (pOwner->m_pCheckComboWnd == this) {
            pOwner->m_pCheckComboWnd = nullptr;
            pOwner->SetState(kControlStateNormal);
            pOwner->Invalidate();
        }
    }    
    BaseClass::OnFinalMessage();
}

void CCheckComboWnd::OnWindowShadowTypeChanged()
{
    if (IsWindow() && (GetRoot() != nullptr)) {
        UpdateComboWnd();
    }
}

void CCheckComboWnd::PreInitWindow()
{
    BaseClass::PreInitWindow();
    //下拉弹窗统一强制使用CPU渲染后端，避免受全局渲染后端配置影响
    SetRenderBackendType(RenderBackendType::kRaster_BackendType, 0, false);
}

void CCheckComboWnd::OnInitWindow()
{
    BaseClass::OnInitWindow();
    CheckCombo* pOwner = m_pOwner.get();
    if (pOwner == nullptr) {
        return;
    }
    SetResourcePath(pOwner->GetWindow()->GetResourcePath());
    SetShadowType(pOwner->GetComboWndShadowType());
    //边框与菜单保持一致：1px 圆角边框由窗口阴影统一绘制，下拉列表不再绘制自身边框
    SetShadowBorderSize(1);
    SetShadowBorderColor(_T("border_window"));

    Box* pRoot = new Box(this);
    pRoot->SetAutoDestroyChild(false);
    pRoot->AddItem(pOwner->GetListBox());
    AttachBox(AttachShadow(pRoot));

    //更新窗口位置
    UpdateComboWnd();
}

void CCheckComboWnd::OnCloseWindow()
{
    m_anchorFollower.Stop();
    Box* pRootBox = GetRoot();
    CheckCombo* pOwner = m_pOwner.get();
    if ((pOwner != nullptr) && (pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
        pOwner->GetListBox()->SetWindow(nullptr);
        pOwner->GetListBox()->SetParent(nullptr);
        pRootBox->RemoveAllItems();
    }
    //宿主控件可能已在下拉窗口关闭完成前被销毁，需判空
    if ((pOwner != nullptr) && (pOwner->GetWindow() != nullptr) && pOwner->GetWindow()->IsWindow()) {
        pOwner->SetPos(pOwner->GetPos());
        if (IsWindowForeground() || IsWindowFocused()) {
            pOwner->SetFocus();
        }
    }
    BaseClass::OnCloseWindow();
}

LRESULT CCheckComboWnd::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    if ((vkCode == kVK_ESCAPE) || (vkCode == kVK_RETURN)) {
        //按ESC键，取消；按回车键，正常关闭
        CloseComboWnd();
        return lResult;
    }
    CheckCombo* pOwner = m_pOwner.get();
    if ((pOwner == nullptr) || (pOwner->GetListBox() == nullptr)) {
        return lResult;
    }
    const size_t itemCount = pOwner->GetListBox()->GetItemCount();
    if (itemCount == 0) {
        return lResult;
    }
    if ((vkCode == kVK_DOWN) || (vkCode == kVK_UP)) {
        //向上/向下箭头：切换键盘导航的当前项
        size_t nIndex = m_nKeyIndex;
        if (!Box::IsValidItemIndex(nIndex) || (nIndex >= itemCount)) {
            nIndex = (vkCode == kVK_DOWN) ? 0 : (itemCount - 1);
        }
        else if (vkCode == kVK_DOWN) {
            if (nIndex + 1 < itemCount) {
                ++nIndex;
            }
        }
        else {
            if (nIndex > 0) {
                --nIndex;
            }
        }
        SetKeyIndex(nIndex);
    }
    else if (vkCode == kVK_SPACE) {
        //按空格键：切换当前项的选中状态
        ToggleKeyIndexItem();
    }
    return lResult;
}

LRESULT CCheckComboWnd::OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKillFocusMsg(pSetFocusWindow, nativeMsg, bHandled);
    //失去焦点，关闭窗口，正常关闭
    if (pSetFocusWindow != this) {
        CloseComboWnd();
    }
    return lResult;
}

////////////////////////////////////////////////////////

#define CHECK_COMBO_DEFAULT_HEIGHT 20 

CheckCombo::CheckCombo(Window* pWindow) :
    Box(pWindow),
    m_pCheckComboWnd(nullptr),
    m_szDropBox(0, 0),
    m_bPopupTop(false),
    m_iOrgHeight(CHECK_COMBO_DEFAULT_HEIGHT),
    m_nShadowType(ShadowType::kShadowSmallRound)
{
    //默认参与TAB键切换（可通过tab_stop属性关闭），并显示焦点框
    SetTabStop(true);
    SetShowFocusedRect(true);
    SetDropBoxSize({0, 150}, true);
    SetMaxHeight(m_iOrgHeight * 3, true);
    SetMinHeight(m_iOrgHeight, true);

    m_pDropList.reset(new ui::ListBox(pWindow, new ui::VLayout));
    m_pDropList->EnableScrollBar();
    
    m_pList.reset(new ui::ListBox(pWindow, new ui::VTileLayout));
    m_pList->SetNoFocus();//内部列表不接收焦点，焦点由CheckCombo自己管理
    m_pList->SetMouseEnabled(false); //内部列表不响应鼠标消息
    m_pList->SetMouseChildEnabled(false);//内部列表子控件也不响应鼠标消息
    m_pList->EnableScrollBar();

    Box::AddItem(m_pList.get());
}

CheckCombo::~CheckCombo()
{
    //宿主析构时先关闭下拉窗口，避免下拉窗口继续持有即将销毁的列表容器
    if (m_pCheckComboWnd != nullptr) {
        m_pCheckComboWnd->CloseComboWndOnOwnerDestroy();
        m_pCheckComboWnd = nullptr;
    }
    SetAutoDestroyChild(false);
    Box::RemoveItem(m_pList.get());
    SetAutoDestroyChild(true);
    m_pList.reset();
    m_pDropList.reset();
}

DString CheckCombo::GetType() const { return DUI_CTR_CHECK_COMBO; }

uint32_t CheckCombo::GetControlFlags() const
{
    //支持TAB键切换焦点（Box基类默认不支持）
    return (IsEnabled() && IsKeyboardEnabled() && IsAllowTabStop()) ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

bool CheckCombo::OnKeyDown(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return false;
    }
    const VirtualKeyCode vkCode = (VirtualKeyCode)msg.vkCode;
    if (m_pCheckComboWnd != nullptr) {
        //下拉列表显示时，按键由下拉窗口处理
        if (vkCode == kVK_ESCAPE) {
            m_pCheckComboWnd->CloseComboWnd();
            return true;
        }
        return false;
    }
    if ((vkCode == kVK_DOWN) || (vkCode == kVK_UP) || (vkCode == kVK_RETURN) ||
        (vkCode == kVK_SPACE) || (vkCode == kVK_F4)) {
        //按Up/Down、Enter、空格、F4 打开下拉列表
        Activate(nullptr);
        return true;
    }
    return false;
}

void CheckCombo::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("dropbox")) {
        SetDropBoxAttributeList(strValue);
    }
    else if (strName == _T("dropbox_item_class")) {
        SetDropboxItemClass(strValue);
    }
    else if (strName == _T("selected_item_class")) {
        SetSelectedItemClass(strValue);
    }
    else if (strName == _T("vscrollbar")) {
    }
    else if ((strName == _T("dropbox_size")) || (strName == _T("dropboxsize"))) {
        UiSize szDropBoxSize;
        AttributeUtil::ParseSizeValue(strValue.c_str(), szDropBoxSize);
        SetDropBoxSize(szDropBoxSize, true);
    }
    else if ((strName == _T("popup_top")) || (strName == _T("popuptop"))) {
        SetPopupTop(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("height")) {
        BaseClass::SetAttribute(strName, strValue);
        if (strValue != _T("stretch") && strValue != _T("auto")) {
            m_iOrgHeight = StringUtil::StringToInt32(strValue);
            ASSERT(m_iOrgHeight >= 0);
            SetMaxHeight(m_iOrgHeight * 3, true);
            SetMinHeight(m_iOrgHeight, true);
        }
    }
    else if (strName == _T("shadow_type")) {
        //设置下拉窗口的阴影类型
        ShadowType nShadowType = ShadowType::kShadowDefault;
        if (Shadow::GetShadowType(strValue, nShadowType)) {
            SetComboWndShadowType(nShadowType);
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

bool CheckCombo::AddItem(Control* pControl)
{
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(pControl);
    if (pCheckBox) {
        pCheckBox->AttachSelect(UiBind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
        pCheckBox->AttachUnSelect(UiBind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
    }
    else    {
        ui::CheckBoxBox* pCheckBoxBox = dynamic_cast<ui::CheckBoxBox*>(pControl);
        if (pCheckBoxBox) {
            pCheckBoxBox->AttachSelect(UiBind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
            pCheckBoxBox->AttachUnSelect(UiBind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
        }
        else {
            ASSERT(0);
            return false;
        }
    }
    return m_pDropList->AddItem(pControl);
}

bool CheckCombo::AddItemAt(Control* pControl, size_t iIndex)
{
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(pControl);
    if (pCheckBox) {
        pCheckBox->AttachSelect(UiBind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
        pCheckBox->AttachUnSelect(UiBind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
    }
    else {
        ui::CheckBoxBox* pCheckBoxBox = dynamic_cast<ui::CheckBoxBox*>(pControl);
        if (pCheckBoxBox) {
            pCheckBoxBox->AttachSelect(UiBind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
            pCheckBoxBox->AttachUnSelect(UiBind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
        }
        else {
            ASSERT(0);
            return false;
        }
    }
    return m_pDropList->AddItemAt(pControl, iIndex);
}

bool CheckCombo::RemoveItem(Control* pControl)
{
    return m_pDropList->RemoveItem(pControl);
}

bool CheckCombo::RemoveItemAt(size_t iIndex)
{
    return m_pDropList->RemoveItemAt(iIndex);
}

void CheckCombo::RemoveAllItems()
{
    m_pDropList->RemoveAllItems();
}

Control* CheckCombo::GetItemAt(size_t iIndex) const
{
    return m_pDropList->GetItemAt(iIndex);
}

size_t CheckCombo::GetItemIndex(Control* pControl) const
{
    return m_pDropList->GetItemIndex(pControl);
}

bool CheckCombo::SetItemIndex(Control* pControl, size_t iIndex)
{
    return m_pDropList->SetItemIndex(pControl, iIndex);
}

size_t CheckCombo::GetItemCount() const
{
    return m_pDropList->GetItemCount();
}

bool CheckCombo::AddTextItem(const DString& itemText)
{
    if (itemText.empty()) {
        return false;
    }
    //避免重复名称
    const size_t itemCount = GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(index));
        if (pCheckBox != nullptr) {
            if (itemText == pCheckBox->GetText()) {
                return false;
            }
        }
    }

    CheckBox* item = new CheckBox(GetWindow());
    SetAttributeList(item, m_dropboxItemClass.c_str());
    item->SetText(itemText);
    return AddItem(item);
}

bool CheckCombo::AddTextIdItem(const DString& itemTextId)
{
    if (itemTextId.empty()) {
        return false;
    }
    //避免重复名称
    const size_t itemCount = GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(index));
        if (pCheckBox != nullptr) {
            if (itemTextId == pCheckBox->GetTextId()) {
                return false;
            }
        }
    }

    CheckBox* item = new CheckBox(GetWindow());
    SetAttributeList(item, m_dropboxItemClass.c_str());
    item->SetTextId(itemTextId);
    return AddItem(item);
}

bool CheckCombo::SelectTextItem(const DString& itemText, bool bSelect)
{
    if (itemText.empty()) {
        return false;
    }
    const size_t itemCount = GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(index));
        if (pCheckBox != nullptr) {
            if (itemText == pCheckBox->GetText()) {
                pCheckBox->Selected(bSelect, true);
                return true;
            }
        }
    }
    return false;
}

bool CheckCombo::SelectTextIdItem(const DString& itemTextId, bool bSelect)
{
    if (itemTextId.empty()) {
        return false;
    }
    const size_t itemCount = GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(index));
        if (pCheckBox != nullptr) {
            if (itemTextId == pCheckBox->GetTextId()) {
                pCheckBox->Selected(bSelect, true);
                return true;
            }
        }
    }
    return false;
}

bool CheckCombo::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    if (!IsActivatable()) {
        return bRet;
    }
    if (m_pCheckComboWnd != nullptr) {
        //如果已经显示，则外部会触发失去焦点关闭（开关属性）
        return bRet;
    }

    m_pCheckComboWnd = new CCheckComboWnd();
    m_pCheckComboWnd->AttachWindowCreateMsg(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
        //触发消息到应用层(下拉窗口创建)
        EventArgs msg;
        msg.SetSender(this);
        msg.eventType = kEventWindowCreate;
        FireNormalEvents(msg);
        return true;
        }));
    m_pCheckComboWnd->InitComboWnd(this);
    m_pCheckComboWnd->AttachWindowCloseMsg(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
        //触发消息到应用层(下拉窗口销毁)
        EventArgs msg;
        msg.SetSender(this);
        msg.eventType = kEventWindowClose;
        FireNormalEvents(msg);
        return true;
    }));
    Invalidate();
    return bRet;
}

void CheckCombo::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }

    UiSize szDropBoxSize = GetDropBoxSize();
    szDropBoxSize = Dpi().GetScaleSize(szDropBoxSize, nOldDpiScale);
    SetDropBoxSize(szDropBoxSize, false);

    if (m_pDropList != nullptr) {
        if (m_pDropList->GetWindow() == nullptr) {
            m_pDropList->SetWindow(GetWindow());
        }
        if ((m_pDropList->GetWindow() == GetWindow()) && (m_pDropList->GetParent() == nullptr)) {
            m_pDropList->ChangeDpiScale(nOldDpiScale, nNewDpiScale);
        }
    }

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void CheckCombo::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    //更新选择列表的高度
    UpdateSelectedListHeight();
}

void CheckCombo::SetDropBoxAttributeList(const DString& pstrList)
{
    SetAttributeList(m_pDropList.get(), pstrList);
}

void CheckCombo::SetDropboxItemClass(const DString& classValue)
{
    m_dropboxItemClass = classValue;
}

void CheckCombo::SetSelectedItemClass(const DString& classValue)
{
    m_selectedItemClass = classValue;
}

void CheckCombo::UpdateComboWndPos()
{
    if (m_pCheckComboWnd != nullptr) {
        m_pCheckComboWnd->UpdateComboWnd();
    }
}

Window* CheckCombo::GetCheckComboWnd() const
{
    return m_pCheckComboWnd.get();
}

void CheckCombo::SetComboWndShadowType(ShadowType nShadowType)
{
    m_nShadowType = nShadowType;
    if (m_pCheckComboWnd != nullptr) {
        m_pCheckComboWnd->SetShadowType(nShadowType);
    }
}

ShadowType CheckCombo::GetComboWndShadowType() const
{
    return m_nShadowType;
}

void CheckCombo::SetAttributeList(Control* pControl, const DString& classValue)
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return;
    }
    std::vector<std::pair<DString, DString>> attributeList;
    AttributeUtil::ParseAttributeList(classValue, attributeList);
    if (!attributeList.empty()) {
        //按属性列表设置
        for (const auto& attribute : attributeList) {
            pControl->SetAttribute(attribute.first, attribute.second);
        }
    }
    else if (!classValue.empty()) {
        //按Class名称设置
        pControl->SetClass(classValue);
    }
}

const UiSize& CheckCombo::GetDropBoxSize() const
{
    return m_szDropBox;
}

void CheckCombo::SetDropBoxSize(UiSize szDropBox, bool bNeedScaleDpi)
{
    ASSERT(szDropBox.cy > 0);
    if (szDropBox.cy <= 0) {
        return;
    }
    szDropBox.Validate();
    if (bNeedScaleDpi) {
        Dpi().ScaleSize(szDropBox);
    }
    m_szDropBox = szDropBox;
}

bool CheckCombo::OnSelectItem(const ui::EventArgs& args)
{
    if (args.GetSender() == nullptr) {
        return true;
    }
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(args.GetSender());
    if (pCheckBox == nullptr) {
        return true;
    }
    DString itemText = pCheckBox->GetText();
    if (itemText.empty()) {
        return true;
    }
    DString itemTextId = pCheckBox->GetTextId();

    Label* item = new Label(m_pList->GetWindow());
    SetAttributeList(item, m_selectedItemClass.c_str());
    if (!itemTextId.empty()) {
        item->SetTextId(itemTextId);
    }
    else {
        item->SetText(itemText);
    }    
    m_pList->AddItem(item);
    UpdateSelectedListHeight();
    return true;
}

bool CheckCombo::OnUnSelectItem(const ui::EventArgs& args)
{
    if (args.GetSender() == nullptr) {
        return true;
    }
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(args.GetSender());
    if (pCheckBox == nullptr) {
        return true;
    }
    DString itemText = pCheckBox->GetText();
    if (itemText.empty()) {
        return true;
    }

    size_t itemCount = m_pList->GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        Label* pLabel = dynamic_cast<Label*>(m_pList->GetItemAt(index));
        if (pLabel != nullptr) {
            if (pLabel->GetText() == itemText) {
                m_pList->RemoveItem(pLabel);
                break;
            }
        }
    }
    UpdateSelectedListHeight();
    return true;
}

void CheckCombo::UpdateSelectedListHeight()
{
    UiFixedInt oldFixedHeight = m_pList->GetFixedHeight();
    m_pList->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
    UiSize szAvailable(GetWidth(), 0);
    const UiPadding rcPadding = GetPadding();
    szAvailable.cx -= (rcPadding.top + rcPadding.bottom);
    UiEstSize estSize = m_pList->EstimateSize(szAvailable);
    m_pList->SetFixedHeight(oldFixedHeight, false, false);
    if (estSize.cy.IsInt32()) {
        int32_t nFixedHeight = estSize.cy.GetInt32();
        const UiPadding rcListPadding = m_pList->GetPadding();
        nFixedHeight -= (rcListPadding.top + rcListPadding.bottom);
        nFixedHeight += (rcPadding.top + rcPadding.bottom);
        if (nFixedHeight > 1) {
            SetFixedHeight(ui::UiFixedInt(nFixedHeight), true, false);
        }        
    }
    else {
        SetFixedHeight(ui::UiFixedInt((int)m_pList->GetItemCount() * m_iOrgHeight), true, true);
    }
}

void CheckCombo::GetSelectedText(std::vector<DString>& selectedText) const
{
    size_t itemCount = m_pList->GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        Label* pLabel = dynamic_cast<Label*>(m_pList->GetItemAt(index));
        if (pLabel != nullptr) {
            selectedText.push_back(pLabel->GetText());
        }
    }
}

void CheckCombo::ClearAll()
{
    m_pList->RemoveAllItems();
    m_pDropList->RemoveAllItems();
    SetFixedHeight(ui::UiFixedInt(m_iOrgHeight), true, true);
}

} //namespace ui
