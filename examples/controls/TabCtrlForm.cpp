#include "TabCtrlForm.h"
#include <limits>

namespace
{
    /** 从语言文件读取指定 STRID 的文本（支持中英双语）
    */
    DString TabCtrlLang(const DString& strId)
    {
        return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
    }
}

TabCtrlForm::TabCtrlForm():
    m_nTabIndex(1),
    m_nTabBoxSelectCount(0),
    m_nCloseRequestCount(0),
    m_nCloseRequestIndex(-1),
    m_nNewTabCount(0)
{
}

TabCtrlForm::~TabCtrlForm()
{
}

DString TabCtrlForm::GetSkinFolder()
{
    return _T("controls");
}

DString TabCtrlForm::GetSkinFile()
{
    return _T("tabctrl.xml");
}

bool TabCtrlForm::OnLanguageChanged()
{
    const bool bRet = BaseClass::OnLanguageChanged();
    //XML 里写死的 text_id 由框架自动刷新；代码生成的动态文本需要自己重建
    RefreshAllState();
    return bRet;
}

void TabCtrlForm::RefreshAllState()
{
    UpdateOrientState();
    UpdateFixedState();
    UpdateHOverflowState();
    UpdateVOverflowState();
    //C1–C9 新增卡片
    UpdateC1State();
    UpdateC2State();
    UpdateC3State();
    UpdateC4State();
    UpdateC5State();
    UpdateC6State();
    UpdateC7State();
    UpdateC8State();
    UpdateC9State();
    //⑮–㉒ 界面美观与交互
    UpdateC15State();
    UpdateC16State();
    UpdateC17State();
    UpdateC18State();
    UpdateC19State();
    UpdateC20State();
    UpdateC21State();
    UpdateC22State();
    //开关类按钮的文字依赖当前状态，语言切换后需要重建
    RefreshToggleButtons();
}

ui::TabCtrl* TabCtrlForm::FindTabCtrl(const DString& name)
{
    return dynamic_cast<ui::TabCtrl*>(FindControl(name));
}

void TabCtrlForm::BindClick(const DString& name, const ui::EventCallback& callback)
{
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(name));
    if (pButton != nullptr) {
        pButton->AttachClick(callback);
    }
}

ui::TabCtrlItem* TabCtrlForm::CreateTabItem(ui::TabCtrl* pTabCtrl, const DString& title, bool bFixed,
                                            int32_t nFixedWidth, bool bAddToCtrl)
{
    if (pTabCtrl == nullptr) {
        return nullptr;
    }
    //标签项的外观来自模板 XML（与 cef_browser 的做法一致）
    ui::TabCtrlItem* pItem = new ui::TabCtrlItem(pTabCtrl->GetWindow());
    ui::GlobalManager::Instance().FillBoxWithCache(pItem, ui::FilePath(_T("controls/tabctrl_item.xml")));
    pItem->SetTitle(title);
    //垂直标签栏里让标签项拉伸填满宽度；水平方向则用固定宽度，保证标签总宽能超出标签栏
    if (nFixedWidth > 0) {
        //必须按 DPI 缩放，否则高 DPI 下标签宽度会小于文字宽度，标题被截断
        pItem->SetMinWidth(nFixedWidth, true);
        pItem->SetMaxWidth(nFixedWidth, true);
        pItem->SetFixedWidth(ui::UiFixedInt(nFixedWidth), true, true);
    }
    else {
        //恢复成"无上下限 + 拉伸"：模板里的 min/max 必须一起放开，
        //否则 max_width 残留会把布局宽度 clamp 成 0（控件就看不见了）
        pItem->SetMinWidth(0, false);
        pItem->SetMaxWidth(std::numeric_limits<int32_t>::max(), false);
        pItem->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
    }
    //先设置固定状态再加入标签栏：AddItem 会据此把标签锚定到固定块末尾
    pItem->SetFixed(bFixed);
    if (bAddToCtrl) {
        pTabCtrl->AddItem(pItem);
    }
    return pItem;
}

DString TabCtrlForm::GetSelectedTitle(ui::TabCtrl* pTabCtrl) const
{
    if (pTabCtrl == nullptr) {
        return TabCtrlLang(_T("STRID_TABCTRL_STATE_NONE"));
    }
    ui::TabCtrlItem* pItem = pTabCtrl->GetSelectedTabItem();
    if (pItem == nullptr) {
        return TabCtrlLang(_T("STRID_TABCTRL_STATE_NONE"));
    }
    return pItem->GetTitle();
}

void TabCtrlForm::SetToggleText(const DString& btnName, const DString& formatId, bool bOn)
{
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(btnName));
    if (pButton == nullptr) {
        return;
    }
    const DString stateText = TabCtrlLang(bOn ? _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF"));
    pButton->SetText(ui::StringUtil::Printf(TabCtrlLang(formatId).c_str(), stateText.c_str()).c_str());
}

void TabCtrlForm::RefreshToggleButtons()
{
    ui::TabCtrl* pCtrl2 = FindTabCtrl(_T("tab_ctrl_c2"));
    ui::TabCtrl* pCtrl5 = FindTabCtrl(_T("tab_ctrl_c5"));
    ui::TabCtrl* pCtrl6 = FindTabCtrl(_T("tab_ctrl_c6"));
    ui::TabCtrl* pCtrl7 = FindTabCtrl(_T("tab_ctrl_c7"));
    ui::TabCtrl* pCtrl8 = FindTabCtrl(_T("tab_ctrl_c8"));

    if (pCtrl2 != nullptr) {
        SetToggleText(_T("btn_c2_middle"), _T("STRID_TABCTRL_BTN_MIDDLE"), pCtrl2->IsEnableMiddleClickClose());
    }
    if (pCtrl5 != nullptr) {
        SetToggleText(_T("btn_c5_wheel"), _T("STRID_TABCTRL_BTN_WHEEL"), pCtrl5->IsEnableWheelSwitch());
        SetToggleText(_T("btn_c5_drag"), _T("STRID_TABCTRL_BTN_DRAG"), pCtrl5->IsEnableDragOrder());
    }
    if (pCtrl6 != nullptr) {
        SetToggleText(_T("btn_c6_buttons"), _T("STRID_TABCTRL_BTN_BUTTONS"), pCtrl6->IsEnableOverflowButtons());
    }
    if (pCtrl7 != nullptr) {
        SetToggleText(_T("btn_c7_justify"), _T("STRID_TABCTRL_BTN_JUSTIFY"), pCtrl7->IsTabItemJustify());
    }
    if (pCtrl8 != nullptr) {
        SetToggleText(_T("btn_c8_newtab"), _T("STRID_TABCTRL_BTN_NEWTAB"), pCtrl8->IsEnableNewTabButton());
    }
    //"可关闭"按钮的文字跟随选中项，由 UpdateC2State 负责
}

bool TabCtrlForm::FillTabCtrl(const DString& name, size_t nTarget, int32_t& nStartIndex, int32_t nFixedWidth)
{
    ui::TabCtrl* pTabCtrl = FindTabCtrl(name);
    if (pTabCtrl == nullptr) {
        return false;
    }
    bool bChanged = false;
    //数量不足时追加（标签文字带序号，占位符由语言文件提供）
    while (pTabCtrl->GetItemCount() < nTarget) {
        const DString title = ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_TAB_N")).c_str(), nStartIndex);
        ++nStartIndex;
        if (CreateTabItem(pTabCtrl, title, false, nFixedWidth) != nullptr) {
            bChanged = true;
        }
    }
    //数量超出时删除末尾
    while (pTabCtrl->GetItemCount() > nTarget) {
        pTabCtrl->RemoveItemAt(pTabCtrl->GetItemCount() - 1);
        bChanged = true;
    }
    //删除后滚动位置可能超出新的滚动范围，这里归零，避免状态显示不一致
    pTabCtrl->SetScrollPos(ui::UiSize64());
    return bChanged;
}

