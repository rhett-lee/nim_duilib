#include "Menu.h"
#include "duilib/Utils/Macros.h"

namespace ui {

ContextMenuObserver& CMenuWnd::GetMenuObserver()
{
	static ContextMenuObserver s_context_menu_observer;
	return s_context_menu_observer;
}

//二级或者多级子菜单的托管类
class CSubMenuUI: public ui::ListBoxItem
{
};

ui::Control* CMenuWnd::CreateControl(const std::wstring& pstrClass)
{
	if (pstrClass == DUI_CTR_MENUELEMENT){
		return new CMenuElementUI();
	}
	else if (pstrClass == DUI_CTR_SUB_MENU) {
		return new CSubMenuUI();
	}
	return NULL;
}

BOOL CMenuWnd::Receive(ContextMenuParam param)
{
	switch (param.wParam)
	{
	case MenuCloseType::eMenuCloseAll:
		CloseMenu();
		break;
		case MenuCloseType::eMenuCloseThis:
		{
			HWND hParent = GetParent(GetHWND());
			while (hParent != NULL) {
				if (hParent == param.hWnd) {
					CloseMenu();
					break;
				}
				hParent = GetParent(hParent);
			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}

CMenuWnd::CMenuWnd(HWND hParent) :
	m_hParent(hParent),
	m_menuPoint({ 0, 0 }),
	m_popupPosType(MenuPopupPosType::RIGHT_TOP),
	m_noFocus(false),
	m_pOwner(nullptr),
	m_pLayout(nullptr)
{
	m_skinFolder = std::wstring(L"menu");
	m_submenuXml = std::wstring(L"submenu.xml");
	m_submenuNodeName = std::wstring(L"submenu");
}

void CMenuWnd::SetSkinFolder(const std::wstring& skinFolder)
{
	m_skinFolder = skinFolder;
}

void CMenuWnd::SetSubMenuXml(const std::wstring& submenuXml, const std::wstring& submenuNodeName)
{
	m_submenuXml = submenuXml;
	m_submenuNodeName = submenuNodeName;
}

void CMenuWnd::ShowMenu(const std::wstring& xml, const UiPoint& point, MenuPopupPosType popupPosType, bool noFocus, CMenuElementUI* pOwner)
{
	m_menuPoint = point;
	m_popupPosType = popupPosType;

	m_xml = xml;
	m_noFocus = noFocus;
	m_pOwner = pOwner;

	CMenuWnd::GetMenuObserver().AddReceiver(this);

	CreateWnd(m_hParent, L"DUILIB_MENU_WINDOW", WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED);
	// HACK: Don't deselect the parent's caption
	HWND hWndParent = GetHWND();
	while (::GetParent(hWndParent) != NULL) {
		hWndParent = ::GetParent(hWndParent);
	}
	::ShowWindow(GetHWND(), noFocus ? SW_SHOWNOACTIVATE : SW_SHOW);
	if (m_pOwner) {
		ResizeSubMenu();
	}
	else {
		ResizeMenu();
	}
	::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
}

void CMenuWnd::CloseMenu()
{
	//立即关闭，避免连续操作时相互干扰
	PostMsg(WM_CLOSE);
}

void CMenuWnd::DetachOwner()
{
	if (m_pOwner != nullptr) {
		if (m_pLayout != nullptr) {
			m_pLayout->SelectItem(Box::InvalidIndex, false, false);
		}

		//将在OnInitWindow中，添加到Layout上的节点，解除关联关系
		std::vector<Control*> submenuControls;
		CMenuElementUI::GetAllSubMenuControls(m_pOwner, submenuControls);
		for (auto pItem : submenuControls) {
			pItem->SetWindow(nullptr);
			pItem->SetParent(nullptr);
		}

		m_pLayout->RemoveAllItems();
		m_pOwner->m_pSubWindow = nullptr;
		m_pOwner->Invalidate();
		m_pOwner = nullptr;
	}
}

void CMenuWnd::OnFinalMessage(HWND hWnd)
{
	RemoveObserver();
	DetachOwner();
	Window::OnFinalMessage(hWnd);
	delete this;
}

std::wstring CMenuWnd::GetSkinFolder()
{
	return m_skinFolder.c_str();
}

std::wstring CMenuWnd::GetSkinFile() 
{
	return m_xml.c_str();
}

std::wstring CMenuWnd::GetWindowClassName() const
{
	return L"MenuWnd";
}

LRESULT CMenuWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	bHandled = true;
	switch (uMsg)
	{
 		case WM_KILLFOCUS:
 		{
			HWND hFocusWnd = (HWND)wParam;

			BOOL bInMenuWindowList = FALSE;
			ContextMenuParam param;
			param.hWnd = GetHWND();

			ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(GetMenuObserver());
			ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
			while (pReceiver != nullptr) {
				CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
				if ((pContextMenu != nullptr) && (pContextMenu->GetHWND() == hFocusWnd)) {
					bInMenuWindowList = TRUE;
					break;
				}
				pReceiver = iterator.next();
			}

			if (!bInMenuWindowList) {
				param.wParam = MenuCloseType::eMenuCloseAll;
				GetMenuObserver().RBroadcast(param);

				return 0;
			}
		}
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE || wParam == VK_LEFT) {
			CloseMenu();
		}
		else if (wParam == VK_RIGHT) {
			if (m_pLayout) {
				size_t index = m_pLayout->GetCurSel();
				CMenuElementUI* pItem = dynamic_cast<CMenuElementUI*>(m_pLayout->GetItemAt(index));
				if (pItem != nullptr) {
					pItem->CheckSubMenuItem();
				}
			}
		}
		else if (wParam == VK_RETURN || wParam == VK_SPACE)
		{
			if (m_pLayout) {
				size_t index = m_pLayout->GetCurSel();
				CMenuElementUI* pItem = dynamic_cast<CMenuElementUI*>(m_pLayout->GetItemAt(index));
				if (pItem != nullptr) {
					if (!pItem->CheckSubMenuItem()) {
						ContextMenuParam param;
						param.hWnd = GetHWND();
						param.wParam = MenuCloseType::eMenuCloseAll;
						CMenuWnd::GetMenuObserver().RBroadcast(param);
					}
				}
			}
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_CONTEXTMENU:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		return 0L;

	default:
		break;
	}
	return __super::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

void CMenuWnd::ResizeMenu()
{
	ui::Control* pRoot = GetRoot();
	//点击在哪里，以哪里的屏幕为主
	ui::UiRect rcWork;
	GetMonitorWorkRect(m_menuPoint, rcWork);

	ui::UiSize szAvailable = { rcWork.Width(), rcWork.Height()};
	UiEstSize estSize = pRoot->EstimateSize(szAvailable);   //这里带上了阴影窗口
	if (estSize.cx.IsInt32()) {
		szAvailable.cx = estSize.cx.GetInt32();
	}
	if (estSize.cy.IsInt32()) {
		szAvailable.cy = estSize.cy.GetInt32();
	}	
	SetInitSize(szAvailable.cx, szAvailable.cy);
	ui::UiPadding rcCorner = GetShadowCorner();
	ui::UiSize szInit = szAvailable;
	szInit.cx -= rcCorner.left + rcCorner.right;
	szInit.cy -= rcCorner.top + rcCorner.bottom; //这里去掉阴影窗口，即用户的视觉有效面积 szInit<=szAvailable
	
	ui::UiPoint point(m_menuPoint);  //这里有个bug，由于坐标点与包含在窗口内，会直接出发mouseenter导致出来子菜单，偏移1个像素
	if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Right)) {
		point.x += -szAvailable.cx + rcCorner.right + rcCorner.left;
		point.x -= 1;
	}
	else if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Left)) {
		point.x += 1;
	}
	if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Bottom))	{
		point.y += -szAvailable.cy + rcCorner.bottom + rcCorner.top;
		point.y += 1;
	}
	else if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Top)) {
		point.y += 1;
	}
	if (static_cast<int>(m_popupPosType) & static_cast<int>(eMenuAlignment_Intelligent)) {
		if (point.x < rcWork.left) {
			point.x = rcWork.left;
		}
		else if (point.x + szInit.cx> rcWork.right) {
			point.x = rcWork.right - szInit.cx;
		}
		if (point.y < rcWork.top) {
			point.y = rcWork.top ;
		}
		else if (point.y + szInit.cy > rcWork.bottom) {
			point.y = rcWork.bottom - szInit.cy;
		}
	}
	if (!m_noFocus) {
		SetForegroundWindow(GetHWND());
		SetFocus(m_pLayout);
	}
	SetWindowPos(HWND_TOPMOST, point.x - rcCorner.left, point.y - rcCorner.top,
		         szAvailable.cx, szAvailable.cy,
		         SWP_SHOWWINDOW | (m_noFocus ? SWP_NOACTIVATE : 0));
}

