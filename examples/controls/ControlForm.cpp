#include "ControlForm.h"
#include "AboutForm.h"
#include "TestForm.h"
#include "AnimationForm.h"
#include "NavigationForm.h"
#include "ChartForm.h"
#include "MarqueeForm.h"
#include "TimelineForm.h"
#include "StatusBarForm.h"
#include "ToolBarForm.h"
#include "ProgressForm.h"
#include "TabCtrlForm.h"
#include "RangeSliderForm.h"
#include "StepperForm.h"
#include "RatingForm.h"
#include "CarouselForm.h"
#include "TreeGridForm.h"
#include "BreadcrumbForm.h"
#include "CascadingSelectForm.h"
#include "PopoverForm.h"
#include "PopconfirmForm.h"
#include "AvatarForm.h"
#include "SwitchForm.h"
#include "ButtonCheckBoxForm.h"
#include "MessageBoxForm.h"
#include "ToastForm.h"
#include "ComboForm.h"
#include "ListViewForm.h"
#include "RichEditForm.h"
#include "DateTimeForm.h"

#include <functional>

//系统全局热键的ID
#define SYSTEM_HOTKEY_ID 111

ControlForm::ControlForm()
{
}


ControlForm::~ControlForm()
{
}

DString ControlForm::GetSkinFolder()
{
    return _T("controls");
}

DString ControlForm::GetSkinFile()
{
    return _T("controls.xml");
}

