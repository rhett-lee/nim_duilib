#include "MainForm.h"
#include "MainThread.h"

const std::wstring MainForm::kClassName = L"MainForm";

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

std::wstring MainForm::GetSkinFolder()
{
	return L"list_ctrl";
}

std::wstring MainForm::GetSkinFile()
{
	return L"list_ctrl.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
	return kClassName;
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    PostQuitMessage(0L);
    return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

void MainForm::OnInitWindow()
{
	ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(L"list_ctrl"));
	ASSERT(pListCtrl != nullptr);
	if (pListCtrl == nullptr) {
		return;
	}
	//填充数据
	InsertItemData(200, 3);

	//表头高度控制
	ui::RichEdit* pHeaderHeightEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"header_height_edit"));
	if (pHeaderHeightEdit != nullptr) {
		pHeaderHeightEdit->SetText(ui::StringHelper::Printf(L"%d", pListCtrl->GetHeaderHeight()));
		pHeaderHeightEdit->AttachTextChange([this, pHeaderHeightEdit, pListCtrl](const ui::EventArgs&) {
			int32_t height = _wtoi(pHeaderHeightEdit->GetText().c_str());
			if (height >= 0) {
				pListCtrl->SetHeaderHeight(height, false);
			}
			return true;
			});
	}

	//表格每行高度控制
	ui::RichEdit* pItemHeightEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"list_item_height_edit"));
	if (pItemHeightEdit != nullptr) {
		pItemHeightEdit->SetText(ui::StringHelper::Printf(L"%d", pListCtrl->GetDataItemHeight()));
		pItemHeightEdit->AttachTextChange([this, pItemHeightEdit, pListCtrl](const ui::EventArgs&) {
			int32_t height = _wtoi(pItemHeightEdit->GetText().c_str());
			if (height >= 0) {
				pListCtrl->SetDataItemHeight(height, false);
			}
			return true;
			});
	}

	//列控制
	ui::Combo* pColumnCombo = dynamic_cast<ui::Combo*>(FindControl(L"column_combo"));
	if (pColumnCombo != nullptr) {
		//填充列数据
		size_t nColumnCount = pListCtrl->GetColumnCount();
		for (size_t i = 0; i < nColumnCount; ++i) {
			ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumn(i);
			ASSERT(pHeaderItem != nullptr);
			if (pHeaderItem != nullptr) {
				size_t nItem = pColumnCombo->AddTextItem(pHeaderItem->GetText());
				ASSERT(nItem != ui::Box::InvalidIndex);
				//保持列的ID
				pColumnCombo->SetItemData(nItem, pHeaderItem->GetColomnId());
			}
		}
		//挂载事件
		pColumnCombo->AttachSelect([this, pColumnCombo](const ui::EventArgs& args) {
			size_t nCurSel = args.wParam;
			size_t nColumnId = pColumnCombo->GetItemData(nCurSel);
			OnColumnChanged(nColumnId);
			return true;
			});

		pColumnCombo->SetCurSel(0);
		OnColumnChanged(pColumnCombo->GetItemData(0));
	}

	ui::CheckBox* pColumnShow = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show"));
	ui::CheckBox* pColumnWidth = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_width"));
	ui::CheckBox* pColumnSort = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_sort"));
	ui::CheckBox* pColumnIcon = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_icon_at_top"));
	ui::CheckBox* pColumnHeaderCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_header_checkbox"));

	ui::Option* pColumnHeaderTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_left"));
	ui::Option* pColumnHeaderTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_center"));
	ui::Option* pColumnHeaderTextAlignRight = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_right"));

	//实现显示该列
	auto OnColumnShowHide = [this, pColumnCombo, pListCtrl](bool bColumnVisible) {
			size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
			ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
			ASSERT(pHeaderItem != nullptr);
			if (pHeaderItem != nullptr) {
				pHeaderItem->SetColumnVisible(bColumnVisible);
			}
		};
	pColumnShow->AttachSelect([this, OnColumnShowHide](const ui::EventArgs&) {
		OnColumnShowHide(true);
		return true;
		});
	pColumnShow->AttachUnSelect([this, OnColumnShowHide](const ui::EventArgs&) {
		OnColumnShowHide(false);
		return true;
		});

	//是否支持列宽调整
	auto OnColumnResizeable = [this, pColumnCombo, pListCtrl](bool bResizeable) {
			size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
			ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
			ASSERT(pHeaderItem != nullptr);
			if (pHeaderItem != nullptr) {
				pHeaderItem->SetColumnResizeable(bResizeable);
			}
		};
	pColumnWidth->AttachSelect([this, OnColumnResizeable](const ui::EventArgs&) {
		OnColumnResizeable(true);
		return true;
		});
	pColumnWidth->AttachUnSelect([this, OnColumnResizeable](const ui::EventArgs& args) {
		OnColumnResizeable(false);
		return true;
		});

	//是否支持排序
	auto OnColumnSort = [this, pColumnCombo, pListCtrl](bool bSort) {
			size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
			ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
			ASSERT(pHeaderItem != nullptr);
			if (pHeaderItem != nullptr) {
				if (bSort) {
					pHeaderItem->SetSortMode(ui::ListCtrlHeaderItem::SortMode::kUp, true);
				}
				else {
					pHeaderItem->SetSortMode(ui::ListCtrlHeaderItem::SortMode::kNone);
				}				
			}
		};
	pColumnSort->AttachSelect([this, OnColumnSort](const ui::EventArgs&) {
		OnColumnSort(true);
		return true;
		});
	pColumnSort->AttachUnSelect([this, OnColumnSort](const ui::EventArgs& args) {
		OnColumnSort(false);
		return true;
		});

	//是否图标显示在上方
	auto OnColumnShowIconOnTop = [this, pColumnCombo, pListCtrl](bool bShowIconAtTop) {
			size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
			ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
			ASSERT(pHeaderItem != nullptr);
			if (pHeaderItem != nullptr) {
				pHeaderItem->SetShowIconAtTop(bShowIconAtTop);
			}
		};
	pColumnIcon->AttachSelect([this, OnColumnShowIconOnTop](const ui::EventArgs&) {
		OnColumnShowIconOnTop(true);
		return true;
		});
	pColumnIcon->AttachUnSelect([this, OnColumnShowIconOnTop](const ui::EventArgs& args) {
		OnColumnShowIconOnTop(false);
		return true;
		});

	//表头是否显示CheckBox
	auto OnSetCheckBoxVisible = [this, pColumnCombo, pListCtrl](bool bCheckBoxVisible) {
		size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
		ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
		ASSERT(pHeaderItem != nullptr);
		if (pHeaderItem != nullptr) {
			pHeaderItem->SetCheckBoxVisible(bCheckBoxVisible);
		}
	};
	pColumnHeaderCheckBox->AttachSelect([this, OnSetCheckBoxVisible](const ui::EventArgs&) {
		OnSetCheckBoxVisible(true);
		return true;
		});
	pColumnHeaderCheckBox->AttachUnSelect([this, OnSetCheckBoxVisible](const ui::EventArgs& args) {
		OnSetCheckBoxVisible(false);
		return true;
		});

	auto OnHeaderTextAlign = [this, pColumnCombo, pListCtrl](ui::HorAlignType alignType) {
			size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
			ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
			ASSERT(pHeaderItem != nullptr);
			if (pHeaderItem != nullptr) {
				pHeaderItem->SetTextHorAlign(alignType);
			}
		};
	pColumnHeaderTextAlignLeft->AttachSelect([this, OnHeaderTextAlign](const ui::EventArgs&) {
		OnHeaderTextAlign(ui::HorAlignType::kHorAlignLeft);
		return true;
		});
	pColumnHeaderTextAlignCenter->AttachSelect([this, OnHeaderTextAlign](const ui::EventArgs&) {
		OnHeaderTextAlign(ui::HorAlignType::kHorAlignCenter);
		return true;
		});
	pColumnHeaderTextAlignRight->AttachSelect([this, OnHeaderTextAlign](const ui::EventArgs&) {
		OnHeaderTextAlign(ui::HorAlignType::kHorAlignRight);
		return true;
		});

	auto OnShowCheckBox = [this, pListCtrl](bool bShowCheckBox) {
			size_t nItemCount = pListCtrl->GetDataItemCount();
			size_t nColumnCount = pListCtrl->GetColumnCount();
			for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
				for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
					pListCtrl->SetShowCheckBox(nItemIndex, nColumnIndex, bShowCheckBox);
				}
			}
		};
	ui::CheckBox* pShowCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_show_row_checkbox"));
	if (pShowCheckBox != nullptr) {
		pShowCheckBox->Selected(true, false);
		pShowCheckBox->AttachSelect([this, OnShowCheckBox](const ui::EventArgs&) {
			OnShowCheckBox(true);
			return true;
			});
		pShowCheckBox->AttachUnSelect([this, OnShowCheckBox](const ui::EventArgs&) {
			OnShowCheckBox(false);
			return true;
			});
	}

	//在列表头点击右键
	ui::ListCtrlHeader* pHeaderCtrl = pListCtrl->GetListCtrlHeader();
	if (pHeaderCtrl != nullptr) {
		pHeaderCtrl->AttachRClick([this](const ui::EventArgs&) {
			if (::MessageBox(nullptr, L"ListCtrlHeader RClick! 是否执行功能测试？", L"", MB_YESNO) == IDYES) {
				RunListCtrlTest();
			}			
			return true;
			});
	}
}

