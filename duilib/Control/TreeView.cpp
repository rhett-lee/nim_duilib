#include "TreeView.h"
#include "duilib/Core/ScrollBar.h"

namespace ui
{

TreeNode::TreeNode() :
	m_bExpand(true),
	m_pTreeView(nullptr),
	m_pParentTreeNode(nullptr),
	m_uDepth(0),
	m_expandCheckBoxPadding(0),
	m_expandIconPadding(0),
	m_expandTextPadding(0),
	m_checkBoxIconPadding(0),
	m_checkBoxTextPadding(0),
	m_iconTextPadding(0),
	m_pExpandImageRect(nullptr),
	m_pCollapseImageRect(nullptr)
{
	m_expandIndent = (uint16_t)ui::GlobalManager::Instance().Dpi().GetScaleInt(4);
	m_checkBoxIndent = (uint16_t)ui::GlobalManager::Instance().Dpi().GetScaleInt(6);
	m_iconIndent = (uint16_t)ui::GlobalManager::Instance().Dpi().GetScaleInt(4);
}

TreeNode::~TreeNode()
{
	if (m_pExpandImageRect != nullptr) {
		delete m_pExpandImageRect;
		m_pExpandImageRect = nullptr;
	}
	if (m_pCollapseImageRect != nullptr) {
		delete m_pCollapseImageRect;
		m_pCollapseImageRect = nullptr;
	}
}

std::wstring TreeNode::GetType() const { return DUI_CTR_TREENODE; }

void TreeNode::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"expand_normal_image") {
		SetExpandStateImage(kControlStateNormal, strValue);
	}
	else if (strName == L"expand_hot_image") {
		SetExpandStateImage(kControlStateHot, strValue);
	}
	else if (strName == L"expand_pushed_image") {
		SetExpandStateImage(kControlStatePushed, strValue);
	}
	else if (strName == L"expand_disabled_image") {
		SetExpandStateImage(kControlStateDisabled, strValue);
	}
	else if (strName == L"collapse_normal_image") {
		SetCollapseStateImage(kControlStateNormal, strValue);
	}
	else if (strName == L"collapse_hot_image") {
		SetCollapseStateImage(kControlStateHot, strValue);
	}
	else if (strName == L"collapse_pushed_image") {
		SetCollapseStateImage(kControlStatePushed, strValue);
	}
	else if (strName == L"collapse_disabled_image") {
		SetCollapseStateImage(kControlStateDisabled, strValue);
	}
	else if (strName == L"expand_image_right_space") {
		int32_t iValue = wcstol(strValue.c_str(), nullptr, 10);
		ui::GlobalManager::Instance().Dpi().ScaleInt(iValue);
		m_expandIndent = ui::TruncateToUInt16(iValue);
	}
	else if (strName == L"check_box_image_right_space") {
		int32_t iValue = wcstol(strValue.c_str(), nullptr, 10);
		ui::GlobalManager::Instance().Dpi().ScaleInt(iValue);
		m_checkBoxIndent = ui::TruncateToUInt16(iValue);
	}
	else if (strName == L"icon_image_right_space") {
		int32_t iValue = wcstol(strValue.c_str(), nullptr, 10);
		ui::GlobalManager::Instance().Dpi().ScaleInt(iValue);
		m_iconIndent = ui::TruncateToUInt16(iValue);
	}
	else {
		__super::SetAttribute(strName, strValue);
	}
}

std::wstring TreeNode::GetExpandStateImage(ControlStateType stateType)
{
	Image* pImage = nullptr;
	if (m_expandImage != nullptr) {
		pImage = m_expandImage->GetStateImage(stateType);
	}
	if (pImage != nullptr) {
		return pImage->GetImageString();
	}
	return std::wstring();
}

void TreeNode::SetExpandStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (m_expandImage == nullptr) {
		m_expandImage.reset(new StateImage);
		m_expandImage->SetControl(this);
	}
	m_expandImage->SetImageString(stateType, strImage);
}

std::wstring TreeNode::GetCollapseStateImage(ControlStateType stateType)
{
	Image* pImage = nullptr;
	if (m_collapseImage != nullptr) {
		pImage = m_collapseImage->GetStateImage(stateType);
	}
	if (pImage != nullptr) {
		return pImage->GetImageString();
	}
	return std::wstring();
}

void TreeNode::SetCollapseStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (m_collapseImage == nullptr) {
		m_collapseImage.reset(new StateImage);
		m_collapseImage->SetControl(this);
	}
	m_collapseImage->SetImageString(stateType, strImage);
}

void TreeNode::PaintStateImages(IRender* pRender)
{
	__super::PaintStateImages(pRender);	
	if (IsExpand()) {
		//绘制展开状态图标，如果没有子节点，不会只这个图标
		if ((m_expandImage != nullptr) && !m_aTreeNodes.empty()){
			if (m_pExpandImageRect == nullptr) {
				m_pExpandImageRect = new UiRect;
			}
			m_expandImage->PaintStateImage(pRender, GetState(), L"", m_pExpandImageRect);
		}
	}
	else {
		//绘制未展开状态图标
		if (m_collapseImage != nullptr) {
			if (m_pCollapseImageRect == nullptr) {
				m_pCollapseImageRect = new UiRect;
			}
			m_collapseImage->PaintStateImage(pRender, GetState(), L"", m_pCollapseImageRect);
		}
	}
}