void ControlForm::OnInitWindow()
{
#ifdef DUILIB_BUILD_FOR_SDL
    //显示SDL的基本信息
    ui::Label* pTitle = dynamic_cast<ui::Label*>(FindControl(_T("window_title")));
    if (pTitle != nullptr) {
        DString title = pTitle->GetText();
        DString driverName = GetVideoDriverName();
        DString renderName = GetWindowRenderName();
        DString newTitle = ui::StringUtil::Printf(_T("%s[SDL: VideoDriver:\"%s\", RenderName:\"%s\"]"), title.c_str(), driverName.c_str(), renderName.c_str());
        pTitle->SetText(newTitle);
    }
#endif

    /* Initialize ListBox data */
    ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("list")));
    if (list != nullptr) {
        for (auto i = 0; i < 3; i++)
        {
            ui::ListBoxItem* element = new ui::ListBoxItem(this);
            element->SetText(ui::StringUtil::Printf(_T("ui::VListBox::ListBoxItem %d"), i));
            element->SetClass(_T("list_item"));
            element->SetFixedHeight(ui::UiFixedInt(22), true, true);
            list->AddItem(element);
        }
    }

    ui::TreeView* pTree = dynamic_cast<ui::TreeView*>(FindControl(_T("tree")));
    if (pTree != nullptr) {
        ui::TreeNode* pRootNode = pTree->GetRootNode();
        ASSERT(pRootNode != nullptr);
        if (pRootNode != nullptr) {
            ui::TreeNode* pTestNode = pRootNode->FindChildNodeByText(_T("TreeView Node 2"), true);
            if (pTestNode != nullptr) {
                ui::TreeNode* pNode0 = new ui::TreeNode(this);
                pNode0->SetClass(_T("tree_node"));
                pNode0->SetText(_T("Dynamic Node 0(top)"));
                pTestNode->AddChildNodeAt(pNode0, 0);

                ui::TreeNode* pNode2 = new ui::TreeNode(this);
                pNode2->SetClass(_T("tree_node"));
                pNode2->SetText(_T("Dynamic Node 1(end)"));
                pTestNode->AddChildNode(pNode2);
            }
        }
    }

    //初始化Combo的数据
    ui::Combo* combo = dynamic_cast<ui::Combo*>(FindControl(_T("combo")));
    if (combo != nullptr) {
        ui::TreeView* pTreeView = combo->GetTreeView();
        ui::TreeNode* pTreeNode = pTreeView->GetRootNode();
        for (auto i = 0; i < 10; i++) {
            ui::TreeNode* node = new ui::TreeNode(this);
            node->SetClass(_T("tree_node"));
            node->SetText(ui::StringUtil::Printf(_T("ui::Combo::TreeNode %d"), i));
            pTreeNode->AddChildNode(node);
        }
    }

    ui::FilterCombo* filterCombo = dynamic_cast<ui::FilterCombo*>(FindControl(_T("filter_combo")));
    if (filterCombo != nullptr) {
        for (auto i = 0; i < 100; i++) {
            filterCombo->AddTextItem(ui::StringUtil::Printf(_T("Item %d FilterCombo"), i));
        }
    }

    ui::CheckCombo* check_combo = dynamic_cast<ui::CheckCombo*>(FindControl(_T("check_combo")));
    if (check_combo != nullptr) {
        // 星期下拉框选项 - 替换为语言ID
        check_combo->AddTextIdItem(_T("STRID_CONTROLS_MONDAY"));    // 星期一
        check_combo->AddTextIdItem(_T("STRID_CONTROLS_TUESDAY"));   // 星期二
        check_combo->AddTextIdItem(_T("STRID_CONTROLS_WEDNESDAY")); // 星期三
        check_combo->AddTextIdItem(_T("STRID_CONTROLS_THURSDAY"));  // 星期四
        check_combo->AddTextIdItem(_T("STRID_CONTROLS_FRIDAY"));    // 星期五
        check_combo->AddTextIdItem(_T("STRID_CONTROLS_SATURDAY"));  // 星期六
        check_combo->AddTextIdItem(_T("STRID_CONTROLS_SUNDAY"));    // 星期日
    }

    /* Post repeat task to update progress value 300 milliseconds once */
    /* Using ToWeakCallback to protect closure when if [ControlForm] was destoryed */
    ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::ThreadIdentifier::kThreadUI,
        ui::UiBind(this, [this]() {
            float fProgress = (float)(std::time(nullptr) % 100);
            ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&ControlForm::OnProgressValueChagned, this, fProgress));
            }),
        300);

    /* Show settings menu */
    ui::Button* settings = dynamic_cast<ui::Button*>(FindControl(_T("settings")));
    if (settings != nullptr) {
        settings->AttachClick([this, settings](const ui::EventArgs& args) {
            ui::UiRect rect = args.GetSender()->GetPos();
            ui::UiPoint point;
            point.x = rect.left;
            point.y = rect.bottom;
            ClientToScreen(point);

            //显示菜单，并保持settings按钮处于Push状态
            ShowPopupMenu(point, settings);
            return true;
            });
    }

    //注册编辑框的拖放事件（演示文件拖入）
    AttachRichEditEvents(FindControl(_T("gallery_edit_color")));

    //RichText显示超级链接
    ui::RichText* pRichText = dynamic_cast<ui::RichText*>(FindControl(_T("rich_text_demo")));
    if (pRichText != nullptr) {
        pRichText->AttachLinkClick([this](const ui::EventArgs& args) {
            const DString::value_type* url = (const DString::value_type*)args.wParam;
            if (url != nullptr) {
                ui::SystemUtil::ShowMessageBox(this, url, _T("RichText"));
            }
            return true;
            });
    }

    //HyperLink控件
    ui::HyperLink* pHyperLink = dynamic_cast<ui::HyperLink*>(FindControl(_T("hyper_link1")));
    if (pHyperLink != nullptr) {
        pHyperLink->AttachLinkClick([this](const ui::EventArgs& args) {
            const DString::value_type* url = (const DString::value_type*)args.wParam;
            if (url != nullptr) {
                ui::SystemUtil::ShowMessageBox(this, url, _T("HyperLink"));
            }
            return true;
            });
    }

    pHyperLink = dynamic_cast<ui::HyperLink*>(FindControl(_T("hyper_link2")));
    if (pHyperLink != nullptr) {
        pHyperLink->AttachLinkClick([this](const ui::EventArgs& /*args*/) {
            ui::SystemUtil::ShowMessageBox(this, _T("Text Button LinkClick Event!"), _T("HyperLink"));
            return true;
            });
    }

    //窗口热键设置
    ui::HotKey* pHotKey = dynamic_cast<ui::HotKey*>(FindControl(_T("set_hot_key")));
    ui::Button* pHotKeyButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_set_hot_key")));
    if (pHotKey && pHotKeyButton) {
        pHotKeyButton->AttachClick([this, pHotKey](const ui::EventArgs& args) {
            uint8_t wVirtualKeyCode = 0;
            uint8_t wModifiers = 0;
            pHotKey->GetHotKey(wVirtualKeyCode, wModifiers);
            if (wVirtualKeyCode != 0) {
                //设置为激活窗口的热键
                int32_t nRet = SetWindowHotKey(wVirtualKeyCode, wModifiers);
                ASSERT(nRet == 1);
            }
            return true;
            });
    }

    //二级页面入口（统一绑定）
    auto bindOpen = [this](const DString& name, const std::function<ui::WindowImplBase*()>& creator) {
        ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(name));
        if (pButton != nullptr) {
            pButton->AttachClick([this, creator](const ui::EventArgs&) {
                ShowChildForm(creator());
                return true;
                });
        }
    };

    //基础控件
    bindOpen(_T("btn_gallery_button"), []() { return new ButtonCheckBoxForm(); });
    bindOpen(_T("btn_gallery_check"), []() { return new ButtonCheckBoxForm(); });
    bindOpen(_T("btn_gallery_badge"), []() { return new ButtonCheckBoxForm(); });
    bindOpen(_T("btn_gallery_switch"), []() { return new SwitchForm(); });
    bindOpen(_T("btn_gallery_elements"), []() { return new TestForm(); });

    //输入控件
    bindOpen(_T("btn_gallery_richedit"), []() { return new RichEditForm(); });
    bindOpen(_T("btn_gallery_combo"), []() { return new ComboForm(); });
    bindOpen(_T("btn_gallery_datetime"), []() { return new DateTimeForm(); });
    bindOpen(_T("btn_gallery_interaction"), []() { return new TestForm(); });
    bindOpen(_T("btn_gallery_io"), []() { return new TestForm(); });

    //数据展示
    bindOpen(_T("btn_gallery_progress"), []() { return new ProgressForm(); });
    bindOpen(_T("btn_gallery_list"), []() { return new ListViewForm(); });
    bindOpen(_T("btn_gallery_chart"), []() { return new ChartForm(); });
    bindOpen(_T("btn_gallery_tree_grid"), []() { return new TreeGridForm(); });
    bindOpen(_T("btn_gallery_rating"), []() { return new RatingForm(); });
    bindOpen(_T("btn_gallery_stepper"), []() { return new StepperForm(); });
    bindOpen(_T("btn_gallery_avatar"), []() { return new AvatarForm(); });
    bindOpen(_T("btn_gallery_carousel"), []() { return new CarouselForm(); });

    //反馈与浮层
    bindOpen(_T("message_box_btn"), []() { return new MessageBoxForm(); });
    bindOpen(_T("toast_btn"), []() { return new ToastForm(); });
    bindOpen(_T("popover_btn"), []() { return new PopoverForm(); });
    bindOpen(_T("popconfirm_btn"), []() { return new PopconfirmForm(); });
    bindOpen(_T("btn_gallery_flyout"), []() { return new TestForm(); });

    //导航与布局
    bindOpen(_T("btn_gallery_tab_ctrl"), []() { return new TabCtrlForm(); });
    bindOpen(_T("btn_gallery_rangeslider"), []() { return new RangeSliderForm(); });
    bindOpen(_T("navigation_btn"), []() { return new NavigationForm(); });
    bindOpen(_T("breadcrumb_btn"), []() { return new BreadcrumbForm(); });
    bindOpen(_T("cascading_select_btn"), []() { return new CascadingSelectForm(); });
    bindOpen(_T("timeline_btn"), []() { return new TimelineForm(); });
    bindOpen(_T("marquee_btn"), []() { return new MarqueeForm(); });
    bindOpen(_T("animation_btn"), []() { return new AnimationForm(); });
    bindOpen(_T("statusbar_btn"), []() { return new StatusBarForm(); });
    bindOpen(_T("toolbar_btn"), []() { return new ToolBarForm(); });

    //系统原生通知（SystemNotification）：与 Toast 无关，走操作系统通知通道
    ui::Button* pSystemNotificationBtn = dynamic_cast<ui::Button*>(FindControl(_T("system_notification_btn")));
    if (pSystemNotificationBtn != nullptr) {
        pSystemNotificationBtn->AttachClick([this](const ui::EventArgs&) {
            if (!ui::SystemNotification::IsSupported()) {
                ui::ToastWnd::Show(this, _T("System notification is not supported on this platform"),
                                   ui::ToastWnd::kTypeWarning, 2000, ui::ToastWnd::kPosTop, false);
                return true;
            }
            ui::SystemNotification::RequestPermission();
            uint32_t notificationId = ui::SystemNotification::Show(_T("nim_duilib"),
                _T("This is a native system notification."),
                ui::SystemNotificationPriority::kNormal, 5000);
            if (notificationId == 0) {
                ui::ToastWnd::Show(this, _T("Failed to show system notification"),
                                   ui::ToastWnd::kTypeWarning, 2000, ui::ToastWnd::kPosTop, false);
            }
            return true;
            });
    }

    //显示模态对话框的拾色器
    ui::Button* pShowColorPicker = dynamic_cast<ui::Button*>(FindControl(_T("show_color_picker")));
    if (pShowColorPicker != nullptr) {
        pShowColorPicker->AttachClick([this](const ui::EventArgs&) {
            ShowColorPicker(true);
            return true;
            });
    }

    //显示模态对话框
    ui::Button* pShowModal = dynamic_cast<ui::Button*>(FindControl(_T("domodal2")));
    if (pShowModal != nullptr) {
        pShowModal->AttachClick([this](const ui::EventArgs&) {
            ShowDoModalDlg();
            return true;
            });
    }

    //托盘图标功能
    ui::CheckBox* pTrayIconCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_tray_icon")));
    if (pTrayIconCheckBox != nullptr) {
        pTrayIconCheckBox->AttachSelect([this](const ui::EventArgs&) {
            //开启
            ShowTrayIcon(true);
            return true;
            });
        pTrayIconCheckBox->AttachUnSelect([this](const ui::EventArgs&) {
            //关闭
            ShowTrayIcon(false);
            return true;
            });
    }
}

