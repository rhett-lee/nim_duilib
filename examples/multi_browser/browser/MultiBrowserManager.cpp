#include "MultiBrowserManager.h"
#include "MultiBrowserForm.h"
#include "BrowserBox.h"
#include "dragdrop/DragForm.h"
#include "dragdrop/DragDrop.h"

namespace
{
    const int kSplitFormXOffset = 20;    //自动拆分浏览器窗口后新窗口的x偏移坐标
    const int kSplitFormYOffset = 20;    //自动拆分浏览器窗口后新窗口的y偏移坐标
    const int kDragFormXOffset = -100;    //拖拽出新浏览器窗口后的相对鼠标的x偏移坐标
    const int kDragFormYOffset = -20;    //拖拽出新浏览器窗口后的相对鼠标的y偏移坐标
}

BrowserBox* MultiBrowserManager::CreateBorwserBox(MultiBrowserForm *browser_form, const std::string &id, const DString &url)
{
    BrowserBox *browser_box = nullptr;
    // 如果启用了窗口合并功能，就把新浏览器盒子都集中创建到某一个浏览器窗口里
    // 否则每个浏览器盒子都创建一个浏览器窗口
    if (m_bEnableMerge) {
        if (!browser_form) {
            browser_form = new MultiBrowserForm;
            if (!browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("MultiBrowser"), true))) {
                browser_form = nullptr;
                return nullptr;
            }
            browser_form->ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);          
        }
        browser_box = browser_form->CreateBox(id, url);
        if (nullptr == browser_box) {
            return nullptr;
        }
    }
    else {
        browser_form = new MultiBrowserForm;
        if (!browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("MultiBrowser"), true))) {
            return nullptr;
        }
        browser_box = browser_form->CreateBox(id, url);
        if (nullptr == browser_box) {
            return nullptr;
        }
        browser_form->ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);
    }

    m_boxMap[id] = browser_box;
    return browser_box;
}

MultiBrowserManager::MultiBrowserManager()
{
    m_bEnableMerge = true;
    m_bUseCustomDragImage = true;
}

MultiBrowserManager::~MultiBrowserManager()
{

}

MultiBrowserManager* MultiBrowserManager::GetInstance()
{
    static MultiBrowserManager self;
    return &self;
}

bool MultiBrowserManager::IsBorwserBoxActive(const std::string& id)
{
    BrowserBox *browser_box = FindBorwserBox(id);
    if (nullptr != browser_box) {
        MultiBrowserForm *parent_form = browser_box->GetBrowserForm();
        return parent_form->IsActiveBox(browser_box);
    }
    return false;
}

BrowserBox* MultiBrowserManager::FindBorwserBox( const std::string &id )
{
    std::map<std::string, BrowserBox*>::const_iterator i = m_boxMap.find(id);
    if (i == m_boxMap.end()) {
        return nullptr;
    }
    else {
        return i->second;
    }
}

void MultiBrowserManager::RemoveBorwserBox( std::string id, const BrowserBox* box /*=nullptr*/)
{
    auto it_box = m_boxMap.find(id);
    if (it_box == m_boxMap.end()) {
        ASSERT(0);
    }
    else {
        if (nullptr == box || box == it_box->second) {
            m_boxMap.erase(it_box);
        }
        else {
            ASSERT(0);
        }
    }

    if (m_boxMap.empty()) {
        ui::CefManager::GetInstance()->PostQuitMessage(0);
    }
}

