#include "MainForm.h"
#include "MainThread.h"
#include "duilib/Utils/DiskUtils_Windows.h"
#include <ShellApi.h>

MainForm::MainForm():
    m_fileList(this),
    m_pTree(nullptr),
    m_pListBox(nullptr),
    m_hShell32Dll(nullptr)
{
}

MainForm::~MainForm()
{
    for (auto p : m_folderList) {
        ui::GlobalManager::Instance().Icon().RemoveIcon(p->m_nIconID);
        delete p;
    }
    if (m_hShell32Dll != nullptr) {
        ::FreeLibrary(m_hShell32Dll);
        m_hShell32Dll = nullptr;
    }
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
    m_pTree = dynamic_cast<ui::TreeView*>(FindControl(_T("tree")));
    ASSERT(m_pTree != nullptr);
    m_pListBox = dynamic_cast<ui::VirtualListBox*>(FindControl(_T("list")));
    ASSERT(m_pListBox != nullptr);
    if (m_pListBox != nullptr) {
        m_pListBox->SetDataProvider(&m_fileList);
    }
    
    //显示虚拟路径
    ShowVirtualDirectoryNode(CSIDL_DESKTOP, FOLDERID_Desktop, _T("桌面"));
    ShowVirtualDirectoryNode(CSIDL_PERSONAL, FOLDERID_Documents, _T("文档"));
    ShowVirtualDirectoryNode(CSIDL_MYPICTURES, FOLDERID_Pictures, _T("图片"));
    ShowVirtualDirectoryNode(CSIDL_MYMUSIC, FOLDERID_Music, _T("音乐"));
    ShowVirtualDirectoryNode(CSIDL_MYVIDEO, FOLDERID_Videos, _T("视频"));
    ShowVirtualDirectoryNode(-1, FOLDERID_Downloads, _T("下载")); 

    //显示磁盘
    ui::TreeNode* pFirstDiskNode = ShowAllDiskNode();

    //在磁盘前面，放一个横线分隔符
    if (pFirstDiskNode != nullptr) {
        ui::Control* pLineControl = new ui::Control(this);
        pLineControl->SetClass(_T("splitline_hor_level1"));
        pLineControl->SetMargin(ui::UiMargin(12, 8, 12, 8), true);
        m_pTree->InsertControlBeforeNode(pFirstDiskNode, pLineControl);
    }

   // ShowAllDiskNode();
}

void MainForm::InsertTreeNodes(ui::TreeNode* pTreeNode,
                               const ui::FilePath& path,
                               const std::vector<FolderStatus>& fileList,
                               bool isFolder)
{
    //校验pTreeNode是否存在（由于经过子线程）
    if (pTreeNode == nullptr) {
        return;
    }
    size_t itemIndex = m_pTree->GetItemIndex(pTreeNode);
    if (!ui::Box::IsValidItemIndex(itemIndex)) {
        //节点已经不存在了，返回
        return;
    }
    
    ui::FilePath folderPath;
    for (const FolderStatus& folder : fileList) {
        folderPath = ui::FilePathUtil::JoinFilePath(path, folder.m_path);
        InsertTreeNode(pTreeNode, folder.m_path.ToString(), folderPath, isFolder, folder.m_nIconID);
    }
}

ui::TreeNode* MainForm::InsertTreeNode(ui::TreeNode* pTreeNode,
                                       const DString& displayName,
                                       const ui::FilePath& path,
                                       bool isFolder,
                                       uint32_t nIconID)
{
    if (m_pTree == nullptr) {
        return nullptr;
    }

    ui::TreeNode* node = new ui::TreeNode(this);
    node->SetClass(_T("tree_node"));//在"global.xml"中定义
    node->SetText(displayName);
        
    FolderStatus* pFolder = new FolderStatus;
    pFolder->m_path = path;
    pFolder->m_nIconID = nIconID;
    pFolder->m_pTreeNode = node;
    m_folderList.push_back(pFolder);
    node->SetUserDataID((size_t)pFolder);

    node->SetBkIconID(nIconID, 16, true);//设置树节点的关联图标(图标大小与CheckBox的原图大小相同，都是16*16)
    node->AttachExpand(UiBind(&MainForm::OnTreeNodeExpand, this, std::placeholders::_1));
    node->AttachClick(UiBind(&MainForm::OnTreeNodeClick, this, std::placeholders::_1));
    node->AttachSelect(UiBind(&MainForm::OnTreeNodeSelect, this, std::placeholders::_1));

    if (isFolder) {
        pFolder->m_bShow = false;
        node->SetExpand(false, false);
    }
    else {
        pFolder->m_bShow = true;
        node->SetExpand(true, false);
    }

    if (pTreeNode == nullptr) {
        pTreeNode = m_pTree->GetRootNode();
    }
    if (pTreeNode != nullptr) {
        pTreeNode->AddChildNode(node);
    }
    return node;
}