void ControlForm::OnCloseWindow()
{
    //关闭TrayIcon
    ShowTrayIcon(false);

    BaseClass::OnCloseWindow();
}

void ControlForm::ShowChildForm(ui::WindowImplBase* pForm)
{
    if (pForm == nullptr) {
        return;
    }
    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("ControlsChildWindow");
    createParam.m_bCenterWindow = true;
    pForm->CreateWnd(this, createParam);
    pForm->ShowModalFake();
}

void ControlForm::AttachRichEditEvents(ui::Control* edit)
{
    if (edit == nullptr) {
        return;
    }
    //文件拖入支持
    edit->AttachDropData([this, edit](const ui::EventArgs& args) {
        ui::FilePath filePath;
        if (args.wParam == ui::kControlDropTypeWindows) {
            const ui::ControlDropData_Windows* dropData = (const ui::ControlDropData_Windows*)args.lParam;
            if ((dropData != nullptr) && !dropData->m_fileList.empty()) {
                filePath = dropData->m_fileList[0];
            }
        }
        else if (args.wParam == ui::kControlDropTypeSDL) {
            const ui::ControlDropData_SDL* dropData = (const ui::ControlDropData_SDL*)args.lParam;
            if ((dropData != nullptr) && !dropData->m_fileList.empty()) {
                filePath = dropData->m_fileList[0];
            }
        }
        if (!filePath.IsEmpty()) {
            ui::FileUtil fileUtil;
            std::vector<uint8_t> fileData;
            fileUtil.ReadFileData(filePath, fileData);
            if (!fileData.empty()) {
                std::wstring result;
                if (ui::StringCharset::GetDataAsString((const char*)fileData.data(), (uint32_t)fileData.size(), result)) {
                    DString text = ui::StringConvert::WStringToT(result);
                    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(edit);
                    if (pRichEdit != nullptr) {
                        pRichEdit->SetText(text);
                    }
                    else {
                        ui::RichEdit2* pRichEdit2 = dynamic_cast<ui::RichEdit2*>(edit);
                        if (pRichEdit2 != nullptr) {
                            pRichEdit2->SetText(text);
                        }
                    }
                }
            }
        }
        return true;
        });
}

