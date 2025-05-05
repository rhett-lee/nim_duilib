#ifndef UI_CONTROL_DIRECTORYTREE_H_
#define UI_CONTROL_DIRECTORYTREE_H_

#include "duilib/Control/TreeView.h"
#include "duilib/Utils/FileTime.h"
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
        DString m_typeName;         //文件类型
        uint64_t m_fileSize = 0;    //文件大小
        FileTime m_lastWriteTime;   //修改时间
        uint32_t m_nIconID = 0;     //关联图标ID（GlobalManager::Instance().Icon().AddIcon的返回值，图标需要实现类添加到管理器）
        bool m_bIconShared = false; //该图标ID关联的图标是否为共享图标（共享图标不允许释放）
        bool m_bFolder = true;      //是否为目录
    };

    /** 回调函数的原型: 用于显示文件夹内容关联的数据
    */
    typedef std::function<void (ui::TreeNode* pTreeNode, const ui::FilePath& path,
                                const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList,
                                const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList)> ShowFolderContentsEvent;

#ifdef DUILIB_BUILD_FOR_WIN
    /** 磁盘属性信息
    */
    struct DiskInfo
    {
        DString m_displayName;  //显示名称
        uint32_t m_nIconID = 0; //关联图标ID（GlobalManager::Instance().Icon().AddIcon的返回值，图标需要实现类添加到管理器）
        bool m_bIconShared = false; //该图标ID关联的图标是否为共享图标（共享图标不允许释放）
        FilePath m_filePath;        //对应的路径

        DString m_volumeName;   //分区卷标
        DString m_volumeType;   //分区类型，如"本地磁盘"
        DString m_fileSystem;   //文件系统类型，如"NTFS"

        uint64_t m_totalBytes = 0; //总空间大小（字节）
        uint64_t m_freeBytes = 0;  //可用空间大小（字节）
    };
#else
    //其他平台
    /** 设备类型枚举
    */
    enum class DeviceType
    {
        UNKNOWN,    // 未知设备
        HDD,        // 机械硬盘 (如/dev/sdX)
        SSD,        // SATA固态硬盘
        NVME,       // NVMe协议固态硬盘 (如/dev/nvmeXn1)
        USB,        // USB存储设备
        SD_CARD,    // SD卡
        CDROM,      // CDROM DVDROM
        LOOP,       // LOOP 虚拟设备
        VIRT_DISK,  // 虚拟设备
        RAMDISK,    // 内存盘 (如/dev/ramX)
        NFS,        // NFS
        SHARE       //共享文件夹
    };

    /** 磁盘属性信息
    */
    struct DiskInfo
    {
        DString m_displayName;      //显示名称
        uint32_t m_nIconID = 0;     //关联图标ID（GlobalManager::Instance().Icon().AddIcon的返回值，图标需要实现类添加到管理器）
        bool m_bIconShared = false; //该图标ID关联的图标是否为共享图标（共享图标不允许释放）
        FilePath m_filePath;        //对应的路径

        DString m_volumeName;   //文件系统，如"/dev/sda2"
        DeviceType m_deviceType = DeviceType::UNKNOWN;   //设备类型，参见枚举值
        DString m_fileSystem;   //文件系统类型，如"ext3"
        DString m_mountOn;      //挂载点，如"/"

        uint64_t m_totalBytes = 0; //总空间大小（字节）
        uint64_t m_freeBytes = 0;  //可用空间大小（字节）
    };
#endif

    /** 回调函数的原型: 用于显示计算机视图的数据
    */
    typedef std::function<void (ui::TreeNode* pTreeNode, 
                                const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList)> ShowMyComputerContentsEvent;

public:
    explicit DirectoryTree(Window* pWindow);
    DirectoryTree(const DirectoryTree& r) = delete;
    DirectoryTree& operator=(const DirectoryTree& r) = delete;
    virtual ~DirectoryTree() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 清空路径列表, 并按需释放图标资源
    * @param [in,out] pathList 输入文件列表，输出时清空
    */
    static void ClearPathInfoList(std::vector<PathInfo>& pathList);

    /** 清空磁盘信息列表, 并按需释放图标资源
    * @param [in,out] pathList 输入文件列表，输出时清空
    */
    static void ClearDiskInfoList(std::vector<DirectoryTree::DiskInfo>& diskList);

