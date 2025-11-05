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

    /** 填充子控件信息
    * @param [in] errorImagePathSet 错误图片路径保存容器
    * @param [in] fileInfo 需要显示的路径信息
    * @param [in] nElementIndex 数据元素下标值
    */
    void FillSubControls(std::unordered_set<DString>& errorImagePathSet,const ui::DirectoryTree::PathInfo& fileInfo, size_t nElementIndex)
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
            //删除图片加载和解码事件的响应函数
            m_pIconControl->DetachEvent(ui::kEventImageLoad);
            m_pIconControl->DetachEvent(ui::kEventImageDecode);

            if (!fileInfo.m_bFolder && IsImageFile(fileInfo.m_displayName)) {
                //图片文件，直接显示图片（屏蔽了，运行速度太慢，待优化）
                int32_t itemWidth = this->GetWidth() / 2;
                //将宽度还原为未经DPI缩放的原值（因为图片加载时，会对width属性进行DPI缩放）
                itemWidth = itemWidth * 100 / (int32_t)Dpi().GetScale();

                //当出现错误（图片加载失败，或者图片解码失败）时，显示一张默认图片
                const DString defaultImage = ui::StringUtil::Printf(_T("file='image-photo.svg' halign='center' valign='center' width='%d'"), itemWidth);

                if (errorImagePathSet.find(fileInfo.m_filePath.ToString()) == errorImagePathSet.end()) {
                    DString imageString = fileInfo.m_filePath.ToString();
                    if (itemWidth > 0) {
                        imageString = ui::StringUtil::Printf(_T("file='%s' halign='center' valign='center' width='%d' async_load='false' assert='false'"), imageString.c_str(), itemWidth);
                    }
                    else {
                        imageString = ui::StringUtil::Printf(_T("file='%s' halign='center' valign='center'"), imageString.c_str());
                    }
                    m_pIconControl->SetBkImage(imageString);
                }
                else {
                    //这是一张有错误的图片, 直接显示错图（避免图片显示时，错误图片的闪烁问题）
                    m_pIconControl->SetBkImage(defaultImage);
                }

                //图片加载失败时，显示一张默认图片
                m_pIconControl->AttachImageLoad([this, defaultImage, &errorImagePathSet](const ui::EventArgs& args) {
                    ui::ImageDecodeResult* pImageDecodeResult = (ui::ImageDecodeResult*)args.wParam;
                    if ((pImageDecodeResult != nullptr) && pImageDecodeResult->m_bLoadError) {
                        errorImagePathSet.insert(pImageDecodeResult->m_imageFilePath);
                        ui::Control* pIconControl = FindSubControl(_T("control_img"));
                        if (pIconControl != nullptr) {
                            pIconControl->SetBkImage(defaultImage);
                        }
                    }
                    return true;
                    });

                m_pIconControl->AttachImageDecode([this, defaultImage, &errorImagePathSet](const ui::EventArgs& args) {
                    ui::ImageDecodeResult* pImageDecodeResult = (ui::ImageDecodeResult*)args.wParam;
                    if ((pImageDecodeResult != nullptr) && pImageDecodeResult->m_bDecodeError) {
                        errorImagePathSet.insert(pImageDecodeResult->m_imageFilePath);
                        ui::Control* pIconControl = FindSubControl(_T("control_img"));
                        if (pIconControl != nullptr) {
                            pIconControl->SetBkImage(defaultImage);
                        }
                    }
                    return true;
                    });
            }
            else {
                //非图片文件或者文件夹，显示图标
                DString iconString = ui::GlobalManager::Instance().Icon().GetIconString(fileInfo.m_nIconID);
                if (!iconString.empty()) {
                    iconString = ui::StringUtil::Printf(_T("file='%s' width='64' height='64' halign='center' valign='center'"), iconString.c_str());
                    m_pIconControl->SetBkImage(iconString);
                }
                else {
                    m_pIconControl->SetBkImage(_T(""));
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
        if ((fileExt == _T(".svg")) ||
            (fileExt == _T(".jpg")) ||
            (fileExt == _T(".jpeg")) ||
            (fileExt == _T(".jpe")) ||
            (fileExt == _T(".jif")) ||
            (fileExt == _T(".jfif")) ||
            (fileExt == _T(".jfi")) ||
            (fileExt == _T(".gif")) ||
            (fileExt == _T(".png")) ||
            (fileExt == _T(".bmp")) ||
            (fileExt == _T(".dib")) ||
            (fileExt == _T(".webp")) ||
            (fileExt == _T(".json")) ||
            (fileExt == _T(".pag")) ||
            (fileExt == _T(".ico"))) {
            return true;
        }
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
    ui::DirectoryTree::ClearPathInfoList(m_pathList);
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
    pItem->FillSubControls(m_errorImagePathSet, fileInfo, nElementIndex);
    pItem->SetUserDataID(nElementIndex);
    return true;
}

size_t SimpleFileView::GetElementCount() const
{
    return m_pathList.size();
}

void SimpleFileView::SetElementSelected(size_t nElementIndex, bool bSelected)
{
    if (nElementIndex < m_pathSelectList.size()) {
        m_pathSelectList[nElementIndex] = bSelected;
    }
}

bool SimpleFileView::IsElementSelected(size_t nElementIndex) const
{
    if (nElementIndex < m_pathSelectList.size()) {
        return m_pathSelectList[nElementIndex];
    }
    return false;
}

void SimpleFileView::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
    selectedIndexs.clear();
    for (size_t nElementIndex = 0; nElementIndex < m_pathSelectList.size(); ++nElementIndex) {
        if (m_pathSelectList[nElementIndex]) {
            selectedIndexs.push_back(nElementIndex);
        }
    }
}

