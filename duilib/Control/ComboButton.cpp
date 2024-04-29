#include "ComboButton.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Box/HBox.h"

namespace ui
{

/** 下拉列表窗口
*/
class ComboButtonWnd: public Window
{
public:
    void InitComboWnd(ComboButton* pOwner, bool bActivated);
	void UpdateComboWnd();
    virtual std::wstring GetWindowClassName() const override;
	virtual void OnFinalMessage(HWND hWnd) override;
	virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	/** 关闭下拉框
	* @param [in] bCanceled true表示取消，否则表示正常关闭
	*/
	void CloseComboWnd(bool bCanceled);

	/** 当前是否为焦点窗口
	*/
	bool IsFocusWindow() const;

private:
	//关联的Combo接口
    ComboButton* m_pOwner = nullptr;

	//是否已经关闭
	bool m_bIsClosed = false;
};

void ComboButtonWnd::InitComboWnd(ComboButton* pOwner, bool bActivated)
{
	ASSERT(pOwner != nullptr);
	if (pOwner == nullptr) {
		return;
	}
    m_pOwner = pOwner;
	m_bIsClosed = false;
    CreateWnd(pOwner->GetWindow()->GetHWND(), L"", WS_POPUP, WS_EX_TOOLWINDOW);
	UpdateComboWnd();
	if (bActivated) {
		HWND hWndParent = GetHWND();
		while (::GetParent(hWndParent) != NULL) {
			hWndParent = ::GetParent(hWndParent);
		}
		::ShowWindow(GetHWND(), SW_SHOW);
		::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
		pOwner->SetState(kControlStateHot);
	}
	else {
		::ShowWindow(GetHWND(), SW_SHOWNOACTIVATE);
	}
}

void ComboButtonWnd::UpdateComboWnd()
{
	ComboButton* pOwner = m_pOwner;
	if (pOwner == nullptr) {
		return;
	}
	// Position the popup window in absolute space
	UiSize szDrop = pOwner->GetDropBoxSize();
	UiRect rcOwner = pOwner->GetPos();
	UiPoint scrollBoxOffset = pOwner->GetScrollOffsetInScrollBox();
	rcOwner.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);

	UiRect rc = rcOwner;
	rc.top = rc.bottom + 1;		    // 父窗口left、bottom位置作为弹出窗口起点
	rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
	if (szDrop.cx > 0) {
		rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度
	}

	int32_t cyFixed = 0;
	if (pOwner->GetComboBox()->GetItemCount() > 0) {
		UiSize szAvailable(rc.Width(), rc.Height());
		UiFixedInt oldFixedHeight = pOwner->GetComboBox()->GetFixedHeight();
		pOwner->GetComboBox()->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
		UiEstSize estSize = pOwner->GetComboBox()->EstimateSize(szAvailable);
		pOwner->GetComboBox()->SetFixedHeight(oldFixedHeight, false, false);
		cyFixed = estSize.cy.GetInt32();
	}
	if (cyFixed == 0) {
		cyFixed = szDrop.cy;
	}
	rc.bottom = rc.top + std::min(cyFixed, szDrop.cy);

	pOwner->GetWindow()->MapWindowRect(rc);