void MainForm::InsertItemData(int32_t nRows, int32_t nColumns)
{
	ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(L"list_ctrl"));
	ASSERT(pListCtrl != nullptr);
	if (pListCtrl == nullptr) {
		return;
	}
	const size_t columnCount = nColumns;
	const size_t rowCount = nRows;
	//添加列
	for (size_t i = 0; i < columnCount; ++i) {
		ui::ListCtrlColumn columnInfo;
		columnInfo.nColumnWidth = 200;
		//columnInfo.nTextFormat = TEXT_LEFT | TEXT_VCENTER;
		columnInfo.text = ui::StringHelper::Printf(L"第 %d 列", i);
		pListCtrl->InsertColumn(-1, columnInfo);
	}
	//填充数据
	pListCtrl->SetDataItemCount(rowCount);
	ASSERT(pListCtrl->GetDataItemCount() == rowCount);
	for (size_t itemIndex = 0; itemIndex < rowCount; ++itemIndex) {
		for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
			pListCtrl->SetDataItem(itemIndex, { columnIndex, ui::StringHelper::Printf(L"第 %03d 行/第 %02d 列", itemIndex, columnIndex), });
		}
	}
	//排序，默认为升序
	pListCtrl->SortDataItems(0, true);

	//设置特殊属性
	if (nRows > 100) {
		pListCtrl->SetDataItemAlwaysAtTop(100, 0);
		pListCtrl->SetDataItemAlwaysAtTop(101, 1);

		pListCtrl->SetDataItemVisible(103, false);
		pListCtrl->SetDataItemHeight(1, 100, true);
		pListCtrl->SetDataItemHeight(2, 200, true);

		pListCtrl->SetDataItemBkColor(100, 0, ui::UiColor(ui::UiColors::MistyRose));
		pListCtrl->SetDataItemBkColor(101, 0, ui::UiColor(ui::UiColors::MistyRose));
		pListCtrl->SetDataItemBkColor(102, 0, ui::UiColor(ui::UiColors::MistyRose));
		pListCtrl->SetDataItemBkColor(103, 0, ui::UiColor(ui::UiColors::MistyRose));
		pListCtrl->SetDataItemBkColor(104, 0, ui::UiColor(ui::UiColors::MistyRose));
	}
}