bool SimpleFileView::IsMultiSelect() const
{
    //不支持多选
    return false;
}

void SimpleFileView::SetMultiSelect(bool /*bMultiSelect*/)
{
    //禁止切换单选和多选，固定为单选模式
}

void SimpleFileView::SetFileList(const ui::FilePath& currentPath, const std::vector<PathInfo>& pathList, const ui::FilePath& selectedPath)
{
    m_errorImagePathSet.clear(); //清除错误图片列表
    m_currentPath = currentPath;
    std::vector<PathInfo> oldPathList;
    oldPathList.swap(m_pathList);

    m_pathList = pathList;
    m_pathSelectList.clear();
    m_pathSelectList.resize(m_pathList.size(), false); //初始状态均为未选择

    // 通知ListBox数据总数变动
    EmitCountChanged();

    //清理原来的图标资源
    ui::DirectoryTree::ClearPathInfoList(oldPathList);

    //选择子项
    size_t nSelectedItemIndex = ui::Box::InvalidIndex;
    if (!selectedPath.IsEmpty()) {
        for (size_t nIndex = 0; nIndex < pathList.size(); ++nIndex) {
            const ui::DirectoryTree::PathInfo& pathInfo = pathList[nIndex];
            if (selectedPath == pathInfo.m_filePath) {
                //保存选择状态
                m_pathSelectList[nIndex] = true;
                nSelectedItemIndex = nIndex;
                break;
            }
        }
    }

    if ((nSelectedItemIndex != ui::Box::InvalidIndex) && (m_pListBox != nullptr)) {
        m_pListBox->Refresh(true);
        m_pListBox->EnsureVisible(nSelectedItemIndex, false);
    }
}

void SimpleFileView::GetCurrentPath(ui::FilePath& currentPath, ui::FilePath& selectedPath) const
{
    currentPath = m_currentPath;
    std::vector<size_t> selectedIndexs;
    GetSelectedElements(selectedIndexs);
    for (size_t nIndex : selectedIndexs) {
        if (nIndex < m_pathList.size()) {
            const ui::DirectoryTree::PathInfo& pathInfo = m_pathList[nIndex];
            if (!pathInfo.m_filePath.IsEmpty()) {
                //记录当前所选的目录
                selectedPath = pathInfo.m_filePath;
                break;
            }
        }
    }
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