void CMenuWnd::ResizeSubMenu()
{
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner == nullptr) {
		return;
	}
	ASSERT(m_pOwner->GetWindow() != nullptr);

	// Position the popup window in absolute space
	UiRect rcOwner = m_pOwner->GetPos();
	UiRect rc = rcOwner;

	int cxFixed = 0;
	int cyFixed = 0;

	UiRect rcWork;
	GetMonitorWorkRect(m_menuPoint, rcWork);
	UiSize szAvailable = { rcWork.Width(), rcWork.Height()};

	const size_t itemCount = m_pLayout->GetItemCount();
	for (size_t it = 0; it < itemCount; ++it) {
		//取子菜单项中的最大值作为菜单项
		CMenuElementUI* pItem = dynamic_cast<CMenuElementUI*>(m_pLayout->GetItemAt(it));
		if (pItem != nullptr) {
			UiEstSize estSize = pItem->EstimateSize(szAvailable);
			UiSize sz(estSize.cx.GetInt32(), estSize.cy.GetInt32());			
			cyFixed += sz.cy;
			if (cxFixed < sz.cx) {
				cxFixed = sz.cx;
			}				
		}
	}
	UiPadding rcCorner = GetShadowCorner();
	UiRect rcWindow;
	m_pOwner->GetWindow()->GetWindowRect(rcWindow);
	//去阴影
	rcWindow.Deflate(rcCorner);

	m_pOwner->GetWindow()->MapWindowRect(rc);
	
	rc.left = rcWindow.right;
	rc.right = rc.left + cxFixed;
	rc.bottom = rc.top + cyFixed;

	bool bReachBottom = false;
	bool bReachRight = false;

	UiRect rcPreWindow;
	ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(GetMenuObserver());
	ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
	while (pReceiver != nullptr) {
		CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
		if (pContextMenu != nullptr) {
			pContextMenu->GetWindowRect(rcPreWindow);  //需要减掉阴影

			bReachRight = (rcPreWindow.left + rcCorner.left) >= rcWindow.right;
			bReachBottom = (rcPreWindow.top + rcCorner.top) >= rcWindow.bottom;
			if (pContextMenu->GetHWND() == m_pOwner->GetWindow()->GetHWND()
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

	if (!m_noFocus) {
		SetForegroundWindow(GetHWND());
		SetFocus(m_pLayout);
	}
	SetWindowPos(HWND_TOPMOST, rc.left - rcCorner.left, rc.top - rcCorner.top,
				 rc.right - rc.left, rc.bottom - rc.top,
				 SWP_SHOWWINDOW | (m_noFocus ? SWP_NOACTIVATE : 0));
}

void CMenuWnd::OnInitWindow()
{
	if (m_pOwner != nullptr) {
		m_pLayout = dynamic_cast<ui::ListBox*>(FindControl(m_submenuNodeName.c_str()));
		ASSERT(m_pLayout);
		if (m_pLayout == nullptr) {
			return;
		}
		//设置不自动销毁Child对象（因为是从owner复制过来的，资源公用，由Owner管理生命对象的周期）
		m_pLayout->SetAutoDestroyChild(false);

		//获取子菜单项需要绘制的控件，并添加到Layout
		std::vector<Control*> submenuControls;
		CMenuElementUI::GetAllSubMenuControls(m_pOwner, submenuControls);
		for (auto pControl : submenuControls) {
			if (pControl != nullptr) {
				m_pLayout->AddItem(pControl); //内部会调用subMenuItem->SetOwner(m_pLayout); 会调用SetWindows，改变了归属窗口、父控件。
				continue;
			}
		}
	}
	else {
		m_pLayout = dynamic_cast<ui::ListBox*>(GetRoot());
		if (m_pLayout == nullptr) {
			//允许外面套层阴影
			if ((GetRoot() != nullptr) && (GetRoot()->GetItemCount() > 0)) {
				m_pLayout = dynamic_cast<ui::ListBox*>(GetRoot()->GetItemAt(0));
			}
		}
		ASSERT(m_pLayout != nullptr);
	}
}

bool CMenuWnd::AddMenuItem(CMenuElementUI* pMenuItem)
{
	//目前只有一级菜单可以访问这个接口
	ASSERT(m_pOwner == nullptr);
	ASSERT(m_pLayout != nullptr);
	if (m_pLayout != nullptr) {
		return m_pLayout->AddItem(pMenuItem);
	}
	return false;
}

bool CMenuWnd::AddMenuItemAt(CMenuElementUI* pMenuItem, size_t iIndex)
{
	//目前只有一级菜单可以访问这个接口
	ASSERT(m_pOwner == nullptr);
	ASSERT(m_pLayout != nullptr);
	
	size_t itemIndex = 0;
	CMenuElementUI* pElementUI = nullptr;
	const size_t count = m_pLayout->GetItemCount();
	for (size_t i = 0; i < count; ++i) {
		Control* pControl = m_pLayout->GetItemAt(i);
		pElementUI = dynamic_cast<CMenuElementUI*>(pControl);
		if (pElementUI != nullptr) {
			if (itemIndex == iIndex) {
				return m_pLayout->AddItemAt(pMenuItem, i);
			}
			++itemIndex;
		}
		pElementUI = nullptr;
	}
	return false;
}

bool CMenuWnd::RemoveMenuItem(CMenuElementUI* pMenuItem)
{
	//目前只有一级菜单可以访问这个接口
	ASSERT(m_pOwner == nullptr);
	ASSERT(m_pLayout != nullptr);
	CMenuElementUI* pElementUI = nullptr;
	if (m_pLayout != nullptr) {
		const size_t count = m_pLayout->GetItemCount();
		for (size_t i = 0; i < count; ++i) {
			pElementUI = dynamic_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i));
			if (pMenuItem == pElementUI) {
				m_pLayout->RemoveItemAt(i);
			}
			pElementUI = nullptr;
		}
	}
	return false;
}