bool TreeNode::ButtonDown(const EventArgs& msg)
{
	bool bRet = __super::ButtonDown(msg);
	if (!IsEnabled()) {
		return bRet;
	}
	UiRect pos = GetPos();
	UiPoint pt(msg.ptMouse);
	pt.Offset(GetScrollOffsetInScrollBox());
	if (!pos.ContainsPt(pt)) {
		return bRet;
	}
	if (IsExpand()) {
		//展开状态
		if ((m_expandImage != nullptr) && !m_aTreeNodes.empty()) {
			//如果点击在展开图标上，则收起
			if ((m_pExpandImageRect != nullptr) && 
				m_pExpandImageRect->ContainsPt(pt)) {
				SetExpand(false, true);
			}
		}
	}
	else {
		//未展开状态
		if (m_collapseImage != nullptr) {
			//如果点击在展开图标上，则展开
			if ((m_pCollapseImageRect != nullptr) && 
				m_pCollapseImageRect->ContainsPt(pt)) {
				SetExpand(true, true);
			}
		}
	}
	return bRet;
}

int32_t TreeNode::GetExpandImagePadding(void) const
{
	int32_t imageWidth = 0;
	Image* pImage = nullptr;
	if (m_collapseImage != nullptr) {
		pImage = m_collapseImage->GetStateImage(kControlStateNormal);
	}
	if(pImage == nullptr){
		if (m_expandImage != nullptr) {
			pImage = m_expandImage->GetStateImage(kControlStateNormal);
		}
	}
	if (pImage != nullptr) {
		if (pImage->GetImageCache() == nullptr) {
			LoadImageData(*pImage);
		}
		if (pImage->GetImageCache() != nullptr) {
			imageWidth = pImage->GetImageCache()->GetWidth();
		}
	}
	if (imageWidth > 0) {
		imageWidth += m_expandIndent;
	}
	return imageWidth;
}

void TreeNode::SetTreeView(TreeView* pTreeView)
{
    m_pTreeView = pTreeView;
}

TreeView* TreeNode::GetTreeView() const
{
	return m_pTreeView;
}

bool TreeNode::OnDoubleClickItem(const EventArgs& args)
{
    TreeNode* pItem = dynamic_cast<TreeNode*>(args.pSender);
	ASSERT(pItem != nullptr);
	if (pItem != nullptr) {
		pItem->SetExpand(!pItem->IsExpand(), true);
	}
    return true;
}

bool TreeNode::OnNodeCheckStatusChanged(const EventArgs& args)
{
	TreeNode* pItem = dynamic_cast<TreeNode*>(args.pSender);
	ASSERT(pItem != nullptr);
	if ((pItem != nullptr) && (m_pTreeView != nullptr)) {
		m_pTreeView->OnNodeCheckStatusChanged(pItem);
	}
	return true;
}

bool TreeNode::IsVisible() const
{
	if (!ListBoxItem::IsVisible()) {
		return false;
	}
	if (m_pParentTreeNode != nullptr) {
		//如果父节点未展开，或者父节点不可见，则该子节点也不可见
		if (!m_pParentTreeNode->IsExpand() || !m_pParentTreeNode->IsVisible()) {
			return false;
		}
	}
	return true;
}

bool TreeNode::SupportCheckedMode() const
{
	bool bHasStateImages = HasStateImages();
	if (!bHasStateImages || (m_pTreeView == nullptr)) {
		//如果没有状态图片(CheckBox打勾的图片)，返回默认值
		return __super::SupportCheckedMode();
	}
	//多选的时候，支持; 单选的时候，不支持
	return m_pTreeView->IsMultiCheckMode();
}

TreeNode* TreeNode::GetParentNode() const
{
	return m_pParentTreeNode;
}

void TreeNode::SetParentNode(TreeNode* pParentTreeNode)
{
	m_pParentTreeNode = pParentTreeNode;
}

bool TreeNode::AddChildNode(TreeNode* pTreeNode)
{
	return AddChildNodeAt(pTreeNode, GetChildNodeCount());
}

