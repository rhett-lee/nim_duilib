#include "main_form.h"
#include "provider.h"

const std::wstring MainForm::kClassName = _T("VirtualListBox");


MainForm::MainForm()
= default;


MainForm::~MainForm()
= default;

std::wstring MainForm::GetSkinFolder()
{
    return _T("virtual_list_box");
}

std::wstring MainForm::GetSkinFile()
{
    return _T("main.xml");
}

std::wstring MainForm::GetWindowClassName() const
{
    return kClassName;
}

void MainForm::OnInitWindow()
{
    m_EditColumn = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_column")));
    m_CheckBoxItemCenter = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_item_center")));
    m_EditTotal = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_total")));
    m_pTileList = dynamic_cast<ui::VirtualListBox*>(FindControl(_T("list")));
    m_OptionColumnFix = dynamic_cast<ui::Option*>(FindControl(_T("option_column_fix")));
    m_EditUpdate = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_update")));
    m_EditTaskName = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_task_name")));
    m_EditDelete = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_delete")));
    m_EditChildMarginX = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_child_margin_x")));
    m_EditChildMarginY = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_child_margin_y")));

    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MainForm::OnClicked, this, std::placeholders::_1));

    // 设置提供者
    m_DataProvider = new Provider;
    m_pTileList->SetDataProvider(m_DataProvider);

    ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
    if ((layoutType != ui::LayoutType::VirtualHTileLayout) &&
        (layoutType != ui::LayoutType::VirtualVTileLayout)){
        //隐藏列的设置，其他的不支持设置列
        if (m_EditColumn != nullptr) {
            m_EditColumn->SetEnabled(false);
            m_EditColumn->SetText(_T("  "));
        }
    }

    if (layoutType == ui::LayoutType::VirtualHTileLayout) {
        m_OptionColumnFix->SetText(_T("固定行数"));
    }
    else if (layoutType == ui::LayoutType::VirtualVTileLayout) {
        m_OptionColumnFix->SetText(_T("固定列数"));
    }
    else if (layoutType == ui::LayoutType::VirtualHLayout) {
        m_OptionColumnFix->SetText(_T("居中设置"));
    }
    else if (layoutType == ui::LayoutType::VirtualVLayout) {
        m_OptionColumnFix->SetText(_T("居中设置"));
    }
}

void MainForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMessage(0L);
}

bool MainForm::OnClicked(const ui::EventArgs& args)
{
    auto sName = args.pSender->GetName();
    if (sName == _T("btn_set_total"))
    {
        if (!m_EditChildMarginX->GetText().empty())    {
            m_pTileList->SetAttribute(_T("child_margin_x"), m_EditChildMarginX->GetText());
        }
        if (!m_EditChildMarginY->GetText().empty()) {
            m_pTileList->SetAttribute(_T("child_margin_y"), m_EditChildMarginY->GetText());
        }
        if (m_OptionColumnFix->IsSelected()) {
            //固定列数/行数
            ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
            if (layoutType == ui::LayoutType::VirtualVTileLayout) {
                m_pTileList->SetAttribute(_T("columns"), m_EditColumn->GetText());
            }
            if (layoutType == ui::LayoutType::VirtualHTileLayout) {
                m_pTileList->SetAttribute(_T("rows"), m_EditColumn->GetText());
            }
            if ((layoutType == ui::LayoutType::VirtualHTileLayout) ||
                (layoutType == ui::LayoutType::VirtualHLayout)){
                //水平布局
                if (m_CheckBoxItemCenter->IsSelected()) {
                    //列表居中
                    m_pTileList->SetAttribute(_T("height"), _T("auto"));
                    m_pTileList->SetAttribute(_T("valign"), _T("center"));
                    m_pTileList->SetAttribute(_T("width"), _T("stretch"));
                }
                else {
                    m_pTileList->SetAttribute(_T("height"), _T("auto"));
                    m_pTileList->SetAttribute(_T("width"), _T("stretch"));
                }
            }
            else {
                //垂直布局
                if (m_CheckBoxItemCenter->IsSelected()) {
                    //列表居中
                    m_pTileList->SetAttribute(_T("width"), _T("auto"));
                    m_pTileList->SetAttribute(_T("halign"), _T("center"));
                }
                else {
                    m_pTileList->SetAttribute(_T("width"), _T("stretch"));
                }
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

        int nTotal = _ttoi(m_EditTotal->GetText().c_str());
        if (nTotal > 0) {
            m_DataProvider->SetTotal(nTotal);
        }
    }
    else if (sName == _T("btn_update")) {
        size_t nIndex = _ttoi(m_EditUpdate->GetText().c_str()) - 1;
        ASSERT(nIndex < m_DataProvider->GetElementCount());
        m_DataProvider->ChangeTaskName(nIndex, m_EditTaskName->GetText());
    }
    else if (sName == _T("btn_delete")) {
        size_t nIndex = _ttoi(m_EditDelete->GetText().c_str()) - 1;
        ASSERT(nIndex < m_DataProvider->GetElementCount());
        m_DataProvider->RemoveTask(nIndex);
    }
    m_pTileList->SetFocus();
    return true;
}