void MainForm::RunListCtrlTest()
{
	ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(L"list_ctrl"));
	ASSERT(pListCtrl != nullptr);
	if (pListCtrl == nullptr) {
		return;
	}
	const size_t nRows = pListCtrl->GetDataItemCount();
	const size_t nColumns = pListCtrl->GetColumnCount();
	if (nRows <= 100) {
		return;
	}

#ifdef _DEBUG

	//基本功能测试
	const std::wstring text = L"1";
	ui::ListCtrlDataItem dataItem;
	dataItem.nColumnIndex = 0;
	dataItem.text = text;
	const size_t nDataItemIndex = pListCtrl->AddDataItem(dataItem);
	ASSERT(pListCtrl->GetDataItemText(nDataItemIndex, 0) == text);

	ui::ListCtrlRowData itemData;
	itemData.nItemHeight = 63;
	pListCtrl->SetDataItemRowData(nDataItemIndex, itemData);
	itemData.nItemHeight = 0;
	pListCtrl->GetDataItemRowData(nDataItemIndex, itemData);
	ASSERT(itemData.nItemHeight == 63);

	ASSERT(pListCtrl->IsDataItemVisible(nDataItemIndex) == true);
	pListCtrl->SetDataItemVisible(nDataItemIndex, false);
	ASSERT(pListCtrl->IsDataItemVisible(nDataItemIndex) == false);
	pListCtrl->SetDataItemVisible(nDataItemIndex, true);

	ASSERT(pListCtrl->IsDataItemSelected(nDataItemIndex) == false);
	pListCtrl->SetDataItemSelected(nDataItemIndex, true);
	ASSERT(pListCtrl->IsDataItemSelected(nDataItemIndex) == true);
	pListCtrl->SetDataItemSelected(nDataItemIndex, false);

	ASSERT(pListCtrl->GetDataItemAlwaysAtTop(nDataItemIndex) == -1);
	pListCtrl->SetDataItemAlwaysAtTop(nDataItemIndex, 1);
	ASSERT(pListCtrl->GetDataItemAlwaysAtTop(nDataItemIndex) == 1);
	pListCtrl->SetDataItemAlwaysAtTop(nDataItemIndex, -1);

	int32_t nOldHeight = pListCtrl->GetDataItemHeight(nDataItemIndex);
	pListCtrl->SetDataItemHeight(nDataItemIndex, nOldHeight + 21, false);
	ASSERT(pListCtrl->GetDataItemHeight(nDataItemIndex) == nOldHeight + 21);
	pListCtrl->SetDataItemHeight(nDataItemIndex, nOldHeight, false);

	ASSERT(pListCtrl->GetDataItemData(nDataItemIndex) == 0);
	pListCtrl->SetDataItemData(nDataItemIndex, 1);
	ASSERT(pListCtrl->GetDataItemData(nDataItemIndex) == 1);
	pListCtrl->SetDataItemData(nDataItemIndex, 0);

	dataItem.bNeedDpiScale = false;
	dataItem.nColumnIndex = 1;
	dataItem.text = L"3";
	dataItem.textColor = ui::UiColor(ui::UiColors::Crimson);
	dataItem.bkColor = ui::UiColor(ui::UiColors::BlanchedAlmond);
	dataItem.bShowCheckBox = false;
	dataItem.nCheckBoxWidth = 40;
	dataItem.nImageIndex = 123;
	dataItem.nTextFormat = ui::TEXT_CENTER | ui::TEXT_VCENTER;
	pListCtrl->SetDataItem(nDataItemIndex, dataItem);

	ui::ListCtrlDataItem dataItem2;
	pListCtrl->GetDataItem(nDataItemIndex, dataItem.nColumnIndex, dataItem2);
	ASSERT(dataItem2.bNeedDpiScale == false);
	ASSERT(dataItem.nColumnIndex == dataItem2.nColumnIndex);
	ASSERT(dataItem.text == std::wstring(dataItem2.text));
	ASSERT(dataItem.textColor == dataItem2.textColor);
	ASSERT(dataItem.bkColor == dataItem2.bkColor);
	ASSERT(dataItem.bShowCheckBox == dataItem2.bShowCheckBox);
	ASSERT(dataItem.nCheckBoxWidth == dataItem2.nCheckBoxWidth);
	ASSERT(dataItem.nImageIndex == dataItem2.nImageIndex);
	ASSERT(dataItem.nTextFormat == dataItem2.nTextFormat);

	ASSERT(pListCtrl->GetDataItemText(nDataItemIndex, dataItem.nColumnIndex) == L"3");

	dataItem.text = L"2";
	dataItem.nColumnIndex = 2;
	pListCtrl->SetDataItemText(nDataItemIndex, dataItem.nColumnIndex, dataItem.text);
	ASSERT(pListCtrl->GetDataItemText(nDataItemIndex, dataItem.nColumnIndex) == dataItem.text);

	dataItem.bkColor = ui::UiColor(ui::UiColors::Aqua);
	dataItem.nColumnIndex = 2;
	pListCtrl->SetDataItemBkColor(nDataItemIndex, dataItem.nColumnIndex, dataItem.bkColor);
	ASSERT(pListCtrl->GetDataItemBkColor(nDataItemIndex, dataItem.nColumnIndex) == dataItem.bkColor);

	dataItem.textColor = ui::UiColor(ui::UiColors::Coral);
	dataItem.nColumnIndex = 2;
	pListCtrl->SetDataItemTextColor(nDataItemIndex, dataItem.nColumnIndex, dataItem.textColor);
	ASSERT(pListCtrl->GetDataItemTextColor(nDataItemIndex, dataItem.nColumnIndex) == dataItem.textColor);

	pListCtrl->SetShowCheckBox(nDataItemIndex, dataItem.nColumnIndex, false);
	ASSERT(pListCtrl->IsShowCheckBox(nDataItemIndex, dataItem.nColumnIndex) == false);
	pListCtrl->SetShowCheckBox(nDataItemIndex, dataItem.nColumnIndex, true);
	ASSERT(pListCtrl->IsShowCheckBox(nDataItemIndex, dataItem.nColumnIndex) == true);

	pListCtrl->SetCheckBoxSelect(nDataItemIndex, dataItem.nColumnIndex, false);
	ASSERT(pListCtrl->IsCheckBoxSelect(nDataItemIndex, dataItem.nColumnIndex) == false);
	pListCtrl->SetCheckBoxSelect(nDataItemIndex, dataItem.nColumnIndex, true);
	ASSERT(pListCtrl->IsCheckBoxSelect(nDataItemIndex, dataItem.nColumnIndex) == true);

	dataItem.text = L"3";
	dataItem.nColumnIndex = 1;
	pListCtrl->InsertDataItem(nDataItemIndex, dataItem);
	ASSERT(pListCtrl->GetDataItemText(nDataItemIndex, dataItem.nColumnIndex) == L"3");
	//pListCtrl->DeleteDataItem(nDataItemIndex);
	//pListCtrl->DeleteAllDataItems();

	const size_t nTotalRows = pListCtrl->GetDataItemCount();
	bool bOldMultiSelect = pListCtrl->IsMultiSelect();
	std::vector<size_t> oldSelectedIndexs;
	pListCtrl->GetSelectedDataItems(oldSelectedIndexs);

	//多选
	pListCtrl->SetMultiSelect(true);
	pListCtrl->SetSelectNone();
	std::vector<size_t> selectedIndexs;
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.empty());

	selectedIndexs.push_back(10);
	selectedIndexs.push_back(20);
	selectedIndexs.push_back(30);
	pListCtrl->SetSelectedDataItems(selectedIndexs, false);

	std::vector<size_t> selectedIndexs2;
	pListCtrl->GetSelectedDataItems(selectedIndexs2);
	ASSERT(selectedIndexs == selectedIndexs2);

	selectedIndexs.push_back(40);
	pListCtrl->SetDataItemSelected(40, true);
	selectedIndexs2.clear();
	pListCtrl->GetSelectedDataItems(selectedIndexs2);
	ASSERT(selectedIndexs == selectedIndexs2);

	selectedIndexs.pop_back();
	pListCtrl->SetSelectedDataItems(selectedIndexs, false);

	selectedIndexs.push_back(40);
	selectedIndexs2.clear();
	pListCtrl->GetSelectedDataItems(selectedIndexs2);
	ASSERT(selectedIndexs == selectedIndexs2);

	selectedIndexs.clear();
	selectedIndexs.push_back(50);
	selectedIndexs.push_back(70);
	pListCtrl->SetSelectedDataItems(selectedIndexs, true);
	selectedIndexs2.clear();
	pListCtrl->GetSelectedDataItems(selectedIndexs2);
	ASSERT(selectedIndexs == selectedIndexs2);

	pListCtrl->SetSelectNone();
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.empty());

	for (size_t i = 0; i < nTotalRows; ++i) {
		pListCtrl->SetDataItemVisible(i, true);
		pListCtrl->SetDataItemAlwaysAtTop(i, -1);
	}

	pListCtrl->SetSelectAll();
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.size() == nTotalRows);

	//单选测试
	pListCtrl->SetSelectAll();	
	pListCtrl->SetMultiSelect(false);
	pListCtrl->SetDataItemSelected(40, true);
	ASSERT(!pListCtrl->IsMultiSelect());
	ASSERT(pListCtrl->IsDataItemSelected(40));
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.size() == 1);
	ASSERT(selectedIndexs[0] == 40);

	pListCtrl->SetDataItemSelected(40, false);
	ASSERT(!pListCtrl->IsDataItemSelected(40));

	pListCtrl->SetDataItemSelected(50, true);
	ASSERT(pListCtrl->IsDataItemSelected(50));

	pListCtrl->SetDataItemSelected(60, true);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(!pListCtrl->IsDataItemSelected(50));
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.size() == 1);
	ASSERT(selectedIndexs[0] == 60);

	pListCtrl->SetSelectNone();
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.empty());
	ASSERT(!pListCtrl->IsDataItemSelected(60));

	//添加删除测试
	std::wstring text60 = pListCtrl->GetDataItemText(60, 0);
	pListCtrl->SetDataItemSelected(60, true);
	ASSERT(pListCtrl->IsDataItemSelected(60));

	ui::ListCtrlDataItem dataItem3;
	dataItem3.nColumnIndex = 0;
	dataItem3.text = L"Test";
	size_t nDataItemIndex3 = pListCtrl->AddDataItem(dataItem3);
	ASSERT(nDataItemIndex3 > 60);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetDataItemText(60, 0) == text60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT((selectedIndexs.size() == 1) && (selectedIndexs[0] == 60));

	bool bOk = pListCtrl->InsertDataItem(65, dataItem3);
	ASSERT(bOk);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetDataItemText(60, 0) == text60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT((selectedIndexs.size() == 1) && (selectedIndexs[0] == 60));

	bOk = pListCtrl->InsertDataItem(50, dataItem3);
	ASSERT(bOk);
	ASSERT(pListCtrl->IsDataItemSelected(61));
	ASSERT(pListCtrl->GetDataItemText(61, 0) == text60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT((selectedIndexs.size() == 1) && (selectedIndexs[0] == 61));

	pListCtrl->DeleteDataItem(50);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetDataItemText(60, 0) == text60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT((selectedIndexs.size() == 1) && (selectedIndexs[0] == 60));

	pListCtrl->SortDataItems(0, true);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.size() == 1);
	ASSERT(pListCtrl->GetDataItemText(selectedIndexs[0], 0) == text60);

	pListCtrl->SortDataItems(0, false);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.size() == 1);
	ASSERT(pListCtrl->GetDataItemText(selectedIndexs[0], 0) == text60);

	text60 = pListCtrl->GetDataItemText(60, 0);
	pListCtrl->SetDataItemSelected(60, true);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	pListCtrl->SetDataItemCount(pListCtrl->GetDataItemCount() + 10);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetDataItemText(60, 0) == text60);

	pListCtrl->SetDataItemCount(61);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetDataItemText(60, 0) == text60);

	pListCtrl->SetDataItemCount(60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.empty());

	pListCtrl->SetDataItemSelected(30, true);
	ASSERT(pListCtrl->IsDataItemSelected(30));

	pListCtrl->DeleteAllDataItems();
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.empty());

	pListCtrl->SetDataItemCount(60);
	pListCtrl->SetDataItemSelected(30, true);
	ASSERT(pListCtrl->IsDataItemSelected(30));
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.size() == 1);

	while (pListCtrl->DeleteColumn(0)) {
	}
	ASSERT(pListCtrl->GetDataItemCount() == 0);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.empty());

	//测试完成恢复
	pListCtrl->SetMultiSelect(bOldMultiSelect);
	pListCtrl->SetSelectedDataItems(oldSelectedIndexs, true);

	pListCtrl->DeleteAllDataItems();

	InsertItemData((int32_t)nRows, (int32_t)nColumns);
