#ifndef UI_CONTROL_DIRECTORYTREE_H_
#define UI_CONTROL_DIRECTORYTREE_H_

#include "duilib/Control/TreeView.h"
#include <vector>
#include <atomic>

namespace ui
{
/** 虚拟目录的类型
*/
enum class UILIB_API VirtualDirectoryType
{
    kUserHome,  //主文件夹
    kDesktop,   //桌面
    kDocuments, //文档
    kPictures,  //图片
    kMusic,     //音乐
    kVideos,    //视频
    kDownloads, //下载
};

class DirectoryTreeImpl;

/** 文件系统的目录树结构
*/
class UILIB_API DirectoryTree: public TreeView
{
public:
    typedef TreeView BaseClass;

    /** 文件或者路径的属性信息
    */
    struct PathInfo
    {
        FilePath m_filePath;        //文件路径        
        DString m_displayName;      //显示名称
        uint32_t m_nIconID;         //关联图标ID（GlobalManager::Instance().Icon().AddIcon的返回值，图标需要实现类添加到管理器）
        bool m_bFolder = true;      //是否为目录
        bool m_bIconShared = false; //该图标ID关联的图标是否为共享图标（共享图标不允许释放）
    };

    /** 回调函数的原型: 用于显示关联的数据
    */
    typedef std::function<void (ui::TreeNode* pTreeNode, const ui::FilePath& path,
                                const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList,
                                const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList)> ShowFolderContentsEvent;

public:
    explicit DirectoryTree(Window* pWindow);
    DirectoryTree(const DirectoryTree& r) = delete;
    DirectoryTree& operator=(const DirectoryTree& r) = delete;
    virtual ~DirectoryTree() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

public:
    /** 设置工作子线程的线程标识符（枚举目录是在子线程中完成，避免导致界面无响应）
    * @param [in] nThreadIdentifier 工作子线程的线程标识符
    */
    void SetThreadIdentifier(int32_t nThreadIdentifier);

    /** 显示虚拟目录
    * @param [in] type 虚拟目录的类型
    * @param [in] displayName 虚拟目录的显示名称
    * @param [in] bDisplayNameIsID 虚拟目录的显示名称是否为语言ID，以支持多语言
    */
    TreeNode* ShowVirtualDirectoryNode(VirtualDirectoryType type, const DString& displayName, bool bDisplayNameIsID = false);

    /** 显示所有磁盘节点, 返回第一个新节点接口
    */
    TreeNode* ShowAllDiskNodes();

    /** 在指定的节点前插入一个横向分割线
    * @param [in] pNode 节点接口
    * @param [in] lineClassName 横向分割线的Class名称（可选）
    */
    bool InsertLineBeforeNode(TreeNode* pNode, const DString& lineClassName = _T(""));

    /** 在树中添加一个节点, 返回新添加的节点接口
    * @param [in] pParentTreeNode 父节点的接口，如果为nullptr则在根节点下添加新的节点
    * @param [in] displayName 虚拟目录的显示名称
    * @param [in] bDisplayNameIsID 虚拟目录的显示名称是否为语言ID，以支持多语言
    * @param [in] path 对应路径
    * @param [in] isFolder true表示为文件夹，否则为普通文件
    * @param [in] nIconID 图标ID(在GlobalManager::Instance().Icon()中管理)，如果为0，表示无关联图标
    * @param [in] bIconShared 该图标ID关联的图标是否为共享图标（共享图标不允许释放）
    */
    TreeNode* InsertTreeNode(TreeNode* pParentTreeNode,
                             const DString& displayName,
                             bool bDisplayNameIsID,
                             const ui::FilePath& path,
                             bool isFolder,
                             uint32_t nIconID,
                             bool bIconShared);

    /** 展开树的节点并选择其子目录
    * @param [in] pTreeNode 树的节点
    * @param [in] subPath 子目录，该路径是pTreeNode对应路径的子目录，可以为空路径
    */
    void ExpandTreeNode(TreeNode* pTreeNode, FilePath subPath);

    /** 选择一个路径(逐级展开目录，并选择最终的目录，确保可见)
    */
    bool SelectPath(FilePath filePath);