public:
    /** 设置工作子线程的线程标识符（枚举目录是在子线程中完成，避免导致界面无响应）
    * @param [in] nThreadIdentifier 工作子线程的线程标识符
    */
    void SetThreadIdentifier(int32_t nThreadIdentifier);

    /** 显示虚拟目录(同步完成)
    * @param [in] type 虚拟目录的类型
    * @param [in] displayName 虚拟目录的显示名称
    * @param [in] bDisplayNameIsID 虚拟目录的显示名称是否为语言ID，以支持多语言
    */
    TreeNode* ShowVirtualDirectoryNode(VirtualDirectoryType type, const DString& displayName, bool bDisplayNameIsID = false);

    /** 显示所有磁盘节点, 返回第一个新节点接口(同步完成)
    * @param [in] computerName 计算机节点的显示名称
    * @param [in] fileSystemName 文件系统的显示名称
    */
    TreeNode* ShowAllDiskNodes(const DString& computerName, const DString& fileSystemName);

    /** 在指定的节点前插入一个横向分割线(同步完成)
    * @param [in] pNode 节点接口
    * @param [in] lineClassName 横向分割线的Class名称（可选）
    */
    bool InsertLineBeforeNode(TreeNode* pNode, const DString& lineClassName = _T(""));

    /** 在树中添加一个节点, 返回新添加的节点接口(同步完成)
    * @param [in] pParentTreeNode 父节点的接口，如果为nullptr则在根节点下添加新的节点
    * @param [in] displayName 虚拟目录的显示名称
    * @param [in] bDisplayNameIsID 虚拟目录的显示名称是否为语言ID，以支持多语言
    * @param [in] path 对应路径
    * @param [in] isFolder true表示为文件夹，否则为普通文件
    * @param [in] nIconID 图标ID(在GlobalManager::Instance().Icon()中管理)，如果为0，表示无关联图标
    * @param [in] bIconShared 该图标ID关联的图标是否为共享图标（共享图标不允许释放）
    * @param [in] bVirtualNode true表示该节点是虚拟节点，否则为普通路径节点
    */
    TreeNode* InsertTreeNode(TreeNode* pParentTreeNode,
                             const DString& displayName,
                             bool bDisplayNameIsID,
                             const ui::FilePath& path,
                             bool isFolder,
                             bool bVirtualNode,
                             uint32_t nIconID,
                             bool bIconShared);

    /** 获取一个路径对应的树节点(同步完成)
    * @param [in] filePath 需要查找的路径
    */
    TreeNode* FindPathTreeNode(FilePath filePath) const;

    /** 获取一个树节点对应的路径(同步完成)
    * @param [in] pTreeNode 树的节点
    */
    FilePath FindTreeNodePath(TreeNode* pTreeNode);

    /** 判断一个树节点是不是"计算机"节点，这个节点需要特殊处理
    * @param [in] pTreeNode 当前的节点
    */
    bool IsMyComputerNode(TreeNode* pTreeNode) const;

public:
    /** 设置用于显示关联的数据的回调函数
    * @param [in] callback 回调函数
    */
    void AttachShowFolderContents(ShowFolderContentsEvent callback);

    /** 设置用于显示"计算机"节点信息视图的回调函数
    */
    void AttachShowMyComputerContents(ShowMyComputerContentsEvent callback);