void MultiBrowserManager::SetEnableMerge(bool enable)
{
    if (m_bEnableMerge == enable) {
        return;
    }

    m_bEnableMerge = enable;

    if (m_bEnableMerge) {
        // 如果当前只有一个浏览器窗口或者浏览器盒子，就不需要进行合并操作
        if (m_boxMap.size() <= 1) {
            return;
        }

        // 选择第一个浏览器盒子所属的窗口作为合并窗口
        MultiBrowserForm *merge_form = m_boxMap.begin()->second->GetBrowserForm();

        // 遍历所有浏览器盒子，脱离原浏览器窗口，再附加到合并窗口里
        for (auto it_box : m_boxMap) {
            ASSERT(nullptr != it_box.second);
            MultiBrowserForm *parent_form = it_box.second->GetBrowserForm();
            if (merge_form != parent_form) {
                if (parent_form->DetachBox(it_box.second)) {
                    merge_form->AttachBox(it_box.second);
                }
            }
        }
    }
    else {
        // 如果当前只有一个浏览器盒子，就不需要进行拆分操作
        if (m_boxMap.size() <= 1) {
            return;
        }

        // 给新拆分的窗口设置坐标
        bool first_sort = true;
        ui::UiRect rect_old_form;
        MultiBrowserForm *sort_form = nullptr;

        // 遍历所有浏览器盒子，脱离原浏览器窗口，创建新的浏览器窗口并附加浏览器盒子
        for (auto it_box : m_boxMap) {
            ASSERT(nullptr != it_box.second);
            MultiBrowserForm *parent_form = it_box.second->GetBrowserForm();
            if (1 == parent_form->GetBoxCount()) {
                sort_form = parent_form;
            }
            else if (parent_form->DetachBox(it_box.second)) {
                MultiBrowserForm *browser_form = new MultiBrowserForm;
                if (!browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("MultiBrowser")))) {
                    ASSERT(0);
                    continue;
                }
                if (!browser_form->AttachBox(it_box.second)) {
                    ASSERT(0);
                    continue;
                }
                browser_form->ShowWindow(ui::kSW_SHOW);
                sort_form = browser_form;
            }

            if (nullptr != sort_form) {
                if (first_sort) {
                    first_sort = false;
                    sort_form->CenterWindow();
                    rect_old_form = sort_form->GetWindowPos(true);
                }
                else {
                    rect_old_form.left += sort_form->Dpi().GetScaleInt(kSplitFormXOffset);
                    rect_old_form.top += sort_form->Dpi().GetScaleInt(kSplitFormXOffset);
                    sort_form->SetWindowPos(ui::InsertAfterWnd(), rect_old_form.left, rect_old_form.top, rect_old_form.Width(), rect_old_form.Height(), ui::kSWP_NOSIZE);
                }
            }
        }
    }
}

bool MultiBrowserManager::IsEnableMerge() const
{
    return m_bEnableMerge;
}

void MultiBrowserManager::SetUseCustomDragImage(bool use)
{
    m_bUseCustomDragImage = use;
}

bool MultiBrowserManager::IsUseCustomDragImage() const
{
    return m_bUseCustomDragImage;
}

bool MultiBrowserManager::IsDragingBorwserBox() const
{
    return m_bEnableMerge && (nullptr != m_pDragingBox);
}

void MultiBrowserManager::SetDropForm(MultiBrowserForm *browser_form)
{
    if (nullptr == browser_form) {
        return;
    }
    m_pDropBrowserForm = browser_form;
}

bool MultiBrowserManager::DoDragBorwserBox(BrowserBox *browser_box, HBITMAP bitmap, POINT pt_offset)
{
    if (!m_bEnableMerge) {
        return false;
    }

    SdkDropSource* drop_src = new SdkDropSource;
    if (drop_src == nullptr) {
        return false;
    }

    SdkDataObject* data_object = CreateDragDataObject(bitmap, pt_offset);
    if (data_object == nullptr) {
        return false;
    }

    // 无论什么时候都让拖拽时光标显示为箭头
    drop_src->SetFeedbackCursor(::LoadCursor(nullptr, IDC_ARROW));

    OnBeforeDragBorwserBox(browser_box, bitmap, pt_offset);

    // 此函数会阻塞直到拖拽完成
    DWORD dwEffect = 0;
    HRESULT hr = ::DoDragDrop(data_object, drop_src, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);

    OnAfterDragBorwserBox();

    // 销毁位图
    DeleteObject(bitmap);
    drop_src->Release();
    data_object->Release();
    return true;
}

SdkDataObject* MultiBrowserManager::CreateDragDataObject(HBITMAP bitmap, POINT pt_offset)
{
    SdkDataObject* data_object = new SdkDataObject;
    if (data_object == nullptr) {
        return nullptr;
    }

    if (m_bUseCustomDragImage) {
        FORMATETC fmtetc = { 0 };
        fmtetc.dwAspect = DVASPECT_CONTENT;
        fmtetc.lindex = -1;
        fmtetc.cfFormat = CF_HDROP;
        fmtetc.tymed = TYMED_NULL;

        STGMEDIUM medium = { 0 };
        medium.tymed = TYMED_NULL;
        data_object->SetData(&fmtetc, &medium, FALSE);
    }
    else {
        FORMATETC fmtetc = { 0 };
        fmtetc.dwAspect = DVASPECT_CONTENT;
        fmtetc.lindex = -1;
        fmtetc.cfFormat = CF_BITMAP;
        fmtetc.tymed = TYMED_GDI;

        STGMEDIUM medium = { 0 };
        medium.tymed = TYMED_GDI;
        HBITMAP hBitmap = (HBITMAP)OleDuplicateData(bitmap, fmtetc.cfFormat, 0);
        medium.hBitmap = hBitmap;
        data_object->SetData(&fmtetc, &medium, FALSE);

        BITMAP bitmap_info;
        GetObject(hBitmap, sizeof(BITMAP), &bitmap_info);
        SIZE bitmap_size = { bitmap_info.bmWidth, bitmap_info.bmHeight };
        SdkDragSourceHelper dragSrcHelper;
        dragSrcHelper.InitializeFromBitmap(hBitmap, pt_offset, bitmap_size, data_object, RGB(255, 0, 255));
    }

    return data_object;
}

