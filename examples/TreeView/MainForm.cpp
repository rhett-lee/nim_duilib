#include "MainForm.h"
#include "MainThread.h"

MainForm::MainForm():
    m_fileList(this),
    m_pTree(nullptr),
    m_pAddressBar(nullptr),
    m_pListBox(nullptr),
    m_pBtnUp(nullptr),
    m_pBtnForward(nullptr),
    m_pBtnBack(nullptr),
    m_bCanAddBackForward(true),
    m_pTreeNode(nullptr),
    m_pTabBox(nullptr),
    m_pComputerListCtrl(nullptr),
    m_nRemoveIconCallbackId(0)
{
}

MainForm::~MainForm()
{
    ui::GlobalManager::Instance().Icon().DetachRemoveIconEvent(m_nRemoveIconCallbackId);
    ClearDiskInfoList(m_diskInfoList);
    m_diskInfoList.clear();
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
    //挂载图标删除事件
    m_nRemoveIconCallbackId = ui::GlobalManager::Instance().Icon().AttachRemoveIconEvent(ui::UiBind(&MainForm::OnRemoveIcon, this, std::placeholders::_1));

    m_pTree = dynamic_cast<ui::DirectoryTree*>(FindControl(_T("tree")));
    ASSERT(m_pTree != nullptr);
    if (m_pTree == nullptr) {
        return;
    }
    m_pAddressBar = dynamic_cast<ui::RichEdit*>(FindControl(_T("file_path")));
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->AttachReturn(UiBind(&MainForm::OnAddressBarReturn, this, std::placeholders::_1));
    }
    m_pTabBox = dynamic_cast<ui::TabBox*>(FindControl(_T("main_view_tab_box")));
    m_pComputerListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("computer_view")));
    InitializeComputerViewHeader();
    if (m_pComputerListCtrl != nullptr) {
        //关联图片列表
        ui::ImageListPtr pImageList = std::make_shared<ui::ImageList>();
        pImageList->SetImageSize(ui::UiSize(16, 16), Dpi(), true);
        m_pComputerListCtrl->SetImageList(ui::ListCtrlType::Report, pImageList);//该指针的资源生命周期由ListCtrl内部管理
        
        //挂载列表项鼠标双击事件
        m_pComputerListCtrl->AttachDoubleClick(UiBind(&MainForm::OnComuterViewDoubleClick, this, std::placeholders::_1));
    }
    m_pListBox = dynamic_cast<ui::VirtualListBox*>(FindControl(_T("file_view")));
    ASSERT(m_pListBox != nullptr);
    if (m_pListBox != nullptr) {
        m_pListBox->SetDataProvider(&m_fileList);
    }

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

void MainForm::OnShowFolderContents(ui::TreeNode* pTreeNode, const ui::FilePath& path,
                                    const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList,
                                    const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList)
{
    ui::GlobalManager::Instance().AssertUIThread();
    if (m_pTabBox != nullptr) {
        m_pTabBox->SelectItem((size_t)FormViewType::kFileView);
    }
    if ((pTreeNode == nullptr) || (m_pTree == nullptr)) {
        return;
    }
    if (!m_pTree->IsValidTreeNode(pTreeNode)) {
        return;
    }
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->SetText(path.ToString());
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
            m_fileList.SetFileList(pathList);
        }
    }
    else {
        //多选，不校验
        SetShowTreeNode(pTreeNode);
        m_fileList.SetFileList(pathList);
    }
    //更新界面状态
    UpdateCommandUI();
}

void MainForm::OnShowMyComputerContents(ui::TreeNode* pTreeNode,
                                        const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList)
{
    ui::GlobalManager::Instance().AssertUIThread();
    if (m_pTabBox != nullptr) {
        m_pTabBox->SelectItem((size_t)FormViewType::kComputerView);
    }
    if ((pTreeNode == nullptr) || (m_pTree == nullptr)) {
        ClearDiskInfoList(diskInfoList);
        return;
    }
    if (!m_pTree->IsValidTreeNode(pTreeNode)) {
        ClearDiskInfoList(diskInfoList);
        return;
    }
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->SetText(_T(""));
    }
    SetShowTreeNode(pTreeNode);
    FillMyComputerContents(diskInfoList);

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