void TabCtrlForm::ToggleOverflowScroll(const DString& name, const DString& btnName)
{
    ui::TabCtrl* pTabCtrl = FindTabCtrl(name);
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(btnName));
    if ((pTabCtrl == nullptr) || (pButton == nullptr)) {
        return;
    }
    const bool bEnable = !pTabCtrl->IsEnableOverflowScroll();
    pTabCtrl->SetEnableOverflowScroll(bEnable);
    pButton->SetText(TabCtrlLang(bEnable ? _T("STRID_TABCTRL_BTN_OVERFLOW_ON") : _T("STRID_TABCTRL_BTN_OVERFLOW_OFF")));
}

void TabCtrlForm::OnInitWindow()
{
    /////////////////////// ① 布局方向 ///////////////////////
    ui::TabCtrl* pOrientCtrl = FindTabCtrl(_T("tab_ctrl_orient"));
    BindClick(_T("btn_orient_vert"), [this, pOrientCtrl](const ui::EventArgs& /*args*/) {
        if (pOrientCtrl != nullptr) {
            pOrientCtrl->SetTabCtrlOrientation(ui::TabCtrlOrientation::kVertical);
            UpdateOrientState();
        }
        return true;
        });
    BindClick(_T("btn_orient_horz"), [this, pOrientCtrl](const ui::EventArgs& /*args*/) {
        if (pOrientCtrl != nullptr) {
            pOrientCtrl->SetTabCtrlOrientation(ui::TabCtrlOrientation::kHorizontal);
            UpdateOrientState();
        }
        return true;
        });
    BindClick(_T("btn_orient_overflow"), [this, pOrientCtrl](const ui::EventArgs& /*args*/) {
        if (pOrientCtrl == nullptr) {
            return true;
        }
        const bool bEnable = !pOrientCtrl->IsEnableOverflowScroll();
        pOrientCtrl->SetEnableOverflowScroll(bEnable);
        dynamic_cast<ui::Button*>(FindControl(_T("btn_orient_overflow")))->SetText(
            TabCtrlLang(bEnable ? _T("STRID_TABCTRL_BTN_OVERFLOW_ON") : _T("STRID_TABCTRL_BTN_OVERFLOW_OFF")));
        UpdateOrientState();
        return true;
        });

    /////////////////////// ② 固定标签 ///////////////////////
    ui::TabCtrl* pFixedCtrl = FindTabCtrl(_T("tab_ctrl_fixed"));
    //追加固定标签：AddItem 会把它锚定到固定块末尾
    BindClick(_T("btn_fix_add"), [this, pFixedCtrl](const ui::EventArgs& /*args*/) {
        if (pFixedCtrl != nullptr) {
            const DString title = ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_FIXED_N")).c_str(), m_nTabIndex);
            ++m_nTabIndex;
            ui::TabCtrlItem* pItem = CreateTabItem(pFixedCtrl, title, true);
            if (pItem != nullptr) {
                const size_t nIndex = pFixedCtrl->GetItemIndex(pItem);
                if (nIndex != ui::Box::InvalidIndex) {
                    pFixedCtrl->SelectItem(nIndex, false, true);
                }
            }
            UpdateFixedState();
        }
        return true;
        });
    //固定/取消固定选中项：SetFixed 会把标签归位到固定块边界
    BindClick(_T("btn_fix_toggle"), [this, pFixedCtrl](const ui::EventArgs& /*args*/) {
        if (pFixedCtrl != nullptr) {
            const size_t nCurSel = pFixedCtrl->GetCurSel();
            if (nCurSel != ui::Box::InvalidIndex) {
                ui::TabCtrlItem* pItem = dynamic_cast<ui::TabCtrlItem*>(pFixedCtrl->GetItemAt(nCurSel));
                if (pItem != nullptr) {
                    pItem->SetFixed(!pItem->IsFixed());
                }
            }
            UpdateFixedState();
        }
        return true;
        });
    //删除选中项
    BindClick(_T("btn_fix_remove"), [this, pFixedCtrl](const ui::EventArgs& /*args*/) {
        if (pFixedCtrl != nullptr) {
            const size_t nCurSel = pFixedCtrl->GetCurSel();
            if (nCurSel != ui::Box::InvalidIndex) {
                pFixedCtrl->RemoveItemAt(nCurSel);
            }
            UpdateFixedState();
        }
        return true;
        });
    //重置为初始的 2 固定 + 3 普通（标题走语言表，保证切换语言后重置仍是当前语言）
    BindClick(_T("btn_fix_reset"), [this, pFixedCtrl](const ui::EventArgs& /*args*/) {
        if (pFixedCtrl != nullptr) {
            pFixedCtrl->RemoveAllItems();
            CreateTabItem(pFixedCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_HOME")), true);
            CreateTabItem(pFixedCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_FAVORITE")), true);
            CreateTabItem(pFixedCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_MSG")), false);
            CreateTabItem(pFixedCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_FEED")), false);
            CreateTabItem(pFixedCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_SETTINGS")), false);
            const size_t nIndex = pFixedCtrl->GetItemIndex(pFixedCtrl->GetItemAt(0));
            if (nIndex != ui::Box::InvalidIndex) {
                pFixedCtrl->SelectItem(nIndex, false, true);
            }
            UpdateFixedState();
        }
        return true;
        });
    if (pFixedCtrl != nullptr) {
        pFixedCtrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateFixedState();
            return true;
            });
    }

    /////////////////////// ③ 水平溢出滚动 ///////////////////////
    //标签栏宽 300、标签项宽 96：第 4 个标签起溢出，10 个可充分验证滚动范围
    BindClick(_T("btn_h_fill"), [this](const ui::EventArgs& /*args*/) {
        FillTabCtrl(_T("tab_ctrl_hoverflow"), 10, m_nTabIndex, 96);
        UpdateHOverflowState();
        return true;
        });
    BindClick(_T("btn_h_toggle"), [this](const ui::EventArgs& /*args*/) {
        ToggleOverflowScroll(_T("tab_ctrl_hoverflow"), _T("btn_h_toggle"));
        UpdateHOverflowState();
        return true;
        });
    BindClick(_T("btn_h_home"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_hoverflow"));
        if (pCtrl != nullptr) {
            //回到最左：注意必须用水平方向的 HomeLeft/EndRight，
            //EndDown/HomeUp 改的是 cy（垂直），对水平标签栏无效
            pCtrl->HomeLeft();
            UpdateHOverflowState();
        }
        return true;
        });
    BindClick(_T("btn_h_end"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_hoverflow"));
        if (pCtrl != nullptr) {
            //滚到最右：EndRight 改的是 sz.cx（水平方向），不能用 EndDown
            pCtrl->EndRight();
            UpdateHOverflowState();
        }
        return true;
        });
    BindClick(_T("btn_h_page"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_hoverflow"));
        if (pCtrl != nullptr) {
            pCtrl->PageRight();
            UpdateHOverflowState();
        }
        return true;
        });
    BindClick(_T("btn_h_clear"), [this](const ui::EventArgs& /*args*/) {
        FillTabCtrl(_T("tab_ctrl_hoverflow"), 0, m_nTabIndex, 96);
        UpdateHOverflowState();
        return true;
        });
    if (ui::TabCtrl* pHCtrl = FindTabCtrl(_T("tab_ctrl_hoverflow"))) {
        pHCtrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateHOverflowState();
            return true;
            });
    }

    /////////////////////// ④ 垂直溢出滚动 ///////////////////////
    //标签栏高 110、标签项高 32+4 边距：第 4 个标签起溢出；宽度用 stretch 填满标签栏
    BindClick(_T("btn_v_fill"), [this](const ui::EventArgs& /*args*/) {
        FillTabCtrl(_T("tab_ctrl_voverflow"), 8, m_nTabIndex, 0);
        UpdateVOverflowState();
        return true;
        });
    BindClick(_T("btn_v_toggle"), [this](const ui::EventArgs& /*args*/) {
        ToggleOverflowScroll(_T("tab_ctrl_voverflow"), _T("btn_v_toggle"));
        UpdateVOverflowState();
        return true;
        });
    BindClick(_T("btn_v_home"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_voverflow"));
        if (pCtrl != nullptr) {
            pCtrl->HomeUp();
            UpdateVOverflowState();
        }
        return true;
        });
    BindClick(_T("btn_v_end"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_voverflow"));
        if (pCtrl != nullptr) {
            pCtrl->EndDown();
            UpdateVOverflowState();
        }
        return true;
        });
    BindClick(_T("btn_v_page"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_voverflow"));
        if (pCtrl != nullptr) {
            pCtrl->PageDown();
            UpdateVOverflowState();
        }
        return true;
        });
    BindClick(_T("btn_v_clear"), [this](const ui::EventArgs& /*args*/) {
        FillTabCtrl(_T("tab_ctrl_voverflow"), 0, m_nTabIndex, 0);
        UpdateVOverflowState();
        return true;
        });
    if (ui::TabCtrl* pVCtrl = FindTabCtrl(_T("tab_ctrl_voverflow"))) {
        pVCtrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateVOverflowState();
            return true;
            });
    }

    //尺寸变化时刷新状态：OnInitWindow 阶段布局还没完成，
    //此时 GetWidth()/GetScrollRange() 都还是 0，必须等 kEventSizeChanged 才能拿到真实值
    auto refreshOverflowState = [this](const ui::EventArgs& /*args*/) {
        UpdateHOverflowState();
        UpdateVOverflowState();
        return true;
        };
    if (ui::TabCtrl* pHCtrl = FindTabCtrl(_T("tab_ctrl_hoverflow"))) {
        pHCtrl->AttachEvent(ui::kEventSizeChanged, refreshOverflowState, 0);
    }
    if (ui::TabCtrl* pVCtrl2 = FindTabCtrl(_T("tab_ctrl_voverflow"))) {
        pVCtrl2->AttachEvent(ui::kEventSizeChanged, refreshOverflowState, 0);
    }

    /////////////////////// ⑥ C1：TabBox ↔ TabCtrl 双向同步 ///////////////////////
    //从 TabCtrl 侧选择：框架会把选择同步到绑定的 TabBox
    for (int32_t i = 0; i < 3; ++i) {
        const DString btnName = ui::StringUtil::Printf(_T("btn_c1_ctrl_%d"), i + 1);
        BindClick(btnName, [this, i](const ui::EventArgs& /*args*/) {
            ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c1"));
            if (pCtrl != nullptr) {
                pCtrl->SelectItem(static_cast<size_t>(i), false, true);
            }
            UpdateC1State();
            return true;
            });
    }
    //从 TabBox 侧选择：框架会把选择同步回 TabCtrl
    for (int32_t i = 0; i < 3; ++i) {
        const DString btnName = ui::StringUtil::Printf(_T("btn_c1_box_%d"), i + 1);
        BindClick(btnName, [this, i](const ui::EventArgs& /*args*/) {
            ui::TabBox* pBox = dynamic_cast<ui::TabBox*>(FindControl(_T("tab_box_c1")));
            if (pBox != nullptr) {
                pBox->SelectItem(static_cast<size_t>(i));
            }
            UpdateC1State();
            return true;
            });
    }
    if (ui::TabCtrl* pC1Ctrl = FindTabCtrl(_T("tab_ctrl_c1"))) {
        pC1Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC1State();
            return true;
            });
    }
    if (ui::TabBox* pC1Box = dynamic_cast<ui::TabBox*>(FindControl(_T("tab_box_c1")))) {
        //统计 TabBox 的选择事件：每次选择变化应只增加 1 次，用于验证同步没有死循环
        pC1Box->AttachTabSelect([this](const ui::EventArgs& /*args*/) {
            ++m_nTabBoxSelectCount;
            UpdateC1State();
            return true;
            });
    }

    /////////////////////// ⑦ C2：关闭请求 / closable / 中键关闭 ///////////////////////
    if (ui::TabCtrl* pC2Ctrl = FindTabCtrl(_T("tab_ctrl_c2"))) {
        //kEventTabClose 只是"请求"，框架不删除标签，删除由业务层决定
        pC2Ctrl->AttachTabClose([this](const ui::EventArgs& args) {
            ++m_nCloseRequestCount;
            m_nCloseRequestIndex = static_cast<int32_t>(args.wParam);
            UpdateC2State();
            return true;
            });
        pC2Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC2State();
            return true;
            });
    }
    //切换选中项的可关闭状态（不可关闭的标签不显示关闭按钮，中键也不触发关闭请求）
    BindClick(_T("btn_c2_closable"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c2"));
        if (pCtrl != nullptr) {
            ui::TabCtrlItem* pItem = pCtrl->GetSelectedTabItem();
            if (pItem != nullptr) {
                pItem->SetClosable(!pItem->IsClosable());
            }
        }
        UpdateC2State();
        return true;
        });
    BindClick(_T("btn_c2_middle"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c2"));
        if (pCtrl != nullptr) {
            pCtrl->SetEnableMiddleClickClose(!pCtrl->IsEnableMiddleClickClose());
        }
        RefreshToggleButtons();
        UpdateC2State();
        return true;
        });
    //业务层响应"关闭请求"：真正删除选中标签
    BindClick(_T("btn_c2_remove"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c2"));
        if (pCtrl != nullptr) {
            const size_t nCurSel = pCtrl->GetCurSel();
            if (nCurSel != ui::Box::InvalidIndex) {
                pCtrl->RemoveItemAt(nCurSel);
            }
        }
        UpdateC2State();
        return true;
        });
    BindClick(_T("btn_c2_reset"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c2"));
        if (pCtrl != nullptr) {
            pCtrl->RemoveAllItems();
            CreateTabItem(pCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_HOME")), false, 96);
            ui::TabCtrlItem* pItem2 = CreateTabItem(pCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_FAVORITE")), false, 96);
            CreateTabItem(pCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_MSG")), false, 96);
            CreateTabItem(pCtrl, TabCtrlLang(_T("STRID_TAB_CTRL_SETTINGS")), false, 96);
            if (pItem2 != nullptr) {
                pItem2->SetClosable(false);
            }
            pCtrl->SelectItem(0, false, true);
        }
        UpdateC2State();
        return true;
        });

    /////////////////////// ⑧ C3：选中项自动滚入视野 ///////////////////////
    BindClick(_T("btn_c3_first"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c3"));
        if (pCtrl != nullptr) {
            pCtrl->SelectItem(0, false, true);
        }
        UpdateC3State();
        return true;
        });
    BindClick(_T("btn_c3_last"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c3"));
        if ((pCtrl != nullptr) && (pCtrl->GetItemCount() > 0)) {
            pCtrl->SelectItem(pCtrl->GetItemCount() - 1, false, true);
        }
        UpdateC3State();
        return true;
        });
    BindClick(_T("btn_c3_prev"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c3"));
        if (pCtrl != nullptr) {
            pCtrl->SelectPrevTab(false, true);
        }
        UpdateC3State();
        return true;
        });
    BindClick(_T("btn_c3_next"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c3"));
        if (pCtrl != nullptr) {
            pCtrl->SelectNextTab(false, true);
        }
        UpdateC3State();
        return true;
        });
    BindClick(_T("btn_c3_fill"), [this](const ui::EventArgs& /*args*/) {
        FillTabCtrl(_T("tab_ctrl_c3"), 10, m_nTabIndex, 96);
        UpdateC3State();
        return true;
        });
    BindClick(_T("btn_c3_clear"), [this](const ui::EventArgs& /*args*/) {
        FillTabCtrl(_T("tab_ctrl_c3"), 0, m_nTabIndex, 96);
        UpdateC3State();
        return true;
        });
    if (ui::TabCtrl* pC3Ctrl = FindTabCtrl(_T("tab_ctrl_c3"))) {
        pC3Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC3State();
            return true;
            });
    }

    /////////////////////// ⑨ C4：键盘导航 ///////////////////////
    BindClick(_T("btn_c4_focus"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c4"));
        if (pCtrl != nullptr) {
            pCtrl->SetFocus();
        }
        UpdateC4State();
        return true;
        });
    BindClick(_T("btn_c4_prev"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c4"));
        if (pCtrl != nullptr) {
            pCtrl->SelectPrevTab(true, true);
        }
        UpdateC4State();
        return true;
        });
    BindClick(_T("btn_c4_next"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c4"));
        if (pCtrl != nullptr) {
            pCtrl->SelectNextTab(true, true);
        }
        UpdateC4State();
        return true;
        });
    if (ui::TabCtrl* pC4Ctrl = FindTabCtrl(_T("tab_ctrl_c4"))) {
        pC4Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC4State();
            return true;
            });
        //焦点变化时刷新状态显示（键盘事件只在标签栏有焦点时生效）
        pC4Ctrl->AttachEvent(ui::kEventSetFocus, [this](const ui::EventArgs& /*args*/) {
            UpdateC4State();
            return true;
            }, 0);
        pC4Ctrl->AttachEvent(ui::kEventKillFocus, [this](const ui::EventArgs& /*args*/) {
            UpdateC4State();
            return true;
            }, 0);
    }

    /////////////////////// ⑩ C5：滚轮切标签 / 拖动排序 ///////////////////////
    BindClick(_T("btn_c5_wheel"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c5"));
        if (pCtrl != nullptr) {
            pCtrl->SetEnableWheelSwitch(!pCtrl->IsEnableWheelSwitch());
        }
        RefreshToggleButtons();
        UpdateC5State();
        return true;
        });
    BindClick(_T("btn_c5_drag"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c5"));
        if (pCtrl != nullptr) {
            pCtrl->SetEnableDragOrder(!pCtrl->IsEnableDragOrder());
        }
        RefreshToggleButtons();
        UpdateC5State();
        return true;
        });
    if (ui::TabCtrl* pC5Ctrl = FindTabCtrl(_T("tab_ctrl_c5"))) {
        pC5Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC5State();
            return true;
            });
    }

    /////////////////////// ⑪ C6：溢出箭头按钮 ///////////////////////
    BindClick(_T("btn_c6_buttons"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c6"));
        if (pCtrl != nullptr) {
            pCtrl->SetEnableOverflowButtons(!pCtrl->IsEnableOverflowButtons());
        }
        RefreshToggleButtons();
        UpdateC6State();
        return true;
        });
    BindClick(_T("btn_c6_fill"), [this](const ui::EventArgs& /*args*/) {
        FillTabCtrl(_T("tab_ctrl_c6"), 10, m_nTabIndex, 96);
        UpdateC6State();
        return true;
        });
    BindClick(_T("btn_c6_home"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c6"));
        if (pCtrl != nullptr) {
            pCtrl->HomeLeft();
        }
        UpdateC6State();
        return true;
        });
    BindClick(_T("btn_c6_end"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c6"));
        if (pCtrl != nullptr) {
            pCtrl->EndRight();
        }
        UpdateC6State();
        return true;
        });
    BindClick(_T("btn_c6_clear"), [this](const ui::EventArgs& /*args*/) {
        FillTabCtrl(_T("tab_ctrl_c6"), 0, m_nTabIndex, 96);
        UpdateC6State();
        return true;
        });
    if (ui::TabCtrl* pC6Ctrl = FindTabCtrl(_T("tab_ctrl_c6"))) {
        pC6Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC6State();
            return true;
            });
        pC6Ctrl->AttachEvent(ui::kEventSizeChanged, [this](const ui::EventArgs& /*args*/) {
            UpdateC6State();
            return true;
            }, 0);
    }

    /////////////////////// ⑫ C7：等宽填充 / min-max 宽度 ///////////////////////
    BindClick(_T("btn_c7_justify"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c7"));
        if (pCtrl != nullptr) {
            pCtrl->SetTabItemJustify(!pCtrl->IsTabItemJustify());
        }
        RefreshToggleButtons();
        UpdateC7State();
        return true;
        });
    if (ui::TabCtrl* pC7Ctrl = FindTabCtrl(_T("tab_ctrl_c7"))) {
        pC7Ctrl->AttachEvent(ui::kEventSizeChanged, [this](const ui::EventArgs& /*args*/) {
            UpdateC7State();
            return true;
            }, 0);
    }

    /////////////////////// ⑬ C8：新建标签按钮 ///////////////////////
    if (ui::TabCtrl* pC8Ctrl = FindTabCtrl(_T("tab_ctrl_c8"))) {
        //kEventTabNew 只是通知，新建标签由业务层完成
        pC8Ctrl->AttachTabNew([this](const ui::EventArgs& /*args*/) {
            ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c8"));
            if (pCtrl != nullptr) {
                const DString title = ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_TAB_N")).c_str(), m_nTabIndex);
                ++m_nTabIndex;
                ui::TabCtrlItem* pItem = CreateTabItem(pCtrl, title, false, 96);
                if (pItem != nullptr) {
                    const size_t nIndex = pCtrl->GetItemIndex(pItem);
                    if (nIndex != ui::Box::InvalidIndex) {
                        pCtrl->SelectItem(nIndex, false, true);
                    }
                }
            }
            ++m_nNewTabCount;
            UpdateC8State();
            return true;
            });
    }
    BindClick(_T("btn_c8_newtab"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c8"));
        if (pCtrl != nullptr) {
            pCtrl->SetEnableNewTabButton(!pCtrl->IsEnableNewTabButton());
        }
        RefreshToggleButtons();
        UpdateC8State();
        return true;
        });
    BindClick(_T("btn_c8_remove"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c8"));
        if ((pCtrl != nullptr) && (pCtrl->GetItemCount() > 0)) {
            pCtrl->RemoveItemAt(pCtrl->GetItemCount() - 1);
        }
        UpdateC8State();
        return true;
        });

    /////////////////////// ⑭ C9：便捷 API ///////////////////////
    BindClick(_T("btn_c9_add"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c9"));
        if (pCtrl != nullptr) {
            const DString title = ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_TAB_N")).c_str(), m_nTabIndex);
            ++m_nTabIndex;
            //先创建（不加入控件树），再用 AddTabItemToEnd 追加
            ui::TabCtrlItem* pItem = CreateTabItem(pCtrl, title, false, 96, false);
            if (pItem != nullptr) {
                pCtrl->AddTabItemToEnd(pItem);
                pCtrl->SetSelectedTabItem(pItem);
            }
            m_c9LastAction = _T("AddTabItemToEnd");
        }
        UpdateC9State();
        return true;
        });
    BindClick(_T("btn_c9_get"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c9"));
        DString title = TabCtrlLang(_T("STRID_TABCTRL_STATE_NONE"));
        if ((pCtrl != nullptr) && (pCtrl->GetSelectedTabItem() != nullptr)) {
            title = pCtrl->GetSelectedTabItem()->GetTitle();
        }
        m_c9LastAction = ui::StringUtil::Printf(_T("GetSelectedTabItem → %s"), title.c_str());
        UpdateC9State();
        return true;
        });
    BindClick(_T("btn_c9_set"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c9"));
        if (pCtrl != nullptr) {
            ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(1);
            if (pItem != nullptr) {
                pCtrl->SetSelectedTabItem(pItem);
                m_c9LastAction = _T("SetSelectedTabItem(2)");
            }
        }
        UpdateC9State();
        return true;
        });
    BindClick(_T("btn_c9_prev"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c9"));
        if (pCtrl != nullptr) {
            pCtrl->SelectPrevTab();
            m_c9LastAction = _T("SelectPrevTab");
        }
        UpdateC9State();
        return true;
        });
    BindClick(_T("btn_c9_next"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c9"));
        if (pCtrl != nullptr) {
            pCtrl->SelectNextTab();
            m_c9LastAction = _T("SelectNextTab");
        }
        UpdateC9State();
        return true;
        });
    if (ui::TabCtrl* pC9Ctrl = FindTabCtrl(_T("tab_ctrl_c9"))) {
        pC9Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC9State();
            return true;
            });
    }

    /////////////////////// ⑮ 指示条滑动动画 ///////////////////////
    BindClick(_T("btn_anim_toggle"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_anim"));
        if (pCtrl != nullptr) {
            pCtrl->SetEnableIndicatorAnimation(!pCtrl->IsEnableIndicatorAnimation());
        }
        UpdateC15State();
        return true;
        });
    BindClick(_T("btn_anim_duration"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_anim"));
        if (pCtrl != nullptr) {
            //时长按 100 → 200 → 400 → 800 ms 循环
            const uint32_t nDuration = pCtrl->GetIndicatorAnimationDuration();
            uint32_t nNext = 100;
            if (nDuration < 200) {
                nNext = 200;
            }
            else if (nDuration < 400) {
                nNext = 400;
            }
            else if (nDuration < 800) {
                nNext = 800;
            }
            pCtrl->SetIndicatorAnimationDuration(nNext);
        }
        UpdateC15State();
        return true;
        });
    BindClick(_T("btn_anim_next"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_anim"));
        if (pCtrl != nullptr) {
            pCtrl->SelectNextTab(false, true);
        }
        UpdateC15State();
        return true;
        });
    if (ui::TabCtrl* pC15Ctrl = FindTabCtrl(_T("tab_ctrl_anim"))) {
        pC15Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC15State();
            return true;
            });
    }

    /////////////////////// ⑯ 选中态层次（加粗 / 淡底色） ///////////////////////
    BindClick(_T("btn_selected_font"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_selected"));
        if (pCtrl != nullptr) {
            ui::TabCtrlItem* pFirst = pCtrl->GetTabItemAt(0);
            const bool bOn = (pFirst != nullptr) && !pFirst->GetSelectedTitleFont().empty();
            for (size_t i = 0; i < pCtrl->GetItemCount(); ++i) {
                ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(i);
                if (pItem != nullptr) {
                    pItem->SetSelectedTitleFont(bOn ? DString() : DString(_T("system_bold_12")));
                }
            }
        }
        UpdateC16State();
        return true;
        });
    BindClick(_T("btn_selected_bg"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_selected"));
        if (pCtrl != nullptr) {
            ui::TabCtrlItem* pFirst = pCtrl->GetTabItemAt(0);
            const bool bOn = (pFirst != nullptr) &&
                             !pFirst->GetSelectStateColor(ui::ControlStateType::kControlStateNormal).empty();
            for (size_t i = 0; i < pCtrl->GetItemCount(); ++i) {
                ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(i);
                if (pItem != nullptr) {
                    pItem->SetSelectedStateColor(ui::ControlStateType::kControlStateNormal,
                                                 bOn ? DString() : DString(_T("bg_tab_ctrl_item_selected")));
                }
            }
        }
        UpdateC16State();
        return true;
        });
    if (ui::TabCtrl* pC16Ctrl = FindTabCtrl(_T("tab_ctrl_selected"))) {
        pC16Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC16State();
            return true;
            });
    }

    /////////////////////// ⑰ 选中 + 悬停反馈 ///////////////////////
    BindClick(_T("btn_hover_title"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_hover"));
        if (pCtrl != nullptr) {
            ui::TabCtrlItem* pFirst = pCtrl->GetTabItemAt(0);
            const bool bOn = (pFirst != nullptr) && !pFirst->GetSelectedHoveredTitleColor().empty();
            for (size_t i = 0; i < pCtrl->GetItemCount(); ++i) {
                ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(i);
                if (pItem != nullptr) {
                    pItem->SetSelectedHoveredTitleColor(bOn ? DString() : DString(_T("color_accent")));
                }
            }
        }
        UpdateC17State();
        return true;
        });
    if (ui::TabCtrl* pC17Ctrl = FindTabCtrl(_T("tab_ctrl_hover"))) {
        pC17Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC17State();
            return true;
            });
    }

    /////////////////////// ⑱ 垂直模式分隔线 ///////////////////////
    BindClick(_T("btn_line_vert"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_line"));
        if (pCtrl != nullptr) {
            pCtrl->SetTabCtrlOrientation(ui::TabCtrlOrientation::kVertical);
        }
        UpdateC18State();
        //分隔线的方向要等标签栏重新布局后才更新，延迟再刷新一次，避免状态显示滞后
        DeferUpdateC18State();
        return true;
        });
    BindClick(_T("btn_line_horz"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_line"));
        if (pCtrl != nullptr) {
            pCtrl->SetTabCtrlOrientation(ui::TabCtrlOrientation::kHorizontal);
        }
        UpdateC18State();
        //分隔线的方向要等标签栏重新布局后才更新，延迟再刷新一次，避免状态显示滞后
        DeferUpdateC18State();
        return true;
        });
    if (ui::TabCtrl* pC18Ctrl = FindTabCtrl(_T("tab_ctrl_line"))) {
        pC18Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC18State();
            return true;
            });
        pC18Ctrl->AttachEvent(ui::kEventSizeChanged, [this](const ui::EventArgs& /*args*/) {
            UpdateC18State();
            return true;
            }, 0);
    }

    /////////////////////// ⑲ 底边线断开策略 ///////////////////////
    BindClick(_T("btn_bottom_line_mode"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_bottom_line"));
        if (pCtrl != nullptr) {
            //auto → break → full → auto 循环
            ui::TabCtrlBottomLineMode mode = pCtrl->GetTabCtrlBottomLineMode();
            if (mode == ui::TabCtrlBottomLineMode::kAuto) {
                mode = ui::TabCtrlBottomLineMode::kBreak;
            }
            else if (mode == ui::TabCtrlBottomLineMode::kBreak) {
                mode = ui::TabCtrlBottomLineMode::kFull;
            }
            else {
                mode = ui::TabCtrlBottomLineMode::kAuto;
            }
            pCtrl->SetTabCtrlBottomLineMode(mode);
        }
        UpdateC19State();
        return true;
        });
    if (ui::TabCtrl* pC19Ctrl = FindTabCtrl(_T("tab_ctrl_bottom_line"))) {
        pC19Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC19State();
            return true;
            });
    }

    /////////////////////// ⑳ 焦点框 / 禁用态 ///////////////////////
    BindClick(_T("btn_focus_tabctrl"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_focus"));
        if (pCtrl != nullptr) {
            pCtrl->SetFocus();
        }
        UpdateC20State();
        return true;
        });
    BindClick(_T("btn_disable_tabctrl"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_focus"));
        if (pCtrl != nullptr) {
            pCtrl->SetEnabled(!pCtrl->IsEnabled());
        }
        UpdateC20State();
        return true;
        });
    if (ui::TabCtrl* pC20Ctrl = FindTabCtrl(_T("tab_ctrl_focus"))) {
        pC20Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC20State();
            return true;
            });
        pC20Ctrl->AttachEvent(ui::kEventSetFocus, [this](const ui::EventArgs& /*args*/) {
            UpdateC20State();
            return true;
            }, 0);
        pC20Ctrl->AttachEvent(ui::kEventKillFocus, [this](const ui::EventArgs& /*args*/) {
            UpdateC20State();
            return true;
            }, 0);
    }

    /////////////////////// ㉑ 浮动角标 ///////////////////////
    BindClick(_T("btn_badge_float"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_badge_float"));
        if (pCtrl != nullptr) {
            ui::TabCtrlItem* pFirst = pCtrl->GetTabItemAt(0);
            const bool bFloat = (pFirst != nullptr) && pFirst->IsBadgeFloat();
            for (size_t i = 0; i < pCtrl->GetItemCount(); ++i) {
                ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(i);
                if (pItem != nullptr) {
                    pItem->SetBadgeFloat(!bFloat);
                }
            }
        }
        UpdateC21State();
        return true;
        });
    BindClick(_T("btn_badge_add"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_badge_float"));
        if (pCtrl != nullptr) {
            for (size_t i = 0; i < pCtrl->GetItemCount(); ++i) {
                ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(i);
                if (pItem != nullptr) {
                    pItem->SetBadgeCount(pItem->GetBadgeCount() + 10);
                }
            }
        }
        UpdateC21State();
        return true;
        });
    if (ui::TabCtrl* pC21Ctrl = FindTabCtrl(_T("tab_ctrl_badge_float"))) {
        pC21Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC21State();
            return true;
            });
    }

    /////////////////////// ㉒ 关闭按钮显示策略 ///////////////////////
    BindClick(_T("btn_close_mode"), [this](const ui::EventArgs& /*args*/) {
        ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_close"));
        if (pCtrl != nullptr) {
            ui::TabCtrlItem* pFirst = pCtrl->GetTabItemAt(0);
            if (pFirst != nullptr) {
                //auto → hover → always → auto 循环
                ui::TabCtrlCloseButtonShowMode mode = pFirst->GetCloseButtonShowMode();
                if (mode == ui::TabCtrlCloseButtonShowMode::kAuto) {
                    mode = ui::TabCtrlCloseButtonShowMode::kHover;
                }
                else if (mode == ui::TabCtrlCloseButtonShowMode::kHover) {
                    mode = ui::TabCtrlCloseButtonShowMode::kAlways;
                }
                else {
                    mode = ui::TabCtrlCloseButtonShowMode::kAuto;
                }
                for (size_t i = 0; i < pCtrl->GetItemCount(); ++i) {
                    ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(i);
                    if (pItem != nullptr) {
                        pItem->SetCloseButtonShowMode(mode);
                    }
                }
            }
        }
        UpdateC22State();
        return true;
        });
    if (ui::TabCtrl* pC22Ctrl = FindTabCtrl(_T("tab_ctrl_close"))) {
        pC22Ctrl->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateC22State();
            return true;
            });
    }

    RefreshAllState();

    //布局完成后刷新一次：部分状态依赖标签项布局后的实际尺寸（如卡片 ⑱）
    ui::GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), [this]() {
        UpdateC18State();
        }, 200, 1);
}