void MultiBrowserManager::OnBeforeDragBorwserBox(BrowserBox *browser_box, HBITMAP bitmap, POINT pt_offset)
{
    // 获取当前被拖拽的浏览器盒子所属的浏览器窗口
    m_pDragingBox = browser_box;
    MultiBrowserForm *drag_browser_form = m_pDragingBox->GetBrowserForm();
    ASSERT(nullptr != drag_browser_form);

    // 获取被拖拽浏览器窗口中浏览器盒子的数量
    int box_count = drag_browser_form->GetBoxCount();
    ASSERT(box_count > 0);
    m_pDropBrowserForm = nullptr;

    drag_browser_form->OnBeforeDragBoxCallback(ui::StringConvert::UTF8ToT(m_pDragingBox->GetId()));

    if (m_bUseCustomDragImage) {
        DragForm::CreateCustomDragImage(bitmap, pt_offset);
    }
}

void MultiBrowserManager::OnAfterDragBorwserBox()
{
    if (m_bUseCustomDragImage) {
        DragForm::CloseCustomDragImage();
    }

    if (nullptr == m_pDragingBox) {
        return;
    }

    // 获取当前被拖拽的浏览器盒子所属的浏览器窗口
    MultiBrowserForm *drag_browser_form = m_pDragingBox->GetBrowserForm();
    ASSERT(nullptr != drag_browser_form);

    // 获取被拖拽浏览器窗口中浏览器盒子的数量
    int box_count = drag_browser_form->GetBoxCount();
    ASSERT(box_count > 0);

    // 如果被拖拽的浏览器盒子放入到一个浏览器窗口里
    if (nullptr != m_pDropBrowserForm) {
        if (drag_browser_form == m_pDropBrowserForm) {
            drag_browser_form->OnAfterDragBoxCallback(false);
        }
        else {
            drag_browser_form->OnAfterDragBoxCallback(true);
            if (drag_browser_form->DetachBox(m_pDragingBox)) {
                m_pDropBrowserForm->AttachBox(m_pDragingBox);
            }
        }

        // 如果被拖拽的浏览器窗口包含多个浏览器盒子，就投递一个WM_LBUTTONUP消息给窗口
        // (因为窗口被拖拽时触发了ButtonDown和ButtonMove消息，但是最终的ButtonUp消息会被忽略，这里补上)
        // 如果只有一个浏览器盒子，则浏览器盒子脱离浏览器窗口时，浏览器窗口就会关闭，不需要投递
        if (box_count > 1) {
            drag_browser_form->PostMsg(WM_LBUTTONUP, 0, 0);
        }
    }
    // 如果没有被拖拽到另一个浏览器窗口里
    else {
        // 如果被拖拽的浏览器窗口里只有一个浏览器盒子,则拖拽失败
        if (1 == box_count) {
            drag_browser_form->OnAfterDragBoxCallback(false);
        }
        // 如果有多个浏览器盒子, 就把浏览器盒子脱离原浏览器窗口，附加到新的浏览器窗口，拖拽成功
        else {
            drag_browser_form->OnAfterDragBoxCallback(true);

            if (drag_browser_form->DetachBox(m_pDragingBox)) {
                MultiBrowserForm *browser_form = new MultiBrowserForm;
                if (browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("MultiBrowser")))) {
                    if (browser_form->AttachBox(m_pDragingBox)) {
                        // 这里设置新浏览器窗口的位置，设置到偏移鼠标坐标100,20的位置
                        ui::UiPoint pt_mouse;
                        browser_form->GetCursorPos(pt_mouse);
                        ui::UiRect rect(pt_mouse.x + browser_form->Dpi().GetScaleInt(kDragFormXOffset),
                                        pt_mouse.y + browser_form->Dpi().GetScaleInt(kDragFormYOffset),
                                        0, 0);
                        browser_form->SetWindowPos(ui::InsertAfterWnd(), rect.left, rect.top, rect.Width(), rect.Height(), ui::kSWP_NOSIZE);
                        browser_form->ShowWindow(ui::kSW_SHOW_NORMAL);
                    }
                }
            }
        }

        // 如果没有被拖拽到另一个浏览器窗口里，这时不会有浏览器窗口被关闭，所以直接投递ButtonUp消息
        drag_browser_form->PostMsg(WM_LBUTTONUP, 0, 0);
    }

    m_pDragingBox = nullptr;
    m_pDropBrowserForm = nullptr;
}