bool CMenuWnd::RemoveMenuItemAt(size_t iIndex)
{
	//目前只有一级菜单可以访问这个接口
	ASSERT(m_pOwner == nullptr);
	CMenuElementUI* pMenuElementUI = GetMenuItemAt(iIndex);
	if (pMenuElementUI != nullptr) {
		return RemoveMenuItem(pMenuElementUI);
	}
	return false;
}

size_t CMenuWnd::GetMenuItemCount() const
{
	//目前只有一级菜单可以访问这个接口
	ASSERT(m_pOwner == nullptr);
	if (m_pLayout == nullptr) {
		return 0;
	}
	size_t itemCount = 0;
	const size_t count = m_pLayout->GetItemCount();
	for (size_t i = 0; i < count; ++i) {
		if (dynamic_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i)) != nullptr) {
			++itemCount;
		}
	}
	return itemCount;
}

CMenuElementUI* CMenuWnd::GetMenuItemAt(size_t iIndex) const
{
	//目前只有一级菜单可以访问这个接口
	ASSERT(m_pOwner == nullptr);
	ASSERT(m_pLayout != nullptr);	
	if (m_pLayout == nullptr) {
		return nullptr;
	}
	size_t itemIndex = 0;
	CMenuElementUI* pElementUI = nullptr;
	const size_t count = m_pLayout->GetItemCount();
	for (size_t i = 0; i < count; ++i) {
		Control* pControl = m_pLayout->GetItemAt(i);
		pElementUI = dynamic_cast<CMenuElementUI*>(pControl);
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

CMenuElementUI* CMenuWnd::GetMenuItemByName(const std::wstring& name) const
{
	//目前只有一级菜单可以访问这个接口
	ASSERT(m_pOwner == nullptr);
	ASSERT(m_pLayout != nullptr);
	CMenuElementUI* pElementUI = nullptr;
	if (m_pLayout != nullptr) {
		const size_t count = m_pLayout->GetItemCount();
		for (size_t i = 0; i < count; ++i) {
			pElementUI = dynamic_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i));
			if ((pElementUI != nullptr) && (pElementUI->IsNameEquals(name))) {
				break;
			}
			pElementUI = nullptr;
		}
	}
	return pElementUI;
}

