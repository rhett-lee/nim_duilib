#include "MainForm.h"
#include "MainThread.h"
#include "ComputerView.h"
#include "SimpleFileView.h"
#include "ExplorerView.h"

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
    m_bComputerView(false),
    m_nDiskCount(0),
    m_nFolderCount(0),
    m_nFileCount(0),
    m_nTotalItemCount(0),
    m_nShowItemCount(0),
    m_dataViewType(DataViewType::kReprortView),
    m_tabBoxViewType(TabBoxViewType::kComputerView)
{
}

MainForm::~MainForm()
{
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
    m_pTree = dynamic_cast<ui::DirectoryTree*>(FindControl(_T("tree")));
    ASSERT(m_pTree != nullptr);
    if (m_pTree == nullptr) {
        return;
    }
    m_pAddressBar = dynamic_cast<ui::AddressBar*>(FindControl(_T("file_path")));
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->AttachPathChanged(UiBind(&MainForm::OnAddressBarPathChanged, this, std::placeholders::_1));
        m_pAddressBar->AttachPathClick(UiBind(&MainForm::OnAddressBarPathClick, this, std::placeholders::_1));
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
    m_pStatusInfo = dynamic_cast<ui::Label*>(FindControl(_T("status_info")));
    m_pStatusMode = dynamic_cast<ui::Label*>(FindControl(_T("status_mode")));

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
            SwithListType(point, m_pBtnViewListType);
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
            SwithSortMode(point, m_pBtnViewSort);
            return true;
            });
    }

    UpdateCommandUI();

    //挂载事件
    m_pTree->AttachShowMyComputerContents(ui::UiBind(&MainForm::OnShowMyComputerContents, this, std::placeholders::_1, std::placeholders::_2));
    m_pTree->AttachShowFolderContents(ui::UiBind(&MainForm::OnShowFolderContents, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
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

void MainForm::Refresh()
{
    if (m_pTree == nullptr) {
        return;
    }
    ui::StdClosure finishCallback = ToWeakCallback([this]() {
            OnRefresh();
        });
    m_pTree->RefreshTree(finishCallback);
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
        //切换到其它目录时，清除搜索条件
        ClearSearchText();
    }
    m_bComputerView = false;
    m_currentPath = currentPath;
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->SetAddressPath(currentPath.ToString());
    }

    //汇总文件夹和文件，并按搜索条件过滤
    m_nFolderCount = (folderList != nullptr) ? (int32_t)folderList->size() : 0;
    m_nFileCount = (fileList != nullptr) ? (int32_t)fileList->size() : 0;
    m_nTotalItemCount = m_nFolderCount + m_nFileCount;
    std::vector<ui::DirectoryTree::PathInfo> pathList;
    pathList.reserve((size_t)m_nTotalItemCount);
    std::vector<ui::DirectoryTree::PathInfo> unusedList;//被搜索条件过滤掉的，尚未使用的项目
    const bool bFilter = !m_searchText.empty();
    auto addItems = [this, &pathList, &unusedList, bFilter](const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& pathInfoList) {
        if (pathInfoList == nullptr) {
            return;
        }
        for (const ui::DirectoryTree::PathInfo& pathInfo : *pathInfoList) {
            if (bFilter && (ui::StringUtil::MakeLowerString(pathInfo.m_displayName).find(m_searchText) == DString::npos)) {
                //不匹配搜索条件
                unusedList.push_back(pathInfo);
                continue;
            }
            pathList.push_back(pathInfo);
        }
    };
    addItems(folderList);
    addItems(fileList);
    m_nShowItemCount = (int32_t)pathList.size();
    if (!unusedList.empty()) {
        //释放未被使用的项目的图标资源
        ui::DirectoryTree::ClearPathInfoList(unusedList);
    }

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

    ShowFileList(currentPath, pathList);

    //更新界面状态
    UpdateCommandUI();
    UpdateStatusBar();
}

void MainForm::ShowFileList(const ui::FilePath& currentPath, const std::vector<ui::DirectoryTree::PathInfo>& pathList)
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
            m_pSimpleFileView->SetFileList(currentPath, pathList, selectedPath);
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
            m_pExplorerView->SetFileList(currentPath, pathList, selectedPath);
        }
    }
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
    ApplySearchFilter();
    return true;
}

void MainForm::ApplySearchFilter()
{
    if ((m_pTree == nullptr) || (m_pTreeNode == nullptr)) {
        return;
    }
    //重新加载当前节点对应目录的内容：回调中会按最新的搜索条件过滤后显示
    //（不缓存原始列表，避免列表中的图标资源被文件视图释放后无法再次使用）
    m_pTree->RefreshFolderContents(m_pTreeNode, nullptr);
}

