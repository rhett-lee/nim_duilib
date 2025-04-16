#include "DirectoryTree.h"
#include "DirectoryTreeImpl.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui
{
DirectoryTree::DirectoryTree(Window* pWindow):
    TreeView(pWindow),
    m_nThreadIdentifier(ui::kThreadWorker),
    m_nIconSize(16)
{
    m_impl = new DirectoryTreeImpl(this);
}

DirectoryTree::~DirectoryTree()
{
    for (auto p : m_folderList) {
        if (!p->m_bIconShared) {
            GlobalManager::Instance().Icon().RemoveIcon(p->m_nIconID);
        }        
        delete p;
    }
    m_folderList.clear();
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
    ui::TreeNode* pFirstNode = nullptr;
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
        ui::Control* pLineControl = new ui::Control(GetWindow());
        if (!lineClassName.empty()) {
            pLineControl->SetClass(lineClassName);
        }
        else {
            pLineControl->SetClass(_T("splitline_hor_level1"));
            pLineControl->SetMargin(ui::UiMargin(12, 8, 12, 8), true);
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
    TreeNode* node = new ui::TreeNode(GetWindow());
    node->SetClass(_T("tree_node"));//在"global.xml"中定义
    if (bDisplayNameIsID) {
        node->SetTextId(displayName);
    }
    else {
        node->SetText(displayName);
    }

    FolderStatus* pFolder = new FolderStatus;
    pFolder->m_path = path;
    pFolder->m_nIconID = nIconID;
    pFolder->m_bIconShared = bIconShared;
    pFolder->m_pTreeNode = node;
    m_folderList.push_back(pFolder);
    node->SetUserDataID((size_t)pFolder);

    node->SetBkIconID(nIconID, m_nIconSize, true);//设置树节点的关联图标(图标大小与CheckBox的原图大小相同，都是16*16)
    

    if (isFolder) {
        pFolder->m_bShow = false;
        node->SetExpand(false, false);
    }
    else {
        pFolder->m_bShow = true;
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
    node->AttachSelect(UiBind(&DirectoryTree::OnTreeNodeSelect, this, std::placeholders::_1));
    return node;
}

bool DirectoryTree::OnTreeNodeExpand(const EventArgs& args)
{
    ui::TreeNode* pTreeNode = dynamic_cast<TreeNode*>(args.GetSender());
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

bool DirectoryTree::OnTreeNodeClick(const EventArgs& args)
{
    TreeNode* pTreeNode = dynamic_cast<TreeNode*>(args.GetSender());
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

bool DirectoryTree::OnTreeNodeSelect(const EventArgs& args)
{
    if (IsMultiSelect()) {
        //多选的时候，不响应选择事件
        return true;
    }
    return OnTreeNodeClick(args);
}

void DirectoryTree::CheckExpandTreeNode(TreeNode* pTreeNode, const FilePath& filePath)
{
    if ((pTreeNode == nullptr) || filePath.IsEmpty()) {
        return;
    }
    size_t itemIndex = GetItemIndex(pTreeNode);
    if (!Box::IsValidItemIndex(itemIndex)) {
        return;
    }

    FolderStatus* pFolder = (FolderStatus*)pTreeNode->GetUserDataID();
    auto iter = std::find(m_folderList.begin(), m_folderList.end(), pFolder);
    if (iter == m_folderList.end()) {
        return;
    }
    if (filePath.ToString().find(pFolder->m_path.ToString()) != 0) {
        //filePath不是该节点的子目录，不处理
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
    
    //由于ShowSubFolders是在子线程中执行的，所以这里也要先发给子线程，再转给UI线程，保证时序正确
    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    ui::GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, pTreeNode, filePath]() {
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

void DirectoryTree::ShowSubFolders(TreeNode* pTreeNode, const FilePath& path)
{
    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(m_nThreadIdentifier)) {
        nThreadIdentifier = m_nThreadIdentifier;
    }
    std::weak_ptr<WeakFlag> treeNodeFlag = pTreeNode->GetWeakFlag();
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, ToWeakCallback([this, treeNodeFlag, pTreeNode, path]() {
            //在子线程中读取子目录数据
            std::shared_ptr<std::vector<DirectoryTree::PathInfo>> folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
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

void DirectoryTree::OnShowSubFolders(TreeNode* pTreeNode, const FilePath& /*path*/, const std::shared_ptr<std::vector<DirectoryTree::PathInfo>>& folderList)
{
    GlobalManager::Instance().AssertUIThread();
    if (pTreeNode == nullptr) {
        return;
    }
    size_t itemIndex = GetItemIndex(pTreeNode);
    if (Box::IsValidItemIndex(itemIndex) && (folderList != nullptr)) {
        for (const DirectoryTree::PathInfo& pathInfo : *folderList) {
            if (!pathInfo.m_filePath.IsEmpty()) {
                InsertTreeNode(pTreeNode, pathInfo.m_displayName, false, pathInfo.m_filePath, pathInfo.m_bFolder, pathInfo.m_nIconID, pathInfo.m_bIconShared);
            }
        }
    }
    if (Box::IsValidItemIndex(itemIndex) && !pTreeNode->IsExpand()) {
        pTreeNode->SetExpand(true, true);
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
            std::shared_ptr<std::vector<DirectoryTree::PathInfo>> folderList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
            std::shared_ptr<std::vector<DirectoryTree::PathInfo>> fileList = std::make_shared<std::vector<DirectoryTree::PathInfo>>();
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
                                         const std::shared_ptr<std::vector<DirectoryTree::PathInfo>>& folderList,
                                         const std::shared_ptr<std::vector<DirectoryTree::PathInfo>>& fileList)
{
    GlobalManager::Instance().AssertUIThread();
    for (ShowFolderContentsEvent callback : m_callbackList) {
        if (callback) {
            callback(pTreeNode, path, folderList, fileList);
        }
    }
}

}//namespace ui
