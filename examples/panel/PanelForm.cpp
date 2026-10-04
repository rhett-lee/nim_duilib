#include "PanelForm.h"

namespace {
/** Panel 控件名称与标题语言 ID 的对照表
* （Panel 控件只支持 title 属性、不支持 text_id，语言切换时需通过 C++ 重设标题）
*/
struct PanelTitleItem
{
    LPCTSTR sControlName;
    LPCTSTR sTitleTextId;
};

const PanelTitleItem kPanelTitleItems[] = {
    { _T("panel_basic"),         _T("STRID_PANEL_TITLE_BASIC") },
    { _T("panel_collapsible"),   _T("STRID_PANEL_TITLE_COLLAPSIBLE") },
    { _T("panel_nested"),        _T("STRID_PANEL_TITLE_NESTED") },
    { _T("panel_nested_inner"),  _T("STRID_PANEL_TITLE_NESTED_INNER") },
    { _T("panel_nested_hlayout"),_T("STRID_PANEL_TITLE_NESTED_HLAYOUT") },
    { _T("panel_v_demo"),        _T("STRID_PANEL_TITLE_V") },
    { _T("panel_h_demo"),        _T("STRID_PANEL_TITLE_H") },
    { _T("panel_stretch_a"),     _T("STRID_PANEL_TITLE_STRETCH_A") },
    { _T("panel_stretch_b"),     _T("STRID_PANEL_TITLE_STRETCH_B") },
    { _T("panel_visible"),       _T("STRID_PANEL_TITLE_VISIBLE") },
    { _T("panel_arrow_only"),    _T("STRID_PANEL_TITLE_ARROW_ONLY") },
    { _T("panel_accordion"),     _T("STRID_PANEL_TITLE_ACCORDION") },
    { _T("panel_acc_profile"),   _T("STRID_PANEL_TITLE_PROFILE") },
    { _T("panel_acc_notify"),    _T("STRID_PANEL_TITLE_NOTIFY") },
    { _T("panel_acc_about"),     _T("STRID_PANEL_TITLE_ABOUT") },
    { _T("panel_custom_arrow"),  _T("STRID_PANEL_TITLE_CUSTOM_ARROW") },
    { _T("panel_arrow_left"),    _T("STRID_PANEL_TITLE_ARROW_LEFT") },
    { _T("panel_slot"),          _T("STRID_PANEL_TITLE_SLOT") },
    { _T("panel_veto"),          _T("STRID_PANEL_TITLE_VETO") },
};
} //namespace

PanelForm::PanelForm():
    m_pCppPanel(nullptr),
    m_pStatusLabel(nullptr),
    m_pHiddenChildLabel(nullptr),
    m_nCppTitleModified(0)
{
}

PanelForm::~PanelForm()
{
}

DString PanelForm::GetSkinFolder()
{
    return _T("panel");
}

DString PanelForm::GetSkinFile()
{
    return _T("panel.xml");
}

void PanelForm::SetStatusText(const DString& strText)
{
    if (m_pStatusLabel != nullptr) {
        m_pStatusLabel->SetText(strText);
    }
}

void PanelForm::ApplyLocalizedPanelTitles()
{
    for (const PanelTitleItem& item : kPanelTitleItems) {
        ui::Control* pControl = FindControl(item.sControlName);
        DString sTitle = ui::GlobalManager::GetTextById(item.sTitleTextId);
        ui::PanelVBox* pPanelV = dynamic_cast<ui::PanelVBox*>(pControl);
        if (pPanelV != nullptr) {
            pPanelV->SetTitle(sTitle);
            continue;
        }
        ui::PanelHBox* pPanelH = dynamic_cast<ui::PanelHBox*>(pControl);
        if (pPanelH != nullptr) {
            pPanelH->SetTitle(sTitle);
        }
    }

    //C++ 控制面板：默认标题与“已修改 N 次”的标题都需要跟随当前语言
    if (m_pCppPanel != nullptr) {
        if (m_nCppTitleModified > 0) {
            m_pCppPanel->SetTitle(ui::StringUtil::Printf(
                ui::GlobalManager::GetTextById(_T("STRID_PANEL_TITLE_CPP_MODIFIED_FMT")).c_str(),
                m_nCppTitleModified));
        }
        else {
            m_pCppPanel->SetTitle(ui::GlobalManager::GetTextById(_T("STRID_PANEL_TITLE_CPP")));
        }
    }
}

bool PanelForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();
    //Panel 标题不支持 text_id，语言切换后手动按当前语言刷新
    ApplyLocalizedPanelTitles();
    return bRet;
}

