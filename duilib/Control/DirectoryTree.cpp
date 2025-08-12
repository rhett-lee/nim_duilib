#include "DirectoryTree.h"
#include "DirectoryTreeImpl.h"
#include "duilib/Utils/FilePathUtil.h"
#include <set>

/** 计算机虚拟节点的识别字符串
*/
#define TREE_NODE_MYCOMPUTER _T("MyComputer")

namespace ui
{
DirectoryTree::DirectoryTree(Window* pWindow):
    TreeView(pWindow),
    m_nThreadIdentifier(ui::kThreadWorker),
#ifdef DUILIB_BUILD_FOR_WIN
    m_nSmallIconSize(16),
#else
    m_nSmallIconSize(20),
#endif
    m_nLargeIconSize(32),
    m_folderKey(0),
    m_bShowHidenFiles(false),
    m_bShowSystemFiles(false),
    m_defaultRefreshFinishCallback(nullptr)
{
    m_impl = new DirectoryTreeImpl(this);
}

DirectoryTree::~DirectoryTree()
{
    for (auto iter : m_folderMap) {
        DeleteFolderStatus(iter.second);
    }
    m_folderMap.clear();

    //移除所有的树节点，避免产生Destroy事件
    RemoveAllNodes();

    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }    
}

void DirectoryTree::DeleteFolderStatus(FolderStatus* pFolderStatus)
{
    if (pFolderStatus != nullptr) {
        if (!pFolderStatus->m_bIconShared) {
            GlobalManager::Instance().Icon().RemoveIcon(pFolderStatus->m_nIconID);
        }
        delete pFolderStatus;
    }
}

DString DirectoryTree::GetType() const { return DUI_CTR_DIRECTORY_TREE; }

