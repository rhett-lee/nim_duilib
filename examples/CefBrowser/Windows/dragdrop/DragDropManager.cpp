#include "DragDropManager.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "DragForm.h"
#include "DragDrop.h"
#include "Windows/BrowserForm_Windows.h"
#include "browser/BrowserManager.h"

namespace
{
    const int kDragFormXOffset = -100;   //拖拽出新浏览器窗口后的相对鼠标的x偏移坐标
    const int kDragFormYOffset = -20;    //拖拽出新浏览器窗口后的相对鼠标的y偏移坐标
}

DragDropManager::DragDropManager():
    m_bUseCustomDragImage(true)
{
}

DragDropManager::~DragDropManager()
{
}

DragDropManager* DragDropManager::GetInstance()
{
    static DragDropManager self;
    return &self;
}

void DragDropManager::SetUseCustomDragImage(bool use)
{
    m_bUseCustomDragImage = use;
}

bool DragDropManager::IsUseCustomDragImage() const
{
    return m_bUseCustomDragImage;
}

bool DragDropManager::IsDragingBorwserBox() const
{
    return BrowserManager::GetInstance()->IsEnableMerge() && (m_pDragingBox != nullptr);
}

void DragDropManager::SetDropForm(BrowserForm_Windows* browser_form)
{
    m_pDropBrowserForm = browser_form;
}

bool DragDropManager::DoDragBorwserBox(BrowserBox *browser_box, HBITMAP bitmap, POINT pt_offset)
{
    if (!BrowserManager::GetInstance()->IsEnableMerge()) {
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

SdkDataObject* DragDropManager::CreateDragDataObject(HBITMAP bitmap, POINT pt_offset)
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

void DragDropManager::OnBeforeDragBorwserBox(BrowserBox *browser_box, HBITMAP bitmap, POINT pt_offset)
{
    // 获取当前被拖拽的浏览器盒子所属的浏览器窗口
    m_pDragingBox = browser_box;
    BrowserForm_Windows* drag_browser_form = dynamic_cast<BrowserForm_Windows*>(m_pDragingBox->GetBrowserForm());
    ASSERT(drag_browser_form != nullptr);
    if (drag_browser_form == nullptr) {
        return;
    }

    // 获取被拖拽浏览器窗口中浏览器盒子的数量
    int box_count = drag_browser_form->GetBoxCount();
    ASSERT(box_count > 0);
    m_pDropBrowserForm = nullptr;

    drag_browser_form->OnBeforeDragBoxCallback(ui::StringConvert::UTF8ToT(m_pDragingBox->GetId()));

    if (m_bUseCustomDragImage) {
        DragForm::CreateCustomDragImage(bitmap, pt_offset);
    }
}

void DragDropManager::OnAfterDragBorwserBox()
{
    if (m_bUseCustomDragImage) {
        DragForm::CloseCustomDragImage();
    }

    if (m_pDragingBox == nullptr) {
        return;
    }

    // 获取当前被拖拽的浏览器盒子所属的浏览器窗口
    BrowserForm_Windows* drag_browser_form = dynamic_cast<BrowserForm_Windows*>(m_pDragingBox->GetBrowserForm());
    ASSERT(drag_browser_form != nullptr);
    if (drag_browser_form == nullptr) {
        return;
    }

    // 获取被拖拽浏览器窗口中浏览器盒子的数量
    int box_count = drag_browser_form->GetBoxCount();
    ASSERT(box_count > 0);

    // 如果被拖拽的浏览器盒子放入到一个浏览器窗口里
    if (m_pDropBrowserForm != nullptr) {
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
                BrowserForm* browser_form = BrowserManager::GetInstance()->CreateBrowserForm();
                if (browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("CefBrowser")))) {
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

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