public:
    /** 选择一个树节点（如果父节点未展开，则级联展开）
    * @param [in] pTreeNode 树的节点
    */
    virtual bool SelectTreeNode(TreeNode* pTreeNode) override;

    /** 展开树的节点并选择其子目录(异步完成)
    * @param [in] pTreeNode 树的节点
    * @param [in] subPath 子目录或者多级子目录，该路径是pTreeNode对应路径的子目录
    * @param [in] finishCallback 刷新完成后的回调函数
    */
    bool SelectSubPath(TreeNode* pTreeNode, FilePath subPath, StdClosure finishCallback);

    /** 选择一个路径(逐级展开目录，并选择最终的目录，确保可见)(异步完成)
    * @param [in] filePath 需要选择的路径
    * @param [in] finishCallback 刷新完成后的回调函数
    */
    bool SelectPath(FilePath filePath, StdClosure finishCallback);

    /** 刷新，保持树结构与文件系统同步(异步完成)
    * @param [in] finishCallback 刷新完成后的回调函数
    */
    bool RefreshTree(StdClosure finishCallback);

    /** 刷新树节点，保持树结构与文件系统同步(异步完成)
    * @param [in] pTreeNode 树的节点
    * @param [in] finishCallback 刷新完成后的回调函数
    */
    bool RefreshTreeNode(TreeNode* pTreeNode, StdClosure finishCallback);

    /** 刷新树节点，保持树结构与文件系统同步(异步完成)
    * @param [in] treeNodes 树的节点列表
    * @param [in] finishCallback 刷新完成后的回调函数
    */
    bool RefreshTreeNodes(const std::vector<TreeNode*>& treeNodes, StdClosure finishCallback);

    /** 刷新指定节点对应目录的内容(异步完成)
    * @param [in] pTreeNode 树的节点
    * @param [in] finishCallback 刷新完成后的回调函数
    */
    bool RefreshFolderContents(TreeNode* pTreeNode, StdClosure finishCallback);

    /** 设置默认的刷新完成事件回调函数
    * @param [in] finishCallback 刷新完成后的回调函数
    */
    void SetRefreshFinishCallback(StdClosure finishCallback);

public:
    /** 设置小图标大小，仅对新添加的节点有效
    */
    void SetSmallIconSize(int32_t nIconSize);

    /** 获取小图标大小(宽度和高度相同)
    */
    int32_t GetSmallIconSize() const;

    /** 设置大图标大小，仅对新添加的节点有效
    */
    void SetLargeIconSize(int32_t nIconSize);

    /** 获取大图标大小(宽度和高度相同)
    */
    int32_t GetLargeIconSize() const;

    /** 设置是否显示隐藏文件
    */
    void SetShowHidenFiles(bool bShowHidenFiles);

    /** 获取是否显示隐藏文件
    */
    bool IsShowHidenFiles() const;

    /** 设置是否显示系统文件
    */
    void SetShowSystemFiles(bool bShowSystemFiles);

    /** 获取是否显示系统文件
    */
    bool IsShowSystemFiles() const;