void MainForm::ShowVirtualDirectoryNode(int csidl, REFKNOWNFOLDERID rfid, const DString& name)
{
    if (m_hShell32Dll == nullptr) {
        m_hShell32Dll = ::LoadLibrary(_T("Shell32.dll"));
    }

    typedef HRESULT (CALLBACK *PFN_SHGetKnownFolderPath)( REFKNOWNFOLDERID rfid,
                                                 DWORD            dwFlags,
                                                 HANDLE           hToken,
                                                 PWSTR * ppszPath );
    typedef HRESULT (CALLBACK *PFN_SHGetKnownFolderIDList)( REFKNOWNFOLDERID rfid,
                                                   DWORD            dwFlags,
                                                   HANDLE           hToken,
                                                   PIDLIST_ABSOLUTE * ppidl );

    PFN_SHGetKnownFolderPath pfnSHGetKnownFolderPath = (PFN_SHGetKnownFolderPath)::GetProcAddress(m_hShell32Dll, "SHGetKnownFolderPath");
    PFN_SHGetKnownFolderIDList pfnSHGetKnownFolderIDList = (PFN_SHGetKnownFolderIDList)::GetProcAddress(m_hShell32Dll, "SHGetKnownFolderIDList");

    WCHAR folder[MAX_PATH] = { 0 };
    LPITEMIDLIST lpPidl = nullptr;

    if (pfnSHGetKnownFolderPath != nullptr) {
        PWSTR ppszPath = nullptr;
        pfnSHGetKnownFolderPath(rfid, 0, nullptr, &ppszPath);
        wcscpy_s(folder, ppszPath);
        if (ppszPath != nullptr) {
            ::CoTaskMemFree(ppszPath);
        }
    }

    if (pfnSHGetKnownFolderIDList != nullptr) {
        pfnSHGetKnownFolderIDList(rfid, 0, nullptr, &lpPidl);
    }

    if (folder[0] == _T('\0')) {
        if (csidl < 0) {
            return;
        }
        if (!::SHGetSpecialFolderPathW(nullptr, folder, csidl, FALSE)) {
            return;
        }
    }

    if (lpPidl == nullptr) {
        if (csidl < 0) {
            return;
        }
        HRESULT hr = ::SHGetSpecialFolderLocation(NativeWnd()->GetHWND(), csidl, &lpPidl);
        if ((hr != S_OK) || (lpPidl == nullptr)) {
            return;
        }
    }

    SHFILEINFO shFileInfo = { 0 };
    if (::SHGetFileInfo((LPCTSTR)lpPidl,
                        0,
                        &shFileInfo,
                        sizeof(SHFILEINFO),
                        SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON)) {
        DString displayName = ui::StringConvert::LocalToT(shFileInfo.szDisplayName);
        if (displayName.empty()) {
            displayName = name;
        }
        ui::FilePath folderName(folder);
        uint32_t nIconID = ui::GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
        InsertTreeNode(nullptr, displayName, folderName, true, nIconID);
        if (shFileInfo.hIcon != nullptr) {
            ::DestroyIcon(shFileInfo.hIcon);
            shFileInfo.hIcon = nullptr;
        }
    }

    if (lpPidl != nullptr) {
        ::CoTaskMemFree(lpPidl);
    }
}

ui::TreeNode* MainForm::ShowAllDiskNode()
{
    ui::TreeNode* pFirstNode = nullptr;
    std::vector<DString> driveList;
    DiskUtils::GetLogicalDriveList(driveList);
    for (auto iter = driveList.begin(); iter != driveList.end(); ++iter) {
        DString driverName = *iter;
        // 过滤A:盘和B:盘
        if (ui::StringUtil::IsEqualNoCase(driverName, _T("A:\\")) ||
            ui::StringUtil::IsEqualNoCase(driverName, _T("B:\\"))) {
            continue;
        }

        uint32_t type = ::GetDriveType(driverName.c_str());
        if ((type != DRIVE_FIXED) && (type != DRIVE_REMOVABLE)) {
            continue;
        }

        ui::FilePath driverPath(driverName);
        if (!driverPath.IsExistsPath()) {
            continue;
        }

        SHFILEINFO shFileInfo;
        ZeroMemory(&shFileInfo, sizeof(SHFILEINFO));
        if (::SHGetFileInfo(driverName.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_DISPLAYNAME)) {
            uint32_t nIconID = ui::GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
            ui::TreeNode* pNewNode = InsertTreeNode(nullptr, shFileInfo.szDisplayName, driverPath, true, nIconID);
            if (pFirstNode == nullptr) {
                pFirstNode = pNewNode;
            }
            if (shFileInfo.hIcon != nullptr) {
                ::DestroyIcon(shFileInfo.hIcon);
                shFileInfo.hIcon = nullptr;
            }
        }
    }
    return pFirstNode;
}

