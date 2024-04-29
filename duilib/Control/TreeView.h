#ifndef UI_CONTROL_TREEVIEW_H_
#define UI_CONTROL_TREEVIEW_H_

#pragma once

#include "duilib/Box/ListBox.h"
#include <map>

namespace ui
{

/** 节点勾选状态
*/
enum class TreeNodeCheck
{
	UnCheck,	 //没有打勾
	CheckedAll,	 //全部打勾
	CheckedPart  //部分打勾
};

class TreeView;
class UILIB_API TreeNode : public ListBoxItem
{
	friend class TreeView;
public:
	TreeNode();
	TreeNode(const TreeNode& r) = delete;
	TreeNode& operator=(const TreeNode& r) = delete;
	virtual ~TreeNode();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual bool IsVisible() const override;
	virtual bool SupportCheckedMode() const override;

private:
	virtual void PaintStateImages(IRender* pRender) override;
	virtual bool ButtonDown(const EventArgs& msg) override;
	virtual bool OnDoubleClickItem(const EventArgs& args);

public:
	/** 设置子项所属的树容器
	 * @param[in] pTreeView 容器指针
	 */
	void SetTreeView(TreeView* pTreeView);

	/** 获取子项所属的树容器
	 */
	TreeView* GetTreeView() const;

	/** 获取父节点指针
	 * @return 返回父节点指针
	 */
	TreeNode* GetParentNode() const;

	/** 设置父节点
	 * @param[in] pParentTreeNode 父节点指针
	 */
	void SetParentNode(TreeNode* pParentTreeNode);

	/** 在最后面添加一个新的子节点
	 * @param[in] pTreeNode 子节点指针
	 * @return 成功返回 true，失败返回 false
	 */
	bool AddChildNode(TreeNode* pTreeNode);

	/** 在指定位置添加子节点
	 * @param[in] pTreeNode 子节点指针
	 * @param[in] iIndex 要插入哪个位置之后
	 * @return 成功返回 true，失败返回 false
	 */
	bool AddChildNodeAt(TreeNode* pTreeNode, const size_t iIndex);

	/** 从指定位置移除一个子节点
	 * @param[in] iIndex 要移除的子节点索引
	 * @return 成功返回 true，失败返回 false
	 */
	bool RemoveChildNodeAt(size_t iIndex);

	/** 根据子节点指针移除一个子节点
	 * @param[in] pTreeNode 子节点指针
	 * @return 成功返回 true，失败返回 false
	 */
	bool RemoveChildNode(TreeNode* pTreeNode);

	/** 移除所有子节点
	 */
	void RemoveAllChildNodes();

	/** 递归获取所有子节点个数
	 * @return 返回所有子节点个数
	 */
	size_t GetDescendantNodeCount() const;

	/** 获取下一级子节点个数
	 * @return 返回子节点个数
	 */
	size_t GetChildNodeCount() const;

	/** 获取一个子节点指针
	 * @param[in] iIndex 要获取的子节点索引
	 * @return 返回子节点指针
	 */
	TreeNode* GetChildNode(size_t iIndex) const;

	/** 根据子节点指针获取子节点位置索引
	 * @param[in] pTreeNode 子节点指针
	 * @return 返回位置索引
	 */
	size_t GetChildNodeIndex(TreeNode* pTreeNode) const;

	/** 判断是否展开状态
	 * @return 返回 true 为展开状态，否则为 false
	 */
	bool IsExpand() const; 

	/** 设置是否展开显示子节点
	 * @param[in] bExpand 为 true 时展开，为 false 是不展开
	 */
	void SetExpand(bool bExpand, bool bTriggerEvent = false);

	/** 获取子项层级
	 * @return 返回当前层级, 根节点的层级为0，根节点是一个虚拟节点，一级节点是实节点，层级是1
	 *         树节点的缩进是：
	 *         int32_t indent = 0;
	 *         if(GetDepth() > 0) {
	 *				indent = (GetDepth() - 1) * TreeView::GetIndent();
	 *		   }
	 */
	uint16_t GetDepth() const;

#ifdef UILIB_IMPL_WINSDK

