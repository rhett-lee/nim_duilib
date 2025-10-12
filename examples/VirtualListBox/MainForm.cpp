#include "MainForm.h"
#include "DataProvider.h"

MainForm::MainForm():
    m_pTileList(nullptr),
    m_pDataProvider(nullptr),
    m_pEditColumn(nullptr),
    m_pEditTotal(nullptr),
    m_pOptionColumnFix(nullptr),
    m_pEditUpdate(nullptr),
    m_pEditTaskName(nullptr),
    m_pEditDelete(nullptr),
    m_pEditChildMarginX(nullptr),
    m_pEditChildMarginY(nullptr)
{

}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("virtual_list_box");
}

DString MainForm::GetSkinFile()
{
    return _T("main.xml");
}

void MainForm::OnInitWindow()
{
    m_pEditColumn = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_column")));
    m_pEditTotal = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_total")));
    m_pTileList = dynamic_cast<ui::VirtualListBox*>(FindControl(_T("list")));
    m_pOptionColumnFix = dynamic_cast<ui::Option*>(FindControl(_T("option_column_fix")));
    m_pEditUpdate = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_update")));
    m_pEditTaskName = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_task_name")));
    m_pEditDelete = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_delete")));
    m_pEditChildMarginX = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_child_margin_x")));
    m_pEditChildMarginY = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_child_margin_y")));

    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MainForm::OnClicked, this, std::placeholders::_1));

    // 设置提供者
    m_pDataProvider = new DataProvider;
    m_pTileList->SetDataProvider(m_pDataProvider);

    const ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
    if ((layoutType != ui::LayoutType::VirtualHTileLayout) &&
        (layoutType != ui::LayoutType::VirtualVTileLayout)){
        //隐藏列的设置，其他的不支持设置列
        if (m_pEditColumn != nullptr) {
            m_pEditColumn->SetEnabled(false);
            m_pEditColumn->SetText(_T("  "));
        }
    }

    if (layoutType == ui::LayoutType::VirtualHTileLayout) {
        m_pOptionColumnFix->SetText(_T("固定行数"));
    }
    else if (layoutType == ui::LayoutType::VirtualVTileLayout) {
        m_pOptionColumnFix->SetText(_T("固定列数"));
    }
    else {
        m_pOptionColumnFix->SetEnabled(false);
        ui::Control* pControl = FindControl(_T("option_column_auto"));
        if (pControl) {
            pControl->SetEnabled(false);
        }
        pControl = FindControl(_T("label_column_row"));
        if (pControl) {
            pControl->SetEnabled(false);
        }
    }

    ui::Option* pOptionAlign1 = dynamic_cast<ui::Option*>(FindControl(_T("option_align1")));
    ui::Option* pOptionAlign2 = dynamic_cast<ui::Option*>(FindControl(_T("option_align2")));
    ui::Option* pOptionAlign3 = dynamic_cast<ui::Option*>(FindControl(_T("option_align3")));
    if ((pOptionAlign1 != nullptr) && (pOptionAlign2 != nullptr) && (pOptionAlign3 != nullptr)) {
        if ((layoutType == ui::LayoutType::VirtualHTileLayout) || (layoutType == ui::LayoutType::VirtualHLayout)) {
            //水平布局
            pOptionAlign1->SetText(_T("靠上对齐(top)"));
            pOptionAlign2->SetText(_T("居中对齐(center)"));
            pOptionAlign3->SetText(_T("靠下对齐(bottom)"));
            pOptionAlign2->Selected(true, true);

            //相应对齐方式变化，动态调整控件的对齐方式
            pOptionAlign1->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_valign"), _T("top"));
                    m_pTileList->Refresh();
                }
                return true;
                });
            pOptionAlign2->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_valign"), _T("center"));
                    m_pTileList->Refresh();
                }
                return true;
                });
            pOptionAlign3->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_valign"), _T("bottom"));
                    m_pTileList->Refresh();
                }
                return true;
                });
        }
        else {
            //垂直布局
            pOptionAlign1->SetText(_T("靠左对齐(left)"));
            pOptionAlign2->SetText(_T("居中对齐(center)"));
            pOptionAlign3->SetText(_T("靠右对齐(right)"));
            pOptionAlign2->Selected(true, true);

            //相应对齐方式变化，动态调整控件的对齐方式
            pOptionAlign1->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_halign"), _T("left"));
                    m_pTileList->Refresh();
                }
                return true;
                });
            pOptionAlign2->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_halign"), _T("center"));
                    m_pTileList->Refresh();
                }
                return true;
                });
            pOptionAlign3->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_halign"), _T("right"));
                    m_pTileList->Refresh();
                }
                return true;
                });
        }
    }
    //显示当前容器类型, 并更新默认属性
    ui::Label* pListType = dynamic_cast<ui::Label*>(FindControl(_T("list_box_type")));
    if (pListType != nullptr) {
        if (m_pTileList->GetType() == DUI_CTR_VIRTUAL_HLISTBOX) {
            pListType->SetText(_T("VirtualHListBox"));
        }
        else if (m_pTileList->GetType() == DUI_CTR_VIRTUAL_VLISTBOX) {
            pListType->SetText(_T("VirtualVListBox"));
        }
        else if (m_pTileList->GetType() == DUI_CTR_VIRTUAL_HTILE_LISTBOX) {
            pListType->SetText(_T("VirtualHTileListBox"));
        }
        else if (m_pTileList->GetType() == DUI_CTR_VIRTUAL_VTILE_LISTBOX) {
            pListType->SetText(_T("VirtualVTileListBox"));
        }
    }

    //更新子项间隙的状态
    if (layoutType == ui::LayoutType::VirtualHLayout) {
        if (m_pEditChildMarginY != nullptr) {
            m_pEditChildMarginY->SetEnabled(false);
        }
        ui::Control* pControl = FindControl(_T("label_child_margin_y"));
        if (pControl) {
            pControl->SetEnabled(false);
        }
    }
    else if (layoutType == ui::LayoutType::VirtualVLayout) {
        if (m_pEditChildMarginX != nullptr) {
            m_pEditChildMarginX->SetEnabled(false);
        }
        ui::Control* pControl = FindControl(_T("label_child_margin_x"));
        if (pControl) {
            pControl->SetEnabled(false);
        }
    }
}