void DirectoryTree::SetAttribute(const DString& strName, const DString& strValue)
{
    //支持的属性列表: 基类实现的直接转发
    if (strName == _T("small_icon_size")) {
        SetSmallIconSize(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("large_icon_size")) {
        SetLargeIconSize(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("show_hiden_files")) {
        SetShowHidenFiles(strValue == _T("true"));
    }
    else if (strName == _T("show_system_files")) {
        SetShowSystemFiles(strValue == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void DirectoryTree::ClearPathInfoList(std::vector<PathInfo>& pathList)
{
    for (const PathInfo& fileInfo : pathList) {
        if (!fileInfo.m_bIconShared) {
            ui::GlobalManager::Instance().Icon().RemoveIcon(fileInfo.m_nIconID);
        }
    }
    pathList.clear();
}

void DirectoryTree::ClearDiskInfoList(std::vector<DirectoryTree::DiskInfo>& diskList)
{
    for (const DirectoryTree::DiskInfo& diskInfo : diskList) {
        if (!diskInfo.m_bIconShared) {
            GlobalManager::Instance().Icon().RemoveIcon(diskInfo.m_nIconID);
        }
    }
    diskList.clear();
}

void DirectoryTree::SetSmallIconSize(int32_t nIconSize)
{
    if (nIconSize < 1) {
#ifdef DUILIB_BUILD_FOR_WIN
        nIconSize = 16;
#else
        nIconSize = 20;
#endif
    }
    m_nSmallIconSize = nIconSize;
}

int32_t DirectoryTree::GetSmallIconSize() const
{
    return m_nSmallIconSize;
}

void DirectoryTree::SetLargeIconSize(int32_t nIconSize)
{
    if (nIconSize < 1) {
        nIconSize = 32;
    }
    m_nLargeIconSize = nIconSize;
}

int32_t DirectoryTree::GetLargeIconSize() const
{
    return m_nLargeIconSize;
}

void DirectoryTree::SetShowHidenFiles(bool bShowHidenFiles)
{
    if(m_bShowHidenFiles != bShowHidenFiles) {
        m_bShowHidenFiles = bShowHidenFiles;
        if (IsInited()) {
            RefreshTree(nullptr);
        }
    }
}

bool DirectoryTree::IsShowHidenFiles() const
{
    return m_bShowHidenFiles;
}

void DirectoryTree::SetShowSystemFiles(bool bShowSystemFiles)
{
    if (m_bShowSystemFiles != bShowSystemFiles) {
        m_bShowSystemFiles = bShowSystemFiles;
        if (IsInited()) {
            RefreshTree(nullptr);
        }
    }
}

bool DirectoryTree::IsShowSystemFiles() const
{
    return m_bShowSystemFiles;
}

void DirectoryTree::SetThreadIdentifier(int32_t nThreadIdentifier)
{
    m_nThreadIdentifier = nThreadIdentifier;
}

void DirectoryTree::AttachShowFolderContents(ShowFolderContentsEvent callback)
{
    if (callback != nullptr) {
        m_callbackList.push_back(callback);
    }    
}

void DirectoryTree::AttachShowMyComputerContents(ShowMyComputerContentsEvent callback)
{
    if (callback != nullptr) {
        m_myComputerCallbackList.push_back(callback);
    }
}

TreeNode* DirectoryTree::ShowVirtualDirectoryNode(VirtualDirectoryType type, const DString& displayName, bool bDisplayNameIsID)
{
    FilePath filePath;
    DString folderName;
    uint32_t nIconID = 0;
    if (!m_impl->GetVirtualDirectoryInfo(type, filePath, folderName, nIconID)) {
        return nullptr;
    }
    if (folderName.empty()) {
        folderName = displayName;
    }
    else {
        bDisplayNameIsID = false;
    }
    return InsertTreeNode(nullptr, folderName, bDisplayNameIsID, filePath, true, false, nIconID, false);
}

TreeNode* DirectoryTree::ShowAllDiskNodes(const DString& computerName, const DString& fileSystemName)
{
    //基本结构:
    //  -计算机
    //     -C:\\盘
    //     -D:\\盘
    //     ...
    uint32_t nMyComputerIcon = m_impl->GetMyComputerIconID();
    TreeNode* pMyComputerNode = InsertTreeNode(nullptr, computerName, false, FilePath(), false, true, nMyComputerIcon, false);
    TreeNode* pFirstNode = pMyComputerNode; //返回计算机节点
    std::vector<DirectoryTree::PathInfo> pathInfoList;
    m_impl->GetRootPathInfoList(false, pathInfoList);
    UNUSED_VARIABLE(fileSystemName);
#ifndef DUILIB_BUILD_FOR_WIN
    bool bFirstNode = true;
#endif
    for (const DirectoryTree::PathInfo& pathInfo : pathInfoList) {
        if (pathInfo.m_filePath.IsEmpty()) {
            continue;
        }
        DString displayName = pathInfo.m_displayName;
#ifndef DUILIB_BUILD_FOR_WIN
        if (bFirstNode && (displayName == _T("/")) && !fileSystemName.empty()) {
            //替换为文件系统
            bFirstNode = false;
            displayName = fileSystemName;
        }
#endif
        TreeNode* pNewNode = InsertTreeNode(pMyComputerNode, displayName, false,
                                            pathInfo.m_filePath, pathInfo.m_bFolder, false,
                                            pathInfo.m_nIconID, pathInfo.m_bIconShared);
        if (pFirstNode == nullptr) {
            pFirstNode = pNewNode;
        }
    }
    return pFirstNode;
}

bool DirectoryTree::InsertLineBeforeNode(TreeNode* pNode, const DString& lineClassName)
{
    bool bRet = false;
    if (pNode != nullptr) {
        Control* pLineControl = new Control(GetWindow());
        if (!lineClassName.empty()) {
            pLineControl->SetClass(lineClassName);
        }
        else {
            pLineControl->SetClass(_T("splitline_hor_level1"));
            pLineControl->SetMargin(UiMargin(12, 8, 12, 8), true);
        }
        bRet = InsertControlBeforeNode(pNode, pLineControl);
    }
    return bRet;
}

TreeNode* DirectoryTree::InsertTreeNode(TreeNode* pParentTreeNode,
                                        const DString& displayName,
                                        bool bDisplayNameIsID,
                                        const FilePath& path,
                                        bool isFolder,
                                        bool bVirtualNode,
                                        uint32_t nIconID,
                                        bool bIconShared)
{
    TreeNode* node = new TreeNode(GetWindow());
    node->SetClass(_T("tree_node"));//在"global.xml"中定义
    if (bDisplayNameIsID) {
        node->SetTextId(displayName);
    }
    else {
        node->SetText(displayName);
    }

    FolderStatus* pFolder = new FolderStatus;
    FilePath filePath(path);
    if (isFolder) {
        filePath.NormalizeDirectoryPath();
    }
    else {
        filePath.NormalizeFilePath();
    }

    pFolder->m_filePath = filePath.ToString();
    pFolder->m_nIconID = nIconID;
    pFolder->m_bIconShared = bIconShared;
    pFolder->m_pTreeNode = node;

    size_t folderKey = ++m_folderKey;
    m_folderMap[folderKey] = pFolder;
    node->SetUserDataID(folderKey);
    if (bVirtualNode) {
        //识别标志
        node->SetDataID(TREE_NODE_MYCOMPUTER);
    }

    node->SetBkIconID(nIconID, GetSmallIconSize(), true);//设置树节点的关联图标(图标大小与CheckBox的原图大小相同)
    

    if (isFolder) {
        pFolder->m_bContentLoaded = false;
        node->SetExpand(false, false);
    }
    else {
        pFolder->m_bContentLoaded = true;
        node->SetExpand(true, false);
    }

    if (pParentTreeNode == nullptr) {
        pParentTreeNode = GetRootNode();
    }
    if (pParentTreeNode != nullptr) {
        pParentTreeNode->AddChildNode(node);
    }

    node->AttachExpand(UiBind(&DirectoryTree::OnTreeNodeExpand, this, std::placeholders::_1));
    node->AttachClick(UiBind(&DirectoryTree::OnTreeNodeClick, this, std::placeholders::_1));
    node->AttachDestroy(UiBind(&DirectoryTree::OnTreeNodeDestroy, this, std::placeholders::_1));
    return node;
}

bool DirectoryTree::IsMyComputerNode(TreeNode* pTreeNode) const
{
    if (pTreeNode != nullptr) {
        if (pTreeNode->GetDataID() == TREE_NODE_MYCOMPUTER) {
            FolderStatus* pFolder = GetFolderData(pTreeNode);
            if (pFolder != nullptr) {
                if (pFolder->m_filePath.empty()) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool DirectoryTree::OnTreeNodeExpand(const EventArgs& args)
{
    TreeNode* pTreeNode = dynamic_cast<TreeNode*>(args.GetSender());
    ASSERT(pTreeNode != nullptr);
    if (IsMyComputerNode(pTreeNode)) {
        //"计算机" 节点，不做任何处理
        return true;
    }
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    if (pFolder != nullptr) {
        //加载子目录列表
        if (!pFolder->m_bContentLoaded) {
            pFolder->m_bContentLoaded = true;
            ShowSubFolders(pTreeNode, FilePath(pFolder->m_filePath.c_str()), nullptr);
        }
    }
    return true;
}

bool DirectoryTree::OnTreeNodeClick(const EventArgs& args)
{
    TreeNode* pTreeNode = dynamic_cast<TreeNode*>(args.GetSender());
    ASSERT(pTreeNode != nullptr);
    if (IsMyComputerNode(pTreeNode)) {
        //"计算机" 节点
        ShowMyComputerContents(pTreeNode, nullptr);
    }
    else {
        //普通的文件夹节点
        FolderStatus* pFolder = GetFolderData(pTreeNode);
        if (pFolder != nullptr) {
            //加载子目录列表到右侧区域
            ShowFolderContents(pTreeNode, FilePath(pFolder->m_filePath.c_str()), nullptr);
        }
    }
    return true;
}

bool DirectoryTree::RefreshFolderContents(TreeNode* pTreeNode, StdClosure finishCallback)
{
    if (!IsValidTreeNode(pTreeNode)) {
        return false;
    }
    bool bRet = false;
    if (IsMyComputerNode(pTreeNode)) {
        //"计算机" 节点
        ShowMyComputerContents(pTreeNode, finishCallback);
        bRet = true;
    }
    else {
        //普通的文件夹节点
        FolderStatus* pFolder = GetFolderData(pTreeNode);
        if (pFolder != nullptr) {
            //加载子目录列表到右侧区域
            ShowFolderContents(pTreeNode, FilePath(pFolder->m_filePath.c_str()), finishCallback);
            bRet = true;
        }
    }
    return bRet;
}

bool DirectoryTree::OnTreeNodeDestroy(const EventArgs& args)
{
    if (m_folderMap.empty()) {
        return true;
    }
    TreeNode* pTreeNode = dynamic_cast<TreeNode*>(args.GetSender());
    if (pTreeNode != nullptr) {
        size_t folderKey = pTreeNode->GetUserDataID();
        auto iter = m_folderMap.find(folderKey);
        if (iter != m_folderMap.end()) {
            DeleteFolderStatus(iter->second);
            m_folderMap.erase(iter);
        }
    }
    return true;
}

void DirectoryTree::ShowSubFolders(TreeNode* pTreeNode, const FilePath& path, StdClosure finishCallback)
{
    //校验是否相同
    if (!IsPathSame(pTreeNode, path)) {
        ASSERT(0);
        return;
    }

    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, path, finishCallback]() {
            //在子线程中读取子目录数据
            PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            if (!treeNodeFlag.expired()) {
                m_impl->GetFolderContents(path, treeNodeFlag, false, *folderList, nullptr);
            }
            if (!treeNodeFlag.expired()) {
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, path, treeNodeFlag, pTreeNode, folderList, finishCallback]() {
                        //这段代码在UI线程中执行
                        bool bAdded = false;
                        if (!treeNodeFlag.expired()) {
                            bAdded = OnShowSubFolders(pTreeNode, path, folderList);
                        }
                        if ((folderList != nullptr) && !bAdded){
                            ClearPathInfoList(*folderList);
                        }
                        if (finishCallback) {
                            finishCallback();
                        }
                    }));
            }
            else {
                ClearPathInfoList(*folderList);
            }
        }));
}

bool DirectoryTree::OnShowSubFolders(TreeNode* pTreeNode, const FilePath& /*path*/, const PathInfoListPtr& folderList)
{
    GlobalManager::Instance().AssertUIThread();
    if (pTreeNode == nullptr) {
        return false;
    }
    size_t itemIndex = GetItemIndex(pTreeNode);
    if (!Box::IsValidItemIndex(itemIndex)) {
        return false;
    }
    if (folderList != nullptr) {
        for (const DirectoryTree::PathInfo& pathInfo : *folderList) {
            if (!pathInfo.m_filePath.IsEmpty()) {
                InsertTreeNode(pTreeNode, pathInfo.m_displayName, false,
                               pathInfo.m_filePath, pathInfo.m_bFolder, false,
                               pathInfo.m_nIconID, pathInfo.m_bIconShared);
            }
        }
    }
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    if (pFolder != nullptr) {
        //标记为已经展开，避免再重复展开
        pFolder->m_bContentLoaded = true;
    }
    if (!pTreeNode->IsExpand()) {
        pTreeNode->SetExpand(true, true);
    }
    return true;
}

bool DirectoryTree::OnShowSubFoldersEx(TreeNode* pTreeNode, const std::vector<FilePath>& filePathList, const std::vector<PathInfoListPtr>& folderListArray)
{
    GlobalManager::Instance().AssertUIThread();
    if (pTreeNode == nullptr) {
        return false;
    }
    size_t itemIndex = GetItemIndex(pTreeNode);
    if (!Box::IsValidItemIndex(itemIndex)) {
        return false;
    }
    ASSERT(filePathList.size() == folderListArray.size());
    if (filePathList.size() != folderListArray.size()) {
        return false;
    }

    std::vector<TreeNode*> treeNodes;
    treeNodes.push_back(pTreeNode);

    TreeNode* pParentTreeNode = pTreeNode;
    for (size_t index = 0; index < folderListArray.size(); ++index) {
        const PathInfoListPtr& folderList = folderListArray[index];
        bool bLastNode = index == (folderListArray.size() - 1);
        FilePath path = !bLastNode ? filePathList[index + 1] : FilePath();
        TreeNode* pNextParentTreeNode = nullptr;
        for (const DirectoryTree::PathInfo& pathInfo : *folderList) {
            if (pathInfo.m_filePath.IsEmpty()) {
                continue;
            }
            TreeNode* pNewTreeNode = InsertTreeNode(pParentTreeNode, pathInfo.m_displayName, false,
                                                    pathInfo.m_filePath, pathInfo.m_bFolder, false,
                                                    pathInfo.m_nIconID, pathInfo.m_bIconShared);
            if ((pNextParentTreeNode == nullptr) && (pNewTreeNode != nullptr) && !path.IsEmpty()) {
                FilePath checkPath = pathInfo.m_filePath;
                checkPath.NormalizeDirectoryPath();
                if (IsSamePath(path.ToString(), checkPath.ToString())) {
                    pNextParentTreeNode = pNewTreeNode;
                }                
            }
        }
        if (!bLastNode) {
            ASSERT(pNextParentTreeNode != nullptr);
            if (pNextParentTreeNode == nullptr) {
                break;
            }
            pParentTreeNode = pNextParentTreeNode;
            treeNodes.push_back(pParentTreeNode);
        }        
    }

    for (size_t index = 0; index < treeNodes.size(); ++index) {
        pParentTreeNode = treeNodes[index];
        if (!pParentTreeNode->IsExpand()) {
            pParentTreeNode->SetExpand(true, false);//不触发展开事件，避免重复加载
        }
    }

    for (auto iter : m_folderMap) {
        FolderStatus* pFolder = iter.second;
        for (size_t index = 0; index < treeNodes.size(); ++index) {
            if (pFolder->m_pTreeNode == treeNodes[index]) {
                //标记为已经展开，避免再重复展开
                pFolder->m_bContentLoaded = true;
                break;
            }
        }
    }

    //选择，并确保可见
    if (!treeNodes.empty()) {
        SelectTreeNode(treeNodes.back());
    }
    return true;
}

void DirectoryTree::ShowFolderContents(TreeNode* pTreeNode, const FilePath& path, StdClosure finishCallback)
{
    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, path, finishCallback]() {
            //在子线程中读取子目录数据
            PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            PathInfoListPtr fileList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            if (!treeNodeFlag.expired()) {
                m_impl->GetFolderContents(path, treeNodeFlag, true, *folderList, fileList.get());
            }
            if (!treeNodeFlag.expired()) {
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, path, treeNodeFlag, pTreeNode, folderList, fileList, finishCallback]() {
                        //这段代码在UI线程中执行
                        bool bAdded = false;
                        if (!treeNodeFlag.expired()) {
                            bAdded = OnShowFolderContents(pTreeNode, path, folderList, fileList);
                        }
                        if (!bAdded) {
                            ClearPathInfoList(*folderList);
                            ClearPathInfoList(*fileList);
                        }
                        if (finishCallback) {
                            finishCallback();
                        }
                    }));
            }
            else {
                ClearPathInfoList(*folderList);
                ClearPathInfoList(*fileList);
            }
        }));
}

