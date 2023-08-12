#ifndef UI_CONTROL_TREEVIEW_H_
#define UI_CONTROL_TREEVIEW_H_

#pragma once

#include "duilib/Box/ListBox.h"
#include <map>

namespace ui
{

/** 节点选择状态
*/
enum class TreeNodeSelect
{
	UnSelect,	//没有选择
	SelectAll,	//全部选择
	SelectPart  //部分选择
};

class TreeView;
class UILIB_API TreeNode : public ListBoxItem
{
public:
	TreeNode();
	TreeNode(const TreeNode& r) = delete;
	TreeNode& operator=(const TreeNode& r) = delete;
	virtual ~TreeNode();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual void PaintStateImages(IRender* pRender) override;
	virtual bool ButtonDown(const EventArgs& msg) override;
	virtual bool IsVisible() const override;
	virtual void SetWindow(Window* pManager, Box* pParent, bool bInit = true) override;
	virtual void SetWindow(Window* pManager) override;

	/** 子项被双击时触发
	 * @param[in] args 消息体
	 * @return 始终返回 true
	 */
	virtual bool OnDoubleClickItem(const EventArgs& args);

	/** 设置子项所属的树容器
	 * @param[in] pTreeView 容器指针
	 */
	void SetTreeView(TreeView* pTreeView);

	/** 获取父节点指针
	 * @return 返回父节点指针
	 */
	TreeNode* GetParentNode();

	/** 设置父节点
	 * @param[in] pParentTreeNode 父节点指针
	 */
	void SetParentNode(TreeNode* pParentTreeNode);

	/** 添加子节点
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
	void RemoveAllChildNode();

	/** 递归获取所有子节点个数
	 * @return 返回所有子节点个数
	 */
	size_t GetDescendantNodeCount();

	/** 获取下一级子节点个数
	 * @return 返回子节点个数
	 */
	size_t GetChildNodeCount();

	/** 获取一个子节点指针
	 * @param[in] iIndex 要获取的子节点索引
	 * @return 返回子节点指针
	 */
	TreeNode* GetChildNode(size_t iIndex);

	/** 根据子节点指针获取子节点位置索引
	 * @param[in] pTreeNode 子节点指针
	 * @return 返回位置索引
	 */
	size_t GetChildNodeIndex(TreeNode* pTreeNode);

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
	void SetCheckBoxClass(const std::wstring& checkBoxClass);

	/** 设置是否显示图标
	*/
	void SetEnableIcon(bool bEnable);

	/** 更改所有子节点的选择状态，但不触发选择变化事件
	* @param [in] bSelected 选择状态
	*/
	void SetChildrenSelected(bool bSelected);

	/** 更改所有父亲节点的选择状态，但不触发选择变化事件
	* @param [in] bUpdateSelf 是否需要更新自己的选择状态
	*/
	void UpdateParentSelected(bool bUpdateSelf);

	/** 更新节点的选择状态(三态选择状态)
	*/
	void UpdateTreeNodeSelect();

	/** 获取当前节点的选择状态(自身和子节点)
	*/
	TreeNodeSelect GetSelectStatus(void) const;

	/** 获取当前节点的子节点选择状态(不包含自身，只包含子节点)
	*/
	TreeNodeSelect GetChildrenSelectStatus(void) const;

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

	/** 监听子项展开事件
	 * @param[in] callback 子项展开时触发的回调函数
	 */
	void AttachExpand(const EventCallback& callback) { AttachEvent(kEventExpand, callback); }

	/** 监听子项收缩事件
	 * @param[in] callback 子项收缩时触发的回调函数
	 */
	void AttachCollapse(const EventCallback& callback) { AttachEvent(kEventCollapse, callback); }

private:
	/** 删除自身
	 * @return 成功返回 true，失败返回 false
	 */
	bool RemoveSelf();

	/** 根据当前的配置，调整展开标志关联的内边距(可重入函数，多次调用无副作用)
	*/
	void AdjustExpandImagePadding();

	/** 根据当前的配置，调整CheckBox关联的内边距(可重入函数，多次调用无副作用)
	*/
	void AdjustCheckBoxPadding();

	/** 根据当前的配置，调整图标关联的内边距(可重入函数，多次调用无副作用)
	*/
	void AdjustIconPadding();

	/** 子项选择状态变化时触发
	 * @param[in] args 消息体
	 * @return 始终返回 true
	 */
	bool OnItemSelectedChanged(const EventArgs& args);

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
public:
	TreeView(void);

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

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

	/** 树节点选择状态变化
	 * @param [in] pTreeNode 树节点接口
	 */
	void OnItemSelectedChanged(TreeNode* pTreeNode);

	/** 在某个树节点前添加普通控件，以实现一些效果，比如不同类型节点间的分隔符等
	* @param [in] pTreeNode 树的节点接口，不允许为空
	* @param [in] pControl 需要添加的普通控件接口，不允许为空
	*/
	bool InsertControlBeforeNode(TreeNode* pTreeNode, Control* pControl);

	/** 删除树节点中的普通控件
	* @param [in] pControl 需要删除的普通控件接口，不允许为空
	*/
	bool RemoveControl(Control* pControl);

private:
	//以下函数故意私有化，表明禁止使用；应该使用TreeNode中的相关函数
	bool AddItem(Control* pControl) override;
	bool AddItemAt(Control* pControl, size_t iIndex) override;
	bool RemoveItem(Control* pControl) override;
	bool RemoveItemAt(size_t iIndex) override;
	void RemoveAllItems() override;

	virtual void SetWindow(Window* pManager, Box* pParent, bool bInit = true) override;
	virtual void SetWindow(Window* pManager) override;

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
};

}

#endif // UI_CONTROL_TREEVIEW_H_