void TabCtrlForm::UpdateOrientState()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_orient"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_orient_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    const bool bVertical = (pCtrl->GetTabCtrlOrientation() == ui::TabCtrlOrientation::kVertical);
    DString text = ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_ORIENT")).c_str(),
                                           TabCtrlLang(bVertical ? _T("STRID_TABCTRL_STATE_DIR_VERT") : _T("STRID_TABCTRL_STATE_DIR_HORZ")).c_str());
    //垂直方向启用纵向滚动条、水平方向启用横向滚动条，这里一并显示出来便于核对
    DString barState = TabCtrlLang(_T("STRID_TABCTRL_STATE_BAR_DISABLED"));
    if (pCtrl->IsEnableOverflowScroll()) {
        const bool bBarValid = bVertical ? pCtrl->IsVScrollBarValid() : pCtrl->IsHScrollBarValid();
        barState = TabCtrlLang(bBarValid ? _T("STRID_TABCTRL_STATE_BAR_SHOWN") : _T("STRID_TABCTRL_STATE_BAR_NONE"));
    }
    text += ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_SCROLLBAR")).c_str(),
                                   TabCtrlLang(bVertical ? _T("STRID_TABCTRL_STATE_AXIS_VERT") : _T("STRID_TABCTRL_STATE_AXIS_HORZ")).c_str(),
                                   barState.c_str());
    pLabel->SetText(text.c_str());
}

