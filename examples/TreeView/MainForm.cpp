#include "MainForm.h"
#include "MainThread.h"
#include "ComputerView.h"
#include "SimpleFileView.h"
#include "ExplorerView.h"

#include <algorithm>
#include <list>
#include <map>
#include <string>

#if defined (DUILIB_BUILD_FOR_WIN)
#include <windows.h>
#include <shellapi.h>
#endif

namespace
{
#if !defined (DUILIB_BUILD_FOR_WIN)
/** 将本地路径转换为 file:// URL（SDL_OpenURL 需要 URL 形式，而不是本地路径）
*/
DString MakeFileUrl(const DString& nativePath)
{
    DString url(_T("file://"));
    const auto utf8Path = ui::StringConvert::TToUTF8(nativePath);
    for (char ch : utf8Path) {
        const bool bKeep = ((ch >= 'a') && (ch <= 'z')) ||
                           ((ch >= 'A') && (ch <= 'Z')) ||
                           ((ch >= '0') && (ch <= '9')) ||
                           (ch == '/') || (ch == '-') || (ch == '_') || (ch == '.') || (ch == '~');
        if (bKeep) {
            url += ch;
        }
        else {
            url += ui::StringUtil::Printf(_T("%%%02X"), (int32_t)(unsigned char)ch);
        }
    }
    return url;
}
#endif

/** 在文件管理器中显示指定的文件或目录
*/
void RevealInFileManager(const ui::FilePath& filePath)
{
    if (filePath.IsEmpty()) {
        return;
    }
#if defined (DUILIB_BUILD_FOR_WIN)
    DString param(_T("/select,\""));
    param += filePath.ToString();
    param += _T("\"");
    ::ShellExecuteW(nullptr, L"open", L"explorer.exe", param.c_str(), nullptr, SW_SHOWNORMAL);
#else
    //其他平台（SDL）：打开文件所在的目录
    ui::FilePath parentPath = filePath.GetParentPath();
    if (parentPath.IsEmpty()) {
        parentPath = filePath;
    }
    ui::SystemUtil::OpenUrl(MakeFileUrl(parentPath.ToString()));
#endif
}

/** 判断控件是否位于指定父控件的子树中（含父控件自身）
*/
bool IsControlInSubTree(const ui::Control* pControl, const ui::Control* pParent)
{
    while (pControl != nullptr) {
        if (pControl == pParent) {
            return true;
        }
        pControl = pControl->GetParent();
    }
    return false;
}

/** 用系统默认程序打开文件或目录
*/
void OpenWithSystem(const ui::FilePath& filePath)
{
    if (filePath.IsEmpty()) {
        return;
    }
#if defined (DUILIB_BUILD_FOR_WIN)
    ui::SystemUtil::OpenUrl(filePath.ToString());
#else
    ui::SystemUtil::OpenUrl(MakeFileUrl(filePath.ToString()));
#endif
}

/** 搜索过滤的最小输入长度：输入长度达到该值后才开始过滤
* （避免输入第一个字符时就重建整个列表：单字符的匹配范围过大，过滤意义不大）
*/
const size_t kMinSearchTextLength = 2;

/** "正在加载"提示的超时时间（毫秒）：正常情况下目录加载完成后会隐藏该提示，
*  这里只用于异常情况下的保护（避免提示一直显示在状态栏）
*/
const uint32_t kLoadingTimeoutMs = 8000;

/** 用户设置文件中的键值对
*/
typedef std::map<std::string, std::string> SettingsMap;

/** 用户设置文本中的一行：键=值（以'#'或者';'开头的行为注释行）
*/
void ParseSettingsText(const std::string& text, SettingsMap& settings)
{
    settings.clear();
    std::list<std::string> lines = ui::StringUtil::Split(text, "\n");
    for (std::string& line : lines) {
        ui::StringUtil::Trim(line);
        if (line.empty() || (line[0] == '#') || (line[0] == ';')) {
            continue;
        }
        const size_t nPos = line.find('=');
        if (nPos == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, nPos);
        std::string value = line.substr(nPos + 1);
        ui::StringUtil::Trim(key);
        ui::StringUtil::Trim(value);
        if (!key.empty()) {
            settings[key] = value;
        }
    }
}

/** 读取用户设置文件（文本格式：每行一个"键=值"，UTF-8编码）
* @param [in] filePath 设置文件的路径
* @param [out] settings 读取到的设置项
*/
void ReadSettingsFile(const ui::FilePath& filePath, SettingsMap& settings)
{
    settings.clear();
    std::vector<uint8_t> fileData;
    if (!ui::FileUtil::ReadFileData(filePath, fileData) || fileData.empty()) {
        return;
    }
    std::string text(fileData.begin(), fileData.end());
    //去除UTF-8 BOM
    if ((text.size() >= 3) &&
        (static_cast<uint8_t>(text[0]) == 0xEF) &&
        (static_cast<uint8_t>(text[1]) == 0xBB) &&
        (static_cast<uint8_t>(text[2]) == 0xBF)) {
        text.erase(0, 3);
    }
    ParseSettingsText(text, settings);
}

/** 从用户设置中读取一个整数
* @param [in] settings 设置项
* @param [in] key 设置项的键
* @param [in] nDefault 设置项不存在时返回的默认值
*/
int32_t GetSettingInt(const SettingsMap& settings, const std::string& key, int32_t nDefault)
{
    auto iter = settings.find(key);
    if (iter == settings.end()) {
        return nDefault;
    }
    return ui::StringUtil::StringToInt32(iter->second.c_str());
}

/** 把整数写入用户设置文本（每行一个"键=值"）
* @param [in,out] settingsText 设置文本
* @param [in] key 设置项的键
* @param [in] nValue 设置项的值
*/
void AppendSettingInt(std::string& settingsText, const std::string& key, int32_t nValue)
{
    settingsText += key;
    settingsText += '=';
    settingsText += std::to_string(nValue);
    settingsText += '\n';
}

/** 菜单中“当前选中项”的勾选图标描述（颜色替换为主题强调色）
*/
DString GetMenuCheckImage()
{
    return _T("file='ui-item-symbolic.svg' width='16' height='16' svg_replace_colors='#0078D4|color_accent'");
}
}//namespace

MainForm::MainForm():
    m_pTree(nullptr),
    m_pAddressBar(nullptr),
    m_pBtnUp(nullptr),
    m_pBtnForward(nullptr),
    m_pBtnBack(nullptr),
    m_bCanAddBackForward(true),
    m_pTreeNode(nullptr),
    m_pTabBox(nullptr),
    m_pBtnViewListType(nullptr),
    m_pBtnViewSort(nullptr),
    m_pSearchBox(nullptr),
    m_pStatusPath(nullptr),
    m_pStatusInfo(nullptr),
    m_pStatusMode(nullptr),
    m_pEmptyTip(nullptr),
    m_bComputerView(false),
    m_nDiskCount(0),
    m_nFolderCount(0),
    m_nFileCount(0),
    m_nTotalItemCount(0),
    m_nShowItemCount(0),
    m_dataViewType(DataViewType::kReprortView),
    m_tabBoxViewType(TabBoxViewType::kComputerView),
    m_pStatusLoading(nullptr),
    m_bLoading(false),
    m_nLoadingSerial(0),
    m_nLoadingTimerId(0)
{
}

MainForm::~MainForm()
{
    //停止"正在加载"提示的超时定时器
    m_loadingTimerFlag.Cancel();
    if (m_nLoadingTimerId != 0) {
        ui::GlobalManager::Instance().Timer().RemoveTimer(m_nLoadingTimerId);
        m_nLoadingTimerId = 0;
    }
    //停止目录监听（等待监听线程退出，避免回调在对象销毁后被执行）
    StopDirectoryWatcher();
    //释放缓存的目录内容（含图标资源）
    ClearCachedContents();
}

void MainForm::OnPreCloseWindow()
{
    //窗口关闭前保存用户设置（此时控件仍然有效）
    SaveSettings();
    //停止目录监听，等待监听线程退出
    StopDirectoryWatcher();
    ui::WindowImplBase::OnPreCloseWindow();
}

DString MainForm::GetSkinFolder()
{
    return _T("tree_view");
}

DString MainForm::GetSkinFile()
{
    return _T("tree_view.xml");
}

