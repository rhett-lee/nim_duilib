#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

#include "resource.h"

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
     * GetSkinFolder        接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile            接口设置你要绘制的窗口的 xml 描述文件
     * GetWindowClassName    接口设置窗口唯一的类名称
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual DString GetWindowClassName() const override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

    /** 窗口类名
    */
    static const DString kClassName;

    /** 触发树节点点击事件
     */
    void CheckExpandTreeNode(ui::TreeNode* pTreeNode, const DString& filePath);

private:
    //目录列表数据结构
    struct FolderStatus
    {
        DString path;
        bool bShow = false;
        HICON hIcon = nullptr;
        ui::TreeNode* pTreeNode = nullptr;
    };

    //显示虚拟目录节点（比如桌面、我的文档等）
    void ShowVirtualDirectoryNode(int csidl, REFKNOWNFOLDERID rfid, const DString& name);

    //显示磁盘节点, 返回第一个新节点接口
    ui::TreeNode* ShowAllDiskNode();

    //显示指定目录的子目录
    void ShowSubFolders(ui::TreeNode* pTreeNode, const DString& path);

    //在树中添加一个节点, 返回新添加的节点接口
    ui::TreeNode* InsertTreeNode(ui::TreeNode* pTreeNode,
                                 const DString& displayName,
                                 const DString& path,
                                 bool isFolder,
                                 HICON hIcon);

    //批量在树中插入一个节点
    void InsertTreeNodes(ui::TreeNode* pTreeNode, 
                        const DString& path,
                        const std::vector<FolderStatus>& fileList,
                        bool isFolder);

    //显示指定目录的内容
    void ShowFolderContents(ui::TreeNode* pTreeNode, const DString& path);

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
    bool IsDirectory(const DString& filePath) const;

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

    /** _T("Shell32.dll") 句柄
    */
    HMODULE m_hShell32Dll;
};

#endif //EXAMPLES_MAIN_FORM_H_
