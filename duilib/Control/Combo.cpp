#include "Combo.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Animation/PopupAnimation.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

/** 焦点丢失后，延迟确认焦点状态的时间（毫秒）
*/
constexpr uint32_t kFocusLostCheckDelayMs = 20;

/** 下拉列表窗口
*/
class CComboWnd: public Window
{
    typedef Window BaseClass;
public:
    /** 创建并显示下拉窗口
    */
    void InitComboWnd(Combo* pOwner, bool bActivated);

    /** 更新下拉窗口的位置和大小
    */
    void UpdateComboWnd();

    //基类虚函数的重写
    virtual void PreInitWindow() override;
    virtual void OnInitWindow() override;
    virtual void OnCloseWindow() override;
    virtual void OnFinalMessage() override;
    virtual void OnWindowShadowTypeChanged() override;

    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 关闭下拉框
    * @param [in] bCanceled true表示取消，否则表示正常关闭
    * @param [in] needUpdateSelItem true表示需要更新选择项，否则不需要更新选择项
    */
    void CloseComboWnd(bool bCanceled, bool needUpdateSelItem);

    /** 宿主控件析构时关闭下拉窗口（不回调宿主，避免访问正在析构的对象）
    */
    void CloseComboWndOnOwnerDestroy();

private:
    /** 计算下拉框的显示矩形
    */
    UiRect GetComboWndRect() const;

private:
    //关联的Combo接口
    ControlPtrT<Combo> m_pOwner;

    //原来的选择项索引
    size_t m_iOldSel = Box::InvalidIndex;

    //原来Edit控件的文本内容
    UiString m_editText;

    //是否已经关闭
    bool m_bIsClosed = false;

    //窗口显示时的淡入动画播放器（受 PopupAnimation 全局开关控制）
    std::unique_ptr<AnimationPlayer> m_pFadeInPlayer;
};

void CComboWnd::InitComboWnd(Combo* pOwner, bool bActivated)
{
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return;
    }
    m_pOwner = pOwner;
    m_iOldSel = m_pOwner->GetCurSel();
    m_editText = m_pOwner->GetText();
    m_bIsClosed = false;

    //设置下拉框的显示位置和大小，避免弹出界面的时候出现黑屏现象
    UiRect rcWnd = GetComboWndRect();
    WindowCreateParam createWndParam;
    createWndParam.m_dwStyle = kWS_POPUP;
    createWndParam.m_dwExStyle = kWS_EX_LAYERED;
#ifdef DUILIB_BUILD_FOR_SDL
    createWndParam.m_dwExStyle |= kWS_EX_NOACTIVATE;
#endif
    createWndParam.m_nX = rcWnd.left;
    createWndParam.m_nY = rcWnd.top;
    createWndParam.m_nWidth = rcWnd.Width();
    createWndParam.m_nHeight = rcWnd.Height();
    CreateWnd(pOwner->GetWindow(), createWndParam);

    //下拉窗口淡入动画：在显示前把 Alpha 置 0，显示时由动画渐变到完全不透明
    PopupAnimation::PlayFadeIn(this, m_pFadeInPlayer);

    UpdateComboWnd();
    if (bActivated) {
        ShowWindow(ui::kSW_SHOW_NORMAL);
        SetWindowForeground();
        KeepParentActive();
        pOwner->GetTreeView()->SetFocus();
        pOwner->SetState(kControlStateHovered);
    }
    else {
        ShowWindow(ui::kSW_SHOW_NA);
    }
    if (Box::IsValidItemIndex(m_iOldSel)) {
        //展开的时候，确保选择可见
        UpdateWindow();
        pOwner->GetTreeView()->EnsureVisible(m_iOldSel, ListBoxVerVisible::kVisibleAtCenter);
    }

    //发送一个事件
    pOwner->SendEvent(kEventWindowCreate);
}