void TabCtrlForm::UpdateFixedState()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_fixed"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_fixed_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    const size_t nFixedCount = pCtrl->GetFixedItemCount();
    DString text = ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_FIXED")).c_str(),
                                           static_cast<int>(pCtrl->GetItemCount()),
                                           static_cast<int>(nFixedCount));
    //列出固定标签名称，便于确认固定块始终位于前端
    for (size_t i = 0; i < nFixedCount; ++i) {
        ui::TabCtrlItem* pItem = dynamic_cast<ui::TabCtrlItem*>(pCtrl->GetItemAt(i));
        if (pItem != nullptr) {
            text += pItem->GetTitle().c_str();
        }
    }
    const size_t nCurSel = pCtrl->GetCurSel();
    if (nCurSel != ui::Box::InvalidIndex) {
        ui::TabCtrlItem* pItem = dynamic_cast<ui::TabCtrlItem*>(pCtrl->GetItemAt(nCurSel));
        if (pItem != nullptr) {
            text += ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_SELECTED")).c_str(),
                                           pItem->GetTitle().c_str(),
                                           TabCtrlLang(pItem->IsFixed() ? _T("STRID_TABCTRL_STATE_KIND_FIXED") : _T("STRID_TABCTRL_STATE_KIND_NORMAL")).c_str());
        }
    }
    pLabel->SetText(text.c_str());
}

