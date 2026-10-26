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
    m_nTabIndex(1)
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

ui::TabCtrlItem* TabCtrlForm::CreateTabItem(ui::TabCtrl* pTabCtrl, const DString& title, bool bFixed, int32_t nFixedWidth)
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
        pItem->SetMinWidth(nFixedWidth, false);
        pItem->SetMaxWidth(nFixedWidth, false);
        pItem->SetFixedWidth(ui::UiFixedInt(nFixedWidth), true, false);
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
    pTabCtrl->AddItem(pItem);
    return pItem;
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

    RefreshAllState();
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
