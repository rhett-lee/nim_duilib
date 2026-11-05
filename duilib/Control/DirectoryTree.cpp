#include "DirectoryTree.h"
#include "DirectoryTreeImpl.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Core/GlobalManager.h"
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
    //移除所有的树节点，避免产生Destroy事件
    RemoveAllNodes();

    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }

    for (auto iter : m_folderMap) {
        DeleteFolderStatus(iter.second);
    }
    m_folderMap.clear();
    m_folderIndex.clear();
    m_pLastLoadedTreeNode = nullptr;
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

void DirectoryTree::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    //支持的属性列表: 基类实现的直接转发
    if (strName == _T("small_icon_size")) {
        SetSmallIconSize(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("large_icon_size")) {
        SetLargeIconSize(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("show_hiden_files")) {
        SetShowHidenFiles(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("show_system_files")) {
        SetShowSystemFiles(StringUtil::IsValueTrue(strValue));
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
            if (m_bRefreshing) {
                //已有刷新任务在运行，合并本次刷新请求
                m_bNeedRefresh = true;
            }
            else {
                RefreshTree(nullptr);
            }
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
            if (m_bRefreshing) {
                //已有刷新任务在运行，合并本次刷新请求
                m_bNeedRefresh = true;
            }
            else {
                RefreshTree(nullptr);
            }
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

TreeNode* DirectoryTree::ShowAllDiskNodes(const DString& computerName, const DString& fileSystemName, bool bDisplayNameIsID)
{
    //基本结构:
    //  -计算机
    //     -C:\\盘
    //     -D:\\盘
    //     ...
    uint32_t nMyComputerIcon = m_impl->GetMyComputerIconID();
    TreeNode* pMyComputerNode = InsertTreeNode(nullptr, computerName, bDisplayNameIsID, FilePath(), false, true, nMyComputerIcon, true);
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
        bool bCurrentDisplayNameIsID = false;
        DString displayName = pathInfo.m_displayName;
#ifndef DUILIB_BUILD_FOR_WIN
        if (bFirstNode && (displayName == _T("/")) && !fileSystemName.empty()) {
            //替换为文件系统
            bFirstNode = false;
            displayName = fileSystemName;
            bCurrentDisplayNameIsID = bDisplayNameIsID;
        }
#endif
        TreeNode* pNewNode = InsertTreeNode(pMyComputerNode, displayName, bCurrentDisplayNameIsID,
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
    FilePath filePath(path);
    if (isFolder) {
        filePath.NormalizeDirectoryPath();
    }
    else {
        filePath.NormalizeFilePath();
    }

    if (pParentTreeNode == nullptr) {
        pParentTreeNode = GetRootNode();
    }

    //合并到已经存在的同路径节点，避免重复插入
    if ((pParentTreeNode != nullptr) && !filePath.IsEmpty()) {
        std::vector<TreeNode*> childNodes;
        pParentTreeNode->GetChildNodes(childNodes);
        const DString pathKey = NormalizePathKey(filePath);
        for (TreeNode* pChildNode : childNodes) {
            FolderStatus* pChildFolder = GetFolderData(pChildNode);
            if ((pChildFolder != nullptr) && !pChildFolder->m_filePath.IsEmpty() &&
                (NormalizePathKey(pChildFolder->m_filePath) == pathKey)) {
                if ((nIconID != 0) && !bIconShared) {
                    GlobalManager::Instance().Icon().RemoveIcon(nIconID);
                }
                return pChildNode;
            }
        }
    }

    TreeNode* node = new TreeNode(GetWindow());
    node->SetClass(_T("tree_node"));//在"global.xml"中定义
    if (bDisplayNameIsID) {
        node->SetTextId(displayName);
    }
    else {
        node->SetText(displayName);
    }

    FolderStatus* pFolder = new FolderStatus;
    pFolder->m_filePath = filePath;
    pFolder->m_nIconID = nIconID;
    pFolder->m_bIconShared = bIconShared;
    pFolder->m_pTreeNode = node;

    size_t folderKey = ++m_folderKey;
    m_folderMap[folderKey] = pFolder;
    if (!filePath.IsEmpty()) {
        m_folderIndex[NormalizePathKey(filePath)] = pFolder;
    }
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
                if (pFolder->m_filePath.IsEmpty()) {
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
            if (!ShowSubFolders(pTreeNode, pFolder->m_filePath, nullptr)) {
                pFolder->m_bContentLoaded = false;
            }
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
        if (m_pLastLoadedTreeNode == pTreeNode) {
            //同一节点重复点击，避免重复枚举
            return true;
        }
        m_pLastLoadedTreeNode = pTreeNode;
        ShowMyComputerContents(pTreeNode, nullptr);
    }
    else {
        //普通的文件夹节点
        FolderStatus* pFolder = GetFolderData(pTreeNode);
        if (pFolder != nullptr) {
            if (m_pLastLoadedTreeNode == pTreeNode) {
                //同一节点重复点击，避免重复枚举
                return true;
            }
            m_pLastLoadedTreeNode = pTreeNode;
            //加载子目录列表到右侧区域
            ShowFolderContents(pTreeNode, pFolder->m_filePath, nullptr);
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
            ShowFolderContents(pTreeNode, pFolder->m_filePath, finishCallback);
            bRet = true;
        }
        else {
            NotifyFinish(finishCallback);
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
            FolderStatus* pFolder = iter->second;
            if ((pFolder != nullptr) && !pFolder->m_filePath.IsEmpty()) {
                auto indexIter = m_folderIndex.find(NormalizePathKey(pFolder->m_filePath));
                if ((indexIter != m_folderIndex.end()) && (indexIter->second == pFolder)) {
                    m_folderIndex.erase(indexIter);
                }
            }
            DeleteFolderStatus(pFolder);
            m_folderMap.erase(iter);
        }
        if (m_pLastLoadedTreeNode == pTreeNode) {
            m_pLastLoadedTreeNode = nullptr;
        }
    }
    return true;
}

bool DirectoryTree::ShowSubFolders(TreeNode* pTreeNode, const FilePath& path, StdClosure finishCallback)
{
    //校验是否相同
    if (!IsPathSame(pTreeNode, path)) {
        ASSERT(0);
        NotifyFinish(finishCallback);
        return false;
    }

    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    //在工作线程中只使用界面线程的快照值，避免跨线程访问控件状态
    const bool bShowHiddenFiles = m_bShowHidenFiles;
    const bool bShowSystemFiles = m_bShowSystemFiles;
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, path, bShowHiddenFiles, bShowSystemFiles, finishCallback]() {
            //在子线程中读取子目录数据
            PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            if (!treeNodeFlag.expired()) {
                m_impl->GetFolderContents(path, treeNodeFlag, false, bShowHiddenFiles, bShowSystemFiles, *folderList, nullptr);
            }
            if (!treeNodeFlag.expired()) {
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, path, treeNodeFlag, pTreeNode, folderList, finishCallback]() {
                        //这段代码在UI线程中执行
                        bool bAdded = false;
                        if (!treeNodeFlag.expired()) {
                            bAdded = OnShowSubFolders(pTreeNode, path, folderList);
                        }
                        if (!bAdded) {
                            if (folderList != nullptr) {
                                ClearPathInfoList(*folderList);
                            }
                            //加载失败，回滚加载标记，允许再次展开时重试
                            FolderStatus* pFolder = GetFolderData(pTreeNode);
                            if (pFolder != nullptr) {
                                pFolder->m_bContentLoaded = false;
                            }
                        }
                        if (finishCallback) {
                            finishCallback();
                        }
                    }));
            }
            else {
                ClearPathInfoList(*folderList);
                //任务已取消，仍需通知调用方结束
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, finishCallback]() {
                        NotifyFinish(finishCallback);
                    }));
            }
        }));
    return true;
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
        for (DirectoryTree::PathInfo& pathInfo : *folderList) {
            if (!pathInfo.m_filePath.IsEmpty()) {
                InsertTreeNode(pTreeNode, pathInfo.m_displayName, false,
                               pathInfo.m_filePath, pathInfo.m_bFolder, false,
                               pathInfo.m_nIconID, pathInfo.m_bIconShared);
                //图标所有权已转移给树节点（或已被去重逻辑释放），避免重复释放
                pathInfo.m_nIconID = 0;
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
        for (DirectoryTree::PathInfo& pathInfo : *folderList) {
            if (pathInfo.m_filePath.IsEmpty()) {
                continue;
            }
            TreeNode* pNewTreeNode = InsertTreeNode(pParentTreeNode, pathInfo.m_displayName, false,
                                                    pathInfo.m_filePath, pathInfo.m_bFolder, false,
                                                    pathInfo.m_nIconID, pathInfo.m_bIconShared);
            //图标所有权已转移给树节点（或已被去重逻辑释放），避免重复释放
            pathInfo.m_nIconID = 0;
            if ((pNextParentTreeNode == nullptr) && (pNewTreeNode != nullptr) && !path.IsEmpty()) {
                FilePath checkPath = pathInfo.m_filePath;
                checkPath.NormalizeDirectoryPath();
                if (IsSamePath(path.ToString(), checkPath.ToString())) {
                    pNextParentTreeNode = pNewTreeNode;
                }                
            }
        }
        if (!bLastNode) {
            if (pNextParentTreeNode == nullptr) {
                //枚举结果中没有找到路径链上的下一级目录(例如目录被过滤、无访问权限或枚举失败)，
                //按路径直接创建节点，保证路径链完整，避免导航中断
                DString pathText = path.ToString();
                size_t nEnd = pathText.size();
                while ((nEnd > 0) && ((pathText[nEnd - 1] == _T('\\')) || (pathText[nEnd - 1] == _T('/')))) {
                    --nEnd;
                }
                DString displayName = FilePath(pathText.substr(0, nEnd)).GetFileName();
                if (displayName.empty()) {
                    displayName = pathText;
                }
                uint32_t nIconID = m_impl->GetFolderIconID(false);
                pNextParentTreeNode = InsertTreeNode(pParentTreeNode, displayName, false, path,
                                                     true, false, nIconID, true);
            }
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

    for (TreeNode* pNode : treeNodes) {
        FolderStatus* pFolder = GetFolderData(pNode);
        if (pFolder != nullptr) {
            //标记为已经展开，避免再重复展开
            pFolder->m_bContentLoaded = true;
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
    //在工作线程中只使用界面线程的快照值，避免跨线程访问控件状态
    const bool bShowHiddenFiles = m_bShowHidenFiles;
    const bool bShowSystemFiles = m_bShowSystemFiles;
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, path, bShowHiddenFiles, bShowSystemFiles, finishCallback]() {
            //在子线程中读取子目录数据
            PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            PathInfoListPtr fileList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            if (!treeNodeFlag.expired()) {
                m_impl->GetFolderContents(path, treeNodeFlag, true, bShowHiddenFiles, bShowSystemFiles, *folderList, fileList.get());
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
                //任务已取消，仍需通知调用方结束
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, finishCallback]() {
                        NotifyFinish(finishCallback);
                    }));
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
        NotifyFinish(finishCallback);
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
            //任务已取消，仍需通知调用方结束
            GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, finishCallback]() {
                    NotifyFinish(finishCallback);
                }));
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
        NotifyFinish(finishCallback);
        return false;
    }
    subPath.NormalizeDirectoryPath();
    if (subPath.IsEmpty() || !subPath.IsExistsDirectory()) {
        NotifyFinish(finishCallback);
        return false;
    }
    //校验是否在目录中
    FolderStatus* pFolderStatus = GetFolderData(pTreeNode);
    ASSERT(pFolderStatus != nullptr);
    if (pFolderStatus == nullptr) {
        NotifyFinish(finishCallback);
        return false;
    }
    if (!pFolderStatus->m_filePath.IsEmpty()) {
        if (!subPath.IsSubDirectory(pFolderStatus->m_filePath)) {
            //路径与树节点不是父子目录关系
            NotifyFinish(finishCallback);
            return false;
        }
    }

    if (GetFolderData(subPath) != nullptr) {
        //该目录已经存在于树的节点中
        NotifyFinish(finishCallback);
        TreeNode* pSubTreeNode = FindPathTreeNode(subPath);
        if (pSubTreeNode != nullptr) {
            SelectTreeNode(pSubTreeNode);
        }
        return true;
    }

    //如果不存在，则逐级展开：构造从顶层到目标自身的路径链
    std::vector<FilePath> chain;
    subPath.GetParentPathList(chain);
    subPath.FormatPathAsDirectory();
    chain.push_back(subPath);
    for (FilePath& item : chain) {
        item.FormatPathAsDirectory();
    }
    //GetParentPathList对"带尾部分隔符的目录路径"会把目录自身也作为父路径返回，
    //这里按路径去重，避免同一目录在路径链中出现两次(否则会重复创建节点)
    for (size_t index = 1; index < chain.size();) {
        if (NormalizePathKey(chain[index]) == NormalizePathKey(chain[index - 1])) {
            chain.erase(chain.begin() + index);
        }
        else {
            ++index;
        }
    }

    TreeNode* pBaseNode = nullptr;
    std::vector<FilePath> remaining;
    if (!FindBaseNodeInChain(chain, pTreeNode, pBaseNode, remaining)) {
        NotifyFinish(finishCallback);
        return false;
    }
    return OnSelectSubPath(pBaseNode, remaining, finishCallback);
}

