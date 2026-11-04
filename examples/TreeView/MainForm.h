#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

// 示例
#include "DirectoryWatcher.h"

#include <stack>
#include <unordered_map>
#include <map>
#include <string>

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

    /** 窗口级按键消息：处理快捷键
     * Enter/Backspace/F5/Ctrl+A/Ctrl+F/Ctrl+C/Esc/Alt+Left/Alt+Right/Alt+Up
     */
    virtual LRESULT OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口关闭之前调用（此时控件仍然有效）：保存用户设置，并停止目录监听
     */
    virtual void OnPreCloseWindow() override;

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

    /** 视图中的选择项发生变化（由各视图的选择事件调用），同步更新状态栏
    */
    void OnViewSelectionChanged();

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
    void SwitchListType(const ui::UiPoint& point, ui::Control* pRelatedControl);

    /** 切换排序方式
    */
    void SwitchSortMode(const ui::UiPoint& point, ui::Control* pRelatedControl);

    /** 更新界面状态
    */
    void UpdateCommandUI();

    /** 更新状态栏显示（当前路径、项目统计、视图与排序方式）
    */
    void UpdateStatusBar();

    /** 用文件列表刷新右侧的文件显示区域
    * @param [in] currentPath 当前显示内容所在路径
    * @param [in] pathList 需要显示的文件和文件夹列表（调用后不再使用，内部会移动数据）
    */
    void ShowFileList(const ui::FilePath& currentPath, std::vector<ui::DirectoryTree::PathInfo> pathList);

    /** 显示空视图（搜索条件没有匹配到任何项目时，提示用户）
    */
    void ShowEmptyView();

    /** 搜索框中的文本发生变化
    */
    bool OnSearchTextChanged(const ui::EventArgs& msg);

    /** 根据当前搜索文本，更新搜索关键字列表（支持空格分隔的多个关键字）
    */
    void UpdateSearchKeywords();

    /** 判断路径信息是否匹配当前的搜索条件
    */
    bool MatchSearchText(const ui::DirectoryTree::PathInfo& pathInfo) const;

    /** 判断磁盘信息是否匹配当前的搜索条件
    */
    bool MatchDiskSearchText(const ui::DirectoryTree::DiskInfo& diskInfo) const;

    /** 应用当前的搜索条件（按缓存的目录内容过滤，不重新枚举目录）
    */
    void ApplySearchFilter();

    /** 显示缓存的目录内容：按最新的搜索条件过滤后，刷新右侧文件区域和状态栏
    * @param [in] pTreeNode 需要显示的树节点
    */
    void ShowCachedContents(ui::TreeNode* pTreeNode);

    /** 清除缓存的目录内容（释放缓存的图标资源）
    */
    void ClearCachedContents();

    /** 清除搜索条件（切换目录时调用）
    */
    void ClearSearchText();

    /** 清除搜索条件，但不重新加载当前目录的内容
     * （调用方正在展示新目录的内容时使用，避免重复刷新旧目录）
     */
    void ResetSearchText();

    /** 获取当前数据视图的名称（用于状态栏显示）
    */
    DString GetCurrentViewName() const;

    /** 获取当前排序方式的显示字符串（用于状态栏显示）
    */
    DString GetCurrentSortInfo() const;

    /** 打开当前视图中选择的项目
    */
    void OpenSelectedItem();

    /** 打开多个文件或目录（目录只进入第一个，文件全部打开）
    */
    void OpenPaths(const std::vector<ui::FilePath>& paths);

    /** 选择当前视图中的所有项目
    */
    void SelectAllItems();

    /** 获取当前视图中选择的路径（无选择时返回空）
    */
    ui::FilePath GetSelectedPath() const;

    /** 获取当前视图中选择的所有路径
    */
    void GetSelectedPaths(std::vector<ui::FilePath>& paths) const;

    /** 将路径列表复制到剪贴板（每行一个完整路径）
    * @param [in] paths 需要复制的路径列表
    */
    void CopyPathsToClipboard(const std::vector<ui::FilePath>& paths) const;

    /** 获取当前视图选择的项目统计信息
    * @param [out] nSelectedCount 选中的项目数
    * @param [out] nSelectedFileSize 选中的文件总大小（不包含文件夹）
    */
    void GetSelectedItemInfo(int32_t& nSelectedCount, uint64_t& nSelectedFileSize) const;

    /** 文件大小转换为显示字符串（与文件列表中的显示格式一致）
    */
    DString FormatItemSize(uint64_t nFileSize) const;