bool MainForm::OnAddressBarReturn(const ui::EventArgs& msg)
{
    DString text;
    if (m_pAddressBar != nullptr) {
        text = m_pAddressBar->GetText();
        ui::StringUtil::Trim(text);
    }
    if (text.empty()) {
        return true;
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
    }
    else {
        //如果文件夹不存在，报错
        DString errMsg = _T("输入的路径不存在：");
        errMsg += text;
        ui::SystemUtil::ShowMessageBox(this, errMsg.c_str(), _T("错误信息"));
    }
    return true;
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
}

void MainForm::InitializeComputerViewHeader()
{
    if (m_pComputerListCtrl == nullptr) {
        return;
    }
    ui::ListCtrlHeaderItem* pHeaderItem = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    ui::ListCtrlColumn columnInfo;
    columnInfo.text = _T("名称");
    columnInfo.nColumnWidth = 200;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kName] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("磁盘类型");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kType] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("分区类型");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kPartitionType] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("总大小");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kTotalSpace] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("可用空间");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kFreeSpace] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("已用");
    columnInfo.nColumnWidth = 80;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kUsedPercent] = pHeaderItem->GetColumnId();
#else
    ui::ListCtrlColumn columnInfo;
    columnInfo.text = _T("文件系统");
    columnInfo.nColumnWidth = 200;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kName] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("设备类型");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kType] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("分区类型");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kPartitionType] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("总大小");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kTotalSpace] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("可用空间");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kFreeSpace] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("已用");
    columnInfo.nColumnWidth = 80;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kUsedPercent] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("挂载点");
    columnInfo.nColumnWidth = 200;
    pHeaderItem = m_pComputerListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ComputerViewColumn::kMountOn] = pHeaderItem->GetColumnId();
#endif
}

void MainForm::FillMyComputerContents(const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList)
{
    ui::GlobalManager::Instance().AssertUIThread();
    ClearDiskInfoList(m_diskInfoList);
    m_diskInfoList = diskInfoList;
    if ((m_pComputerListCtrl == nullptr)){
        return;
    }
    m_pComputerListCtrl->DeleteAllDataItems();
    ui::ImageListPtr pImageList;
    if (m_pComputerListCtrl != nullptr) {
        pImageList = m_pComputerListCtrl->GetImageList(ui::ListCtrlType::Report);
    }
    if (pImageList != nullptr) {
        pImageList->Clear();
    }

    ui::ListCtrlSubItemData itemData;
    size_t nItemIndex = 0;
    size_t nSubItemIndex = 0;
#ifdef DUILIB_BUILD_FOR_WIN
    for (size_t nIndex = 0; nIndex < diskInfoList.size(); ++nIndex) {
        const ui::DirectoryTree::DiskInfo& diskInfo = diskInfoList[nIndex];   
        nItemIndex = m_pComputerListCtrl->AddDataItem(itemData);
        if (ui::Box::IsValidItemIndex(nItemIndex)) {
            //记录关联关系
            m_pComputerListCtrl->SetDataItemUserData(nItemIndex, nIndex);
            //设置图标
            if (pImageList != nullptr) {
                DString iconString = ui::GlobalManager::Instance().Icon().GetIconString(diskInfo.m_nIconID);
                if (!iconString.empty()) {
                    int32_t nImageId = pImageList->AddImageString(iconString, Dpi());
                    m_iconToImageMap[diskInfo.m_nIconID] = nImageId;
                    m_pComputerListCtrl->SetDataItemImageId(nItemIndex, nImageId);
                }
            }

            ui::ListCtrlSubItemData subItemData;
            subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;

            subItemData.text = diskInfo.m_displayName;
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kName);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //名称
            //设置"名称"列的排序方式
            m_pComputerListCtrl->SetSubItemUserDataN(nItemIndex, nSubItemIndex, nIndex);
            m_pComputerListCtrl->SetSubItemUserDataS(nItemIndex, nSubItemIndex, diskInfo.m_filePath.ToString());
            m_pComputerListCtrl->SetColumnSortFlag(nSubItemIndex, ui::ListCtrlSubItemSortFlag::kSortByUserDataN);

            subItemData.nTextFormat = ui::DrawStringFormat::TEXT_CENTER | ui::DrawStringFormat::TEXT_VCENTER;

            subItemData.text = diskInfo.m_volumeType;
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kType);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //磁盘类型

            subItemData.text = diskInfo.m_fileSystem;
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kPartitionType);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //分区类型

            subItemData.text = FormatDiskSpace(diskInfo.m_totalBytes);
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kTotalSpace);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //总大小

            subItemData.text = FormatDiskSpace(diskInfo.m_freeBytes);
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kFreeSpace);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //可用空间

            subItemData.text = FormatUsedPercent(diskInfo.m_totalBytes, diskInfo.m_freeBytes);
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kUsedPercent);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //已用百分比
        }
    }