void MainForm::OnInitWindow()
{
    //绑定标题栏的窗口按钮与语言/主题切换按钮
    ui::WindowImplBase::OnInitWindow();

    m_pTree = dynamic_cast<ui::DirectoryTree*>(FindControl(_T("tree")));
    ASSERT(m_pTree != nullptr);
    if (m_pTree == nullptr) {
        return;
    }
    //树节点选中时立即更新地址栏（目录内容枚举完成前也不滞后）
    m_pTree->AttachEvent(ui::kEventSelect, UiBind(&MainForm::OnTreeNodeSelect, this, std::placeholders::_1), 0);
    m_pAddressBar = dynamic_cast<ui::AddressBar*>(FindControl(_T("file_path")));
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->AttachPathChanged(UiBind(&MainForm::OnAddressBarPathChanged, this, std::placeholders::_1));
        m_pAddressBar->AttachPathClick(UiBind(&MainForm::OnAddressBarPathClick, this, std::placeholders::_1));
        //预提交：先校验路径，校验失败时地址栏保持原路径（不会先显示错误路径再回滚）
        m_pAddressBar->AttachPathChanging([this](DString& newPath) {
            return OnAddressBarPathChanging(newPath);
            });
    }
    m_pTabBox = dynamic_cast<ui::TabBox*>(FindControl(_T("main_view_tab_box")));
    ui::ListCtrl* pComputerListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("computer_view")));
    m_pComputerView = std::make_unique<ComputerView>(this, pComputerListCtrl);
    ui::VirtualListBox* pListBox = dynamic_cast<ui::VirtualListBox*>(FindControl(_T("simple_file_view")));
    m_pSimpleFileView = std::make_unique<SimpleFileView>(this, pListBox);
    ui::ListCtrl* pExplorerListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("explorer_view")));
    m_pExplorerView = std::make_unique<ExplorerView>(this, pExplorerListCtrl);

    //搜索框：在当前目录中按名称过滤文件夹和文件
    m_pSearchBox = dynamic_cast<ui::SearchBox*>(FindControl(_T("search_box")));
    if (m_pSearchBox != nullptr) {
        m_pSearchBox->AttachEvent(ui::kEventTextChanged, UiBind(&MainForm::OnSearchTextChanged, this, std::placeholders::_1), 0);
    }

    //状态栏
    m_pStatusPath = dynamic_cast<ui::Label*>(FindControl(_T("status_path")));
    if (m_pStatusPath != nullptr) {
        //路径比较长时，鼠标悬停显示完整的路径（状态栏空间不足时）
        m_pStatusPath->SetAutoToolTip(true);
    }
    //"正在加载"提示（目录内容在后台线程中枚举期间显示）
    m_pStatusLoading = dynamic_cast<ui::Label*>(FindControl(_T("status_loading")));
    m_pStatusInfo = dynamic_cast<ui::Label*>(FindControl(_T("status_info")));
    m_pStatusMode = dynamic_cast<ui::Label*>(FindControl(_T("status_mode")));
    //空视图中的提示文字（搜索没有匹配到任何项目时显示）
    m_pEmptyTip = dynamic_cast<ui::Label*>(FindControl(_T("empty_tip")));

    //刷新按钮
    ui::Button* pRefreshBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_view_refresh")));
    if (pRefreshBtn != nullptr) {
        pRefreshBtn->AttachClick([this](const ui::EventArgs&) {
            Refresh();
            return true;
            });
    }

    //向上按钮
    m_pBtnUp = dynamic_cast<ui::Button*>(FindControl(_T("btn_view_up")));
    if (m_pBtnUp != nullptr) {
        m_pBtnUp->AttachClick([this](const ui::EventArgs&) {
            ShowUp();
            return true;
            });
    }

    //后退按钮
    m_pBtnBack = dynamic_cast<ui::Button*>(FindControl(_T("btn_view_left")));
    if (m_pBtnBack != nullptr) {
        m_pBtnBack->AttachClick([this](const ui::EventArgs&) {
            ShowBack();
            return true;
            });
    }

    //前进按钮
    m_pBtnForward = dynamic_cast<ui::Button*>(FindControl(_T("btn_view_right")));
    if (m_pBtnForward != nullptr) {
        m_pBtnForward->AttachClick([this](const ui::EventArgs&) {
            ShowForward();
            return true;
            });
    }

    //切换视图模式
    m_pBtnViewListType = dynamic_cast<ui::ButtonHBox*>(FindControl(_T("btn_view_list_type")));
    if (m_pBtnViewListType != nullptr) {
        m_pBtnViewListType->AttachClick([this](const ui::EventArgs& args) {
            ui::UiRect rect = args.GetSender()->GetPos();
            ui::UiPoint point;
            point.x = rect.left;
            point.y = rect.bottom;
            ClientToScreen(point);
            point.y += Dpi().GetScaleInt(4);
            SwitchListType(point, m_pBtnViewListType);
            return true;
            });
    }

    //切换排序模式
    m_pBtnViewSort = dynamic_cast<ui::ButtonHBox*>(FindControl(_T("btn_view_sort")));
    if (m_pBtnViewSort != nullptr) {
        m_pBtnViewSort->AttachClick([this](const ui::EventArgs& args) {
            ui::UiRect rect = args.GetSender()->GetPos();
            ui::UiPoint point;
            point.x = rect.left;
            point.y = rect.bottom;
            ClientToScreen(point);
            point.y += Dpi().GetScaleInt(4);
            SwitchSortMode(point, m_pBtnViewSort);
            return true;
            });
    }

    //"支持多选"复选框：仅控制左侧目录树的多选状态（右侧文件列表固定支持多选）
    ui::CheckBox* pTreeMultiSelectCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_tree_multi_select")));
    if (pTreeMultiSelectCheckBox != nullptr) {
        pTreeMultiSelectCheckBox->AttachSelect(UiBind(&MainForm::OnTreeMultiSelectChanged, this, std::placeholders::_1));
        pTreeMultiSelectCheckBox->AttachUnSelect(UiBind(&MainForm::OnTreeMultiSelectChanged, this, std::placeholders::_1));
        //按复选框的初始状态，同步目录树的多选设置
        SetTreeMultiSelect(pTreeMultiSelectCheckBox->IsSelected());
    }

    //恢复上次关闭时保存的用户设置（视图模式、排序列、列宽、树宽）
    LoadSettings();

    UpdateCommandUI();

    //挂载事件
    m_pTree->AttachShowMyComputerContents(ui::UiBind(&MainForm::OnShowMyComputerContents, this, std::placeholders::_1, std::placeholders::_2));
    m_pTree->AttachShowFolderContents(ui::UiBind(&MainForm::OnShowFolderContents, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    //点击目录树中的节点时，目录内容是在后台线程中枚举的：先显示"正在加载"提示
    //（节点的事件会冒泡到目录树控件，所以这里用AttachBubbledEvent监听）
    m_pTree->AttachBubbledEvent(ui::kEventClick, [this](const ui::EventArgs& args) {
        if (dynamic_cast<ui::TreeNode*>(args.GetSender()) != nullptr) {
            BeginLoading();
        }
        return true;
        }, 0);
    ui::StdClosure finishCallback = ToWeakCallback([this]() {
        OnRefresh();
        });
    m_pTree->SetRefreshFinishCallback(finishCallback);

    //显示虚拟路径
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kUserHome, _T("STRID_TREEVIEW_NODE_USER_HOME"), true);
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDesktop, _T("STRID_TREEVIEW_NODE_DESKTOP"), true);
    ui::TreeNode* pDocumentsNode = m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDocuments, _T("STRID_TREEVIEW_NODE_DOCUMENTS"), true);
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kPictures, _T("STRID_TREEVIEW_NODE_PICTURES"), true);
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kMusic, _T("STRID_TREEVIEW_NODE_MUSIC"), true);
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kVideos, _T("STRID_TREEVIEW_NODE_VIDEOS"), true);
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDownloads, _T("STRID_TREEVIEW_NODE_DOWNLOADS"), true);

    //显示磁盘
    ui::TreeNode* pComputerNode = m_pTree->ShowAllDiskNodes(_T("STRID_TREEVIEW_NODE_MYCOMPUTER"), _T("STRID_TREEVIEW_NODE_FILESYSTEM"), true);
    if (pComputerNode != nullptr) {
        //在磁盘前面，放一个横线分隔符
        m_pTree->InsertLineBeforeNode(pComputerNode);
    }

    //初始启动时，默认"计算机"视图
    if (pComputerNode != nullptr) {
        m_pTree->SelectTreeNode(pComputerNode);
    }
}

bool MainForm::OnLanguageChanged()
{
    const bool bRet = ui::WindowImplBase::OnLanguageChanged();
    //状态栏文本是代码拼接的，需要按当前语言重新生成
    UpdateStatusBar();
    //空视图的提示文本也是代码设置的
    if (m_pEmptyTip != nullptr) {
        m_pEmptyTip->SetText(ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_SEARCH_NO_MATCH")));
    }
    return bRet;
}

void MainForm::Refresh()
{
    if (m_pTree == nullptr) {
        return;
    }
    ui::StdClosure finishCallback = ToWeakCallback([this]() {
            OnRefresh();
        });
    BeginLoading();
    if (!m_pTree->RefreshTree(finishCallback)) {
        //没有需要刷新的节点：结束加载状态
        EndLoading();
    }
}

void MainForm::SetShowTreeNode(ui::TreeNode* pTreeNode)
{
    if (!m_pTree->IsValidTreeNode(pTreeNode)) {
        return;
    }
    if (m_bCanAddBackForward && m_pTree->IsValidTreeNode(m_pTreeNode) && (m_pTreeNode != pTreeNode)) {
        m_backStack.push(m_pTreeNode);
        m_forwardStack = std::stack<ui::TreeNode*>();
    }
    m_pTreeNode = pTreeNode;
    m_parentTreeNodes.clear();
    if (pTreeNode != nullptr) {
        ui::TreeNode* p = pTreeNode->GetParentNode();
        while (p != nullptr) {
            m_parentTreeNodes.push_back(p);
            p = p->GetParentNode();
            if (p == m_pTree->GetRootNode()) {
                break;
            }
        }
    }
    if (!m_parentTreeNodes.empty()) {
        std::reverse(m_parentTreeNodes.begin(), m_parentTreeNodes.end());
    }    
    m_bCanAddBackForward = true;
}

void MainForm::OnShowFolderContents(ui::TreeNode* pTreeNode, const ui::FilePath& currentPath,
                                    const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList,
                                    const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList)
{
    ui::GlobalManager::Instance().AssertUIThread();
    //目录内容已经加载完成（失败时也结束加载状态，避免提示一直显示）
    EndLoading();
    if ((pTreeNode == nullptr) || (m_pTree == nullptr) || !m_pTree->IsValidTreeNode(pTreeNode)) {
        if (folderList != nullptr) {
            ui::DirectoryTree::ClearPathInfoList(*folderList);
        }
        if (fileList != nullptr) {
            ui::DirectoryTree::ClearPathInfoList(*fileList);
        }
        return;
    }
    if (pTreeNode != m_pTreeNode) {
        //切换到其它目录时，清除搜索条件（这里不重新加载旧目录，避免重复刷新）
        ResetSearchText();
    }
    m_bComputerView = false;
    m_currentPath = currentPath;
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->SetAddressPath(currentPath.ToString());
    }

    //缓存本次枚举的目录内容：后续按搜索条件过滤时直接使用缓存，不再重新枚举目录
    //（缓存接管列表及图标资源的所有权；显示列表中只使用共享图标，由本对象负责释放）
    ClearCachedContents();
    m_cachedPath = currentPath;
    m_cachedFolderList = folderList;
    m_cachedFileList = fileList;

    //按搜索条件过滤后显示
    ShowCachedContents(pTreeNode);

    //监听当前目录的内容变化：外部增删文件后自动刷新（当前平台不支持目录监听时，该调用无效果）
    StartDirectoryWatcher(currentPath);
}