bool DirectoryTree::OnShowFolderContents(TreeNode* pTreeNode, const FilePath& path,
                                         const PathInfoListPtr& folderList,
                                         const PathInfoListPtr& fileList)
{
    bool bHasCallback = false;
    GlobalManager::Instance().AssertUIThread();
    for (ShowFolderContentsEvent callback : m_callbackList) {
        if (callback) {
            callback(pTreeNode, path, folderList, fileList);
            bHasCallback = true;
        }
    }
    return bHasCallback;
}

void DirectoryTree::ShowMyComputerContents(TreeNode* pTreeNode, StdClosure finishCallback)
{
    if (!IsMyComputerNode(pTreeNode)) {
        return;
    }
    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, finishCallback]() {
        //在子线程中读取子目录数据
        std::shared_ptr<std::vector<ui::DirectoryTree::DiskInfo>> spDiskInfoList = std::make_shared<std::vector<ui::DirectoryTree::DiskInfo>>();
        if (!treeNodeFlag.expired()) {
            m_impl->GetDiskInfoList(treeNodeFlag, false, *spDiskInfoList);
        }        
        if (!treeNodeFlag.expired()) {
            GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, treeNodeFlag, pTreeNode, spDiskInfoList, finishCallback]() {
                    //这段代码在UI线程中执行
                    bool bAdded = false;
                    if (!treeNodeFlag.expired()) {
                        bAdded = OnShowMyComputerContents(pTreeNode, *spDiskInfoList);
                    }
                    if (!bAdded) {
                        ClearDiskInfoList(*spDiskInfoList);
                    }
                    if (finishCallback) {
                        finishCallback();
                    }
                }));
        }
        else {
            ClearDiskInfoList(*spDiskInfoList);
        }
        }));
}