bool MainForm::OnTreeNodeExpand(const ui::EventArgs& args)
{
    ui::TreeNode* pTreeNode = dynamic_cast<ui::TreeNode*>(args.GetSender());
    ASSERT(pTreeNode != nullptr);
    if (pTreeNode != nullptr) {
        FolderStatus* pFolder = (FolderStatus*)pTreeNode->GetUserDataID();
        auto iter = std::find(m_folderList.begin(), m_folderList.end(), pFolder);
        if (iter != m_folderList.end()) {
            //加载子目录列表
            if (!pFolder->m_bShow) {
                pFolder->m_bShow = true;
                ShowSubFolders(pTreeNode, pFolder->m_path);                
            }
        }
    }
    return true;
}

void MainForm::CheckExpandTreeNode(ui::TreeNode* pTreeNode, const ui::FilePath& filePath)
{
    if (pTreeNode == nullptr) {
        return;
    }
    FolderStatus* pFolder = (FolderStatus*)pTreeNode->GetUserDataID();
    auto iter = std::find(m_folderList.begin(), m_folderList.end(), pFolder);
    if (iter == m_folderList.end()) {
        return;
    }

    if (!pFolder->m_bShow) {
        //加载子目录列表
        pFolder->m_bShow = true;
        ShowSubFolders(pTreeNode, pFolder->m_path);
    }
    else {
        //展开子目录
        if (!pTreeNode->IsExpand()) {
            pTreeNode->SetExpand(true, true);
        }
    }
    if (filePath.IsEmpty()) {
        return;
    }
    //由于ShowSubFolders是在子线程中执行的，所以这里也要先发给子线程，再转给UI线程，保证时序正确
    ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadWorker, ToWeakCallback([this, pTreeNode, filePath]() {
        //这段代码在工作线程中执行，枚举目录内容完成后，然后发给UI线程添加到树节点上
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, pTreeNode, filePath]() {
            //这段代码在UI线程中执行
            for (const FolderStatus* folder : m_folderList) {
                if (folder->m_path == filePath) {
                    ui::TreeNode* pSubTreeNode = folder->m_pTreeNode;
                    if (pSubTreeNode != nullptr) {
                        pSubTreeNode->Activate(nullptr);
                    }
                }
            }
        }));
    }));
}

void MainForm::ShowSubFolders(ui::TreeNode* pTreeNode, const ui::FilePath& path)
{
    ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadWorker, ToWeakCallback([this, path, pTreeNode]() {
        //这段代码在工作线程中执行，枚举目录内容完成后，然后发给UI线程添加到树节点上
        ui::FilePath findPath = ui::FilePathUtil::JoinFilePath(path, ui::FilePath(_T("*.*")));
        WIN32_FIND_DATAW findData;
        HANDLE hFile = ::FindFirstFileW(findPath.ToStringW().c_str(), &findData);
        if (hFile == INVALID_HANDLE_VALUE) {
            return;
        }

        std::vector<FolderStatus> folderList;
        std::vector<FolderStatus> fileList;

        do {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
                (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
                continue;
            }

            if (ui::StringUtil::IsEqualNoCase(findData.cFileName, L".") ||
                ui::StringUtil::IsEqualNoCase(findData.cFileName, L"..")) {
                continue;
            }

            ui::FilePath folderPath = ui::FilePathUtil::JoinFilePath(path, ui::FilePath(findData.cFileName));

            SHFILEINFOW shFileInfo;
            ZeroMemory(&shFileInfo, sizeof(SHFILEINFOW));
            if (::SHGetFileInfoW(folderPath.ToStringW().c_str(), 0, &shFileInfo, sizeof(SHFILEINFOW), SHGFI_ICON | SHGFI_SMALLICON)) {
                uint32_t nIconID = ui::GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
                if (shFileInfo.hIcon != nullptr) {
                    ::DestroyIcon(shFileInfo.hIcon);
                    shFileInfo.hIcon = nullptr;
                }

                if (folderPath.IsExistsDirectory()) {
                    //目录
                    folderList.push_back({ ui::FilePath(findData.cFileName), false, nIconID });
                }
                else {
                    //文件
                    fileList.push_back({ ui::FilePath(findData.cFileName), false, nIconID });
                }
            }
        } while (::FindNextFileW(hFile, &findData));
        ::FindClose(hFile);
        hFile = INVALID_HANDLE_VALUE;

        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, path, pTreeNode, folderList]() {
            //这段代码在UI线程中执行
            InsertTreeNodes(pTreeNode, path, folderList, true);
            if (!pTreeNode->IsExpand()) {
                pTreeNode->SetExpand(true, true);
            }
        }));
    }));
}