void TabCtrlForm::UpdateHOverflowState()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_hoverflow"));
    ui::Label* pTitleLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_hoverflow_state")));
    ui::Label* pRangeLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_hoverflow_range")));
    ui::Label* pPosLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_hoverflow_pos")));
    if ((pCtrl == nullptr) || (pRangeLabel == nullptr) || (pPosLabel == nullptr)) {
        return;
    }
    //滚动范围即"被折叠起来看不见的像素数"，即溢出量
    const int64_t nOverflow = pCtrl->GetScrollRange().cx;
    const int64_t nPos = pCtrl->GetScrollPos().cx;
    const int64_t nVisible = pCtrl->GetWidth();
    if (pTitleLabel != nullptr) {
        DString titleText = ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_COUNT")).c_str(),
                                                    static_cast<int>(pCtrl->GetItemCount()));
        if (!pCtrl->IsEnableOverflowScroll()) {
            titleText += TabCtrlLang(_T("STRID_TABCTRL_STATE_OVERFLOW_OFF")).c_str();
        }
        pTitleLabel->SetText(titleText.c_str());
    }
    pRangeLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_RANGE_H")).c_str(),
                                                 static_cast<int>(nVisible),
                                                 static_cast<int>(nVisible + nOverflow),
                                                 static_cast<int>(nOverflow)).c_str());
    pPosLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_POS")).c_str(),
                                               static_cast<int>(nPos),
                                               static_cast<int>(nOverflow),
                                               TabCtrlLang(pCtrl->IsHScrollBarValid() ?
                                                   _T("STRID_TABCTRL_BAR_SHOWN_SHORT") : _T("STRID_TABCTRL_STATE_BAR_NONE")).c_str()).c_str());
}

