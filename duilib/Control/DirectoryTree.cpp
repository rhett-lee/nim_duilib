#include "DirectoryTree.h"
#include "DirectoryTreeImpl.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui
{
DirectoryTree::DirectoryTree(Window* pWindow):
    TreeView(pWindow),
    m_nThreadIdentifier(ui::kThreadWorker),
    m_nIconSize(16),
    m_folderKey(0)
{
    m_impl = new DirectoryTreeImpl(this);
}

DirectoryTree::~DirectoryTree()
{
    for (auto iter : m_folderMap) {
        if (!iter.second->m_bIconShared) {
            GlobalManager::Instance().Icon().RemoveIcon(iter.second->m_nIconID);
        }        
        delete iter.second;
    }
    m_folderMap.clear();
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

DString DirectoryTree::GetType() const { return DUI_CTR_DIRECTORY_TREE; }

void DirectoryTree::SetAttribute(const DString& strName, const DString& strValue)
{
    //支持的属性列表: 基类实现的直接转发
    if (strName == _T("")) {
        
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
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
    return InsertTreeNode(nullptr, folderName, bDisplayNameIsID, filePath, true, nIconID, false);
}

TreeNode* DirectoryTree::ShowAllDiskNodes()
{
    TreeNode* pFirstNode = nullptr;
    std::vector<DirectoryTree::PathInfo> pathInfoList;
    m_impl->GetRootPathInfoList(pathInfoList);
    for (const DirectoryTree::PathInfo& pathInfo : pathInfoList) {
        if (!pathInfo.m_filePath.IsEmpty()) {
            TreeNode* pNewNode = InsertTreeNode(nullptr, pathInfo.m_displayName, false, pathInfo.m_filePath, pathInfo.m_bFolder, pathInfo.m_nIconID, pathInfo.m_bIconShared);
            if (pFirstNode == nullptr) {
                pFirstNode = pNewNode;
            }
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

    node->SetBkIconID(nIconID, m_nIconSize, true);//设置树节点的关联图标(图标大小与CheckBox的原图大小相同，都是16*16)
    

    if (isFolder) {
        pFolder->m_bContentLoaded = false;
        node->SetExpand(false, false);
    }
    else {
        pFolder->m_bContentLoaded = true;
        node->SetExpand(true, false);
    }

#ifdef _DEBUG
    if (pParentTreeNode != nullptr) {
        //校验：以免挂错目录
        ASSERT(path.GetFileName() == displayName);
    }
#endif

    if (pParentTreeNode == nullptr) {
        pParentTreeNode = GetRootNode();
    }
    if (pParentTreeNode != nullptr) {
        pParentTreeNode->AddChildNode(node);
    }

    node->AttachExpand(UiBind(&DirectoryTree::OnTreeNodeExpand, this, std::placeholders::_1));
    node->AttachClick(UiBind(&DirectoryTree::OnTreeNodeClick, this, std::placeholders::_1));
    node->AttachSelect(UiBind(&DirectoryTree::OnTreeNodeSelect, this, std::placeholders::_1));
    return node;
}

bool DirectoryTree::OnTreeNodeExpand(const EventArgs& args)
{
    TreeNode* pTreeNode = dynamic_cast<TreeNode*>(args.GetSender());
    ASSERT(pTreeNode != nullptr);
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    if (pFolder != nullptr) {
        //加载子目录列表
        if (!pFolder->m_bContentLoaded) {
            pFolder->m_bContentLoaded = true;
            ShowSubFolders(pTreeNode, FilePath(pFolder->m_filePath.c_str()));
        }
    }
    return true;
}

bool DirectoryTree::OnTreeNodeClick(const EventArgs& args)
{
    TreeNode* pTreeNode = dynamic_cast<TreeNode*>(args.GetSender());
    ASSERT(pTreeNode != nullptr);
    FolderStatus* pFolder = GetFolderData(pTreeNode);
    if (pFolder != nullptr) {
        //加载子目录列表到右侧区域
        ShowFolderContents(pTreeNode, FilePath(pFolder->m_filePath.c_str()));
    }
    return true;
}

bool DirectoryTree::OnTreeNodeSelect(const EventArgs& args)
{
    if (IsMultiSelect()) {
        //多选的时候，不响应选择事件
        return true;
    }
    return OnTreeNodeClick(args);
}

void DirectoryTree::ExpandTreeNode(TreeNode* pTreeNode, FilePath subPath)
{
    if (pTreeNode == nullptr) {
        return;
    }
    size_t itemIndex = GetItemIndex(pTreeNode);
    if (!Box::IsValidItemIndex(itemIndex)) {
        return;
    }
    //校验是否在目录中
    if (!subPath.IsEmpty()) {
        if (!IsPathInDirectory(pTreeNode, subPath)) {
            ASSERT(0);
            return;
        }
        subPath.NormalizeDirectoryPath();
    }

    FolderStatus* pFolder = GetFolderData(pTreeNode);
    ASSERT(pFolder != nullptr);
    if (pFolder == nullptr) {
        return;
    }

    if (!pFolder->m_bContentLoaded) {
        //加载子目录列表
        pFolder->m_bContentLoaded = true;
        ShowSubFolders(pTreeNode, FilePath(pFolder->m_filePath.c_str()));
    }
    else {
        //展开子目录
        if (!pTreeNode->IsExpand()) {
            pTreeNode->SetExpand(true, true);
        }
    }

    if (subPath.IsEmpty()) {
        return;
    }
    
    //由于ShowSubFolders是在子线程中执行的，所以这里也要先发给子线程，再转给UI线程，保证时序正确
    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, pTreeNode, subPath]() {
            //这段代码在工作线程中执行，枚举目录内容完成后，然后发给UI线程添加到树节点上
            GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, pTreeNode, subPath]() {
                    //这段代码在UI线程中执行
                    FolderStatus* pFolder = GetFolderData(subPath);
                    if (pFolder != nullptr) {
                        TreeNode* pSubTreeNode = pFolder->m_pTreeNode;
                        if ((pSubTreeNode != nullptr) && pSubTreeNode->IsVisible()) {
                            pSubTreeNode->Activate(nullptr);
                        }
                    }
                }));
        }));
}

