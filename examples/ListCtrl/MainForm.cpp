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
	ui::ImageList* pReportImageList = new ui::ImageList;
	ui::ImageList* pIconImageList = new ui::ImageList;
	ui::ImageList* pListImageList = new ui::ImageList;
	pListCtrl->SetImageList(ui::ListCtrlType::Report, pReportImageList);
	pListCtrl->SetImageList(ui::ListCtrlType::Icon, pIconImageList);
	pListCtrl->SetImageList(ui::ListCtrlType::List, pListImageList);

	pReportImageList->SetImageSize(ui::UiSize(18, 18), true);
	pListImageList->SetImageSize(ui::UiSize(32, 32), true);
	pIconImageList->SetImageSize(ui::UiSize(64, 64), true);

	//添加图片资源
	uint32_t imageId = pReportImageList->AddImageString(L"file='1.svg' width='18' height='18'");
	pListImageList->AddImageString(L"file='1.svg' width='32' height='32' valign='center' halign='center'");
	pIconImageList->AddImageString(L"file='1.svg' width='64' height='64' valign='center' halign='center'");

	//填充数据
	InsertItemData(400, 9, (int32_t)imageId);

	// 表格类型
	ui::Combo* pTypeCombo = dynamic_cast<ui::Combo*>(FindControl(L"list_ctrl_type_combo"));
	if (pTypeCombo != nullptr) {
		pTypeCombo->SetCurSel((int32_t)pListCtrl->GetListCtrlType());
		pTypeCombo->AttachSelect([this, pListCtrl, pTypeCombo](const ui::EventArgs& args) {
			size_t nCurSel = args.wParam;
			size_t nType = pTypeCombo->GetItemData(nCurSel);
			if (nType == 0) {
				if (pListCtrl) {
					pListCtrl->SetListCtrlType(ui::ListCtrlType::Report);
				}				
			}
			else if (nType == 1) {
				if (pListCtrl) {
					pListCtrl->SetListCtrlType(ui::ListCtrlType::Icon);
				}
			}
			else if (nType == 2) {
				if (pListCtrl) {
					pListCtrl->SetListCtrlType(ui::ListCtrlType::List);
				}
			}
			return true;
			});
	}

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
	ui::CheckBox* pColumnDragOrder = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_drag_order"));
	ui::CheckBox* pColumnEditable = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_editable"));
	ui::CheckBox* pColumnHeaderCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_header_checkbox"));
	ui::CheckBox* pColumnShowCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_checkbox"));

	ui::CheckBox* pColumnHeaderIcon = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_header_icon"));
	ui::CheckBox* pColumnShowIcon = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_icon"));

	ui::Option* pColumnHeaderTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_left"));
	ui::Option* pColumnHeaderTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_center"));
	ui::Option* pColumnHeaderTextAlignRight = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_right"));

	ui::Option* pColumnTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(L"column_text_align_left"));
	ui::Option* pColumnTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(L"column_text_align_center"));
	ui::Option* pColumnTextAlignRight = dynamic_cast<ui::Option*>(FindControl(L"column_text_align_right"));

	ui::CheckBox* pHeaderCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_show_header_checkbox"));
	ui::CheckBox* pShowCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_show_checkbox"));

	ui::CheckBox* pShowIcon= dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_show_icon"));

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


	//是否可拖动调整顺序
	auto OnColumnDragOrder = [this, pColumnCombo, pListCtrl](bool bEnableDragOrder) {
		size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
		ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
		ASSERT(pHeaderItem != nullptr);
		if (pHeaderItem != nullptr) {
			pHeaderItem->SetEnableDragOrder(bEnableDragOrder);
		}
		};
	pColumnDragOrder->AttachSelect([this, OnColumnDragOrder](const ui::EventArgs&) {
		OnColumnDragOrder(true);
		return true;
		});
	pColumnDragOrder->AttachUnSelect([this, OnColumnDragOrder](const ui::EventArgs& args) {
		OnColumnDragOrder(false);
		return true;
		});

	//是否可编辑
	auto OnColumnEditable = [this, pColumnCombo, pListCtrl](bool bEditable) {
		size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
		size_t nColumnIndex = pListCtrl->GetColumnIndex(nColumnId);
		size_t nItemCount = pListCtrl->GetDataItemCount();
		for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
			pListCtrl->SetSubItemEditable(nItemIndex, nColumnIndex, bEditable);
		}
		};
	pColumnEditable->AttachSelect([this, OnColumnEditable](const ui::EventArgs&) {
		OnColumnEditable(true);
		return true;
		});
	pColumnEditable->AttachUnSelect([this, OnColumnEditable](const ui::EventArgs& args) {
		OnColumnEditable(false);
		return true;
		});

	//列级CheckBox：表头是否显示CheckBox
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

	//列级CheckBox：在每列显示CheckBox
	auto OnShowCheckBox = [this, pColumnCombo, pListCtrl](bool bShowCheckBox) {
			size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
			size_t nColumnIndex = pListCtrl->GetColumnIndex(nColumnId);
			size_t nItemCount = pListCtrl->GetDataItemCount();
			for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
				pListCtrl->SetSubItemShowCheckBox(nItemIndex, nColumnIndex, bShowCheckBox);
			}
		};	
	if (pColumnShowCheckBox != nullptr) {
		bool bCheckBoxVisible = false;
		ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumn(0);
		ASSERT(pHeaderItem != nullptr);
		if (pHeaderItem != nullptr) {
			bCheckBoxVisible = pHeaderItem->IsCheckBoxVisible();
		}
		pColumnShowCheckBox->Selected(bCheckBoxVisible, false);
		pColumnShowCheckBox->AttachSelect([this, OnShowCheckBox](const ui::EventArgs&) {
			OnShowCheckBox(true);
			return true;
			});
		pColumnShowCheckBox->AttachUnSelect([this, OnShowCheckBox](const ui::EventArgs&) {
			OnShowCheckBox(false);
			return true;
			});
	}

	if (pColumnHeaderIcon != nullptr) {
		auto OnSetHeaderShowIcon = [this, pColumnCombo, pListCtrl](bool bShow) {
				size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
				ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
				ASSERT(pHeaderItem != nullptr);
				if (pHeaderItem != nullptr) {
					pHeaderItem->SetImageId(bShow ? 0 : -1);
				}
			};
		pColumnHeaderIcon->AttachSelect([this, OnSetHeaderShowIcon](const ui::EventArgs&) {
			OnSetHeaderShowIcon(true);
			return true;
			});
		pColumnHeaderIcon->AttachUnSelect([this, OnSetHeaderShowIcon](const ui::EventArgs&) {
			OnSetHeaderShowIcon(false);
			return true;
			});
	}
	if (pColumnShowIcon != nullptr) {
		auto OnShowColumnIcon = [this, pColumnCombo, pListCtrl](bool bShow) {
				size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
				size_t nColumnIndex = pListCtrl->GetColumnIndex(nColumnId);
				size_t nItemCount = pListCtrl->GetDataItemCount();
				for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
					pListCtrl->SetSubItemImageId(nItemIndex, nColumnIndex, bShow ? 0 : -1);
				}
			};
		pColumnShowIcon->AttachSelect([this, OnShowColumnIcon](const ui::EventArgs&) {
			OnShowColumnIcon(true);
			return true;
			});
		pColumnShowIcon->AttachUnSelect([this, OnShowColumnIcon](const ui::EventArgs&) {
			OnShowColumnIcon(false);
			return true;
			});
	}

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

	auto OnColumnTextAlign = [this, pColumnCombo, pListCtrl](int32_t nTextFormat) {
		size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
		size_t nColumnIndex = pListCtrl->GetColumnIndex(nColumnId);
		if (nColumnIndex != ui::Box::InvalidIndex) {
			size_t nCount = pListCtrl->GetDataItemCount();
			for (size_t index = 0; index < nCount; ++index) {
				int32_t nNewTextFormat = pListCtrl->GetSubItemTextFormat(index, nColumnIndex);
				if (nTextFormat & ui::TEXT_CENTER) {
					nNewTextFormat &= ~(ui::TEXT_RIGHT | ui::TEXT_LEFT);
					nNewTextFormat |= ui::TEXT_CENTER;
				}
				else if (nTextFormat & ui::TEXT_RIGHT) {
					nNewTextFormat &= ~(ui::TEXT_CENTER | ui::TEXT_LEFT);
					nNewTextFormat |= ui::TEXT_RIGHT;
				}
				else {
					nNewTextFormat &= ~(ui::TEXT_CENTER | ui::TEXT_RIGHT);
					nNewTextFormat |= ui::TEXT_LEFT;
				}
				pListCtrl->SetSubItemTextFormat(index, nColumnIndex, nNewTextFormat);
			}
		}
		};
	pColumnTextAlignLeft->AttachSelect([this, OnColumnTextAlign](const ui::EventArgs&) {
		OnColumnTextAlign(ui::TEXT_LEFT);
		return true;
		});
	pColumnTextAlignCenter->AttachSelect([this, OnColumnTextAlign](const ui::EventArgs&) {
		OnColumnTextAlign(ui::TEXT_CENTER);
		return true;
		});
	pColumnTextAlignRight->AttachSelect([this, OnColumnTextAlign](const ui::EventArgs&) {
		OnColumnTextAlign(ui::TEXT_RIGHT);
		return true;
		});

	//是否支持多选
	ui::CheckBox* pMultiSelect = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_multi_select"));
	if (pMultiSelect != nullptr) {
		pMultiSelect->Selected(pListCtrl->IsMultiSelect(), false);
	}
	//在列表头点击右键
	ui::ListCtrlHeader* pHeaderCtrl = pListCtrl->GetHeaderCtrl();
	if (pHeaderCtrl != nullptr) {
		pHeaderCtrl->AttachRClick([this](const ui::EventArgs&) {
			if (::MessageBox(nullptr, L"ListCtrlHeader RClick! 是否执行功能测试？", L"", MB_YESNO) == IDYES) {
				RunListCtrlTest();
			}			
			return true;
			});
	}

	//控制表头或者行首是否显示CheckBox
	if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible() && pHeaderCtrl->IsShowCheckBox()) {
		pHeaderCheckBox->Selected(true, false);
	}
	else {
		pHeaderCheckBox->Selected(false, false);
	}
	pHeaderCheckBox->AttachSelect([this, pListCtrl](const ui::EventArgs&) {
		pListCtrl->SetHeaderShowCheckBox(true);
		return true;
		});
	pHeaderCheckBox->AttachUnSelect([this, pListCtrl](const ui::EventArgs&) {
		pListCtrl->SetHeaderShowCheckBox(false);
		return true;
		});

	pShowCheckBox->Selected(pListCtrl->IsDataItemShowCheckBox());
	pShowCheckBox->AttachSelect([this, pListCtrl](const ui::EventArgs&) {
		pListCtrl->SetDataItemShowCheckBox(true);
		return true;
		});
	pShowCheckBox->AttachUnSelect([this, pListCtrl](const ui::EventArgs&) {
		pListCtrl->SetDataItemShowCheckBox(false);
		return true;
		});

	if (pShowIcon != nullptr) {
		pShowIcon->Selected(pListCtrl->GetDataItemImageId(0) >= 0, false);
		auto OnShowDataItemIcon = [this, pListCtrl](bool bShow) {
				size_t nItemCount = pListCtrl->GetDataItemCount();
				pListCtrl->SetEnableRefresh(false);
				for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
					pListCtrl->SetDataItemImageId(nItemIndex, bShow ? 0 : -1);
				}
				pListCtrl->SetEnableRefresh(true);
				pListCtrl->Refresh();
			};
		pShowIcon->AttachSelect([this, OnShowDataItemIcon](const ui::EventArgs&) {
			OnShowDataItemIcon(true);
			return true;
			});
		pShowIcon->AttachUnSelect([this, OnShowDataItemIcon](const ui::EventArgs&) {
			OnShowDataItemIcon(false);
			return true;
			});
	}

	//事件挂载，测试事件接口
	auto OnListCtrlEvent = [this, pListCtrl](const ui::EventArgs& args) {
			ASSERT(pListCtrl == args.pSender);
			ui::ListCtrlItem* pItem = (ui::ListCtrlItem*)args.wParam;
			size_t itemIndex = args.lParam;
			ui::UiPoint mousePt = args.ptMouse;
			std::wstring text;
			if (pItem != nullptr) {
				size_t columnIndex = pItem->GetSubItemIndex(mousePt);
				if (columnIndex != ui::Box::InvalidIndex) {
					text = pListCtrl->GetSubItemText(itemIndex, columnIndex);
				}				
			}

			if (args.Type == ui::kEventSelect) {
				std::wstring sInfo = ui::StringHelper::Printf(L"kEventSelect，数据索引号：%d", (int32_t)itemIndex);
				//::MessageBox(nullptr, sInfo.c_str(), L"", MB_OK);
			}
			else if (args.Type == ui::kEventSelChange) {
				//::MessageBox(nullptr, L"kEventSelChange", L"", MB_OK);
			}
			else if(args.Type == ui::kEventMouseDoubleClick) {				
				//::MessageBox(nullptr, L"kEventMouseDoubleClick", text.c_str(), MB_OK);
			}
			else if (args.Type == ui::kEventClick) {
				//::MessageBox(nullptr, L"kEventClick", text.c_str(), MB_OK);
			}
			else if (args.Type == ui::kEventRClick) {
				//::MessageBox(nullptr, L"kEventRClick", text.c_str(), MB_OK);
			}
			else if (args.Type == ui::kEventReturn) {
				std::wstring sInfo = ui::StringHelper::Printf(L"kEventReturn，数据索引号：%d", (int32_t)itemIndex);
				//::MessageBox(nullptr, sInfo.c_str(), L"", MB_OK);
			}
		};

	//挂载事件，转接给外层
	pListCtrl->AttachSelect([this, OnListCtrlEvent](const ui::EventArgs& args) {
		OnListCtrlEvent(args);
		return true;
		});
	pListCtrl->AttachSelChange([this, OnListCtrlEvent](const ui::EventArgs& args) {
		OnListCtrlEvent(args);
		return true;
		});
	pListCtrl->AttachDoubleClick([this, OnListCtrlEvent](const ui::EventArgs& args) {
		OnListCtrlEvent(args);
		return true;
		});
	pListCtrl->AttachClick([this, OnListCtrlEvent](const ui::EventArgs& args) {
		OnListCtrlEvent(args);
		return true;
		});
	pListCtrl->AttachRClick([this, OnListCtrlEvent](const ui::EventArgs& args) {
		OnListCtrlEvent(args);
		return true;
		});
	pListCtrl->AttachReturn([this, OnListCtrlEvent](const ui::EventArgs& args) {
		OnListCtrlEvent(args);
		return true;
		});
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
	ui::CheckBox* pColumnDragOrder = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_drag_order"));
	ui::CheckBox* pColumnEditable = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_editable"));
	ui::CheckBox* pColumnHeaderCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_header_checkbox"));
	ui::CheckBox* pColumnShowCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_checkbox"));

	ui::CheckBox* pColumnHeaderIcon = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_header_icon"));
	ui::CheckBox* pColumnShowIcon = dynamic_cast<ui::CheckBox*>(FindControl(L"checkbox_column_show_icon"));

	ui::Option* pColumnHeaderTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_left"));
	ui::Option* pColumnHeaderTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_center"));
	ui::Option* pColumnHeaderTextAlignRight = dynamic_cast<ui::Option*>(FindControl(L"header_text_align_right"));

	ui::Option* pColumnTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(L"column_text_align_left"));
	ui::Option* pColumnTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(L"column_text_align_center"));
	ui::Option* pColumnTextAlignRight = dynamic_cast<ui::Option*>(FindControl(L"column_text_align_right"));

	ASSERT(pHeaderItem->IsColumnVisible() == pHeaderItem->IsVisible());
	pColumnShow->Selected(pHeaderItem->IsColumnVisible(), false);
	pColumnWidth->Selected(pHeaderItem->IsColumnResizeable(), false);

	ui::ListCtrlHeaderItem::SortMode sortMode = pHeaderItem->GetSortMode();
	pColumnSort->Selected(sortMode != ui::ListCtrlHeaderItem::SortMode::kNone, false);

	pColumnIcon->Selected(pHeaderItem->IsShowIconAtTop(), false);
	pColumnDragOrder->Selected(pHeaderItem->IsEnableDragOrder(), false);
	pColumnHeaderCheckBox->Selected(pHeaderItem->IsCheckBoxVisible(), false);
	pColumnHeaderIcon->SetSelected(pHeaderItem->GetImageId() >= 0);

	bool bColumnDataHasCheckBox = false;
	bool bColumnDataHasIcon = false;
	bool bColumnEditable = pListCtrl->IsSubItemEditable(0, 0);
	ui::ListCtrlItem* pItem = pListCtrl->GetFirstDisplayItem();
	if (pItem != nullptr) {
		ui::ListCtrlSubItem* pSubItem = pItem->GetSubItem(pListCtrl->GetColumnIndex(nColumnId));
		if (pSubItem != nullptr) {
			bColumnDataHasCheckBox = pSubItem->IsCheckBoxVisible();
			bColumnDataHasIcon = pSubItem->GetImageId() >= 0;
		}
	}
	else {
		bColumnDataHasCheckBox = pListCtrl->IsSubItemShowCheckBox(0, 0);
		bColumnDataHasIcon = pListCtrl->GetSubItemImageId(0, 0) >= 0;
	}
	pColumnShowCheckBox->Selected(bColumnDataHasCheckBox, false);
	pColumnShowIcon->Selected(bColumnDataHasIcon, false);
	pColumnEditable->Selected(bColumnEditable, false);

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

	int32_t nTextFormat = pListCtrl->GetSubItemTextFormat(0, pListCtrl->GetColumnIndex(nColumnId));
	if (nTextFormat & ui::TEXT_CENTER) {
		pColumnTextAlignCenter->Selected(true, false);
	}
	else if (nTextFormat & ui::TEXT_RIGHT) {
		pColumnTextAlignRight->Selected(true, false);
	}
	else {
		pColumnTextAlignLeft->Selected(true, false);
	}
}