bool TreeNode::AddChildNodeAt(TreeNode* pTreeNode, const size_t iIndex)
{
	ASSERT(pTreeNode != nullptr);
	if (pTreeNode == nullptr) {
		return false;
	}
	ASSERT(m_pTreeView != nullptr);
	if (m_pTreeView == nullptr) {
		return false;
	}
	ASSERT(iIndex <= m_aTreeNodes.size());
	if (iIndex > m_aTreeNodes.size()) {
		return false;
	}
	ASSERT(std::find(m_aTreeNodes.begin(), m_aTreeNodes.end(), pTreeNode) == m_aTreeNodes.end());
	if (std::find(m_aTreeNodes.begin(), m_aTreeNodes.end(), pTreeNode) != m_aTreeNodes.end()) {
		return false;
	}
	
	ASSERT(m_uDepth <= UINT16_MAX);//最大为65535个层级
	/*if (m_uDepth >= UINT16_MAX) {
		return false;
	}*/

	pTreeNode->m_uDepth = m_uDepth + 1;
	pTreeNode->SetParentNode(this);
	pTreeNode->SetTreeView(m_pTreeView);
	pTreeNode->SetWindow(GetWindow());

	//监听双击事件：用于展开子节点
	pTreeNode->AttachEvent(kEventMouseDoubleClick, nbase::Bind(&TreeNode::OnDoubleClickItem, this, std::placeholders::_1));
	
	//监听勾选事件：用于多选时同步勾选子节点和同步父节点的三态选择状态
	pTreeNode->AttachChecked(nbase::Bind(&TreeNode::OnNodeCheckStatusChanged, this, std::placeholders::_1));
	pTreeNode->AttachUnCheck(nbase::Bind(&TreeNode::OnNodeCheckStatusChanged, this, std::placeholders::_1));

	UiPadding padding = GetPadding();
	
	if (m_uDepth != 0) {
		//如果当前不是根节点（根节点的m_uDepth是0），需要添加一层缩进
		padding.left += m_pTreeView->GetIndent();
	}
	pTreeNode->SetPadding(padding, false);

	//[未展开/展开]图片标志
	std::wstring expandImageClass = m_pTreeView->GetExpandImageClass();
	pTreeNode->SetExpandImageClass(expandImageClass);

	//CheckBox选项
	std::wstring checkBoxClass = m_pTreeView->GetCheckBoxClass();
	pTreeNode->SetCheckBoxClass(checkBoxClass);

	//是否显示图标
	pTreeNode->SetEnableIcon(m_pTreeView->IsEnableIcon());

	//添加到ListBox容器中
	size_t nInsertIndex = GetDescendantNodeMaxListBoxIndex();
	if (!Box::IsValidItemIndex(nInsertIndex)) {
		//第一个节点
		nInsertIndex = 0;
	}
	else {
		//不是第一个节点时，插入位置需要放在所有子孙节点的后面
		nInsertIndex += 1;
	}
	ASSERT(nInsertIndex <= m_pTreeView->ListBox::GetItemCount());
	m_aTreeNodes.insert(m_aTreeNodes.begin() + iIndex, pTreeNode);
	bool bAdded = m_pTreeView->ListBox::AddItemAt(pTreeNode, nInsertIndex);
	if (bAdded) {
		if (SupportCheckedMode()) {
			//新添加的节点状态，跟随父节点
			pTreeNode->SetChecked(IsChecked());
			//更新节点的勾选状态
			UpdateSelfCheckStatus();
			UpdateParentCheckStatus(false);
		}
	}
	else {
		//添加失败的话，移除
		auto iter = std::find(m_aTreeNodes.begin(), m_aTreeNodes.end(), pTreeNode);
		if (iter != m_aTreeNodes.end()) {
			m_aTreeNodes.erase(iter);
		}
	}
	return bAdded;
}

#ifdef UILIB_IMPL_WINSDK

void TreeNode::SetBkIcon(HICON hIcon)
{
	GlobalManager::Instance().Icon().AddIcon(hIcon);
	std::wstring iconString = GlobalManager::Instance().Icon().GetIconString(hIcon);
	std::wstring oldIconString = GetBkImagePath();
	if (iconString == oldIconString) {
		//没有变化，直接返回
		return;
	}
	//旧的图标大小
	HICON hOldIcon = GlobalManager::Instance().Icon().GetIcon(oldIconString);
	UiSize oldIconSize = GlobalManager::Instance().Icon().GetIconSize(hOldIcon);

	//新的图标大小
	UiSize newIconSize = GlobalManager::Instance().Icon().GetIconSize(hIcon);

	bool bAdjustIconPadding = true;
	if (!oldIconString.empty()) {
		if (newIconSize.cx != oldIconSize.cx) {
			//旧图标存在，并且图标大小不同，首先清除原来的图标
			SetBkImage(L"");
			AdjustIconPadding();
		}
		else {
			//新旧图标大小相同，不需要调整图标内边距
			bAdjustIconPadding = false;
		}
	}

	if (!iconString.empty()) {
		iconString = StringHelper::Printf(L"file='%s' halign='left' valign='center'", iconString.c_str());
		SetBkImage(iconString);
	}
	else {
		SetBkImage(L"");
	}

	if (bAdjustIconPadding) {
		AdjustIconPadding();
	}

	//按需修改是否显示图标标志
	if (m_pTreeView != nullptr) {
		SetEnableIcon(m_pTreeView->IsEnableIcon());
	}
}

#endif //UILIB_IMPL_WINSDK

void TreeNode::SetExpandImageClass(const std::wstring& expandClass)
{
	if (!expandClass.empty()) {
		//开启展开标志功能
		SetClass(expandClass);
	}
	else {
		//关闭展开标志功能
		m_expandImage.reset();
		m_collapseImage.reset();
		if (m_pExpandImageRect != nullptr) {
			delete m_pExpandImageRect;
			m_pExpandImageRect = nullptr;
		}
		if (m_pCollapseImageRect != nullptr) {
			delete m_pCollapseImageRect;
			m_pCollapseImageRect = nullptr;
		}
	}
	AdjustExpandImagePadding();
}

bool TreeNode::SetCheckBoxClass(const std::wstring& checkBoxClass)
{
	bool bSetOk = true;
	if (!checkBoxClass.empty()) {
		//开启CheckBox功能
		SetClass(checkBoxClass);
		if (!HasStateImage(kStateImageBk) && !HasStateImage(kStateImageSelectedBk)) {
			ASSERT(!"TreeNode::SetCheckBoxClass failed!");
			bSetOk = false;
		}
	}
	else {
		//关闭CheckBox功能
		ClearStateImages();
	}
	AdjustCheckBoxPadding();
	return bSetOk;
}

