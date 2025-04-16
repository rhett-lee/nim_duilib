#include "MainForm.h"
#include "MainThread.h"

MainForm::MainForm():
    m_fileList(this),
    m_pTree(nullptr),
    m_pAddressBar(nullptr),
    m_pListBox(nullptr)
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
    m_pAddressBar = dynamic_cast<ui::RichEdit*>(FindControl(_T("file_path")));
    m_pListBox = dynamic_cast<ui::VirtualListBox*>(FindControl(_T("list")));
    ASSERT(m_pListBox != nullptr);
    if (m_pListBox != nullptr) {
        m_pListBox->SetDataProvider(&m_fileList);
    }
    //挂载事件
    m_pTree->AttachShowFolderContents(ui::UiBind(&MainForm::OnShowFolderContents, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    
    //显示虚拟路径
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kUserHome, _T("主文件夹"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDesktop, _T("桌面"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDocuments, _T("文档"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kPictures, _T("图片"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kMusic, _T("音乐"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kVideos, _T("视频"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDownloads, _T("下载"));

    //显示磁盘
    ui::TreeNode* pFirstDiskNode = m_pTree->ShowAllDiskNodes();
    if (pFirstDiskNode != nullptr) {
        //在磁盘前面，放一个横线分隔符
        m_pTree->InsertLineBeforeNode(pFirstDiskNode);
    }
}

void MainForm::OnShowFolderContents(ui::TreeNode* pTreeNode, const ui::FilePath& path,
                                    const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList,
                                    const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList)
{
    ui::GlobalManager::Instance().AssertUIThread();
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
            m_fileList.SetFileList(pTreeNode, pathList);
        }
    }
    else {
        //多选，不校验
        m_fileList.SetFileList(pTreeNode, pathList);
    }
}

void MainForm::CheckExpandTreeNode(ui::TreeNode* pTreeNode, const ui::FilePath& filePath)
{
    m_pTree->CheckExpandTreeNode(pTreeNode, filePath);
}