#else
    for (size_t nIndex = 0; nIndex < diskInfoList.size(); ++nIndex) {
        const ui::DirectoryTree::DiskInfo& diskInfo = diskInfoList[nIndex];        
        nItemIndex = m_pComputerListCtrl->AddDataItem(itemData);
        if (ui::Box::IsValidItemIndex(nItemIndex)) {
            //记录关联关系
            m_pComputerListCtrl->SetDataItemUserData(nItemIndex, nIndex);

            //设置图标
            if (pImageList != nullptr) {
                DString iconString = ui::GlobalManager::Instance().Icon().GetIconString(diskInfo.m_nIconID);
                if (!iconString.empty()) {
                    int32_t nImageId = pImageList->AddImageString(iconString, Dpi());
                    m_iconToImageMap[diskInfo.m_nIconID] = nImageId;
                    m_pComputerListCtrl->SetDataItemImageId(nItemIndex, nImageId);
                }
            }

            ui::ListCtrlSubItemData subItemData;
            subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
            subItemData.text = diskInfo.m_displayName;
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kName);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //文件系统

            subItemData.nTextFormat = ui::DrawStringFormat::TEXT_CENTER | ui::DrawStringFormat::TEXT_VCENTER;

            subItemData.text = GetDeviceTypeString(diskInfo.m_deviceType);
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kType);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //设备类型

            subItemData.text = diskInfo.m_fileSystem;
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kPartitionType);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //分区类型

            subItemData.text = FormatDiskSpace(diskInfo.m_totalBytes);
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kTotalSpace);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //总大小

            subItemData.text = FormatDiskSpace(diskInfo.m_freeBytes);
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kFreeSpace);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //可用空间

            subItemData.text = FormatUsedPercent(diskInfo.m_totalBytes, diskInfo.m_freeBytes);
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kUsedPercent);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //已用百分比

            subItemData.text = diskInfo.m_mountOn;
            subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
            nSubItemIndex = GetColumnIndex(ComputerViewColumn::kMountOn);
            m_pComputerListCtrl->SetSubItemData(nItemIndex, nSubItemIndex, subItemData); //挂载点
        }
    }
#endif
}

#ifndef DUILIB_BUILD_FOR_WIN
DString MainForm::GetDeviceTypeString(ui::DirectoryTree::DeviceType deviceType) const
{
    DString deviceTypeString = _T("未知");
    switch (deviceType) {
    case ui::DirectoryTree::DeviceType::HDD:
        deviceTypeString = _T("机械硬盘");
        break;
    case ui::DirectoryTree::DeviceType::SSD:
        deviceTypeString = _T("SATA固态硬盘");
        break;
    case ui::DirectoryTree::DeviceType::NVME:
        deviceTypeString = _T("NVMe固态硬盘");
        break;
    case ui::DirectoryTree::DeviceType::USB:
        deviceTypeString = _T("USB存储");
        break;
    case ui::DirectoryTree::DeviceType::SD_CARD:
        deviceTypeString = _T("SD卡");
        break;
    case ui::DirectoryTree::DeviceType::CDROM:
        deviceTypeString = _T("CD/DVD");
        break;
    case ui::DirectoryTree::DeviceType::LOOP:
        deviceTypeString = _T("LOOP虚拟存储");
        break;
    case ui::DirectoryTree::DeviceType::VIRT_DISK:
        deviceTypeString = _T("虚拟存储");
        break;
    case ui::DirectoryTree::DeviceType::RAMDISK:
        deviceTypeString = _T("内存盘");
        break;
    case ui::DirectoryTree::DeviceType::NFS:
        deviceTypeString = _T("NFS");
        break;
    case ui::DirectoryTree::DeviceType::SHARE:
        deviceTypeString = _T("共享文件夹");
        break;
    default:
        break;
    }
    return deviceTypeString;
}
#endif