UiRect CComboWnd::GetComboWndRect() const
{
    ControlPtrT<Combo> pOwner = m_pOwner;
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
    if (pOwner->GetTreeView()->GetFixedHeight().IsStretch() && pOwner->GetTreeView()->GetFixedWidth().IsStretch()) {
        size_t nItemCount = pOwner->GetTreeView()->GetItemCount();
        if (nItemCount > 0) {
            bCanEstimateSize = false;
            for (size_t nItemIndex = 0; nItemIndex < nItemCount; nItemIndex++) {
                Control* pControl = pOwner->GetTreeView()->GetItemAt(nItemIndex);
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
    if (bCanEstimateSize && pOwner->GetTreeView()->GetItemCount() > 0) {
        UiSize szAvailable(rc.Width(), rc.Height());
        UiFixedInt oldFixedHeight = pOwner->GetTreeView()->GetFixedHeight();
        pOwner->GetTreeView()->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
        UiEstSize estSize = pOwner->GetTreeView()->EstimateSize(szAvailable);
        pOwner->GetTreeView()->SetFixedHeight(oldFixedHeight, false, false);
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
    if (rc.bottom > rcWork.bottom || m_pOwner->IsPopupTop()) {
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

void CComboWnd::UpdateComboWnd()
{
    UiRect rc = GetComboWndRect();
    if (rc.IsEmpty()) {
        return;
    }
    SetWindowPos(InsertAfterWnd(), rc.left, rc.top, rc.Width(), rc.Height(), kSWP_NOZORDER | kSWP_NOACTIVATE);
}

void CComboWnd::OnFinalMessage()
{
    if (m_pOwner != nullptr) {
        if (m_pOwner->GetTreeView()->GetWindow() == this) {
            m_pOwner->GetTreeView()->SetWindow(nullptr);
            m_pOwner->GetTreeView()->SetParent(nullptr);
        }
        if (m_pOwner->m_pWindow == this) {            
            m_pOwner->m_pWindow = nullptr;
            m_pOwner->SetState(kControlStateNormal);
            m_pOwner->Invalidate();
        }
    }
    BaseClass::OnFinalMessage();
}

void CComboWnd::OnWindowShadowTypeChanged()
{
    if (IsWindow() && (GetRoot() != nullptr)) {
        UpdateComboWnd();
    }
}

void CComboWnd::CloseComboWnd(bool bCanceled, bool needUpdateSelItem)
{
    if (m_bIsClosed) {
        return;
    }
    m_bIsClosed = true;
    Box* pRootBox = GetRoot();
    if ((m_pOwner != nullptr) && (pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
        m_pOwner->GetTreeView()->SetWindow(nullptr);
        m_pOwner->GetTreeView()->SetParent(nullptr);
        pRootBox->RemoveAllItems();
    }
    //先将前端窗口切换为父窗口，避免前端窗口关闭后，切换到其他窗口
    ControlPtrT<Combo> pOwner = m_pOwner;
    if ((pOwner != nullptr) && (pOwner->GetWindow() != nullptr)) {
        if (IsWindowForeground()) {
            pOwner->GetWindow()->SetWindowForeground();
        }
    }

    CloseWnd();
    if (m_pOwner != nullptr) {
        if (bCanceled) {
            m_pOwner->GetTreeView()->SelectItem(m_iOldSel, false, false);
        }
        m_pOwner->OnComboWndClosed(bCanceled, needUpdateSelItem, m_editText.c_str());
    }
}

void CComboWnd::PreInitWindow()
{
    BaseClass::PreInitWindow();
    //下拉弹窗统一强制使用CPU渲染后端，避免受全局渲染后端配置影响
    SetRenderBackendType(RenderBackendType::kRaster_BackendType, 0, false);
}

void CComboWnd::OnInitWindow()
{
    BaseClass::OnInitWindow();

    Combo* pOwner = m_pOwner.get();
    if (pOwner == nullptr) {
        return;
    }
    SetResourcePath(pOwner->GetWindow()->GetResourcePath());
    SetShadowType(pOwner->GetComboWndShadowType());
    //边框与菜单保持一致：1px 圆角边框由窗口阴影统一绘制，TreeView 不再绘制自身边框
    SetShadowBorderSize(1);
    SetShadowBorderColor(_T("border_window"));

    Box* pRoot = new Box(this);
    pRoot->SetAutoDestroyChild(false);
    pRoot->AddItem(pOwner->GetTreeView());
    AttachBox(AttachShadow(pRoot));

    //更新窗口位置
    UpdateComboWnd();
}

void CComboWnd::CloseComboWndOnOwnerDestroy()
{
    if (m_bIsClosed) {
        return;
    }
    m_bIsClosed = true;
    Box* pRootBox = GetRoot();
    if (pRootBox != nullptr) {
        if (m_pOwner != nullptr) {
            m_pOwner->GetTreeView()->SetWindow(nullptr);
            m_pOwner->GetTreeView()->SetParent(nullptr);
        }
        pRootBox->RemoveAllItems();
    }
    //宿主正在析构，后续不再访问宿主
    m_pOwner.reset();
    CloseWnd();
}

void CComboWnd::OnCloseWindow()
{
    Box* pRootBox = GetRoot();
    if ((m_pOwner != nullptr) && (pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
        m_pOwner->GetTreeView()->SetWindow(nullptr);
        m_pOwner->GetTreeView()->SetParent(nullptr);
        pRootBox->RemoveAllItems();
    }
    //宿主控件可能已在下拉窗口关闭完成前被销毁（例如宿主在选中事件中重建了子控件），需判空
    if ((m_pOwner != nullptr) && (m_pOwner->GetWindow() != nullptr) && m_pOwner->GetWindow()->IsWindow()) {
        m_pOwner->SetPos(m_pOwner->GetPos());
        m_pOwner->SetFocus();
    }    
    BaseClass::OnCloseWindow();
}

LRESULT CComboWnd::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (((vkCode == kVK_UP) || (vkCode == kVK_DOWN)) &&
        (m_pOwner != nullptr) &&
        (m_pOwner->GetComboType() == Combo::kCombo_DropDown)) {
        //下拉窗口被鼠标点击激活后，键盘消息由本窗口接收，需要转发给Combo处理方向键
        m_pOwner->ProcessDropListKeyDown(vkCode);
        bHandled = true;
        return 0;
    }

    LRESULT lResult = BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    if (vkCode == kVK_ESCAPE) {
        //按住ESC键，取消
        CloseComboWnd(true, false);
    }
    else if (vkCode == kVK_RETURN) {
        //按回车键，关闭窗口，正常关闭
        CloseComboWnd(false, true);
    }
    return lResult;
}

LRESULT CComboWnd::OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKillFocusMsg(pSetFocusWindow, nativeMsg, bHandled);
    //失去焦点，关闭窗口，正常关闭
    //注意："不抢焦点"的下拉窗口不持有键盘焦点：SDL下鼠标点击此窗口（或窗口显示）时，
    //系统会短暂把键盘焦点移给本窗口，随后又把焦点还给宿主窗口，此时产生的失焦
    //不属于真正的焦点转移。若在此关闭下拉窗口，会导致点击列表项无响应、
    //DropList模式点击后下拉列表立即关闭。
    //这种窗口的关闭由以下路径负责：宿主窗口失焦、按ESC/回车、选择列表项、再次点击宿主控件。
    const bool bNoActivateWnd = (NativeWnd() != nullptr) && NativeWnd()->IsWindowNoActivate();
    if (bNoActivateWnd) {
        //焦点被还给宿主窗口时，系统可能把宿主窗口置顶，导致下拉窗口被遮挡（看起来像没有展开）：
        //此时把下拉窗口重新置顶（不激活、不抢占焦点）
        Combo* pOwner = m_pOwner.get();
        Window* pOwnerWindow = (pOwner != nullptr) ? pOwner->GetWindow() : nullptr;
        if ((pOwnerWindow != nullptr) && pOwnerWindow->IsWindowForeground()) {
            SetWindowPos(InsertAfterWnd(InsertAfterFlag::kHWND_TOP), 0, 0, 0, 0,
                         kSWP_NOMOVE | kSWP_NOSIZE | kSWP_NOACTIVATE);
        }
    }
    else if (pSetFocusWindow != this) {
        CloseComboWnd(false, false);
    }
    return lResult;
}

////////////////////////////////////////////////////////

Combo::Combo(Window* pWindow) :
    Box(pWindow),
    m_treeView(pWindow),
    m_pWindow(nullptr),
    m_bPopupTop(false),
    m_iCurSel(Box::InvalidIndex),
    m_pIconControl(nullptr),
    m_pEditControl(nullptr),
    m_pButtonControl(nullptr),
    m_comboType(kCombo_DropDown),
    m_bDropListShown(false),
    m_nShadowType(ShadowType::kShadowSmallRound)
{
    SetDropBoxSize({0, 150}, true);
    m_treeView.SetSelectNextWhenActiveRemoved(false);
    m_treeView.AttachSelect(UiBind(&Combo::OnSelectItem, this, std::placeholders::_1));
    //交互控件的键盘焦点反馈：默认显示焦点框（仅在键盘操作后绘制，鼠标操作时不显示）
    SetShowFocusedRect(true);
}

Combo::~Combo()
{
    //宿主析构时先关闭下拉窗口，避免下拉窗口继续持有即将销毁的 TreeView 等成员
    if (m_pWindow != nullptr) {
        m_pWindow->CloseComboWndOnOwnerDestroy();
        m_pWindow = nullptr;
    }
    if (!IsInited()) {
        if (m_pIconControl != nullptr) {
            delete m_pIconControl.get();
            m_pIconControl = nullptr;
        }
        if (m_pEditControl != nullptr) {
            delete m_pEditControl.get();
            m_pEditControl = nullptr;
        }
        if (m_pButtonControl != nullptr) {
            delete m_pButtonControl.get();
            m_pButtonControl = nullptr;
        }
    }
}

DString Combo::GetType() const { return DUI_CTR_COMBO; }

void Combo::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("combo_type")) {
        if (strValue == _T("drop_list")) {
            SetComboType(kCombo_DropList);
        }
        else if (strValue == _T("drop_down")) {
            SetComboType(kCombo_DropDown);
        }
    }
    else if (strName == _T("shadow_type")) {
        //设置下拉窗口的阴影类型
        ShadowType nShadowType = ShadowType::kShadowDefault;
        if (Shadow::GetShadowType(strValue, nShadowType)) {
            SetComboWndShadowType(nShadowType);
        }
    }
    else if ((strName == _T("dropbox_size")) || (strName == _T("dropboxsize")) ) {
        //设置下拉列表的大小（宽度和高度）
        UiSize szDropBoxSize;
        AttributeUtil::ParseSizeValue(strValue.c_str(), szDropBoxSize);
        SetDropBoxSize(szDropBoxSize, true);
    }
    else if ((strName == _T("popup_top")) || (strName == _T("popuptop"))) {
        //下拉列表是否向上弹出
        SetPopupTop(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("combo_tree_view_class")) {
        SetComboTreeClass(strValue);
    }
    else if (strName == _T("combo_tree_node_class")) {
        SetComboTreeNodeClass(strValue);
    }
    else if (strName == _T("combo_icon_class")) {
        SetIconControlClass(strValue);
    }
    else if (strName == _T("combo_edit_class")) {
        SetEditControlClass(strValue);
    }
    else if (strName == _T("combo_button_class")) {
        SetButtonControlClass(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Combo::SetComboWndShadowType(ShadowType nShadowType)
{
    m_nShadowType = nShadowType;
    if (m_pWindow != nullptr) {
        m_pWindow->SetShadowType(nShadowType);
    }
}

ShadowType Combo::GetComboWndShadowType() const
{
    return m_nShadowType;
}

void Combo::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }

    UiSize szDropBoxSize = GetDropBoxSize();
    szDropBoxSize = Dpi().GetScaleSize(szDropBoxSize, nOldDpiScale);
    SetDropBoxSize(szDropBoxSize, false);

    if (m_treeView.GetWindow() == nullptr) {
        m_treeView.SetWindow(GetWindow());
    }
    if ((m_treeView.GetWindow() == GetWindow()) && (m_treeView.GetParent() == nullptr)) {
        m_treeView.ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void Combo::SetComboTreeClass(const DString& classValue)
{
    if (m_treeView.GetWindow() == nullptr) {
        m_treeView.SetWindow(GetWindow());
    }
    SetAttributeList(&m_treeView, classValue);
}

void Combo::SetComboTreeNodeClass(const DString& classValue)
{
    m_treeNodeClass = classValue;
}

void Combo::SetIconControlClass(const DString& classValue)
{
    if (classValue.empty()) {
        RemoveControl(m_pIconControl.get());
        if (m_pIconControl != nullptr) {
            delete m_pIconControl.get();
            m_pIconControl = nullptr;
        }
    }
    else {
        if (m_pIconControl == nullptr) {
            m_pIconControl = new Control(GetWindow());
        }
        SetAttributeList(m_pIconControl.get(), classValue);
    }
}

void Combo::SetEditControlClass(const DString& classValue)
{
    if (classValue.empty()) {
        RemoveControl(m_pEditControl.get());
        if (m_pEditControl != nullptr) {
            delete m_pEditControl.get();
            m_pEditControl = nullptr;
        }
    }
    else {
        if (m_pEditControl == nullptr) {
            m_pEditControl = new RichEdit(GetWindow());
        }
        SetAttributeList(m_pEditControl.get(), classValue);
    }    
}

void Combo::SetButtonControlClass(const DString& classValue)
{
    if (classValue.empty()) {
        RemoveControl(m_pButtonControl.get());
        if (m_pButtonControl != nullptr) {
            delete m_pButtonControl.get();
            m_pButtonControl = nullptr;
        }
    }
    else {
        if (m_pButtonControl == nullptr) {
            m_pButtonControl = new Button(GetWindow());
        }
        SetAttributeList(m_pButtonControl.get(), classValue);
    }
}

void Combo::SetAttributeList(Control* pControl, const DString& classValue)
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
    else if(!classValue.empty()) {
        //按Class名称设置
        pControl->SetClass(classValue);
    }
}

void Combo::RemoveControl(Control* pControl)
{
    if (IsInited() && (GetItemCount() > 0)) {
        HBox* pBox = dynamic_cast<HBox*>(GetItemAt(0));
        if (pBox != nullptr) {
            pBox->RemoveItem(pControl);
        }
    }
}

bool Combo::CanPlaceCaptionBar() const
{
    return true;
}

DString Combo::GetBorderColor(ControlStateType stateType) const
{
    DString borderColor;
    if (m_pIconControl != nullptr) {
        if (m_pIconControl->IsFocused() || m_pIconControl->IsMouseFocused()) {
            borderColor = BaseClass::GetBorderColor(kControlStateHovered);
        }
    }
    if (borderColor.empty() && (m_pEditControl != nullptr)) {
        if (m_pEditControl->IsFocused() || m_pEditControl->IsMouseFocused()) {
            borderColor = BaseClass::GetBorderColor(kControlStateHovered);
        }
    }
    if (borderColor.empty() && (m_pButtonControl != nullptr)) {
        if (m_pButtonControl->IsFocused() || m_pButtonControl->IsMouseFocused()) {
            borderColor = BaseClass::GetBorderColor(kControlStateHovered);
        }
    }
    if (borderColor.empty() && (m_pWindow != nullptr) && !m_pWindow->IsClosingWnd()) {
        borderColor = BaseClass::GetBorderColor(kControlStateHovered);
    }
    if (borderColor.empty()) {
        borderColor = BaseClass::GetBorderColor(stateType);
    }
    return borderColor;
}

void Combo::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    HBox* pBox = new HBox(GetWindow());
    AddItem(pBox);
    pBox->SetNoFocus();
    AttachMouseEvents(pBox);

    if (m_pIconControl != nullptr) {        
        pBox->AddItem(m_pIconControl.get());
        AttachMouseEvents(m_pIconControl.get());
    }
    if (m_pEditControl != nullptr) {
        pBox->AddItem(m_pEditControl.get());
    }
    if (m_pButtonControl != nullptr) {
        pBox->AddItem(m_pButtonControl.get());
        AttachMouseEvents(m_pButtonControl.get());
    }

    if (m_pIconControl != nullptr) {
        m_pIconControl->SetNoFocus();
    }
    if (m_pButtonControl != nullptr) {
        m_pButtonControl->SetNoFocus();        
        m_pButtonControl->AttachButtonDown(UiBind(&Combo::OnButtonDown, this, std::placeholders::_1));
        m_pButtonControl->AttachClick(UiBind(&Combo::OnButtonClicked, this, std::placeholders::_1));
    }
    if (m_pEditControl != nullptr) {
        m_pEditControl->SetWantReturn(false);
        m_pEditControl->SetWantCtrlReturn(false);
        m_pEditControl->AttachButtonDown(UiBind(&Combo::OnEditButtonDown, this, std::placeholders::_1));
        m_pEditControl->AttachButtonUp(UiBind(&Combo::OnEditButtonUp, this, std::placeholders::_1));
        m_pEditControl->AttachEvent(kEventKeyDown, UiBind(&Combo::OnEditKeyDown, this, std::placeholders::_1), 0);
        m_pEditControl->AttachSetFocus(UiBind(&Combo::OnEditSetFocus, this, std::placeholders::_1));
        m_pEditControl->AttachKillFocus(UiBind(&Combo::OnEditKillFocus, this, std::placeholders::_1));
        m_pEditControl->AttachEvent(kEventWindowKillFocus, UiBind(&Combo::OnWindowKillFocus, this, std::placeholders::_1), 0);
        m_pEditControl->AttachEvent(kEventWindowMove, UiBind(&Combo::OnWindowMove, this, std::placeholders::_1), 0);
        m_pEditControl->AttachEvent(kEventWindowPosChanged, UiBind(&Combo::OnWindowMove, this, std::placeholders::_1), 0);
        m_pEditControl->AttachTextChanged(UiBind(&Combo::OnEditTextChanged, this, std::placeholders::_1));
    }
    SetNoFocus();
    SetComboType(GetComboType());
}

TreeView* Combo::GetTreeView()
{
    return &m_treeView;
}

Control* Combo::GetIconControl() const
{
    return m_pIconControl.get();
}

RichEdit* Combo::GetEditControl() const
{
    return m_pEditControl.get();
}

Button* Combo::GetButtonContrl() const
{
    return m_pButtonControl.get();
}

void Combo::SetComboType(ComboType comboType)
{
    if (comboType == kCombo_DropList) {
        m_comboType = kCombo_DropList;
        if (m_pEditControl != nullptr) {
            //设置为只读，不显示光标
            m_pEditControl->SetReadOnly(true);
            m_pEditControl->SetNoCaretReadonly();
            m_pEditControl->SetCursorType(CursorType::kCursorArrow);
            m_pEditControl->SetUseControlCursor(true);
        }
    }
    else if (comboType == kCombo_DropDown) {
        m_comboType = kCombo_DropDown;
        if (m_pEditControl != nullptr) {
            m_pEditControl->SetReadOnly(false);
            m_pEditControl->SetUseControlCursor(false);
        }
    }
}

Combo::ComboType Combo::GetComboType() const
{
    if (m_comboType == kCombo_DropList) {
        return kCombo_DropList;
    }
    else if (m_comboType == kCombo_DropDown) {
        return kCombo_DropDown;
    }
    return kCombo_DropList;
}

const UiSize& Combo::GetDropBoxSize() const
{
    return m_szDropBox;
}

void Combo::SetDropBoxSize(UiSize szDropBox, bool bNeedScaleDpi)
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

size_t Combo::GetCount() const
{
    return m_treeView.GetRootNode()->GetDescendantNodeCount();
}

size_t Combo::GetCurSel() const
{ 
    return m_treeView.GetCurSel();
}

bool Combo::SetCurSel(size_t iIndex, bool bTriggerEvent)
{
    size_t iOldSel = m_iCurSel;
    bool bRet = m_treeView.SelectItem(iIndex, false, false);
    m_iCurSel = m_treeView.GetCurSel();
    OnSelectedItemChanged();
    if (m_iCurSel != iOldSel) {
        Invalidate();

        //触发选择变化事件
        if (bTriggerEvent) {
            SendEvent(kEventSelect, m_iCurSel, iOldSel);
        }        
    }  
    return bRet;
}

size_t Combo::GetItemData(size_t iIndex) const
{
    Control* pControl = m_treeView.GetItemAt(iIndex);
    if (pControl != nullptr) {
        ASSERT(dynamic_cast<TreeNode*>(pControl) != nullptr);
        return pControl->GetUserDataID();
    }
    return 0;
}

bool Combo::HasItemData(size_t iIndex) const
{
    Control* pControl = m_treeView.GetItemAt(iIndex);
    if (pControl != nullptr) {
        ASSERT(dynamic_cast<TreeNode*>(pControl) != nullptr);
        return true;
    }
    return false;
}

bool Combo::SetItemData(size_t iIndex, size_t itemData)
{
    Control* pControl = m_treeView.GetItemAt(iIndex);
    if (pControl != nullptr) {
        ASSERT(dynamic_cast<TreeNode*>(pControl) != nullptr);
        pControl->SetUserDataID(itemData);
        return true;
    }
    return false;
}

DString Combo::GetItemText(size_t iIndex) const
{
    Control* pControl = m_treeView.GetItemAt(iIndex);
    if (pControl != nullptr) {
        TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
        ASSERT(pTreeNode != nullptr);
        if (pTreeNode != nullptr) {
            return pTreeNode->GetText();
        }        
    }
    return DString();
}

DString Combo::GetItemTextId(size_t iIndex) const
{
    Control* pControl = m_treeView.GetItemAt(iIndex);
    if (pControl != nullptr) {
        TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
        ASSERT(pTreeNode != nullptr);
        if (pTreeNode != nullptr) {
            return pTreeNode->GetTextId();
        }
    }
    return DString();
}

bool Combo::SetItemText(size_t iIndex, const DString& itemText)
{
    Control* pControl = m_treeView.GetItemAt(iIndex);
    if (pControl != nullptr) {
        TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
        ASSERT(pTreeNode != nullptr);
        if (pTreeNode != nullptr) {
            pTreeNode->SetText(itemText);
            OnSelectedItemChanged();
            return true;
        }
    }
    return false;
}

bool Combo::SetItemTextId(size_t iIndex, const DString& itemTextId)
{
    Control* pControl = m_treeView.GetItemAt(iIndex);
    if (pControl != nullptr) {
        TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
        ASSERT(pTreeNode != nullptr);
        if (pTreeNode != nullptr) {
            pTreeNode->SetTextId(itemTextId);
            OnSelectedItemChanged();
            return true;
        }
    }
    return false;
}

size_t Combo::AddTextItem(const DString& itemText)
{
    return InsertTextItem(GetCount(), itemText);
}

size_t Combo::AddTextIdItem(const DString& itemTextId)
{
    return PrivateInsertTextItem(GetCount(), itemTextId, true);
}

size_t Combo::InsertTextItem(size_t iIndex, const DString& itemText)
{
    return PrivateInsertTextItem(iIndex, itemText, false);
}

size_t Combo::InsertTextIdItem(size_t iIndex, const DString& itemTextId)
{
    return PrivateInsertTextItem(iIndex, itemTextId, true);
}

size_t Combo::PrivateInsertTextItem(size_t iIndex, const DString& itemText, bool bTextId)
{
    ASSERT(iIndex <= GetCount());
    if (iIndex > GetCount()) {
        return Box::InvalidIndex;
    }
    size_t newIndex = Box::InvalidIndex;
    if (iIndex == GetCount()) {
        //在最后面插入新的节点
        TreeNode* pNewNode = CreateTreeNode(itemText, bTextId);
        m_treeView.GetRootNode()->AddChildNode(pNewNode);
        newIndex = m_treeView.GetItemIndex(pNewNode);
    }
    else {
        //在指定位置插入新的节点
        Control* pControl = m_treeView.GetItemAt(iIndex);
        if (pControl != nullptr) {
            TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
            ASSERT(pTreeNode != nullptr);
            if (pTreeNode != nullptr) {
                TreeNode* pParentNode = pTreeNode->GetParentNode();
                ASSERT(pParentNode != nullptr);
                if (pParentNode != nullptr) {
                    size_t iChildIndex = pParentNode->GetChildNodeIndex(pTreeNode);
                    TreeNode* pNewNode = CreateTreeNode(itemText, bTextId);
                    pParentNode->AddChildNodeAt(pNewNode, iChildIndex);
                    newIndex = m_treeView.GetItemIndex(pNewNode);
                }
            }
        }
    }
    ASSERT(newIndex != Box::InvalidIndex);
    return newIndex;
}

bool Combo::DeleteItem(size_t iIndex)
{
    bool bRemoved = false;
    Control* pControl = m_treeView.GetItemAt(iIndex);
    if (pControl != nullptr) {
        TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
        ASSERT(pTreeNode != nullptr);
        if (pTreeNode != nullptr) {
            TreeNode* pParentNode = pTreeNode->GetParentNode();
            ASSERT(pParentNode != nullptr);
            if (pParentNode != nullptr) {
                bRemoved = pParentNode->RemoveChildNode(pTreeNode);
            }
        }
    }
    if (bRemoved) {
        OnSelectedItemChanged();
    }
    return bRemoved;
}

void Combo::DeleteAllItems()
{
    m_iCurSel = Box::InvalidIndex;
    m_treeView.GetRootNode()->RemoveAllChildNodes();
    OnSelectedItemChanged();
}

size_t Combo::SelectTextItem(const DString& itemText, bool bTriggerEvent)
{
    size_t nSelIndex = Box::InvalidIndex;
    size_t itemCount = m_treeView.GetItemCount();
    for (size_t nIndex = 0; nIndex < itemCount; ++nIndex) {
        Control* pControl = m_treeView.GetItemAt(nIndex);
        if (pControl != nullptr) {
            TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
            ASSERT(pTreeNode != nullptr);
            if (pTreeNode != nullptr) {
                if (pTreeNode->GetText() == itemText) {
                    nSelIndex = nIndex;
                    break;
                }
            }
        }
    }
    m_treeView.SelectItem(nSelIndex, false, bTriggerEvent);
    if (!bTriggerEvent) {
        OnSelectedItemChanged();
    }
    return nSelIndex;
}

TreeNode* Combo::CreateTreeNode(const DString& itemText, bool bTextId)
{
    TreeNode* pNewNode = new TreeNode(GetWindow());
    if (!m_treeNodeClass.empty()) {
        SetAttributeList(pNewNode, m_treeNodeClass.c_str());
    }
    if (bTextId) {
        pNewNode->SetTextId(itemText);
    }
    else {
        pNewNode->SetText(itemText);
    }    
    return pNewNode;
}

DString Combo::GetText() const
{
    if (m_pEditControl != nullptr) {
        return m_pEditControl->GetText();
    }
    return DString();
}

void Combo::SetText(const DString& text)
{
    if (m_pEditControl != nullptr) {
        m_pEditControl->SetText(text);
    }
}

bool Combo::OnSelectItem(const EventArgs& /*args*/)
{
    size_t iOldSel = m_iCurSel;
    m_iCurSel = m_treeView.GetCurSel();
    OnSelectedItemChanged();
    if (m_iCurSel != iOldSel) {
        SendEvent(kEventSelect, m_iCurSel, iOldSel);
        Invalidate();
    }
    if (Box::IsValidItemIndex(m_iCurSel) && (m_pWindow != nullptr) && !m_pWindow->IsClosingWnd()) {
        const Control* pControl = m_treeView.GetItemAt(m_iCurSel);
        if (pControl != nullptr) {
            //如果是鼠标触发选择，那么关闭下拉列表：
            //1. 下拉窗口的点击控件不一定是条目控件本身（例如点击条目内的子控件）；
            //2. 鼠标按下时可能已经触发选择，此时左键仍处于按下状态。
            //键盘触发的选择不关闭下拉列表，等用户按回车键后再关闭。
            const Control* pEventClick = m_pWindow->GetEventClick();
            const bool bMouseSelect = (pEventClick != nullptr) || Keyboard::IsKeyDown(kVK_LBUTTON);
            if (bMouseSelect) {
                m_pWindow->CloseComboWnd(false, false);
            }
        }
    }
    return true;
}

void Combo::OnComboWndClosed(bool bCanceled, bool needUpdateSelItem, const DString& oldEditText)
{
    if (bCanceled) {
        size_t iOldSel = m_iCurSel;
        m_iCurSel = m_treeView.GetCurSel();
        if (m_iCurSel != iOldSel) {
            SendEvent(kEventSelect, m_iCurSel, iOldSel);
            Invalidate();
        }
        SetText(oldEditText);
    }
    if (needUpdateSelItem) {
        OnSelectedItemChanged();
    }
    SendEvent(kEventWindowClose);
    Invalidate();
}

bool Combo::OnButtonDown(const EventArgs& /*args*/)
{
    m_bDropListShown = (m_pWindow != nullptr) ? true : false;
    if (m_pWindow != nullptr) {
        //如果下拉框是显示状态，则再次点击的时候，关闭
        HideComboList();
    }
    return true;
}

bool Combo::OnButtonClicked(const EventArgs& /*args*/)
{
    //如果鼠标按下的时候，正在显示列表，那么点击后不显示下拉列表
    if (!m_bDropListShown) {        
        ShowComboList();
    }
    if (m_comboType == kCombo_DropDown) {
        if (m_pEditControl != nullptr) {
            m_pEditControl->SetFocus();
        }
    }
    return true;
}

bool Combo::OnEditButtonDown(const EventArgs& /*args*/)
{
    if (m_comboType == kCombo_DropList) {
        m_bDropListShown = (m_pWindow != nullptr) ? true : false;
    }
    return true;
}

bool Combo::OnEditButtonUp(const EventArgs& /*args*/)
{
    if (m_comboType == kCombo_DropList) {
        if (m_bDropListShown) {
            //如果鼠标按下的时候，正在显示列表，那么点击后不显示下拉列表
            return true;
        }
        //显示下拉列表
        ShowComboList();
    }
    return true;
}

bool Combo::OnEditKeyDown(const EventArgs& args)
{
    const VirtualKeyCode vkCode = (VirtualKeyCode)args.vkCode;
    const bool bListShown = (m_pWindow != nullptr) && !m_pWindow->IsClosingWnd();
    const bool bAltDownOpen = (vkCode == kVK_DOWN) && IsKeyDown(args, ModifierKey::kAlt);

    //按方向键打开下拉列表时：如果没有选中项，则定位到首项/末项
    auto selectOnOpen = [this](bool bSelectLast) {
        if (Box::IsValidItemIndex(m_treeView.GetCurSel())) {
            return;
        }
        const size_t itemCount = GetCount();
        if (itemCount == 0) {
            return;
        }
        m_treeView.SelectItem(bSelectLast ? (itemCount - 1) : 0, false, true);
        };

    if (vkCode == kVK_ESCAPE) {
        //按住ESC键，取消
        if (bListShown) {
            m_pWindow->CloseComboWnd(true, false);
            return false;
        }
        return true;
    }

    if (m_comboType == kCombo_DropList) {
        //不可编辑模式：Alt+Down、F4 在打开/关闭下拉列表之间切换
        if ((vkCode == kVK_F4) || bAltDownOpen) {
            if (bListShown) {
                m_pWindow->CloseComboWnd(false, false);
            }
            else {
                ShowComboList();
            }
            return false;
        }
        //不可编辑模式：Up/Down 打开下拉列表并选择，Enter/空格打开或确认
        if ((vkCode == kVK_UP) || (vkCode == kVK_DOWN) ||
            (vkCode == kVK_RETURN) || (vkCode == kVK_SPACE)) {
            if (!bListShown) {
                if (vkCode == kVK_UP) {
                    selectOnOpen(true);
                }
                else if (vkCode == kVK_DOWN) {
                    selectOnOpen(false);
                }
                ShowComboList();
            }
            else if ((vkCode == kVK_RETURN) || (vkCode == kVK_SPACE)) {
                m_pWindow->CloseComboWnd(false, false);
            }
            return false;
        }
        return true;
    }

    //可编辑模式(kCombo_DropDown)：Alt+Down、F4 在打开/关闭下拉列表之间切换
    if ((vkCode == kVK_F4) || bAltDownOpen) {
        if (bListShown) {
            m_pWindow->CloseComboWnd(false, false);
        }
        else {
            ShowComboList();
        }
        return false;
    }

    //可编辑模式(kCombo_DropDown)
    if (vkCode == kVK_DOWN) {
        if (!bListShown) {
            //按向下箭头打开下拉列表时，选中项定位到首项
            selectOnOpen(false);
            ShowComboList();
        }
        else {
            //按向下箭头的时候: 如果已经显示，则进行选择切换
            const size_t itemCount = GetCount();
            const size_t nCurSel = m_treeView.GetCurSel();
            if (!Box::IsValidItemIndex(nCurSel)) {
                //如果未选择，则开始选择第一项
                if (itemCount > 0) {
                    m_treeView.SelectItem(0, false, true);
                }
            }
            else {
                if ((itemCount > 0) && (nCurSel < itemCount)) {
                    if (nCurSel == (itemCount - 1)) {
                        //如果已经是最后一项，则取消选择
                        m_treeView.SelectItem(Box::InvalidIndex, false, true);
                    }
                    else {
                        //如果不是最后一项，则选择下一项
                        m_treeView.SelectItem(nCurSel + 1, false, true);
                    }
                }
            }
        }
        return false;
    }
    else if (vkCode == kVK_UP) {
        //按向上箭头
        if (!bListShown) {
            //打开下拉列表，并定位到末项
            selectOnOpen(true);
            ShowComboList();
        }
        else {
            const size_t itemCount = GetCount();
            const size_t nCurSel = m_treeView.GetCurSel();
            if (Box::IsValidItemIndex(nCurSel)) {
                if (nCurSel == 0) {
                    //已经是第一项，取消选择
                    m_treeView.SelectItem(Box::InvalidIndex, false, true);
                }
                else {
                    //不是第一项，选择前面一项
                    m_treeView.SelectItem(nCurSel - 1, false, true);
                }
            }
            else {
                //如果未选择，则选择最后一项
                m_treeView.SelectItem(itemCount - 1, false, true);
            }
        }
        return false;
    }
    else if (vkCode == kVK_RETURN) {
        //按回车键，关闭窗口，正常关闭
        if (bListShown) {
            m_pWindow->CloseComboWnd(false, false);
            return false;
        }
        return true;
    }
    return true;
}

bool Combo::ProcessDropListKeyDown(VirtualKeyCode vkCode)
{
    EventArgs args;
    args.eventType = kEventKeyDown;
    args.wParam = vkCode;
    args.vkCode = vkCode;
    return OnEditKeyDown(args);
}

bool Combo::OnEditSetFocus(const EventArgs& /*args*/)
{
    //将RichEdit控件的焦点状态，作为Combo的焦点状态
    SendEvent(kEventSetFocus);
    return true;
}

bool Combo::OnEditKillFocus(const EventArgs& /*args*/)
{
    if (m_pWindow != nullptr) {
        if (m_pWindow->IsWindowFocused()) {
            return true;
        }
    }
    HideComboListOnFocusLost();
    Invalidate();
    //将RichEdit控件的焦点状态，作为Combo的焦点状态
    SendEvent(kEventKillFocus);
    return true;
}

bool Combo::OnWindowKillFocus(const EventArgs& /*args*/)
{
    if (m_pWindow != nullptr) {
        if (m_pWindow->IsWindowFocused()) {
            return true;
        }
    }
    HideComboListOnFocusLost();
    return true;
}

bool Combo::OnWindowMove(const EventArgs& /*args*/)
{
    UpdateComboList();
    return true;
}

void Combo::OnSelectedItemChanged()
{
    if (m_pEditControl != nullptr) {
        size_t nSelIndex = GetCurSel();
        if (Box::IsValidItemIndex(nSelIndex)) {
            m_pEditControl->SetTextNoEvent(GetItemText(nSelIndex));
        }
        else {
            m_pEditControl->SetTextNoEvent(DString());
        }
    }
}

void Combo::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    //语言发生变化
    ComboType comboType = GetComboType();
    if (comboType == ComboType::kCombo_DropList) {
        //不可编辑时，直接覆盖编辑框
        OnSelectedItemChanged();
    }
    else {
        //可编辑时，也直接覆盖编辑框（后续有好的策略时，考虑修改）
        if (m_pEditControl != nullptr) {
            size_t nSelIndex = GetCurSel();
            if (Box::IsValidItemIndex(nSelIndex)) {
                m_pEditControl->SetTextNoEvent(GetItemText(nSelIndex));
            }
        }
    }
}

bool Combo::OnEditTextChanged(const ui::EventArgs& /*args*/)
{
    if ((m_pWindow != nullptr) && !m_pWindow->IsClosingWnd()) {
        DString editText = GetText();
        //转换成小写，比较的时候，不区分大小写
        editText = StringUtil::MakeLowerString(editText);
        size_t itemCount = m_treeView.GetItemCount();
        for (size_t iIndex = 0; iIndex < itemCount; ++iIndex) {
            Control* pControl = m_treeView.GetItemAt(iIndex);
            if (pControl != nullptr) {
                TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
                ASSERT(pTreeNode != nullptr);
                if (pTreeNode != nullptr) {
                    pTreeNode->SetExpand(true, false);
                    DString nodeText = StringUtil::MakeLowerString(pTreeNode->GetText());
                    if (nodeText.find(editText) != DString::npos) {
                        m_treeView.EnsureVisible(iIndex, ListBoxVerVisible::kVisibleAtCenter);
                        break;
                    }
                }
            }
        }
    }
    return true;
}

void Combo::ShowComboList()
{
    //显示下拉列表
    if ((m_pWindow == nullptr) || m_pWindow->IsClosingWnd()) {
        m_pWindow = new CComboWnd();
        if (m_comboType == kCombo_DropList) {
            m_pWindow->InitComboWnd(this, true);
        }
        else {
            m_pWindow->InitComboWnd(this, false);
        }        
    }
}

void Combo::HideComboList()
{
    if(m_pWindow != nullptr) {
        m_pWindow->CloseComboWnd(false, false);
    }
}

void Combo::HideComboListOnFocusLost()
{
    if (m_pWindow == nullptr) {
        return;
    }
    if (m_nFocusLostTimerId != 0) {
        GlobalManager::Instance().Timer().RemoveTimer(m_nFocusLostTimerId);
        m_nFocusLostTimerId = 0;
    }
    Window* pComboWnd = m_pWindow;
    m_nFocusLostTimerId = GlobalManager::Instance().Timer().AddTimer(
        GetWeakFlag(),
        [this, pComboWnd]() {
            m_nFocusLostTimerId = 0;
            if ((m_pWindow == nullptr) || (m_pWindow != pComboWnd)) {
                //下拉窗口已经关闭或者被新的下拉窗口替换
                return;
            }
            Window* pOwnerWindow = GetWindow();
            if ((pOwnerWindow != nullptr) && !pOwnerWindow->IsWindowFocused()) {
                //宿主窗口已不是活动窗口（例如切换到其他窗口或者应用）：关闭下拉列表，
                //避免应用失去激活状态后下拉列表残留显示
                HideComboList();
                return;
            }
            if (m_pWindow->IsWindowFocused() || m_pWindow->IsWindowForeground()) {
                //焦点在下拉窗口中，由下拉窗口自己处理关闭
                return;
            }
            if ((m_pEditControl != nullptr) && m_pEditControl->IsFocused()) {
                //焦点仍在本控件的编辑框中，保持下拉列表展开
                return;
            }
            if (IsFocused()) {
                //焦点仍在本控件
                return;
            }
            HideComboList();
        },
        kFocusLostCheckDelayMs, 1);
    if (m_nFocusLostTimerId == 0) {
        //定时器创建失败：直接关闭
        HideComboList();
    }
}

void Combo::UpdateComboList()
{
    if (m_pWindow != nullptr) {
        m_pWindow->UpdateComboWnd();
    }
}

void Combo::UpdateComboWndPos()
{
    if (m_pWindow != nullptr) {
        m_pWindow->UpdateComboWnd();
    }
}

Window* Combo::GetComboWnd() const
{
    return m_pWindow;
}

void Combo::AttachMouseEvents(Control* pControl)
{
    if (pControl == nullptr) {
        return;
    }
    auto SetRichEditFocus = [this]() {
        if ((m_pEditControl != nullptr) && m_pEditControl->IsVisible()) {
            if (!m_pEditControl->IsFocused()) {
                m_pEditControl->SetFocus();
            }
        }
        };
    pControl->AttachButtonDown([SetRichEditFocus](const EventArgs&) {
        SetRichEditFocus();
        return true;
        });
    pControl->AttachRButtonDown([SetRichEditFocus](const EventArgs&) {
        SetRichEditFocus();
        return true;
        });
}

void Combo::SetFocus()
{
    //将焦点设置在Edit控件上
    if (IsNoFocus()) {
        if ((m_pEditControl != nullptr) && m_pEditControl->IsVisible()) {
            if (!m_pEditControl->IsFocused()) {
                m_pEditControl->SetFocus();
            }
        }
    }
    else {
        BaseClass::SetFocus();
    }
}

} // namespace ui