void MainForm::InsertItemData(int32_t nRows, int32_t nColumns, int32_t nImageId)
{
	ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(L"list_ctrl"));
	ASSERT(pListCtrl != nullptr);
	if (pListCtrl == nullptr) {
		return;
	}
	const size_t columnCount = nColumns;
	const size_t rowCount = nRows;
	bool bShowCheckBox = true; //是否显示CheckBox
	//添加列
	for (size_t i = 0; i < columnCount; ++i) {
		ui::ListCtrlColumn columnInfo;
		columnInfo.nColumnWidth = 200;
		//columnInfo.nTextFormat = TEXT_LEFT | TEXT_VCENTER;
		columnInfo.text = ui::StringHelper::Printf(L"第 %d 列", i);
		columnInfo.bShowCheckBox = bShowCheckBox;
		columnInfo.nImageId = nImageId;
		pListCtrl->InsertColumn(-1, columnInfo);
	}
	//填充数据
	pListCtrl->SetDataItemCount(rowCount);
	ASSERT(pListCtrl->GetDataItemCount() == rowCount);
	for (size_t itemIndex = 0; itemIndex < rowCount; ++itemIndex) {
		for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
			ui::ListCtrlSubItemData subItemData;
			subItemData.text = ui::StringHelper::Printf(L"第 %03d 行/第 %02d 列", itemIndex, columnIndex);
			subItemData.bShowCheckBox = bShowCheckBox;
			subItemData.nImageId = nImageId;
			if (columnIndex == 0) {
				subItemData.text += L"-测试1234567890-测试1234567890-测试1234567890-测试1234567890";
			}
			pListCtrl->SetSubItemData(itemIndex, columnIndex, subItemData);
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

		pListCtrl->SetSubItemBkColor(100, 0, ui::UiColor(ui::UiColors::MistyRose));
		pListCtrl->SetSubItemBkColor(101, 0, ui::UiColor(ui::UiColors::MistyRose));
		pListCtrl->SetSubItemBkColor(102, 0, ui::UiColor(ui::UiColors::MistyRose));
		pListCtrl->SetSubItemBkColor(103, 0, ui::UiColor(ui::UiColors::MistyRose));
		pListCtrl->SetSubItemBkColor(104, 0, ui::UiColor(ui::UiColors::MistyRose));
	}
	//重绘
	UpdateWindow();
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
	ui::ListCtrlSubItemData subItemData;
	subItemData.text = text;
	const size_t nDataItemIndex = pListCtrl->AddDataItem(subItemData);
	ASSERT(pListCtrl->GetSubItemText(nDataItemIndex, 0) == text);

	ui::ListCtrlItemData itemData;
	itemData.nItemHeight = 63;
	pListCtrl->SetDataItemData(nDataItemIndex, itemData);
	itemData.nItemHeight = 0;
	pListCtrl->GetDataItemData(nDataItemIndex, itemData);
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

	ASSERT(pListCtrl->GetDataItemUserData(nDataItemIndex) == 0);
	pListCtrl->SetDataItemUserData(nDataItemIndex, 1);
	ASSERT(pListCtrl->GetDataItemUserData(nDataItemIndex) == 1);
	pListCtrl->SetDataItemUserData(nDataItemIndex, 0);

	size_t nColumnIndex = 1;
	subItemData.text = L"3";
	subItemData.textColor = ui::UiColor(ui::UiColors::Crimson);
	subItemData.bkColor = ui::UiColor(ui::UiColors::BlanchedAlmond);
	subItemData.bShowCheckBox = false;
	subItemData.nImageId = 123;
	subItemData.nTextFormat = ui::TEXT_CENTER | ui::TEXT_VCENTER;
	pListCtrl->SetSubItemData(nDataItemIndex, nColumnIndex, subItemData);

	ui::ListCtrlSubItemData dataItem2;
	pListCtrl->GetSubItemData(nDataItemIndex, nColumnIndex, dataItem2);
	ASSERT(subItemData.text == std::wstring(dataItem2.text));
	ASSERT(subItemData.textColor == dataItem2.textColor);
	ASSERT(subItemData.bkColor == dataItem2.bkColor);
	ASSERT(subItemData.bShowCheckBox == dataItem2.bShowCheckBox);
	ASSERT(subItemData.nImageId == dataItem2.nImageId);
	ASSERT(subItemData.nTextFormat == dataItem2.nTextFormat);

	ASSERT(pListCtrl->GetSubItemText(nDataItemIndex, nColumnIndex) == L"3");

	subItemData.text = L"2";
	nColumnIndex = 2;
	pListCtrl->SetSubItemText(nDataItemIndex, nColumnIndex, subItemData.text);
	ASSERT(pListCtrl->GetSubItemText(nDataItemIndex, nColumnIndex) == subItemData.text);

	subItemData.bkColor = ui::UiColor(ui::UiColors::Aqua);
	nColumnIndex = 2;
	pListCtrl->SetSubItemBkColor(nDataItemIndex, nColumnIndex, subItemData.bkColor);
	ASSERT(pListCtrl->GetSubItemBkColor(nDataItemIndex, nColumnIndex) == subItemData.bkColor);

	subItemData.textColor = ui::UiColor(ui::UiColors::Coral);
	nColumnIndex = 2;
	pListCtrl->SetSubItemTextColor(nDataItemIndex, nColumnIndex, subItemData.textColor);
	ASSERT(pListCtrl->GetSubItemTextColor(nDataItemIndex, nColumnIndex) == subItemData.textColor);

	pListCtrl->SetSubItemShowCheckBox(nDataItemIndex, nColumnIndex, false);
	ASSERT(pListCtrl->IsSubItemShowCheckBox(nDataItemIndex, nColumnIndex) == false);
	pListCtrl->SetSubItemShowCheckBox(nDataItemIndex, nColumnIndex, true);
	ASSERT(pListCtrl->IsSubItemShowCheckBox(nDataItemIndex, nColumnIndex) == true);

	pListCtrl->SetSubItemCheck(nDataItemIndex, nColumnIndex, false);
	ASSERT(pListCtrl->IsSubItemChecked(nDataItemIndex, nColumnIndex) == false);
	pListCtrl->SetSubItemCheck(nDataItemIndex, nColumnIndex, true);
	ASSERT(pListCtrl->IsSubItemChecked(nDataItemIndex, nColumnIndex) == true);

	pListCtrl->SetDataItemImageId(nDataItemIndex, 666);
	ASSERT(pListCtrl->GetDataItemImageId(nDataItemIndex) == 666);

	pListCtrl->SetSubItemImageId(nDataItemIndex, nColumnIndex, 667);
	ASSERT(pListCtrl->GetSubItemImageId(nDataItemIndex, nColumnIndex) == 667);

	subItemData.text = L"3";
	nColumnIndex = 0;
	pListCtrl->InsertDataItem(nDataItemIndex, subItemData);
	ASSERT(pListCtrl->GetSubItemText(nDataItemIndex, nColumnIndex) == L"3");
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
	std::wstring text60 = pListCtrl->GetSubItemText(60, 0);
	pListCtrl->SetDataItemSelected(60, true);
	ASSERT(pListCtrl->IsDataItemSelected(60));

	ui::ListCtrlSubItemData dataItem3;
	nColumnIndex = 0;
	dataItem3.text = L"Test";
	size_t nDataItemIndex3 = pListCtrl->AddDataItem(dataItem3);
	ASSERT(nDataItemIndex3 > 60);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetSubItemText(60, 0) == text60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT((selectedIndexs.size() == 1) && (selectedIndexs[0] == 60));

	bool bOk = pListCtrl->InsertDataItem(65, dataItem3);
	ASSERT(bOk);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetSubItemText(60, 0) == text60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT((selectedIndexs.size() == 1) && (selectedIndexs[0] == 60));

	bOk = pListCtrl->InsertDataItem(50, dataItem3);
	ASSERT(bOk);
	ASSERT(pListCtrl->IsDataItemSelected(61));
	ASSERT(pListCtrl->GetSubItemText(61, 0) == text60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT((selectedIndexs.size() == 1) && (selectedIndexs[0] == 61));

	pListCtrl->DeleteDataItem(50);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetSubItemText(60, 0) == text60);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT((selectedIndexs.size() == 1) && (selectedIndexs[0] == 60));

	pListCtrl->SortDataItems(0, true);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.size() == 1);
	ASSERT(pListCtrl->GetSubItemText(selectedIndexs[0], 0) == text60);

	pListCtrl->SortDataItems(0, false);
	pListCtrl->GetSelectedDataItems(selectedIndexs);
	ASSERT(selectedIndexs.size() == 1);
	ASSERT(pListCtrl->GetSubItemText(selectedIndexs[0], 0) == text60);

	text60 = pListCtrl->GetSubItemText(60, 0);
	pListCtrl->SetDataItemSelected(60, true);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	pListCtrl->SetDataItemCount(pListCtrl->GetDataItemCount() + 10);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetSubItemText(60, 0) == text60);

	pListCtrl->SetDataItemCount(61);
	ASSERT(pListCtrl->IsDataItemSelected(60));
	ASSERT(pListCtrl->GetSubItemText(60, 0) == text60);

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

	InsertItemData((int32_t)nRows, (int32_t)nColumns, -1);
#endif
}