void DirectoryTree::ShowSubFolders(TreeNode* pTreeNode, const FilePath& path)
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
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, path]() {
            //在子线程中读取子目录数据
            PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            m_impl->GetFolderContents(path, treeNodeFlag, *folderList, nullptr);
            if (!treeNodeFlag.expired()) {
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, path, treeNodeFlag, pTreeNode, folderList]() {
                        //这段代码在UI线程中执行
                        if (!treeNodeFlag.expired()) {
                            OnShowSubFolders(pTreeNode, path, folderList);
                        }
                    }));
            }
        }));
}

void DirectoryTree::OnShowSubFolders(TreeNode* pTreeNode, const FilePath& /*path*/, const PathInfoListPtr& folderList)
{
    GlobalManager::Instance().AssertUIThread();
    if (pTreeNode == nullptr) {
        return;
    }
    size_t itemIndex = GetItemIndex(pTreeNode);
    if (!Box::IsValidItemIndex(itemIndex)) {
        return;
    }
    if (folderList != nullptr) {
        for (const DirectoryTree::PathInfo& pathInfo : *folderList) {
            if (!pathInfo.m_filePath.IsEmpty()) {
                InsertTreeNode(pTreeNode, pathInfo.m_displayName, false, pathInfo.m_filePath, pathInfo.m_bFolder, pathInfo.m_nIconID, pathInfo.m_bIconShared);
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
}

void DirectoryTree::OnShowSubFoldersEx(TreeNode* pTreeNode, const std::vector<FilePath>& filePathList, const std::vector<PathInfoListPtr>& folderListArray)
{
    GlobalManager::Instance().AssertUIThread();
    if (pTreeNode == nullptr) {
        return;
    }
    size_t itemIndex = GetItemIndex(pTreeNode);
    if (!Box::IsValidItemIndex(itemIndex)) {
        return;
    }
    ASSERT(filePathList.size() == folderListArray.size());
    if (filePathList.size() != folderListArray.size()) {
        return;
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
            TreeNode* pNewTreeNode = InsertTreeNode(pParentTreeNode, pathInfo.m_displayName, false, pathInfo.m_filePath, pathInfo.m_bFolder, pathInfo.m_nIconID, pathInfo.m_bIconShared);
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
}

void DirectoryTree::ShowFolderContents(TreeNode* pTreeNode, const FilePath& path)
{
    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, path]() {
            //在子线程中读取子目录数据
            PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            PathInfoListPtr fileList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            m_impl->GetFolderContents(path, treeNodeFlag, *folderList, fileList.get());
            if (!treeNodeFlag.expired()) {
                GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, path, treeNodeFlag, pTreeNode, folderList, fileList]() {
                        //这段代码在UI线程中执行
                        OnShowFolderContents(pTreeNode, path, folderList, fileList);
                    }));
            }
        }));
}