bool DirectoryTree::OnShowMyComputerContents(TreeNode* pTreeNode,
                                             const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_myComputerCallbackList.empty()) {
        //没有设置回调函数，忽略
        return false;
    }

    for (ShowMyComputerContentsEvent callback : m_myComputerCallbackList) {
        if (callback) {
            callback(pTreeNode, diskInfoList);
        }
    }
    return true;
}

bool DirectoryTree::SelectSubPath(TreeNode* pTreeNode, FilePath subPath, StdClosure finishCallback)
{
    if (!IsValidTreeNode(pTreeNode)) {
        return false;
    }
    subPath.NormalizeDirectoryPath();
    if (subPath.IsEmpty() || !subPath.IsExistsDirectory()) {
        return false;
    }
    //校验是否在目录中
    FolderStatus* pFolderStatus = GetFolderData(pTreeNode);
    ASSERT(pFolderStatus != nullptr);
    if (pFolderStatus == nullptr) {
        return false;
    }
    if (!IsMyComputerNode(pTreeNode)) {
        if (!subPath.IsSubDirectory(FilePath(pFolderStatus->m_filePath.c_str()))) {
            //路径与树节点不是父子目录关系
            return false;
        }
    }

    pFolderStatus = nullptr;
    const UiString filePathString = subPath.ToString();
    for (auto iter : m_folderMap) {
        if ((iter.second->m_pTreeNode != pTreeNode) && !IsChildTreeNode(pTreeNode, iter.second->m_pTreeNode)) {
            //跳过非关联树节点
            continue;
        }
        if (IsSamePath(iter.second->m_filePath, filePathString)) {
            pFolderStatus = iter.second;
            break;
        }
    }
    if (pFolderStatus != nullptr) {
        //该目录已经存在于树的节点中
        TreeNode* pSubTreeNode = pFolderStatus->m_pTreeNode;
        if (pSubTreeNode != nullptr) {
            SelectTreeNode(pSubTreeNode);
        }
        if (finishCallback) {
            finishCallback();
        }
        return true;
    }

    //如果不存在，则逐级展开    
    std::vector<FilePath> filePathList;
    subPath.GetParentPathList(filePathList);
    for (FilePath& normalPath : filePathList) {
        normalPath.FormatPathAsDirectory();
    }

    TreeNode* pDestTreeNode = nullptr; //需要挂载的树节点
    for (auto pos : m_folderMap) {
        FolderStatus* pFolder = pos.second;
        if ((pFolder->m_pTreeNode != pTreeNode) && !IsChildTreeNode(pTreeNode, pFolder->m_pTreeNode)) {
            //跳过非关联树节点
            continue;
        }
        auto iter = filePathList.begin();
        while (iter != filePathList.end()) {
            const FilePath& checkPath = *iter;
            if (IsSamePath(pFolder->m_filePath, checkPath.ToString())) {
                //存在的
                pDestTreeNode = pFolder->m_pTreeNode;
                if (pFolder->m_bContentLoaded) {
                    //存在的，已经加载的，移除
                    iter = filePathList.erase(iter);
                }
                else {
                    ++iter;
                }
            }
            else {
                ++iter;
            }
        }
    }
    return OnSelectSubPath(pDestTreeNode, filePathList, finishCallback);
}

