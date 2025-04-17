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
    if (m_pAddressBar != nullptr) {
        m_pAddressBar->AttachReturn(UiBind(&MainForm::OnAddressBarReturn, this, std::placeholders::_1));
    }
    m_pListBox = dynamic_cast<ui::VirtualListBox*>(FindControl(_T("list")));
    ASSERT(m_pListBox != nullptr);
    if (m_pListBox != nullptr) {
        m_pListBox->SetDataProvider(&m_fileList);
    }

    //刷新按钮
    ui::Button* pRefreshBtn = dynamic_cast<ui::Button*>(FindControl(_T("tree_refresh")));
    if (pRefreshBtn != nullptr) {
        pRefreshBtn->AttachClick([this](const ui::EventArgs&) {
                if (m_pTree != nullptr) {
                    m_pTree->Refresh();
                }
                return true;
            });
    }

    //挂载事件
    m_pTree->AttachShowFolderContents(ui::UiBind(&MainForm::OnShowFolderContents, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    
    //显示虚拟路径
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kUserHome, _T("主文件夹"));
    m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDesktop, _T("桌面"));
    ui::TreeNode* pDocumentsNode = m_pTree->ShowVirtualDirectoryNode(ui::VirtualDirectoryType::kDocuments, _T("文档"));
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

    //初始启动时，默认选择文档
    if (pDocumentsNode != nullptr) {
        m_pTree->SelectTreeNode(pDocumentsNode);
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
    if (m_pTree != nullptr) {
        m_pTree->ExpandTreeNode(pTreeNode, filePath);
    }    
}

bool MainForm::OnAddressBarReturn(const ui::EventArgs& msg)
{
    DString text;
    if (m_pAddressBar != nullptr) {
        text = m_pAddressBar->GetText();
    }
    if (!text.empty()) {
        ui::FilePath filePath(text);
        if (filePath.IsExistsDirectory()) {
            //地址栏上面的是有效路径，让左树展开对应的路径，并选择
            if (m_pTree != nullptr) {
                m_pTree->SelectPath(filePath);
            }
        }
    }
    return true;
}