void MainForm::ShowCachedContents(ui::TreeNode* pTreeNode)
{
    if ((m_pTree == nullptr) || (pTreeNode == nullptr)) {
        return;
    }
    const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList = m_cachedFolderList;
    const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList = m_cachedFileList;

    //汇总文件夹和文件，并按搜索条件过滤
    m_nFolderCount = (folderList != nullptr) ? (int32_t)folderList->size() : 0;
    m_nFileCount = (fileList != nullptr) ? (int32_t)fileList->size() : 0;
    m_nTotalItemCount = m_nFolderCount + m_nFileCount;
    std::vector<ui::DirectoryTree::PathInfo> pathList;
    pathList.reserve((size_t)m_nTotalItemCount);
    const bool bFilter = !m_searchKeywords.empty();
    auto addItems = [this, &pathList, bFilter](const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& pathInfoList) {
        if (pathInfoList == nullptr) {
            return;
        }
        for (const ui::DirectoryTree::PathInfo& pathInfo : *pathInfoList) {
            if (bFilter && !MatchSearchText(pathInfo)) {
                //不匹配搜索条件（图标资源归缓存所有，这里不释放）
                continue;
            }
            ui::DirectoryTree::PathInfo item = pathInfo;
            //显示列表中的图标资源由目录内容缓存统一管理，视图不负责释放，避免重复释放
            item.m_bIconShared = true;
            pathList.push_back(item);
        }
    };
    addItems(folderList);
    addItems(fileList);
    m_nShowItemCount = (int32_t)pathList.size();

    if (!m_pTree->IsMultiSelect()) {
        //单选，进行校验
        if (pTreeNode->IsSelected()) {
            SetShowTreeNode(pTreeNode);
        }
    }
    else {
        //多选，不校验
        SetShowTreeNode(pTreeNode);
    }

    ShowFileList(m_cachedPath, std::move(pathList));

    //更新界面状态
    UpdateCommandUI();
    UpdateStatusBar();
}

void MainForm::ClearCachedContents()
{
    if (m_cachedFolderList != nullptr) {
        ui::DirectoryTree::ClearPathInfoList(*m_cachedFolderList);
        m_cachedFolderList.reset();
    }
    if (m_cachedFileList != nullptr) {
        ui::DirectoryTree::ClearPathInfoList(*m_cachedFileList);
        m_cachedFileList.reset();
    }
    m_cachedPath.Clear();
}

void MainForm::ShowFileList(const ui::FilePath& currentPath, std::vector<ui::DirectoryTree::PathInfo> pathList)
{
    if (m_dataViewType == kPictureView) {
        SwitchToTabBoxViewType(TabBoxViewType::kFileView);
        if (m_pSimpleFileView != nullptr) {
            //保持当前视图的路径和所选择的路径
            ui::FilePath oldCurrentPath;
            ui::FilePath oldSelectedPath;
            m_pSimpleFileView->GetCurrentPath(oldCurrentPath, oldSelectedPath);
            if (!oldCurrentPath.IsEmpty()) {
                m_selectedPathInfo[oldCurrentPath] = oldSelectedPath;
            }
            ui::FilePath selectedPath;
            auto iter = m_selectedPathInfo.find(currentPath);
            if (iter != m_selectedPathInfo.end()) {
                selectedPath = iter->second;
            }
            m_pSimpleFileView->SetFileList(currentPath, std::move(pathList), selectedPath);
        }
    }
    else {
        SwitchToTabBoxViewType(TabBoxViewType::kExplorerView);
        if (m_pExplorerView != nullptr) {
            //保持当前视图的路径和所选择的路径
            ui::FilePath oldCurrentPath;
            ui::FilePath oldSelectedPath;
            m_pExplorerView->GetCurrentPath(oldCurrentPath, oldSelectedPath);
            if (!oldCurrentPath.IsEmpty()) {
                m_selectedPathInfo[oldCurrentPath] = oldSelectedPath;
            }
            ui::FilePath selectedPath;
            auto iter = m_selectedPathInfo.find(currentPath);
            if (iter != m_selectedPathInfo.end()) {
                selectedPath = iter->second;
            }
            m_pExplorerView->SetFileList(currentPath, std::move(pathList), selectedPath, m_searchKeywords);
        }
    }

    //搜索没有匹配到任何项目时，切换到空结果提示视图
    if (!m_searchKeywords.empty() && (m_nShowItemCount == 0)) {
        ShowEmptyView();
    }
}