CMenuElementUI::CMenuElementUI():
	m_pSubWindow(nullptr)
{
	//在菜单元素上，不让子控件响应鼠标消息
	SetMouseChildEnabled(false);
}

void CMenuElementUI::GetAllSubMenuItem(const CMenuElementUI* pParentElementUI,
								       std::vector<CMenuElementUI*>& submenuItems)
{
	submenuItems.clear();
	ASSERT(pParentElementUI != nullptr);
	if (pParentElementUI == nullptr) {
		return;
	}
	const size_t itemCount = pParentElementUI->GetItemCount();
	for (size_t i = 0; i < itemCount; ++i) {
		Control* pControl = pParentElementUI->GetItemAt(i);
		CMenuElementUI* menuElementUI = dynamic_cast<CMenuElementUI*>(pControl);
		if (menuElementUI != nullptr) {
			submenuItems.push_back(menuElementUI);
			continue;
		}

		menuElementUI = nullptr;
		CSubMenuUI* subMenu = dynamic_cast<CSubMenuUI*>(pControl);
		if (subMenu != nullptr) {
			const size_t count = subMenu->GetItemCount();
			for (size_t j = 0; j < count; ++j) {
				menuElementUI = dynamic_cast<CMenuElementUI*>(subMenu->GetItemAt(j));
				if (menuElementUI != nullptr) {
					submenuItems.push_back(menuElementUI);
					continue;
				}
			}
		}
	}
}