void TabCtrlForm::UpdateVOverflowState()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_voverflow"));
    ui::Label* pTitleLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_voverflow_state")));
    ui::Label* pRangeLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_voverflow_range")));
    ui::Label* pPosLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_voverflow_pos")));
    if ((pCtrl == nullptr) || (pRangeLabel == nullptr) || (pPosLabel == nullptr)) {
        return;
    }
    const int64_t nOverflow = pCtrl->GetScrollRange().cy;
    const int64_t nPos = pCtrl->GetScrollPos().cy;
    const int64_t nVisible = pCtrl->GetHeight();
    if (pTitleLabel != nullptr) {
        DString titleText = ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_COUNT")).c_str(),
                                                    static_cast<int>(pCtrl->GetItemCount()));
        if (!pCtrl->IsEnableOverflowScroll()) {
            titleText += TabCtrlLang(_T("STRID_TABCTRL_STATE_OVERFLOW_OFF")).c_str();
        }
        pTitleLabel->SetText(titleText.c_str());
    }
    pRangeLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_RANGE_V")).c_str(),
                                                 static_cast<int>(nVisible),
                                                 static_cast<int>(nVisible + nOverflow),
                                                 static_cast<int>(nOverflow)).c_str());
    pPosLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_POS")).c_str(),
                                               static_cast<int>(nPos),
                                               static_cast<int>(nOverflow),
                                               TabCtrlLang(pCtrl->IsVScrollBarValid() ?
                                                   _T("STRID_TABCTRL_BAR_SHOWN_SHORT") : _T("STRID_TABCTRL_STATE_BAR_NONE")).c_str()).c_str());
}

