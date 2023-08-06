#include "MainForm.h"
#include "DiskUtils.h"
#include <Shlobj.h>
#include <ShellApi.h>

const std::wstring MainForm::kClassName = L"MainForm";

MainForm::MainForm():
    m_pTree(nullptr),
    m_pListBox(nullptr)
{
}

MainForm::~MainForm()
{
    for (auto p : m_folderList) {
        ::DestroyIcon(p->hIcon);
        delete p;
    }
}

std::wstring MainForm::GetSkinFolder()
{
	return L"tree_view";
}

std::wstring MainForm::GetSkinFile()
{
	return L"tree_view.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
	return kClassName;
}

void MainForm::OnInitWindow()
{
	m_pTree = dynamic_cast<ui::TreeView*>(FindControl(L"tree"));
    ASSERT(m_pTree != nullptr);
    m_pListBox = dynamic_cast<ui::VirtualListBox*>(FindControl(L"list"));
    ASSERT(m_pListBox != nullptr);
    if (m_pListBox != nullptr) {
        m_pListBox->SetDataProvider(&m_fileList);
    }
	ShowDesktopNode();
    ShowAllDiskNode();
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

void MainForm::InsertTreeNode(ui::TreeNode* pTreeNode, 
                              const std::wstring& displayName,
                              const std::wstring& path,
                              bool isFolder,
                              HICON hIcon)
{
    if (m_pTree != NULL) {
        ui::TreeNode* node = new ui::TreeNode;
        node->SetClass(L"listitem");
        node->SetFixedHeight(ui::UiFixedInt(20), true);
        node->SetText(displayName);
        node->AttachExpand(nbase::Bind(&MainForm::OnTreeNodeExpand, this, std::placeholders::_1));
        node->AttachClick(nbase::Bind(&MainForm::OnTreeNodeClick, this, std::placeholders::_1));

        FolderStatus* pFolder = new FolderStatus;
        pFolder->path = path;
        pFolder->hIcon = hIcon;
        m_folderList.push_back(pFolder);
        node->SetUserDataID((size_t)pFolder);

        if (isFolder) {
            pFolder->bShow = false;
            node->SetExpand(false, false);
        }
        else {
            pFolder->bShow = true;
            node->SetExpand(true, false);
        }

        if (pTreeNode == nullptr) {
            pTreeNode = m_pTree->GetRootNode();
        }
        if (pTreeNode != nullptr) {
            pTreeNode->AddChildNode(node);
        }
    }
}

void MainForm::ShowDesktopNode()
{
    TCHAR desktopFolder[MAX_PATH] = { 0 };
    if (!::SHGetSpecialFolderPath(nullptr, desktopFolder, CSIDL_DESKTOPDIRECTORY, FALSE)) {
        return;
    }

    LPITEMIDLIST lpPidlDesktop = nullptr;
    HRESULT hr = ::SHGetSpecialFolderLocation(GetHWND(), CSIDL_DESKTOP, &lpPidlDesktop);
    if ((hr != S_OK) || (lpPidlDesktop == nullptr)) {
        return;
    }

    SHFILEINFO shFileInfo = { 0 };
    std::wstring name;
    int32_t imageIndex = 0;
    if (::SHGetFileInfo((LPCTSTR)lpPidlDesktop,
        0,
        &shFileInfo,
        sizeof(SHFILEINFO),
        SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON)) {
        InsertTreeNode(nullptr, shFileInfo.szDisplayName, desktopFolder, true, shFileInfo.hIcon);
    }

    if (lpPidlDesktop != nullptr) {
        ::CoTaskMemFree(lpPidlDesktop);
    }
}

void MainForm::ShowAllDiskNode()
{
    std::vector<std::wstring> driveList;
    DiskUtils::GetLogicalDriveList(driveList);
    for (auto iter = driveList.begin(); iter != driveList.end(); ++iter) {
        std::wstring driverName = *iter;
        // 过滤A:盘和B:盘
        if (ui::StringHelper::IsEqualNoCase(driverName, L"A:\\") ||
            ui::StringHelper::IsEqualNoCase(driverName, L"B:\\")) {
            continue;
        }

        uint32_t type = ::GetDriveType(driverName.c_str());
        if ((type != DRIVE_FIXED) && (type != DRIVE_REMOVABLE)) {
            continue;
        }

        if (!ui::StringHelper::IsExistsPath(driverName)) {
            continue;
        }

        std::wstring name;
        int32_t imageIndex = 0;

        SHFILEINFO shFileInfo;
        ZeroMemory(&shFileInfo, sizeof(SHFILEINFO));
        if (::SHGetFileInfo(driverName.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_DISPLAYNAME)) {      
            InsertTreeNode(nullptr, shFileInfo.szDisplayName, driverName, true, shFileInfo.hIcon);
        }
    }
}

bool MainForm::OnTreeNodeExpand(const ui::EventArgs& args)
{
    ui::TreeNode* pTreeNode = dynamic_cast<ui::TreeNode*>(args.pSender);
    ASSERT(pTreeNode != nullptr);
    if (pTreeNode != nullptr) {
        FolderStatus* pFolder = (FolderStatus*)pTreeNode->GetUserDataID();
        auto iter = std::find(m_folderList.begin(), m_folderList.end(), pFolder);
        if (iter != m_folderList.end()) {
            //加载子目录列表
            if (!pFolder->bShow) {
                pFolder->bShow = true;
                ShowSubFolders(pTreeNode, pFolder->path);                
            }
        }
    }
    return true;
}

void MainForm::ShowSubFolders(ui::TreeNode* pTreeNode, const std::wstring& path)
{
    std::wstring findPath = ui::StringHelper::JoinFilePath(path, L"*.*");
    WIN32_FIND_DATA findData;
    HANDLE hFile = ::FindFirstFile(findPath.c_str(), &findData);
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

        if (ui::StringHelper::IsEqualNoCase(findData.cFileName, L".") ||
            ui::StringHelper::IsEqualNoCase(findData.cFileName, L"..")) {
            continue;
        }

        std::wstring folderPath = ui::StringHelper::JoinFilePath(path, findData.cFileName);

        SHFILEINFO shFileInfo;
        ZeroMemory(&shFileInfo, sizeof(SHFILEINFO));
        if (::SHGetFileInfo(folderPath.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES | SHGFI_ICON | SHGFI_SMALLICON)) {
            if ((shFileInfo.dwAttributes & SFGAO_FOLDER) == SFGAO_FOLDER) {
                //目录
                folderList.push_back({ findData.cFileName, false, shFileInfo.hIcon });
            }
            else {
                //文件
                fileList.push_back({ findData.cFileName, false, shFileInfo.hIcon });
            }         
        }
    } while (::FindNextFile(hFile, &findData));
    ::FindClose(hFile);
    hFile = INVALID_HANDLE_VALUE;

    std::wstring folderPath;
    for (const FolderStatus& folder : folderList) {
        folderPath = ui::StringHelper::JoinFilePath(path, folder.path);
        InsertTreeNode(pTreeNode, folder.path, folderPath, true, folder.hIcon);
    }
    /*for (const FolderStatus& folder : fileList) {
        folderPath = ui::StringHelper::JoinFilePath(path, folder.path);
        InsertTreeNode(pTreeNode, folder.path, folderPath, false, folder.hIcon);
    }*/
}

