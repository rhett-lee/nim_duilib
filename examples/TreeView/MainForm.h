#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

#include <stack>
#include <map>

class ComputerView;
class SimpleFileView;
class ExplorerView;

class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
    virtual ~MainForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 触发树节点点击事件, 选择对应的目录
     */
    void SelectSubPath(const ui::FilePath& filePath);

private:
    /** 已获取指定目录的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    * @param [in] folderList 返回path目录中的所有子目录列表
    * @param [in] fileList 返回path目录中的所有文件列表
    */
    void OnShowFolderContents(ui::TreeNode* pTreeNode, const ui::FilePath& path,
                              const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList,
                              const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList);

    /** 显示"计算机"节点的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] diskInfoList 所有磁盘的信息列表
    */
    void OnShowMyComputerContents(ui::TreeNode* pTreeNode,
                                  const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList);

    /** 在地址栏中输入回车
    */
    bool OnAddressBarReturn(const ui::EventArgs& msg);

    /** 刷新
    */
    void Refresh();

    /** 刷新左树完成后，更新文件显示区域
    */
    void OnRefresh();

    /** 向上
    */
    void ShowUp();

    /** 向后
    */
    void ShowBack();

    /** 向前
    */
    void ShowForward();

    /** 切换视图模式
    */
    void SwithListType(const ui::UiPoint& point, ui::Control* pRelatedControl);

    /** 切换排序方式
    */
    void SwithSortMode(const ui::UiPoint& point, ui::Control* pRelatedControl);

    /** 更新界面状态
    */
    void UpdateCommandUI();

private:
    /** 设置当前选择的树节点
    */
    void SetShowTreeNode(ui::TreeNode* pTreeNode);

private:
    /** TabBox的视图类型(枚举值与XML中定义的顺序相同)
    */
    enum TabBoxViewType
    {
        kFileView       = 0,    //文件列表视图
        kExplorerView   = 1,    //文件浏览器视图
        kComputerView   = 2,    //计算机视图
        kErrorView      = 3     //出错视图
    };

    //数据视图类型
    enum DataViewType
    {
        kIconViewBig    = 0,    //图标视图(大图标)
        kIconViewMedium = 1,    //图标视图(中图标)
        kIconViewSmall  = 2,    //图标视图(小图标)
        kListViewBig    = 3,    //列表视图(大图标)
        kListViewMedium = 4,    //列表视图(中图标)
        kListViewSmall  = 5,    //列表视图(小图标)
        kReprortView    = 6,    //详细信息视图
        kPictureView    = 7     //图片列表视图
    };

    /** 切换TabBox的视图
    */
    void SwitchToTabBoxViewType(TabBoxViewType tabBoxViewType);

    /** 切换
    */
    void SwitchToDataViewType(DataViewType dataViewType);

    /** 获取当前的数据视图类型
    */
    DataViewType GetDataViewType() const;

private:
    /** 左侧树节点的接口
    */
    ui::DirectoryTree* m_pTree;

    /** 当前路径显示
    */
    ui::RichEdit* m_pAddressBar;

    /** TabBox容器
    */
    ui::TabBox* m_pTabBox;

private:
    /** 计算机视图
    */
    std::unique_ptr<ComputerView> m_pComputerView;

    /** 右侧文件列表视图：简单视图
    */
    std::unique_ptr<SimpleFileView> m_pSimpleFileView;

    /** 右侧文件列表视图：文件浏览器视图
    */
    std::unique_ptr<ExplorerView> m_pExplorerView;

    /** TabBox的视图类型(枚举值与XML中定义的顺序相同)
    */
    TabBoxViewType m_tabBoxViewType;

    /** 当前的数据视图类型
    */
    DataViewType m_dataViewType;

private:
    /** 当前显示的树节点
    */
    ui::TreeNode* m_pTreeNode;

    /** 当前选择树节点的父节点列表（用于刷新机制）
    */
    std::vector<ui::TreeNode*> m_parentTreeNodes;

private:
    /** 向上按钮
    */
    ui::Button* m_pBtnUp;

    /** 后退按钮
    */
    ui::Button* m_pBtnBack;

    /** 前进按钮
    */
    ui::Button* m_pBtnForward;

    /** 后退列表
    */
    std::stack<ui::TreeNode*> m_backStack;

    /** 前进列表
    */
    std::stack<ui::TreeNode*> m_forwardStack;

    /** 是否可以添加到前进后退列表
    */
    bool m_bCanAddBackForward;

    /** 切换列表类型
    */
    ui::ButtonHBox* m_pBtnViewListType;

    /** 切换排序模式
    */
    ui::ButtonHBox* m_pBtnViewSort;
};

#endif //EXAMPLES_MAIN_FORM_H_