void TreeNode::AdjustExpandImagePadding()
{
	uint16_t expandPadding = ui::TruncateToUInt8(GetExpandImagePadding());
	if (expandPadding != 0) {
		//显示[展开/收起]标志
		if (m_expandCheckBoxPadding == 0) {
			int32_t leftOffset = (int32_t)expandPadding;
			if (AdjustStateImagesPaddingLeft(leftOffset, false)) {
				m_expandCheckBoxPadding = expandPadding;
			}
		}
		else if (!HasStateImage(kStateImageBk)) {
			//CheckBox图标已经隐藏
			m_expandCheckBoxPadding = 0;
		}

		if (m_expandIconPadding == 0) {
			//有CheckBox状态图片, 需要设置背景图片的内边距，避免两个图片重叠
			UiPadding rcBkPadding = GetBkImagePadding();
			rcBkPadding.left += expandPadding;
			if (SetBkImagePadding(rcBkPadding, false)) {
				m_expandIconPadding = expandPadding;
			}
		}

		if (m_expandTextPadding == 0) {
			//设置文字的内边距
			UiPadding rcTextPadding = GetTextPadding();
			rcTextPadding.left += expandPadding;
			SetTextPadding(rcTextPadding, false);
			m_expandTextPadding = expandPadding;
		}
	}
	else {
		//不显示[展开/收起]标志
		if (m_expandCheckBoxPadding > 0) {
			int32_t leftOffset = -(int32_t)m_expandCheckBoxPadding;
			AdjustStateImagesPaddingLeft(leftOffset, false);
			m_expandCheckBoxPadding = 0;
		}

		if (m_expandIconPadding > 0) {
			UiPadding rcBkPadding = GetBkImagePadding();
			rcBkPadding.left -= (int32_t)m_expandIconPadding;
			if (rcBkPadding.left >= 0) {
				SetBkImagePadding(rcBkPadding, false);
			}
			m_expandIconPadding = 0;
		}
		if (m_expandTextPadding > 0) {
			UiPadding rcTextPadding = GetTextPadding();
			rcTextPadding.left -= (int32_t)m_expandTextPadding;
			if (rcTextPadding.left >= 0) {
				SetTextPadding(rcTextPadding, false);
			}
			m_expandTextPadding = 0;
		}		
	}
	Invalidate();
}

void TreeNode::AdjustCheckBoxPadding()
{
	if (HasStateImage(kStateImageBk)) {
		//显示CheckBox
		uint16_t extraPadding = m_checkBoxIndent;
		UiSize imageSize = GetStateImageSize(kStateImageBk, kControlStateNormal);
		uint16_t checkBoxPadding = TruncateToUInt16(imageSize.cx);
		if (checkBoxPadding > 0) {
			checkBoxPadding += extraPadding;
		}
		
		if ((checkBoxPadding > 0) && (m_checkBoxIconPadding == 0)){
			//有CheckBox状态图片, 需要设置背景图片的内边距，避免两个图片重叠
			UiPadding rcBkPadding = GetBkImagePadding();
			rcBkPadding.left += checkBoxPadding;
			if (SetBkImagePadding(rcBkPadding, false)) {
				m_checkBoxIconPadding = checkBoxPadding;
			}			
		}

		if ((checkBoxPadding > 0) && (m_checkBoxTextPadding == 0)) {
			//设置文字的内边距
			UiPadding rcTextPadding = GetTextPadding();
			rcTextPadding.left += checkBoxPadding;
			SetTextPadding(rcTextPadding, false);
			m_checkBoxTextPadding = checkBoxPadding;
		}
	}
	else {
		//隐藏CheckBox
		if (m_checkBoxIconPadding > 0) {
			UiPadding rcBkPadding = GetBkImagePadding();
			rcBkPadding.left -= (int32_t)m_checkBoxIconPadding;
			if (rcBkPadding.left >= 0) {
				SetBkImagePadding(rcBkPadding, false);
			}
			m_checkBoxIconPadding = 0;
		}
		if (m_checkBoxTextPadding > 0) {
			UiPadding rcTextPadding = GetTextPadding();
			rcTextPadding.left -= (int32_t)m_checkBoxTextPadding;
			if (rcTextPadding.left >= 0) {
				SetTextPadding(rcTextPadding, false);
			}
			m_checkBoxTextPadding = 0;
		}
	}
	AdjustExpandImagePadding();
}

void TreeNode::AdjustIconPadding()
{
	std::wstring iconString = GetBkImagePath();
	if (!IsBkImagePaintEnabled()) {
		//禁止绘制图标
		iconString.clear();
	}
	if (!iconString.empty()) {
		//显示图标
		if (m_iconTextPadding == 0) {
			const uint16_t extraPadding = m_iconIndent;
			UiSize imageSize = GetBkImageSize();
			uint16_t iconTextPadding = TruncateToUInt16(imageSize.cx);
			if (iconTextPadding > 0) {
				iconTextPadding += extraPadding;
			}
			if (iconTextPadding > 0) {
				//设置文字的内边距
				UiPadding rcTextPadding = GetTextPadding();
				rcTextPadding.left += iconTextPadding;
				SetTextPadding(rcTextPadding, false);
				m_iconTextPadding = iconTextPadding;
			}
		}
	}
	else {
		//隐藏图标
		if (m_iconTextPadding > 0) {
			UiPadding rcTextPadding = GetTextPadding();
			rcTextPadding.left -= (int32_t)m_iconTextPadding;
			if (rcTextPadding.left >= 0) {
				SetTextPadding(rcTextPadding, false);
			}
			m_iconTextPadding = 0;
		}
	}
	AdjustCheckBoxPadding();
}

void TreeNode::SetEnableIcon(bool bEnable)
{
	if (IsBkImagePaintEnabled() != bEnable) {
		SetBkImagePaintEnabled(bEnable);
		AdjustIconPadding();
	}
}

void TreeNode::SetChildrenCheckStatus(bool bChecked)
{
	if (!SupportCheckedMode()) {
		//单选或者不显示CheckBox：忽略
		return;
	}
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode) {
			pTreeNode->SetChecked(bChecked, false);
			pTreeNode->SetChildrenCheckStatus(bChecked);
		}
	}
}

void TreeNode::UpdateParentCheckStatus(bool bUpdateSelf)
{
	if (!SupportCheckedMode()) {
		//单选或者不显示CheckBox：忽略
		return;
	}
	if (bUpdateSelf) {
		UpdateSelfCheckStatus();
	}
	if (m_pParentTreeNode != nullptr) {
		m_pParentTreeNode->UpdateParentCheckStatus(true);
	}
}