void MainForm::ClearSearchText()
{
    m_searchText.clear();
    if (m_pSearchBox != nullptr) {
        if (!m_pSearchBox->GetSearchText().empty()) {
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
        DString strInfo;
        if (m_bComputerView) {
            const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_DISKS"));
            strInfo = ui::StringUtil::Printf(strFormat.c_str(), m_nShowItemCount);
        }
        else if (m_searchText.empty()) {
            const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_ITEMS"));
            strInfo = ui::StringUtil::Printf(strFormat.c_str(), m_nTotalItemCount, m_nFolderCount, m_nFileCount);
        }
        else {
            const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TREEVIEW_STATUS_FILTERED"));
            strInfo = ui::StringUtil::Printf(strFormat.c_str(), m_nTotalItemCount, m_nShowItemCount);
        }
        m_pStatusInfo->SetText(strInfo);
    }

    //视图与排序方式
    if (m_pStatusMode != nullptr) {
        DString strMode;
        if (!m_bComputerView) {
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

void MainForm::OnShowMyComputerContents(ui::TreeNode* pTreeNode,
                                        const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList)
{
    ui::GlobalManager::Instance().AssertUIThread();
    if ((pTreeNode == nullptr) || (m_pTree == nullptr) || !m_pTree->IsValidTreeNode(pTreeNode)) {
        std::vector<ui::DirectoryTree::DiskInfo> tempDiskInfoList(diskInfoList);
        ui::DirectoryTree::ClearDiskInfoList(tempDiskInfoList);
        return;
    }
    if (pTreeNode != m_pTreeNode) {
        //切换到其它节点时，清除搜索条件
        ClearSearchText();
    }
    SwitchToTabBoxViewType(TabBoxViewType::kComputerView);
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->SetAddressPath(_T(""));
    }
    m_bComputerView = true;
    m_currentPath = ui::FilePath();
    m_nDiskCount = (int32_t)diskInfoList.size();

    //显示计算机视图的内容（按搜索条件过滤磁盘）
    if (m_pComputerView != nullptr) {
        if (m_searchText.empty()) {
            m_nShowItemCount = m_nDiskCount;
            m_pComputerView->ShowMyComputerContents(diskInfoList);
        }
        else {
            std::vector<ui::DirectoryTree::DiskInfo> filteredDiskList;
            std::vector<ui::DirectoryTree::DiskInfo> unusedDiskList;//被搜索条件过滤掉的磁盘
            for (const ui::DirectoryTree::DiskInfo& diskInfo : diskInfoList) {
                if (ui::StringUtil::MakeLowerString(diskInfo.m_displayName).find(m_searchText) != DString::npos) {
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

    //更新界面状态
    UpdateCommandUI();
    UpdateStatusBar();
}

void MainForm::SelectSubPath(const ui::FilePath& filePath)
{
    if (!filePath.IsExistsDirectory()) {
        //如果文件夹不存在，报错
        DString errMsg = _T("Path not exists:");
        errMsg += filePath.ToString();
        ui::SystemUtil::ShowMessageBox(this, errMsg.c_str(), _T("Error"));
        return;
    }

    if (m_pTree != nullptr) {
        m_pTree->SelectSubPath(m_pTreeNode, filePath, nullptr);
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
        DString errMsg = _T("Path not exists:");
        errMsg += text;
        ui::SystemUtil::ShowMessageBox(this, errMsg.c_str(), _T("Error"));
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
    if (m_pTreeNode != nullptr) {
        ui::TreeNode* pTreeNode = m_pTreeNode->GetParentNode();
        if ((m_pTree != nullptr) && (pTreeNode != nullptr)) {
            m_bCanAddBackForward = true;
            m_pTree->SelectTreeNode(pTreeNode);
        }
    }
}

void MainForm::ShowBack()
{
    //后退
    if (m_pTree == nullptr) {
        return;
    }
    if (m_pTree->IsValidTreeNode(m_pTreeNode)) {
        m_forwardStack.push(m_pTreeNode);
    }

    while (!m_backStack.empty()) {
        ui::TreeNode* pTreeNode = m_backStack.top();
        m_backStack.pop();
        if (m_pTree->IsValidTreeNode(pTreeNode)) {
            m_bCanAddBackForward = false;
            m_pTree->SelectTreeNode(pTreeNode);
            break;
        }
    }
}

void MainForm::ShowForward()
{
    if (m_pTree == nullptr) {
        return;
    }
    if (m_pTree->IsValidTreeNode(m_pTreeNode)) {
        m_backStack.push(m_pTreeNode);
    }

    while (!m_forwardStack.empty()) {
        ui::TreeNode* pTreeNode = m_forwardStack.top();
        m_forwardStack.pop();
        if (m_pTree->IsValidTreeNode(pTreeNode)) {
            m_bCanAddBackForward = false;
            m_pTree->SelectTreeNode(pTreeNode);
            break;
        }
    }
}

void MainForm::SwithListType(const ui::UiPoint& point, ui::Control* pRelatedControl)
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
        pSelectBtn->SetBkImage(_T("ui-item-symbolic.svg"));
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

void MainForm::SwithSortMode(const ui::UiPoint& point, ui::Control* pRelatedControl)
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
            pSortBtn->SetBkImage(_T("ui-item-symbolic.svg"));
        }
        if (pSortColumnBtn != nullptr) {
            pSortColumnBtn->SetBkImage(_T("ui-item-symbolic.svg"));
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