bool DirectoryTree::SelectPath(FilePath filePath, StdClosure finishCallback)
{
    if (filePath.IsEmpty()) {
        NotifyFinish(finishCallback);
        return false;
    }
    filePath.NormalizeDirectoryPath();
    if (!filePath.IsExistsDirectory()) {
        NotifyFinish(finishCallback);
        return false;
    }

    FolderStatus* pFolderStatus = GetFolderData(filePath);
    if (pFolderStatus != nullptr) {
        //该目录已经存在于树的节点中
        TreeNode* pSubTreeNode = pFolderStatus->m_pTreeNode;
        if (pSubTreeNode != nullptr) {
            SelectTreeNode(pSubTreeNode);
        }
        NotifyFinish(finishCallback);
        return true;
    }

    //如果不存在，则逐级展开：构造从顶层到目标自身的路径链
    std::vector<FilePath> chain;
    filePath.GetParentPathList(chain);
    filePath.FormatPathAsDirectory();
    chain.push_back(filePath);
    for (FilePath& item : chain) {
        item.FormatPathAsDirectory();
    }
    //GetParentPathList对"带尾部分隔符的目录路径"会把目录自身也作为父路径返回，
    //这里按路径去重，避免同一目录在路径链中出现两次(否则会重复创建节点)
    for (size_t index = 1; index < chain.size();) {
        if (NormalizePathKey(chain[index]) == NormalizePathKey(chain[index - 1])) {
            chain.erase(chain.begin() + index);
        }
        else {
            ++index;
        }
    }

    TreeNode* pBaseNode = nullptr;
    std::vector<FilePath> remaining;
    if (!FindBaseNodeInChain(chain, nullptr, pBaseNode, remaining)) {
        NotifyFinish(finishCallback);
        return false;
    }
    return OnSelectSubPath(pBaseNode, remaining, finishCallback);
}

