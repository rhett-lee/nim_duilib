#include "TestForm.h"
#include "MessageBoxForm.h"
#include "ToastForm.h"
#include "ProgressForm.h"
#include "TabCtrlForm.h"
#include "RangeSliderForm.h"
#include "duilib/Control/SearchBox.h"
#include <ctime>

TestForm::TestForm()
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
    //显示进度条测试窗口（独立演示窗口）
    ui::Button* pProgressBtn = dynamic_cast<ui::Button*>(FindControl(_T("progress_btn")));
    if (pProgressBtn != nullptr) {
        pProgressBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ShowProgressWindow();
            return true;
            });
    }

    //标签栏测试窗口（垂直标签 / 固定标签 / 溢出滚动 / 程序化增删标签）
    ui::Button* pTabCtrlBtn = dynamic_cast<ui::Button*>(FindControl(_T("tab_ctrl_test_btn")));
    if (pTabCtrlBtn != nullptr) {
        pTabCtrlBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ShowTabCtrlWindow();
            return true;
            });
    }

    //范围滑块测试窗口（双滑块范围选择控件）
    ui::Button* pRangeSliderBtn = dynamic_cast<ui::Button*>(FindControl(_T("rangeslider_btn")));
    if (pRangeSliderBtn != nullptr) {
        pRangeSliderBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ShowRangeSliderWindow();
            return true;
            });
    }

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

    //TagInput 标签输入框：按钮演示程序化添加标签（默认不允许重复标签，递增序号保证唯一）
    ui::TagInput* pTagInput = dynamic_cast<ui::TagInput*>(FindControl(_T("tag_input")));
    ui::Button* pAddTagBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_add_tag")));
    if ((pTagInput != nullptr) && (pAddTagBtn != nullptr)) {
        pAddTagBtn->AttachClick([pTagInput](const ui::EventArgs&) {
            static int32_t s_nTagIndex = 1;
            while (s_nTagIndex < 10000) {
                DString tag = ui::StringUtil::Printf(_T("Tag%d"), s_nTagIndex);
                ++s_nTagIndex;
                if (pTagInput->AddTag(tag)) {
                    break;
                }
            }
            pTagInput->SetFocus();
            return true;
            });
    }

    //AutoComplete 自动完成输入框：选中候选项后用 Toast 提示
    ui::AutoComplete* pAutoComplete = dynamic_cast<ui::AutoComplete*>(FindControl(_T("auto_complete")));
    if (pAutoComplete != nullptr) {
        //数据源已通过 XML 的 items 属性配置（前缀匹配），这里仅绑定选中事件
        pAutoComplete->AttachSelect([this, pAutoComplete](const ui::EventArgs& /*args*/) {
            DString text = pAutoComplete->GetText();
            ui::ToastWnd::Show(this, ui::StringUtil::Printf(_T("Selected: %s"), text.c_str()),
                               ui::ToastWnd::kTypeInfo, 1500, ui::ToastWnd::kPosTop, false);
            return true;
            });
    }

    //第二个控件：代码填充数据源，match_mode="contains" 在 XML 中配置（包含匹配）
    ui::AutoComplete* pAutoComplete2 = dynamic_cast<ui::AutoComplete*>(FindControl(_T("auto_complete2")));
    if (pAutoComplete2 != nullptr) {
        pAutoComplete2->SetCandidates({
            _T("https://github.com"),
            _T("https://github.com/nimdemo/nim_duilib"),
            _T("https://gitee.com"),
            _T("https://www.baidu.com"),
            _T("https://www.google.com"),
            _T("https://duilib.com"),
            _T("https://learn.microsoft.com"),
            _T("https://stackoverflow.com"),
            _T("https://en.cppreference.com")
            });
        pAutoComplete2->AttachSelect([this, pAutoComplete2](const ui::EventArgs& /*args*/) {
            DString text = pAutoComplete2->GetText();
            ui::ToastWnd::Show(this, ui::StringUtil::Printf(_T("Selected: %s"), text.c_str()),
                               ui::ToastWnd::kTypeInfo, 1500, ui::ToastWnd::kPosTop, false);
            return true;
            });
    }

    //SearchBox 搜索框：选中建议或回车搜索后用 Toast 提示（并演示搜索历史记录）
    ui::SearchBox* pSearchBox = dynamic_cast<ui::SearchBox*>(FindControl(_T("search_box")));
    if (pSearchBox != nullptr) {
        //搜索场景防抖：连续输入仅在停顿 300ms 后才触发 kEventTextChanged
        pSearchBox->SetDebounceInterval(300);
        pSearchBox->AttachSelect([this, pSearchBox](const ui::EventArgs& /*args*/) {
            ui::ToastWnd::Show(this, ui::StringUtil::Printf(_T("Selected: %s"), pSearchBox->GetSearchText().c_str()),
                               ui::ToastWnd::kTypeInfo, 1500, ui::ToastWnd::kPosTop, false);
            return true;
            });
        pSearchBox->AttachReturn([this, pSearchBox](const ui::EventArgs& /*args*/) {
            ui::ToastWnd::Show(this, ui::StringUtil::Printf(_T("Search: %s  (history=%d)"),
                               pSearchBox->GetSearchText().c_str(), static_cast<int>(pSearchBox->GetHistory().size())),
                               ui::ToastWnd::kTypeInfo, 1500, ui::ToastWnd::kPosTop, false);
            return true;
            });
    }

    //上下文菜单：一行代码为任意控件绑定右键菜单(Control::AttachPopupMenu)
    ui::Button* pContextMenuBtn = dynamic_cast<ui::Button*>(FindControl(_T("context_menu_btn")));
    if (pContextMenuBtn != nullptr) {
        pContextMenuBtn->AttachPopupMenu(_T("menu/settings_menu.xml"),
            [this](const DString& itemName, size_t /*nItemIndex*/) {
                ui::ToastWnd::Show(this, ui::StringUtil::Printf(_T("Context menu item: %s"), itemName.c_str()),
                                   ui::ToastWnd::kTypeInfo, 1500, ui::ToastWnd::kPosTop, false);
            });
    }

    //遮罩/蒙版层（ui::Mask）：演示三种典型场景
    //(1) 模态遮罩：覆盖整个窗口并拦截鼠标输入，点击卡片上的按钮关闭
    ui::Control* pMaskModal = FindControl(_T("mask_modal"));
    ui::Button* pMaskModalBtn = dynamic_cast<ui::Button*>(FindControl(_T("mask_modal_btn")));
    if ((pMaskModal != nullptr) && (pMaskModalBtn != nullptr)) {
        pMaskModalBtn->AttachClick([pMaskModal](const ui::EventArgs& /*args*/) {
            pMaskModal->SetVisible(true);
            return true;
            });
    }
    ui::Button* pMaskModalClose = dynamic_cast<ui::Button*>(FindControl(_T("mask_modal_close")));
    if ((pMaskModal != nullptr) && (pMaskModalClose != nullptr)) {
        pMaskModalClose->AttachClick([pMaskModal](const ui::EventArgs& /*args*/) {
            pMaskModal->SetVisible(false);
            return true;
            });
    }

    //(2) 新手引导蒙层：镂空高亮目标按钮，镂空区域内点击可穿透到目标按钮
    ui::Control* pMaskGuide = FindControl(_T("mask_guide"));
    ui::Button* pMaskGuideBtn = dynamic_cast<ui::Button*>(FindControl(_T("mask_guide_btn")));
    if ((pMaskGuide != nullptr) && (pMaskGuideBtn != nullptr)) {
        pMaskGuideBtn->AttachClick([pMaskGuide](const ui::EventArgs& /*args*/) {
            pMaskGuide->SetVisible(true);
            return true;
            });
    }
    //镂空区域内点击穿透到目标按钮：完成引导并关闭蒙层
    ui::Button* pMaskGuideTarget = dynamic_cast<ui::Button*>(FindControl(_T("mask_guide_target")));
    if ((pMaskGuide != nullptr) && (pMaskGuideTarget != nullptr)) {
        pMaskGuideTarget->AttachClick([this, pMaskGuide](const ui::EventArgs& /*args*/) {
            if (pMaskGuide->IsVisible()) {
                //引导中：点击镂空区域内的目标按钮，完成引导
                pMaskGuide->SetVisible(false);
                ui::ToastWnd::Show(this, _T("STRID_CONTROLS_MASK_GUIDE_DONE"), ui::ToastWnd::kTypeSuccess,
                                   1500, ui::ToastWnd::kPosTop, true);
            }
            else {
                //未处于引导状态：提示先点击"新手引导"按钮
                ui::ToastWnd::Show(this, _T("STRID_CONTROLS_MASK_GUIDE_TIP"), ui::ToastWnd::kTypeInfo,
                                   1500, ui::ToastWnd::kPosTop, true);
            }
            return true;
            });
    }
    //点击镂空区域之外的蒙层区域：关闭引导
    if (pMaskGuide != nullptr) {
        pMaskGuide->AttachButtonUp([pMaskGuide](const ui::EventArgs& /*args*/) {
            pMaskGuide->SetVisible(false);
            return true;
            });
    }

    //(3) 加载遮罩：显示后拦截交互，2 秒后由定时器自动关闭（模拟异步加载完成）
    ui::Control* pMaskLoading = FindControl(_T("mask_loading"));
    ui::Button* pMaskLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("mask_loading_btn")));
    if ((pMaskLoading != nullptr) && (pMaskLoadingBtn != nullptr)) {
        pMaskLoadingBtn->AttachClick([this, pMaskLoading](const ui::EventArgs& /*args*/) {
            pMaskLoading->SetVisible(true);
            ui::GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), [pMaskLoading]() {
                pMaskLoading->SetVisible(false);
                }, 2000, 1);
            return true;
            });
    }
}

void TestForm::ShowProgressWindow()
{
    ProgressForm* testForm = new ProgressForm();
    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("ProgressWindow");
    createParam.m_bCenterWindow = true;
    testForm->CreateWnd(this, createParam);
    testForm->ShowModalFake();
}

void TestForm::ShowTabCtrlWindow()
{
    TabCtrlForm* testForm = new TabCtrlForm();
    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("TabCtrlForm");
    createParam.m_bCenterWindow = true;
    testForm->CreateWnd(this, createParam);
    testForm->ShowModalFake();
}

void TestForm::ShowRangeSliderWindow()
{
    RangeSliderForm* testForm = new RangeSliderForm();
    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("RangeSliderWindow");
    createParam.m_bCenterWindow = true;
    testForm->CreateWnd(this, createParam);
    testForm->ShowModalFake();
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

