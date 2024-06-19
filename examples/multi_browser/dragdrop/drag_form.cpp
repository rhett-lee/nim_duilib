#include "drag_form.h"
#include "bitmap_control.h"

using namespace ui;

DragForm *DragForm::s_drag_form = NULL;
HHOOK DragForm::s_mouse_hook = NULL;
POINT DragForm::s_point_offset = { 0, 0 };

LRESULT CALLBACK DragForm::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        if (wParam == WM_MOUSEMOVE &&::GetKeyState(VK_LBUTTON) < 0)
        {
            MOUSEHOOKSTRUCT *pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
            if (NULL != pMouseStruct)
            {
                if (NULL != s_drag_form)
                {
                    //系统拖拽窗口。标题：Drag，类名：SysDragImage，GWL_EXSTYLE:524456
                    //WS_EX_TOPMOST--WS_EX_TRANSPARENT--WS_EX_PALETTEWINDOW--WS_EX_LAYERED--WS_EX_TOOLWINDOW
                    ui::UiRect rc(pMouseStruct->pt.x - s_point_offset.x, pMouseStruct->pt.y - s_point_offset.y, 0, 0);
                    UiPadding rcShadow;
                    s_drag_form->GetShadowCorner(rcShadow);
                    rc.Inflate(rcShadow);
                    s_drag_form->SetWindowPos(InsertAfterWnd(), rc.left, rc.top, rc.Width(), rc.Height(), kSWP_NOSIZE);
                }
            }
        }
    }

    return CallNextHookEx(s_mouse_hook, nCode, wParam, lParam);
}

void DragForm::CreateCustomDragImage(HBITMAP bitmap, POINT pt_offset)
{
    ASSERT(NULL != bitmap);
    s_mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)DragForm::LowLevelMouseProc, GetModuleHandle(NULL), 0);
    s_drag_form = DragForm::CreateDragForm(bitmap, pt_offset);
    s_point_offset = pt_offset;
}

void DragForm::CloseCustomDragImage()
{
    if (NULL != s_drag_form)
    {
        s_drag_form->CloseWnd();
        s_drag_form = NULL;
    }

    if (NULL != s_mouse_hook)
    {
        UnhookWindowsHookEx(s_mouse_hook);
        s_mouse_hook = NULL;
    }
    
}

DragForm* DragForm::CreateDragForm(HBITMAP bitmap, POINT pt_offset)
{
    DragForm *drag_form = new DragForm;
    ui::WindowCreateParam createParam;
    createParam.m_dwExStyle = ui::kWS_EX_TOPMOST | ui::kWS_EX_TRANSPARENT | ui::kWS_EX_LAYERED | ui::kWS_EX_TOOLWINDOW;
    createParam.m_windowTitle = _T("DragForm");
    drag_form->CreateWnd(nullptr, createParam);
    ASSERT(drag_form->IsWindow());
    drag_form->SetDragImage(bitmap);

    POINT pt;
    BITMAP bitmap_info;
    ::GetCursorPos(&pt);
    GetObject(bitmap, sizeof(BITMAP), &bitmap_info);
    
    // 窗口初始化到鼠标光标的中上方位置
    UiRect rect;
    rect.left = pt.x - pt_offset.x;
    rect.top = pt.y - pt_offset.y;
    rect.right = rect.left + bitmap_info.bmWidth;
    rect.bottom = rect.top + bitmap_info.bmHeight;

    UiPadding rcShadow;
    drag_form->GetShadowCorner(rcShadow);
    rect.Inflate(rcShadow);
    drag_form->SetWindowPos(InsertAfterWnd(), rect.left, rect.top, rect.Width(), rect.Height(), kSWP_SHOWWINDOW);
    return drag_form;
}

DragForm::DragForm()
{
    bitmap_control_ = NULL;
}

DString DragForm::GetSkinFolder()
{
    return _T("multi_browser");
}

DString DragForm::GetSkinFile()
{
    return _T("drag_form.xml");
}

ui::Control* DragForm::CreateControl(const DString& pstrClass)
{
    if (pstrClass == _T("BitmapControl"))
    {
        return new BitmapControl(this);
    }

    return NULL;
}

void DragForm::OnInitWindow()
{
    bitmap_control_ = static_cast<BitmapControl*>(FindControl(_T("bitmap")));
    ASSERT(NULL != bitmap_control_);

    // 设置背景透明度
    bitmap_control_->SetAlpha(128);
    this->SetFocusControl(bitmap_control_);
}

void DragForm::SetDragImage(HBITMAP bitmap)
{
    ASSERT(NULL != bitmap);
    bitmap_control_->SetBitmapImage(bitmap);
}