bool DirectoryTree::OnSelectSubPath(TreeNode* pTreeNode, std::vector<FilePath> filePathList, StdClosure finishCallback)
{
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    ASSERT(pFolder != nullptr);
    if (pFolder == nullptr) {
        NotifyFinish(finishCallback);
        return false;
    }
    FilePath destFilePath(pFolder->m_filePath);
    destFilePath.NormalizeDirectoryPath();
    const bool bVirtualNode = pFolder->m_filePath.IsEmpty();
    const DString destKey = NormalizePathKey(destFilePath);

    //移除冗余目录(过滤掉，不是树节点子目录的路径)
    //注意：首个锚点必须是基准节点自身的路径，OnShowSubFoldersEx按"路径链的前一项作为父节点"挂载，
    //即使基准节点的内容已经加载过，也不能移除该锚点(否则子目录会被挂载到错误的父节点上)
    std::vector<FilePath>::iterator iter = filePathList.begin();
    while (iter != filePathList.end()) {
        const FilePath& checkPath = *iter;
        if (NormalizePathKey(checkPath) == destKey) {
            ++iter;
        }
        else if (!bVirtualNode && !checkPath.IsSubDirectory(destFilePath)) {
            //如果不是子目录，移除
            iter = filePathList.erase(iter);
        }
        else {
            ++iter;
        }
    }
    if (filePathList.empty()) {
        //没有需要展开的目录，直接选中基准节点
        SelectTreeNode(pTreeNode);
        NotifyFinish(finishCallback);
        return true;
    }

    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    //在工作线程中只使用界面线程的快照值，避免跨线程访问控件状态
    const bool bShowHiddenFiles = m_bShowHidenFiles;
    const bool bShowSystemFiles = m_bShowSystemFiles;
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, filePathList, bShowHiddenFiles, bShowSystemFiles, finishCallback]() {
            //在子线程中读取子目录数据：
            //显式导航时，需要保证目标路径上的目录（可能是隐藏目录或系统目录）能够展开，
            //因此枚举时不做属性过滤，随后按显示设置过滤，但保留导航路径链上的目录项
            std::set<DString> chainKeys;
            for (const FilePath& filePath : filePathList) {
                chainKeys.insert(NormalizePathKey(filePath));
            }
            std::vector<PathInfoListPtr> folderListArray;
            if (!treeNodeFlag.expired()) {
                for (const FilePath& filePath : filePathList) {
                    PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
                    m_impl->GetFolderContents(filePath, treeNodeFlag, false, true, true, *folderList, nullptr);
                    std::vector<DirectoryTree::PathInfo> filteredList;
                    for (DirectoryTree::PathInfo& pathInfo : *folderList) {
                        const bool bKeep = m_impl->NeedShowDirPath(pathInfo.m_filePath, bShowHiddenFiles, bShowSystemFiles) ||
                                           (chainKeys.find(NormalizePathKey(pathInfo.m_filePath)) != chainKeys.end());
                        if (bKeep) {
                            //保留的目录项，交接图标所有权
                            filteredList.push_back(pathInfo);
                            pathInfo.m_nIconID = 0;
                        }
                        else if (!pathInfo.m_bIconShared) {
                            GlobalManager::Instance().Icon().RemoveIcon(pathInfo.m_nIconID);
                            pathInfo.m_nIconID = 0;
                        }
                    }
                    folderList->swap(filteredList);
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
                //任务已取消，仍需通知调用方结束
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, finishCallback]() {
                        NotifyFinish(finishCallback);
                    }));
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
        filePath = pFolder->m_filePath;
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