	UiRect rcWork;
	GetMonitorWorkRect(rcWork);
	if (rc.bottom > rcWork.bottom || m_pOwner->IsPopupTop()) {
		rc.left = rcOwner.left;
		rc.right = rcOwner.right;
		if (szDrop.cx > 0) {
			rc.right = rc.left + szDrop.cx;
		}
		rc.top = rcOwner.top - std::min(cyFixed, szDrop.cy);
		rc.bottom = rcOwner.top;
		pOwner->GetWindow()->MapWindowRect(rc);
	}
	SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

std::wstring ComboButtonWnd::GetWindowClassName() const
{
    return L"ComboWnd";
}

void ComboButtonWnd::OnFinalMessage(HWND hWnd)
{
	if (m_pOwner != nullptr) {
		if (m_pOwner->GetComboBox()->GetWindow() == this) {
			m_pOwner->GetComboBox()->SetWindow(nullptr);
			m_pOwner->GetComboBox()->SetParent(nullptr);
		}
		if (m_pOwner->m_pWindow == this) {			
			m_pOwner->m_pWindow = nullptr;			
			m_pOwner->Invalidate();
		}
	}
	__super::OnFinalMessage(hWnd);
    delete this;
}

void ComboButtonWnd::CloseComboWnd(bool bCanceled)
{
	if (m_bIsClosed) {
		return;
	}
	m_bIsClosed = true;
	Box* pRootBox = GetRoot();
	if ((pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
		m_pOwner->GetComboBox()->SetWindow(nullptr);
		m_pOwner->GetComboBox()->SetParent(nullptr);
		pRootBox->RemoveAllItems();
	}
	//先将前端窗口切换为父窗口，避免前端窗口关闭后，切换到其他窗口
	HWND hWnd = GetHWND();
	HWND hParentWnd = ::GetParent(hWnd);
	HWND hForeWnd = ::GetForegroundWindow();
	if ((hForeWnd == hWnd) || hForeWnd == hParentWnd) {
		if (hParentWnd != nullptr) {
			::SetForegroundWindow(hParentWnd);
		}
	}
	CloseWnd();
	if (m_pOwner != nullptr) {
		m_pOwner->OnComboWndClosed(bCanceled);
	}
}

bool ComboButtonWnd::IsFocusWindow() const
{
	return ::GetFocus() == GetHWND();
}

LRESULT ComboButtonWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	bHandled = false;
    if( uMsg == WM_CREATE ) {
		InitWnd(GetHWND());
		Box* pRoot = new Box;
		pRoot->SetAutoDestroyChild(false);
		pRoot->AddItem(m_pOwner->GetComboBox());
		AttachBox(pRoot);
		SetResourcePath(m_pOwner->GetWindow()->GetResourcePath());
		SetShadowAttached(false);
		SetRenderTransparent(true);
		bHandled = true;
    }
    else if( uMsg == WM_CLOSE ) {
		Box* pRootBox = GetRoot();
		if ((pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
			m_pOwner->GetComboBox()->SetWindow(nullptr);
			m_pOwner->GetComboBox()->SetParent(nullptr);
			pRootBox->RemoveAllItems();
		}
        m_pOwner->SetPos(m_pOwner->GetPos());
        m_pOwner->SetFocus();
    }
	LRESULT lResult = 0;
	if (!bHandled) {
		lResult = __super::OnWindowMessage(uMsg, wParam, lParam, bHandled);
	}
	if (uMsg == WM_KILLFOCUS) {
		//失去焦点，关闭窗口，正常关闭
		if (GetHWND() != (HWND)wParam) {
			CloseComboWnd(false);
		}
	}
	else if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
		//按住ESC键，取消
		CloseComboWnd(true);
	}
	else if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
		//按回车键，关闭窗口，正常关闭
		CloseComboWnd(false);
	}
	return lResult;
}

////////////////////////////////////////////////////////

ComboButton::ComboButton() :
	m_pWindow(nullptr),
	m_bPopupTop(false),
	m_pComboBox(nullptr),
	m_pLeftButton(nullptr),
	m_pLabelTop(nullptr),
	m_pLabelBottom(nullptr),
	m_pRightButton(nullptr),
	m_bDropListShown(false)
{
	//需要调用设置函数，内部有DPI自适应的逻辑调整大小
	SetDropBoxSize({ 0, 150 }, true);
	m_pComboBox = new Box;
}

ComboButton::~ComboButton()
{
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

std::wstring ComboButton::GetType() const { return DUI_CTR_COMBO_BUTTON; }

void ComboButton::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if ((strName == L"dropbox_size") || (strName == L"dropboxsize") ) {
		//设置下拉列表的大小（宽度和高度）
		UiSize szDropBoxSize;
		AttributeUtil::ParseSizeValue(strValue.c_str(), szDropBoxSize);
		SetDropBoxSize(szDropBoxSize, true);
	}
	else if ((strName == L"popup_top") || (strName == L"popuptop")) {
		//下拉列表是否向上弹出
		SetPopupTop(strValue == L"true");
	}
	else if (strName == L"combo_box_class") {
		SetComboBoxClass(strValue);
	}
	else if (strName == L"left_button_class") {
		SetLeftButtonClass(strValue);
	}
	else if (strName == L"left_button_top_label_class") {
		SetLeftButtonTopLabelClass(strValue);
	}
	else if (strName == L"left_button_bottom_label_class") {
		SetLeftButtonBottomLabelClass(strValue);
	}
	else if (strName == L"left_button_top_label_text") {
		if (m_pLabelTop != nullptr) {
			m_pLabelTop->SetText(strValue);
		}
	}
	else if (strName == L"left_button_bottom_label_text") {
		if (m_pLabelBottom != nullptr) {
			m_pLabelBottom->SetText(strValue);
		}
	}
	else if (strName == L"left_button_top_label_bkcolor") {
		if (m_pLabelTop != nullptr) {
			m_pLabelTop->SetBkColor(strValue);
		}
	}
	else if (strName == L"left_button_bottom_label_bkcolor") {
		if (m_pLabelBottom != nullptr) {
			m_pLabelBottom->SetBkColor(strValue);
		}
	}
	else if (strName == L"right_button_class") {
		SetRightButtonClass(strValue);
	}
	else {
		__super::SetAttribute(strName, strValue);
	}
}