bool DirectoryTree::SelectPath(FilePath filePath, StdClosure finishCallback)
{
    if (filePath.IsEmpty()) {
        return false;
    }
    filePath.NormalizeDirectoryPath();
    if (!filePath.IsExistsDirectory()) {
        return false;
    }

    FolderStatus* pFolderStatus = GetFolderData(filePath);
    if (pFolderStatus != nullptr) {
        //该目录已经存在于树的节点中
        TreeNode* pSubTreeNode = pFolderStatus->m_pTreeNode;
        if (pSubTreeNode != nullptr) {
            SelectTreeNode(pSubTreeNode);
        }
        if (finishCallback) {
            finishCallback();
        }
        return true;
    }

    //如果不存在，则逐级展开    
    std::vector<FilePath> filePathList;
    filePath.GetParentPathList(filePathList);
    for (FilePath& normalPath : filePathList) {
        normalPath.FormatPathAsDirectory();
    }

    TreeNode* pTreeNode = nullptr;
    for (auto pos : m_folderMap) {
        FolderStatus* pFolder = pos.second;
        auto iter = filePathList.begin();
        while (iter != filePathList.end()) {
            const FilePath& checkPath = *iter;
            if (IsSamePath(pFolder->m_filePath, checkPath.ToString())) {
                //存在的
                pTreeNode = pFolder->m_pTreeNode;
                if (pFolder->m_bContentLoaded) {
                    //存在的，已经加载的，移除
                    iter = filePathList.erase(iter);
                }
                else {
                    ++iter;
                }
            }
            else {
                ++iter;
            }
        }
    }
    return OnSelectSubPath(pTreeNode, filePathList, finishCallback);
}

