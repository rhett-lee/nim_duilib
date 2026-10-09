#include "TestForm.h"
#include "MessageBoxForm.h"
#include "ToastForm.h"
#include <ctime>

TestForm::TestForm():
    m_nProgressValue(0.0)
{
}

TestForm::~TestForm()
{
}

DString TestForm::GetSkinFolder()
{
    return _T("controls");
}

DString TestForm::GetSkinFile()
{
    return _T("test.xml");
}

void TestForm::OnInitWindow()
{
    //启动一个定时器
    ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadUI,
        ui::UiBind(this, [this]() {
                OnTimer();
            }),
        100);

    //显示MessageBox的测试界面
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("message_box_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& args) {
            MessageBoxForm* testForm = new MessageBoxForm();
            ui::WindowCreateParam createParam;
            createParam.m_dwStyle = ui::kWS_POPUP;
            createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
            createParam.m_windowTitle = _T("MessageBoxForm");
            createParam.m_bCenterWindow = true;
            testForm->CreateWnd(this, createParam);
            testForm->ShowModalFake();
            return true;
            });
    }

    //显示Toast的测试界面
    ui::Button* pToastButton = dynamic_cast<ui::Button*>(FindControl(_T("toast_btn")));
    if (pToastButton != nullptr) {
        pToastButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            ToastForm* testForm = new ToastForm();
            ui::WindowCreateParam createParam;
            createParam.m_dwStyle = ui::kWS_POPUP;
            createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
            createParam.m_windowTitle = _T("ToastForm");
            createParam.m_bCenterWindow = true;
            testForm->CreateWnd(this, createParam);
            testForm->ShowModalFake();
            return true;
            });
    }

    //Flyout 浮层：4 个按钮分别从下/上/左/右弹出
    struct FlyoutDemoItem
    {
        DString name;
        ui::Flyout::Placement placement;
    };
    const FlyoutDemoItem flyoutItems[] = {
        { _T("flyout_bottom"), ui::Flyout::Placement::Bottom },
        { _T("flyout_top"),    ui::Flyout::Placement::Top },
        { _T("flyout_left"),   ui::Flyout::Placement::Left },
        { _T("flyout_right"),  ui::Flyout::Placement::Right },
    };
    for (const FlyoutDemoItem& item : flyoutItems) {
        ui::Button* pFlyoutButton = dynamic_cast<ui::Button*>(FindControl(item.name));
        if (pFlyoutButton != nullptr) {
            pFlyoutButton->AttachClick([this, placement = item.placement](const ui::EventArgs& args) {
                ui::Control* pAnchor = args.GetSender();
                if (pAnchor == nullptr) {
                    return true;
                }
                ShowFlyoutDemo(pAnchor, placement);
                return true;
                });
        }
    }

    //Calendar 演示：单选/范围 两个按钮分别弹出 CalendarFlyout
    ui::Button* pCalSingle = dynamic_cast<ui::Button*>(FindControl(_T("calendar_single")));
    if (pCalSingle != nullptr) {
        pCalSingle->AttachClick([this](const ui::EventArgs& args) {
            ui::Control* pAnchor = args.GetSender();
            if (pAnchor == nullptr) {
                return true;
            }
            //同一锚点再次点击：切换为关闭已打开的浮层
            ui::Flyout* pActive = ui::Flyout::GetActiveFlyout();
            if ((pActive != nullptr) && (pActive->GetAnchor() == pAnchor)) {
                pActive->Dismiss();
                return true;
            }
            ui::CalendarFlyout* pFlyout = new ui::CalendarFlyout(this);
            pFlyout->SetMode(0); //单选
            pFlyout->AttachDateSelected([this](WPARAM wParam, LPARAM lParam) {
                if (wParam == 0) {
                    time_t t = (time_t)lParam;
                    struct tm date = ui::Calendar::TimeTToDate(t);
                    DString text = ui::Calendar::FormatDateString(date);
                    //text 是格式化后的日期字符串，不是多语言ID，bTextId 必须为 false
                    ui::ToastWnd::Show(this, text, ui::ToastWnd::kTypeInfo, 2000, ui::ToastWnd::kPosTop, false);
                }
            });
            struct tm today = ui::Calendar::GetToday();
            if (!pFlyout->ShowAt(pAnchor, today, ui::Flyout::Placement::Bottom)) {
                //创建失败时 ShowAt 内部已销毁对象
                return true;
            }
            return true;
            });
    }

    ui::Button* pCalRange = dynamic_cast<ui::Button*>(FindControl(_T("calendar_range")));
    if (pCalRange != nullptr) {
        pCalRange->AttachClick([this](const ui::EventArgs& args) {
            ui::Control* pAnchor = args.GetSender();
            if (pAnchor == nullptr) {
                return true;
            }
            //同一锚点再次点击：切换为关闭已打开的浮层
            ui::Flyout* pActive = ui::Flyout::GetActiveFlyout();
            if ((pActive != nullptr) && (pActive->GetAnchor() == pAnchor)) {
                pActive->Dismiss();
                return true;
            }
            ui::CalendarFlyout* pFlyout = new ui::CalendarFlyout(this);
            pFlyout->SetMode(1); //范围
            pFlyout->AttachDateSelectedEx([this](WPARAM wParam, LPARAM lParam, const ui::Calendar::DateRange* pRange) {
                if (wParam == 1) {
                    //范围模式：起止值统一从 pRange 读取（完整 64 位），lParam 无意义
                    if (pRange == nullptr) {
                        return;
                    }
                    time_t tStart = pRange->start;
                    time_t tEnd = pRange->end;
                    struct tm start = ui::Calendar::TimeTToDate(tStart);
                    struct tm end = ui::Calendar::TimeTToDate(tEnd);
                    DString text = ui::Calendar::FormatDateString(start) + _T(" ~ ") + ui::Calendar::FormatDateString(end);
                    //text 是格式化后的日期字符串，不是多语言ID，bTextId 必须为 false
                    ui::ToastWnd::Show(this, text, ui::ToastWnd::kTypeInfo, 2000, ui::ToastWnd::kPosTop, false);
                }
            });
            struct tm today = ui::Calendar::GetToday();
            if (!pFlyout->ShowAt(pAnchor, today, ui::Flyout::Placement::Bottom)) {
                //创建失败时 ShowAt 内部已销毁对象
                return true;
            }
            return true;
            });
    }
}

