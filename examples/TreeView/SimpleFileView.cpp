#include "SimpleFileView.h"
#include "MainForm.h"
#include "MainThread.h"

namespace
{
/** 是否为图片文件（按文件扩展名判断）
*/
bool IsImageFile(const DString& fileName)
{
    DString fileExt;
    size_t pos = fileName.rfind(_T('.'));
    if (pos != DString::npos) {
        fileExt = fileName.substr(pos);
    }
    fileExt = ui::StringUtil::MakeLowerString(fileExt);
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
        (fileExt == _T(".ico")) ||
        (fileExt == _T(".cur")) ) {
        return true;
    }
    return false;
}

/** 缩略图延迟加载的文件大小阈值：超过该大小的图片，先显示占位图，
*  等列表填充完成、界面空闲时再加载真正的缩略图（避免列表填充时集中解码大图导致卡顿）
*/
const uint64_t kThumbnailDeferFileSize = 1024 * 1024;

/** 生成缩略图的最大文件大小：超过该大小的图片，只显示占位图，
*  避免长时间解码并占用大量内存
*/
const uint64_t kThumbnailMaxFileSize = 64 * 1024 * 1024;

/** 缩略图延迟加载定时器的触发间隔（毫秒）
*/
const uint32_t kThumbnailTimerMs = 100;

/** 定时器每次最多加载缩略图的数量（限制并发解码数量，避免集中解码导致CPU占用过高）
*/
const size_t kMaxThumbnailPerTick = 16;

/** 定时器连续多次没有可加载的缩略图时，停止定时器的次数上限
* （列表尚未完成布局，或者待加载项目都不可见时，先等待，避免过早丢弃待加载的项目）
*/
const size_t kMaxThumbIdleCount = 10;