bool DirectoryTree::OnSelectSubPath(TreeNode* pTreeNode, std::vector<FilePath> filePathList, StdClosure finishCallback)
{
    ASSERT(pTreeNode != nullptr);
    if (pTreeNode == nullptr) {
        return false;
    }
    //移除冗余目录(过滤掉，不是树节点子目录的路径)
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    ASSERT(pFolder != nullptr);
    if (pFolder == nullptr) {
        return false;
    }
    FilePath destFilePath(pFolder->m_filePath.c_str());
    destFilePath.NormalizeDirectoryPath();
    std::vector<FilePath>::iterator iter = filePathList.begin();
    while (iter != filePathList.end()) {
        const FilePath& checkPath = *iter;
        if (IsSamePath(checkPath.ToString().c_str(), destFilePath.ToString().c_str())) {
            if (pFolder->m_bContentLoaded) {
                //如果已经展开，则移除
                iter = filePathList.erase(iter);
            }
            else {
                ++iter;
            }
        }
        else {
            if (!checkPath.IsSubDirectory(destFilePath)) {
                //如果不是子目录，移除
                iter = filePathList.erase(iter);
            }
            else {
                ++iter;
            }
        }
    }
    ASSERT(!filePathList.empty());
    if (filePathList.empty()) {
        return false;
    }

    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, filePathList, finishCallback]() {
            //在子线程中读取子目录数据        
            std::vector<PathInfoListPtr> folderListArray;
            if (!treeNodeFlag.expired()) {
                for (const FilePath& filePath : filePathList) {
                    PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
                    m_impl->GetFolderContents(filePath, treeNodeFlag, false, *folderList, nullptr);
                    folderListArray.push_back(folderList);
                    if (treeNodeFlag.expired()) {
                        break;
                    }
                }
            }        
            if (!treeNodeFlag.expired()) {
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, filePathList, treeNodeFlag, pTreeNode, folderListArray, finishCallback]() {
                        //这段代码在UI线程中执行
                        bool bAdded = false;
                        if (!treeNodeFlag.expired()) {
                            bAdded = OnShowSubFoldersEx(pTreeNode, filePathList, folderListArray);
                        }
                        if (!bAdded) {
                            for (const PathInfoListPtr& spPathInfoList : folderListArray) {
                                if (spPathInfoList != nullptr) {
                                    ClearPathInfoList(*spPathInfoList);
                                }
                            }
                        }
                        if (finishCallback) {
                            finishCallback();
                        }
                    }));
            }
            else {
                for (const PathInfoListPtr& spPathInfoList : folderListArray) {
                    if (spPathInfoList != nullptr) {
                        ClearPathInfoList(*spPathInfoList);
                    }
                }                
            }
        }));
    return true;
}

TreeNode* DirectoryTree::FindPathTreeNode(FilePath filePath) const
{
    TreeNode* pTreeNode = nullptr;
    FolderStatus* pFolder = GetFolderData(filePath);
    if (pFolder != nullptr) {
        pTreeNode = pFolder->m_pTreeNode;
    }
    return pTreeNode;
}

FilePath DirectoryTree::FindTreeNodePath(TreeNode* pTreeNode)
{
    FilePath filePath;
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    if (pFolder != nullptr) {
        filePath = FilePath(pFolder->m_filePath.c_str());
    }
    return filePath;
}

bool DirectoryTree::IsChildTreeNode(TreeNode* pTreeNode, TreeNode* pChildTreeNode) const
{
    if ((pTreeNode == nullptr) || (pChildTreeNode == nullptr)) {
        return false;
    }
    if (!IsValidTreeNode(pTreeNode) || !IsValidTreeNode(pChildTreeNode)) {
        return false;
    }
    bool bRet = false;
    TreeNode* p = pChildTreeNode->GetParentNode();
    while (p != nullptr) {
        if (p == pTreeNode) {
            bRet = true;
            break;
        }
        p = p->GetParentNode();
    }
    return bRet;
}

bool DirectoryTree::IsSamePath(const UiString& p1, const UiString& p2) const
{
#if !defined (DUILIB_BUILD_FOR_LINUX) && !defined (DUILIB_BUILD_FOR_FREEBSD)
    //Windows/MacOS文件名不区分大小写，Linux/FreeBSD区分大小写
    return StringUtil::IsEqualNoCase(p1.c_str(), p2.c_str());
#else
    return p1 == p2;
#endif
}

bool DirectoryTree::IsPathSame(TreeNode* pTreeNode, FilePath path) const
{
    if ((pTreeNode == nullptr) || path.IsEmpty()) {
        return false;
    }
    FilePath dirPath;
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    if (pFolder != nullptr) {
        dirPath = pFolder->m_filePath.c_str();
    }
    if (dirPath.IsEmpty()) {
        return false;
    }
    dirPath.NormalizeDirectoryPath();
    path.NormalizeDirectoryPath();
    return path == dirPath;
}

DirectoryTree::FolderStatus* DirectoryTree::GetFolderData(TreeNode* pTreeNode) const
{
    FolderStatus* pFolder = nullptr;
    if (pTreeNode != nullptr) {
        size_t folderKey = pTreeNode->GetUserDataID();
        auto iter = m_folderMap.find(folderKey);
        if (iter != m_folderMap.end()) {
            pFolder = iter->second;
            ASSERT(pFolder != nullptr);
            ASSERT(pFolder->m_pTreeNode == pTreeNode);
        }
    }
    return pFolder;
}