private:
    /** 设置当前选择的树节点
    */
    void SetShowTreeNode(ui::TreeNode* pTreeNode);

    /** 显示地址栏上的地址
    */
    bool OnShowAddressPath(const DString& newFilePath);

private:
    /** 开始监听指定目录的内容变化（当前平台不支持目录监听时不做任何处理）
    */
    void StartDirectoryWatcher(const ui::FilePath& directory);

    /** 停止监听目录的内容变化
    */
    void StopDirectoryWatcher();

    /** 当前目录的内容发生了变化（在UI线程中回调）：自动刷新当前目录
    */
    void OnDirectoryChanged();

private:
    /** 用户设置文件的路径（保存在程序目录下的user子目录中）
    */
    ui::FilePath GetSettingsFilePath() const;

    /** 读取并应用用户设置（视图模式、排序列、列宽、树宽）
    */
    void LoadSettings();

    /** 保存用户设置（在窗口关闭之前调用）
    */
    void SaveSettings() const;

    /** 恢复列表各列的宽度
    * @param [in] pListCtrl 列表控件
    * @param [in] settings 设置项（键为"前缀+列序号"）
    * @param [in] keyPrefix 设置项的键前缀
    */
    void RestoreColumnWidths(ui::ListCtrl* pListCtrl, const std::map<std::string, std::string>& settings,
                             const std::string& keyPrefix);

    /** 保存列表各列的宽度
    * @param [in] pListCtrl 列表控件
    * @param [in,out] settingsText 设置文本（每行一个"键=值"）
    * @param [in] keyPrefix 设置项的键前缀
    */
    void SaveColumnWidths(ui::ListCtrl* pListCtrl, std::string& settingsText, const std::string& keyPrefix) const;

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

    /** 空视图（搜索没有匹配到任何项目时显示）中的提示文字
    */
    ui::Label* m_pEmptyTip;

    /** 当前搜索关键字（已转换为小写，为空表示未搜索）
    */
    DString m_searchText;

    /** 搜索关键字列表（支持空格分隔的多个关键字，每个关键字都已转换为小写）
    */
    std::vector<DString> m_searchKeywords;

    /** 缓存的目录内容：目录中的子目录列表（图标资源所有权归本对象，显示列表只使用共享图标）
    */
    std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>> m_cachedFolderList;

    /** 缓存的目录内容：目录中的文件列表（图标资源所有权归本对象，显示列表只使用共享图标）
    */
    std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>> m_cachedFileList;

    /** 缓存内容对应的路径（为空表示没有可用的缓存）
    */
    ui::FilePath m_cachedPath;

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

private:
    /** 开始加载目录内容：在状态栏显示"正在加载"提示
    * （加载完成时调用EndLoading隐藏该提示）
    */
    void BeginLoading();

    /** 结束加载目录内容：隐藏状态栏中的"正在加载"提示
    */
    void EndLoading();

    /** 加载提示的显示超时（异常情况下自动隐藏，避免提示一直显示）
    */
    void OnLoadingTimeout(uint32_t nSerial);

    /** 设置目录树和文件列表的多选状态（由"多选"复选框控制）
    * @param [in] bMultiSelect true表示支持多选，false表示单选
    */
    void SetMultiSelect(bool bMultiSelect);

    /** "多选"复选框的选择状态发生变化
    */
    bool OnMultiSelectChanged(const ui::EventArgs& msg);

private:
    /** 状态栏：正在加载提示
    */
    ui::Label* m_pStatusLoading;

    /** 是否正在加载目录内容
    */
    bool m_bLoading;

    /** 加载请求序号（用于区分过期的超时回调）
    */
    uint32_t m_nLoadingSerial;

    /** 加载提示超时定时器的ID
    */
    size_t m_nLoadingTimerId;

    /** 加载提示超时定时器的弱回调标志（窗口销毁时自动失效）
    */
    ui::WeakCallbackFlag m_loadingTimerFlag;

private:
    /** 当前目录的内容变化监听器（仅Windows平台生效，其他平台自动降级为手动刷新）
    */
    DirectoryWatcher m_directoryWatcher;
};

#endif //EXAMPLES_MAIN_FORM_H_
