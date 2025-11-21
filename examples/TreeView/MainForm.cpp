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
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kUserHome, _T("主文件夹"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDesktop, _T("桌面"));
    ui::TreeNode* pDocumentsNode = m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDocuments, _T("文档"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kPictures, _T("图片"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kMusic, _T("音乐"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kVideos, _T("视频"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDownloads, _T("下载"));

    //显示磁盘
    ui::TreeNode* pComputerNode = m_pTree->ShowAllDiskNodes(_T("计算机"), _T("文件系统"));
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
    if (m_bCanAddBackForward && m_pTree->IsValidTreeNode(m_pTreeNode)) {
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
            ui::DirectoryTree::ClearPathInfoList(*fileList);
        }
        if (folderList != nullptr) {
            ui::DirectoryTree::ClearPathInfoList(*fileList);
        }
        return;
    }
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->SetAddressPath(currentPath.ToString());
    }
    std::vector<ui::DirectoryTree::PathInfo> pathList;
    if (folderList != nullptr) {
        pathList = *folderList;
    }
    if (fileList != nullptr) {
        pathList.insert(pathList.end(), fileList->begin(), fileList->end());
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
    //更新界面状态
    UpdateCommandUI();
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
    SwitchToTabBoxViewType(TabBoxViewType::kComputerView);
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->SetAddressPath(_T(""));
    }

    //显示计算机视图的内容
    if (m_pComputerView != nullptr) {
        m_pComputerView->ShowMyComputerContents(diskInfoList);
    }   

    //保存显示的树节点
    SetShowTreeNode(pTreeNode);   

    //更新界面状态
    UpdateCommandUI();
}

void MainForm::SelectSubPath(const ui::FilePath& filePath)
{
    if (!filePath.IsExistsDirectory()) {
        //如果文件夹不存在，报错
        DString errMsg = _T("路径不存在：");
        errMsg += filePath.ToString();
        ui::SystemUtil::ShowMessageBox(this, errMsg.c_str(), _T("错误信息"));
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
        DString errMsg = _T("输入的路径不存在：");
        errMsg += text;
        ui::SystemUtil::ShowMessageBox(this, errMsg.c_str(), _T("错误信息"));
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