void MainForm::ShowEmptyView()
{
    //空结果提示：搜索条件没有匹配到任何项目
    if (m_pEmptyTip != nullptr) {
        m_pEmptyTip->SetText(ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_SEARCH_NO_MATCH")));
    }
    SwitchToTabBoxViewType(TabBoxViewType::kErrorView);
}

bool MainForm::OnSearchTextChanged(const ui::EventArgs& msg)
{
    DString text;
    if (m_pSearchBox != nullptr) {
        text = m_pSearchBox->GetSearchText();
    }
    ui::StringUtil::Trim(text);
    DString keyword = ui::StringUtil::MakeLowerString(text);
    if (keyword == m_searchText) {
        //搜索条件未发生变化
        return true;
    }
    m_searchText = keyword;
    const bool bOldFilter = !m_searchKeywords.empty();
    UpdateSearchKeywords();
    const bool bNewFilter = !m_searchKeywords.empty();
    if (!bOldFilter && !bNewFilter) {
        //过滤条件未生效（输入长度不足）：列表保持原样，只更新状态栏中的提示
        UpdateStatusBar();
        return true;
    }
    //搜索框已通过"debounce_interval"属性做了输入防抖（见皮肤文件tree_view.xml），
    //这里只按缓存的目录内容过滤，不重新枚举目录，开销很小
    ApplySearchFilter();
    return true;
}

void MainForm::UpdateSearchKeywords()
{
    m_searchKeywords.clear();
    if (m_searchText.empty()) {
        return;
    }
    if (m_searchText.size() < kMinSearchTextLength) {
        //输入长度不足时不触发过滤：此时不生成关键字，列表显示全部项目
        return;
    }
    //支持以空格分隔的多个关键字：每个关键字都需要匹配
    auto keywordList = ui::StringUtil::Split(m_searchText, _T(" "));
    for (DString& keyword : keywordList) {
        ui::StringUtil::Trim(keyword);
        if (!keyword.empty()) {
            m_searchKeywords.push_back(keyword);
        }
    }
}

bool MainForm::MatchSearchText(const ui::DirectoryTree::PathInfo& pathInfo) const
{
    if (m_searchKeywords.empty()) {
        return true;
    }
    //匹配范围：显示名称、文件类型名称
    const DString displayName = ui::StringUtil::MakeLowerString(pathInfo.m_displayName);
    const DString typeName = ui::StringUtil::MakeLowerString(pathInfo.m_typeName);
    for (const DString& keyword : m_searchKeywords) {
        if ((displayName.find(keyword) == DString::npos) && (typeName.find(keyword) == DString::npos)) {
            return false;
        }
    }
    return true;
}

bool MainForm::MatchDiskSearchText(const ui::DirectoryTree::DiskInfo& diskInfo) const
{
    if (m_searchKeywords.empty()) {
        return true;
    }
    const DString displayName = ui::StringUtil::MakeLowerString(diskInfo.m_displayName);
    for (const DString& keyword : m_searchKeywords) {
        if (displayName.find(keyword) == DString::npos) {
            return false;
        }
    }
    return true;
}

void MainForm::ApplySearchFilter()
{
    if ((m_pTree == nullptr) || (m_pTreeNode == nullptr)) {
        return;
    }
    if (m_cachedPath.IsEmpty() || (m_cachedPath != m_currentPath)) {
        //没有可用的缓存（或缓存的内容不是当前目录的）：重新枚举当前目录的内容，
        //由 OnShowFolderContents 更新缓存后，再按最新的搜索条件过滤显示
        m_pTree->RefreshFolderContents(m_pTreeNode, nullptr);
        return;
    }
    //按缓存的目录内容过滤后显示：避免每次搜索都重新枚举整个目录
    ShowCachedContents(m_pTreeNode);
}

void MainForm::ClearSearchText()
{
    const bool bHasSearchText = !m_searchText.empty();
    ResetSearchText();
    if (bHasSearchText) {
        //搜索条件已清除，需要重新加载当前目录的内容，去掉过滤效果
        ApplySearchFilter();
    }
}

void MainForm::ResetSearchText()
{
    m_searchText.clear();
    m_searchKeywords.clear();
    if (m_pSearchBox != nullptr) {
        if (!m_pSearchBox->GetSearchText().empty()) {
            //清空搜索框中的文本（关键字已经相同，OnSearchTextChanged 不会重复刷新）
            m_pSearchBox->SetSearchText(DString());
        }
    }
}

DString MainForm::GetCurrentViewName() const
{
    DString strId;
    switch (m_dataViewType) {
    case kIconViewBig:
        strId = _T("STRID_TREEVIEW_ICON_VIEW_BIG");
        break;
    case kIconViewMedium:
        strId = _T("STRID_TREEVIEW_ICON_VIEW_MEDIUM");
        break;
    case kIconViewSmall:
        strId = _T("STRID_TREEVIEW_ICON_VIEW_SMALL");
        break;
    case kListViewBig:
        strId = _T("STRID_TREEVIEW_LIST_VIEW_BIG");
        break;
    case kListViewMedium:
        strId = _T("STRID_TREEVIEW_LIST_VIEW_MEDIUM");
        break;
    case kListViewSmall:
        strId = _T("STRID_TREEVIEW_LIST_VIEW_SMALL");
        break;
    case kReprortView:
        strId = _T("STRID_TREEVIEW_DETAIL_VIEW");
        break;
    case kPictureView:
        strId = _T("STRID_TREEVIEW_IMAGE_LIST_VIEW");
        break;
    default:
        break;
    }
    if (strId.empty()) {
        return DString();
    }
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}

DString MainForm::GetCurrentSortInfo() const
{
    if ((m_pExplorerView == nullptr) || (m_dataViewType == kPictureView)) {
        return DString();
    }
    ExplorerView::ExplorerViewColumn sortColumn = ExplorerView::ExplorerViewColumn::kName;
    bool bSortUp = false;
    if (!m_pExplorerView->GetSortColumnInfo(sortColumn, bSortUp)) {
        return DString();
    }
    DString strNameId;
    switch (sortColumn) {
    case ExplorerView::ExplorerViewColumn::kName:
        strNameId = _T("STRID_TREEVIEW_FILE_NAME");
        break;
    case ExplorerView::ExplorerViewColumn::kModifyDateTime:
        strNameId = _T("STRID_TREEVIEW_FILE_MODIFY_TIME");
        break;
    case ExplorerView::ExplorerViewColumn::kType:
        strNameId = _T("STRID_TREEVIEW_FILE_TYPE");
        break;
    case ExplorerView::ExplorerViewColumn::kSize:
        strNameId = _T("STRID_TREEVIEW_FILE_SIZE");
        break;
    default:
        break;
    }
    DString sortInfo = ui::GlobalManager::Instance().Lang().GetStringByID(strNameId);
    sortInfo += _T(" ");
    if (bSortUp) {
        sortInfo += ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_ASC"));
    }
    else {
        sortInfo += ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_DESC"));
    }
    return sortInfo;
}

void MainForm::UpdateStatusBar()
{
    //当前路径
    if (m_pStatusPath != nullptr) {
        if (m_bComputerView) {
            m_pStatusPath->SetText(ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_COMPUTER")));
        }
        else {
            m_pStatusPath->SetText(m_currentPath.ToString());
        }
    }

    //项目统计
    if (m_pStatusInfo != nullptr) {
        const bool bFilter = !m_searchKeywords.empty();
        DString strInfo;
        if (m_bComputerView) {
            if (bFilter) {
                const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_FILTERED_DISKS"));
                strInfo = ui::StringUtil::Printf(strFormat.c_str(), m_nDiskCount, m_nShowItemCount);
            }
            else {
                const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_DISKS"));
                strInfo = ui::StringUtil::Printf(strFormat.c_str(), m_nShowItemCount);
            }
        }
        else if (!bFilter) {
            const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_ITEMS"));
            strInfo = ui::StringUtil::Printf(strFormat.c_str(), m_nTotalItemCount, m_nFolderCount, m_nFileCount);
        }
        else {
            const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_FILTERED"));
            strInfo = ui::StringUtil::Printf(strFormat.c_str(), m_nTotalItemCount, m_nShowItemCount);
        }

        if (!bFilter && !m_searchText.empty()) {
            //搜索框中有内容，但输入长度不足，暂未开始过滤：提示用户继续输入
            if (!strInfo.empty()) {
                strInfo += _T(" · ");
            }
            const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_SEARCH_TOO_SHORT"));
            strInfo += ui::StringUtil::Printf(strFormat.c_str(), (int32_t)kMinSearchTextLength);
        }

        //选中项信息：数量与文件总大小
        int32_t nSelectedCount = 0;
        uint64_t nSelectedFileSize = 0;
        GetSelectedItemInfo(nSelectedCount, nSelectedFileSize);
        if (nSelectedCount > 0) {
            if (!strInfo.empty()) {
                strInfo += _T(" · ");
            }
            if (!m_bComputerView && (nSelectedFileSize > 0)) {
                const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_SELECTED_SIZE"));
                strInfo += ui::StringUtil::Printf(strFormat.c_str(), nSelectedCount, FormatItemSize(nSelectedFileSize).c_str());
            }
            else {
                const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_SELECTED"));
                strInfo += ui::StringUtil::Printf(strFormat.c_str(), nSelectedCount);
            }
        }
        m_pStatusInfo->SetText(strInfo);
    }

    //视图与排序方式
    if (m_pStatusMode != nullptr) {
        DString strMode;
        if (m_bComputerView) {
            //"计算机"视图：显示"磁盘视图"以及所有磁盘的容量合计
            uint64_t nTotalBytes = 0;
            uint64_t nFreeBytes = 0;
            if ((m_pComputerView != nullptr) && m_pComputerView->GetDiskSpaceInfo(nTotalBytes, nFreeBytes)) {
                const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_DISK_SPACE"));
                strMode = ui::StringUtil::Printf(strFormat.c_str(),
                                                m_pComputerView->FormatDiskSpace(nTotalBytes).c_str(),
                                                m_pComputerView->FormatDiskSpace(nFreeBytes).c_str());
            }
            else {
                strMode = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_VIEW_TYPE_DISK"));
            }
        }
        else {
            strMode = GetCurrentViewName();
            const DString strSortInfo = GetCurrentSortInfo();
            if (!strSortInfo.empty()) {
                if (!strMode.empty()) {
                    strMode += _T(" · ");
                }
                strMode += strSortInfo;
            }
        }
        m_pStatusMode->SetText(strMode);
    }
}

DString MainForm::FormatItemSize(uint64_t nFileSize) const
{
    //与文件列表中的"文件大小"列显示格式保持一致
    return ExplorerView::FormatFileSize(false, nFileSize);
}

void MainForm::OnShowMyComputerContents(ui::TreeNode* pTreeNode,
                                        const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList)
{
    ui::GlobalManager::Instance().AssertUIThread();
    //目录内容已经加载完成（失败时也结束加载状态，避免提示一直显示）
    EndLoading();
    if ((pTreeNode == nullptr) || (m_pTree == nullptr) || !m_pTree->IsValidTreeNode(pTreeNode)) {
        std::vector<ui::DirectoryTree::DiskInfo> tempDiskInfoList(diskInfoList);
        ui::DirectoryTree::ClearDiskInfoList(tempDiskInfoList);
        return;
    }
    if (pTreeNode != m_pTreeNode) {
        //切换到其它节点时，清除搜索条件（这里不重新加载旧目录，避免重复刷新）
        ResetSearchText();
    }
    SwitchToTabBoxViewType(TabBoxViewType::kComputerView);
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->SetAddressPath(_T(""));
    }
    m_bComputerView = true;
    m_currentPath = ui::FilePath();
    //"计算机"视图不需要监听目录内容的变化
    StopDirectoryWatcher();
    //"计算机"视图没有缓存的目录内容，释放缓存的目录内容（含图标资源）
    ClearCachedContents();
    m_nDiskCount = (int32_t)diskInfoList.size();

    //显示计算机视图的内容（按搜索条件过滤磁盘）
    if (m_pComputerView != nullptr) {
        if (m_searchKeywords.empty()) {
            m_nShowItemCount = m_nDiskCount;
            m_pComputerView->ShowMyComputerContents(diskInfoList);
        }
        else {
            std::vector<ui::DirectoryTree::DiskInfo> filteredDiskList;
            std::vector<ui::DirectoryTree::DiskInfo> unusedDiskList;//被搜索条件过滤掉的磁盘
            for (const ui::DirectoryTree::DiskInfo& diskInfo : diskInfoList) {
                if (MatchDiskSearchText(diskInfo)) {
                    filteredDiskList.push_back(diskInfo);
                }
                else {
                    unusedDiskList.push_back(diskInfo);
                }
            }
            m_nShowItemCount = (int32_t)filteredDiskList.size();
            m_pComputerView->ShowMyComputerContents(filteredDiskList);
            if (!unusedDiskList.empty()) {
                //释放未被使用的磁盘的图标资源
                ui::DirectoryTree::ClearDiskInfoList(unusedDiskList);
            }
        }
    }

    //保存显示的树节点
    SetShowTreeNode(pTreeNode);   

    //搜索没有匹配到任何磁盘时，切换到空结果提示视图
    if (!m_searchKeywords.empty() && (m_nShowItemCount == 0)) {
        ShowEmptyView();
    }

    //更新界面状态
    UpdateCommandUI();
    UpdateStatusBar();
}

void MainForm::SelectSubPath(const ui::FilePath& filePath)
{
    if (!filePath.IsExistsDirectory()) {
        //如果文件夹不存在，报错
        const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_PATH_NOT_EXISTS"));
        const DString errMsg = ui::StringUtil::Printf(strFormat.c_str(), filePath.ToString().c_str());
        const DString title = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_ERROR_TITLE"));
        ui::SystemUtil::ShowMessageBox(this, errMsg.c_str(), title.c_str());
        return;
    }

    if (m_pTree != nullptr) {
        //导航开始时即更新地址栏，避免目录枚举完成前地址栏仍显示旧路径
        if (m_pAddressBar != nullptr) {
            m_pAddressBar->SetAddressPath(filePath.ToString());
        }
        BeginLoading();
        if (!m_pTree->SelectSubPath(m_pTreeNode, filePath, nullptr)) {
            EndLoading();
            if (m_pAddressBar != nullptr) {
                m_pAddressBar->SetAddressPath(m_pAddressBar->GetPreviousAddressPath());
            }
        }
    }
}

void MainForm::OpenPath(const ui::FilePath& filePath)
{
    if (filePath.IsEmpty()) {
        return;
    }
    if (filePath.IsExistsDirectory()) {
        //目录：进入该目录
        SelectSubPath(filePath);
        return;
    }
    if (filePath.IsExistsFile()) {
        //文件：用系统默认程序打开（Windows使用ShellExecute，其他平台使用SDL_OpenURL）
        OpenWithSystem(filePath);
    }
}

void MainForm::ShowViewContextMenu(ui::Control* pView, const ui::FilePath& targetPath)
{
    //使用系统光标位置作为菜单的弹出位置（避免DPI缩放时客户区坐标换算出错）
    ui::UiPoint point;
    GetCursorPos(point);

    //右键点在已选中的项目上时，菜单对全部选中项生效；否则只对右键的项目生效
    std::vector<ui::FilePath> selectedPaths;
    GetSelectedPaths(selectedPaths);
    bool bTargetSelected = false;
    for (const ui::FilePath& path : selectedPaths) {
        if (path == targetPath) {
            bTargetSelected = true;
            break;
        }
    }
    std::vector<ui::FilePath> actionPaths;
    if (bTargetSelected) {
        actionPaths = selectedPaths;
    }
    else if (!targetPath.IsEmpty()) {
        actionPaths.push_back(targetPath);
    }
    const bool bHasTarget = !actionPaths.empty();

    ui::Menu* menu = new ui::Menu(this, pView);//需要设置父窗口，否则菜单弹出时程序状态栏会变成非激活状态
    menu->SetSkinFolder(GetResourcePath().ToString());
    DString xml(_T("menu/context_menu.xml"));
    menu->ShowMenu(xml, point);

    //右键点在空白区域时，只保留"刷新"可用
    ui::MenuItem* pOpenItem = menu->GetMenuItemByName(_T("menu_item_open"));
    if (pOpenItem != nullptr) {
        pOpenItem->SetEnabled(bHasTarget);
    }
    ui::MenuItem* pRevealItem = menu->GetMenuItemByName(_T("menu_item_reveal"));
    if (pRevealItem != nullptr) {
        pRevealItem->SetEnabled(bHasTarget);
    }
    ui::MenuItem* pCopyItem = menu->GetMenuItemByName(_T("menu_item_copy_path"));
    if (pCopyItem != nullptr) {
        pCopyItem->SetEnabled(bHasTarget);
    }

    menu->AttachMenuItemActivated([this, actionPaths, targetPath, bHasTarget](const DString& /*menuName*/, int32_t /*nMenuLevel*/,
                                                                              const DString& itemName, size_t /*nItemIndex*/) {
        if (itemName == _T("menu_item_refresh")) {
            Refresh();
            return;
        }
        if (!bHasTarget) {
            return;
        }
        if (itemName == _T("menu_item_open")) {
            //打开所有选中项：目录只进入第一个，文件全部用系统默认程序打开
            OpenPaths(actionPaths);
        }
        else if (itemName == _T("menu_item_reveal")) {
            //"在文件资源管理器中显示"只针对右键的项目
            if (!targetPath.IsEmpty()) {
                RevealInFileManager(targetPath);
            }
        }
        else if (itemName == _T("menu_item_copy_path")) {
            //多选时，每行一个完整路径
            CopyPathsToClipboard(actionPaths);
        }
    });
}

void MainForm::OnColumnSorted()
{
    //点击表头排序后，同步更新状态栏中的排序信息
    UpdateStatusBar();
}

void MainForm::OnViewSelectionChanged()
{
    //选择项变化后，同步更新状态栏中的选中项信息
    UpdateStatusBar();
}

LRESULT MainForm::OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = ui::Window::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    if (bHandled) {
        return lResult;
    }
    //快捷键在窗口级处理：焦点可能在目录树、文件列表或按钮上，按键不应因焦点变化而失效

    //Esc：清除搜索条件（任何焦点下都生效，避免搜索后无法快速清除）
    if (vkCode == ui::kVK_ESCAPE) {
        if (!m_searchText.empty()) {
            ClearSearchText();
            bHandled = true;
        }
        return lResult;
    }
    //F5：刷新
    if (vkCode == ui::kVK_F5) {
        Refresh();
        bHandled = true;
        return lResult;
    }
    //Ctrl+F：聚焦搜索框
    if (((modifierKey & ui::kControl) != 0) && (vkCode == ui::kVK_F)) {
        if (m_pSearchBox != nullptr) {
            m_pSearchBox->SetFocus();
            bHandled = true;
        }
        return lResult;
    }
    //Ctrl+L / Alt+D：聚焦地址栏（与浏览器、Windows资源管理器的习惯一致）
    if ((((modifierKey & ui::kControl) != 0) && (vkCode == ui::kVK_L)) ||
        (((modifierKey & ui::kAlt) != 0) && (vkCode == ui::kVK_D))) {
        if (m_pAddressBar != nullptr) {
            //复用控件自身的编辑态入口，保证与控件内置的 F6/Ctrl+L 行为一致（含全选路径）
            m_pAddressBar->EnterAddressEdit();
            bHandled = true;
        }
        return lResult;
    }
    //Alt+Left / Alt+Right / Alt+Up：后退、前进、返回上级目录（与Windows资源管理器的习惯一致）
    if ((modifierKey & ui::kAlt) != 0) {
        if (vkCode == ui::kVK_LEFT) {
            ShowBack();
            bHandled = true;
            return lResult;
        }
        if (vkCode == ui::kVK_RIGHT) {
            ShowForward();
            bHandled = true;
            return lResult;
        }
        if (vkCode == ui::kVK_UP) {
            //焦点在地址栏内时，Alt+Up 已由 AddressBar 内置逻辑处理，这里不再重复导航
            ui::Control* pFocus = GetFocusControl();
            if ((m_pAddressBar == nullptr) || (pFocus == nullptr) || !IsControlInSubTree(pFocus, m_pAddressBar)) {
                ShowUp();
                bHandled = true;
            }
            return lResult;
        }
    }

    //以下快捷键只在目录树和文件列表中有焦点时生效，避免干扰输入框、按钮等控件
    ui::Control* pFocus = GetFocusControl();
    const bool bViewFocus = (pFocus == nullptr) || IsControlInSubTree(pFocus, m_pTabBox);
    const bool bTreeFocus = (pFocus == nullptr) || IsControlInSubTree(pFocus, m_pTree);

    //Enter：打开选中的文件或目录
    if (bViewFocus && (vkCode == ui::kVK_RETURN)) {
        OpenSelectedItem();
        bHandled = true;
        return lResult;
    }
    //Backspace：返回上级目录（目录树有焦点时也生效，因为切换目录后焦点可能落在树节点上）
    if ((bViewFocus || bTreeFocus) && (vkCode == ui::kVK_BACK)) {
        ShowUp();
        bHandled = true;
        return lResult;
    }
    //Ctrl+A：全选当前视图中的所有项目
    if (bViewFocus && ((modifierKey & ui::kControl) != 0) && (vkCode == ui::kVK_A)) {
        SelectAllItems();
        bHandled = true;
        return lResult;
    }
    //Ctrl+C：复制选中项的完整路径（多选时每行一个路径）
    if (bViewFocus && ((modifierKey & ui::kControl) != 0) && (vkCode == ui::kVK_C)) {
        std::vector<ui::FilePath> selectedPaths;
        GetSelectedPaths(selectedPaths);
        CopyPathsToClipboard(selectedPaths);
        bHandled = true;
        return lResult;
    }
    return lResult;
}