void ComboButton::SetLeftButtonClass(const std::wstring& classValue)
{
	if (classValue.empty()) {
		RemoveControl(m_pLeftButton);
		m_pLeftButton->SetAutoDestroyChild(false);
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
	}
	else {
		if (m_pLeftButton == nullptr) {
			m_pLeftButton = new ButtonVBox;
		}
		SetAttributeList(m_pLeftButton, classValue);
	}
}

void ComboButton::SetLeftButtonTopLabelClass(const std::wstring& classValue)
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
			m_pLabelTop = new Label;
		}
		SetAttributeList(m_pLabelTop, classValue);
	}
}

void ComboButton::SetLeftButtonBottomLabelClass(const std::wstring& classValue)
{
	if (classValue.empty()) {
		if ((m_pLabelBottom != nullptr) && (m_pLabelBottom != nullptr)) {
			m_pLeftButton->RemoveItem(m_pLabelBottom);
		}
		if (m_pLabelBottom != nullptr) {
			delete m_pLabelBottom;
			m_pLabelBottom = nullptr;
		}
	}
	else {
		if (m_pLabelBottom == nullptr) {
			m_pLabelBottom = new Label;
		}
		SetAttributeList(m_pLabelBottom, classValue);
	}
}

void ComboButton::SetRightButtonClass(const std::wstring& classValue)
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
			m_pRightButton = new Button;
		}
		SetAttributeList(m_pRightButton, classValue);
	}
}

void ComboButton::SetComboBoxClass(const std::wstring& classValue)
{
	ASSERT(m_pComboBox != nullptr);
	ASSERT(!classValue.empty());
	if ((m_pComboBox != nullptr) && !classValue.empty()) {
		SetAttributeList(m_pComboBox, classValue);
	}
}

void ComboButton::ParseAttributeList(const std::wstring& strList,
							   std::vector<std::pair<std::wstring, std::wstring>>& attributeList) const
{
	if (strList.empty()) {
		return;
	}
	std::wstring strValue = strList;
	//这个是手工写入的属性，以花括号{}代替双引号，编写的时候就不需要转义字符了；
	StringHelper::ReplaceAll(L"{", L"\"", strValue);
	StringHelper::ReplaceAll(L"}", L"\"", strValue);
	if (strValue.find(L"\"") != std::wstring::npos) {
		AttributeUtil::ParseAttributeList(strValue, L'\"', attributeList);
	}
	else if (strValue.find(L"\'") != std::wstring::npos) {
		AttributeUtil::ParseAttributeList(strValue, L'\'', attributeList);
	}
}

void ComboButton::SetAttributeList(Control* pControl, const std::wstring& classValue)
{
	std::vector<std::pair<std::wstring, std::wstring>> attributeList;
	ParseAttributeList(classValue, attributeList);
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

std::wstring ComboButton::GetBorderColor(ControlStateType stateType) const
{
	std::wstring borderColor;
	if (borderColor.empty() && (m_pLeftButton != nullptr)) {
		if (m_pLeftButton->IsFocused() || m_pLeftButton->IsMouseFocused() || m_pLeftButton->IsHotState()) {
			borderColor = __super::GetBorderColor(kControlStateHot);
		}
	}
	if (borderColor.empty() && (m_pRightButton != nullptr)) {
		if (m_pRightButton->IsFocused() || m_pRightButton->IsMouseFocused() || m_pLeftButton->IsHotState()) {
			borderColor = __super::GetBorderColor(kControlStateHot);
		}
	}
	if (borderColor.empty() && (m_pWindow != nullptr) && !m_pWindow->IsClosingWnd()) {
		borderColor = __super::GetBorderColor(kControlStateHot);
	}
	if (borderColor.empty()) {
		borderColor = __super::GetBorderColor(stateType);
	}
	return borderColor;
}

void ComboButton::OnInit()
{
	if (IsInited()) {
		return;
	}
	__super::OnInit();

	HBox* pBox = new HBox;
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

		m_pLeftButton->AttachClick(nbase::Bind(&ComboButton::OnLeftButtonClicked, this, std::placeholders::_1));
		m_pLeftButton->AttachStateChange(nbase::Bind(&ComboButton::OnButtonStateChanged, this, std::placeholders::_1));
	}

	if (m_pRightButton != nullptr) {
		pBox->AddItem(m_pRightButton);
		m_pRightButton->SetNoFocus();
		m_pRightButton->AttachButtonDown(nbase::Bind(&ComboButton::OnRightButtonDown, this, std::placeholders::_1));
		m_pRightButton->AttachClick(nbase::Bind(&ComboButton::OnRightButtonClicked, this, std::placeholders::_1));
		m_pRightButton->AttachStateChange(nbase::Bind(&ComboButton::OnButtonStateChanged, this, std::placeholders::_1));
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

UiSize ComboButton::GetDropBoxSize() const
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
		GlobalManager::Instance().Dpi().ScaleSize(szDropBox);
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
		if (m_pWindow->IsFocusWindow()) {
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
