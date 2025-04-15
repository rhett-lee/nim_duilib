#ifndef UI_CONTROL_DIRECTORYTREE_H_
#define UI_CONTROL_DIRECTORYTREE_H_

#include "duilib/Control/TreeView.h"

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

/** 文件系统的目录树结构
*/
class UILIB_API DirectoryTree: public TreeView
{
    typedef TreeView BaseClass;
public:
    explicit DirectoryTree(Window* pWindow);
    DirectoryTree(const DirectoryTree& r) = delete;
    DirectoryTree& operator=(const DirectoryTree& r) = delete;
    virtual ~DirectoryTree() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

public:
    /** 显示虚拟目录
    * @param [in] type 虚拟目录的类型
    * @param [in] displayName 虚拟目录的显示名称
    * @param [in] bDisplayNameIsID 虚拟目录的显示名称是否为语言ID，以支持多语言
    */
    TreeNode* ShowVirtualDirectoryNode(VirtualDirectoryType type, const DString& displayName, bool bDisplayNameIsID = false);

    /** 显示所有磁盘节点, 返回第一个新节点接口
    */
    TreeNode* ShowAllDiskNode();

    /** 在指定的节点前插入一个横向分割线
    * @param [in] pNode 节点接口
    * @param [in] lineClassName 横向分割线的Class名称（可选）
    */
    bool InsertLineBeforeNode(TreeNode* pNode, const DString& lineClassName = _T(""));
};

}

#endif // UI_CONTROL_DIRECTORYTREE_H_