void ControlForm::ShowColorPicker(bool bDoModal)
{
    bool bModalDlg = bDoModal; //是否使用模态对话框
    ui::ColorPicker colorDlg;
    ui::ColorPicker* pColorPicker = nullptr;
    if (!bModalDlg) {
        pColorPicker = new ui::ColorPicker;
    }
    else {
        pColorPicker = &colorDlg;
    }

    //窗口创建完成执行的函数
    auto OnInitColorPicker = [this, pColorPicker](const ui::EventArgs&) {
        ui::RichEdit* pEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("gallery_edit_color")));
        if (pEdit != nullptr) {
            DString oldTextColor = pEdit->GetTextColor();
            if (!oldTextColor.empty()) {
                pColorPicker->SetSelectedColor(pEdit->GetUiColor(oldTextColor));
            }
            //如果在界面选择颜色，则临时更新RichEdit控件文本的颜色
            pColorPicker->AttachSelectColor([this, pEdit](const ui::EventArgs& args) {
                ui::UiColor newColor = ui::UiColor((uint32_t)args.wParam);
                pEdit->SetTextColor(pEdit->GetColorString(newColor));
                return true;
                });

            //窗口关闭事件
            pColorPicker->AttachWindowClose([this, pColorPicker, pEdit, oldTextColor](const ui::EventArgs& args) {
                ui::UiColor newColor = pColorPicker->GetSelectedColor();
                if ((args.wParam == ui::kWindowCloseOK) && !newColor.IsEmpty()) {
                    //如果是"确认"，则设置RichEdit控件的文本颜色
                    pEdit->SetTextColor(pEdit->GetColorString(newColor));
                }
                else {
                    //如果是"取消"或者关闭窗口，则恢复原来的颜色
                    pEdit->SetTextColor(oldTextColor);
                }
                return true;
                });

        }
        return true;
        };
    pColorPicker->AttachWindowCreateMsg(OnInitColorPicker);

    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_bCenterWindow = true;
    if (!bModalDlg) {
        pColorPicker->CreateWnd(this, createParam);
        pColorPicker->ShowModalFake();
    }
    else {
        pColorPicker->DoModal(this, createParam);
    }
}

