#include "ComboButton.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Core/WindowAnchorFollower.h"
#include "duilib/Box/HBox.h"

namespace ui
{

/** 下拉列表窗口
*/
class ComboButtonWnd: public Window
{
    typedef Window BaseClass;
public:
    void InitComboWnd(ComboButton* pOwner, bool bActivated);
    void UpdateComboWnd();
    virtual void PreInitWindow() override;
    virtual void OnInitWindow() override;
    virtual void OnCloseWindow() override;
    virtual void OnFinalMessage() override;
    virtual void OnWindowShadowTypeChanged() override;

    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 关闭下拉框
    * @param [in] bCanceled true表示取消，否则表示正常关闭
    */
    void CloseComboWnd(bool bCanceled);

    /** 宿主控件析构时关闭下拉窗口（不回调宿主，避免访问正在析构的对象）
    */
    void CloseComboWndOnOwnerDestroy();

private:
    /** 计算下拉窗口的显示矩形（屏幕坐标，包含阴影区域）
    */
    UiRect GetComboWndRect() const;

private:
    //关联的宿主控件（弱引用，宿主析构后自动失效）
    ControlPtrT<ComboButton> m_pOwner;

    //是否已经关闭
    bool m_bIsClosed = false;

    //跟随宿主控件的位置变化（宿主窗口移动、所在滚动容器滚动）
    WindowAnchorFollower m_anchorFollower;
};

void ComboButtonWnd::InitComboWnd(ComboButton* pOwner, bool bActivated)
{
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return;
    }
    m_pOwner = pOwner;
    m_bIsClosed = false;

    //设置下拉框的显示位置和大小：创建时就使用最终位置，避免弹出时出现位置跳变
    //（macOS(Cocoa)显示窗口时会先按创建时的位置摆放一帧，再应用后续定位）
    const UiRect rcWnd = GetComboWndRect();
    WindowCreateParam createWndParam;
    createWndParam.m_dwStyle = kWS_POPUP;
    createWndParam.m_dwExStyle = kWS_EX_LAYERED;
    if (!rcWnd.IsEmpty()) {
        createWndParam.m_nX = rcWnd.left;
        createWndParam.m_nY = rcWnd.top;
        createWndParam.m_nWidth = rcWnd.Width();
        createWndParam.m_nHeight = rcWnd.Height();
    }
    CreateWnd(pOwner->GetWindow(), createWndParam);
    UpdateComboWnd();
    if (bActivated) {
        ShowWindow(ui::kSW_SHOW_NORMAL);
        KeepParentActive();
        pOwner->SetState(kControlStateHovered);
    }
    else {
        ShowWindow(ui::kSW_SHOW_NA);
    }