bool MainForm::OnTreeNodeClick(const ui::EventArgs& args)
{
    ui::TreeNode* pTreeNode = dynamic_cast<ui::TreeNode*>(args.GetSender());
    ASSERT(pTreeNode != nullptr);
    if (pTreeNode != nullptr) {
        FolderStatus* pFolder = (FolderStatus*)pTreeNode->GetUserDataID();
        auto iter = std::find(m_folderList.begin(), m_folderList.end(), pFolder);
        if (iter != m_folderList.end()) {
            //加载子目录列表到右侧区域
            ShowFolderContents(pTreeNode, pFolder->m_path);
        }
    }
    return true;
}

bool MainForm::OnTreeNodeSelect(const ui::EventArgs& args)
{
    if ((m_pTree != nullptr) && m_pTree->IsMultiSelect()) {
        //多选的时候，不响应选择事件
        return true;
    }
    return OnTreeNodeClick(args);
}

void MainForm::ShowFolderContents(ui::TreeNode* pTreeNode, const ui::FilePath& path)
{
    ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadWorker, ToWeakCallback([this, pTreeNode, path]() {
        //这段代码在工作线程中执行，枚举目录内容完成后，然后发给UI线程添加到树节点上
        ui::FilePath findPath = ui::FilePathUtil::JoinFilePath(path, ui::FilePath(L"*.*"));
        WIN32_FIND_DATAW findData;
        HANDLE hFile = ::FindFirstFileW(findPath.ToStringW().c_str(), &findData);
        if (hFile == INVALID_HANDLE_VALUE) {
            return;
        }

        std::vector<FolderStatus> folderList;
        std::vector<FolderStatus> fileList;

        do {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
                (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
                continue;
            }

            if (ui::StringUtil::IsEqualNoCase(findData.cFileName, L".") ||
                ui::StringUtil::IsEqualNoCase(findData.cFileName, L"..")) {
                continue;
            }

            ui::FilePath folderPath = ui::FilePathUtil::JoinFilePath(path, ui::FilePath(findData.cFileName));

            SHFILEINFOW shFileInfo;
            ZeroMemory(&shFileInfo, sizeof(SHFILEINFOW));
            if (::SHGetFileInfoW(folderPath.ToStringW().c_str(), 0, &shFileInfo, sizeof(SHFILEINFOW), SHGFI_ICON | SHGFI_LARGEICON)) {
                uint32_t nIconID = ui::GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
                if (folderPath.IsExistsDirectory()) {
                    //目录
                    folderList.push_back({ ui::FilePath(findData.cFileName), false, nIconID });
                }
                else {
                    //文件
                    fileList.push_back({ ui::FilePath(findData.cFileName), false, nIconID });
                }
                if (shFileInfo.hIcon != nullptr) {
                    ::DestroyIcon(shFileInfo.hIcon);
                    shFileInfo.hIcon = nullptr;
                }
            }
        } while (::FindNextFileW(hFile, &findData));
        ::FindClose(hFile);
        hFile = INVALID_HANDLE_VALUE;

        std::vector<FileInfo> pathList;
        ui::FilePath folderPath;
        for (const FolderStatus& folder : folderList) {
            folderPath = ui::FilePathUtil::JoinFilePath(path, folder.m_path);
            pathList.push_back({ folder.m_path, folderPath, true, folder.m_nIconID });
        }
        for (const FolderStatus& folder : fileList) {
            folderPath = ui::FilePathUtil::JoinFilePath(path, folder.m_path);
            pathList.push_back({ folder.m_path, folderPath, false, folder.m_nIconID });
        }

        //发给UI线程
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, pTreeNode, path, pathList]() {
            //这段代码在UI线程中执行
            if (m_pTree != nullptr) {
                if (!m_pTree->IsMultiSelect()) {
                    //单选，进行校验
                    if (pTreeNode->IsSelected()) {
                        m_fileList.SetFileList(pTreeNode, pathList);
                    }
                }
                else {
                    //多选，不校验
                    m_fileList.SetFileList(pTreeNode, pathList);
                }
            }            
        }));
    }));
}