void CMenuElementUI::GetAllSubMenuControls(const CMenuElementUI* pParentElementUI,
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
		CMenuElementUI* menuElementUI = dynamic_cast<CMenuElementUI*>(pControl);
		if (menuElementUI != nullptr) {
			submenuControls.push_back(menuElementUI);
			continue;
		}

		CSubMenuUI* subMenu = dynamic_cast<CSubMenuUI*>(pControl);
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

bool CMenuElementUI::AddSubMenuItem(CMenuElementUI* pMenuItem)
{
	return AddItem(pMenuItem);
}

bool CMenuElementUI::AddSubMenuItemAt(CMenuElementUI* pMenuItem, size_t iIndex)
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
		CMenuElementUI* menuElementUI = dynamic_cast<CMenuElementUI*>(pControl);
		if (menuElementUI != nullptr) {
			if (itemIndex == iIndex) {
				//在当前节点下匹配到
				return AddItemAt(pMenuItem, i);
			}
			++itemIndex;
			continue;
		}

		menuElementUI = nullptr;
		CSubMenuUI* subMenu = dynamic_cast<CSubMenuUI*>(pControl);
		if (subMenu != nullptr) {
			const size_t count = subMenu->GetItemCount();
			for (size_t j = 0; j < count; ++j) {
				menuElementUI = dynamic_cast<CMenuElementUI*>(subMenu->GetItemAt(j));
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

bool CMenuElementUI::RemoveSubMenuItem(CMenuElementUI* pMenuItem)
{
	const size_t itemCount = GetItemCount();
	for (size_t i = 0; i < itemCount; ++i) {
		Control* pControl = GetItemAt(i);
		CMenuElementUI* menuElementUI = dynamic_cast<CMenuElementUI*>(pControl);
		if (menuElementUI != nullptr) {
			if (pMenuItem == menuElementUI) {
				//在当前节点下匹配到
				return RemoveItemAt(i);
			}
			continue;
		}

		menuElementUI = nullptr;
		CSubMenuUI* subMenu = dynamic_cast<CSubMenuUI*>(pControl);
		if (subMenu != nullptr) {
			const size_t count = subMenu->GetItemCount();
			for (size_t j = 0; j < count; ++j) {
				menuElementUI = dynamic_cast<CMenuElementUI*>(subMenu->GetItemAt(j));
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
bool CMenuElementUI::RemoveSubMenuItemAt(size_t iIndex)
{
	size_t itemIndex = 0;
	const size_t itemCount = GetItemCount();
	for (size_t i = 0; i < itemCount; ++i) {
		Control* pControl = GetItemAt(i);
		CMenuElementUI* menuElementUI = dynamic_cast<CMenuElementUI*>(pControl);
		if (menuElementUI != nullptr) {
			if (itemIndex == iIndex) {
				//在当前节点下匹配到
				return RemoveItemAt(i);
			}
			++itemIndex;
			continue;
		}

		menuElementUI = nullptr;
		CSubMenuUI* subMenu = dynamic_cast<CSubMenuUI*>(pControl);
		if (subMenu != nullptr) {
			const size_t count = subMenu->GetItemCount();
			for (size_t j = 0; j < count; ++j) {
				menuElementUI = dynamic_cast<CMenuElementUI*>(subMenu->GetItemAt(j));
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

void CMenuElementUI::RemoveAllSubMenuItem()
{
	RemoveAllItems();
}

size_t CMenuElementUI::GetSubMenuItemCount() const
{
	std::vector<CMenuElementUI*> submenuItems;
	GetAllSubMenuItem(this, submenuItems);
	return submenuItems.size();
};

CMenuElementUI* CMenuElementUI::GetSubMenuItemAt(size_t iIndex) const
{
	CMenuElementUI* foundItem = nullptr;
	std::vector<CMenuElementUI*> submenuItems;
	GetAllSubMenuItem(this, submenuItems);
	if (iIndex < submenuItems.size()) {
		foundItem = submenuItems.at(iIndex);
	}
	return foundItem;
}

CMenuElementUI* CMenuElementUI::GetSubMenuItemByName(const std::wstring& name) const
{
	std::vector<CMenuElementUI*> submenuItems;
	GetAllSubMenuItem(this, submenuItems);
	CMenuElementUI* subMenuItem = nullptr;
	for (auto item : submenuItems) {
		if ((item != nullptr) && (item->GetName() == name)) {
			subMenuItem = item;
			break;
		}
	}
	return subMenuItem;
}

bool CMenuElementUI::ButtonUp(const ui::EventArgs& msg)
{
	Window* pWindow = GetWindow();
	ASSERT(pWindow != nullptr);
	if (pWindow == nullptr) {
		return false;
	}
	std::weak_ptr<nbase::WeakFlag> weakFlag = pWindow->GetWeakFlag();
	bool ret = __super::ButtonUp(msg);
	if (ret && !weakFlag.expired()) {
		//这里处理下如果有子菜单则显示子菜单
		if (!CheckSubMenuItem()){
			ContextMenuParam param;
			param.hWnd = pWindow->GetHWND();
			param.wParam = MenuCloseType::eMenuCloseAll;
			CMenuWnd::GetMenuObserver().RBroadcast(param);
		}
	}
	return ret;
}

bool CMenuElementUI::MouseEnter(const ui::EventArgs& msg)
{
	Window* pWindow = GetWindow();
	ASSERT(pWindow != nullptr);
	if (pWindow == nullptr) {
		return __super::MouseEnter(msg);
	}
	std::weak_ptr<nbase::WeakFlag> weakFlag = pWindow->GetWeakFlag();
	bool ret = __super::MouseEnter(msg);
	if (IsHotState() && !weakFlag.expired()) {
		//这里处理下如果有子菜单则显示子菜单
		if (!CheckSubMenuItem()) {
			ContextMenuParam param;
			param.hWnd = pWindow->GetHWND();
			param.wParam = MenuCloseType::eMenuCloseThis;
			CMenuWnd::GetMenuObserver().RBroadcast(param);
			//这里得把之前选中的置为未选中
			if (GetOwner() != nullptr) {
				GetOwner()->SelectItem(Box::InvalidIndex, false, false);
			}
		}
	}
	return ret;
}

void CMenuElementUI::PaintChild(ui::IRender* pRender, const ui::UiRect& rcPaint)
{
	UiRect rcTemp;
	if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
		return;
	}

	for (auto item : m_items) {
		Control* pControl = item;

		//对于多级菜单项的内容，不绘制
		CMenuElementUI* menuElementUI = dynamic_cast<CMenuElementUI*>(pControl);
		if (menuElementUI != nullptr){
			continue;
		}
		CSubMenuUI* subMenu = dynamic_cast<CSubMenuUI*>(pControl);
		if (subMenu != nullptr) {
			continue;
		}
		
		if (!pControl->IsVisible()) {
			continue;
		}
		pControl->AlphaPaint(pRender, rcPaint);
	}
}

bool CMenuElementUI::CheckSubMenuItem()
{
	bool hasSubMenu = false;
	for (auto item : m_items) {
		CMenuElementUI* subMenuItem = dynamic_cast<CMenuElementUI*>(item);
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

void CMenuElementUI::CreateMenuWnd()
{
	ASSERT(m_pSubWindow == nullptr);
	if (m_pSubWindow != nullptr) {
		return;
	}

	HWND hWnd = nullptr;
	Window* pWindow = GetWindow();
	if (pWindow != nullptr) {
		hWnd = pWindow->GetHWND();
	}
	m_pSubWindow = new CMenuWnd(hWnd);
	ContextMenuParam param;
	param.hWnd = hWnd;
	param.wParam = MenuCloseType::eMenuCloseThis;
	CMenuWnd::GetMenuObserver().RBroadcast(param);

	//上级级菜单窗口接口，用于同步配置信息
	CMenuWnd* pParentWindow = dynamic_cast<CMenuWnd*>(pWindow);
	ASSERT(pParentWindow != nullptr);
	if (pParentWindow != nullptr) {
		m_pSubWindow->SetSkinFolder(pParentWindow->m_skinFolder.c_str());
		m_pSubWindow->SetSubMenuXml(pParentWindow->m_submenuXml.c_str(), pParentWindow->m_submenuNodeName.c_str());
		m_pSubWindow->ShowMenu(pParentWindow->m_submenuXml.c_str(), UiPoint(), MenuPopupPosType::RIGHT_BOTTOM, false, this);
	}
}

} // namespace ui