void TestForm::OnTimer()
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("progress_text")));
    std::vector<DString> controlList = {_T("progress11"), _T("progress12"), _T("progress13"), _T("progress14"),
                                        _T("progress21"), _T("progress22"), _T("progress23"), _T("progress24") };
    for (const DString& name : controlList) {
        ui::Progress* pProgress = dynamic_cast<ui::Progress*>(FindControl(name));
        if (pProgress != nullptr) {
            if (pLabel != nullptr) {
                pLabel->SetText(ui::StringUtil::Printf(_T("%d%%"), (int32_t)m_nProgressValue));                
            }
            pProgress->SetValue(m_nProgressValue);
        }
    }

    m_nProgressValue += 0.4;
    if (m_nProgressValue > 100.0) {
        m_nProgressValue = 0.0;
    }
}

void TestForm::ShowFlyoutDemo(ui::Control* pAnchor, ui::Flyout::Placement placement)
{
    if (pAnchor == nullptr) {
        return;
    }

    //同一锚点再次点击：切换为关闭已打开的浮层
    ui::Flyout* pActive = ui::Flyout::GetActiveFlyout();
    if ((pActive != nullptr) && (pActive->GetAnchor() == pAnchor)) {
        pActive->Dismiss();
        return;
    }

    //非模态浮层，关闭后框架自动 delete
    //TestForm 的资源路径已是 "controls"，XML 文件名直接相对该路径
    ui::Flyout* pFlyout = new ui::Flyout(this);
    pFlyout->SetSkinFolder(GetResourcePath().ToString());
    if (!pFlyout->ShowAt(pAnchor, _T("flyout_demo.xml"), placement)) {
        //创建失败时 ShowAt 内部已销毁对象
        return;
    }

    //浮层内“主要操作”：关闭浮层并弹出成功 Toast
    ui::Button* pAction1 = dynamic_cast<ui::Button*>(pFlyout->FindControl(_T("flyout_action1")));
    if (pAction1 != nullptr) {
        pAction1->AttachClick([this, pFlyout](const ui::EventArgs& /*args*/) {
            pFlyout->Dismiss();
            ui::ToastWnd::Show(this, _T("STRID_FLYOUT_TOAST_ACTION1"), ui::ToastWnd::kTypeSuccess,
                               2000, ui::ToastWnd::kPosTop, true);
            return true;
            });
    }

    //浮层内“次要操作”：关闭浮层并弹出信息 Toast
    ui::Button* pAction2 = dynamic_cast<ui::Button*>(pFlyout->FindControl(_T("flyout_action2")));
    if (pAction2 != nullptr) {
        pAction2->AttachClick([this, pFlyout](const ui::EventArgs& /*args*/) {
            pFlyout->Dismiss();
            ui::ToastWnd::Show(this, _T("STRID_FLYOUT_TOAST_ACTION2"), ui::ToastWnd::kTypeInfo,
                               2000, ui::ToastWnd::kPosTop, true);
            return true;
            });
    }

    //浮层内“关闭”按钮
    ui::Button* pClose = dynamic_cast<ui::Button*>(pFlyout->FindControl(_T("flyout_close")));
    if (pClose != nullptr) {
        pClose->AttachClick([pFlyout](const ui::EventArgs& /*args*/) {
            pFlyout->Dismiss();
            return true;
            });
    }
}