private:
    /** 树节点展开事件
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnTreeNodeExpand(const EventArgs& args);

    /** 树节点的点击事件
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnTreeNodeClick(const EventArgs& args);

    /** 树节点销毁事件
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnTreeNodeDestroy(const EventArgs& args);

    /** 显示指定目录的子目录(异步完成)
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    */
    void ShowSubFolders(TreeNode* pTreeNode, const FilePath& path, StdClosure finishCallback);

    /** 显示指定目录的内容(异步完成)
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    */
    void ShowFolderContents(TreeNode* pTreeNode, const FilePath& path, StdClosure finishCallback);

    /** 显示"计算机"节点的内容
    */
    void ShowMyComputerContents(TreeNode* pTreeNode, StdClosure finishCallback);

    /** 判断两个树节点是否为父子关系
    * @param [in] pTreeNode 父节点
    * @param [in] pChildTreeNode 子节点
    */
    bool IsChildTreeNode(TreeNode* pTreeNode, TreeNode* pChildTreeNode) const;

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
    bool OnShowSubFolders(TreeNode* pTreeNode, const FilePath& path, const PathInfoListPtr& folderList);

    /** 显示指定目录的子目录（多级子目录）
    * @param [in] pTreeNode 当前的节点
    * @param [in] filePathList 路径列表
    * @param [in] folderList 返回每个目录中的所有子目录列表
    */
    bool OnShowSubFoldersEx(TreeNode* pTreeNode,
                            const std::vector<FilePath>& filePathList,
                            const std::vector<PathInfoListPtr>& folderListArray);

    /** 已获取指定目录的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    * @param [in] folderList path目录中的所有子目录列表
    * @param [in] fileList path目录中的所有文件列表
    */
    bool OnShowFolderContents(ui::TreeNode* pTreeNode, const ui::FilePath& path,
                              const PathInfoListPtr& folderList,
                              const PathInfoListPtr& fileList);

    /** 显示"计算机"节点的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] diskInfoList 所有磁盘的信息列表
    */
    bool OnShowMyComputerContents(ui::TreeNode* pTreeNode,
                                  const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList);

    /** 在某个树节点下展开子目录，并选择最后一级目录
    * @param [in] pTreeNode 当前的节点
    * @param [in] filePathList 需要展开的目录（这些目录尚未添加到树节点）
    * @param [in] finishCallback 完成回调函数
    */
    bool OnSelectSubPath(TreeNode* pTreeNode, std::vector<FilePath> filePathList, StdClosure finishCallback);

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
    /** 删除目录列表数据
    */
    void DeleteFolderStatus(FolderStatus* pFolderStatus);

    /** 根据Key获取目录结构数据
    */
    FolderStatus* GetFolderData(TreeNode* pTreeNode) const;

    /** 根据路径查找目录结构数据
    */
    FolderStatus* GetFolderData(FilePath filePath) const;

private:
    /** 刷新树节点的数据结构
    */
    struct RefreshNodeData
    {
        TreeNode* m_pTreeNode = nullptr;            //树节点
        std::weak_ptr<WeakFlag> m_weakFlag;         //树节点生命周期
        bool m_bContentLoaded = false;              //当前目录的子目录是否已经加载过
        FilePath m_dirPath;                         //树节点对应的目录
        size_t m_nParentIndex = 0;                  //父节点在容器中的索引号
        std::vector<FilePath> m_childPaths;         //子节点和子目录数据，用于比较是否有增加的目录

        //当前路径是否被删除
        bool m_bDeleted = false;

        //当前目录中新增加的路径列表
        std::vector<DirectoryTree::PathInfo> m_newFolderList;
    };

    /** 从界面中递归获取树节点数据，形成一个列表
    */
    void GetTreeNodeData(size_t nParentIndex, TreeNode* pTreeNode, std::vector<std::shared_ptr<RefreshNodeData>>& refreshData) const;

    /** 根据目录树读取文件系统的最新状态(标记已经删除的目录，添加新增的目录)
    */
    void RefreshPathInfo(std::vector<std::shared_ptr<RefreshNodeData>>& refreshData);

    /** 根据最新的状态，更新树的结构
    */
    void UpdateTreeNodeData(const std::vector<std::shared_ptr<RefreshNodeData>>& refreshData);

private:
    /** 枚举目录的内部实现（不同平台不同实现）
    */
    DirectoryTreeImpl* m_impl;

    /** 工作线程的线程标识符
    */
    int32_t m_nThreadIdentifier;

    /** 小图标的大小
    */
    int32_t m_nSmallIconSize;

    /** 大图标的大小
    */
    int32_t m_nLargeIconSize;

    /** 是否显示隐藏文件
    */
    bool m_bShowHidenFiles;

    /** 是否显示系统文件
    */
    bool m_bShowSystemFiles;

    /** 目录树的KEY值
    */
    size_t m_folderKey;

    /** 目录树（左侧树显示）
    */
    std::map<size_t, FolderStatus*> m_folderMap;

    /** 用于显示关联的文件夹数据的回调函数
    */
    std::vector<ShowFolderContentsEvent> m_callbackList;

    /** 用于显示关联的"计算机"数据的回调函数
    */
    std::vector<ShowMyComputerContentsEvent> m_myComputerCallbackList;

    /** 默认的刷新完成后的回调函数
    */
    StdClosure m_defaultRefreshFinishCallback;
};

}

#endif // UI_CONTROL_DIRECTORYTREE_H_
