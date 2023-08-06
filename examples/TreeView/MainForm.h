#pragma once

#include "resource.h"

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

#include "FileInfoList.h"

class MainForm : public ui::WindowImplBase
{
public:
	MainForm();
	~MainForm();

	/**
	 * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
	 * GetSkinFolder		接口设置你要绘制的窗口皮肤资源路径
	 * GetSkinFile			接口设置你要绘制的窗口的 xml 描述文件
	 * GetWindowClassName	接口设置窗口唯一的类名称
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;

	/**
	 * 收到 WM_CREATE 消息时该函数会被调用，通常做一些控件初始化的操作
	 */
	virtual void OnInitWindow() override;

	/**
	 * 收到 WM_CLOSE 消息时该函数会被调用
	 */
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	static const std::wstring kClassName;

private:
	//显示桌面节点
	void ShowDesktopNode();

	//显示磁盘节点
	void ShowAllDiskNode();

	//显示指定目录的子目录
	void ShowSubFolders(ui::TreeNode* pTreeNode, const std::wstring& path);

	//在树中插入一个节点
	void InsertTreeNode(ui::TreeNode* pTreeNode, 
		                const std::wstring& displayName,
		                const std::wstring& path,
		                bool isFolder,
		                HICON hIcon);

	//显示指定目录的内容
	void ShowFolderContents(const std::wstring& path);

	/** 树节点展开事件
	 * @param[in] args 消息体
	 * @return 始终返回 true
	 */
	bool OnTreeNodeExpand(const ui::EventArgs& args);

	/** 树节点点击事件
	 * @param[in] args 消息体
	 * @return 始终返回 true
	 */
	bool OnTreeNodeClick(const ui::EventArgs& args);

private:
	//树节点的接口
	ui::TreeView* m_pTree;

	//目录列表（左侧树显示）
	struct FolderStatus
	{
		std::wstring path;
		bool bShow = false;
		HICON hIcon = nullptr;
	};
	std::vector<FolderStatus*> m_folderList;

	/** 文件列表（右侧虚表显示）
	*/
	FileInfoList m_fileList;

	//文件列表的接口
	ui::VirtualListBox* m_pListBox;
};