	/** 设置背景图片(HICON句柄)
	 * @param [in] hIcon 要设置的图标句柄，设置后，由ui::GlobalManager::Instance().Icon()管理资源的生命周期
	               如果hIcon为nullptr, 则删除节点的图标，但不会从ui::GlobalManager::Instance().Icon()移除原来关联的图标句柄
	 */
	void SetBkIcon(HICON hIcon);

#endif

	/** 设置是否显示图标
	*/
	void SetEnableIcon(bool bEnable);

	/** 监听子项展开事件
	 * @param[in] callback 子项展开时触发的回调函数
	 */
	void AttachExpand(const EventCallback& callback) { AttachEvent(kEventExpand, callback); }

	/** 监听子项收缩事件
	 * @param[in] callback 子项收缩时触发的回调函数
	 */
	void AttachCollapse(const EventCallback& callback) { AttachEvent(kEventCollapse, callback); }

private:
	/** 设置[未展开/展开]标志图片关联的Class，如果不为空表示开启展开标志功能，为空则关闭展开标志功能
	*   应用范围：该节点本身
	* @param [in] expandClass 展开标志图片的Class属性
	*/
	void SetExpandImageClass(const std::wstring& expandClass);

	/** 设置CheckBox关联的Class，如果不为空表示开启CheckBox功能，为空则关闭CheckBox功能
	*   应用范围：该节点本身
	* @param [in] checkBoxClass ui::CheckBox的Class属性，一般设置的主要属性有：
				  normal_image：正常状态的图片，必选属性(即不打勾时的图片)
				  selected_normal_image：选择时，正常状态的图片，必选属性(即打勾时的图片)
	*/
	bool SetCheckBoxClass(const std::wstring& checkBoxClass);

	/** 更改所有子节点的勾选状态，但不触发选择变化事件
	* @param [in] bChecked 勾选状态（打勾或者不打勾）
	*/
	void SetChildrenCheckStatus(bool bChecked);

	/** 更新自己和所有父亲节点的勾选状态（打勾或者不打勾），但不触发选择变化事件
	*   当节点的勾选状态发生变化/子节点的添加/删除时，需要调用此函数更新节点的勾选状态
	* @param [in] bUpdateSelf 是否需要更新自己的选择状态
	*/
	void UpdateParentCheckStatus(bool bUpdateSelf);

	/** 更新当前节点的勾选状态(三态选择状态)
	*/
	void UpdateSelfCheckStatus();

	/** 获取当前节点的勾选状态(自身和子节点)
	*/
	TreeNodeCheck GetCheckStatus(void) const;

	/** 获取当前节点的子节点勾选状态(不包含自身，只包含子节点)
	*/
	TreeNodeCheck GetChildrenCheckStatus(void) const;

	/** 获取展开状态的图片
	 * @param [in] stateType 要获取何种状态下的图片，参考 ControlStateType 枚举
	 * @return 返回图片路径和属性
	 */
	std::wstring GetExpandStateImage(ControlStateType stateType);

	/** 设置展开状态的图片
	 * @param [in] stateType 要设置哪中状态下的图片
	 * @param [in] strImage 图片路径和属性
	 */
	void SetExpandStateImage(ControlStateType stateType, const std::wstring& strImage);

	/** 获取未展开状态的图片
	 * @param [in] stateType 要获取何种状态下的图片，参考 ControlStateType 枚举
	 * @return 返回图片路径和属性
	 */
	std::wstring GetCollapseStateImage(ControlStateType stateType);

	/** 设置未展开状态的图片
	 * @param [in] stateType 要设置哪中状态下的图片
	 * @param [in] strImage 图片路径和属性
	 */
	void SetCollapseStateImage(ControlStateType stateType, const std::wstring& strImage);

private:
	/** 删除自身
	 * @return 成功返回 true，失败返回 false
	 */
	bool RemoveSelf();