void DirectoryTree::OnShowFolderContents(TreeNode* pTreeNode, const FilePath& path,
                                         const PathInfoListPtr& folderList,
                                         const PathInfoListPtr& fileList)
{
    GlobalManager::Instance().AssertUIThread();
    for (ShowFolderContentsEvent callback : m_callbackList) {
        if (callback) {
            callback(pTreeNode, path, folderList, fileList);
        }
    }
}

bool DirectoryTree::SelectPath(FilePath filePath)
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
        return true;
    }

    //如果不存在，则逐级展开
    const UiString filePathString = filePath.ToString();
    std::vector<std::filesystem::path> path_list;
    std::filesystem::path path = filePathString.c_str();
    auto p = path.parent_path();
    auto rootPath = path.root_path();
    while (!p.empty()) {
        path_list.push_back(p);
        if (p == rootPath) {
            break;
        }
        p = p.parent_path();
        if (p == path_list.back()) {
            break;
        }
    }
    std::reverse(path_list.begin(), path_list.end());

    std::vector<FilePath> filePathList;
    for (const std::filesystem::path& s : path_list) {
        FilePath normalPath(s.native());
        normalPath.FormatPathAsDirectory();
        filePathList.push_back(normalPath);
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
    ASSERT(!filePathList.empty());
    if (filePathList.empty()) {
        return false;
    }

    ASSERT(pTreeNode != nullptr);
    if (pTreeNode == nullptr) {
        return false;
    }

    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, filePathList]() {
        //在子线程中读取子目录数据        
        std::vector<PathInfoListPtr> folderListArray;
        if (!treeNodeFlag.expired()) {
            for (const FilePath& filePath : filePathList) {
                PathInfoListPtr folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
                m_impl->GetFolderContents(filePath, treeNodeFlag, *folderList, nullptr);
                folderListArray.push_back(folderList);
                if (treeNodeFlag.expired()) {
                    break;
                }
            }
        }        
        if (!treeNodeFlag.expired()) {
            GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, filePathList, treeNodeFlag, pTreeNode, folderListArray]() {
                //这段代码在UI线程中执行
                if (!treeNodeFlag.expired()) {
                    OnShowSubFoldersEx(pTreeNode, filePathList, folderListArray);
                }
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
        filePath = FilePath(pFolder->m_filePath.c_str());
    }
    return filePath;
}

bool DirectoryTree::IsSamePath(const UiString& p1, const UiString& p2) const
{
#if !defined (DUILIB_BUILD_FOR_LINUX)
    //Windows文件名不区分大小写
    return StringUtil::IsEqualNoCase(p1.c_str(), p2.c_str());
#else
    return p1 == p2;
#endif
}

bool DirectoryTree::IsPathInDirectory(TreeNode* pTreeNode, const FilePath& path) const
{
    if ((pTreeNode == nullptr) || path.IsEmpty()) {
        return false;
    }
    if (!path.IsExistsDirectory()) {
        //不是目录，返回
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
    DString dirPathStr = dirPath.ToString();

    FilePath pathNormal(path);
    pathNormal.NormalizeFilePath();
    DString childPathStr = pathNormal.ToString();
#if !defined (DUILIB_BUILD_FOR_LINUX)
    //Windows文件名不区分大小写
    dirPathStr = StringUtil::MakeLowerString(dirPathStr);
    childPathStr = StringUtil::MakeLowerString(childPathStr);
#endif
    if (childPathStr.size() <= dirPathStr.size()) {
        return false;
    }
    size_t nPos = childPathStr.find(dirPathStr);
    if (nPos != 0) {
        return false;
    }
    DString fileName = childPathStr.substr(dirPathStr.size());
    return fileName.find(FilePath::GetPathSeparatorStr()) == DString::npos;
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

void DirectoryTree::RefreshTree()
{

}

void DirectoryTree::RefreshTreeNode(TreeNode* pTreeNode)
{

}

}//namespace ui