void MainForm::OpenSelectedItem()
{
    std::vector<ui::FilePath> selectedPaths;
    GetSelectedPaths(selectedPaths);
    if (!selectedPaths.empty()) {
        //多选时：打开所有选中的文件；如果有目录，只进入第一个目录
        OpenPaths(selectedPaths);
    }
}

void MainForm::OpenPaths(const std::vector<ui::FilePath>& paths)
{
    bool bFolderOpened = false;
    for (const ui::FilePath& path : paths) {
        if (path.IsExistsDirectory()) {
            if (!bFolderOpened) {
                OpenPath(path);
                bFolderOpened = true;
            }
        }
        else if (path.IsExistsFile()) {
            //文件：全部用系统默认程序打开
            OpenWithSystem(path);
        }
    }
}

void MainForm::SelectAllItems()
{
    if (m_bComputerView) {
        if (m_pComputerView != nullptr) {
            m_pComputerView->SelectAll();
        }
    }
    else if (m_dataViewType == kPictureView) {
        if (m_pSimpleFileView != nullptr) {
            m_pSimpleFileView->SelectAll();
        }
    }
    else if (m_pExplorerView != nullptr) {
        m_pExplorerView->SelectAll();
    }
    //ListCtrl的全选不会触发选择事件，这里主动更新状态栏中的选中项信息
    UpdateStatusBar();
}

ui::FilePath MainForm::GetSelectedPath() const
{
    std::vector<ui::FilePath> paths;
    GetSelectedPaths(paths);
    if (!paths.empty()) {
        return paths.front();
    }
    return ui::FilePath();
}

void MainForm::GetSelectedPaths(std::vector<ui::FilePath>& paths) const
{
    paths.clear();
    if (m_bComputerView) {
        if (m_pComputerView != nullptr) {
            m_pComputerView->GetSelectedPaths(paths);
        }
    }
    else if (m_dataViewType == kPictureView) {
        if (m_pSimpleFileView != nullptr) {
            m_pSimpleFileView->GetSelectedPaths(paths);
        }
    }
    else if (m_pExplorerView != nullptr) {
        m_pExplorerView->GetSelectedPaths(paths);
    }
}

void MainForm::GetSelectedItemInfo(int32_t& nSelectedCount, uint64_t& nSelectedFileSize) const
{
    nSelectedCount = 0;
    nSelectedFileSize = 0;
    if (m_bComputerView) {
        if (m_pComputerView != nullptr) {
            m_pComputerView->GetSelectedItemInfo(nSelectedCount, nSelectedFileSize);
        }
    }
    else if (m_dataViewType == kPictureView) {
        if (m_pSimpleFileView != nullptr) {
            m_pSimpleFileView->GetSelectedItemInfo(nSelectedCount, nSelectedFileSize);
        }
    }
    else if (m_pExplorerView != nullptr) {
        m_pExplorerView->GetSelectedItemInfo(nSelectedCount, nSelectedFileSize);
    }
}

void MainForm::CopyPathsToClipboard(const std::vector<ui::FilePath>& paths) const
{
    if (paths.empty()) {
        return;
    }
#if defined (DUILIB_BUILD_FOR_WIN)
    const DString lineBreak(_T("\r\n"));
#else
    const DString lineBreak(_T("\n"));
#endif
    DString text;
    for (const ui::FilePath& path : paths) {
        if (path.IsEmpty()) {
            continue;
        }
        if (!text.empty()) {
            text += lineBreak;
        }
        text += path.ToString();
    }
    if (!text.empty()) {
        //剪贴板使用UTF8编码的文本
        ui::Clipboard::SetClipboardText(ui::StringConvert::TToUTF8(text));
    }
}

bool MainForm::OnAddressBarPathChanged(const ui::EventArgs& msg)
{
    if (m_pAddressBar != nullptr) {
        DString text = m_pAddressBar->GetAddressPath();
        if (!OnShowAddressPath(text)) {
            m_pAddressBar->SetAddressPath(m_pAddressBar->GetPreviousAddressPath());
        }
    }
    return true;
}

bool MainForm::OnAddressBarPathClick(const ui::EventArgs& msg)
{
    if (m_pAddressBar != nullptr) {
        DString text = m_pAddressBar->GetClickedAddressPath();
        OnShowAddressPath(text);
    }
    return true;
}

