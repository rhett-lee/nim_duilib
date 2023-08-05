#ifndef UI_CONTROL_TREEVIEW_H_
#define UI_CONTROL_TREEVIEW_H_

#pragma once

#include "duilib/Box/ListBox.h"

namespace ui
{

#define ROOT_NODE_DEPTH  -1

class TreeView;
class UILIB_API TreeNode : public ListBoxItem
{
public:
	TreeNode();
	TreeNode(const TreeNode& r) = delete;
	TreeNode& operator=(const TreeNode& r) = delete;

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual bool IsVisible() const override;
	virtual void SetWindow(Window* pManager, Box* pParent, bool bInit = true) override;
	virtual void SetWindow(Window* pManager) override;

	/** 子项被点击时触发
	 * @param[in] param 消息体
	 * @return 始终返回 true
	 */
	virtual bool OnClickItem(const EventArgs& param);

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
	bool AddChildNodeAt(TreeNode* pTreeNode, size_t iIndex);

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
	 * @return 返回当前层级
	 */
	int GetDepth() const;

	/** 监听子项展开事件
	 * @param[in] callback 子项展开时触发的回调函数
	 */
	void AttachExpand(const EventCallback& callback) { AttachEvent(kEventExpand, callback); }

	/** 监听子项收缩事件
	 * @param[in] callback 子项收缩时触发的回调函数
	 */
	void AttachUnExpand(const EventCallback& callback) { AttachEvent(kEventUnExpand, callback); }

private:
	/** 删除自身
	 * @return 成功返回 true，失败返回 false
	 */
	bool RemoveSelf();

private:
	//子项层级
	int m_iDepth;
	//是否展开显示子节点
	bool m_bExpand;
	//子项所属的树容器
	TreeView* m_pTreeView;
	//父节点
	TreeNode *m_pParentTreeNode;
	//子节点列表
	std::vector<TreeNode*> m_aTreeNodes;
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
	 * @param[in] indent 要设置的缩进值, 单位为像素
	 */
	void SetIndent(int32_t indent);

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

	//树的根节点
	std::unique_ptr<TreeNode> m_rootNode;
};

}

#endif // UI_CONTROL_TREEVIEW_H_