DirectoryTree::FolderStatus* DirectoryTree::GetFolderData(FilePath filePath) const
{
    FolderStatus* pFolder = nullptr;
    if (filePath.IsEmpty()) {
        return pFolder;
    }
    filePath.NormalizeDirectoryPath();
    const UiString filePathString = filePath.ToString();
    for (auto iter : m_folderMap) {
        if (IsSamePath(iter.second->m_filePath, filePathString)) {
            pFolder = iter.second;
            break;
        }
    }
    return pFolder;
}

bool DirectoryTree::RefreshTree(StdClosure finishCallback)
{
    std::vector<TreeNode*> childNodes;
    TreeNode* pTreeNode = GetRootNode();
    if (pTreeNode != nullptr) {
        pTreeNode->GetChildNodes(childNodes);
    }
    return RefreshTreeNodes(childNodes, finishCallback);
}

bool DirectoryTree::RefreshTreeNode(TreeNode* pTreeNode, StdClosure finishCallback)
{
    std::vector<TreeNode*> childNodes;
    childNodes.push_back(pTreeNode);
    return RefreshTreeNodes(childNodes, finishCallback);
}

bool DirectoryTree::RefreshTreeNodes(const std::vector<TreeNode*>& treeNodes, StdClosure finishCallback)
{
    if (finishCallback == nullptr) {
        finishCallback = m_defaultRefreshFinishCallback;
    }
    std::vector<std::shared_ptr<RefreshNodeData>> refreshData;
    for (TreeNode* pTreeNode : treeNodes) {
        GetTreeNodeData((size_t)-1, pTreeNode, refreshData);
    }

    if (refreshData.empty()) {
        return false;
    }

    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, weakFlag, refreshData, finishCallback]() {
            //在子线程中读取子目录数据的最新状态
            std::vector<std::shared_ptr<RefreshNodeData>> updatedRefreshData(refreshData);
            RefreshPathInfo(updatedRefreshData);

            //在UI线程中，更新界面
            if (!weakFlag.expired()) {
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, updatedRefreshData, finishCallback]() {
                        UpdateTreeNodeData(updatedRefreshData);
                        if (finishCallback) {
                            finishCallback();
                        }
                    }));
            }
            else {
                for (const std::shared_ptr<RefreshNodeData>& spRefreshNodeData : updatedRefreshData) {
                    if (spRefreshNodeData != nullptr) {
                        ClearPathInfoList(spRefreshNodeData->m_newFolderList);
                    }
                }
            }
        }));
    return true;
}

void DirectoryTree::RefreshPathInfo(std::vector<std::shared_ptr<RefreshNodeData>>& refreshData)
{
    const size_t nNodeCount = refreshData.size();
    for (std::shared_ptr<RefreshNodeData>& pNodeData : refreshData) {
        if (pNodeData->m_weakFlag.expired() || (pNodeData->m_pTreeNode == nullptr)) {
            //该节点已经被删除，无需同步
            continue;
        }
        if (pNodeData->m_nParentIndex < nNodeCount) {
            if (refreshData[pNodeData->m_nParentIndex]->m_bDeleted) {
                //父节点已经删除, 标记字节为删除，不需要再处理
                pNodeData->m_bDeleted = true;
            }
        }

        if (pNodeData->m_bDeleted) {
            continue;
        }

        if (IsMyComputerNode(pNodeData->m_pTreeNode)) {
            //"计算机"节点，检查磁盘列表是否有变化（仅需要检测新增）
            std::vector<DirectoryTree::PathInfo> pathInfoList;
            m_impl->GetRootPathInfoList(false, pathInfoList);

            std::vector<DString> oldPathList;
            std::vector<TreeNode*> childNodes;
            pNodeData->m_pTreeNode->GetChildNodes(childNodes);
            for (TreeNode* pChildNode : childNodes) {
                FolderStatus* pChildFolder = GetFolderData(pChildNode);
                if (pChildFolder != nullptr) {
                    oldPathList.push_back(pChildFolder->m_filePath.c_str());
                }
            }
            
            auto iter = pathInfoList.begin();
            while (iter != pathInfoList.end()) {
                bool bFound = false;
                for (const DString& oldPath : oldPathList) {
                    FilePath checkPath = iter->m_filePath;
                    checkPath.NormalizeDirectoryPath();
                    if (IsSamePath(oldPath.c_str(), checkPath.ToString().c_str())) {
                        bFound = true;
                        break;
                    }
                }
                if (bFound) {
                    //找到，不是新增
                    ++iter;
                }
                else {
                    //未找到，是新增
                    pNodeData->m_newFolderList.push_back(*iter);
                    iter = pathInfoList.erase(iter);
                }
            }
            ClearPathInfoList(pathInfoList);
            continue;
        }

        if (!m_impl->NeedShowDirPath(pNodeData->m_dirPath)) {
            //标记为删除
            pNodeData->m_bDeleted = true;
            continue;
        }

        if (!pNodeData->m_bContentLoaded) {
            //该节点未展开，不需要同步子目录信息
            ASSERT(pNodeData->m_childPaths.empty());
            continue;
        }

        //读取最新的子目录信息
        std::vector<DirectoryTree::PathInfo> folderList;
        m_impl->GetFolderContents(pNodeData->m_dirPath, pNodeData->m_weakFlag, false, folderList, nullptr);
        if (!folderList.empty()) {
            std::set<DString> dirSet;
            for (const FilePath& dirPath : pNodeData->m_childPaths) {
                DString dirPathString = dirPath.ToString();
#if !defined (DUILIB_BUILD_FOR_LINUX) && !defined (DUILIB_BUILD_FOR_FREEBSD)
                dirPathString = StringUtil::MakeLowerString(dirPathString);
#endif
                if (!dirPathString.empty()) {
                    if (dirPathString[dirPathString.size() - 1] == FilePath::GetPathSeparator()) {
                        //移除目录分隔符
                        dirPathString.pop_back();
                    }
                }
                dirSet.insert(dirPathString);
            }

            for (const DirectoryTree::PathInfo& pathInfo : folderList) {
                DString filePath = pathInfo.m_filePath.ToString();
#if !defined (DUILIB_BUILD_FOR_LINUX) && !defined (DUILIB_BUILD_FOR_FREEBSD)
                filePath = StringUtil::MakeLowerString(filePath);
#endif
                if (!filePath.empty()) {
                    if (filePath[filePath.size() - 1] == FilePath::GetPathSeparator()) {
                        //移除目录分隔符
                        filePath.pop_back();
                    }
                }
                if (dirSet.find(filePath) == dirSet.end()) {
                    //新增的目录项
                    pNodeData->m_newFolderList.push_back(pathInfo);
                }
            }
        }
    }
}