    //跟随宿主控件：窗口移动或滚动容器滚动时，下拉窗口同步更新位置
    m_anchorFollower.Start(
        [this]() -> UiRect {
            ComboButton* pOwner = m_pOwner.get();
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

void ComboButtonWnd::UpdateComboWnd()
{
    const UiRect rc = GetComboWndRect();
    if (rc.IsEmpty()) {
        return;
    }
    SetWindowPos(InsertAfterWnd(), rc.left, rc.top, rc.Width(), rc.Height(), kSWP_NOZORDER | kSWP_NOACTIVATE);
}

UiRect ComboButtonWnd::GetComboWndRect() const
{
    ComboButton* pOwner = m_pOwner.get();
    if (pOwner == nullptr) {
        return UiRect();
    }
    ui::Box* pComboBox = pOwner->GetComboBox();
    if (pComboBox == nullptr) {
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
    if (pComboBox->GetFixedHeight().IsStretch() && pComboBox->GetFixedWidth().IsStretch()) {
        size_t nItemCount = pComboBox->GetItemCount();
        if (nItemCount > 0) {
            bCanEstimateSize = false;
            for (size_t nItemIndex = 0; nItemIndex < nItemCount; nItemIndex++) {
                Control* pControl = pComboBox->GetItemAt(nItemIndex);
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
    if (bCanEstimateSize && pComboBox->GetItemCount() > 0) {
        UiSize szAvailable(rc.Width(), rc.Height());
        UiFixedInt oldFixedHeight = pComboBox->GetFixedHeight();
        pComboBox->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
        UiEstSize estSize = pComboBox->EstimateSize(szAvailable);
        pComboBox->SetFixedHeight(oldFixedHeight, false, false);
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
    return rc;
}

void ComboButtonWnd::OnFinalMessage()
{
    m_anchorFollower.Stop();
    ComboButton* pOwner = m_pOwner.get();
    if (pOwner != nullptr) {
        if (pOwner->GetComboBox()->GetWindow() == this) {
            pOwner->GetComboBox()->SetWindow(nullptr);
            pOwner->GetComboBox()->SetParent(nullptr);
        }
        if (pOwner->m_pWindow == this) {            
            pOwner->m_pWindow = nullptr;
            pOwner->SetState(kControlStateNormal);
            pOwner->Invalidate();
        }
    }
    BaseClass::OnFinalMessage();
}

void ComboButtonWnd::OnWindowShadowTypeChanged()
{
    if (IsWindow() && (GetRoot() != nullptr)) {
        UpdateComboWnd();
    }
}

void ComboButtonWnd::CloseComboWnd(bool bCanceled)
{
    if (m_bIsClosed) {
        return;
    }
    m_bIsClosed = true;
    m_anchorFollower.Stop();
    Box* pRootBox = GetRoot();
    ComboButton* pOwner = m_pOwner.get();
    if ((pOwner != nullptr) && (pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
        pOwner->GetComboBox()->SetWindow(nullptr);
        pOwner->GetComboBox()->SetParent(nullptr);
        pRootBox->RemoveAllItems();
    }
    //先将前端窗口切换为父窗口，避免前端窗口关闭后，切换到其他窗口
    if ((pOwner != nullptr) && (pOwner->GetWindow() != nullptr)) {
        if (IsWindowForeground()) {
            pOwner->GetWindow()->SetWindowForeground();
        }
    }
    CloseWnd();
    if (pOwner != nullptr) {
        pOwner->OnComboWndClosed(bCanceled);
    }
}

void ComboButtonWnd::CloseComboWndOnOwnerDestroy()
{
    if (m_bIsClosed) {
        return;
    }
    m_bIsClosed = true;
    m_anchorFollower.Stop();
    Box* pRootBox = GetRoot();
    ComboButton* pOwner = m_pOwner.get();
    if (pRootBox != nullptr) {
        if (pOwner != nullptr) {
            pOwner->GetComboBox()->SetWindow(nullptr);
            pOwner->GetComboBox()->SetParent(nullptr);
        }
        pRootBox->RemoveAllItems();
    }
    //宿主正在析构，后续不再访问宿主
    m_pOwner.reset();
    CloseWnd();
}

void ComboButtonWnd::PreInitWindow()
{
    BaseClass::PreInitWindow();
    //下拉弹窗统一强制使用CPU渲染后端，避免受全局渲染后端配置影响
    SetRenderBackendType(RenderBackendType::kRaster_BackendType, 0, false);
}

void ComboButtonWnd::OnInitWindow()
{
    BaseClass::OnInitWindow();
    ComboButton* pOwner = m_pOwner.get();
    if (pOwner == nullptr) {
        return;
    }
    SetResourcePath(pOwner->GetWindow()->GetResourcePath());
    SetShadowType(pOwner->GetComboWndShadowType());

    Box* pRoot = new Box(this);
    pRoot->SetAutoDestroyChild(false);
    pRoot->AddItem(pOwner->GetComboBox());
    AttachBox(AttachShadow(pRoot));

    //更新窗口位置
    UpdateComboWnd();
}

void ComboButtonWnd::OnCloseWindow()
{
    m_anchorFollower.Stop();
    Box* pRootBox = GetRoot();
    ComboButton* pOwner = m_pOwner.get();
    if ((pOwner != nullptr) && (pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
        pOwner->GetComboBox()->SetWindow(nullptr);
        pOwner->GetComboBox()->SetParent(nullptr);
        pRootBox->RemoveAllItems();
    }
    //宿主控件可能已在下拉窗口关闭完成前被销毁，需判空
    if ((pOwner != nullptr) && (pOwner->GetWindow() != nullptr) && pOwner->GetWindow()->IsWindow()) {
        pOwner->SetPos(pOwner->GetPos());
        pOwner->SetFocus();
    }
    BaseClass::OnCloseWindow();
}

LRESULT ComboButtonWnd::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    if (vkCode == kVK_ESCAPE) {
        //按住ESC键，取消
        CloseComboWnd(true);
    }
    else if (vkCode == kVK_RETURN) {
        //按回车键，关闭窗口，正常关闭
        CloseComboWnd(false);
    }
    return lResult;
}

LRESULT ComboButtonWnd::OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKillFocusMsg(pSetFocusWindow, nativeMsg, bHandled);
    //失去焦点，关闭窗口，正常关闭
    if (pSetFocusWindow != this) {
        CloseComboWnd(false);
    }
    return lResult;
}

////////////////////////////////////////////////////////

ComboButton::ComboButton(Window* pWindow) :
    Box(pWindow),
    m_pWindow(nullptr),
    m_bPopupTop(false),
    m_pComboBox(nullptr),
    m_pLeftButton(nullptr),
    m_pLabelTop(nullptr),
    m_pLabelBottom(nullptr),
    m_pRightButton(nullptr),
    m_bDropListShown(false),
    m_nShadowType(ShadowType::kShadowSmallRound)
{
    m_pComboBox = new Box(pWindow);
    SetDropBoxSize({0, 150}, true);
}

ComboButton::~ComboButton()
{
    //宿主析构时先关闭下拉窗口，避免下拉窗口继续持有即将销毁的下拉容器
    if (m_pWindow != nullptr) {
        m_pWindow->CloseComboWndOnOwnerDestroy();
        m_pWindow = nullptr;
    }
    if (!IsInited()) {
        if (m_pLeftButton != nullptr) {
            delete m_pLeftButton;
            m_pLeftButton = nullptr;
        }
        if (m_pLabelTop != nullptr) {
            delete m_pLabelTop;
            m_pLabelTop = nullptr;
        }
        if (m_pLabelBottom != nullptr) {
            delete m_pLabelBottom;
            m_pLabelBottom = nullptr;
        }
        if (m_pRightButton != nullptr) {
            delete m_pRightButton;
            m_pRightButton = nullptr;
        }
    }
    if (m_pComboBox != nullptr) {
        delete m_pComboBox;
        m_pComboBox = nullptr;
    }
}

DString ComboButton::GetType() const { return DUI_CTR_COMBO_BUTTON; }

void ComboButton::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("dropbox_size")) || (strName == _T("dropboxsize")) ) {
        //设置下拉列表的大小（宽度和高度）
        UiSize szDropBoxSize;
        AttributeUtil::ParseSizeValue(strValue.c_str(), szDropBoxSize);
        SetDropBoxSize(szDropBoxSize, true);
    }
    else if ((strName == _T("popup_top")) || (strName == _T("popuptop"))) {
        //下拉列表是否向上弹出
        SetPopupTop(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("combo_box_class")) {
        SetComboBoxClass(strValue);
    }
    else if (strName == _T("left_button_class")) {
        SetLeftButtonClass(strValue);
    }
    else if (strName == _T("left_button_top_label_class")) {
        SetLeftButtonTopLabelClass(strValue);
    }
    else if (strName == _T("left_button_bottom_label_class")) {
        SetLeftButtonBottomLabelClass(strValue);
    }
    else if (strName == _T("left_button_top_label_text")) {
        if (m_pLabelTop != nullptr) {
            m_pLabelTop->SetText(strValue);
        }
    }
    else if (strName == _T("left_button_bottom_label_text")) {
        if (m_pLabelBottom != nullptr) {
            m_pLabelBottom->SetText(strValue);
        }
    }
    else if (strName == _T("left_button_top_label_bkcolor")) {
        if (m_pLabelTop != nullptr) {
            m_pLabelTop->SetBkColor(strValue);
        }
    }
    else if (strName == _T("left_button_bottom_label_bkcolor")) {
        if (m_pLabelBottom != nullptr) {
            m_pLabelBottom->SetBkColor(strValue);
        }
    }
    else if (strName == _T("right_button_class")) {
        SetRightButtonClass(strValue);
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

void ComboButton::SetComboWndShadowType(ShadowType nShadowType)
{
    m_nShadowType = nShadowType;
    if (m_pWindow != nullptr) {
        m_pWindow->SetShadowType(nShadowType);
    }
}

ShadowType ComboButton::GetComboWndShadowType() const
{
    return m_nShadowType;
}

void ComboButton::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }

    UiSize szDropBoxSize = GetDropBoxSize();
    szDropBoxSize = Dpi().GetScaleSize(szDropBoxSize, nOldDpiScale);
    SetDropBoxSize(szDropBoxSize, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void ComboButton::SetLeftButtonClass(const DString& classValue)
{
    if (classValue.empty()) {        
        if (m_pLeftButton != nullptr) {
            RemoveControl(m_pLeftButton);
            m_pLeftButton->SetAutoDestroyChild(false);
        
            delete m_pLeftButton;
            m_pLeftButton = nullptr;
        }
        if (m_pLabelTop != nullptr) {
            delete m_pLabelTop;
            m_pLabelTop = nullptr;
        }
        if (m_pLabelBottom != nullptr) {
            delete m_pLabelBottom;
            m_pLabelBottom = nullptr;
        }
    }
    else {
        if (m_pLeftButton == nullptr) {
            m_pLeftButton = new ButtonVBox(GetWindow());
        }
        SetAttributeList(m_pLeftButton, classValue);
    }
}

void ComboButton::SetLeftButtonTopLabelClass(const DString& classValue)
{
    if (classValue.empty()) {
        if ((m_pLeftButton != nullptr) && (m_pLabelTop != nullptr)) {
            m_pLeftButton->RemoveItem(m_pLabelTop);
        }
        if (m_pLabelTop != nullptr) {
            delete m_pLabelTop;
            m_pLabelTop = nullptr;
        }
    }
    else {
        if (m_pLabelTop == nullptr) {
            m_pLabelTop = new Label(GetWindow());
        }
        SetAttributeList(m_pLabelTop, classValue);
    }
}

void ComboButton::SetLeftButtonBottomLabelClass(const DString& classValue)
{
    if (classValue.empty()) {
        if ((m_pLeftButton != nullptr) && (m_pLabelBottom != nullptr)) {
            m_pLeftButton->RemoveItem(m_pLabelBottom);
        }
        if (m_pLabelBottom != nullptr) {
            delete m_pLabelBottom;
            m_pLabelBottom = nullptr;
        }
    }
    else {
        if (m_pLabelBottom == nullptr) {
            m_pLabelBottom = new Label(GetWindow());
        }
        SetAttributeList(m_pLabelBottom, classValue);
    }
}

void ComboButton::SetRightButtonClass(const DString& classValue)
{
    if (classValue.empty()) {
        RemoveControl(m_pRightButton);
        if (m_pRightButton != nullptr) {
            delete m_pRightButton;
            m_pRightButton = nullptr;
        }
    }
    else {
        if (m_pRightButton == nullptr) {
            m_pRightButton = new Button(GetWindow());
        }
        SetAttributeList(m_pRightButton, classValue);
    }
}

void ComboButton::SetComboBoxClass(const DString& classValue)
{
    ASSERT(m_pComboBox != nullptr);
    ASSERT(!classValue.empty());
    if ((m_pComboBox != nullptr) && !classValue.empty()) {
        SetAttributeList(m_pComboBox, classValue);
    }
}

void ComboButton::SetAttributeList(Control* pControl, const DString& classValue)
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

void ComboButton::RemoveControl(Control* pControl)
{
    if (IsInited() && (GetItemCount() > 0)) {
        HBox* pBox = dynamic_cast<HBox*>(GetItemAt(0));
        if (pBox != nullptr) {
            pBox->RemoveItem(pControl);
        }
    }
}

bool ComboButton::CanPlaceCaptionBar() const
{
    return true;
}

DString ComboButton::GetBorderColor(ControlStateType stateType) const
{
    DString borderColor;
    if (borderColor.empty() && (m_pLeftButton != nullptr)) {
        if (m_pLeftButton->IsFocused() || m_pLeftButton->IsMouseFocused() || m_pLeftButton->IsHoveredState()) {
            borderColor = BaseClass::GetBorderColor(kControlStateHovered);
        }
    }
    if (borderColor.empty() && (m_pRightButton != nullptr)) {
        if (m_pRightButton->IsFocused() || m_pRightButton->IsMouseFocused() || m_pRightButton->IsHoveredState()) {
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

void ComboButton::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    HBox* pBox = new HBox(GetWindow());
    AddItem(pBox);
    pBox->SetNoFocus();

    if (m_pLeftButton == nullptr) {
        if (m_pLabelTop != nullptr) {
            delete m_pLabelTop;
            m_pLabelTop = nullptr;
        }
        if (m_pLabelBottom != nullptr) {
            delete m_pLabelBottom;
            m_pLabelBottom = nullptr;
        }
    }
    else {
        pBox->AddItem(m_pLeftButton);
        m_pLeftButton->SetNoFocus();

        if (m_pLabelTop != nullptr) {
            m_pLeftButton->AddItem(m_pLabelTop);
            m_pLabelTop->SetMouseEnabled(false);
            m_pLabelTop->SetKeyboardEnabled(false);
            m_pLabelTop->SetNoFocus();
        }

        if (m_pLabelBottom != nullptr) {
            m_pLeftButton->AddItem(m_pLabelBottom);
            m_pLabelBottom->SetMouseEnabled(false);
            m_pLabelBottom->SetKeyboardEnabled(false);
            m_pLabelBottom->SetNoFocus();
        }

        m_pLeftButton->AttachClick(UiBind(&ComboButton::OnLeftButtonClicked, this, std::placeholders::_1));
        m_pLeftButton->AttachStateChanged(UiBind(&ComboButton::OnButtonStateChanged, this, std::placeholders::_1));
    }

    if (m_pRightButton != nullptr) {
        pBox->AddItem(m_pRightButton);
        m_pRightButton->SetNoFocus();
        m_pRightButton->AttachButtonDown(UiBind(&ComboButton::OnRightButtonDown, this, std::placeholders::_1));
        m_pRightButton->AttachClick(UiBind(&ComboButton::OnRightButtonClicked, this, std::placeholders::_1));
        m_pRightButton->AttachStateChanged(UiBind(&ComboButton::OnButtonStateChanged, this, std::placeholders::_1));
    }
}

Box* ComboButton::GetComboBox() const
{
    return m_pComboBox;
}

ButtonVBox* ComboButton::GetLeftButtonBox() const
{
    return m_pLeftButton;
}

Label* ComboButton::GetLabelTop() const
{
    return m_pLabelTop;
}

Label* ComboButton::GetLabelBottom() const
{
    return m_pLabelBottom;
}

Button* ComboButton::GetRightButton() const
{
    return m_pRightButton;
}

void ComboButton::UpdateComboWndPos()
{
    if (m_pWindow != nullptr) {
        m_pWindow->UpdateComboWnd();
    }
}

const UiSize& ComboButton::GetDropBoxSize() const
{
    return m_szDropBox;
}

void ComboButton::SetDropBoxSize(UiSize szDropBox, bool bNeedScaleDpi)
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

void ComboButton::OnComboWndClosed(bool bCanceled)
{
    SendEvent(kEventWindowClose, bCanceled ? 1 : 0);
    if (m_pLeftButton != nullptr) {
        m_pLeftButton->Invalidate();
    }
    if (m_pRightButton != nullptr) {
        m_pRightButton->Invalidate();
    }
    Invalidate();
}

bool ComboButton::OnLeftButtonClicked(const EventArgs& /*args*/)
{
    SendEvent(kEventClick);
    Invalidate();
    return true;
}

bool ComboButton::OnRightButtonDown(const EventArgs& /*args*/)
{
    m_bDropListShown = (m_pWindow != nullptr) ? true : false;
    if (m_pWindow != nullptr) {
        //如果下拉框是显示状态，则再次点击的时候，关闭
        HideComboList();
    }
    return true;
}

bool ComboButton::OnRightButtonClicked(const EventArgs& /*args*/)
{
    //如果鼠标按下的时候，正在显示列表，那么点击后不显示下拉列表
    if (!m_bDropListShown) {        
        ShowComboList();
    }
    return true;
}

bool ComboButton::OnWindowKillFocus(const EventArgs& /*args*/)
{
    if (m_pWindow != nullptr) {
        if (m_pWindow->IsWindowFocused()) {
            return true;
        }
    }
    HideComboList();
    return true;
}

bool ComboButton::OnWindowMove(const EventArgs& /*args*/)
{
    UpdateComboList();
    return true;
}

void ComboButton::ShowComboList()
{
    //显示下拉列表
    if ((m_pWindow == nullptr) || m_pWindow->IsClosingWnd()) {
        m_pWindow = new ComboButtonWnd();
        m_pWindow->InitComboWnd(this, true);
    }
}

void ComboButton::HideComboList()
{
    if(m_pWindow != nullptr) {
        m_pWindow->CloseComboWnd(false);
    }
}

void ComboButton::UpdateComboList()
{
    if (m_pWindow != nullptr) {
        m_pWindow->UpdateComboWnd();
    }
}

bool ComboButton::OnButtonStateChanged(const EventArgs& /*args*/)
{
    //刷新当前控件，同步状态
    Invalidate();
    return true;
}

} // namespace ui
