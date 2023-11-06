#include "stdafx.h"
#include "main_form.h"
#include "provider.h"

const std::wstring MainForm::kClassName = L"VirtualListBox";


MainForm::MainForm()
= default;


MainForm::~MainForm()
= default;

std::wstring MainForm::GetSkinFolder()
{
	return L"virtual_list_box";
}

std::wstring MainForm::GetSkinFile()
{
	return L"main.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
	return kClassName;
}

void MainForm::OnInitWindow()
{
	m_EditColumn = dynamic_cast<ui::RichEdit*>(FindControl(L"edit_column"));
	m_CheckBoxItemCenter = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_item_center"));
	m_EditTotal = dynamic_cast<ui::RichEdit*>(FindControl(L"edit_total"));
	m_pTileList = dynamic_cast<ui::VirtualListBox*>(FindControl(L"list"));
	m_OptionColumnFix = dynamic_cast<ui::Option*>(FindControl(L"option_column_fix"));
	m_EditUpdate = dynamic_cast<ui::RichEdit*>(FindControl(L"edit_update"));
	m_EditTaskName = dynamic_cast<ui::RichEdit*>(FindControl(L"edit_task_name"));
	m_EditDelete = dynamic_cast<ui::RichEdit*>(FindControl(L"edit_delete"));
	m_EditChildMarginX = dynamic_cast<ui::RichEdit*>(FindControl(L"edit_child_margin_x"));
	m_EditChildMarginY = dynamic_cast<ui::RichEdit*>(FindControl(L"edit_child_margin_y"));

	GetRoot()->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&MainForm::OnClicked, this, std::placeholders::_1));

	// 设置提供者
	m_DataProvider = new Provider;
	m_pTileList->SetDataProvider(m_DataProvider);

	ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
	if ((layoutType != ui::LayoutType::VirtualHTileLayout) &&
	    (layoutType != ui::LayoutType::VirtualVTileLayout)){
		//隐藏列的设置，其他的不支持设置列
		if (m_EditColumn != nullptr) {
			m_EditColumn->SetEnabled(false);
			m_EditColumn->SetText(L"  ");
		}
	}

	if (layoutType == ui::LayoutType::VirtualHTileLayout) {
		m_OptionColumnFix->SetText(L"固定行数");
	}
	else if (layoutType == ui::LayoutType::VirtualVTileLayout) {
		m_OptionColumnFix->SetText(L"固定列数");
	}
	else if (layoutType == ui::LayoutType::VirtualHLayout) {
		m_OptionColumnFix->SetText(L"居中设置");
	}
	else if (layoutType == ui::LayoutType::VirtualVLayout) {
		m_OptionColumnFix->SetText(L"居中设置");
	}
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

bool MainForm::OnClicked(const ui::EventArgs& args)
{
	auto sName = args.pSender->GetName();
	if (sName == L"btn_set_total")
	{
		if (!m_EditChildMarginX->GetText().empty())	{
			m_pTileList->SetAttribute(L"child_margin_x", m_EditChildMarginX->GetText());
		}
		if (!m_EditChildMarginY->GetText().empty()) {
			m_pTileList->SetAttribute(L"child_margin_y", m_EditChildMarginY->GetText());
		}
		if (m_OptionColumnFix->IsSelected()) {
			//固定列数/行数
			ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
			if (layoutType == ui::LayoutType::VirtualVTileLayout) {
				m_pTileList->SetAttribute(L"columns", m_EditColumn->GetText());
			}
			if (layoutType == ui::LayoutType::VirtualHTileLayout) {
				m_pTileList->SetAttribute(L"rows", m_EditColumn->GetText());
			}
			if ((layoutType == ui::LayoutType::VirtualHTileLayout) ||
			    (layoutType == ui::LayoutType::VirtualHLayout)){
				//水平布局
				if (m_CheckBoxItemCenter->IsSelected()) {
					//列表居中
					m_pTileList->SetAttribute(L"height", L"auto");
					m_pTileList->SetAttribute(L"valign", L"center");
					m_pTileList->SetAttribute(L"width", L"stretch");
				}
				else {
					m_pTileList->SetAttribute(L"height", L"auto");
					m_pTileList->SetAttribute(L"width", L"stretch");
				}
			}
			else {
				//垂直布局
				if (m_CheckBoxItemCenter->IsSelected()) {
					//列表居中
					m_pTileList->SetAttribute(L"width", L"auto");
					m_pTileList->SetAttribute(L"halign", L"center");
				}
				else {
					m_pTileList->SetAttribute(L"width", L"stretch");
				}
			}
		}
		else {
			//自动计算列数
			m_pTileList->SetAttribute(L"width", L"stretch");
			ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
			if (layoutType == ui::LayoutType::VirtualVTileLayout) {
				m_pTileList->SetAttribute(L"columns", L"auto");
			}
			if (layoutType == ui::LayoutType::VirtualHTileLayout) {
				m_pTileList->SetAttribute(L"rows", L"auto");
			}
		}

		int nTotal = _ttoi(m_EditTotal->GetText().c_str());
		if (nTotal > 0) {
			m_DataProvider->SetTotal(nTotal);
		}
	}
	else if (sName == L"btn_update") {
		size_t nIndex = _ttoi(m_EditUpdate->GetText().c_str()) - 1;
		ASSERT(nIndex < m_DataProvider->GetElementCount());
		m_DataProvider->ChangeTaskName(nIndex, m_EditTaskName->GetText());
	}
	else if (sName == L"btn_delete") {
		size_t nIndex = _ttoi(m_EditDelete->GetText().c_str()) - 1;
		ASSERT(nIndex < m_DataProvider->GetElementCount());
		m_DataProvider->RemoveTask(nIndex);
	}
	m_pTileList->SetFocus();
	return true;
}