bool DirectoryTree::IsSamePath(const DString& p1, const DString& p2) const
{
#if !defined (DUILIB_BUILD_FOR_LINUX) && !defined (DUILIB_BUILD_FOR_FREEBSD)
    //Windows/MacOS文件名不区分大小写，Linux/FreeBSD区分大小写
    return StringUtil::IsEqualNoCase(p1, p2);
#else
    return p1 == p2;
#endif
}

bool DirectoryTree::IsPathSame(TreeNode* pTreeNode, FilePath path) const
{
    if ((pTreeNode == nullptr) || path.IsEmpty()) {
        return false;
    }
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    if ((pFolder == nullptr) || pFolder->m_filePath.IsEmpty()) {
        return false;
    }
    return NormalizePathKey(pFolder->m_filePath) == NormalizePathKey(path);
}

DString DirectoryTree::NormalizePathKey(const FilePath& path) const
{
    DString pathKey;
    if (path.IsEmpty()) {
        return pathKey;
    }
    FilePath normalPath(path);
    normalPath.NormalizeDirectoryPath();
    pathKey = normalPath.ToString();
#if !defined (DUILIB_BUILD_FOR_LINUX) && !defined (DUILIB_BUILD_FOR_FREEBSD)
    //Windows/MacOS文件名不区分大小写，Linux/FreeBSD区分大小写
    pathKey = StringUtil::MakeLowerString(pathKey);
#endif
    return pathKey;
}