void TabCtrlForm::UpdateC1State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c1"));
    ui::TabBox* pBox = dynamic_cast<ui::TabBox*>(FindControl(_T("tab_box_c1")));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c1_state")));
    if ((pCtrl == nullptr) || (pBox == nullptr) || (pLabel == nullptr)) {
        return;
    }
    const int32_t nCtrlSel = (pCtrl->GetCurSel() == ui::Box::InvalidIndex) ? -1 : static_cast<int32_t>(pCtrl->GetCurSel());
    const int32_t nBoxSel = (pBox->GetCurSel() == ui::Box::InvalidIndex) ? -1 : static_cast<int32_t>(pBox->GetCurSel());
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C1")).c_str(),
                                            nCtrlSel, nBoxSel, m_nTabBoxSelectCount).c_str());
}

void TabCtrlForm::UpdateC2State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c2"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c2_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    ui::TabCtrlItem* pItem = pCtrl->GetSelectedTabItem();
    const bool bClosable = (pItem != nullptr) && pItem->IsClosable();
    //"可关闭"按钮的文字跟随当前选中项
    SetToggleText(_T("btn_c2_closable"), _T("STRID_TABCTRL_BTN_CLOSABLE"), bClosable);
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C2")).c_str(),
                                            static_cast<int>(pCtrl->GetItemCount()),
                                            GetSelectedTitle(pCtrl).c_str(),
                                            TabCtrlLang(bClosable ? _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str(),
                                            m_nCloseRequestCount,
                                            m_nCloseRequestIndex).c_str());
}

void TabCtrlForm::UpdateC3State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c3"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c3_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C3")).c_str(),
                                            GetSelectedTitle(pCtrl).c_str(),
                                            static_cast<int>(pCtrl->GetScrollPos().cx),
                                            static_cast<int>(pCtrl->GetScrollRange().cx)).c_str());
}

void TabCtrlForm::UpdateC4State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c4"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c4_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C4")).c_str(),
                                            GetSelectedTitle(pCtrl).c_str(),
                                            TabCtrlLang(pCtrl->IsFocused() ?
                                                _T("STRID_TABCTRL_STATE_FOCUS_YES") : _T("STRID_TABCTRL_STATE_FOCUS_NO")).c_str()).c_str());
}

void TabCtrlForm::UpdateC5State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c5"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c5_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C5")).c_str(),
                                            GetSelectedTitle(pCtrl).c_str(),
                                            TabCtrlLang(pCtrl->IsEnableWheelSwitch() ?
                                                _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str()).c_str());
}

void TabCtrlForm::UpdateC6State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c6"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c6_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    //箭头模式下滚动条应当处于隐藏状态（两者互斥）：
    //关闭箭头后滚动条恢复可用（悬浮时显示），这里按"可用"报告
    const bool bArrowOn = pCtrl->IsEnableOverflowButtons();
    ui::ScrollBar* pScrollBar = pCtrl->GetHScrollBar();
    const bool bBarShown = pCtrl->IsHScrollBarValid() && (!bArrowOn || ((pScrollBar != nullptr) && pScrollBar->IsVisible()));
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C6")).c_str(),
                                            TabCtrlLang(bArrowOn ?
                                                _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str(),
                                            TabCtrlLang(bBarShown ?
                                                _T("STRID_TABCTRL_STATE_SHOWN") : _T("STRID_TABCTRL_STATE_HIDDEN")).c_str(),
                                            static_cast<int>(pCtrl->GetScrollRange().cx),
                                            static_cast<int>(pCtrl->GetScrollPos().cx)).c_str());
}

void TabCtrlForm::UpdateC7State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c7"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c7_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    //列出各标签的实际宽度，便于确认等宽填充与 min/max 约束生效
    DString widths;
    for (size_t i = 0; i < pCtrl->GetItemCount(); ++i) {
        ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(i);
        if (pItem == nullptr) {
            continue;
        }
        if (!widths.empty()) {
            widths += _T(", ");
        }
        widths += ui::StringUtil::Printf(_T("%d"), static_cast<int>(pItem->GetWidth()));
    }
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C7")).c_str(),
                                            TabCtrlLang(pCtrl->IsTabItemJustify() ?
                                                _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str(),
                                            widths.empty() ? TabCtrlLang(_T("STRID_TABCTRL_STATE_NONE")).c_str() : widths.c_str()).c_str());
}

void TabCtrlForm::UpdateC8State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c8"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c8_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C8")).c_str(),
                                            static_cast<int>(pCtrl->GetItemCount()),
                                            m_nNewTabCount).c_str());
}

void TabCtrlForm::UpdateC9State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_c9"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_c9_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C9")).c_str(),
                                            static_cast<int>(pCtrl->GetItemCount()),
                                            GetSelectedTitle(pCtrl).c_str(),
                                            m_c9LastAction.empty() ?
                                                TabCtrlLang(_T("STRID_TABCTRL_STATE_NONE")).c_str() : m_c9LastAction.c_str()).c_str());
}

void TabCtrlForm::UpdateC15State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_anim"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_anim_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    const bool bAnimOn = pCtrl->IsEnableIndicatorAnimation();
    SetToggleText(_T("btn_anim_toggle"), _T("STRID_TABCTRL_BTN_ANIM"), bAnimOn);
    if (ui::Button* pDurationBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_anim_duration")))) {
        pDurationBtn->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_BTN_ANIM_DURATION")).c_str(),
                                                     static_cast<int>(pCtrl->GetIndicatorAnimationDuration())).c_str());
    }
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C15")).c_str(),
                                            TabCtrlLang(bAnimOn ? _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str(),
                                            static_cast<int>(pCtrl->GetIndicatorAnimationDuration()),
                                            GetSelectedTitle(pCtrl).c_str()).c_str());
}

void TabCtrlForm::UpdateC16State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_selected"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_selected_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    ui::TabCtrlItem* pItem = pCtrl->GetSelectedTabItem();
    const bool bFontOn = (pItem != nullptr) && !pItem->GetSelectedTitleFont().empty();
    const bool bBgOn = (pItem != nullptr) &&
                       !pItem->GetSelectStateColor(ui::ControlStateType::kControlStateNormal).empty();
    SetToggleText(_T("btn_selected_font"), _T("STRID_TABCTRL_BTN_SEL_FONT"), bFontOn);
    SetToggleText(_T("btn_selected_bg"), _T("STRID_TABCTRL_BTN_SEL_BG"), bBgOn);
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C16")).c_str(),
                                            TabCtrlLang(bFontOn ? _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str(),
                                            TabCtrlLang(bBgOn ? _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str(),
                                            GetSelectedTitle(pCtrl).c_str()).c_str());
}