void TreeNode::UpdateSelfCheckStatus()
{
	if (!SupportCheckedMode()) {
		//单选或者不显示CheckBox：忽略
		return;
	}
	bool bChecked = IsChecked();
	TreeNodeCheck nodeCheck = GetChildrenCheckStatus();//根据子节点的选择状态，修改当前节点的选择状态
	if (nodeCheck == TreeNodeCheck::UnCheck) {
		if (!bChecked) {
			return;
		}
		else {
			//更新为：TreeNodeCheck::UnCheck
			SetChecked(false);
			SetPartSelected(false);
			Invalidate();
		}
	}
	else if (nodeCheck == TreeNodeCheck::CheckedAll) {
		//更新为：TreeNodeCheck::CheckedAll
		if (bChecked) {
			if (IsPartSelected()) {
				SetPartSelected(false);
				Invalidate();
			}
		}
		else {			
			SetChecked(true);
			SetPartSelected(false);
			Invalidate();
		}
	}
	else if (nodeCheck == TreeNodeCheck::CheckedPart) {
		//更新为：TreeNodeCheck::CheckedPart
		SetChecked(true);
		SetPartSelected(true);
		Invalidate();
	}
}

TreeNodeCheck TreeNode::GetCheckStatus(void) const
{
	if (!SupportCheckedMode()) {
		//单选或者不显示CheckBox：只按当前节点状态判断结果
		return IsSelected() ? TreeNodeCheck::CheckedAll : TreeNodeCheck::UnCheck;
	}

	//多选
	bool bChecked = IsChecked();
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode == nullptr) {
			continue;
		}
		if (bChecked != pTreeNode->IsChecked()) {
			return TreeNodeCheck::CheckedPart;
		}
	}

	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode == nullptr) {
			continue;
		}
		TreeNodeCheck childCheck = pTreeNode->GetCheckStatus();
		if (bChecked) {
			if (childCheck == TreeNodeCheck::UnCheck) {
				return TreeNodeCheck::CheckedPart;
			}
			else if (childCheck == TreeNodeCheck::CheckedPart) {
				return TreeNodeCheck::CheckedPart;
			}
		}
		else {
			if (childCheck == TreeNodeCheck::CheckedAll) {
				return TreeNodeCheck::CheckedPart;
			}
			else if (childCheck == TreeNodeCheck::CheckedPart) {
				return TreeNodeCheck::CheckedPart;
			}
		}
	}
	return bChecked ? TreeNodeCheck::CheckedAll : TreeNodeCheck::UnCheck;
}

TreeNodeCheck TreeNode::GetChildrenCheckStatus(void) const
{
	if (!SupportCheckedMode()) {
		//单选或者不显示CheckBox：只按当前节点状态判断结果
		return IsSelected() ? TreeNodeCheck::CheckedAll : TreeNodeCheck::UnCheck;
	}

	if (m_aTreeNodes.empty()) {
		//没有子节点：返回当前节点的状态
		return IsChecked() ? TreeNodeCheck::CheckedAll : TreeNodeCheck::UnCheck;
	}
	//多选: 先扫描一级子节点
	bool bLastChecked = false;
	bool bSetLastChecked = false;
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode == nullptr) {
			continue;
		}
		bool bChildChecked = pTreeNode->IsChecked();
		if (!bSetLastChecked) {
			bLastChecked = bChildChecked;
			bSetLastChecked = true;
		}
		else {
			if (bLastChecked != bChildChecked) {
				return TreeNodeCheck::CheckedPart;
			}
		}		
	}

	//再扫描多级子节点
	TreeNodeCheck lastChildCheck = TreeNodeCheck::UnCheck;
	bool bSetLastChildCheck = false;
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode == nullptr){
			continue;
		}
		TreeNodeCheck childSelect = pTreeNode->GetCheckStatus();//获取包含自身/子节点的勾选状态
		if (childSelect == TreeNodeCheck::CheckedPart) {
			return TreeNodeCheck::CheckedPart;
		}
		if (!bSetLastChildCheck) {
			lastChildCheck = childSelect;
			bSetLastChildCheck = true;
		}
		else {
			if (childSelect != lastChildCheck) {
				return TreeNodeCheck::CheckedPart;
			}
		}
	}

	//如果状态都一样，返回第一个子节点的状态
	TreeNode* pTreeNode = m_aTreeNodes.front();
	if (pTreeNode != nullptr) {
		return pTreeNode->IsChecked() ? TreeNodeCheck::CheckedAll : TreeNodeCheck::UnCheck;
	}
	else {
		return IsChecked() ? TreeNodeCheck::CheckedAll : TreeNodeCheck::UnCheck;
	}
}

bool TreeNode::RemoveChildNodeAt(size_t iIndex, bool bUpdateCheckStatus)
{
	ASSERT(iIndex < m_aTreeNodes.size());
	if (iIndex >= m_aTreeNodes.size()) {
		return false;
	}

	bool bRemoved = false;
	TreeNode* pTreeNode = ((TreeNode*)m_aTreeNodes[iIndex]);
	m_aTreeNodes.erase(m_aTreeNodes.begin() + iIndex);
	if (pTreeNode != nullptr) {
		bRemoved = pTreeNode->RemoveSelf();
	}
	if (bUpdateCheckStatus && SupportCheckedMode()) {
		//更新节点的勾选状态
		UpdateSelfCheckStatus();
		UpdateParentCheckStatus(false);
	}
	return bRemoved;
}