bool MainForm::OnAddressBarPathChanging(DString& newPath)
{
    ui::StringUtil::Trim(newPath);
    if (newPath.empty()) {
        return false;
    }
    ui::FilePath inputFilePath(newPath);
    if (!inputFilePath.IsAbsolutePath()) {
        //相对路径与当前树节点的路径拼接
        ui::FilePath curFilePath;
        if ((m_pTree != nullptr) && (m_pTreeNode != nullptr)) {
            curFilePath = m_pTree->FindTreeNodePath(m_pTreeNode);
        }
        if (!curFilePath.IsEmpty()) {
            inputFilePath = curFilePath.JoinFilePath(inputFilePath);
        }
        newPath = inputFilePath.ToString();
    }
    if (inputFilePath.IsExistsDirectory()) {
        m_pendingLocateFile = ui::FilePath();
        return true;
    }
    if (inputFilePath.IsExistsFile()) {
        //输入的是文件路径：转换为父目录，并记录需要在列表中选中的文件
        ui::FilePath parentPath = inputFilePath.GetParentPath();
        if (parentPath.IsExistsDirectory()) {
            m_pendingLocateFile = inputFilePath;
            newPath = parentPath.ToString();
            return true;
        }
    }
    //路径不存在：提示用户，地址栏保持原路径
    const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_PATH_NOT_EXISTS"));
    const DString errMsg = ui::StringUtil::Printf(strFormat.c_str(), newPath.c_str());
    const DString title = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_ERROR_TITLE"));
    ui::SystemUtil::ShowMessageBox(this, errMsg.c_str(), title.c_str());
    return false;
}

bool MainForm::OnTreeNodeSelect(const ui::EventArgs& msg)
{
    ui::TreeNode* pTreeNode = (ui::TreeNode*)msg.wParam;
    if ((pTreeNode != nullptr) && (m_pTree != nullptr) && m_pTree->IsValidTreeNode(pTreeNode)) {
        SetAddressBarPathFromTreeNode(pTreeNode);
    }
    return true;
}

void MainForm::SetAddressBarPathFromTreeNode(ui::TreeNode* pTreeNode)
{
    if ((m_pAddressBar == nullptr) || (m_pTree == nullptr) || (pTreeNode == nullptr)) {
        return;
    }
    if (m_pTree->IsMyComputerNode(pTreeNode)) {
        m_pAddressBar->SetAddressPath(_T(""));
        return;
    }
    ui::FilePath filePath = m_pTree->FindTreeNodePath(pTreeNode);
    if (!filePath.IsEmpty()) {
        m_pAddressBar->SetAddressPath(filePath.ToString());
    }
}

bool MainForm::OnShowAddressPath(const DString& newFilePath)
{
    DString text = newFilePath;
    ui::StringUtil::Trim(text);
    if (text.empty()) {
        return false;
    }
    ui::FilePath curFilePath;//当前树节点对应的目录
    ui::TreeNode* pTreeNode = m_pTreeNode;
    if (pTreeNode != nullptr) {
        curFilePath = m_pTree->FindTreeNodePath(pTreeNode);
    }
    ui::FilePath inputFilePath(text); //当前输入的目录
    ui::TreeNode* pParentTreeNode = nullptr;
    if (!inputFilePath.IsAbsolutePath()) {
        //如果是相对路径，与当前树节点的路径拼接
        if (!curFilePath.IsEmpty()) {
            inputFilePath = curFilePath.JoinFilePath(inputFilePath);
            pParentTreeNode = m_pTreeNode;
        }
    }
    else if(!curFilePath.IsEmpty() && inputFilePath.IsSubDirectory(curFilePath)) {
        //新输入的目录，在当前树节点目录下
        pParentTreeNode = m_pTreeNode;
    }
    if (inputFilePath.IsAbsolutePath() && inputFilePath.IsExistsDirectory()) {
        //如果地址栏输入的是一个文件路径，则定位到其父目录并在列表中选择该文件
        if (!m_pendingLocateFile.IsEmpty()) {
            ui::FilePath pendingParent = m_pendingLocateFile.GetParentPath();
            if (pendingParent == inputFilePath) {
                //键值与ShowFileList中使用的目录路径格式保持一致（目录路径以路径分隔符结尾），否则无法匹配到选择项
                ui::FilePath locateKey = inputFilePath;
                locateKey.FormatPathAsDirectory();
                m_selectedPathInfo[locateKey] = m_pendingLocateFile;
            }
            m_pendingLocateFile = ui::FilePath();
        }
        //地址栏上面的是有效路径，让左树展开对应的路径，并选择
        if (m_pTree != nullptr) {
            if (pParentTreeNode != nullptr) {
                m_pTree->SelectSubPath(pParentTreeNode, inputFilePath, nullptr);
            }
            else {
                m_pTree->SelectPath(inputFilePath, nullptr);
            }
        }
        return true;
    }
    else {
        //如果文件夹不存在，报错
        const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_PATH_NOT_EXISTS"));
        const DString errMsg = ui::StringUtil::Printf(strFormat.c_str(), text.c_str());
        const DString title = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_ERROR_TITLE"));
        ui::SystemUtil::ShowMessageBox(this, errMsg.c_str(), title.c_str());
        return false;
    }
}

void MainForm::OnRefresh()
{
    if (m_pTree == nullptr) {
        return;
    }
    ui::TreeNode* pTreeNode = m_pTreeNode;
    if (!m_pTree->IsValidTreeNode(pTreeNode)) {
        //树节点已经被删除
        pTreeNode = nullptr;
    }
    if (pTreeNode == nullptr) {
        //如果当前显示的节点被删除，那么查找父节点
        int32_t nCount = (int32_t)m_parentTreeNodes.size();
        for (int32_t nIndex = nCount - 1; nIndex >= 0; --nIndex) {
            ui::TreeNode* pParentTreeNode = m_parentTreeNodes[nIndex];
            if (m_pTree->IsValidTreeNode(pParentTreeNode)) {
                if (m_pTree->IsMyComputerNode(pParentTreeNode)) {
                    pTreeNode = pParentTreeNode;
                    break;
                }
                else {
                    ui::FilePath filePath = m_pTree->FindTreeNodePath(pParentTreeNode);
                    if (filePath.IsExistsDirectory()) {
                        pTreeNode = pParentTreeNode;
                        break;
                    }
                }
            }
        }
    }
    ASSERT(pTreeNode != nullptr);
    if ((m_pTree != nullptr) && (pTreeNode != nullptr)) {
        m_bCanAddBackForward = false;
        m_pTree->SelectTreeNode(pTreeNode);
    }
}

void MainForm::ShowUp()
{
    if ((m_pTree == nullptr) || (m_pTreeNode == nullptr)) {
        return;
    }
    ui::TreeNode* pParentNode = m_pTreeNode->GetParentNode();
    if (pParentNode == nullptr) {
        return;
    }
    if (pParentNode == m_pTree->GetRootNode()) {
        //已经是顶层节点（父节点是根节点），不需要再向上
        return;
    }
    m_bCanAddBackForward = true;
    SetAddressBarPathFromTreeNode(pParentNode);
    BeginLoading();
    if (!m_pTree->SelectTreeNode(pParentNode)) {
        EndLoading();
        SetAddressBarPathFromTreeNode(m_pTreeNode);
    }
}

void MainForm::ShowBack()
{
    //后退
    if (m_pTree == nullptr) {
        return;
    }
    //从后退栈中查找一个有效的节点（跳过已经被删除的节点）
    ui::TreeNode* pTargetNode = nullptr;
    while (!m_backStack.empty()) {
        ui::TreeNode* pTreeNode = m_backStack.top();
        m_backStack.pop();
        if (m_pTree->IsValidTreeNode(pTreeNode)) {
            pTargetNode = pTreeNode;
            break;
        }
    }
    if (pTargetNode == nullptr) {
        //没有可以后退的节点时，不修改前进栈，只刷新按钮状态
        UpdateCommandUI();
        return;
    }
    if (m_pTree->IsValidTreeNode(m_pTreeNode)) {
        m_forwardStack.push(m_pTreeNode);
    }
    m_bCanAddBackForward = false;
    SetAddressBarPathFromTreeNode(pTargetNode);
    BeginLoading();
    if (!m_pTree->SelectTreeNode(pTargetNode)) {
        EndLoading();
    }
    UpdateCommandUI();
}

void MainForm::ShowForward()
{
    if (m_pTree == nullptr) {
        return;
    }
    //从前进栈中查找一个有效的节点（跳过已经被删除的节点）
    ui::TreeNode* pTargetNode = nullptr;
    while (!m_forwardStack.empty()) {
        ui::TreeNode* pTreeNode = m_forwardStack.top();
        m_forwardStack.pop();
        if (m_pTree->IsValidTreeNode(pTreeNode)) {
            pTargetNode = pTreeNode;
            break;
        }
    }
    if (pTargetNode == nullptr) {
        //没有可以前进的节点时，不修改后退栈，只刷新按钮状态
        UpdateCommandUI();
        return;
    }
    if (m_pTree->IsValidTreeNode(m_pTreeNode)) {
        m_backStack.push(m_pTreeNode);
    }
    m_bCanAddBackForward = false;
    SetAddressBarPathFromTreeNode(pTargetNode);
    BeginLoading();
    if (!m_pTree->SelectTreeNode(pTargetNode)) {
        EndLoading();
    }
    UpdateCommandUI();
}

void MainForm::SwitchListType(const ui::UiPoint& point, ui::Control* pRelatedControl)
{
    ui::Menu* menu = new ui::Menu(this, pRelatedControl);//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    menu->SetSkinFolder(GetResourcePath().ToString());
    DString xml(_T("menu/list_type_menu.xml"));
    menu->ShowMenu(xml, point);

    std::map<DataViewType, DString> btnNameMap;
    btnNameMap[DataViewType::kIconViewBig] = _T("btn_menu_item_icon_big");
    btnNameMap[DataViewType::kIconViewMedium] = _T("btn_menu_item_icon_medium");
    btnNameMap[DataViewType::kIconViewSmall] = _T("btn_menu_item_icon_small");
    btnNameMap[DataViewType::kListViewBig] = _T("btn_menu_item_list_big");
    btnNameMap[DataViewType::kListViewMedium] = _T("btn_menu_item_list_medium");
    btnNameMap[DataViewType::kListViewSmall] = _T("btn_menu_item_list_small");
    btnNameMap[DataViewType::kReprortView] = _T("btn_menu_item_report");
    btnNameMap[DataViewType::kPictureView] = _T("btn_menu_item_picture");

    DString selectBtnName = btnNameMap[GetDataViewType()];
    ui::Button* pSelectBtn = dynamic_cast<ui::Button*>(menu->FindControl(selectBtnName));
    if (pSelectBtn != nullptr) {
        pSelectBtn->SetBkImage(GetMenuCheckImage());
    }

    //挂载选择菜单项事件
    menu->AttachMenuItemActivated([this](const DString& /*menuName*/, int32_t /*nMenuLevel*/,
                                         const DString& itemName, size_t /*nItemIndex*/) {
            //与XML中的菜单项名字匹配
            std::map<DataViewType, DString> itemNameMap;
            itemNameMap[DataViewType::kIconViewBig] = _T("menu_item_icon_big");
            itemNameMap[DataViewType::kIconViewMedium] = _T("menu_item_icon_medium");
            itemNameMap[DataViewType::kIconViewSmall] = _T("menu_item_icon_small");
            itemNameMap[DataViewType::kListViewBig] = _T("menu_item_list_big");
            itemNameMap[DataViewType::kListViewMedium] = _T("menu_item_list_medium");
            itemNameMap[DataViewType::kListViewSmall] = _T("menu_item_list_small");
            itemNameMap[DataViewType::kReprortView] = _T("menu_item_report");
            itemNameMap[DataViewType::kPictureView] = _T("menu_item_picture");
            for (auto iter : itemNameMap) {
                if (iter.second == itemName) {
                    DataViewType dataViewType = iter.first;
                    SwitchToDataViewType(dataViewType);
                    break;
                }
            }
        });
}