	/** 从指定位置移除一个子节点
	 * @param [in] iIndex 要移除的子节点索引
	 * @param [in] bUpdateCheckStatus 是否更新勾选状态
	 * @return 成功返回 true，失败返回 false
	 */
	bool RemoveChildNodeAt(size_t iIndex, bool bUpdateCheckStatus);

	/** 根据当前的配置，调整展开标志关联的内边距(可重入函数，多次调用无副作用)
	*/
	void AdjustExpandImagePadding();

	/** 根据当前的配置，调整CheckBox关联的内边距(可重入函数，多次调用无副作用)
	*/
	void AdjustCheckBoxPadding();

	/** 根据当前的配置，调整图标关联的内边距(可重入函数，多次调用无副作用)
	*/
	void AdjustIconPadding();

	/** 子项勾选状态变化时触发
	 * @param[in] args 消息体
	 * @return 始终返回 true
	 */
	bool OnNodeCheckStatusChanged(const EventArgs& args);

	/** 获取展开状态图标占用的内边距宽度
	*/
	int32_t GetExpandImagePadding(void) const;

	/** 获取包含自己、自己的子孙节点中，ListBox索引号最大值，用于计算新添加节点的插入位置
	*   如果没有有效元素，则返回 Box::InvalidIndex
	*/
	size_t GetDescendantNodeMaxListBoxIndex() const;
	
private:
	//子项层级
	uint16_t m_uDepth;

	//是否展开显示子节点
	bool m_bExpand;

	//子项所属的树容器
	TreeView* m_pTreeView;

	//父节点
	TreeNode* m_pParentTreeNode;

	//子节点列表
	std::vector<TreeNode*> m_aTreeNodes;

	//图片/文字元素之间的固定间隔
	uint16_t m_expandIndent;	//[展开/收起]按钮后面的间隔
	uint16_t m_checkBoxIndent;	//CheckBox 后面的间隔
	uint16_t m_iconIndent;		//icon 图标后面的间隔

	//Expand图标关联的图标/文字内边距：3个
	uint16_t m_expandCheckBoxPadding;
	uint16_t m_expandIconPadding;
	uint16_t m_expandTextPadding;

	//CheckBox关联的图标/文字内边距：2个
	uint16_t m_checkBoxIconPadding;
	uint16_t m_checkBoxTextPadding;

	//图标关联的文字内边距：1个
	uint16_t m_iconTextPadding;

	/** 控件展开状态的图片类型与状态图片的MAP, 绘制的目标矩形
	*/
	std::unique_ptr<StateImage> m_expandImage;
	UiRect* m_pExpandImageRect;

	/** 控件未展开状态的图片类型与状态图片的MAP, 绘制的目标矩形
	*/
	std::unique_ptr<StateImage> m_collapseImage;
	UiRect* m_pCollapseImageRect;
};

class UILIB_API TreeView : public ListBox
{
	friend class TreeNode;
public:
	TreeView();
	virtual ~TreeView();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual void SetParent(Box* pParent) override;
	virtual void SetWindow(Window* pManager) override;

	/** 获取根节点
	 * @return 返回根节点指针
	 */
	TreeNode* GetRootNode()	const{ return m_rootNode.get(); }

	/** 获取子节点缩进值
	 * @return 返回子节点缩进值
	 */
	int32_t GetIndent() const { return m_iIndent;	}

	/** 设置子节点缩进值
	 * @param [in] indent 要设置的缩进值, 单位为像素
	 * @param [in] bNeedDpiScale 是否需要DPI缩放
	 */
	void SetIndent(int32_t indent, bool bNeedDpiScale);

	/** 设置[未展开/展开]标志图片关联的Class，如果不为空表示开启展开标志功能，为空则关闭展开标志功能
	*   应用范围：该树的所有节点
	* @param [in] className 展开标志图片的Class属性
	*/
	void SetExpandImageClass(const std::wstring& className);

	/** 获取[未展开/展开]标志图片关联的Class
	*/
	std::wstring GetExpandImageClass() const;