    /** 获取一个路径对应的树节点
    * @param [in] filePath 需要查找的路径
    */
    TreeNode* FindPathTreeNode(FilePath filePath) const;

    /** 获取一个树节点对应的路径
    * @param [in] pTreeNode 树的节点
    */
    FilePath FindTreeNodePath(TreeNode* pTreeNode);

    /** 刷新，保持树结构与文件系统同步
    */
    void RefreshTree();

    /** 刷新树节点，保持树结构与文件系统同步
    */
    void RefreshTreeNode(TreeNode* pTreeNode);

    /** 设置用于显示关联的数据的回调函数
    * @param [in] callback 回调函数
    */
    void AttachShowFolderContents(ShowFolderContentsEvent callback);

private:
    /** 树节点展开事件
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnTreeNodeExpand(const EventArgs& args);

    /** 树节点点击事件
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnTreeNodeClick(const EventArgs& args);

    /** 树节点选择事件
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnTreeNodeSelect(const EventArgs& args);

    /** 显示指定目录的子目录
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    */
    void ShowSubFolders(TreeNode* pTreeNode, const FilePath& path);

    /** 显示指定目录的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    */
    void ShowFolderContents(TreeNode* pTreeNode, const FilePath& path);

    /** 判断一个路径是否在指定节点对应的目录中
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    */
    bool IsPathInDirectory(TreeNode* pTreeNode, const FilePath& path) const;

    /** 判断一个路径是否与指定节点对应的目录相同
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    */
    bool IsPathSame(TreeNode* pTreeNode, FilePath path) const;

private:
    /** 显示指定目录的子目录
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    * @param [in] folderList 返回path目录中的所有子目录列表
    */
    typedef std::shared_ptr<std::vector<DirectoryTree::PathInfo>> PathInfoListPtr;
    void OnShowSubFolders(TreeNode* pTreeNode, const FilePath& path, const PathInfoListPtr& folderList);

    /** 显示指定目录的子目录（多级子目录）
    * @param [in] pTreeNode 当前的节点
    * @param [in] filePathList 路径列表
    * @param [in] folderList 返回每个目录中的所有子目录列表
    */
    void OnShowSubFoldersEx(TreeNode* pTreeNode, const std::vector<FilePath>& filePathList, const std::vector<PathInfoListPtr>& folderListArray);

    /** 已获取指定目录的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    * @param [in] folderList 返回path目录中的所有子目录列表
    * @param [in] fileList 返回path目录中的所有文件列表
    */
    void OnShowFolderContents(TreeNode* pTreeNode, const ui::FilePath& path,
                              const PathInfoListPtr& folderList,
                              const PathInfoListPtr& fileList);

    /** 比较两个路径是否相同
    */
    bool IsSamePath(const UiString& p1, const UiString& p2) const;

private:

    /** 目录列表数据结构
    */
    struct FolderStatus
    {
        UiString m_filePath;                //规则的文件路径
        TreeNode* m_pTreeNode = nullptr;    //关联的树节点指针
        uint32_t m_nIconID = 0;             //关联的图标ID
        bool m_bContentLoaded = false;      //当前目录的子目录是否已经加载过
        bool m_bFolder = true;              //是否为文件夹
        bool m_bIconShared = false;         //该图标ID关联的图标是否为共享图标（共享图标不允许释放）        
    };

    /** 根据Key获取目录结构数据
    */
    FolderStatus* GetFolderData(TreeNode* pTreeNode) const;

    /** 根据路径查找目录结构数据
    */
    FolderStatus* GetFolderData(FilePath filePath) const;

private:
    /** 枚举目录的内部实现（不同平台不同实现）
    */
    DirectoryTreeImpl* m_impl;

    /** 工作线程的线程标识符
    */
    int32_t m_nThreadIdentifier;

    /** 图标大小
    */
    const int32_t m_nIconSize;

    /** 目录树的KEY值
    */
    size_t m_folderKey;

    /** 目录树（左侧树显示）
    */
    std::map<size_t, FolderStatus*> m_folderMap;

    /** 用于显示关联的数据的回调函数
    */
    std::vector<ShowFolderContentsEvent> m_callbackList;
};

}

#endif // UI_CONTROL_DIRECTORYTREE_H_