void MainForm::SwitchSortMode(const ui::UiPoint& point, ui::Control* pRelatedControl)
{
    if (m_pExplorerView == nullptr) {
        return;
    }
    ui::Menu* menu = new ui::Menu(this, pRelatedControl);//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    menu->SetSkinFolder(GetResourcePath().ToString());
    DString xml(_T("menu/sort_mode_menu.xml"));
    menu->ShowMenu(xml, point);

    //获取排序方式
    ExplorerView::ExplorerViewColumn sortColumn;
    bool bSortUp = false;
    bool bSorted = m_pExplorerView->GetSortColumnInfo(sortColumn, bSortUp);
    if (bSorted) {
        ui::Button* pSortColumnBtn = nullptr;
        if (sortColumn == ExplorerView::ExplorerViewColumn::kName) {
            pSortColumnBtn = dynamic_cast<ui::Button*>(menu->FindControl(_T("btn_file_name")));
        }
        else if (sortColumn == ExplorerView::ExplorerViewColumn::kModifyDateTime) {
            pSortColumnBtn = dynamic_cast<ui::Button*>(menu->FindControl(_T("btn_file_modify_time")));
        }
        else if (sortColumn == ExplorerView::ExplorerViewColumn::kType) {
            pSortColumnBtn = dynamic_cast<ui::Button*>(menu->FindControl(_T("btn_file_type")));
        }
        else if (sortColumn == ExplorerView::ExplorerViewColumn::kSize) {
            pSortColumnBtn = dynamic_cast<ui::Button*>(menu->FindControl(_T("btn_file_size")));
        }

        ui::Button* pSortBtn = nullptr;
        if (bSortUp) {
            //升序
            pSortBtn = dynamic_cast<ui::Button*>(menu->FindControl(_T("btn_sort_ascending")));
        }
        else {
            //降序
            pSortBtn = dynamic_cast<ui::Button*>(menu->FindControl(_T("btn_sort_descending")));
        }
        if (pSortBtn != nullptr) {
            pSortBtn->SetBkImage(GetMenuCheckImage());
        }
        if (pSortColumnBtn != nullptr) {
            pSortColumnBtn->SetBkImage(GetMenuCheckImage());
        }
    }
    else {
        sortColumn = ExplorerView::ExplorerViewColumn::kName;
        bSortUp = false;
    }

    //挂载选择菜单项事件
    menu->AttachMenuItemActivated([this, bSorted, bSortUp, sortColumn](const DString& menuName, int32_t nMenuLevel,
                                                                       const DString& itemName, size_t nItemIndex) {
            //与XML中的菜单项名字匹配
            if (itemName == _T("menu_item_file_name")) {
                //文件名称
                if (m_pExplorerView != nullptr) {
                    m_pExplorerView->SortByColumn(ExplorerView::ExplorerViewColumn::kName, bSortUp);
                }
            }
            else if (itemName == _T("menu_item_file_modify_time")) {
                //修改日期
                if (m_pExplorerView != nullptr) {
                    m_pExplorerView->SortByColumn(ExplorerView::ExplorerViewColumn::kModifyDateTime, bSortUp);
                }
            }
            else if (itemName == _T("menu_item_file_type")) {
                //文件类型
                if (m_pExplorerView != nullptr) {
                    m_pExplorerView->SortByColumn(ExplorerView::ExplorerViewColumn::kType, bSortUp);
                }
            }
            else if (itemName == _T("menu_item_file_size")) {
                //文件大小
                if (m_pExplorerView != nullptr) {
                    m_pExplorerView->SortByColumn(ExplorerView::ExplorerViewColumn::kSize, bSortUp);
                }
            }
            else if (itemName == _T("menu_item_sort_ascending")) {
                //递增排序
                if (!bSorted || !bSortUp) {
                    if (m_pExplorerView != nullptr) {
                        m_pExplorerView->SortByColumn(sortColumn, true);
                    }
                }
            }
            else if (itemName == _T("menu_item_sort_descending")) {
                //递减排序
                if (!bSorted || bSortUp) {
                    if (m_pExplorerView != nullptr) {
                        m_pExplorerView->SortByColumn(sortColumn, false);
                    }
                }
            }
            //排序方式变化后，同步更新状态栏
            UpdateStatusBar();
        });
}

void MainForm::UpdateCommandUI()
{
    bool bEnableUp = false;
    bool bEnableBack = !m_backStack.empty();
    bool bEnableForward = !m_forwardStack.empty();
    if ((m_pTreeNode != nullptr) && (m_pTree != nullptr)) {
        ui::TreeNode* pParentNode = m_pTreeNode->GetParentNode();
        if ((pParentNode != nullptr) && (pParentNode != m_pTree->GetRootNode())) {
            bEnableUp = true;
        }
    }
    if (m_pBtnUp != nullptr) {
        m_pBtnUp->SetEnabled(bEnableUp);
    }
    if (m_pBtnBack != nullptr) {
        m_pBtnBack->SetEnabled(bEnableBack);
    }
    if (m_pBtnForward != nullptr) {
        m_pBtnForward->SetEnabled(bEnableForward);
    }

    bool bIsComputerView = false;
    if (m_pTree != nullptr) {
        bIsComputerView = m_pTree->IsMyComputerNode(m_pTreeNode);
    }
    //切换列表类型
    if (m_pBtnViewListType != nullptr) {
        m_pBtnViewListType->SetEnabled(!bIsComputerView);
    }

    //切换排序模式
    if (m_pBtnViewSort != nullptr) {
        m_pBtnViewSort->SetEnabled(!bIsComputerView);
    }
}

void MainForm::SwitchToTabBoxViewType(TabBoxViewType tabBoxViewType)
{
    m_tabBoxViewType = tabBoxViewType;
    if (m_pTabBox != nullptr) {
        m_pTabBox->SelectItem((size_t)tabBoxViewType);
    }
}

void MainForm::SwitchToDataViewType(DataViewType dataViewType)
{
    if (m_dataViewType == dataViewType) {
        return;
    }
    m_dataViewType = dataViewType;
    //更新图片列表和视图样式
    ui::ImageListPtr spImageList;
    ui::ListCtrl* pListCtrl = nullptr;
    if (m_pExplorerView != nullptr) {
        pListCtrl = m_pExplorerView->GetListCtrl();
    }
    ui::UiSize szItemSize;
    if (pListCtrl != nullptr) {
        switch (m_dataViewType) {
        case DataViewType::kIconViewBig:
            pListCtrl->SetListCtrlType(ui::ListCtrlType::Icon);
            spImageList = pListCtrl->GetImageList(ui::ListCtrlType::Icon);
            if (spImageList != nullptr) {
                spImageList->SetImageSize(ui::UiSize(64, 64), Dpi(), true);
            }
            szItemSize = pListCtrl->GetIconView()->GetItemSize();
            szItemSize.cy = Dpi().GetScaleInt(40 + 4 + 4 + 64);
            pListCtrl->GetIconView()->SetItemSize(szItemSize);
            break;
        case DataViewType::kIconViewMedium:
            pListCtrl->SetListCtrlType(ui::ListCtrlType::Icon);
            spImageList = pListCtrl->GetImageList(ui::ListCtrlType::Icon);
            if (spImageList != nullptr) {
                spImageList->SetImageSize(ui::UiSize(32, 32), Dpi(), true);
            }
            szItemSize = pListCtrl->GetIconView()->GetItemSize();
            szItemSize.cy = Dpi().GetScaleInt(40 + 4 + 4 + 32);
            pListCtrl->GetIconView()->SetItemSize(szItemSize);
            break;
        case DataViewType::kIconViewSmall:
            pListCtrl->SetListCtrlType(ui::ListCtrlType::Icon);
            spImageList = pListCtrl->GetImageList(ui::ListCtrlType::Icon);
            if (spImageList != nullptr) {
                spImageList->SetImageSize(ui::UiSize(20, 20), Dpi(), true);
            }
            szItemSize = pListCtrl->GetIconView()->GetItemSize();
            szItemSize.cy = Dpi().GetScaleInt(40 + 4 + 4 + 20);
            pListCtrl->GetIconView()->SetItemSize(szItemSize);
            break;
        case DataViewType::kListViewBig:
            pListCtrl->SetListCtrlType(ui::ListCtrlType::List);
            spImageList = pListCtrl->GetImageList(ui::ListCtrlType::List);
            if (spImageList != nullptr) {
                spImageList->SetImageSize(ui::UiSize(64, 64), Dpi(), true);
            }
            szItemSize.cx = Dpi().GetScaleInt(300);
            szItemSize.cy = Dpi().GetScaleInt(8 + 64);
            pListCtrl->GetListView()->SetItemSize(szItemSize);
            pListCtrl->GetListView()->SetTextSingleLine(false);
            break;
        case DataViewType::kListViewMedium:
            pListCtrl->SetListCtrlType(ui::ListCtrlType::List);
            spImageList = pListCtrl->GetImageList(ui::ListCtrlType::List);
            if (spImageList != nullptr) {
                spImageList->SetImageSize(ui::UiSize(32, 32), Dpi(), true);
            }
            szItemSize.cx = Dpi().GetScaleInt(280);
            szItemSize.cy = Dpi().GetScaleInt(8 + 32);
            pListCtrl->GetListView()->SetItemSize(szItemSize);
            pListCtrl->GetListView()->SetTextSingleLine(false);
            break;
        case DataViewType::kListViewSmall:
            pListCtrl->SetListCtrlType(ui::ListCtrlType::List);
            spImageList = pListCtrl->GetImageList(ui::ListCtrlType::List);
            if (spImageList != nullptr) {
                spImageList->SetImageSize(ui::UiSize(20, 20), Dpi(), true);
            }
            szItemSize.cx = Dpi().GetScaleInt(260);
            szItemSize.cy = Dpi().GetScaleInt(36);
            pListCtrl->GetListView()->SetItemSize(szItemSize);
            pListCtrl->GetListView()->SetTextSingleLine(true);
            break;
        case DataViewType::kReprortView:
            pListCtrl->SetListCtrlType(ui::ListCtrlType::Report);
            break;
        case DataViewType::kPictureView:
            break;
        default:
            break;
        }
    }

    //刷新当前显示内容
    if ((m_pTree != nullptr) && (m_pTreeNode != nullptr)) {
        m_pTree->RefreshFolderContents(m_pTreeNode, nullptr);
    }
}