bool TreeNode::RemoveChildNodeAt(size_t iIndex)
{
	return RemoveChildNodeAt(iIndex, true);
}

bool TreeNode::RemoveChildNode(TreeNode* pTreeNode)
{
	auto it = std::find(m_aTreeNodes.begin(), m_aTreeNodes.end(), pTreeNode);
	if (it == m_aTreeNodes.end()) {
		return false;
	}		
	size_t iIndex = it - m_aTreeNodes.begin();
	return RemoveChildNodeAt(iIndex, true);
}
	
void TreeNode::RemoveAllChildNodes()
{
	while (m_aTreeNodes.size() > 0) {
		RemoveChildNodeAt(0, false);
	}
}

bool TreeNode::RemoveSelf()
{
	for(TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode != nullptr) {
			pTreeNode->RemoveSelf();
		}
	}
	m_aTreeNodes.clear();

	//从ListBox中移除元素
	bool bRemoved = false;
	size_t nListBoxIndex = GetListBoxIndex();
	if (Box::IsValidItemIndex(nListBoxIndex)) {
		ASSERT(m_pTreeView->ListBox::GetItemAt(nListBoxIndex) == this);
		bRemoved = m_pTreeView->ListBox::RemoveItemAt(nListBoxIndex);
	}
	return bRemoved;
}

size_t TreeNode::GetDescendantNodeCount() const
{
	size_t nodeCount = GetChildNodeCount();
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode != nullptr) {
			nodeCount += pTreeNode->GetDescendantNodeCount();
		}
	}
	return nodeCount;
}

size_t TreeNode::GetChildNodeCount() const
{
	return m_aTreeNodes.size();
}

size_t TreeNode::GetDescendantNodeMaxListBoxIndex() const
{
	size_t maxListBoxIndex = GetListBoxIndex();
	if (!Box::IsValidItemIndex(maxListBoxIndex)) {
		if (m_aTreeNodes.empty()) {
			return maxListBoxIndex;
		}
		maxListBoxIndex = 0;
	}
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode != nullptr) {
			maxListBoxIndex = std::max(pTreeNode->GetDescendantNodeMaxListBoxIndex(), maxListBoxIndex);
		}
	}
	return maxListBoxIndex;
}
	
TreeNode* TreeNode::GetChildNode(size_t iIndex) const
{
	if (iIndex >= m_aTreeNodes.size()) {
		return nullptr;
	}
	return m_aTreeNodes[iIndex];
}
	
size_t TreeNode::GetChildNodeIndex(TreeNode* pTreeNode) const
{
	auto it = std::find(m_aTreeNodes.begin(), m_aTreeNodes.end(), pTreeNode);
	if (it == m_aTreeNodes.end()) {
		return Box::InvalidIndex;
	}
	return it - m_aTreeNodes.begin();
}

bool TreeNode::IsExpand() const
{
	return m_bExpand;
}

void TreeNode::SetExpand(bool bExpand, bool bTriggerEvent)
{
	if(m_bExpand == bExpand) {
		return;
	}
	m_bExpand = bExpand;

	if (bTriggerEvent) {
		SendEvent(m_bExpand ? kEventExpand : kEventCollapse);
	}
	if (m_pTreeView != nullptr) {
		m_pTreeView->Arrange();
	}	
}

uint16_t TreeNode::GetDepth() const
{
	return m_uDepth;
}

TreeView::TreeView() :
	ListBox(new VLayout),
	m_iIndent(0),
	m_rootNode(),
	m_bEnableIcon(true)
{
	m_bMultiCheckMode = __super::IsMultiSelect();
	m_rootNode.reset(new TreeNode());
	m_rootNode->SetTreeView(this);
	//缩进默认设置为20个像素
	SetIndent(20, true);
}

TreeView::~TreeView()
{
	m_rootNode->RemoveSelf();
	m_rootNode.reset();
}

std::wstring TreeView::GetType() const { return DUI_CTR_TREEVIEW; }

void TreeView::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	//支持的属性列表: 基类实现的直接转发
	if (strName == L"indent") {
		//树节点的缩进（每层节点缩进一个indent单位）
		SetIndent(_wtoi(strValue.c_str()), true);
	}
	else if (strName == L"multi_select") {
		//多选，默认是单选，在基类实现
		SetMultiSelect(strValue == L"true");
	}
	else if (strName == L"check_box_class") {
		//是否显示CheckBox
		SetCheckBoxClass(strValue);
	}
	else if (strName == L"expand_image_class") {
		//是否显示[展开/收起]图标
		SetExpandImageClass(strValue);
	}
	else if (strName == L"show_icon") {
		//是否显示图标
		SetEnableIcon(strValue == L"true");
	}
	else {
		ListBox::SetAttribute(strName, strValue);
	}
}

bool TreeView::IsMultiSelect() const
{
	if (!m_checkBoxClass.empty()) {
		//如果显示CheckBox，以Check模式为准, 对于树的选择状态，按单选处理
		if (IsMultiCheckMode()) {
			return false;
		}		
	}
	return __super::IsMultiSelect();
}