	/** 设置CheckBox关联的Class，如果不为空表示开启CheckBox功能，为空则关闭CheckBox功能
	*   应用范围：该树的所有节点
	* @param [in] className ui::CheckBox的Class属性，一般设置的主要属性有：
				  normal_image：正常状态的图片，必选属性(即不打勾时的图片)
				  selected_normal_image：选择时，正常状态的图片，必选属性(即打勾时的图片)
	*/
	void SetCheckBoxClass(const std::wstring& className);

	/** 获取CheckBox关联的Class
	*/
	std::wstring GetCheckBoxClass() const;

	/** 设置是否显示图标
	*/
	void SetEnableIcon(bool bEnable);

	/** 判断是否显示图标
	*/
	bool IsEnableIcon() const;

	/** 在某个树节点前添加普通控件，以实现一些效果，比如不同类型节点间的分隔符等
	* @param [in] pTreeNode 树的节点接口，不允许为空
	* @param [in] pControl 需要添加的普通控件接口，不允许为空
	*/
	bool InsertControlBeforeNode(TreeNode* pTreeNode, Control* pControl);

	/** 删除树节点中的普通控件
	* @param [in] pControl 需要删除的普通控件接口，不允许为空
	*/
	bool RemoveControl(Control* pControl);

	/** 是否允许多选
	*/
	virtual bool IsMultiSelect() const override;

	/** 设置是否允许多选
	*/
	virtual void SetMultiSelect(bool bMultiSelect) override;

	/** 是否为多选的勾选模式，在这个函数为true的模式下，业务逻辑说明：
	*   (1) 对于树本身的ListBox：表现为单选逻辑；
	*   (2) 对于树节点上的CheckBox：选择逻辑（即IsSelected()相关逻辑）未使用；
	        使用的是Check逻辑（即IsChecked()相关逻辑），Check逻辑是可以多勾选的。
	*/
	bool IsMultiCheckMode() const;

private:
	/** 树节点勾选状态变化
	 * @param [in] pTreeNode 树节点接口
	 */
	void OnNodeCheckStatusChanged(TreeNode* pTreeNode);

	/** 是否绘制选择状态下的背景色，提供虚函数作为可选项
	   （比如ListBox/TreeView节点在多选时，由于有勾选项，并不需要绘制选择状态的背景色）
	   @param [in] bHasStateImages 当前列表项是否有CheckBox勾选项
	*/
	virtual bool CanPaintSelectedColors(bool bHasStateImages) const override;

private:
	//以下函数故意私有化，表明禁止使用；应该使用TreeNode中的相关函数
	bool AddItem(Control* pControl) override;
	bool AddItemAt(Control* pControl, size_t iIndex) override;
	bool RemoveItem(Control* pControl) override;
	bool RemoveItemAt(size_t iIndex) override;
	void RemoveAllItems() override;

	/** 当从多选切换为单选模式的时候，需要确保列表中只有一个选择项
	* @return 如果有变化返回true，否则返回false
	*/
	virtual bool OnSwitchToSingleSelect() override;

	/** 同步当前选择项的勾选状态
	* @return 如果有变化返回true，否则返回false
	*/
	bool UpdateCurSelItemCheckStatus();

	/** 当CheckBox从显示切换到隐藏后，同步Check与Select
	    （1）将Checked的，改为Selected；
		（2）将所有Checked标志改为false
	   @return 返回true表示需要重绘，否则不需要重绘
	*/
	bool OnCheckBoxHided();

	/** 当CheckBox从隐藏切换到显示后，同步Select与Check
		（1）将Selected的，改为Checked；
		（2）Selected状态不改变
	  @return 返回true表示需要重绘，否则不需要重绘
	*/
	bool OnCheckBoxShown();

private:
	//子节点的缩进值，单位为像素
	int32_t m_iIndent;

	//展开标志图片的Class
	UiString m_expandImageClass;

	//CheckBox的Class
	UiString m_checkBoxClass;

	//是否显示图标
	bool m_bEnableIcon;

	//树的根节点
	std::unique_ptr<TreeNode> m_rootNode;

	//是否允许多选(勾选模式)
	bool m_bMultiCheckMode;
};

}

#endif // UI_CONTROL_TREEVIEW_H_