MainForm::DataViewType MainForm::GetDataViewType() const
{
    return m_dataViewType;
}

void MainForm::StartDirectoryWatcher(const ui::FilePath& directory)
{
    //回调在UI线程中执行（已做防抖处理）
    m_directoryWatcher.Start(directory, ToWeakCallback([this]() {
        OnDirectoryChanged();
        }));
}

void MainForm::StopDirectoryWatcher()
{
    m_directoryWatcher.Stop();
}

void MainForm::OnDirectoryChanged()
{
    if (m_bComputerView || (m_pTree == nullptr)) {
        //"计算机"视图没有监听目录内容的变化
        return;
    }
    ui::TreeNode* pTreeNode = m_pTreeNode;
    if ((pTreeNode == nullptr) || !m_pTree->IsValidTreeNode(pTreeNode)) {
        return;
    }
    //只刷新当前目录的树节点：文件列表按路径做增量更新，不会全量重建
    BeginLoading();
    if (!m_pTree->RefreshTreeNode(pTreeNode, nullptr)) {
        EndLoading();
    }
}

void MainForm::BeginLoading()
{
    if ((m_pStatusLoading == nullptr) || m_bLoading) {
        //控件不存在，或者已经处于加载状态时，不重复处理
        return;
    }
    m_bLoading = true;
    ++m_nLoadingSerial;
    m_pStatusLoading->SetVisible(true);

    //启动超时定时器（异常情况下自动隐藏提示，避免提示一直显示在状态栏）
    const uint32_t nSerial = m_nLoadingSerial;
    m_loadingTimerFlag.Cancel();
    m_nLoadingTimerId = ui::GlobalManager::Instance().Timer().AddTimer(m_loadingTimerFlag.GetWeakFlag(),
        [this, nSerial]() {
            OnLoadingTimeout(nSerial);
        }, kLoadingTimeoutMs, 1);
}

void MainForm::EndLoading()
{
    if (!m_bLoading) {
        return;
    }
    m_bLoading = false;
    ++m_nLoadingSerial;
    m_loadingTimerFlag.Cancel();
    if (m_nLoadingTimerId != 0) {
        ui::GlobalManager::Instance().Timer().RemoveTimer(m_nLoadingTimerId);
        m_nLoadingTimerId = 0;
    }
    if (m_pStatusLoading != nullptr) {
        m_pStatusLoading->SetVisible(false);
    }
}

void MainForm::OnLoadingTimeout(uint32_t nSerial)
{
    if (!m_bLoading || (nSerial != m_nLoadingSerial)) {
        //加载已经结束（或者已经开始了新的加载），忽略本次超时回调
        return;
    }
    m_nLoadingTimerId = 0;
    EndLoading();
}

void MainForm::SetTreeMultiSelect(bool bMultiSelect)
{
    //目录树（多选模式下，节点显示复选框）
    if (m_pTree != nullptr) {
        m_pTree->SetMultiSelect(bMultiSelect);
    }
    //切换为单选模式时，多余的选择项会被取消，这里刷新状态栏中的选中项统计
    UpdateStatusBar();
}

bool MainForm::OnTreeMultiSelectChanged(const ui::EventArgs& msg)
{
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(msg.GetSender());
    if (pCheckBox != nullptr) {
        SetTreeMultiSelect(pCheckBox->IsSelected());
    }
    return true;
}

ui::FilePath MainForm::GetSettingsFilePath() const
{
    //保存在程序目录下的user子目录中（跨平台，且不依赖系统的用户配置目录）
    ui::FilePath filePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    filePath.JoinFilePath(ui::FilePath(_T("user")));
    filePath.JoinFilePath(ui::FilePath(_T("TreeView_settings.ini")));
    return filePath;
}

void MainForm::LoadSettings()
{
    SettingsMap settings;
    ReadSettingsFile(GetSettingsFilePath(), settings);
    if (settings.empty()) {
        return;
    }

    //1. 视图模式（图标视图、列表视图、详细信息视图、图片列表视图）
    const int32_t nDataViewType = GetSettingInt(settings, "view_type", -1);
    if ((nDataViewType >= (int32_t)DataViewType::kIconViewBig) &&
        (nDataViewType <= (int32_t)DataViewType::kPictureView)) {
        SwitchToDataViewType((DataViewType)nDataViewType);
    }

    //2. 左侧目录树的宽度
    const int32_t nTreeWidth = GetSettingInt(settings, "tree_width", -1);
    if (nTreeWidth > 0) {
        ui::Control* pTreePanel = FindControl(_T("tree_panel"));
        if (pTreePanel != nullptr) {
            //限制在合理范围内，避免窗口很小时保存下来的宽度导致目录树不可见
            const int32_t nMinTreeWidth = 60;
            const int32_t nMaxTreeWidth = 1024;
            const int32_t nWidth = std::max(nMinTreeWidth, std::min(nTreeWidth, nMaxTreeWidth));
            pTreePanel->SetFixedWidth(ui::UiFixedInt(nWidth), true, true);
        }
    }

    //3. 列表中各列的宽度
    if (m_pExplorerView != nullptr) {
        RestoreColumnWidths(m_pExplorerView->GetListCtrl(), settings, "explorer_col_");
    }
    if (m_pComputerView != nullptr) {
        RestoreColumnWidths(m_pComputerView->GetListCtrl(), settings, "computer_col_");
    }

    //4. 排序列（只对文件列表视图有效）
    const int32_t nSortColumn = GetSettingInt(settings, "sort_column", -1);
    const int32_t nSortUp = GetSettingInt(settings, "sort_up", 1);
    if ((nSortColumn >= (int32_t)ExplorerView::ExplorerViewColumn::kName) &&
        (nSortColumn <= (int32_t)ExplorerView::ExplorerViewColumn::kSize)) {
        if (m_pExplorerView != nullptr) {
            m_pExplorerView->SortByColumn((ExplorerView::ExplorerViewColumn)nSortColumn, nSortUp != 0);
        }
    }
}

void MainForm::SaveSettings() const
{
    std::string settingsText;
    settingsText += "# TreeView 示例的用户设置（程序自动生成，删除该文件后恢复默认设置）\n";

    //视图模式
    AppendSettingInt(settingsText, "view_type", (int32_t)m_dataViewType);

    //排序列
    int32_t nSortColumn = -1;
    int32_t nSortUp = 1;
    if (m_pExplorerView != nullptr) {
        ExplorerView::ExplorerViewColumn viewColumn = ExplorerView::ExplorerViewColumn::kName;
        bool bSortUp = true;
        if (m_pExplorerView->GetSortColumnInfo(viewColumn, bSortUp)) {
            nSortColumn = (int32_t)viewColumn;
            nSortUp = bSortUp ? 1 : 0;
        }
    }
    AppendSettingInt(settingsText, "sort_column", nSortColumn);
    AppendSettingInt(settingsText, "sort_up", nSortUp);

    //左侧目录树的宽度
    int32_t nTreeWidth = -1;
    ui::Control* pTreePanel = FindControl(_T("tree_panel"));
    if (pTreePanel != nullptr) {
        nTreeWidth = Dpi().GetUnscaleInt(pTreePanel->GetWidth());
    }
    AppendSettingInt(settingsText, "tree_width", nTreeWidth);

    //列表中各列的宽度
    if (m_pExplorerView != nullptr) {
        SaveColumnWidths(m_pExplorerView->GetListCtrl(), settingsText, "explorer_col_");
    }
    if (m_pComputerView != nullptr) {
        SaveColumnWidths(m_pComputerView->GetListCtrl(), settingsText, "computer_col_");
    }

    const ui::FilePath filePath = GetSettingsFilePath();
    //创建设置文件所在的目录（不存在时自动创建）
    ui::FilePathUtil::CreateDirectories(filePath.GetParentPath().ToString());
    const std::vector<uint8_t> fileData(settingsText.begin(), settingsText.end());
    ui::FileUtil::WriteFileData(filePath, fileData);
}

void MainForm::RestoreColumnWidths(ui::ListCtrl* pListCtrl, const SettingsMap& settings,
                                   const std::string& keyPrefix)
{
    if (pListCtrl == nullptr) {
        return;
    }
    const size_t nColumnCount = pListCtrl->GetColumnCount();
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
        const int32_t nWidth = GetSettingInt(settings, keyPrefix + std::to_string(nColumnIndex), -1);
        if (nWidth > 0) {
            //限制最小宽度，避免列宽过小导致内容不可见
            const int32_t nMinColumnWidth = 40;
            pListCtrl->SetColumnWidth(nColumnIndex, std::max(nWidth, nMinColumnWidth), true);
        }
    }
}

void MainForm::SaveColumnWidths(ui::ListCtrl* pListCtrl, std::string& settingsText, const std::string& keyPrefix) const
{
    if (pListCtrl == nullptr) {
        return;
    }
    const size_t nColumnCount = pListCtrl->GetColumnCount();
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
        //列宽保存为未缩放的值，恢复时再按当前DPI进行缩放
        const int32_t nWidth = Dpi().GetUnscaleInt(pListCtrl->GetColumnWidth(nColumnIndex));
        AppendSettingInt(settingsText, keyPrefix + std::to_string(nColumnIndex), nWidth);
    }
}