void DirectoryTree::NotifyFinish(const StdClosure& finishCallback) const
{
    if (finishCallback) {
        finishCallback();
    }
}

bool DirectoryTree::FindBaseNodeInChain(const std::vector<FilePath>& chain,
                                        TreeNode* pBaseNode,
                                        TreeNode*& pBaseOut,
                                        std::vector<FilePath>& remaining) const
{
    pBaseOut = nullptr;
    remaining.clear();
    if (chain.empty()) {
        return false;
    }

    //确定查找的起始位置：指定基准节点时，只在其之后查找
    size_t startIndex = 0;
    const bool bVirtualBaseNode = IsMyComputerNode(pBaseNode);
    if (pBaseNode != nullptr) {
        FolderStatus* pBaseFolder = GetFolderData(pBaseNode);
        if (pBaseFolder == nullptr) {
            return false;
        }
        if (bVirtualBaseNode) {
            //"计算机"等虚拟节点没有路径，在整条路径链中查找
            pBaseOut = pBaseNode;
        }
        else {
            const DString baseKey = NormalizePathKey(pBaseFolder->m_filePath);
            bool bFoundBase = false;
            for (size_t index = 0; index < chain.size(); ++index) {
                if (NormalizePathKey(chain[index]) == baseKey) {
                    startIndex = index + 1;
                    bFoundBase = true;
                    break;
                }
            }
            if (!bFoundBase) {
                return false;
            }
            pBaseOut = pBaseNode;
        }
    }

    //自顶向下查找最深的已经存在的树节点
    size_t foundIndex = (size_t)-1;
    for (size_t index = startIndex; index < chain.size(); ++index) {
        FolderStatus* pFolder = GetFolderData(chain[index]);
        if ((pFolder != nullptr) && (pFolder->m_pTreeNode != nullptr)) {
            foundIndex = index;
            pBaseOut = pFolder->m_pTreeNode;
        }
    }

    if (bVirtualBaseNode && (foundIndex == (size_t)-1)) {
        //虚拟节点下没有找到任何已经存在的节点，无法继续展开
        pBaseOut = nullptr;
        return false;
    }

    if (pBaseOut == nullptr) {
        return false;
    }

    //剩余路径包含基准节点自身(以便在未加载时枚举其子目录)以及其后的路径
    size_t firstIndex = (foundIndex == (size_t)-1) ? startIndex : foundIndex;
    if ((foundIndex == (size_t)-1) && !bVirtualBaseNode && (startIndex > 0)) {
        firstIndex = startIndex - 1;
    }
    for (size_t index = firstIndex; index < chain.size(); ++index) {
        remaining.push_back(chain[index]);
    }
    return true;
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
    const DString pathKey = NormalizePathKey(filePath);
    auto iter = m_folderIndex.find(pathKey);
    if (iter != m_folderIndex.end()) {
        pFolder = iter->second;
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
        NotifyFinish(finishCallback);
        return false;
    }

    m_bRefreshing = true;
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
                        m_bRefreshing = false;
                        const bool bNeedRefresh = m_bNeedRefresh;
                        m_bNeedRefresh = false;
                        if (bNeedRefresh) {
                            //刷新期间收到的新请求，补执行一次
                            RefreshTree(nullptr);
                        }
                        NotifyFinish(finishCallback);
                    }));
            }
            else {
                for (const std::shared_ptr<RefreshNodeData>& spRefreshNodeData : updatedRefreshData) {
                    if (spRefreshNodeData != nullptr) {
                        ClearPathInfoList(spRefreshNodeData->m_newFolderList);
                    }
                }
                //任务已取消，仍需通知调用方结束
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, finishCallback]() {
                        NotifyFinish(finishCallback);
                    }));
            }
        }));
    return true;
}

