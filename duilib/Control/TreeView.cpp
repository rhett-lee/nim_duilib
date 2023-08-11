#include "TreeView.h"
#include "duilib/Core/ScrollBar.h"

namespace ui
{

TreeNode::TreeNode() :
	m_bExpand(true),
	m_pTreeView(nullptr),
	m_pParentTreeNode(nullptr),
	m_iDepth(ROOT_NODE_DEPTH),
	m_extraPadding(3),
	m_expandCheckBoxPadding(0),
	m_expandIconPadding(0),
	m_expandTextPadding(0),
	m_checkBoxIconPadding(0),
	m_checkBoxTextPadding(0),
	m_iconTextPadding(0),
	m_pExpandImageRect(nullptr),
	m_pUnExpandImageRect(nullptr)
{
}

TreeNode::~TreeNode()
{
	if (m_pExpandImageRect != nullptr) {
		delete m_pExpandImageRect;
		m_pExpandImageRect = nullptr;
	}
	if (m_pUnExpandImageRect != nullptr) {
		delete m_pUnExpandImageRect;
		m_pUnExpandImageRect = nullptr;
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
	else if (strName == L"unexpand_normal_image") {
		SetUnExpandStateImage(kControlStateNormal, strValue);
	}
	else if (strName == L"unexpand_hot_image") {
		SetUnExpandStateImage(kControlStateHot, strValue);
	}
	else if (strName == L"unexpand_pushed_image") {
		SetUnExpandStateImage(kControlStatePushed, strValue);
	}
	else if (strName == L"unexpand_disabled_image") {
		SetUnExpandStateImage(kControlStateDisabled, strValue);
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

std::wstring TreeNode::GetUnExpandStateImage(ControlStateType stateType)
{
	Image* pImage = nullptr;
	if (m_unexpandImage != nullptr) {
		pImage = m_unexpandImage->GetStateImage(stateType);
	}
	if (pImage != nullptr) {
		return pImage->GetImageString();
	}
	return std::wstring();
}

void TreeNode::SetUnExpandStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (m_unexpandImage == nullptr) {
		m_unexpandImage.reset(new StateImage);
		m_unexpandImage->SetControl(this);
	}
	m_unexpandImage->SetImageString(stateType, strImage);
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
		if (m_unexpandImage != nullptr) {
			if (m_pUnExpandImageRect == nullptr) {
				m_pUnExpandImageRect = new UiRect;
			}
			m_unexpandImage->PaintStateImage(pRender, GetState(), L"", m_pUnExpandImageRect);
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
	if (!pos.ContainsPt(msg.ptMouse)) {
		return bRet;
	}
	if (IsExpand()) {
		//展开状态
		if ((m_expandImage != nullptr) && !m_aTreeNodes.empty()) {
			//如果点击在展开图标上，则收起
			if ((m_pExpandImageRect != nullptr) && 
				m_pExpandImageRect->ContainsPt(msg.ptMouse)) {
				SetExpand(false, true);
			}
		}
	}
	else {
		//未展开状态
		if (m_unexpandImage != nullptr) {
			//如果点击在展开图标上，则展开
			if ((m_pUnExpandImageRect != nullptr) && 
				m_pUnExpandImageRect->ContainsPt(msg.ptMouse)) {
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
	if (m_unexpandImage != nullptr) {
		pImage = m_unexpandImage->GetStateImage(kControlStateNormal);
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
		const int32_t extraPadding = ui::GlobalManager::Instance().Dpi().GetScaleInt(m_extraPadding);
		imageWidth += extraPadding;
	}
	return imageWidth;
}

void TreeNode::SetTreeView(TreeView* pTreeView)
{
    m_pTreeView = pTreeView;
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

bool TreeNode::OnItemSelectedChanged(const EventArgs& args)
{
	TreeNode* pItem = dynamic_cast<TreeNode*>(args.pSender);
	ASSERT(pItem != nullptr);
	if ((pItem != nullptr) && (m_pTreeView != nullptr)) {
		m_pTreeView->OnItemSelectedChanged(pItem);
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

void TreeNode::SetWindow(Window* pManager, Box* pParent, bool bInit)
{
	ListBoxItem::SetWindow(pManager, pParent, bInit);
}

void TreeNode::SetWindow(Window* pManager)
{
	__super::SetWindow(pManager);
}

TreeNode* TreeNode::GetParentNode()
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

bool TreeNode::AddChildNodeAt(TreeNode* pTreeNode, size_t iIndex)
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
	m_aTreeNodes.insert(m_aTreeNodes.begin() + iIndex, pTreeNode);
		
	pTreeNode->m_iDepth = m_iDepth + 1;
	pTreeNode->SetParentNode(this);
	pTreeNode->SetTreeView(m_pTreeView);
	if (GetWindow() != nullptr) {
		GetWindow()->InitControls(pTreeNode, nullptr);
	}
	pTreeNode->AttachEvent(kEventMouseDoubleClick, nbase::Bind(&TreeNode::OnDoubleClickItem, this, std::placeholders::_1));
	pTreeNode->AttachSelect(nbase::Bind(&TreeNode::OnItemSelectedChanged, this, std::placeholders::_1));
	pTreeNode->AttachUnSelect(nbase::Bind(&TreeNode::OnItemSelectedChanged, this, std::placeholders::_1));

	UiPadding padding = GetPadding();
	size_t nodeIndex = 0;
	if (m_iDepth != ROOT_NODE_DEPTH) {
		nodeIndex = GetListBoxIndex() + 1;
		padding.left += m_pTreeView->GetIndent();
	}
	pTreeNode->SetPadding(padding, false);

	size_t nGlobalIndex = iIndex;
	for (size_t i = 0; i < iIndex; i++)	{
		nGlobalIndex += ((TreeNode*)m_aTreeNodes[i])->GetDescendantNodeCount();
	}

	//[未展开/展开]图片标志
	std::wstring expandImageClass = m_pTreeView->GetExpandImageClass();
	pTreeNode->SetExpandImageClass(expandImageClass);

	//CheckBox选项
	std::wstring checkBoxClass = m_pTreeView->GetCheckBoxClass();
	pTreeNode->SetCheckBoxClass(checkBoxClass);

	//是否显示图标
	pTreeNode->SetEnableIcon(m_pTreeView->IsEnableIcon());

	//添加到ListBox容器中
	return m_pTreeView->ListBox::AddItemAt(pTreeNode, nodeIndex + nGlobalIndex);
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
		m_unexpandImage.reset();
		if (m_pExpandImageRect != nullptr) {
			delete m_pExpandImageRect;
			m_pExpandImageRect = nullptr;
		}
		if (m_pUnExpandImageRect != nullptr) {
			delete m_pUnExpandImageRect;
			m_pUnExpandImageRect = nullptr;
		}
	}
	AdjustExpandImagePadding();
}

void TreeNode::SetCheckBoxClass(const std::wstring& checkBoxClass)
{
	if (!checkBoxClass.empty()) {
		//开启CheckBox功能
		SetClass(checkBoxClass);
	}
	else {
		//关闭CheckBox功能
		ClearStateImages();
	}
	AdjustCheckBoxPadding();
}

void TreeNode::AdjustExpandImagePadding()
{
	uint16_t expandPadding = ui::TruncateToUInt8(GetExpandImagePadding());
	if (expandPadding != 0) {
		//显示[展开/未展开]标志
		if (m_expandCheckBoxPadding == 0) {
			int32_t leftOffset = (int32_t)expandPadding;
			if (AdjustStateImagesPaddingLeft(leftOffset, false)) {
				m_expandCheckBoxPadding = expandPadding;
			}
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
		//不显示[展开/未展开]标志
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
		const int32_t extraPadding = ui::GlobalManager::Instance().Dpi().GetScaleInt(m_extraPadding);
		UiSize imageSize = GetStateImageSize(kStateImageBk, kControlStateNormal);
		uint16_t checkBoxPadding = TruncateToUInt16(imageSize.cx);
		if (checkBoxPadding > 0) {
			checkBoxPadding += (uint16_t)extraPadding;
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
			const int32_t extraPadding = ui::GlobalManager::Instance().Dpi().GetScaleInt(m_extraPadding);
			UiSize imageSize = GetBkImageSize();
			uint16_t iconTextPadding = TruncateToUInt16(imageSize.cx);
			if (iconTextPadding > 0) {
				iconTextPadding += (uint16_t)extraPadding;
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

void TreeNode::SetChildrenSelected(bool bSelected)
{
	if ((m_pTreeView == nullptr) || !m_pTreeView->IsMultiSelect()) {
		//单选: 直接返回
		return;
	}
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode) {
			pTreeNode->SetItemSelected(bSelected);
			pTreeNode->SetChildrenSelected(bSelected);
		}
	}
}

void TreeNode::UpdateParentSelected(bool bUpdateSelf)
{
	if ((m_pTreeView != nullptr) && !m_pTreeView->IsMultiSelect()) {
		//单选: 直接返回
		return;
	}
	if (bUpdateSelf) {
		UpdateTreeNodeSelect();
	}
	if (m_pParentTreeNode != nullptr) {
		m_pParentTreeNode->UpdateParentSelected(true);
	}
}

void TreeNode::UpdateTreeNodeSelect()
{
	if ((m_pTreeView != nullptr) && !m_pTreeView->IsMultiSelect()) {
		//单选: 直接返回
		return;
	}
	std::wstring text = this->GetText();
	bool bSelected = IsSelected();
	TreeNodeSelect nodeSelect = GetChildrenSelectStatus();//根据子节点的选择状态，修改当前节点的选择状态
	if (nodeSelect == TreeNodeSelect::UnSelect) {
		if (!bSelected) {
			return;
		}
		else {
			//更新为：TreeNodeSelect::UnSelect
			SetSelected(false);
			SetPartSelected(false);
			Invalidate();
		}
	}
	else if (nodeSelect == TreeNodeSelect::SelectAll) {
		//更新为：TreeNodeSelect::SelectAll
		if (bSelected) {
			if (IsPartSelected()) {
				SetPartSelected(false);
				Invalidate();
			}
		}
		else {			
			SetSelected(true);
			SetPartSelected(false);
			Invalidate();
		}
	}
	else if (nodeSelect == TreeNodeSelect::SelectPart) {
		//更新为：TreeNodeSelect::SelectPart
		SetSelected(true);
		SetPartSelected(true);
		Invalidate();
	}
}

TreeNodeSelect TreeNode::GetSelectStatus(void) const
{
	if ((m_pTreeView != nullptr) && !m_pTreeView->IsMultiSelect()) {
		//单选: 只按当前节点状态判断结果
		return IsSelected() ? TreeNodeSelect::SelectAll : TreeNodeSelect::UnSelect;
	}
	//多选
	bool bSelected = IsSelected();
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode == nullptr) {
			continue;
		}
		bool bChildSelected = pTreeNode->IsSelected();
		if (bSelected != bChildSelected) {
			return TreeNodeSelect::SelectPart;
		}
	}

	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode == nullptr) {
			continue;
		}
		TreeNodeSelect childSelect = pTreeNode->GetSelectStatus();
		if (bSelected) {
			if (childSelect == TreeNodeSelect::UnSelect) {
				return TreeNodeSelect::SelectPart;
			}
			else if (childSelect == TreeNodeSelect::SelectPart) {
				return TreeNodeSelect::SelectPart;
			}
		}
		else {
			if (childSelect == TreeNodeSelect::SelectAll) {
				return TreeNodeSelect::SelectPart;
			}
			else if (childSelect == TreeNodeSelect::SelectPart) {
				return TreeNodeSelect::SelectPart;
			}
		}
	}
	return bSelected ? TreeNodeSelect::SelectAll : TreeNodeSelect::UnSelect;
}

TreeNodeSelect TreeNode::GetChildrenSelectStatus(void) const
{
	if ((m_pTreeView != nullptr) && !m_pTreeView->IsMultiSelect()) {
		//单选: 只按当前节点状态判断结果
		return IsSelected() ? TreeNodeSelect::SelectAll : TreeNodeSelect::UnSelect;
	}
	if (m_aTreeNodes.empty()) {
		//没有子节点：返回当前节点的状态
		return IsSelected() ? TreeNodeSelect::SelectAll : TreeNodeSelect::UnSelect;
	}
	//多选: 先扫描一级子节点
	bool bLastSelected = false;
	bool bSetLastSelected = false;
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode == nullptr) {
			continue;
		}
		bool bChildSelected = pTreeNode->IsSelected();
		if (!bSetLastSelected) {
			bLastSelected = bChildSelected;
			bSetLastSelected = true;
		}
		else {
			if (bLastSelected != bChildSelected) {
				return TreeNodeSelect::SelectPart;
			}
		}		
	}

	//再扫描多级子节点
	TreeNodeSelect lastChildSelect = TreeNodeSelect::UnSelect;
	bool bSetLastChildrenSelect = false;
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode == nullptr){
			continue;
		}
		TreeNodeSelect childSelect = pTreeNode->GetSelectStatus();//获取包含自身/子节点的选择状态
		if (childSelect == TreeNodeSelect::SelectPart) {
			return TreeNodeSelect::SelectPart;
		}
		if (!bSetLastChildrenSelect) {
			lastChildSelect = childSelect;
			bSetLastChildrenSelect = true;
		}
		else {
			if (childSelect != lastChildSelect) {
				return TreeNodeSelect::SelectPart;
			}
		}
	}

	//如果状态都一样，返回第一个子节点的状态
	TreeNode* pTreeNode = m_aTreeNodes.front();
	if (pTreeNode != nullptr) {
		return pTreeNode->IsSelected() ? TreeNodeSelect::SelectAll : TreeNodeSelect::UnSelect;
	}
	else {
		return IsSelected() ? TreeNodeSelect::SelectAll : TreeNodeSelect::UnSelect;
	}
}

bool TreeNode::RemoveChildNodeAt(size_t iIndex)
{
	ASSERT(iIndex < m_aTreeNodes.size());
	if (iIndex >= m_aTreeNodes.size()) {
		return false;
	}

	TreeNode* pTreeNode = ((TreeNode*)m_aTreeNodes[iIndex]);
	m_aTreeNodes.erase(m_aTreeNodes.begin() + iIndex);
	if (pTreeNode) {
		return pTreeNode->RemoveSelf();
	}
	return false;
}

bool TreeNode::RemoveChildNode(TreeNode* pTreeNode)
{
	auto it = std::find(m_aTreeNodes.begin(), m_aTreeNodes.end(), pTreeNode);
	if (it == m_aTreeNodes.end()) {
		return false;
	}		
	size_t iIndex = it - m_aTreeNodes.begin();
	return RemoveChildNodeAt(iIndex);
}
	
void TreeNode::RemoveAllChildNode()
{
	while (m_aTreeNodes.size() > 0) {
		RemoveChildNodeAt(0);
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
	if (m_iDepth != ROOT_NODE_DEPTH) {
		return m_pTreeView->ListBox::RemoveItemAt(GetListBoxIndex());
	}
	return false;
}

size_t TreeNode::GetDescendantNodeCount()
{
	size_t nodeCount = GetChildNodeCount();
	for (TreeNode* pTreeNode : m_aTreeNodes) {
		if (pTreeNode != nullptr) {
			nodeCount += pTreeNode->GetDescendantNodeCount();
		}
	}
	return nodeCount;
}

size_t TreeNode::GetChildNodeCount()
{
	return m_aTreeNodes.size();
}
	
TreeNode* TreeNode::GetChildNode(size_t iIndex)
{
	if (iIndex >= m_aTreeNodes.size()) {
		return nullptr;
	}
	return m_aTreeNodes[iIndex];
}
	
size_t TreeNode::GetChildNodeIndex(TreeNode* pTreeNode)
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
		SendEvent(m_bExpand ? kEventExpand : kEventUnExpand);
	}
	if (m_pTreeView != nullptr) {
		m_pTreeView->Arrange();
	}	
}

int32_t TreeNode::GetDepth() const
{
	return m_iDepth;
}

void TreeNode::SetExtraPadding(int32_t extraPadding)
{
	m_extraPadding = TruncateToUInt8(extraPadding);
}

TreeView::TreeView() :
	ListBox(new VLayout),
	m_iIndent(0),
	m_rootNode(),
	m_bEnableIcon(true)
{
	m_rootNode.reset(new TreeNode());
	m_rootNode->SetTreeView(this);
	//缩进默认设置为20个像素
	SetIndent(20, true);
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
		ListBox::SetAttribute(strName, strValue);
	}
	else if (strName == L"check_box_class") {
		//是否显示CheckBox
		SetCheckBoxClass(strValue);
	}
	else if (strName == L"expand_image_class") {
		//是否显示[展开/未展开]图标
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
	bool isChanged = m_checkBoxClass != className;
	m_checkBoxClass = className;
	if (isChanged) {
		for (Control* pControl : m_items) {
			TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
			if (pTreeNode != nullptr) {
				pTreeNode->SetCheckBoxClass(className);
			}
		}
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

void TreeView::SetWindow(Window* pManager, Box* pParent, bool bInit)
{
	ListBox::SetWindow(pManager, pParent, bInit);
	m_rootNode->SetWindow(pManager, pParent, bInit);
}

void TreeView::SetWindow(Window* pManager)
{
	__super::SetWindow(pManager);
	m_rootNode->SetWindow(pManager);
}

void TreeView::OnItemSelectedChanged(TreeNode* pTreeNode)
{
	if (pTreeNode == nullptr) {
		return;
	}
	if (!IsMultiSelect()) {
		//单选：直接返回
		return;
	}

	//多选
	bool isSelected = pTreeNode->IsSelected();
	//同步子节点的选择状态：跟随当前节点
	pTreeNode->SetChildrenSelected(isSelected);

	//同步父节点的选择状态
	pTreeNode->UpdateParentSelected(false);
}

}