void ControlForm::ShowDoModalDlg()
{
    ui::WindowImplBase simpleWnd;
    simpleWnd.InitSkin(_T("controls"), _T("about.xml"));
    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("AboutForm");
    createParam.m_bCenterWindow = true;
    simpleWnd.DoModal(this, createParam);
}

void ControlForm::ShowPopupMenu(const ui::UiPoint& point, ui::Control* pRelatedControl)
{
    ui::Menu* menu = new ui::Menu(this, pRelatedControl);//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    menu->SetSkinFolder(GetResourcePath().ToString());
    DString xml(_T("menu/settings_menu.xml"));
    menu->ShowMenu(xml, point);

    //在二级菜单中，添加子菜单项
    ui::MenuItem* menu_fourth = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("fourth")));
    if (menu_fourth != nullptr) {
        ui::MenuItem* menu_item = new ui::MenuItem(menu);
        menu_item->SetText(_T("Dynamically created"));
        menu_item->SetClass(_T("menu_element"));
        menu_item->SetFixedWidth(ui::UiFixedInt(180), true, true);
        menu_item->SetFontId(_T("system_14"));
        menu_item->SetTextPadding({ 20, 0, 20, 0 }, true);
        menu_fourth->AddSubMenuItemAt(menu_item, 1);//添加后，资源由菜单统一管理
    }

    //CheckBox菜单项的功能演示
    static bool s_is_checked_01_flag = false;
    bool& flag = s_is_checked_01_flag;
    ui::MenuItem* menu_check_01 = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("menu_check_01")));
    if (menu_check_01 != nullptr) {
        menu_check_01->AttachClick([&flag](const ui::EventArgs& args) {
            flag = true;
            return true;
            });
    }
    ui::CheckBox* menuCheckBox01 = dynamic_cast<ui::CheckBox*>(menu->FindControl(_T("menu_checkbox_01")));
    if (menuCheckBox01 != nullptr) {
        menuCheckBox01->Selected(s_is_checked_01_flag);
    }

    ui::MenuItem* menu_check_02 = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("menu_check_02")));
    if (menu_check_02 != nullptr) {
        menu_check_02->AttachClick([&flag](const ui::EventArgs& args) {
            flag = false;
            return true;
            });
    }

    ui::CheckBox* menuCheckBox02 = dynamic_cast<ui::CheckBox*>(menu->FindControl(_T("menu_checkbox_02")));
    if (menuCheckBox02 != nullptr) {
        menuCheckBox02->Selected(!s_is_checked_01_flag);
    }

    /* About menu */
    ui::MenuItem* menu_about = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("about")));
    if (menu_about != nullptr) {
        menu_about->AttachClick([this](const ui::EventArgs& args) {
            ShowChildForm(new AboutForm());
            return true;
            });
    }
}

void ControlForm::OnProgressValueChagned(float value)
{
    //回调给的进度范围是：[0, 99), 转换为[0, 100]
    value = value * 100 / 99 + 0.5f;
    auto progress = dynamic_cast<ui::Progress*>(FindControl(_T("progress")));
    if (progress) {
        progress->SetValue(value);
    }

    auto circleprogress = dynamic_cast<ui::Progress*>(FindControl(_T("circleprogress")));
    if (circleprogress) {
        circleprogress->SetValue(value);
        circleprogress->SetText(ui::StringUtil::Printf(_T("%.0f%%"), value));
    }
}