void TreeView::SetMultiSelect(bool bMultiSelect)
{
	bool bSelectChanged = (bMultiSelect != __super::IsMultiSelect()) ||
						  (m_bMultiCheckMode != bMultiSelect);
	if (!bSelectChanged) {
		return;
	}
	bool bOldCheckMode = IsMultiCheckMode();
	m_bMultiCheckMode = bMultiSelect;
	__super::SetMultiSelect(bMultiSelect);

	bool isChanged = false;
	if (IsMultiCheckMode()) {
		//切换到：Check模式（即CheckBox显示的情况）
		if (OnCheckBoxShown()) {
			isChanged = true;
		}
	}
	else if (IsMultiSelect()) {
		//切换到多选模式（即CheckBox被隐藏的情况）
		if (OnCheckBoxHided()) {
			isChanged = true;
		}
	}
	else {
		//切换到单选模式
		if (bOldCheckMode) {
			//从Check模式切换到单选：需要先同步当前选择项，避免出现切换后选择项不一致问题（没有勾选的，变成了当前选择项）
			if (UpdateCurSelItemCheckStatus()) {
				isChanged = true;
			}
		}
		//切换为单选模式：确保ListBox里面的数据是单选的
		if (OnSwitchToSingleSelect()) {
			isChanged = true;
		}
	}
	if (isChanged) {
		Invalidate();
	}
}

bool TreeView::IsMultiCheckMode() const
{
	if (!m_checkBoxClass.empty()) {
		ASSERT(m_bMultiCheckMode == __super::IsMultiSelect());
		return m_bMultiCheckMode;
	}
	return false;
}

bool TreeView::CanPaintSelectedColors(bool bHasStateImages) const
{
	if (bHasStateImages && IsMultiCheckMode()) {
		//如果有CheckBox，Check模式的时候，默认不显示选择背景色
		return false;
	}
	return __super::CanPaintSelectedColors(bHasStateImages);
}

bool TreeView::OnSwitchToSingleSelect()
{
	ASSERT(!IsMultiSelect());
	bool bChanged = __super::OnSwitchToSingleSelect();
	if (IsMultiCheckMode()) {
		return bChanged;
	}
	//已经切换为单选
	TreeNode* pItem = nullptr;
	const size_t itemCount = m_items.size();
	for (size_t i = 0; i < itemCount; ++i) {
		pItem = dynamic_cast<TreeNode*>(m_items[i]);
		if ((pItem != nullptr) && pItem->IsChecked()) {
			if (GetCurSel() != i) {
				//改为单选后，如果不是当前选择项，Checked标志全部改为false
				pItem->SetChecked(false, false);
				pItem->Invalidate();
				bChanged = true;
			}
		}
	}
	return bChanged;
}

bool TreeView::UpdateCurSelItemCheckStatus()
{
	//以Check勾选项为准，设置当前选择项
	bool bChanged = false;
	size_t curSelIndex = GetCurSel();
	if (Box::IsValidItemIndex(curSelIndex)) {
		TreeNode* pItem = dynamic_cast<TreeNode*>(GetItemAt(curSelIndex));
		if (pItem != nullptr) {
			if (!pItem->IsChecked()) {
				SetCurSel(Box::InvalidIndex);
				pItem->SetSelected(false);
				pItem->Invalidate();
				bChanged = true;
			}
			else {
				pItem->SetSelected(true);
				pItem->Invalidate();
			}
		}
	}
	return bChanged;
}

bool TreeView::OnCheckBoxHided()
{
	ASSERT(IsMultiSelect() && !IsMultiCheckMode());
	//同步方向: Check -> Select
	if (m_items.empty()) {
		return false;
	}
	bool isChaned = false;
	TreeNode* pItem = nullptr;
	const size_t itemCount = m_items.size();
	for (size_t i = 0; i < itemCount; ++i) {
		pItem = dynamic_cast<TreeNode*>(m_items[i]);
		if (pItem == nullptr) {
			continue;
		}
		//同步Check和Select标志
		if (pItem->IsSelected() != pItem->IsChecked()) {
			pItem->SetSelected(pItem->IsChecked());
			pItem->Invalidate();
			isChaned = true;
		}
		if (pItem->IsChecked()) {
			//Check全部改为false
			pItem->SetChecked(false);
			pItem->Invalidate();
			isChaned = true;
		}		
	}
	//同步当前选择项
	if (UpdateCurSelItemSelectStatus()) {
		isChaned = true;
	}
	return isChaned;
}

bool TreeView::OnCheckBoxShown()
{
	ASSERT(IsMultiCheckMode());
	//同步方向: Select -> Check
	if (m_items.empty()) {
		return false;
	}
	bool isChanged = false;
	const size_t curSelIndex = GetCurSel();
	TreeNode* pItem = nullptr;
	const size_t itemCount = m_items.size();
	for (size_t i = 0; i < itemCount; ++i) {
		pItem = dynamic_cast<TreeNode*>(m_items[i]);
		if (pItem == nullptr) {
			continue;
		}
		//同步Select和Check标志
		if (pItem->IsChecked() != pItem->IsSelected()) {
			pItem->SetChecked(pItem->IsSelected());
			pItem->Invalidate();
			isChanged = true;
		}
		if (curSelIndex != i) {
			//把选择项的状态去掉，因为Check模式下，ListBox的行为是单选行为
			if (pItem->IsSelected()) {
				pItem->SetSelected(false);
				pItem->Invalidate();
				isChanged = true;
			}
		}
	}
	//同步当前选择项	
	if (Box::IsValidItemIndex(curSelIndex)) {
		bool bSelectItem = false;
		pItem = dynamic_cast<TreeNode*>(GetItemAt(curSelIndex));
		if (pItem != nullptr) {
			bSelectItem = pItem->IsSelected();
		}
		if (!bSelectItem) {
			SetCurSel(Box::InvalidIndex);
			isChanged = true;
		}
		else if(pItem->IsChecked()){
			//同步勾选状态
			pItem->UpdateParentCheckStatus(true);
		}
	}
	return isChanged;
}

void TreeView::SetIndent(int32_t indent, bool bNeedDpiScale)
{
	ASSERT(indent >= 0);
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(indent);
	}
	if (indent >= 0) {
		m_iIndent = indent;
	}	
}