#endif
}

void MainForm::OnColumnChanged(size_t nColumnId)
{
	ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(L"list_ctrl"));
	ASSERT(pListCtrl != nullptr);
	if (pListCtrl == nullptr) {
		return;
	}

	ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
	ASSERT(pHeaderItem != nullptr);
	if (pHeaderItem == nullptr) {
		return;
	}


	ui::CheckBox* pColumnShow = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show"));
	ui::CheckBox* pColumnWidth = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_width"));
	ui::CheckBox* pColumnSort = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_sort"));
	ui::CheckBox* pColumnIcon = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_icon_at_top"));
	ui::CheckBox* pColumnHeaderCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_header_checkbox"));

	ui::Option* pColumnHeaderTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_left"));
	ui::Option* pColumnHeaderTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_center"));
	ui::Option* pColumnHeaderTextAlignRight = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_right"));

	ASSERT(pHeaderItem->IsColumnVisible() == pHeaderItem->IsVisible());
	pColumnShow->Selected(pHeaderItem->IsColumnVisible(), false);
	pColumnWidth->Selected(pHeaderItem->IsColumnResizeable(), false);

	ui::ListCtrlHeaderItem::SortMode sortMode = pHeaderItem->GetSortMode();
	pColumnSort->Selected(sortMode != ui::ListCtrlHeaderItem::SortMode::kNone, false);

	pColumnIcon->Selected(pHeaderItem->IsShowIconAtTop(), false);
	pColumnHeaderCheckBox->Selected(pHeaderItem->IsCheckBoxVisible(), false);

	ui::HorAlignType hAlignType = pHeaderItem->GetTextHorAlign();
	if (hAlignType == ui::HorAlignType::kHorAlignCenter) {
		pColumnHeaderTextAlignCenter->Selected(true, false);
	}
	else if (hAlignType == ui::HorAlignType::kHorAlignRight) {
		pColumnHeaderTextAlignRight->Selected(true, false);
	}
	else {
		pColumnHeaderTextAlignLeft->Selected(true, false);
	}
}