bool MainForm::OnClicked(const ui::EventArgs& args)
{
    auto sName = args.GetSender()->GetName();
    if (sName == _T("btn_set_total"))
    {
        if (!m_pEditChildMarginX->GetText().empty())    {
            m_pTileList->SetAttribute(_T("child_margin_x"), m_pEditChildMarginX->GetText());
        }
        if (!m_pEditChildMarginY->GetText().empty()) {
            m_pTileList->SetAttribute(_T("child_margin_y"), m_pEditChildMarginY->GetText());
        }
        if (m_pOptionColumnFix->IsSelected()) {
            //固定列数/行数
            ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
            if (layoutType == ui::LayoutType::VirtualVTileLayout) {
                m_pTileList->SetAttribute(_T("columns"), m_pEditColumn->GetText());
            }
            if (layoutType == ui::LayoutType::VirtualHTileLayout) {
                m_pTileList->SetAttribute(_T("rows"), m_pEditColumn->GetText());
            }            
        }
        else {
            //自动计算列数
            m_pTileList->SetAttribute(_T("width"), _T("stretch"));
            ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
            if (layoutType == ui::LayoutType::VirtualVTileLayout) {
                m_pTileList->SetAttribute(_T("columns"), _T("auto"));
            }
            if (layoutType == ui::LayoutType::VirtualHTileLayout) {
                m_pTileList->SetAttribute(_T("rows"), _T("auto"));
            }
        }

        int nTotal = ui::StringUtil::StringToInt32(m_pEditTotal->GetText().c_str());
        if (nTotal > 0) {
            m_pDataProvider->SetTotal(nTotal);
        }
    }
    else if (sName == _T("btn_update")) {
        size_t nIndex = (size_t)ui::StringUtil::StringToInt32(m_pEditUpdate->GetText().c_str());
        ASSERT(nIndex < m_pDataProvider->GetElementCount());
        m_pDataProvider->ChangeTaskName(nIndex, m_pEditTaskName->GetText());
    }
    else if (sName == _T("btn_delete")) {
        size_t nIndex = (size_t)ui::StringUtil::StringToInt32(m_pEditDelete->GetText().c_str());
        ASSERT(nIndex < m_pDataProvider->GetElementCount());
        m_pDataProvider->RemoveTask(nIndex);
    }
    m_pTileList->SetFocus();
    return true;
}