void TreeView::SetCheckBoxClass(const std::wstring& className)
{
	if (m_checkBoxClass == className) {
		return;
	}
	std::wstring oldCheckBoxClass = m_checkBoxClass.c_str();
	m_checkBoxClass = className;
	bool bSetOk = true;
	bool hasSetOk = false;
	for (Control* pControl : m_items) {
		TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
		if (pTreeNode != nullptr) {
			if (!pTreeNode->SetCheckBoxClass(className)) {
				bSetOk = false;
			}
			else {
				hasSetOk = true;
			}
		}
	}

	if (!bSetOk && !hasSetOk) {
		//无效的设置
		ASSERT(!"TreeView::SetCheckBoxClass failed!");
		m_checkBoxClass.clear();
		if (oldCheckBoxClass == m_checkBoxClass) {
			return;
		}
	}

	bool isChanged = false;
	if (m_checkBoxClass.empty()) {
		//从显示CheckBox到隐藏CheckBox：有两种模式需要处理（多选模式、单选模式）
		//需要先同步当前选择项，避免出现切换后选择项不一致问题（没有勾选的，变成了当前选择项）
		if (UpdateCurSelItemCheckStatus()) {
			isChanged = true;
		}
		if (IsMultiSelect()) {
			//切换为多选模式
			if (OnCheckBoxHided()) {
				isChanged = true;
			}
		}
		else {
			//切换为单选模式：确保ListBox里面的数据是单选的
			if (OnSwitchToSingleSelect()) {
				isChanged = true;
			}
		}
	}
	else {
		//从隐藏CheckBox切换为显示CheckBox：有两种模式需要处理（Check模式、单选模式）
		if (IsMultiCheckMode()) {
			//Check模式（即CheckBox显示的情况）
			isChanged = OnCheckBoxShown();
		}
		else if(!IsMultiSelect()) {
			//单选模式：确保ListBox里面的数据是单选的
			if (OnSwitchToSingleSelect()) {
				isChanged = true;
			}
		}
		else {
			//不存在这个情况
			ASSERT(!"ERROR!");
		}
	}
	if (isChanged) {
		Invalidate();
	}
}

std::wstring TreeView::GetCheckBoxClass() const
{
	return m_checkBoxClass.c_str();
}

void TreeView::SetExpandImageClass(const std::wstring& className)
{
	bool isChanged = m_expandImageClass != className;
	m_expandImageClass = className;
	if (isChanged) {
		for (Control* pControl : m_items) {
			TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
			if (pTreeNode != nullptr) {
				pTreeNode->SetExpandImageClass(className);
			}
		}
	}
}

std::wstring TreeView::GetExpandImageClass() const
{
	return m_expandImageClass.c_str();
}

void TreeView::SetEnableIcon(bool bEnable)
{
	bool isChanged = m_bEnableIcon != bEnable;
	m_bEnableIcon = bEnable;
	if (isChanged) {
		for (Control* pControl : m_items) {
			TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
			if (pTreeNode != nullptr) {
				pTreeNode->SetEnableIcon(bEnable);
			}
		}
	}
}

bool TreeView::IsEnableIcon() const
{
	return m_bEnableIcon;
}

bool TreeView::InsertControlBeforeNode(TreeNode* pTreeNode, Control* pControl)
{
	if ((pTreeNode == nullptr) || (pControl == nullptr)) {
		return false;
	}
	bool bAdded = false;
	if (dynamic_cast<TreeNode*>(pControl) != nullptr) {
		//不允许通过该接口添加树节点
		return false;
	}
	size_t iIndex = pTreeNode->GetListBoxIndex();
	if (Box::IsValidItemIndex(iIndex)) {
		bAdded = ListBox::AddItemAt(pControl, iIndex);
	}
	return bAdded;
}

bool TreeView::RemoveControl(Control* pControl)
{
	if (pControl == nullptr) {
		return false;
	}
	bool bRemoved = false;
	if (dynamic_cast<TreeNode*>(pControl) != nullptr) {
		//不允许通过该接口移除树节点
		return false;
	}
	bRemoved = ListBox::RemoveItem(pControl);
	return bRemoved;
}

bool TreeView::AddItem(Control* /*pControl*/)
{
	ASSERT(FALSE);
	return false;
}

bool TreeView::AddItemAt(Control* /*pControl*/, size_t /*iIndex*/)
{
	ASSERT(FALSE);
	return false;
}

bool TreeView::RemoveItem(Control* /*pControl*/)
{
	ASSERT(FALSE);
	return false;
}

bool TreeView::RemoveItemAt(size_t /*iIndex*/)
{
	ASSERT(FALSE);
	return false;
}

void TreeView::RemoveAllItems()
{
	ASSERT(FALSE);
}

void TreeView::SetParent(Box* pParent)
{
	ListBox::SetParent(pParent);
	m_rootNode->SetParent(pParent);
}

void TreeView::SetWindow(Window* pManager)
{
	__super::SetWindow(pManager);
	m_rootNode->SetWindow(pManager);
}

void TreeView::OnNodeCheckStatusChanged(TreeNode* pTreeNode)
{
	if (pTreeNode == nullptr) {
		return;
	}
	if (!pTreeNode->SupportCheckedMode()) {
		//单选或者不显示CheckBox：直接返回
		return;
	}

	//多选
	bool isChecked = pTreeNode->IsChecked();
	//同步子节点的勾选状态：跟随当前节点
	pTreeNode->SetChildrenCheckStatus(isChecked);

	//同步父节点的勾选状态
	pTreeNode->UpdateParentCheckStatus(false);
}

}