void DirectoryTree::RefreshPathInfo(std::vector<std::shared_ptr<RefreshNodeData>>& refreshData)
{
    //本函数在工作线程中执行，只使用界面线程传入的快照数据，禁止访问任何UI对象
    const size_t nNodeCount = refreshData.size();
    for (size_t nNodeIndex = 0; nNodeIndex < nNodeCount; ++nNodeIndex) {
        std::shared_ptr<RefreshNodeData>& pNodeData = refreshData[nNodeIndex];
        if ((pNodeData == nullptr) || pNodeData->m_weakFlag.expired() || (pNodeData->m_pTreeNode == nullptr)) {
            //该节点已经被删除，无需同步
            continue;
        }
        if (pNodeData->m_nParentIndex < nNodeCount) {
            if (refreshData[pNodeData->m_nParentIndex]->m_bDeleted) {
                //父节点已经删除，标记为删除，不需要再处理
                pNodeData->m_bDeleted = true;
            }
        }

        if (pNodeData->m_bDeleted) {
            continue;
        }

        if (pNodeData->m_bMyComputer) {
            //"计算机"节点，检查磁盘列表是否有变化（新增与移除都需要处理）
            std::vector<DirectoryTree::PathInfo> pathInfoList;
            m_impl->GetRootPathInfoList(false, pathInfoList);

            std::set<DString> oldKeys;
            for (const FilePath& childPath : pNodeData->m_childPaths) {
                oldKeys.insert(NormalizePathKey(childPath));
            }

            //标记已经移除的磁盘节点
            for (size_t index = 0; index < nNodeCount; ++index) {
                std::shared_ptr<RefreshNodeData>& pChildData = refreshData[index];
                if ((pChildData == nullptr) || pChildData->m_bDeleted) {
                    continue;
                }
                if (pChildData->m_nParentIndex != nNodeIndex) {
                    continue;
                }
                if (pChildData->m_dirPath.IsEmpty()) {
                    continue;
                }
                if (oldKeys.find(NormalizePathKey(pChildData->m_dirPath)) == oldKeys.end()) {
                    continue;
                }
                const DString childKey = NormalizePathKey(pChildData->m_dirPath);
                bool bExists = false;
                for (const DirectoryTree::PathInfo& pathInfo : pathInfoList) {
                    if (NormalizePathKey(pathInfo.m_filePath) == childKey) {
                        bExists = true;
                        break;
                    }
                }
                if (!bExists) {
                    //磁盘已经移除，从树中删除该节点
                    pChildData->m_bDeleted = true;
                }
            }

            //计算新增的磁盘节点，并释放未使用的图标
            for (DirectoryTree::PathInfo& pathInfo : pathInfoList) {
                if (oldKeys.find(NormalizePathKey(pathInfo.m_filePath)) == oldKeys.end()) {
                    pNodeData->m_newFolderList.push_back(pathInfo);
                    pathInfo.m_nIconID = 0;
                }
            }
            ClearPathInfoList(pathInfoList);
            continue;
        }

        if (pNodeData->m_dirPath.IsEmpty()) {
            //虚拟节点，无需同步
            continue;
        }

        if (!m_impl->NeedShowDirPath(pNodeData->m_dirPath, pNodeData->m_bShowHiddenFiles, pNodeData->m_bShowSystemFiles)) {
            //标记为删除
            pNodeData->m_bDeleted = true;
            continue;
        }

        if (!pNodeData->m_bContentLoaded) {
            //该节点未展开，不需要同步子目录信息
            continue;
        }

        //读取最新的子目录信息（只同步新增的目录项）
        std::vector<DirectoryTree::PathInfo> folderList;
        m_impl->GetFolderContents(pNodeData->m_dirPath, pNodeData->m_weakFlag, false,
                                  pNodeData->m_bShowHiddenFiles, pNodeData->m_bShowSystemFiles,
                                  folderList, nullptr);
        std::set<DString> dirSet;
        for (const FilePath& dirPath : pNodeData->m_childPaths) {
            dirSet.insert(NormalizePathKey(dirPath));
        }
        for (DirectoryTree::PathInfo& pathInfo : folderList) {
            if (dirSet.find(NormalizePathKey(pathInfo.m_filePath)) == dirSet.end()) {
                //新增的目录项
                pNodeData->m_newFolderList.push_back(pathInfo);
                pathInfo.m_nIconID = 0;
            }
        }
        ClearPathInfoList(folderList);
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
            for (DirectoryTree::PathInfo& pathInfo : pNodeData->m_newFolderList) {
                if (!pathInfo.m_filePath.IsEmpty()) {
                    InsertTreeNode(pNodeData->m_pTreeNode, pathInfo.m_displayName, false,
                                   pathInfo.m_filePath, pathInfo.m_bFolder, false,
                                   pathInfo.m_nIconID, pathInfo.m_bIconShared);
                    //图标所有权已转移给树节点（或已被去重逻辑释放），避免重复释放
                    pathInfo.m_nIconID = 0;
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
    FilePath dirPath(pFolder->m_filePath);

    std::shared_ptr<RefreshNodeData> nodeData = std::make_shared<RefreshNodeData>();
    refreshData.push_back(nodeData);

    nodeData->m_pTreeNode = pTreeNode;
    nodeData->m_weakFlag = pTreeNode->GetWeakFlag();
    nodeData->m_bContentLoaded = pFolder->m_bContentLoaded;
    nodeData->m_bMyComputer = IsMyComputerNode(pTreeNode);
    //记录界面线程的快照值，供工作线程使用
    nodeData->m_bShowHiddenFiles = m_bShowHidenFiles;
    nodeData->m_bShowSystemFiles = m_bShowSystemFiles;
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
        FilePath subDirPath(pSubFolder->m_filePath);
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
    //主动选中节点（导航、前进后退等）需要刷新目录内容，清除重复点击的去重标记，
    //确保随后的Activate事件能触发目录加载
    m_pLastLoadedTreeNode = nullptr;
    bool bRet = BaseClass::SelectTreeNode(pTreeNode);
    if (bRet) {
        //选中节点后派发事件，供业务层在目录内容加载完成前即时更新地址栏等界面
        std::weak_ptr<WeakFlag> nodeFlag = pTreeNode->GetWeakFlag();
        SendEvent(kEventSelect, (WPARAM)pTreeNode);
        //主动选择时，激活该节点
        if (!nodeFlag.expired()) {
            pTreeNode->Activate(nullptr);
        }
    }
    return bRet;
}

void DirectoryTree::SetRefreshFinishCallback(StdClosure finishCallback)
{
    m_defaultRefreshFinishCallback = finishCallback;
}

}//namespace ui