/** 非图片文件的图标原始尺寸（加载时会按DPI缩放，这里使用未经DPI缩放的原值）
*/
const int32_t kImageIconSize = 64;
}//namespace

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
    * @param [in] bDeferThumbnail true表示延迟加载图片缩略图（这里只显示占位图片），false表示立即加载
    */
    void FillSubControls(std::unordered_set<DString>& errorImagePathSet,const ui::DirectoryTree::PathInfo& fileInfo, size_t nElementIndex, bool bDeferThumbnail)
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

            if (!fileInfo.m_bFolder && IsImageFile(fileInfo.m_displayName) &&
                (fileInfo.m_fileSize < kThumbnailMaxFileSize)) {
                if (bDeferThumbnail) {
                    //大图片，先显示占位图片：等列表填充完成、界面空闲时再加载真正的缩略图
                    m_pIconControl->SetBkImage(GetPlaceholderImage());
                }
                else {
                    //图片文件，直接显示缩略图（按显示尺寸解码，避免加载原图）
                    FillThumbnail(fileInfo, errorImagePathSet);
                }
            }
            else {
                //非图片文件或者文件夹，显示图标
                //图标尺寸使用未经DPI缩放的原值（图片加载时会对width/height属性做DPI缩放），
                //避免高DPI下图标尺寸比单元格还大
                DString iconString = ui::GlobalManager::Instance().Icon().GetIconString(fileInfo.m_nIconID);
                if (!iconString.empty()) {
                    int32_t iconSize = kImageIconSize;
                    Dpi().UnscaleInt(iconSize);
                    if (iconSize > 0) {
                        iconString = ui::StringUtil::Printf(_T("file='%s' width='%d' height='%d' halign='center' valign='center'"),
                                                            iconString.c_str(), iconSize, iconSize);
                    }
                    else {
                        iconString = ui::StringUtil::Printf(_T("file='%s' halign='center' valign='center'"), iconString.c_str());
                    }
                    m_pIconControl->SetBkImage(iconString);
                }
                else {
                    m_pIconControl->SetBkImage(_T(""));
                }
            }
        }
    }

    /** 加载图片缩略图（延迟加载时，由文件视图在界面空闲时调用）
    * @param [in] fileInfo 需要显示的路径信息
    * @param [in] errorImagePathSet 错误图片路径保存容器（调用方需保证其生命周期长于本控件）
    */
    void FillThumbnail(const ui::DirectoryTree::PathInfo& fileInfo, std::unordered_set<DString>& errorImagePathSet)
    {
        if (m_pIconControl == nullptr) {
            m_pIconControl = FindSubControl(_T("control_img"));
        }
        if (m_pIconControl == nullptr) {
            return;
        }
        int32_t itemWidth = this->GetWidth() / 2;
        //将宽度还原为未经DPI缩放的原值（因为图片加载时，会对width属性进行DPI缩放）
        Dpi().UnscaleInt(itemWidth);

        //当出现错误（图片加载失败，或者图片解码失败）时，显示一张默认图片
        const DString defaultImage = GetPlaceholderImage();

        if (errorImagePathSet.find(fileInfo.m_filePath.ToString()) == errorImagePathSet.end()) {
            DString imageString = fileInfo.m_filePath.ToString();
            if (itemWidth > 0) {
                imageString = ui::StringUtil::Printf(_T("file='%s' halign='center' valign='center' width='%d' assert='false'"), imageString.c_str(), itemWidth);
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

private:
    /** 生成占位图片描述字符串（缩略图加载前显示，尺寸与缩略图相同）
    */
    DString GetPlaceholderImage()
    {
        int32_t itemWidth = this->GetWidth() / 2;
        //将宽度还原为未经DPI缩放的原值（因为图片加载时，会对width属性进行DPI缩放）
        Dpi().UnscaleInt(itemWidth);
        if (itemWidth > 0) {
            return ui::StringUtil::Printf(_T("file='image-photo.svg' halign='center' valign='center' width='%d'"), itemWidth);
        }
        return _T("file='image-photo.svg' halign='center' valign='center'");
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
    m_pListBox(pListBox),
    m_bMultiSelect(false),
    m_nThumbTimerId(0),
    m_nThumbIdleCount(0)
{
    if (m_pListBox != nullptr) {
        m_pListBox->SetDataProvider(this);
        //右键菜单
        m_pListBox->AttachRClick(UiBind(&SimpleFileView::OnItemRClick, this, std::placeholders::_1));
        //选择项发生变化时，同步更新状态栏中的选中项信息
        m_pListBox->AttachSelect([this](const ui::EventArgs&) {
            if (m_pMainForm != nullptr) {
                m_pMainForm->OnViewSelectionChanged();
            }
            return true;
            });
        m_pListBox->AttachSelChanged([this](const ui::EventArgs&) {
            if (m_pMainForm != nullptr) {
                m_pMainForm->OnViewSelectionChanged();
            }
            return true;
            });
    }
}

SimpleFileView::~SimpleFileView()
{
    StopThumbnailTimer();
    m_pendingThumbIndexs.clear();
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
    item->AttachDoubleClick(UiBind(&SimpleFileView::OnDoubleClickItem, this, std::placeholders::_1));
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
    //大图片的缩略图延迟加载：填充阶段先显示占位图片，等界面空闲时再加载真正的缩略图
    const bool bDeferThumbnail = IsThumbnailDeferred(fileInfo);
    pItem->FillSubControls(m_errorImagePathSet, fileInfo, nElementIndex, bDeferThumbnail);
    if (bDeferThumbnail) {
        m_pendingThumbIndexs.insert(nElementIndex);
        StartThumbnailTimer();
    }
    pItem->SetUserDataID(nElementIndex);
    return true;
}

bool SimpleFileView::IsThumbnailDeferred(const PathInfo& fileInfo) const
{
    if (fileInfo.m_bFolder || !IsImageFile(fileInfo.m_displayName)) {
        return false;
    }
    if (fileInfo.m_fileSize >= kThumbnailMaxFileSize) {
        //文件太大，只显示占位图片，不生成缩略图
        return false;
    }
    return fileInfo.m_fileSize >= kThumbnailDeferFileSize;
}

void SimpleFileView::StartThumbnailTimer()
{
    if (m_nThumbTimerId != 0) {
        return;
    }
    m_nThumbIdleCount = 0;
    m_thumbTimerFlag.Cancel(); //先取消旧的 weakFlag，避免残留
    std::function<void()> callback = UiBind(&SimpleFileView::OnThumbnailTimer, this);
    m_nThumbTimerId = ui::GlobalManager::Instance().Timer().AddTimer(m_thumbTimerFlag.GetWeakFlag(),
                                                                    callback, kThumbnailTimerMs, -1);
    if (m_nThumbTimerId == 0) {
        //定时器启动失败时，直接在当前流程中加载缩略图
        OnThumbnailTimer();
    }
}

void SimpleFileView::StopThumbnailTimer()
{
    if (m_nThumbTimerId != 0) {
        ui::GlobalManager::Instance().Timer().RemoveTimer(m_nThumbTimerId);
        m_nThumbTimerId = 0;
    }
    m_thumbTimerFlag.Cancel();
}

void SimpleFileView::OnThumbnailTimer()
{
    if ((m_pListBox == nullptr) || m_pendingThumbIndexs.empty()) {
        StopThumbnailTimer();
        m_pendingThumbIndexs.clear();
        return;
    }
    //只加载当前可见项目的缩略图，每次限制加载数量，避免集中解码导致CPU占用过高
    size_t nLoadedCount = 0;
    std::vector<size_t> displayElementIndexs;
    m_pListBox->GetDisplayElements(displayElementIndexs);
    for (size_t nElementIndex : displayElementIndexs) {
        if (nLoadedCount >= kMaxThumbnailPerTick) {
            break;
        }
        if (m_pendingThumbIndexs.find(nElementIndex) == m_pendingThumbIndexs.end()) {
            continue;
        }
        LoadThumbnail(nElementIndex);
        m_pendingThumbIndexs.erase(nElementIndex);
        ++nLoadedCount;
    }
    if (nLoadedCount > 0) {
        m_nThumbIdleCount = 0;
        if (m_pendingThumbIndexs.empty()) {
            //所有待加载的缩略图都已加载完成，停止定时器（滚动到可见区域时会重新触发加载）
            StopThumbnailTimer();
        }
        return;
    }
    //本次没有可加载的缩略图：可能列表尚未完成布局，或者待加载的项目已经不可见，
    //等待若干次后再停止定时器（避免过早丢弃待加载的项目，导致缩略图永远不加载）
    ++m_nThumbIdleCount;
    if (m_nThumbIdleCount >= kMaxThumbIdleCount) {
        StopThumbnailTimer();
        m_pendingThumbIndexs.clear();
    }
}

void SimpleFileView::LoadThumbnail(size_t nElementIndex)
{
    if ((m_pListBox == nullptr) || (nElementIndex >= m_pathList.size())) {
        return;
    }
    const size_t nItemIndex = m_pListBox->GetDisplayItemIndex(nElementIndex);
    if (nItemIndex == ui::Box::InvalidIndex) {
        //该数据元素已经不可见，不需要加载
        return;
    }
    FileInfoItem* pItem = dynamic_cast<FileInfoItem*>(m_pListBox->GetItemAt(nItemIndex));
    if (pItem == nullptr) {
        return;
    }
    pItem->FillThumbnail(m_pathList[nElementIndex], m_errorImagePathSet);
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
    //与关联的虚表控件保持一致（由控件在关联数据源时同步过来）
    return m_bMultiSelect;
}

void SimpleFileView::SetMultiSelect(bool bMultiSelect)
{
    m_bMultiSelect = bMultiSelect;
}

void SimpleFileView::SetFileList(const ui::FilePath& currentPath, std::vector<PathInfo> pathList, const ui::FilePath& selectedPath)
{
    //新的文件列表：取消尚未加载完成的缩略图任务
    StopThumbnailTimer();
    m_pendingThumbIndexs.clear();

    m_errorImagePathSet.clear(); //清除错误图片列表
    m_currentPath = currentPath;
    std::vector<PathInfo> oldPathList;
    oldPathList.swap(m_pathList);

    //移动数据，避免大量路径信息的深拷贝
    m_pathList = std::move(pathList);
    m_pathSelectList.clear();
    m_pathSelectList.resize(m_pathList.size(), false); //初始状态均为未选择

    // 通知ListBox数据总数变动
    EmitCountChanged();

    //清理原来的图标资源
    ui::DirectoryTree::ClearPathInfoList(oldPathList);

    //选择子项
    size_t nSelectedItemIndex = ui::Box::InvalidIndex;
    if (!selectedPath.IsEmpty()) {
        for (size_t nIndex = 0; nIndex < m_pathList.size(); ++nIndex) {
            const ui::DirectoryTree::PathInfo& pathInfo = m_pathList[nIndex];
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

void SimpleFileView::GetSelectedPaths(std::vector<ui::FilePath>& paths) const
{
    paths.clear();
    std::vector<size_t> selectedIndexs;
    GetSelectedElements(selectedIndexs);
    for (size_t nIndex : selectedIndexs) {
        if (nIndex < m_pathList.size()) {
            const ui::DirectoryTree::PathInfo& pathInfo = m_pathList[nIndex];
            if (!pathInfo.m_filePath.IsEmpty()) {
                paths.push_back(pathInfo.m_filePath);
            }
        }
    }
}

void SimpleFileView::GetSelectedItemInfo(int32_t& nSelectedCount, uint64_t& nSelectedFileSize) const
{
    nSelectedCount = 0;
    nSelectedFileSize = 0;
    std::vector<size_t> selectedIndexs;
    GetSelectedElements(selectedIndexs);
    for (size_t nIndex : selectedIndexs) {
        if (nIndex < m_pathList.size()) {
            const ui::DirectoryTree::PathInfo& pathInfo = m_pathList[nIndex];
            ++nSelectedCount;
            if (!pathInfo.m_bFolder) {
                nSelectedFileSize += pathInfo.m_fileSize;
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
            if (!fileInfo.m_filePath.IsEmpty()) {
                //双击目录则进入其中，双击文件则用系统默认程序打开
                m_pMainForm->OpenPath(fileInfo.m_filePath);
            }
        }
    }
    return true;
}

bool SimpleFileView::OnItemRClick(const ui::EventArgs& args)
{
    if ((m_pMainForm == nullptr) || (m_pListBox == nullptr)) {
        return true;
    }
    ui::FilePath targetPath;
    if (((size_t)args.lParam != ui::Box::InvalidIndex) && ((size_t)args.lParam < m_pathList.size())) {
        targetPath = m_pathList[(size_t)args.lParam].m_filePath;
    }
    m_pMainForm->ShowViewContextMenu(m_pListBox.get(), targetPath);
    return true;
}

void SimpleFileView::SelectAll()
{
    if (m_pListBox != nullptr) {
        m_pListBox->SetSelectAll();
    }
}