LRESULT ControlForm::OnHotKeyMsg(int32_t hotkeyId, ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnHotKeyMsg(hotkeyId, vkCode, modifierKey, nativeMsg, bHandled);
    bHandled = true;
    if (hotkeyId == SYSTEM_HOTKEY_ID) {
        SetWindowForeground();
        ui::SystemUtil::ShowMessageBox(this, _T("Received system hotkey command"), _T("ControlForm::OnHotKeyMsg"));
    }
    return lResult;
}

void ControlForm::ShowTrayIcon(bool bShow)
{
    if (bShow) {
        // 创建托盘图标
        m_pTrayIcon = ui::TrayIcon::Create(this, _T("public/caption/logo.ico"), _T("controls(nim_duilib)"));
        if (m_pTrayIcon != nullptr) {
            // 设置消息回调
            m_pTrayIcon->SetMessageCallback([this](ui::TrayIconMessageType msgType, int32_t x, int32_t y)
                {
                    OnTrayIconMessage(msgType, x, y);
                });

            // 显示欢迎气泡
            m_pTrayIcon->ShowBalloon(
                ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CONTROLS_TRAY_BALLOON_TITLE")),
                ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CONTROLS_TRAY_BALLOON_TEXT")),
                3000);
        }

    }
    else {
        m_pTrayIcon.reset();
    }
}

void ControlForm::OnTrayIconMessage(ui::TrayIconMessageType msgType, int32_t x, int32_t y)
{
    switch (msgType)
    {
    case ui::TrayIconMessageType::kLeftClick:
        // 左键单击：显示/隐藏窗口
        if (IsWindowVisible()) {
            ShowWindow(ui::ShowWindowCommands::kSW_HIDE);
        }
        else {
            ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);
            SetWindowForeground();
        }
        break;

    case ui::TrayIconMessageType::kLeftDoubleClick:
        // 左键双击：显示窗口
        ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);
        SetWindowForeground();
        break;

    case ui::TrayIconMessageType::kRightClick:
        // 右键单击：显示右键菜单
        ShowTrayMenu(x, y);
        break;
    case ui::TrayIconMessageType::kShowBalloon:
        // 气泡被点击
        break;
    default:
        break;
    }
}

void ControlForm::ShowTrayMenu(int32_t x, int32_t y)
{
    ui::Window* pParentWnd = IsWindowVisible() ? this : nullptr;//窗口隐藏时，不设置父窗口，避免菜单不显示
    ui::Menu* menu = new ui::Menu(pParentWnd, nullptr);
    menu->SetSkinFolder(GetResourcePath().ToString());
    DString xml(_T("menu/tray_menu.xml"));
    menu->ShowMenu(xml, ui::UiPoint(x, y));

    //菜单项点击响应
    ui::MenuItem* pMenuItem = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("tray_menu_item_1")));
    if (pMenuItem != nullptr) {
        pMenuItem->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::SystemUtil::ShowMessageBox(this, _T("tray_menu_item_1 clicked!"), _T("TrayIconTest"));
            return true;
            });
    }
    pMenuItem = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("tray_menu_item_2")));
    if (pMenuItem != nullptr) {
        pMenuItem->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::SystemUtil::ShowMessageBox(this, _T("tray_menu_item_2 clicked!"), _T("TrayIconTest"));
            return true;
            });
    }
    pMenuItem = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("tray_menu_item_3")));
    if (pMenuItem != nullptr) {
        pMenuItem->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::SystemUtil::ShowMessageBox(this, _T("tray_menu_item_3 clicked!"), _T("TrayIconTest"));
            return true;
            });
    }
    pMenuItem = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("tray_menu_item_4")));
    if (pMenuItem != nullptr) {
        pMenuItem->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::SystemUtil::ShowMessageBox(this, _T("tray_menu_item_4 clicked!"), _T("TrayIconTest"));
            return true;
            });
    }
    pMenuItem = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("tray_menu_exit")));
    if (pMenuItem != nullptr) {
        pMenuItem->AttachClick([this](const ui::EventArgs& /*args*/) {
            this->CloseWnd();
            return true;
            });
    }
}
