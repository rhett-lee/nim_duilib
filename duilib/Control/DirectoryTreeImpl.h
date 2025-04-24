#ifndef UI_CONTROL_DIRECTORYTREE_IMPL_H_
#define UI_CONTROL_DIRECTORYTREE_IMPL_H_

#include "DirectoryTree.h"

namespace ui
{
class DirectoryTree;

/** 文件系统的目录树结构枚举的实现接口
*/
class DirectoryTreeImpl
{   
public:
    explicit DirectoryTreeImpl(DirectoryTree* pTree);
    DirectoryTreeImpl(const DirectoryTreeImpl& r) = delete;
    DirectoryTreeImpl& operator=(const DirectoryTreeImpl& r) = delete;
    ~DirectoryTreeImpl();

public:
    /** 获取虚拟目录的真实路径/显示名称/图标(图标大小：16*16)
    * @param [in] type 虚拟目录的类型
    * @param [out] filePath 虚拟目录的真实路径
    * @param [out] displayName 虚拟目录的显示名称    
    * @param [out] nIconID 虚拟目录的关联图标（GlobalManager::Instance().Icon().AddIcon的返回值，图标需要实现类添加到管理器）
    * @return 成功返回true，失败返回false
    */
    bool GetVirtualDirectoryInfo(VirtualDirectoryType type, FilePath& filePath, DString& displayName, uint32_t& nIconID);

    /** 显示所有根目录下的路径节点列表（Windows下返回的是所有磁盘节点数据）
    * @param [in] bLargeIcon 是否获取大图标
    * @param [out] pathInfoList 返回的是所有磁盘节点数据(图标大小：16*16)
    */
    void GetRootPathInfoList(bool bLargeIcon, std::vector<DirectoryTree::PathInfo>& pathInfoList);

    /** 获取指定路径下的子目录和文件列表
    * @param [in] path 目录
    * @param [in] weakFlag 取消标记，关联的控件已经失效则表示操作已经取消
    * @param [in] bLargeIcon 是否为大图标，大图标为32*32，小图标为16*16
    * @param [out] folderList 返回path目录中的所有子目录列表
    * @param [out] fileList 返回path目录中的所有文件列表
    */
    void GetFolderContents(const FilePath& path,
                           const std::weak_ptr<WeakFlag>& weakFlag,
                           bool bLargeIcon,
                           std::vector<DirectoryTree::PathInfo>& folderList,
                           std::vector<DirectoryTree::PathInfo>* fileList);

    /** 获取"计算机"中的磁盘列表（Windows平台获取驱动器列表，其他平台获取Mount的节点列表）
    * @param [in] weakFlag 取消标记，关联的控件已经失效则表示操作已经取消
    * @param [in] bLargeIcon 是否为大图标，大图标为32*32，小图标为16*16
    * @param [out] diskInfoList 返回磁盘列表
    */
    void GetDiskInfoList(const std::weak_ptr<WeakFlag>& weakFlag,
                         bool bLargeIcon,
                         std::vector<DirectoryTree::DiskInfo>& diskInfoList);

    /** 是否需要显示此目录（根据属性设置判断）
    */
    bool NeedShowDirPath(const FilePath& path) const;

    /** 获取计算机的图标ID
    */
    uint32_t GetMyComputerIconID() const;

private:
    /** 目录树UI接口
    */
    DirectoryTree* m_pTree;

private:
    /** 私有实现数据
    */
    struct TImpl;
    TImpl* m_impl;
};

}

#endif // UI_CONTROL_DIRECTORYTREE_IMPL_H_
