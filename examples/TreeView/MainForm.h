#pragma once

#include "resource.h"

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

#include "FileInfoList.h"
#include <Shlobj.h>

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

	/** 触发树节点点击事件
	 */
	void CheckExpandTreeNode(ui::TreeNode* pTreeNode, const std::wstring& filePath);

private:
	//目录列表数据结构
	struct FolderStatus
	{
		std::wstring path;
		bool bShow = false;
		HICON hIcon = nullptr;
		ui::TreeNode* pTreeNode = nullptr;
	};

	//显示虚拟目录节点（比如桌面、我的文档等）
	void ShowVirtualDirectoryNode(int csidl, REFKNOWNFOLDERID rfid, const std::wstring& name);

	//显示磁盘节点, 返回第一个新节点接口
	ui::TreeNode* ShowAllDiskNode();

	//显示指定目录的子目录
	void ShowSubFolders(ui::TreeNode* pTreeNode, const std::wstring& path);

	//在树中添加一个节点, 返回新添加的节点接口
	ui::TreeNode* InsertTreeNode(ui::TreeNode* pTreeNode,
		                         const std::wstring& displayName,
								 const std::wstring& path,
								 bool isFolder,
								 HICON hIcon);

	//批量在树中插入一个节点
	void InsertTreeNodes(ui::TreeNode* pTreeNode, 
		                const std::wstring& path,
						const std::vector<FolderStatus>& fileList,
		                bool isFolder);

	//显示指定目录的内容
	void ShowFolderContents(ui::TreeNode* pTreeNode, const std::wstring& path);

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

	/** 树节点选择事件
	 * @param[in] args 消息体
	 * @return 始终返回 true
	 */
	bool OnTreeNodeSelect(const ui::EventArgs& args);

	/** 判断一个路径是否为目录
	*/
	bool IsDirectory(const std::wstring& filePath) const;

private:
	//树节点的接口
	ui::TreeView* m_pTree;

	//目录列表（左侧树显示）
	std::vector<FolderStatus*> m_folderList;

	/** 文件列表（右侧虚表显示）
	*/
	FileInfoList m_fileList;

	//文件列表的接口
	ui::VirtualListBox* m_pListBox;

	/** L"Shell32.dll" 句柄
	*/
	HMODULE m_hShell32Dll;
};


