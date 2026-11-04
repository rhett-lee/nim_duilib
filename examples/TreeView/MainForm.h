#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

#include <stack>
#include <unordered_map>

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

    /** 窗口级按键消息：处理快捷键（Enter/Backspace/F5/Ctrl+A/Ctrl+F/Esc）
    */
    virtual LRESULT OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 触发树节点点击事件, 选择对应的目录
     */
    void SelectSubPath(const ui::FilePath& filePath);

    /** 打开文件或目录：目录则进入其中，文件则用系统默认程序打开
    * @param [in] filePath 需要打开的文件或目录
    */
    void OpenPath(const ui::FilePath& filePath);

    /** 显示视图区域的右键菜单（由各视图在收到右键事件时调用）
    * @param [in] pView 接收右键事件的视图控件
    * @param [in] targetPath 右键选中的文件或目录，为空表示右键点在空白区域
    */
    void ShowViewContextMenu(ui::Control* pView, const ui::FilePath& targetPath);

    /** 列表的排序列发生变化（点击表头排序时由视图调用），同步更新状态栏
    */
    void OnColumnSorted();

private:
    /** 已获取指定目录的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] currentPath 当前显示内容所在路径
    * @param [in] folderList 返回path目录中的所有子目录列表
    * @param [in] fileList 返回path目录中的所有文件列表
    */
    void OnShowFolderContents(ui::TreeNode* pTreeNode, const ui::FilePath& currentPath,
                              const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList,
                              const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList);

    /** 显示"计算机"节点的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] diskInfoList 所有磁盘的信息列表
    */
    void OnShowMyComputerContents(ui::TreeNode* pTreeNode,
                                  const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList);

    /** 地址栏中的路径发生变化
    */
    bool OnAddressBarPathChanged(const ui::EventArgs& msg);

    /** 地址栏中的点击了子路径按钮
    */
    bool OnAddressBarPathClick(const ui::EventArgs& msg);

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

    /** 更新状态栏显示（当前路径、项目统计、视图与排序方式）
    */
    void UpdateStatusBar();

    /** 用文件列表刷新右侧的文件显示区域
    * @param [in] currentPath 当前显示内容所在路径
    * @param [in] pathList 需要显示的文件和文件夹列表
    */
    void ShowFileList(const ui::FilePath& currentPath, const std::vector<ui::DirectoryTree::PathInfo>& pathList);

    /** 搜索框中的文本发生变化
    */
    bool OnSearchTextChanged(const ui::EventArgs& msg);

    /** 应用当前的搜索条件（重新加载当前目录的内容）
    */
    void ApplySearchFilter();

    /** 清除搜索条件（切换目录时调用）
    */
    void ClearSearchText();

    /** 获取当前数据视图的名称（用于状态栏显示）
    */
    DString GetCurrentViewName() const;

    /** 获取当前排序方式的显示字符串（用于状态栏显示）
    */
    DString GetCurrentSortInfo() const;

    /** 打开当前视图中选择的项目
    */
    void OpenSelectedItem();

    /** 选择当前视图中的所有项目
    */
    void SelectAllItems();

    /** 获取当前视图中选择的路径（无选择时返回空）
    */
    ui::FilePath GetSelectedPath() const;

private:
    /** 设置当前选择的树节点
    */
    void SetShowTreeNode(ui::TreeNode* pTreeNode);

    /** 显示地址栏上的地址
    */
    bool OnShowAddressPath(const DString& newFilePath);

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
    ui::AddressBar* m_pAddressBar;

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

    /** 保存每个路径下选择的文件或者目录，在前进后退时保持原选择，改善体验
    */
    std::unordered_map<ui::FilePath, ui::FilePath> m_selectedPathInfo;

    /** 是否可以添加到前进后退列表
    */
    bool m_bCanAddBackForward;

    /** 切换列表类型
    */
    ui::ButtonHBox* m_pBtnViewListType;

    /** 切换排序模式
    */
    ui::ButtonHBox* m_pBtnViewSort;

private:
    /** 搜索框
    */
    ui::SearchBox* m_pSearchBox;

    /** 状态栏：当前路径
    */
    ui::Label* m_pStatusPath;

    /** 状态栏：项目统计信息
    */
    ui::Label* m_pStatusInfo;

    /** 状态栏：视图与排序方式
    */
    ui::Label* m_pStatusMode;

    /** 当前搜索关键字（已转换为小写，为空表示未搜索）
    */
    DString m_searchText;

    /** 当前显示内容所在路径
    */
    ui::FilePath m_currentPath;

    /** 当前是否为"计算机"视图
    */
    bool m_bComputerView;

    /** "计算机"视图中的磁盘总数
    */
    int32_t m_nDiskCount;

    /** 当前目录中的文件夹总数
    */
    int32_t m_nFolderCount;

    /** 当前目录中的文件总数
    */
    int32_t m_nFileCount;

    /** 当前目录中的项目总数（未过滤）
    */
    int32_t m_nTotalItemCount;

    /** 当前显示区域的项目总数（过滤后）
    */
    int32_t m_nShowItemCount;
};

#endif //EXAMPLES_MAIN_FORM_H_