DString MainForm::FormatDiskSpace(uint64_t nSpace) const
{
    DString value;
    if (nSpace > 1 * 1024 * 1024 * 1024) {
        //GB
        double total_gb = static_cast<double>(nSpace) / (1024 * 1024 * 1024);
        value = ui::StringUtil::Printf(_T("%.01lf GB"), total_gb);
    }
    else if (nSpace > 1 * 1024 * 1024) {
        //MB
        double total_mb = static_cast<double>(nSpace) / (1024 * 1024);
        value = ui::StringUtil::Printf(_T("%.01lf MB"), total_mb);
    }
    else if (nSpace > 1 * 1024) {
        //KB
        double total_kb = static_cast<double>(nSpace) / (1024);
        value = ui::StringUtil::Printf(_T("%.01lf KB"), total_kb);
    }
    else if (nSpace == 0) {
        value = _T("0");
    }
    else {
        //B
        value = ui::StringUtil::Printf(_T("%d B"), (int32_t)nSpace);
    }
    return value;
}

DString MainForm::FormatUsedPercent(uint64_t nTotalSpace, uint64_t nFreeSpace) const
{
    DString value;
    if ((nFreeSpace <= nTotalSpace) && (nTotalSpace != 0)) {
        double fPercent = static_cast<double>(nTotalSpace - nFreeSpace) / nTotalSpace;
        value = ui::StringUtil::Printf(_T("%.01lf%%"), fPercent * 100);
    }
    return value;
}

bool MainForm::OnComuterViewDoubleClick(const ui::EventArgs& msg)
{
    if ((m_pComputerListCtrl != nullptr) && (msg.wParam != 0) && (m_pTree != nullptr) && (m_pTreeNode != nullptr)) {
        size_t nItemIndex = msg.lParam;
        size_t nIndex = m_pComputerListCtrl->GetDataItemUserData(nItemIndex);
        if (nIndex < m_diskInfoList.size()) {
            const ui::DirectoryTree::DiskInfo& diskInfo = m_diskInfoList[nIndex];
            if (!diskInfo.m_filePath.IsEmpty() && diskInfo.m_filePath.IsExistsDirectory() && m_pTree->IsMyComputerNode(m_pTreeNode)) {
                //进入所选的目录
                m_pTree->SelectSubPath(m_pTreeNode, diskInfo.m_filePath, nullptr);
            }
        }
    }
    return true;
}

void MainForm::ClearDiskInfoList(const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList) const
{
    for (const ui::DirectoryTree::DiskInfo& diskInfo : diskInfoList) {
        if (!diskInfo.m_bIconShared) {            
            ui::GlobalManager::Instance().Icon().RemoveIcon(diskInfo.m_nIconID);
        }
    }
}

size_t MainForm::GetColumnIndex(ComputerViewColumn nOriginIndex) const
{
    size_t nColumnIndex = ui::Box::InvalidIndex;
    auto iter = m_columnIdMap.find(nOriginIndex);
    if (iter != m_columnIdMap.end()) {
        size_t nColumnId = iter->second;
        if (m_pComputerListCtrl != nullptr) {
            nColumnIndex = m_pComputerListCtrl->GetColumnIndex(nColumnId);
        }
    }
    return nColumnIndex;
}

void MainForm::OnRemoveIcon(uint32_t nIconId)
{
    if (!ui::GlobalManager::Instance().IsInUIThread()) {
        //如果不是在主线程中执行，则转到主线程去执行
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, this->ToWeakCallback([this, nIconId]() {
                OnRemoveIcon(nIconId);
            }));
        return;
    }

    //在主线程中执行
    ui::GlobalManager::Instance().AssertUIThread();
    auto iter = m_iconToImageMap.find(nIconId);
    if (iter != m_iconToImageMap.end()) {
        int32_t nImageId = iter->second;
        m_iconToImageMap.erase(iter);

        ui::ImageListPtr pImageList;
        if (m_pComputerListCtrl != nullptr) {
            pImageList = m_pComputerListCtrl->GetImageList(ui::ListCtrlType::Report);
        }
        if (pImageList != nullptr) {
            pImageList->RemoveImageString(nImageId);
        }
    }
}
