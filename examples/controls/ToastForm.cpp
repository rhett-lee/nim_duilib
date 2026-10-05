#include "ToastForm.h"

ToastForm::ToastForm()
{
}

ToastForm::~ToastForm()
{
}

DString ToastForm::GetSkinFolder()
{
    return _T("controls");
}

DString ToastForm::GetSkinFile()
{
    return _T("toast_test.xml");
}

void ToastForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //单个通知演示项：按钮名、文本ID、类型、停留时长、显示位置（文本均按多语言ID解析）
    struct DemoItem
    {
        DString buttonName;
        DString textId;
        ui::ToastWnd::ToastType type;
        int32_t durationMs;
        ui::ToastWnd::ToastPosition position;
    };
    const DemoItem demos[] = {
        { _T("btn_demo_info"),    _T("STRID_TOAST_DEMO_INFO_TEXT"),
          ui::ToastWnd::kTypeInfo,    3000, ui::ToastWnd::kPosTop },
        { _T("btn_demo_success"), _T("STRID_TOAST_DEMO_SUCCESS_TEXT"),
          ui::ToastWnd::kTypeSuccess, 3000, ui::ToastWnd::kPosTop },
        { _T("btn_demo_warning"), _T("STRID_TOAST_DEMO_WARNING_TEXT"),
          ui::ToastWnd::kTypeWarning, 5000, ui::ToastWnd::kPosTop },
        { _T("btn_demo_error"),   _T("STRID_TOAST_DEMO_ERROR_TEXT"),
          ui::ToastWnd::kTypeError,   5000, ui::ToastWnd::kPosTop },
        { _T("btn_demo_long"),    _T("STRID_TOAST_DEMO_LONG_TEXT"),
          ui::ToastWnd::kTypeInfo,    6000, ui::ToastWnd::kPosTop },
        { _T("btn_demo_manual"),  _T("STRID_TOAST_DEMO_MANUAL_TEXT"),
          ui::ToastWnd::kTypeWarning, 0,    ui::ToastWnd::kPosTop },
        { _T("btn_demo_position"),_T("STRID_TOAST_DEMO_POSITION_TEXT"),
          ui::ToastWnd::kTypeInfo,    3000, ui::ToastWnd::kPosBottomRight }
    };

    for (const DemoItem& item : demos) {
        ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(item.buttonName));
        if (pButton == nullptr) {
            continue;
        }
        DemoItem itemCopy = item;
        pButton->AttachClick([this, itemCopy](const ui::EventArgs& /*args*/) {
            ui::ToastWnd::Show(this, itemCopy.textId, itemCopy.type,
                               itemCopy.durationMs, itemCopy.position, true);
            return true;
        });
    }

    //连续弹出 4 条不同类型通知，演示从顶部向下堆叠与关闭后自动补齐
    ui::Button* pStackButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_demo_stack")));
    if (pStackButton != nullptr) {
        pStackButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::ToastWnd::Show(this, _T("STRID_TOAST_DEMO_INFO_TEXT"),
                               ui::ToastWnd::kTypeInfo, 4000, ui::ToastWnd::kPosTop, true);
            ui::ToastWnd::Show(this, _T("STRID_TOAST_DEMO_SUCCESS_TEXT"),
                               ui::ToastWnd::kTypeSuccess, 4000, ui::ToastWnd::kPosTop, true);
            ui::ToastWnd::Show(this, _T("STRID_TOAST_DEMO_WARNING_TEXT"),
                               ui::ToastWnd::kTypeWarning, 4000, ui::ToastWnd::kPosTop, true);
            ui::ToastWnd::Show(this, _T("STRID_TOAST_DEMO_ERROR_TEXT"),
                               ui::ToastWnd::kTypeError, 4000, ui::ToastWnd::kPosTop, true);
            return true;
        });
    }
}