void DirectoryTree::UpdateTreeNodeData(const std::vector<std::shared_ptr<RefreshNodeData>>& refreshData)
{
    GlobalManager::Instance().AssertUIThread();
    for (const std::shared_ptr<RefreshNodeData>& pNodeData : refreshData) {
        if (pNodeData == nullptr) {
            continue;
        }
        if (pNodeData->m_weakFlag.expired() || (pNodeData->m_pTreeNode == nullptr)) {
            //树的节点已经无效
            ClearPathInfoList(pNodeData->m_newFolderList);
            continue;
        }
        if (pNodeData->m_bDeleted) {
            //该节点已经被删除, 从树上移除该节点
            RemoveTreeNode(pNodeData->m_pTreeNode);
        }
    }

    for (const std::shared_ptr<RefreshNodeData>& pNodeData : refreshData) {
        if (pNodeData == nullptr) {
            continue;
        }
        if (pNodeData->m_bDeleted || pNodeData->m_weakFlag.expired() || (pNodeData->m_pTreeNode == nullptr)) {
            //树的节点已经无效(或已经删除)
            ClearPathInfoList(pNodeData->m_newFolderList);
            continue;
        }
        if (!pNodeData->m_newFolderList.empty()) {
            //该节点下，有新的目录建立，添加到树节点
            for (const DirectoryTree::PathInfo& pathInfo : pNodeData->m_newFolderList) {
                if (!pathInfo.m_filePath.IsEmpty()) {
                    InsertTreeNode(pNodeData->m_pTreeNode, pathInfo.m_displayName, false,
                                   pathInfo.m_filePath, pathInfo.m_bFolder, false,
                                   pathInfo.m_nIconID, pathInfo.m_bIconShared);
                }
            }            
        }
    }
}

void DirectoryTree::GetTreeNodeData(size_t nParentIndex, TreeNode* pTreeNode, std::vector<std::shared_ptr<RefreshNodeData>>& refreshData) const
{
    GlobalManager::Instance().AssertUIThread();
    DirectoryTree::FolderStatus* pFolder = GetFolderData(pTreeNode);
    if (pFolder == nullptr) {
        return;
    }
    //当前节点对应的路径
    FilePath dirPath(pFolder->m_filePath.c_str());

    std::shared_ptr<RefreshNodeData> nodeData = std::make_shared<RefreshNodeData>();
    refreshData.push_back(nodeData);

    nodeData->m_pTreeNode = pTreeNode;
    nodeData->m_weakFlag = pTreeNode->GetWeakFlag();
    nodeData->m_bContentLoaded = pFolder->m_bContentLoaded;
    nodeData->m_dirPath.Swap(dirPath);
    nodeData->m_nParentIndex = nParentIndex;

    std::vector<TreeNode*> childNodes;
    pTreeNode->GetChildNodes(childNodes);
    for (TreeNode* pChildTreeNode : childNodes) {
        FolderStatus* pSubFolder = GetFolderData(pChildTreeNode);
        if (pSubFolder == nullptr) {
            continue;
        }
        //当前子节点对应的路径
        FilePath subDirPath(pSubFolder->m_filePath.c_str());
        nodeData->m_childPaths.emplace_back(std::move(subDirPath));
    }

    //递归子节点(展开后，为非递归的结构)
    size_t nCurrentParentIndex = refreshData.size() - 1;
    for (TreeNode* pChildTreeNode : childNodes) {
        GetTreeNodeData(nCurrentParentIndex, pChildTreeNode, refreshData);
    }
}

bool DirectoryTree::SelectTreeNode(TreeNode* pTreeNode)
{
    if (pTreeNode == nullptr) {
        return false;
    }
    bool bRet = BaseClass::SelectTreeNode(pTreeNode);
    if (bRet) {
        //主动选择时，激活该节点
        pTreeNode->Activate(nullptr);
    }
    return bRet;
}

void DirectoryTree::SetRefreshFinishCallback(StdClosure finishCallback)
{
    m_defaultRefreshFinishCallback = finishCallback;
}

}//namespace ui