void PanelForm::OnInitWindow()
{
    m_pStatusLabel = dynamic_cast<ui::Label*>(FindControl(_T("lbl_status")));

    //C++ 控制的可折叠面板：监听折叠/展开事件
    m_pCppPanel = dynamic_cast<ui::PanelVBox*>(FindControl(_T("panel_cpp")));
    if (m_pCppPanel != nullptr) {
        //通过 C++ 代码设置 300ms 折叠/展开动画（XML 中未配置 collapse_anim）
        m_pCppPanel->SetCollapseAnimMillSeconds(300);
        ui::PanelVBox* pPanel = m_pCppPanel; //捕获弱引用，避免面板销毁后访问
        std::weak_ptr<ui::WeakFlag> weakFlag = pPanel->GetWeakFlag();
        pPanel->AttachExpand([this, weakFlag](const ui::EventArgs& args) {
            if (weakFlag.expired()) {
                return true;
            }
            SetStatusText(ui::GlobalManager::GetTextById(_T("STRID_PANEL_STATUS_CPP_EXPANDED")));
            return true;
        });
        pPanel->AttachCollapse([this, weakFlag](const ui::EventArgs& args) {
            if (weakFlag.expired()) {
                return true;
            }
            SetStatusText(ui::GlobalManager::GetTextById(_T("STRID_PANEL_STATUS_CPP_COLLAPSED")));
            return true;
        });
    }

    //通过 C++ 代码切换折叠状态
    ui::Button* pToggleBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_toggle")));
    if (pToggleBtn != nullptr) {
        pToggleBtn->AttachClick([this](const ui::EventArgs& args) {
            if (m_pCppPanel != nullptr) {
                m_pCppPanel->SetCollapsed(!m_pCppPanel->IsCollapsed());
            }
            return true;
        });
    }

    //无动画切换：SetCollapsed 第三个参数 bPlayAnim=false，立即落到最终状态
    ui::Button* pToggleNoAnimBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_toggle_noanim")));
    if (pToggleNoAnimBtn != nullptr) {
        pToggleNoAnimBtn->AttachClick([this](const ui::EventArgs& args) {
            if (m_pCppPanel != nullptr) {
                m_pCppPanel->SetCollapsed(!m_pCppPanel->IsCollapsed(), true, false);
            }
            return true;
        });
    }

    //通过 C++ 代码动态修改标题
    ui::Button* pTitleBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_set_title")));
    if (pTitleBtn != nullptr) {
        pTitleBtn->AttachClick([this](const ui::EventArgs& args) {
            if (m_pCppPanel != nullptr) {
                ++m_nCppTitleModified;
                m_pCppPanel->SetTitle(ui::StringUtil::Printf(
                    ui::GlobalManager::GetTextById(_T("STRID_PANEL_TITLE_CPP_MODIFIED_FMT")).c_str(),
                    m_nCppTitleModified));
            }
            return true;
        });
    }

    //回归测试：用户控制的子标签显隐，与面板折叠状态互不影响
    m_pHiddenChildLabel = dynamic_cast<ui::Label*>(FindControl(_T("lbl_hidden_child")));
    ui::Button* pHiddenBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_toggle_hidden")));
    if (pHiddenBtn != nullptr) {
        pHiddenBtn->AttachClick([this](const ui::EventArgs& args) {
            if (m_pHiddenChildLabel != nullptr) {
                m_pHiddenChildLabel->SetVisible(!m_pHiddenChildLabel->IsVisible());
            }
            return true;
        });
    }

    //标题栏槽位按钮：证明槽位内的交互控件可正常响应，且不会触发面板折叠
    ui::Button* pSlotSettings = dynamic_cast<ui::Button*>(FindControl(_T("btn_slot_settings")));
    if (pSlotSettings != nullptr) {
        pSlotSettings->AttachClick([this](const ui::EventArgs& args) {
            SetStatusText(ui::GlobalManager::GetTextById(_T("STRID_PANEL_STATUS_SLOT_SETTINGS")));
            return true;
        });
    }
    ui::Button* pSlotRefresh = dynamic_cast<ui::Button*>(FindControl(_T("btn_slot_refresh")));
    if (pSlotRefresh != nullptr) {
        pSlotRefresh->AttachClick([this](const ui::EventArgs& args) {
            SetStatusText(ui::GlobalManager::GetTextById(_T("STRID_PANEL_STATUS_SLOT_REFRESH")));
            return true;
        });
    }

    //折叠前可取消事件：未勾选『允许折叠』时返回 false 取消折叠
    ui::PanelVBox* pVetoPanel = dynamic_cast<ui::PanelVBox*>(FindControl(_T("panel_veto")));
    ui::CheckBox* pAllowCollapse = dynamic_cast<ui::CheckBox*>(FindControl(_T("chk_allow_collapse")));
    if (pVetoPanel != nullptr) {
        pVetoPanel->AttachCollapsing([this, pAllowCollapse](const ui::EventArgs& args) {
            bool bAllow = (pAllowCollapse != nullptr) && pAllowCollapse->IsSelected();
            if (!bAllow) {
                SetStatusText(ui::GlobalManager::GetTextById(_T("STRID_PANEL_STATUS_VETO_CANCELLED")));
            }
            return bAllow;
        });
        pVetoPanel->AttachCollapse([this](const ui::EventArgs& args) {
            SetStatusText(ui::GlobalManager::GetTextById(_T("STRID_PANEL_STATUS_VETO_COLLAPSED")));
            return true;
        });
    }

    //XML 中 Panel 的 title 属性为中文兜底值，窗口创建后按当前语言刷新一次
    ApplyLocalizedPanelTitles();
}