void TabCtrlForm::UpdateC17State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_hover"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_hover_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    ui::TabCtrlItem* pItem = pCtrl->GetSelectedTabItem();
    const bool bOn = (pItem != nullptr) && !pItem->GetSelectedHoveredTitleColor().empty();
    SetToggleText(_T("btn_hover_title"), _T("STRID_TABCTRL_BTN_HOVER_TITLE"), bOn);
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C17")).c_str(),
                                            TabCtrlLang(bOn ? _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str(),
                                            GetSelectedTitle(pCtrl).c_str()).c_str());
}

void TabCtrlForm::DeferUpdateC18State()
{
    ui::GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), [this]() {
        UpdateC18State();
        }, 100, 1);
}

void TabCtrlForm::UpdateC18State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_line"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_line_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    const bool bVertical = (pCtrl->GetTabCtrlOrientation() == ui::TabCtrlOrientation::kVertical);
    ui::TabCtrlItem* pItem = pCtrl->GetTabItemAt(0);
    ui::Control* pLine = (pItem != nullptr) ? pItem->GetLineControl() : nullptr;
    //垂直模式下分隔线应变为底部横线（高度约 1 像素），水平模式下为右侧竖线
    const bool bLineHorizontal = (pLine != nullptr) && (pLine->GetHeight() <= 2);
    DString itemSize = TabCtrlLang(_T("STRID_TABCTRL_STATE_NONE"));
    if (pItem != nullptr) {
        itemSize = ui::StringUtil::Printf(_T("%d×%d"), static_cast<int>(pItem->GetWidth()), static_cast<int>(pItem->GetHeight()));
    }
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C18")).c_str(),
                                            TabCtrlLang(bVertical ? _T("STRID_TABCTRL_STATE_DIR_VERT") : _T("STRID_TABCTRL_STATE_DIR_HORZ")).c_str(),
                                            TabCtrlLang(bLineHorizontal ? _T("STRID_TABCTRL_STATE_LINE_HORZ") : _T("STRID_TABCTRL_STATE_LINE_VERT")).c_str(),
                                            itemSize.c_str()).c_str());
}

void TabCtrlForm::UpdateC19State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_bottom_line"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_bottom_line_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    const ui::TabCtrlBottomLineMode mode = pCtrl->GetTabCtrlBottomLineMode();
    DString modeTextId = _T("STRID_TABCTRL_MODE_AUTO");
    if (mode == ui::TabCtrlBottomLineMode::kBreak) {
        modeTextId = _T("STRID_TABCTRL_MODE_BREAK");
    }
    else if (mode == ui::TabCtrlBottomLineMode::kFull) {
        modeTextId = _T("STRID_TABCTRL_MODE_FULL");
    }
    //kAuto 仅在绘制选中背景时断开；当前测试栏没有选中背景，所以 auto 表现为连续
    const bool bBreak = (mode == ui::TabCtrlBottomLineMode::kBreak) ||
                        ((mode == ui::TabCtrlBottomLineMode::kAuto) && pCtrl->IsSelectedTabItemShowBg());
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C19")).c_str(),
                                            TabCtrlLang(modeTextId).c_str(),
                                            TabCtrlLang(bBreak ? _T("STRID_TABCTRL_STATE_LINE_BREAK") : _T("STRID_TABCTRL_STATE_LINE_CONTINUOUS")).c_str()).c_str());
}

void TabCtrlForm::UpdateC20State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_focus"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_focus_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    //焦点可能落在标签项或关闭按钮上，需要沿父链判断是否在本标签栏内
    bool bFocusWithin = false;
    ui::Window* pWindow = pCtrl->GetWindow();
    ui::Control* pFocus = (pWindow != nullptr) ? pWindow->GetFocusControl() : nullptr;
    for (ui::Control* pControl = pFocus; pControl != nullptr; pControl = pControl->GetParent()) {
        if (pControl == pCtrl) {
            bFocusWithin = true;
            break;
        }
    }
    const bool bEnabled = pCtrl->IsEnabled();
    const bool bFocusRect = bFocusWithin && bEnabled && pCtrl->IsShowFocusedRect() &&
                            (pCtrl->GetSelectedTabItem() != nullptr);
    SetToggleText(_T("btn_disable_tabctrl"), _T("STRID_TABCTRL_BTN_DISABLE"), !bEnabled);
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C20")).c_str(),
                                            TabCtrlLang(bFocusWithin ? _T("STRID_TABCTRL_STATE_FOCUS_YES") : _T("STRID_TABCTRL_STATE_FOCUS_NO")).c_str(),
                                            TabCtrlLang(bEnabled ? _T("STRID_TABCTRL_SWITCH_OFF") : _T("STRID_TABCTRL_SWITCH_ON")).c_str(),
                                            TabCtrlLang(bFocusRect ? _T("STRID_TABCTRL_STATE_SHOWN") : _T("STRID_TABCTRL_STATE_HIDDEN")).c_str()).c_str());
}

void TabCtrlForm::UpdateC21State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_badge_float"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_badge_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    ui::TabCtrlItem* pFirst = pCtrl->GetTabItemAt(0);
    const bool bFloat = (pFirst != nullptr) && pFirst->IsBadgeFloat();
    const int64_t nCount = (pFirst != nullptr) ? pFirst->GetBadgeCount() : 0;
    SetToggleText(_T("btn_badge_float"), _T("STRID_TABCTRL_BTN_BADGE_FLOAT"), bFloat);
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C21")).c_str(),
                                            TabCtrlLang(bFloat ? _T("STRID_TABCTRL_SWITCH_ON") : _T("STRID_TABCTRL_SWITCH_OFF")).c_str(),
                                            static_cast<int>(nCount),
                                            GetSelectedTitle(pCtrl).c_str()).c_str());
}

void TabCtrlForm::UpdateC22State()
{
    ui::TabCtrl* pCtrl = FindTabCtrl(_T("tab_ctrl_close"));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_close_state")));
    if ((pCtrl == nullptr) || (pLabel == nullptr)) {
        return;
    }
    ui::TabCtrlItem* pFirst = pCtrl->GetTabItemAt(0);
    const ui::TabCtrlCloseButtonShowMode mode = (pFirst != nullptr) ? pFirst->GetCloseButtonShowMode()
                                                                    : ui::TabCtrlCloseButtonShowMode::kAuto;
    DString modeTextId = _T("STRID_TABCTRL_MODE_CLOSE_AUTO");
    if (mode == ui::TabCtrlCloseButtonShowMode::kHover) {
        modeTextId = _T("STRID_TABCTRL_MODE_CLOSE_HOVER");
    }
    else if (mode == ui::TabCtrlCloseButtonShowMode::kAlways) {
        modeTextId = _T("STRID_TABCTRL_MODE_CLOSE_ALWAYS");
    }
    //kAuto 下选中项常显关闭按钮；kHover 下选中项也只在悬停时显示
    const bool bSelectedAlways = (mode != ui::TabCtrlCloseButtonShowMode::kHover);
    pLabel->SetText(ui::StringUtil::Printf(TabCtrlLang(_T("STRID_TABCTRL_STATE_C22")).c_str(),
                                            TabCtrlLang(modeTextId).c_str(),
                                            TabCtrlLang(bSelectedAlways ? _T("STRID_TABCTRL_STATE_CLOSE_ALWAYS_SHOWN") : _T("STRID_TABCTRL_STATE_CLOSE_HOVER_ONLY")).c_str()).c_str());
}