bool MainForm::OnTreeNodeClick(const ui::EventArgs& args)
{
    ui::TreeNode* pTreeNode = dynamic_cast<ui::TreeNode*>(args.pSender);
    ASSERT(pTreeNode != nullptr);
    if (pTreeNode != nullptr) {
        FolderStatus* pFolder = (FolderStatus*)pTreeNode->GetUserDataID();
        auto iter = std::find(m_folderList.begin(), m_folderList.end(), pFolder);
        if (iter != m_folderList.end()) {
            //加载子目录列表到右侧区域
            ShowFolderContents(pFolder->path);
        }
    }
    return true;
}

void MainForm::ShowFolderContents(const std::wstring& path)
{
    std::wstring findPath = ui::StringHelper::JoinFilePath(path, L"*.*");
    WIN32_FIND_DATA findData;
    HANDLE hFile = ::FindFirstFile(findPath.c_str(), &findData);
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

        if (ui::StringHelper::IsEqualNoCase(findData.cFileName, L".") ||
            ui::StringHelper::IsEqualNoCase(findData.cFileName, L"..")) {
            continue;
        }

        std::wstring folderPath = ui::StringHelper::JoinFilePath(path, findData.cFileName);

        SHFILEINFO shFileInfo;
        ZeroMemory(&shFileInfo, sizeof(SHFILEINFO));
        if (::SHGetFileInfo(folderPath.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES | SHGFI_ICON | SHGFI_SMALLICON)) {
            if ((shFileInfo.dwAttributes & SFGAO_FOLDER) == SFGAO_FOLDER) {
                //目录
                folderList.push_back({ findData.cFileName, false, shFileInfo.hIcon });
            }
            else {
                //文件
                fileList.push_back({ findData.cFileName, false, shFileInfo.hIcon });
            }
        }
    } while (::FindNextFile(hFile, &findData));
    ::FindClose(hFile);
    hFile = INVALID_HANDLE_VALUE;

    std::vector<FileInfo> pathList;
    std::wstring folderPath;
    for (const FolderStatus& folder : folderList) {
        folderPath = ui::StringHelper::JoinFilePath(path, folder.path);
        pathList.push_back({ folder.path, folderPath, true, folder.hIcon });
    }
    for (const FolderStatus& folder : fileList) {
        folderPath = ui::StringHelper::JoinFilePath(path, folder.path);
        pathList.push_back({ folder.path, folderPath, true, folder.hIcon });
    }
    m_fileList.SetFileList(pathList);
}
