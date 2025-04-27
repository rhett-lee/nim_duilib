#include "SimpleFileView.h"
#include "MainForm.h"
#include "MainThread.h"

class FileInfoItem : public ui::ListBoxItem
{
public:
    explicit FileInfoItem(ui::Window* pWindow):
        ui::ListBoxItem(pWindow),
        m_nElementIndex(ui::Box::InvalidIndex),
        m_pIconControl(nullptr),
        m_pTextControl(nullptr)
    {
    }

    virtual ~FileInfoItem() override
    {
    }
    void InitSubControls(const ui::DirectoryTree::PathInfo& fileInfo, size_t nElementIndex)
    {
        m_nElementIndex = nElementIndex;
        if (m_pIconControl == nullptr) {
            m_pIconControl = FindSubControl(_T("control_img"));
        }
        if (m_pTextControl == nullptr) {
            m_pTextControl = dynamic_cast<ui::Label*>(FindSubControl(_T("control_text")));
        }
        if (m_pTextControl != nullptr) {
            m_pTextControl->SetAutoToolTip(true);
            m_pTextControl->SetText(fileInfo.m_displayName);
        }
        if (m_pIconControl != nullptr) {
            if (!fileInfo.m_bFolder && IsImageFile(fileInfo.m_displayName)) {
                //图片文件，直接显示图片（屏蔽了，运行速度太慢，待优化）
                int32_t itemWidth = 32;// this->GetWidth();
                DString imageString = fileInfo.m_filePath.ToString();
                if (itemWidth > 0) {
                    imageString = ui::StringUtil::Printf(_T("file='%s' halign='center' valign='center' width='%d'"), imageString.c_str(), itemWidth);
                }
                else {
                    imageString = ui::StringUtil::Printf(_T("file='%s' halign='center' valign='center'"), imageString.c_str());
                }
                m_pIconControl->SetBkImage(imageString);
            }
            else {
                //非图片文件或者文件夹，显示图标
                DString iconString = ui::GlobalManager::Instance().Icon().GetIconString(fileInfo.m_nIconID);
                if (!iconString.empty()) {
                    iconString = ui::StringUtil::Printf(_T("file='%s' width='64' height='64' halign='center' valign='center'"), iconString.c_str());
                    m_pIconControl->SetBkImage(iconString);
                }
            }
        }
    }

private:
    /** 是否为图片文件
    */
    bool IsImageFile(const DString& filePath) const
    {
        DString fileExt;
        size_t pos = filePath.rfind(_T('.'));
        if (pos != DString::npos) {
            fileExt = filePath.substr(pos);
        }
        fileExt = ui::StringUtil::MakeLowerString(fileExt);
        if (fileExt == _T(".svg")) {
            return true;
        }
        /*if ((fileExt == _T(".svg")) ||
            (fileExt == _T(".jpg")) ||
            (fileExt == _T(".jpeg")) ||
            (fileExt == _T(".gif")) ||
            (fileExt == _T(".png")) ||
            (fileExt == _T(".bmp")) ||
            (fileExt == _T(".webp")) ||
            (fileExt == _T(".ico"))) {
            return true;
        }*/
        return false;
    }

private:
    //元素的索引号
    size_t m_nElementIndex;

    //图标控件
    ui::Control* m_pIconControl;

    //文字控件
    ui::Label* m_pTextControl;
};

SimpleFileView::SimpleFileView(MainForm* pMainForm, ui::VirtualListBox* pListBox):
    m_pMainForm(pMainForm),
    m_pListBox(pListBox)
{
    if (m_pListBox != nullptr) {
        m_pListBox->SetDataProvider(this);
    }
}

SimpleFileView::~SimpleFileView()
{
    ClearFileList(m_pathList);
    if (m_pListBox != nullptr) {
        m_pListBox->SetDataProvider(nullptr);
    }
}

ui::Control* SimpleFileView::CreateElement(ui::VirtualListBox* pVirtualListBox)
{
    ASSERT(pVirtualListBox != nullptr);
    if (pVirtualListBox == nullptr) {
        return nullptr;
    }
    ASSERT(pVirtualListBox->GetWindow() != nullptr);
    FileInfoItem* item = new FileInfoItem(pVirtualListBox->GetWindow());
    item->AttachEvent(ui::kEventMouseDoubleClick, UiBind(&SimpleFileView::OnDoubleClickItem, this, std::placeholders::_1));
    ui::GlobalManager::Instance().FillBoxWithCache(item, ui::FilePath(_T("tree_view/tree_node.xml")));
    return item;
}

bool SimpleFileView::FillElement(ui::Control* pControl, size_t nElementIndex)
{
    FileInfoItem* pItem = dynamic_cast<FileInfoItem*>(pControl);
    ASSERT(pItem != nullptr);
    ASSERT(nElementIndex < m_pathList.size());
    if ((pItem == nullptr) || (nElementIndex >= m_pathList.size())) {
        return false;
    }
    const PathInfo& fileInfo = m_pathList[nElementIndex];
    pItem->InitSubControls(fileInfo, nElementIndex);
    pItem->SetUserDataID(nElementIndex);
    return true;
}

size_t SimpleFileView::GetElementCount() const
{
    return m_pathList.size();
}

void SimpleFileView::SetElementSelected(size_t nElementIndex, bool bSelected)
{

}

bool SimpleFileView::IsElementSelected(size_t nElementIndex) const
{
    return false;
}

void SimpleFileView::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
}

bool SimpleFileView::IsMultiSelect() const
{
    return false;
}

void SimpleFileView::SetMultiSelect(bool /*bMultiSelect*/)
{
}

void SimpleFileView::SetFileList(const std::vector<PathInfo>& pathList)
{
    std::vector<PathInfo> oldPathList;
    oldPathList.swap(m_pathList);

    m_pathList.reserve(m_pathList.size() + pathList.size());
    for (const PathInfo& fileInfo : pathList) {
        m_pathList.emplace_back(fileInfo);
    }
    // 通知ListBox数据总数变动
    EmitCountChanged();

    //清理原来的图标资源
    ClearFileList(oldPathList);
}

void SimpleFileView::ClearFileList(std::vector<PathInfo>& pathList) const
{
    for (const PathInfo& fileInfo : pathList) {
        if (!fileInfo.m_bIconShared) {
            ui::GlobalManager::Instance().Icon().RemoveIcon(fileInfo.m_nIconID);
        }        
    }
    pathList.clear();
}

bool SimpleFileView::OnDoubleClickItem(const ui::EventArgs& args)
{
    if (m_pMainForm == nullptr) {
        return true;
    }

    FileInfoItem* pItem = dynamic_cast<FileInfoItem*>(args.GetSender());
    if (pItem != nullptr) {
        size_t nElementIndex = pItem->GetUserDataID();
        if (nElementIndex < m_pathList.size()) {
            const PathInfo& fileInfo = m_pathList[nElementIndex];
            if (fileInfo.m_bFolder) {
                //双击在一个目录上
                m_pMainForm->SelectSubPath(fileInfo.m_filePath);
            }
        }
    }
    return true;
}
