#include "MainForm.h"
#include "MainThread.h"

#include <cmath>
#include <chrono>

#include "duilib/Utils/FilePath.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    #include <psapi.h>
    #pragma comment(lib, "psapi.lib")
#endif

MainForm::MainForm():
    m_fLoadingPercent(0),
    m_imageId(0),
    m_bStressDataLoaded(false),
    m_bAllColumnsEditable(false)
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("list_ctrl");
}

DString MainForm::GetSkinFile()
{
    return _T("list_ctrl.xml");
}

void MainForm::OnInitWindow()
{
    ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
    ASSERT(pListCtrl != nullptr);
    if (pListCtrl == nullptr) {
        return;
    }
    ui::ImageListPtr pReportImageList = std::make_shared<ui::ImageList>();
    ui::ImageListPtr pIconImageList = std::make_shared<ui::ImageList>();
    ui::ImageListPtr pListImageList = std::make_shared<ui::ImageList>();
    pListCtrl->SetImageList(ui::ListCtrlType::Report, pReportImageList);
    pListCtrl->SetImageList(ui::ListCtrlType::Icon, pIconImageList);
    pListCtrl->SetImageList(ui::ListCtrlType::List, pListImageList);

    pReportImageList->SetImageSize(ui::UiSize(22, 22), Dpi(), true);
    pListImageList->SetImageSize(ui::UiSize(32, 32), Dpi(), true);
    pIconImageList->SetImageSize(ui::UiSize(64, 64), Dpi(), true);

    //添加图片资源
    m_imageId = pReportImageList->AddImageString(_T("file='display-color.svg' width='22' height='22'"), Dpi());
    pListImageList->AddImageString(_T("file='display-color.svg' width='32' height='32' valign='center' halign='center'"), Dpi());
    pIconImageList->AddImageString(_T("file='display-color.svg' width='64' height='64' valign='center' halign='center'"), Dpi());

    //默认在表头显示CheckBox
    if (pListCtrl->GetHeaderCtrl() != nullptr) {
        pListCtrl->GetHeaderCtrl()->SetShowCheckBox(true);
    }

    //填充数据
    InsertItemData(400, 9, (int32_t)m_imageId);

    //初始化本程序的测试功能相关UI事件
    InitListCtrlEvents(pListCtrl);

    //事件挂载，测试事件接口
    TestListCtrlEvents(pListCtrl);

    //测试loading功能
    TestListCtrlLoading(pListCtrl);

    //演示单格刷新功能（RefreshSubItem）
    //TestRefreshSubItem(pListCtrl);

    //演示ListCtrl级别的列显隐接口（SetColumnVisible/IsColumnVisible）
    TestColumnVisibleApi(pListCtrl);

    //初始化"压力测试"面板（Phase 0 性能基线）
    InitStressTest(pListCtrl);

    //pListCtrl->AttachViewSizeChanged([this, pListCtrl](const ui::EventArgs& args) {
    //    if (args.listCtrlType == (int32_t)ui::ListCtrlType::Report) {
    //        //视图大小变化时，自动调整列宽
    //        std::vector<ui::UiFixedInt> columnWidthList;
    //        size_t nColumnCount = pListCtrl->GetColumnCount();
    //        for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; nColumnIndex++) {
    //            columnWidthList.push_back(ui::UiFixedInt::MakeStretch());
    //        }
    //        bool bRet = pListCtrl->SetColumnWidth(columnWidthList, true);
    //        UNUSED_VARIABLE(bRet);
    //    }        
    //    return true;
    //    });
}

bool MainForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();
    //语言变化时，需要更新列表中的数据
    ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
    ASSERT(pListCtrl != nullptr);
    if (pListCtrl != nullptr) {
        pListCtrl->SetEnableRefresh(false);
        pListCtrl->DeleteAllColumns();
        pListCtrl->DeleteAllDataItems();
        InsertItemData(400, 9, (int32_t)m_imageId);
        pListCtrl->SetEnableRefresh(true);
    }    
    return bRet;
}

void MainForm::OnInitLayout()
{
    //测试自动调整列宽为等比例
    /*ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
    ASSERT(pListCtrl != nullptr);
    if (pListCtrl == nullptr) {
        return;
    }
    std::vector<ui::UiFixedInt> columnWidthList;
    size_t nColumnCount = pListCtrl->GetColumnCount();
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; nColumnIndex++) {
        columnWidthList.push_back(ui::UiFixedInt::MakeStretch());
    }
    bool bRet = pListCtrl->SetColumnWidth(columnWidthList, true);
    UNUSED_VARIABLE(bRet);*/
}

void MainForm::InitListCtrlEvents(ui::ListCtrl* pListCtrl)
{
    // 表格类型
    ui::Combo* pTypeCombo = dynamic_cast<ui::Combo*>(FindControl(_T("list_ctrl_type_combo")));
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
    ui::RichEdit* pHeaderHeightEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("header_height_edit")));
    if (pHeaderHeightEdit != nullptr) {
        pHeaderHeightEdit->SetText(ui::StringUtil::Printf(_T("%d"), pListCtrl->GetHeaderHeight()));
        pHeaderHeightEdit->AttachTextChanged([this, pHeaderHeightEdit, pListCtrl](const ui::EventArgs&) {
            int32_t height = ui::StringUtil::StringToInt32(pHeaderHeightEdit->GetText());
            if (height >= 0) {
                pListCtrl->SetHeaderHeight(height, false);
            }
            return true;
            });
    }

    //表格每行高度控制
    ui::RichEdit* pItemHeightEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("list_item_height_edit")));
    if (pItemHeightEdit != nullptr) {
        pItemHeightEdit->SetText(ui::StringUtil::Printf(_T("%d"), pListCtrl->GetDataItemHeight()));
        pItemHeightEdit->AttachTextChanged([this, pItemHeightEdit, pListCtrl](const ui::EventArgs&) {
            int32_t height = ui::StringUtil::StringToInt32(pItemHeightEdit->GetText());
            if (height >= 0) {
                pListCtrl->SetDataItemHeight(height, false);
            }
            return true;
            });
    }

    //列控制
    ui::Combo* pColumnCombo = dynamic_cast<ui::Combo*>(FindControl(_T("column_combo")));
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
                pColumnCombo->SetItemData(nItem, pHeaderItem->GetColumnId());
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

    ui::CheckBox* pColumnShow = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show")));
    ui::CheckBox* pColumnWidth = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_width")));
    ui::CheckBox* pColumnSort = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_sort")));
    ui::CheckBox* pColumnIcon = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_icon_at_top")));
    ui::CheckBox* pColumnDragOrder = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_drag_order")));
    ui::CheckBox* pColumnEditable = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_editable")));
    ui::CheckBox* pColumnHeaderCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show_header_checkbox")));
    ui::CheckBox* pColumnShowCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show_checkbox")));

    ui::CheckBox* pColumnHeaderIcon = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show_header_icon")));
    ui::CheckBox* pColumnShowIcon = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show_icon")));

    ui::Option* pColumnHeaderTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(_T("header_text_align_left")));
    ui::Option* pColumnHeaderTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(_T("header_text_align_center")));
    ui::Option* pColumnHeaderTextAlignRight = dynamic_cast<ui::Option*>(FindControl(_T("header_text_align_right")));

    ui::Option* pColumnTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(_T("column_text_align_left")));
    ui::Option* pColumnTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(_T("column_text_align_center")));
    ui::Option* pColumnTextAlignRight = dynamic_cast<ui::Option*>(FindControl(_T("column_text_align_right")));

    ui::CheckBox* pHeaderCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_show_header_checkbox")));
    ui::CheckBox* pShowCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_show_checkbox")));

    ui::CheckBox* pShowIcon = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_show_icon")));

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
            pHeaderItem->SetShowCheckBox(bCheckBoxVisible);
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
        if (pListCtrl->GetColumnCount() > 0) {
            ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumn(0);
            ASSERT(pHeaderItem != nullptr);
            if (pHeaderItem != nullptr) {
                bCheckBoxVisible = pHeaderItem->IsShowCheckBox();
            }
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
        OnHeaderTextAlign(ui::HorAlignType::kAlignLeft);
        return true;
        });
    pColumnHeaderTextAlignCenter->AttachSelect([this, OnHeaderTextAlign](const ui::EventArgs&) {
        OnHeaderTextAlign(ui::HorAlignType::kAlignCenter);
        return true;
        });
    pColumnHeaderTextAlignRight->AttachSelect([this, OnHeaderTextAlign](const ui::EventArgs&) {
        OnHeaderTextAlign(ui::HorAlignType::kAlignRight);
        return true;
        });

    auto OnColumnTextAlign = [this, pColumnCombo, pListCtrl](int32_t nTextFormat) {
        size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
        size_t nColumnIndex = pListCtrl->GetColumnIndex(nColumnId);
        if (nColumnIndex != ui::Box::InvalidIndex) {
            size_t nCount = pListCtrl->GetDataItemCount();
            for (size_t index = 0; index < nCount; ++index) {
                int32_t nNewTextFormat = pListCtrl->GetSubItemTextFormat(index, nColumnIndex);
                if (nTextFormat & ui::TEXT_HCENTER) {
                    nNewTextFormat &= ~(ui::TEXT_HALIGN_ALL);
                    nNewTextFormat |= ui::TEXT_HCENTER;
                }
                else if (nTextFormat & ui::TEXT_RIGHT) {
                    nNewTextFormat &= ~(ui::TEXT_HALIGN_ALL);
                    nNewTextFormat |= ui::TEXT_RIGHT;
                }
                else {
                    nNewTextFormat &= ~(ui::TEXT_HALIGN_ALL);
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
        OnColumnTextAlign(ui::TEXT_HCENTER);
        return true;
        });
    pColumnTextAlignRight->AttachSelect([this, OnColumnTextAlign](const ui::EventArgs&) {
        OnColumnTextAlign(ui::TEXT_RIGHT);
        return true;
        });

    //单元格自动换行：对当前选中列的单元格，开启/关闭自动换行（仅Report视图有效）
    //说明：换行开启后，文本超出单元格宽度时自动换行；需配合 SetDataItemHeight 把行高调大，否则超出部分会被裁剪
    ui::Option* pCellSingleLine = dynamic_cast<ui::Option*>(FindControl(_T("cell_single_line")));
    ui::Option* pCellWordWrap = dynamic_cast<ui::Option*>(FindControl(_T("cell_word_wrap")));
    if ((pCellSingleLine != nullptr) && (pCellWordWrap != nullptr)) {
        auto OnCellWrap = [this, pColumnCombo, pListCtrl](bool bSingleLine) {
            size_t nColumnId = pColumnCombo->GetItemData(pColumnCombo->GetCurSel());
            size_t nColumnIndex = pListCtrl->GetColumnIndex(nColumnId);
            if (nColumnIndex == ui::Box::InvalidIndex) {
                return;
            }
            size_t nItemCount = pListCtrl->GetDataItemCount();
            for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
                pListCtrl->SetSubItemSingleLine(nItemIndex, nColumnIndex, bSingleLine);
            }
            //换行后需要调大行高以容纳多行文本；恢复单行时恢复默认行高（默认行高为32）
            pListCtrl->SetDataItemHeight(bSingleLine ? 32 : 64, true);
        };
        pCellSingleLine->AttachSelect([this, OnCellWrap](const ui::EventArgs&) {
            OnCellWrap(true);
            return true;
            });
        pCellWordWrap->AttachSelect([this, OnCellWrap](const ui::EventArgs&) {
            OnCellWrap(false);
            return true;
            });
    }

    //是否支持多选
    ui::CheckBox* pMultiSelect = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_multi_select")));
    if (pMultiSelect != nullptr) {
        pMultiSelect->Selected(pListCtrl->IsMultiSelect(), false);
    }

    //所有列是否可编辑（勾选：所有行、所有列的数据均设置为可编辑；不勾选：全部不可编辑，默认状态）
    //勾选状态保存在成员变量中：数据被重新载入（切换语言、还原演示数据等）后会自动重新应用
    ui::CheckBox* pAllColumnsEditable = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_all_columns_editable")));
    if (pAllColumnsEditable != nullptr) {
        m_bAllColumnsEditable = (pListCtrl->GetDataItemCount() > 0) && pListCtrl->IsSubItemEditable(0, 0);
        pAllColumnsEditable->Selected(m_bAllColumnsEditable, false);
        pAllColumnsEditable->AttachSelect([this, pListCtrl](const ui::EventArgs&) {
            m_bAllColumnsEditable = true;
            SetAllColumnsEditable(pListCtrl, true);
            return true;
            });
        pAllColumnsEditable->AttachUnSelect([this, pListCtrl](const ui::EventArgs&) {
            m_bAllColumnsEditable = false;
            SetAllColumnsEditable(pListCtrl, false);
            return true;
            });
    }

    //选择方式：按行选择 / 按单元格选择（单元格模式仅Report视图有效）
    ui::Option* pSelectionModeRow = dynamic_cast<ui::Option*>(FindControl(_T("selection_mode_row")));
    ui::Option* pSelectionModeCell = dynamic_cast<ui::Option*>(FindControl(_T("selection_mode_cell")));
    if ((pSelectionModeRow != nullptr) && (pSelectionModeCell != nullptr)) {
        const bool bCellMode = (pListCtrl->GetSelectionMode() == ui::ListCtrlSelectionMode::kCell);
        pSelectionModeRow->Selected(!bCellMode, false);
        pSelectionModeCell->Selected(bCellMode, false);
    }
    //在列表头点击右键
    ui::ListCtrlHeader* pHeaderCtrl = pListCtrl->GetHeaderCtrl();
//    if (pHeaderCtrl != nullptr) {
//        pHeaderCtrl->AttachRClick([this](const ui::EventArgs&) {
//#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
//            if (::MessageBox(nullptr, _T("ListCtrlHeader RClick! 是否执行功能测试？"), _T(""), MB_YESNO) == IDYES) {
//                RunListCtrlTest();
//            }
//#else
//            ui::SystemUtil::ShowMessageBox(this, _T("开始执行功能测试"), _T("ListCtrlHeader RClick!"));
//            RunListCtrlTest();
//#endif
//            return true;
//            });
//    }

    //按比例设置各个列的宽度，使其充满整个视图
    ui::Button* pAutoStretchBtn = dynamic_cast<ui::Button*>(FindControl(_T("set_column_stretch")));
    if (pAutoStretchBtn != nullptr) {
        pAutoStretchBtn->AttachClick([pListCtrl, this](const ui::EventArgs& /*args*/) {
            std::vector<ui::UiFixedInt> columnWidthList;
            size_t nColumnCount = pListCtrl->GetColumnCount();
            for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; nColumnIndex++) {
                columnWidthList.push_back(ui::UiFixedInt::MakeStretch());
            }
            bool bRet = pListCtrl->SetColumnWidth(columnWidthList, true);
            UNUSED_VARIABLE(bRet);
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
        if (pListCtrl->GetDataItemCount() > 0) {
            pShowIcon->Selected(pListCtrl->GetDataItemImageId(0) >= 0, false);
        }
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
}

void MainForm::SetAllColumnsEditable(ui::ListCtrl* pListCtrl, bool bEditable)
{
    if (pListCtrl == nullptr) {
        return;
    }
    const size_t nItemCount = pListCtrl->GetDataItemCount();
    const size_t nColumnCount = pListCtrl->GetColumnCount();
    //批量修改期间关闭自动刷新，避免逐格触发刷新
    const bool bOldEnableRefresh = pListCtrl->SetEnableRefresh(false);
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
            pListCtrl->SetSubItemEditable(nItemIndex, nColumnIndex, bEditable);
        }
    }
    pListCtrl->SetEnableRefresh(bOldEnableRefresh);
    if (bOldEnableRefresh) {
        pListCtrl->Refresh();
    }
}

namespace
{
/** 高精度计时器：取当前计数值（Windows 使用 QueryPerformanceCounter 的原始 tick；其他平台使用 steady_clock 的微秒） */
int64_t GetPerfTick()
{
#if defined (DUILIB_BUILD_FOR_WIN)
    LARGE_INTEGER nTick = { 0 };
    ::QueryPerformanceCounter(&nTick);
    return nTick.QuadPart;
#else
    return (int64_t)std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
#endif
}

/** 把计数值的差值换算为毫秒 */
double PerfTickToMs(int64_t nTickCount)
{
#if defined (DUILIB_BUILD_FOR_WIN)
    LARGE_INTEGER nFreq = { 0 };
    ::QueryPerformanceFrequency(&nFreq);
    if (nFreq.QuadPart <= 0) {
        return 0.0;
    }
    return (double)nTickCount * 1000.0 / (double)nFreq.QuadPart;
#else
    return (double)nTickCount / 1000.0;
#endif
}

/** 读取当前进程内存占用；返回 false 表示当前平台不支持该指标（仅 Windows 提供）
    @param [out] nWorkingSet 当前工作集（字节）
    @param [out] nPeakWorkingSet 进程启动以来的峰值工作集（字节）
    @param [out] nPrivateUsage 私有提交内存（字节）
*/
bool GetProcessMemorySnapshot(uint64_t& nWorkingSet, uint64_t& nPeakWorkingSet, uint64_t& nPrivateUsage)
{
    nWorkingSet = 0;
    nPeakWorkingSet = 0;
    nPrivateUsage = 0;
#if defined (DUILIB_BUILD_FOR_WIN)
    PROCESS_MEMORY_COUNTERS_EX counters = { 0 };
    counters.cb = sizeof(counters);
    if (!::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&counters, sizeof(counters))) {
        return false;
    }
    nWorkingSet = (uint64_t)counters.WorkingSetSize;
    nPeakWorkingSet = (uint64_t)counters.PeakWorkingSetSize;
    nPrivateUsage = (uint64_t)counters.PrivateUsage;
    return true;
#else
    return false;
#endif
}
}//namespace

void MainForm::InitStressTest(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_load_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestLoad(pListCtrl);
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_insert_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestInsert(pListCtrl);
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_delete_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestDelete(pListCtrl);
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_delete_batch_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestDeleteBatch(pListCtrl);
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_compact_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestCompact(pListCtrl);
            return true;
            });
    }
    //C2：复制选中内容到剪贴板（TSV格式，与 Ctrl+C 效果一致）
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("copy_selection_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            const bool bRet = pListCtrl->CopySelectionToClipboard(false);
            ui::SystemUtil::ShowMessageBox(this,
                bRet ? _T("Copy to clipboard: OK (paste into Excel to verify)") :
                       _T("Copy to clipboard: FAILED (no selection, or enable_copy is false)"),
                ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_COPY_EXPORT_TITLE")));
            return true;
            });
    }
    //C2：导出CSV文件（仅导出当前选中内容；文件位于资源目录，可用 Excel 直接打开）
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("export_csv_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            ui::FilePath csvPath = ui::GlobalManager::GetResourceRootPath(false);
            csvPath /= ui::FilePath(_T("list_ctrl_export.csv"));
            const DString strFilePath = csvPath.ToString();
            const bool bRet = pListCtrl->ExportToCsvFile(strFilePath, true);
            DString msg;
            if (bRet) {
                msg = ui::StringUtil::Printf(_T("Export CSV: OK\nFile: %s"), strFilePath.c_str());
            }
            else {
                msg = _T("Export CSV: FAILED (no selection/data, or enable_csv_export is false)");
            }
            ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_COPY_EXPORT_TITLE")));
            return true;
            });
    }
    //C3：进入当前单元格的编辑状态（效果与选中单元格后按 F2 相同）
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("enter_cell_edit_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            const bool bRet = pListCtrl->EnterCurrentCellEdit();
            if (!bRet) {
                //仅失败时弹提示；成功时静默进入编辑（若弹模态框，主窗口会失去焦点，
                //进而向编辑框派发窗口失去焦点事件，刚进入的编辑态会被立即拆除）
                ui::SystemUtil::ShowMessageBox(this,
                    _T("Enter cell edit: FAILED (need cell selection mode, a current editable cell, and enable_item_edit)"),
                    ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_ENTER_CELL_EDIT")));
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_sort_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestSort(pListCtrl);
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_repaint_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestRepaint(pListCtrl);
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_restore_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestRestore(pListCtrl);
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_load_sparse_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestLoadSparse(pListCtrl);
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stress_memory_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pListCtrl](const ui::EventArgs&) {
            StressTestMemory(pListCtrl);
            return true;
            });
    }
}

void MainForm::StressTestLoad(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    const size_t nRows = 100000;    //10万行
    const size_t nColumns = 10;     //10列

    //批量数据操作期间关闭界面刷新，避免绘制干扰对数据层耗时的测量
    pListCtrl->SetEnableRefresh(false);
    pListCtrl->DeleteAllColumns();
    pListCtrl->DeleteAllDataItems();

    //1) 添加列
    const int64_t nTick0 = GetPerfTick();
    for (size_t nColumn = 0; nColumn < nColumns; ++nColumn) {
        ui::ListCtrlColumn columnInfo;
        columnInfo.nColumnWidth = 120;
        columnInfo.text = ui::StringUtil::Printf(_T("Col %zu"), nColumn);
        columnInfo.bShowCheckBox = false;
        columnInfo.nImageId = -1;
        pListCtrl->InsertColumn(-1, columnInfo);
    }
    const int64_t nTick1 = GetPerfTick();

    //2) 设置行数（虚表：仅记录行数，单元格数据按需填充）
    pListCtrl->SetDataItemCount(nRows);
    const int64_t nTick2 = GetPerfTick();

    //3) 填充单元格数据（nRows × nColumns）
    for (size_t nRow = 0; nRow < nRows; ++nRow) {
        for (size_t nColumn = 0; nColumn < nColumns; ++nColumn) {
            ui::ListCtrlSubItemData subItemData;
            subItemData.text = ui::StringUtil::Printf(_T("%zu-%zu"), nRow, nColumn);
            subItemData.bShowCheckBox = false;
            subItemData.nImageId = -1;
            pListCtrl->SetSubItemData(nRow, nColumn, subItemData);
        }
    }
    const int64_t nTick3 = GetPerfTick();

    pListCtrl->SetEnableRefresh(true);
    pListCtrl->Refresh(true);
    m_bStressDataLoaded = true;

    DString msg = ui::StringUtil::Printf(
        _T("Load stress data\nRows=%zu, Columns=%zu\n")
        _T("InsertColumn: %.1f ms\nSetDataItemCount: %.1f ms\nFill cells: %.1f ms\nTotal: %.1f ms"),
        nRows, nColumns,
        PerfTickToMs(nTick1 - nTick0),
        PerfTickToMs(nTick2 - nTick1),
        PerfTickToMs(nTick3 - nTick2),
        PerfTickToMs(nTick3 - nTick0));
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestLoadSparse(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    const size_t nRows = 100000;    //10万行（仅声明行数，不整体填充）
    const size_t nColumns = 10;     //10列
    const size_t nFillRows = 1000;  //稀疏填充：仅写第 0 列的前 1000 行

    pListCtrl->SetEnableRefresh(false);
    pListCtrl->DeleteAllColumns();
    pListCtrl->DeleteAllDataItems();

    //1) 添加列
    const int64_t nTick0 = GetPerfTick();
    for (size_t nColumn = 0; nColumn < nColumns; ++nColumn) {
        ui::ListCtrlColumn columnInfo;
        columnInfo.nColumnWidth = 120;
        columnInfo.text = ui::StringUtil::Printf(_T("Col %zu"), nColumn);
        columnInfo.bShowCheckBox = false;
        columnInfo.nImageId = -1;
        pListCtrl->InsertColumn(-1, columnInfo);
    }
    const int64_t nTick1 = GetPerfTick();

    //2) 仅设置行数（虚表：不写单元格，验证分页存储的懒分配——未写入的页不分配内存）
    pListCtrl->SetDataItemCount(nRows);
    const int64_t nTick2 = GetPerfTick();

    //3) 稀疏填充：只写第 0 列的前 nFillRows 行（其余单元格不写入）
    for (size_t nRow = 0; nRow < nFillRows; ++nRow) {
        ui::ListCtrlSubItemData subItemData;
        subItemData.text = ui::StringUtil::Printf(_T("s-%zu-0"), nRow);
        subItemData.bShowCheckBox = false;
        subItemData.nImageId = -1;
        pListCtrl->SetSubItemData(nRow, 0, subItemData);
    }
    const int64_t nTick3 = GetPerfTick();

    pListCtrl->SetEnableRefresh(true);
    pListCtrl->Refresh(true);
    m_bStressDataLoaded = true;

    DString msg = ui::StringUtil::Printf(
        _T("Load sparse data\nRows=%zu, Columns=%zu, Filled cells=%zu (column 0, first %zu rows)\n")
        _T("InsertColumn: %.1f ms\nSetDataItemCount: %.1f ms\nFill cells: %.1f ms\nTotal: %.1f ms"),
        nRows, nColumns, nFillRows, nFillRows,
        PerfTickToMs(nTick1 - nTick0),
        PerfTickToMs(nTick2 - nTick1),
        PerfTickToMs(nTick3 - nTick2),
        PerfTickToMs(nTick3 - nTick0));
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestMemory(ui::ListCtrl* pListCtrl)
{
    (void)pListCtrl;    //仅读取进程内存，不需要操作控件
    uint64_t nWorkingSet = 0;
    uint64_t nPeakWorkingSet = 0;
    uint64_t nPrivateUsage = 0;
    if (!GetProcessMemorySnapshot(nWorkingSet, nPeakWorkingSet, nPrivateUsage)) {
        ui::SystemUtil::ShowMessageBox(this,
            _T("Memory reading is not supported on this platform."),
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
        return;
    }
    const double dMB = 1024.0 * 1024.0;
    DString msg = ui::StringUtil::Printf(
        _T("Process memory\nWorking set: %.1f MB\nPeak working set: %.1f MB\nPrivate usage: %.1f MB\n")
        _T("\nNote: peak is measured since process start;\nrestart the app to measure one scenario alone."),
        (double)nWorkingSet / dMB,
        (double)nPeakWorkingSet / dMB,
        (double)nPrivateUsage / dMB);
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestInsert(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    if (!m_bStressDataLoaded) {
        ui::SystemUtil::ShowMessageBox(this,
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_NEED_LOAD")),
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
        return;
    }
    const size_t nInsertCount = 1000;   //插入1000行

    pListCtrl->SetEnableRefresh(false);
    const int64_t nTick0 = GetPerfTick();
    //始终在列表的中间位置插入（触发中间位置的搬移，验证增删改性能不降级）
    for (size_t n = 0; n < nInsertCount; ++n) {
        const size_t nInsertPos = pListCtrl->GetDataItemCount() / 2;
        ui::ListCtrlSubItemData subItemData;
        subItemData.text = _T("insert");
        subItemData.bShowCheckBox = false;
        subItemData.nImageId = -1;
        pListCtrl->InsertDataItem(nInsertPos, subItemData);
    }
    const int64_t nTick1 = GetPerfTick();
    pListCtrl->SetEnableRefresh(true);
    pListCtrl->Refresh(true);

    const double dTotalMs = PerfTickToMs(nTick1 - nTick0);
    DString msg = ui::StringUtil::Printf(
        _T("Insert rows at middle\nCount: %zu\nTotal: %.1f ms\nAverage: %.3f ms/row"),
        nInsertCount, dTotalMs, dTotalMs / (double)nInsertCount);
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestDelete(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    if (!m_bStressDataLoaded) {
        ui::SystemUtil::ShowMessageBox(this,
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_NEED_LOAD")),
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
        return;
    }
    const size_t nDeleteCount = 500;    //删除500行

    pListCtrl->SetEnableRefresh(false);
    const int64_t nTick0 = GetPerfTick();
    //循环删除中间位置的行（触发中间位置的搬移，验证批量删除性能不降级）
    for (size_t n = 0; n < nDeleteCount; ++n) {
        const size_t nDeleteIndex = pListCtrl->GetDataItemCount() / 2;
        pListCtrl->DeleteDataItem(nDeleteIndex);
    }
    const int64_t nTick1 = GetPerfTick();
    pListCtrl->SetEnableRefresh(true);
    pListCtrl->Refresh(true);

    const double dTotalMs = PerfTickToMs(nTick1 - nTick0);
    DString msg = ui::StringUtil::Printf(
        _T("Delete rows at middle\nCount: %zu\nTotal: %.1f ms\nAverage: %.3f ms/row"),
        nDeleteCount, dTotalMs, dTotalMs / (double)nDeleteCount);
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestDeleteBatch(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    if (!m_bStressDataLoaded) {
        ui::SystemUtil::ShowMessageBox(this,
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_NEED_LOAD")),
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
        return;
    }
    const size_t nDeleteCount = 500;    //批量删除500行
    const size_t nTotal = pListCtrl->GetDataItemCount();
    if (nTotal == 0) {
        return;
    }

    //构造500个"乱序"的待删除显示索引：均匀分布于整个列表，覆盖多个存储页，验证单趟压缩与页回收
    std::vector<size_t> deleteIndexs;
    deleteIndexs.reserve(nDeleteCount);
    const size_t nStep = (nTotal > nDeleteCount) ? (nTotal / nDeleteCount) : 1;
    for (size_t n = 0; n < nDeleteCount; ++n) {
        deleteIndexs.push_back((n * nStep) % nTotal);
    }

    pListCtrl->SetEnableRefresh(false);
    const int64_t nTick0 = GetPerfTick();
    const size_t nDeleted = pListCtrl->DeleteDataItems(deleteIndexs);
    const int64_t nTick1 = GetPerfTick();
    pListCtrl->SetEnableRefresh(true);
    pListCtrl->Refresh(true);

    const double dTotalMs = PerfTickToMs(nTick1 - nTick0);
    DString msg = ui::StringUtil::Printf(
        _T("Batch delete rows (unordered)\nRequested: %zu\nDeleted: %zu\nTotal: %.1f ms"),
        nDeleteCount, nDeleted, dTotalMs);
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestCompact(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    if (!m_bStressDataLoaded) {
        ui::SystemUtil::ShowMessageBox(this,
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_NEED_LOAD")),
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
        return;
    }

    //压缩前的内存快照
    uint64_t nWorkingSet0 = 0, nPeakWorkingSet0 = 0, nPrivateUsage0 = 0;
    GetProcessMemorySnapshot(nWorkingSet0, nPeakWorkingSet0, nPrivateUsage0);

    //执行存储压缩（重编号 + 整体压缩），并测量耗时
    pListCtrl->SetEnableRefresh(false);
    const int64_t nTick0 = GetPerfTick();
    pListCtrl->CompactStorage();
    const int64_t nTick1 = GetPerfTick();
    pListCtrl->SetEnableRefresh(true);
    pListCtrl->Refresh(true);

    //压缩后的内存快照
    uint64_t nWorkingSet1 = 0, nPeakWorkingSet1 = 0, nPrivateUsage1 = 0;
    GetProcessMemorySnapshot(nWorkingSet1, nPeakWorkingSet1, nPrivateUsage1);

    const double dTotalMs = PerfTickToMs(nTick1 - nTick0);
    const double dMb = 1024.0 * 1024.0;
    DString msg = ui::StringUtil::Printf(
        _T("Compact storage\nRows: %zu\nTotal: %.1f ms\nWorking set: %.1f -> %.1f MB\nPrivate: %.1f -> %.1f MB"),
        pListCtrl->GetDataItemCount(), dTotalMs,
        (double)nWorkingSet0 / dMb, (double)nWorkingSet1 / dMb,
        (double)nPrivateUsage0 / dMb, (double)nPrivateUsage1 / dMb);
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestSort(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    if (!m_bStressDataLoaded) {
        ui::SystemUtil::ShowMessageBox(this,
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_NEED_LOAD")),
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
        return;
    }
    if (pListCtrl->GetColumnCount() == 0) {
        return;
    }

    pListCtrl->SetEnableRefresh(false);
    const int64_t nTick0 = GetPerfTick();
    const bool bRet = pListCtrl->SortDataItems(0, true);    //按第1列升序排序（整表排序）
    const int64_t nTick1 = GetPerfTick();
    pListCtrl->SetEnableRefresh(true);
    pListCtrl->Refresh(true);

    DString msg = ui::StringUtil::Printf(
        _T("Sort whole table (column 0, ascending)\nResult: %s\nTotal: %.1f ms"),
        bRet ? _T("true") : _T("false"),
        PerfTickToMs(nTick1 - nTick0));
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestRepaint(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    if (!m_bStressDataLoaded) {
        ui::SystemUtil::ShowMessageBox(this,
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_NEED_LOAD")),
            ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
        return;
    }
    const int32_t nFrames = 20;         //滚动并重绘的帧数
    const size_t nStep = 200;           //每帧向下滚动的行数
    const size_t nDataCount = pListCtrl->GetDataItemCount();
    if (nDataCount == 0) {
        return;
    }

    double dTotalMs = 0.0;
    size_t nItemIndex = 0;
    for (int32_t nFrame = 0; nFrame < nFrames; ++nFrame) {
        if (nItemIndex >= nDataCount) {
            nItemIndex = 0;
        }
        //滚动到指定行（置顶），再强制同步重绘可见区域，测量"滚动 + 绘制"的单帧耗时
        pListCtrl->EnsureDataItemVisible(nItemIndex, true);
        const int64_t nTick0 = GetPerfTick();
        pListCtrl->Refresh(true);
        const int64_t nTick1 = GetPerfTick();
        dTotalMs += PerfTickToMs(nTick1 - nTick0);
        nItemIndex += nStep;
    }

    DString msg = ui::StringUtil::Printf(
        _T("Scroll & repaint\nFrames: %d\nTotal: %.1f ms\nAverage: %.3f ms/frame"),
        nFrames, dTotalMs, dTotalMs / (double)nFrames);
    ui::SystemUtil::ShowMessageBox(this, msg, ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_STRESS_TEST")));
}

void MainForm::StressTestRestore(ui::ListCtrl* pListCtrl)
{
    if (pListCtrl == nullptr) {
        return;
    }
    pListCtrl->SetEnableRefresh(false);
    pListCtrl->DeleteAllColumns();
    pListCtrl->DeleteAllDataItems();
    InsertItemData(400, 9, (int32_t)m_imageId);
    pListCtrl->SetEnableRefresh(true);
    pListCtrl->Refresh(true);
    m_bStressDataLoaded = false;
}

void MainForm::TestListCtrlLoading(ui::ListCtrl* pListCtrl)
{
    //测试loading功能
    ui::Button* pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_progress_btn1")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            if (!pListCtrl->IsLoading()) {
                pListCtrl->SetLoadingAttribute(_T("file='loading_progress1.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'"));
                OnTestLoadingProgress();
            }
            return true;
            });
    }

    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_progress_btn2")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            if (!pListCtrl->IsLoading()) {
                pListCtrl->SetLoadingAttribute(_T("file='loading_progress2.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'"));
                OnTestLoadingProgress();
            }
            return true;
            });
    }

    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_btn1")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            if (!pListCtrl->IsLoading()) {
                pListCtrl->SetLoadingAttribute(_T("file='loading1.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'"));
                //实际应用中，事件处理可参考OnTestLoadingProgress的逻辑
                pListCtrl->StartLoading(100, -1);
            }
            return true;
            });
    }
    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_btn2")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            if (!pListCtrl->IsLoading()) {
                pListCtrl->SetLoadingAttribute(_T("file='loading2.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'"));
                //实际应用中，事件处理可参考OnTestLoadingProgress的逻辑
                pListCtrl->StartLoading(100, -1);
            }
            return true;
            });
    }
    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_btn3")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            if (!pListCtrl->IsLoading()) {
                pListCtrl->SetLoadingAttribute(_T("file='loading3.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'"));
                //实际应用中，事件处理可参考OnTestLoadingProgress的逻辑
                pListCtrl->StartLoading(100, -1);
            }
            return true;
            });
    }
    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_btn4")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            if (!pListCtrl->IsLoading()) {
                pListCtrl->SetLoadingAttribute(_T("file='loading4.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'"));
                //实际应用中，事件处理可参考OnTestLoadingProgress的逻辑
                pListCtrl->StartLoading(100, -1);
            }
            return true;
            });
    }
    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_btn5")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            if (!pListCtrl->IsLoading()) {
                pListCtrl->SetLoadingAttribute(_T("file='loading5.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'"));
                //实际应用中，事件处理可参考OnTestLoadingProgress的逻辑
                pListCtrl->StartLoading(100, -1);
            }
            return true;
            });
    }
    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_btn6")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            if (!pListCtrl->IsLoading()) {
                pListCtrl->SetLoadingAttribute(_T("file='loading6.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'"));
                //实际应用中，事件处理可参考OnTestLoadingProgress的逻辑
                pListCtrl->StartLoading(100, -1);
            }
            return true;
            });
    }

    //骨架屏测试：列表骨架屏
    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_skeleton_btn1")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            OnTestLoadingSkeleton(false);
            return true;
            });
    }
    //骨架屏测试：卡片骨架屏
    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_skeleton_btn2")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            OnTestLoadingSkeleton(true);
            return true;
            });
    }
    //停止骨架屏占位
    pLoadingBtn = dynamic_cast<ui::Button*>(FindControl(_T("loading_stop_btn")));
    if (pLoadingBtn != nullptr) {
        pLoadingBtn->AttachClick([pListCtrl, this](const ui::EventArgs& args) {
            OnStopLoadingSkeleton();
            return true;
            });
    }
}

void MainForm::OnTestLoadingProgress()
{
    ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
    ASSERT(pListCtrl != nullptr);
    if (pListCtrl == nullptr) {
        return;
    }
    if (pListCtrl->IsLoading()) {
        return;
    }

    m_fLoadingPercent = 0;//当前进度，模拟值
    pListCtrl->DetachEvent(ui::kEventLoadingStart); //解除其他地方注册的回调，避免相互干扰
    pListCtrl->AttachLoadingStart([](const ui::EventArgs& args) {
        ui::ControlLoadingStatus* pLoadingStatus = (ui::ControlLoadingStatus*)args.wParam;
        if (pLoadingStatus != nullptr) {
            //获取关联控件的接口，使用前需要判断是否为nullptr
            //ui::ControlPtrT<ui::Control> pControl = pLoadingStatus->m_pControl;
            //ui::ControlPtrT<ui::Box> pLoadingUiRootBox = pLoadingStatus->m_pLoadingUiRootBox;
            //ui::ControlPtrT<ui::Control> pAnimationControl = pLoadingStatus->m_pAnimationControl;

        }
        return true;
        });
    pListCtrl->DetachEvent(ui::kEventLoading); //解除其他地方注册的回调，避免相互干扰
    pListCtrl->AttachLoading([pListCtrl, this](const ui::EventArgs& args) {
        if (!pListCtrl->IsLoading()) {
            //loading状态已经终止，停止派发该事件（当注册多个事件回调函数时，会遇到这种现象）
            return false;
        }
        bool bRet = true;
        ui::ControlLoadingStatus* pLoadingStatus = (ui::ControlLoadingStatus*)args.wParam;
        if (pLoadingStatus != nullptr) {
            //获取关联控件的接口，使用前需要判断是否为nullptr
            ui::ControlPtrT<ui::Control> pControl = pLoadingStatus->m_pControl;
            ui::ControlPtrT<ui::Box> pLoadingUiRootBox = pLoadingStatus->m_pLoadingUiRootBox;
            ui::ControlPtrT<ui::Control> pAnimationControl = pLoadingStatus->m_pAnimationControl;

            //设置进度
            bool bFinished = false;
            if (pAnimationControl != nullptr) {
                uint32_t nFrameCount = pAnimationControl->GetImageAnimationFrameCount();
                uint32_t nFrameIndex = (uint32_t)(m_fLoadingPercent * nFrameCount / 100);
                if (nFrameIndex >= nFrameCount) {
                    //完成
                    nFrameIndex = nFrameCount - 1;
                    bFinished = true;
                }
                pAnimationControl->SetImageAnimationFrame(nFrameIndex);
            }
            if (bFinished) {
                //停止加载中的状态
                pLoadingStatus->m_bStopLoading = true;
            }
            else {
                m_fLoadingPercent += 0.35f;//当前进度，模拟值
            }
        }
        return bRet;
        });
    pListCtrl->DetachEvent(ui::kEventLoadingStop); //解除其他地方注册的回调，避免相互干扰
    pListCtrl->AttachLoadingStop([pListCtrl](const ui::EventArgs& args) {
        ui::ControlLoadingStatus* pLoadingStatus = (ui::ControlLoadingStatus*)args.wParam;
        if (pLoadingStatus != nullptr) {
            //获取关联控件的接口，使用前需要判断是否为nullptr
            //ui::ControlPtrT<ui::Control> pControl = pLoadingStatus->m_pControl;
            //ui::ControlPtrT<ui::Box> pLoadingUiRootBox = pLoadingStatus->m_pLoadingUiRootBox;
            //ui::ControlPtrT<ui::Control> pAnimationControl = pLoadingStatus->m_pAnimationControl;

        }
        //解除注册的回调，避免影响其他业务
        pListCtrl->DetachEvent(ui::kEventLoadingStart);
        pListCtrl->DetachEvent(ui::kEventLoading);
        pListCtrl->DetachEvent(ui::kEventLoadingStop);
        return true;
        });

    pListCtrl->StartLoading(33, -1);
}

void MainForm::OnTestLoadingSkeleton(bool bCardLayout)
{
    ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
    ASSERT(pListCtrl != nullptr);
    if (pListCtrl == nullptr) {
        return;
    }
    //若上一轮骨架屏因异常卡在loading状态，先强制复位，否则后续所有点击都会被这里挡住
    if (pListCtrl->IsLoading()) {
        pListCtrl->StopLoading();
    }

    //骨架屏的占位界面完全由XML描述，C++侧只负责指定皮肤与超时兜底：
    //  width='0' height='0' + 皮肤根容器的 stretch = 铺满ListCtrl（clamp 生效），无需算尺寸
    //  不指定 animation_control：骨架屏是静态占位块，不存在"动画播完"的时刻
    //  不设置 auto_stop：必须由业务侧根据 m_nElapsedTimeMs 主动结束
    const DString strSkin = bCardLayout ? _T("loading_skeleton_card.xml") : _T("loading_skeleton_list.xml");
    const DString strLoading = _T("file='") + strSkin + _T("' width='0' height='0' offset_x='-1' offset_y='-1' ") +
        _T("valign='center' halign='center' fade='255'");
    if (!pListCtrl->SetLoadingAttribute(strLoading)) {
        OutputDebugLog(_T("SetLoadingAttribute failed, skin=") + strSkin);
        return;
    }

    pListCtrl->DetachEvent(ui::kEventLoadingStart);
    pListCtrl->DetachEvent(ui::kEventLoading);
    pListCtrl->DetachEvent(ui::kEventLoadingStop);

    pListCtrl->AttachLoadingStart([this](const ui::EventArgs& args) {
        ui::ControlLoadingStatus* pLoadingStatus = (ui::ControlLoadingStatus*)args.wParam;
        if (pLoadingStatus != nullptr) {
            //骨架屏的占位根容器，可用于动态增删占位块。
            //注意：StopLoading时这个Box会被销毁，句柄随之失效，不能跨事件保存。
            ui::ControlPtrT<ui::Box> pLoadingUiRootBox = pLoadingStatus->m_pLoadingUiRootBox;
            OutputDebugLog(_T("skeleton loading start"));
        }
        return true;
        });

    pListCtrl->AttachLoading([pListCtrl, this](const ui::EventArgs& args) {
        bool bRet = true;
        ui::ControlLoadingStatus* pLoadingStatus = (ui::ControlLoadingStatus*)args.wParam;
        if (pLoadingStatus == nullptr) {
            return false;
        }

        //超时兜底：nMaxCount只是回调次数上限，真正的时间超时必须自己判断。
        //没有这一步，骨架屏会一直停在那里，比转圈更糟——用户失去了"还在加载"的确信感。
        const uint64_t nTimeoutMs = 5000;
        if (pLoadingStatus->m_nElapsedTimeMs >= nTimeoutMs) {
            pLoadingStatus->m_bStopLoading = true;
            return true;
        }

        //呼吸动画：让"占位块"呼吸，**不能给根容器设 alpha**。
        //原因：ControlLoading 的 loading 层是叠加绘制而不是遮罩，根容器的 bkcolor 承担遮住
        //下方列表内容的职责。一旦根容器 alpha < 255，底板也跟着半透明，下方内容会透出来
        //（表现为骨架屏只有灰块、下方旧列表隐约可见）。
        //正确做法：根容器固定 255，只遍历它的子控件（占位块）调 SetAlpha。
        //如需"尊重减少动态效果"的无障碍设置，可在此处判断系统偏好后跳过本段。
        ui::ControlPtrT<ui::Box> pLoadingUiRootBox = pLoadingStatus->m_pLoadingUiRootBox;
        if (pLoadingUiRootBox != nullptr) {
            const double fSeconds = (double)pLoadingStatus->m_nElapsedTimeMs / 1000.0;
            //约2秒一个周期，alpha 在 140~255 之间起伏（不降到太低，保持占位块清晰可辨）
            const uint8_t nAlpha = (uint8_t)(197.5 + 57.5 * std::sin(fSeconds * 3.1415926));
            for (size_t i = 0; i < pLoadingUiRootBox->GetItemCount(); ++i) {
                ui::Control* pChild = pLoadingUiRootBox->GetItemAt(i);
                if (pChild != nullptr) {
                    pChild->SetAlpha(nAlpha);
                }
            }
        }
        return bRet;
        });

    pListCtrl->AttachLoadingStop([pListCtrl](const ui::EventArgs& args) {
        //解除注册的回调，避免影响其他业务
        pListCtrl->DetachEvent(ui::kEventLoadingStart);
        pListCtrl->DetachEvent(ui::kEventLoading);
        pListCtrl->DetachEvent(ui::kEventLoadingStop);
        return true;
        });

    //33ms一次心跳；nMaxCount=-1表示不靠次数停止，完全由上面的超时逻辑控制
    if (!pListCtrl->StartLoading(33, -1)) {
        OutputDebugLog(_T("StartLoading failed, skin=") + strSkin);
    }
}

void MainForm::OnStopLoadingSkeleton()
{
    ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
    ASSERT(pListCtrl != nullptr);
    if (pListCtrl == nullptr) {
        return;
    }
    //注意：Control::StartLoading 内部会 SetEnabled(false)，Control::StopLoading 内部会
    // SetEnabled(true)。如果骨架屏因为任何原因卡在 loading 状态（例如 XML 解析失败导致
    // 定时器没跑起来），ListCtrl 会一直处于禁用状态，下方整个列表区域都点不动。
    //所以这里不判断 IsLoading()，无条件停止并恢复可用状态，作为兜底。
    pListCtrl->StopLoading();
    pListCtrl->SetEnabled(true);
    OutputDebugLog(_T("skeleton loading stopped manually"));
}

void MainForm::OnColumnChanged(size_t nColumnId)
{
    ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
    ASSERT(pListCtrl != nullptr);
    if (pListCtrl == nullptr) {
        return;
    }
    if (pListCtrl->GetDataItemCount() == 0) {
        //无数据
        return;
    }

    ui::ListCtrlHeaderItem* pHeaderItem = pListCtrl->GetColumnById(nColumnId);
    ASSERT(pHeaderItem != nullptr);
    if (pHeaderItem == nullptr) {
        return;
    }


    ui::CheckBox* pColumnShow = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show")));
    ui::CheckBox* pColumnWidth = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_width")));
    ui::CheckBox* pColumnSort = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_sort")));
    ui::CheckBox* pColumnIcon = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_icon_at_top")));
    ui::CheckBox* pColumnDragOrder = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_drag_order")));
    ui::CheckBox* pColumnEditable = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_editable")));
    ui::CheckBox* pColumnHeaderCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show_header_checkbox")));
    ui::CheckBox* pColumnShowCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show_checkbox")));

    ui::CheckBox* pColumnHeaderIcon = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show_header_icon")));
    ui::CheckBox* pColumnShowIcon = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_column_show_icon")));

    ui::Option* pColumnHeaderTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(_T("header_text_align_left")));
    ui::Option* pColumnHeaderTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(_T("header_text_align_center")));
    ui::Option* pColumnHeaderTextAlignRight = dynamic_cast<ui::Option*>(FindControl(_T("header_text_align_right")));

    ui::Option* pColumnTextAlignLeft = dynamic_cast<ui::Option*>(FindControl(_T("column_text_align_left")));
    ui::Option* pColumnTextAlignCenter = dynamic_cast<ui::Option*>(FindControl(_T("column_text_align_center")));
    ui::Option* pColumnTextAlignRight = dynamic_cast<ui::Option*>(FindControl(_T("column_text_align_right")));

    ASSERT(pHeaderItem->IsColumnVisible() == pHeaderItem->IsVisible());
    pColumnShow->Selected(pHeaderItem->IsColumnVisible(), false);
    pColumnWidth->Selected(pHeaderItem->IsColumnResizeable(), false);

    ui::ListCtrlHeaderItem::SortMode sortMode = pHeaderItem->GetSortMode();
    pColumnSort->Selected(sortMode != ui::ListCtrlHeaderItem::SortMode::kNone, false);

    pColumnIcon->Selected(pHeaderItem->IsShowIconAtTop(), false);
    pColumnDragOrder->Selected(pHeaderItem->IsEnableDragOrder(), false);
    pColumnHeaderCheckBox->Selected(pHeaderItem->IsShowCheckBox(), false);
    pColumnHeaderIcon->SetSelected(pHeaderItem->GetImageId() >= 0);

    bool bColumnDataHasCheckBox = false;
    bool bColumnDataHasIcon = false;
    bool bColumnEditable = pListCtrl->IsSubItemEditable(0, 0);
    ui::ListCtrlItem* pItem = pListCtrl->GetFirstDisplayItem();
    if (pItem != nullptr) {
        ui::ListCtrlSubItem* pSubItem = pItem->GetSubItem(pListCtrl->GetColumnIndex(nColumnId));
        if (pSubItem != nullptr) {
            bColumnDataHasCheckBox = pSubItem->IsShowCheckBox();
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
    if (hAlignType == ui::HorAlignType::kAlignCenter) {
        pColumnHeaderTextAlignCenter->Selected(true, false);
    }
    else if (hAlignType == ui::HorAlignType::kAlignRight) {
        pColumnHeaderTextAlignRight->Selected(true, false);
    }
    else {
        pColumnHeaderTextAlignLeft->Selected(true, false);
    }

    int32_t nTextFormat = pListCtrl->GetSubItemTextFormat(0, pListCtrl->GetColumnIndex(nColumnId));
    if (nTextFormat & ui::TEXT_HCENTER) {
        pColumnTextAlignCenter->Selected(true, false);
    }
    else if (nTextFormat & ui::TEXT_RIGHT) {
        pColumnTextAlignRight->Selected(true, false);
    }
    else {
        pColumnTextAlignLeft->Selected(true, false);
    }

    //单元格换行状态：与当前列单元格的文本格式(TEXT_SINGLELINE位)保持一致
    ui::Option* pCellSingleLine = dynamic_cast<ui::Option*>(FindControl(_T("cell_single_line")));
    ui::Option* pCellWordWrap = dynamic_cast<ui::Option*>(FindControl(_T("cell_word_wrap")));
    if ((pCellSingleLine != nullptr) && (pCellWordWrap != nullptr)) {
        bool bSingleLine = (nTextFormat & ui::TEXT_SINGLELINE) != 0;
        pCellSingleLine->Selected(bSingleLine, false);
        pCellWordWrap->Selected(!bSingleLine, false);
    }
}

void MainForm::InsertItemData(int32_t nRows, int32_t nColumns, int32_t nImageId)
{
    ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
    ASSERT(pListCtrl != nullptr);
    if (pListCtrl == nullptr) {
        return;
    }

    const size_t columnCount = nColumns;
    const size_t rowCount = nRows;
    bool bShowCheckBox = true; //是否显示CheckBox
    //添加列
    const DString fmtHeader = ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_DATA_COLUMN"));
    for (size_t i = 0; i < columnCount; ++i) {
        ui::ListCtrlColumn columnInfo;
        columnInfo.nColumnWidth = 200;
        //columnInfo.nTextFormat = TEXT_LEFT | TEXT_VCENTER;        
        columnInfo.text = ui::StringUtil::Printf(fmtHeader.c_str(), i);//_T("第 %d 列")
        columnInfo.bShowCheckBox = bShowCheckBox;
        columnInfo.nImageId = nImageId;
        pListCtrl->InsertColumn(-1, columnInfo);
    }
    //演示：列级比较模式（对Report视图的表头点击排序生效）
    //说明：排序方式按列声明（SetColumnSortFlag/SetColumnSortFlagById），表头点击排序时会自动使用该列声明的比较模式，
    //      从而由"列的语义"决定比较方式，而不是按单元格当前状态推断
    pListCtrl->SetColumnSortFlag(1, ui::ListCtrlSubItemSortFlag::kSortByProgress);      //第2列（进度条列）按进度值排序
    if (columnCount > 3) {
        pListCtrl->SetColumnSortFlag(3, ui::ListCtrlSubItemSortFlag::kSortByTextAsNumber);//第4列（数值文本列）按数值大小排序
    }
    if (columnCount > 4) {
        pListCtrl->SetColumnSortFlag(4, ui::ListCtrlSubItemSortFlag::kSortByTextAsDate);//第5列（日期文本列）按日期时间先后排序
    }
    //C3：按列声明单元格编辑器类型（提交编辑时按该类型校验输入内容）
    //      kEditNumber：仅允许空串或合法的十进制数值；kEditDate：仅允许空串或 yyyy-mm-dd / yyyy/mm/dd
    pListCtrl->SetColumnEditorType(1, ui::ListCtrlEditorType::kEditNumber);      //第2列（进度条列）按数值校验
    if (columnCount > 3) {
        pListCtrl->SetColumnEditorType(3, ui::ListCtrlEditorType::kEditNumber);//第4列（数值文本列）按数值校验
    }
    if (columnCount > 4) {
        pListCtrl->SetColumnEditorType(4, ui::ListCtrlEditorType::kEditDate);  //第5列（日期文本列）按日期校验
    }
    //填充数据
    pListCtrl->SetDataItemCount(rowCount);
    ASSERT(pListCtrl->GetDataItemCount() == rowCount);
    const DString fmtData = ui::GlobalManager::GetTextById(_T("STRID_LISTCTRL_DATA_ROW_COLUMN"));
    for (size_t itemIndex = 0; itemIndex < rowCount; ++itemIndex) {
        for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
            ui::ListCtrlSubItemData subItemData;
            subItemData.text = ui::StringUtil::Printf(fmtData.c_str(), itemIndex, columnIndex);
            subItemData.bShowCheckBox = bShowCheckBox;
            subItemData.nImageId = nImageId;
            if (columnIndex == 0) {
                subItemData.text += _T("-1234567890-ABCDEFG-1234567890-abcdefg");
            }
            else if (columnIndex == 1) {
                //演示：单元格进度条（第2列按进度条方式显示）
                subItemData.nProgress = (int32_t)((itemIndex * 7 + 15) % 101);
                subItemData.text = ui::StringUtil::Printf(_T("%d%%"), subItemData.nProgress);
                subItemData.nImageId = -1;
            }
            else if (columnIndex == 2) {
                //演示：单元格富文本（第3列按富文本方式显示）
                subItemData.bRichText = true;
                subItemData.text = ui::StringUtil::Printf(_T("<b>Row %03d</b> <font color=\"#1E90FF\">Col %02d</font>"), itemIndex, columnIndex);
                subItemData.nImageId = -1;
            }
            else if (columnIndex == 3) {
                //演示：列级比较模式中的"数值文本"（见 SetColumnSortFlag(kSortByTextAsNumber)）
                //文本为纯数字，点击该列表头时按数值大小排序（如 10 > 9），而不是按字典序（如 "10" < "9"）
                subItemData.text = ui::StringUtil::Printf(_T("%d"), (int32_t)((itemIndex * 13 + 7) % 1000));
            }
            else if (columnIndex == 4) {
                //演示：列级比较模式中的"日期时间文本"（见 SetColumnSortFlag(kSortByTextAsDate)）
                //日期不补零，点击该列表头时按日期先后排序（如 2020-3-7 在 2020-10-1 之前），
                //而不是按字典序（字典序会认为 "2020-10-1" 在 "2020-3-7" 之前）
                int32_t nYear = 2020 + (int32_t)((itemIndex * 7) % 5);
                int32_t nMonth = 1 + (int32_t)((itemIndex * 5) % 12);
                int32_t nDay = 1 + (int32_t)((itemIndex * 11) % 28);
                subItemData.text = ui::StringUtil::Printf(_T("%d-%d-%d"), nYear, nMonth, nDay);
            }
            pListCtrl->SetSubItemData(itemIndex, columnIndex, subItemData);
        }
    }
    //演示：冻结首列（与界面上的"冻结首列"选项状态保持一致）
    pListCtrl->SetFrozenColumnCount(1);
    //排序，默认为升序
    pListCtrl->SortDataItems(0, true);

    //设置特殊属性
    if (nRows > 100) {
        pListCtrl->SetDataItemAlwaysAtTop(100, 0);
        pListCtrl->SetDataItemAlwaysAtTop(101, 1);

        pListCtrl->SetDataItemVisible(103, false);
        pListCtrl->SetDataItemHeight(1, 100, true);
        pListCtrl->SetDataItemHeight(2, 200, true);

        //pListCtrl->SetSubItemBkColor(100, 0, ui::UiColor(ui::UiColors::MistyRose));
        //pListCtrl->SetSubItemBkColor(101, 0, ui::UiColor(ui::UiColors::MistyRose));
        //pListCtrl->SetSubItemBkColor(102, 0, ui::UiColor(ui::UiColors::MistyRose));
        //pListCtrl->SetSubItemBkColor(103, 0, ui::UiColor(ui::UiColors::MistyRose));
        //pListCtrl->SetSubItemBkColor(104, 0, ui::UiColor(ui::UiColors::MistyRose));
    }
    //保持"所有列可编辑"勾选框的状态：数据被重新载入后，按当前状态重新应用一次
    if (m_bAllColumnsEditable) {
        SetAllColumnsEditable(pListCtrl, true);
    }
    //重绘
    UpdateWindow();
}

void MainForm::TestRefreshSubItem(ui::ListCtrl* pListCtrl)
{
    //演示：单格刷新（RefreshSubItem）
    //场景：进度类列（第 2 列，列索引为 1）的进度来自后台任务，应用层自己维护进度值，
    //      不写入 ListCtrl 的数据模型，而是在单元格填充回调(kEventReportViewSubItemFilled)中设置显示内容；
    //      后台进度更新时，只调用 RefreshSubItem 刷新对应的单元格，不刷新整行。
    if (pListCtrl == nullptr) {
        return;
    }
    const size_t nProgressColumnIndex = 1;  //进度列的索引号（第 2 列）
    const size_t nMaxTestRowCount = 20;     //参与演示的行数（避免演示数据过多影响性能）
    const size_t nDataCount = pListCtrl->GetDataItemCount();

    //初始化进度值（模拟后台数据源，取值范围[0, 100]，与 InsertItemData 中的演示数据保持一致）
    m_progressValues.assign(nDataCount, 0);
    for (size_t nRow = 0; nRow < nDataCount; ++nRow) {
        m_progressValues[nRow] = (int32_t)((nRow * 7 + 15) % 101);
    }

    //在单元格填充回调中：按数据项索引取出应用层维护的进度值，设置到该单元格上
    pListCtrl->AttachReportViewSubItemFilled([this, nProgressColumnIndex](const ui::EventArgs& args) {
        if (args.eventType != ui::kEventReportViewSubItemFilled) {
            return true;
        }
        ui::ListCtrlSubItem* pSubItem = (ui::ListCtrlSubItem*)args.pEventData;
        if (pSubItem == nullptr) {
            return true;
        }
        if (pSubItem->GetDataColumnIndex() != nProgressColumnIndex) {
            //仅处理进度列
            return true;
        }
        size_t nRowIndex = pSubItem->GetDataItemIndex();
        if (nRowIndex >= m_progressValues.size()) {
            return true;
        }
        //设置进度条的进度值（-1 表示不显示进度条）和显示的文本
        int32_t nProgress = m_progressValues[nRowIndex];
        pSubItem->SetProgress(nProgress);
        pSubItem->SetText(ui::StringUtil::Printf(_T("%d%%"), nProgress));
        return true;
        });

    //定时器回调：在UI线程中执行，可以安全访问控件
    auto refreshCallback = [this, pListCtrl, nProgressColumnIndex, nMaxTestRowCount]() {
        for (size_t nRow = 0; nRow < nMaxTestRowCount; ++nRow) {
            if (nRow >= m_progressValues.size()) {
                break;
            }
            //模拟后台进度推进（到达 100 后回到 0，循环演示）
            int32_t nProgress = m_progressValues[nRow] + 5;
            if (nProgress > 100) {
                nProgress = 0;
            }
            m_progressValues[nRow] = nProgress;

            //只刷新这一个单元格（如果该单元格不在可见范围内，RefreshSubItem 返回 false，无需额外处理）
            pListCtrl->RefreshSubItem(nRow, nProgressColumnIndex);
        }
    };
    //每 800ms 触发一次，重复执行；定时器与ListCtrl的生命周期绑定，控件销毁后自动取消
    ui::GlobalManager::Instance().Timer().AddTimer(pListCtrl->GetWeakFlag(), refreshCallback, 800);
}

void MainForm::TestColumnVisibleApi(ui::ListCtrl* pListCtrl)
{
    //测试：ListCtrl级别的列显隐接口（SetColumnVisible/IsColumnVisible）与列显隐变化事件（kEventColumnVisibleChanged）
    //说明：界面上"第2列显隐"的两个选项，通过ListCtrl::SetColumnVisible接口来隐藏/显示第2列
    ui::Option* pHideColumn = dynamic_cast<ui::Option*>(FindControl(_T("column_visible_hide")));
    ui::Option* pShowColumn = dynamic_cast<ui::Option*>(FindControl(_T("column_visible_show")));
    ASSERT(pHideColumn != nullptr);
    ASSERT(pShowColumn != nullptr);
    if ((pListCtrl == nullptr) || (pHideColumn == nullptr) || (pShowColumn == nullptr)) {
        return;
    }
    const size_t nColumnIndex = 1;  //第2列（列索引为1）
    const size_t nColumnId = pListCtrl->GetColumnId(nColumnIndex);
    if (nColumnId == ui::Box::InvalidIndex) {
        return;
    }

    //按该列的实际显示状态，同步界面上选项的选中状态
    auto updateOptionSelected = [pListCtrl, nColumnIndex, pHideColumn, pShowColumn]() {
        bool bColumnVisible = pListCtrl->IsColumnVisible(nColumnIndex);
        pHideColumn->Selected(!bColumnVisible, false);
        pShowColumn->Selected(bColumnVisible, false);
        };
    updateOptionSelected();

    //监听列显隐变化事件：该列的显示状态变化时（包括从其他入口变化，比如表头项接口），同步界面的选中状态
    pListCtrl->AttachColumnVisibleChanged([nColumnId, updateOptionSelected](const ui::EventArgs& args) {
        if (args.wParam == (WPARAM)nColumnId) {
            updateOptionSelected();
        }
        return true;
        });

    //隐藏第2列
    pHideColumn->AttachSelect([pListCtrl, nColumnIndex](const ui::EventArgs&) {
        pListCtrl->SetColumnVisible(nColumnIndex, false);
        return true;
        });
    //显示第2列
    pShowColumn->AttachSelect([pListCtrl, nColumnIndex](const ui::EventArgs&) {
        pListCtrl->SetColumnVisible(nColumnIndex, true);
        return true;
        });
    //判断第2列当前是否显示：pListCtrl->IsColumnVisible(nColumnIndex)
}

void MainForm::RunListCtrlTest()
{
    ui::ListCtrl* pListCtrl = dynamic_cast<ui::ListCtrl*>(FindControl(_T("list_ctrl")));
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
    const DString text = _T("1");
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
    subItemData.text = _T("3");
    subItemData.textColor = ui::UiColor(ui::UiColors::Crimson);
    subItemData.bkColor = ui::UiColor(ui::UiColors::BlanchedAlmond);
    subItemData.bShowCheckBox = false;
    subItemData.nImageId = 123;
    subItemData.nTextFormat = ui::TEXT_HCENTER | ui::TEXT_VCENTER;
    pListCtrl->SetSubItemData(nDataItemIndex, nColumnIndex, subItemData);

    ui::ListCtrlSubItemData dataItem2;
    pListCtrl->GetSubItemData(nDataItemIndex, nColumnIndex, dataItem2);
    ASSERT(subItemData.text == DString(dataItem2.text));
    ASSERT(subItemData.textColor == dataItem2.textColor);
    ASSERT(subItemData.bkColor == dataItem2.bkColor);
    ASSERT(subItemData.bShowCheckBox == dataItem2.bShowCheckBox);
    ASSERT(subItemData.nImageId == dataItem2.nImageId);
    ASSERT(subItemData.nTextFormat == dataItem2.nTextFormat);

    ASSERT(pListCtrl->GetSubItemText(nDataItemIndex, nColumnIndex) == _T("3"));

    subItemData.text = _T("2");
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

    int32_t nOldValue = pListCtrl->GetDataItemImageId(nDataItemIndex);
    pListCtrl->SetDataItemImageId(nDataItemIndex, 666);
    ASSERT(pListCtrl->GetDataItemImageId(nDataItemIndex) == 666);
    pListCtrl->SetDataItemImageId(nDataItemIndex, nOldValue);

    nOldValue = pListCtrl->GetSubItemImageId(nDataItemIndex, nColumnIndex);
    pListCtrl->SetSubItemImageId(nDataItemIndex, nColumnIndex, 667);
    ASSERT(pListCtrl->GetSubItemImageId(nDataItemIndex, nColumnIndex) == 667);
    pListCtrl->SetSubItemImageId(nDataItemIndex, nColumnIndex, nOldValue);

    subItemData.text = _T("3");
    nColumnIndex = 0;
    pListCtrl->InsertDataItem(nDataItemIndex, subItemData);
    ASSERT(pListCtrl->GetSubItemText(nDataItemIndex, nColumnIndex) == _T("3"));
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
    DString text60 = pListCtrl->GetSubItemText(60, 0);
    pListCtrl->SetDataItemSelected(60, true);
    ASSERT(pListCtrl->IsDataItemSelected(60));

    ui::ListCtrlSubItemData dataItem3;
    nColumnIndex = 0;
    dataItem3.text = _T("Test");
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

    pListCtrl->DeleteAllColumns();
    pListCtrl->DeleteAllDataItems();

    InsertItemData((int32_t)nRows, (int32_t)nColumns, m_imageId);
#endif
}

void MainForm::TestListCtrlEvents(ui::ListCtrl* pListCtrl)
{
    //事件挂载，测试事件接口
    auto OnListCtrlEvent = [this, pListCtrl](const ui::EventArgs& args) {
        ASSERT(pListCtrl == args.GetSender());
        DString sInfo = GetEventDisplayInfo(args);
        OutputDebugLog(sInfo);
        };

    //挂载事件，转接给外层
    pListCtrl->AttachSelect([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });
    pListCtrl->AttachSelChanged([this, OnListCtrlEvent](const ui::EventArgs& args) {
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
    pListCtrl->AttachItemMouseEnter([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });
    pListCtrl->AttachItemMouseLeave([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });
    pListCtrl->AttachSubItemMouseEnter([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        OnReportViewSubItemMouseEnter(args);
        return true;
        });
    pListCtrl->AttachSubItemMouseLeave([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        OnReportViewSubItemMouseLeave(args);
        return true;
        });
    pListCtrl->AttachReturn([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });
    pListCtrl->AttachKeyDown([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });
    pListCtrl->AttachKeyUp([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });
    pListCtrl->AttachViewTypeChanged([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });
    pListCtrl->AttachViewPosChanged([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });
    pListCtrl->AttachViewSizeChanged([this, OnListCtrlEvent](const ui::EventArgs& args) {
        OnListCtrlEvent(args);
        return true;
        });

    auto OnListCtrlItemFilledEvent = [this, pListCtrl](const ui::EventArgs& args) {
        ASSERT(pListCtrl == args.GetSender());
        DString sInfo = GetItemFilledEventDisplayInfo(args);
        OutputDebugLog(sInfo);
        };
    pListCtrl->AttachReportViewItemFilled([this, OnListCtrlItemFilledEvent](const ui::EventArgs& args) {
        OnListCtrlItemFilledEvent(args);
        return true;
        });
    pListCtrl->AttachReportViewSubItemFilled([this, OnListCtrlItemFilledEvent](const ui::EventArgs& args) {
        OnListCtrlItemFilledEvent(args);
        OnReportViewSubItemFilled(args);
        return true;
        });
    pListCtrl->AttachListViewItemFilled([this, OnListCtrlItemFilledEvent](const ui::EventArgs& args) {
        OnListCtrlItemFilledEvent(args);
        return true;
        });
    pListCtrl->AttachIconViewItemFilled([this, OnListCtrlItemFilledEvent](const ui::EventArgs& args) {
        OnListCtrlItemFilledEvent(args);
        return true;
        });
}

DString MainForm::GetEventDisplayInfo(const ui::EventArgs& args)
{
    DString sInfo = ui::EventUtils::EventTypeToString(args.eventType);
    while (sInfo.size() < 24) {
        sInfo += _T(" ");
    }
    if ((args.eventType == ui::kEventSelect) ||
        (args.eventType == ui::kEventSelChanged) ||
        (args.eventType == ui::kEventMouseDoubleClick) ||
        (args.eventType == ui::kEventClick) ||
        (args.eventType == ui::kEventRClick) ||
        (args.eventType == ui::kEventItemMouseEnter) ||
        (args.eventType == ui::kEventItemMouseLeave) ||
        (args.eventType == ui::kEventSubItemMouseEnter) ||
        (args.eventType == ui::kEventSubItemMouseLeave) ||
        (args.eventType == ui::kEventReturn) ||
        (args.eventType == ui::kEventKeyDown) ||
        (args.eventType == ui::kEventKeyUp)) {
        DString labelText;
        int32_t nDataItemIndex = -1;
        int32_t nDataColumnIndex = -1;
        ui::ListCtrlType listCtrlType = (ui::ListCtrlType)args.listCtrlType;
        if (listCtrlType == ui::ListCtrlType::Report) {
            sInfo += _T("ListCtrlType::Report: ");
            ui::ListCtrlItem* pItem = nullptr;
            if ((args.eventType == ui::kEventSubItemMouseEnter) || (args.eventType == ui::kEventSubItemMouseLeave)) {
                ui::ListCtrlSubItem* pSubItem = (ui::ListCtrlSubItem*)args.pEventData;
                if (pSubItem != nullptr) {
                    pItem = pSubItem->GetListCtrlItem();

                    nDataColumnIndex = (int32_t)pSubItem->GetDataColumnIndex();
                    labelText = pSubItem->GetText();
                }
            }
            else {
                pItem = (ui::ListCtrlItem*)args.pEventData;
            }
            if (pItem != nullptr) {
                nDataItemIndex = (int32_t)pItem->GetDataItemIndex();
                if ((args.eventType >= ui::kEventMouseBegin) && (args.eventType <= ui::kEventMouseEnd)) {
                    //鼠标消息：获取当前列(根据当前鼠标所在的位置，获取列所在的子控件)
                    ui::ListCtrlSubItem* pSubItem = pItem->GetSubItem(args.ptMouse);
                    if (pSubItem != nullptr) {
                        nDataColumnIndex = (int32_t)pSubItem->GetDataColumnIndex();
                        labelText = pSubItem->GetText();
                    }
                }
            }
        }
        else if (listCtrlType == ui::ListCtrlType::Icon) {
            sInfo += _T("ListCtrlType::Icon: ");
            ui::ListCtrlIconViewItem* pItem = (ui::ListCtrlIconViewItem*)args.pEventData;
            if (pItem != nullptr) {
                nDataItemIndex = (int32_t)pItem->GetDataItemIndex();
                labelText = pItem->GetLabelText();
            }
        }
        else if (listCtrlType == ui::ListCtrlType::List) {
            sInfo += _T("ListCtrlType::List: ");
            ui::ListCtrlListViewItem* pItem = (ui::ListCtrlListViewItem*)args.pEventData;
            if (pItem != nullptr) {
                nDataItemIndex = (int32_t)pItem->GetDataItemIndex();
                labelText = pItem->GetLabelText();
            }
        }
        else {
            sInfo += _T("ListCtrl: ");
        }
        if (nDataItemIndex >= 0) {
            if ((args.eventType >= ui::kEventKeyBegin) && (args.eventType <= ui::kEventKeyEnd)) {
                //键盘消息
                DString keyName = ui::Keyboard::GetKeyName(args.vkCode, false);
                DString modifierKey;
                if (args.vkCode != ui::VirtualKeyCode::kVK_CONTROL) {
                    if (ui::Keyboard::IsKeyDown(ui::VirtualKeyCode::kVK_CONTROL)) {
                        modifierKey += _T("Ctrl+");
                    }
                }
                if (args.vkCode != ui::VirtualKeyCode::kVK_SHIFT) {
                    if (ui::Keyboard::IsKeyDown(ui::VirtualKeyCode::kVK_SHIFT)) {
                        modifierKey += _T("Shift+");
                    }
                }
                if (args.vkCode != ui::VirtualKeyCode::kVK_MENU) {
                    if (ui::Keyboard::IsKeyDown(ui::VirtualKeyCode::kVK_MENU)) {
                        modifierKey += _T("Alt+");
                    }
                }
                sInfo += _T("<");
                sInfo += modifierKey;
                sInfo += keyName;
                sInfo += _T(">");
                sInfo += _T(" ");
            }
            if (labelText.empty()) {
                if (nDataColumnIndex >= 0) {
                    sInfo += ui::StringUtil::Printf(_T("nDataItemIndex=%d, nDataColumnIndex=%d"), nDataItemIndex, nDataColumnIndex);
                }
                else {
                    sInfo += ui::StringUtil::Printf(_T("nDataItemIndex=%d"), nDataItemIndex);
                }
            }
            else {
                if (nDataColumnIndex >= 0) {
                    sInfo += ui::StringUtil::Printf(_T("nDataItemIndex=%d, nDataColumnIndex=%d, LabelText='%s'"), nDataItemIndex, nDataColumnIndex, labelText.c_str());
                }
                else {
                    sInfo += ui::StringUtil::Printf(_T("nDataItemIndex=%d, LabelText='%s'"), nDataItemIndex, labelText.c_str());
                }
            }
        }
    }
    else if ((args.eventType == ui::kEventViewTypeChanged) ||
             (args.eventType == ui::kEventViewPosChanged)  ||
             (args.eventType == ui::kEventViewSizeChanged)) {
        ui::ListCtrlType listCtrlType = (ui::ListCtrlType)args.listCtrlType;
        ui::Control* pControl = nullptr;
        if (listCtrlType == ui::ListCtrlType::Report) {
            sInfo += _T("ListCtrlType::Report: ");
            ui::ListCtrlReportView* pView = (ui::ListCtrlReportView*)args.pEventData;
            pControl = dynamic_cast<ui::Control*>(pView);
        }
        else if (listCtrlType == ui::ListCtrlType::Icon) {
            sInfo += _T("ListCtrlType::Icon: ");
            ui::ListCtrlIconView* pView = (ui::ListCtrlIconView*)args.pEventData;
            pControl = dynamic_cast<ui::Control*>(pView);
        }
        else if (listCtrlType == ui::ListCtrlType::List) {
            sInfo += _T("ListCtrlType::List: ");
            ui::ListCtrlListView* pView = (ui::ListCtrlListView*)args.pEventData;
            pControl = dynamic_cast<ui::Control*>(pView);
        }
        ASSERT(pControl != nullptr);
        if (args.eventType == ui::kEventViewTypeChanged) {
            //不展示
            sInfo += _T("ViewTypeChanged");
        }
        else if (args.eventType == ui::kEventViewPosChanged) {
            sInfo += ui::StringUtil::Printf(_T("left:%d, top: %d"), pControl->GetRect().left, pControl->GetRect().top);
        }
        else if (args.eventType == ui::kEventViewSizeChanged) {
            sInfo += ui::StringUtil::Printf(_T("width:%d, height: %d"), pControl->GetRect().Width(), pControl->GetRect().Height());
        }
    }
    else {
        ASSERT(0);
    }
   
    sInfo += _T("\n");
    return sInfo;
}

DString MainForm::GetItemFilledEventDisplayInfo(const ui::EventArgs& args)
{
    DString sInfo = ui::EventUtils::EventTypeToString(args.eventType);
    while (sInfo.size() < 32) {
        sInfo += _T(" ");
    }

    sInfo += ui::StringUtil::Printf(_T("ListBoxItemIndex=%zu "), (size_t)args.wParam);
    sInfo += ui::StringUtil::Printf(_T("DataItemIndex=%zu "), (size_t)args.lParam);

    if (args.eventType == ui::kEventReportViewItemFilled) {
        ui::ListCtrlItem* pItem = (ui::ListCtrlItem*)args.pEventData;
        ASSERT(pItem != nullptr);
        if (pItem != nullptr) {
            ASSERT(pItem->GetListBoxIndex() == (size_t)args.wParam);
            ASSERT(pItem->GetDataItemIndex() == (size_t)args.lParam);
        }
    }
    else if (args.eventType == ui::kEventReportViewSubItemFilled) {
        ui::ListCtrlSubItem* pSubItem = (ui::ListCtrlSubItem*)args.pEventData;
        ASSERT(pSubItem != nullptr);
        if (pSubItem != nullptr) {
            ASSERT(pSubItem->GetDataItemIndex() == (size_t)args.lParam);
            ui::ListCtrlItem* pItem = pSubItem->GetListCtrlItem();
            ASSERT(pItem != nullptr);
            if (pItem != nullptr) {
                ASSERT(pItem->GetListBoxIndex() == (size_t)args.wParam);
                ASSERT(pItem->GetDataItemIndex() == (size_t)args.lParam);
            }
            sInfo += ui::StringUtil::Printf(_T("DataColumnIndex='%zu' "), pSubItem->GetDataColumnIndex());
            sInfo += ui::StringUtil::Printf(_T("LabelText='%s' "), pSubItem->GetText().c_str());
        }
    }
    else if (args.eventType == ui::kEventListViewItemFilled) {
        ui::ListCtrlListViewItem* pItem = (ui::ListCtrlListViewItem*)args.pEventData;
        ASSERT(pItem != nullptr);
        if (pItem != nullptr) {
            ASSERT(pItem->GetListBoxIndex() == (size_t)args.wParam);
            ASSERT(pItem->GetDataItemIndex() == (size_t)args.lParam);
            sInfo += ui::StringUtil::Printf(_T("LabelText='%s'"), pItem->GetLabelText().c_str());
        }
    }
    else if (args.eventType == ui::kEventIconViewItemFilled) {
        ui::ListCtrlIconViewItem* pItem = (ui::ListCtrlIconViewItem*)args.pEventData;
        ASSERT(pItem != nullptr);
        if (pItem != nullptr) {
            ASSERT(pItem->GetListBoxIndex() == (size_t)args.wParam);
            ASSERT(pItem->GetDataItemIndex() == (size_t)args.lParam);
            sInfo += ui::StringUtil::Printf(_T("LabelText='%s'"), pItem->GetLabelText().c_str());
        }
    }
    return sInfo;
}

void MainForm::OutputDebugLog(const DString& logMsg)
{
#if defined DUILIB_BUILD_FOR_WIN && defined _DEBUG
    //::OutputDebugString(logMsg.c_str());
#endif
}

void MainForm::OnReportViewSubItemFilled(const ui::EventArgs& args)
{
    //默认不开启该测试
    return;

    if (args.eventType != ui::kEventReportViewSubItemFilled) {
        return;
    }
    ui::ListCtrlSubItem* pSubItem = (ui::ListCtrlSubItem*)args.pEventData;
    ASSERT(pSubItem != nullptr);
    if (pSubItem == nullptr) {
        return;
    }

    if (pSubItem->GetItemCount() == 0) {
        //功能演示：动态添加一个新按钮
        ui::Button* pHoverButton = new ui::Button(pSubItem->GetWindow());
        pHoverButton->SetClass(_T("btn_recycle"));
        pHoverButton->SetAttribute(_T("width"), _T("auto"));
        pHoverButton->SetAttribute(_T("height"), _T("auto"));
        pHoverButton->SetAttribute(_T("halign"), _T("right"));
        pHoverButton->SetAttribute(_T("valign"), _T("top"));
        pHoverButton->SetAttribute(_T("margin"), _T("0,8,8,0"));
        pHoverButton->SetToolTipText(_T("Hover Button"));

        //浮动按钮
        pHoverButton->SetFloat(true);
        pHoverButton->SetVisible(false);
        pSubItem->AddItem(pHoverButton);

        //当前的行和列信息
        size_t nDataItemIndex = pSubItem->GetDataItemIndex();
        size_t nDataColumnIndex = pSubItem->GetDataColumnIndex();

        //挂载事件
        pHoverButton->AttachClick([this, nDataItemIndex, nDataColumnIndex](const ui::EventArgs& /*args*/){
            DString title = _T("Hover Button Clicked");
            DString content = ui::StringUtil::Printf(_T("DataItemIndex:%zu, DataColumnIndex:%zu"), nDataItemIndex, nDataColumnIndex);
            ui::SystemUtil::ShowMessageBox(this, content, title);
            return true;
            });
    }
}

void MainForm::OnReportViewSubItemMouseEnter(const ui::EventArgs& args)
{
    if (args.eventType != ui::kEventSubItemMouseEnter) {
        return;
    }
    if ((ui::ListCtrlType)args.listCtrlType != ui::ListCtrlType::Report) {
        return;
    }
    ui::ListCtrlSubItem* pSubItem = (ui::ListCtrlSubItem*)args.pEventData;
    ASSERT(pSubItem != nullptr);
    if (pSubItem == nullptr) {
        return;
    }
    if (pSubItem->GetItemCount() > 0) {
        //鼠标划入时：如果存在子控件，则显示该子控件
        ui::Control* pDemoControl = pSubItem->GetItemAt(0);
        if ((pDemoControl != nullptr) && !pDemoControl->IsVisible()) {
            pDemoControl->SetVisible(true);
        }
    }
}

void MainForm::OnReportViewSubItemMouseLeave(const ui::EventArgs& args)
{
    if (args.eventType != ui::kEventSubItemMouseLeave) {
        return;
    }
    if ((ui::ListCtrlType)args.listCtrlType != ui::ListCtrlType::Report) {
        return;
    }
    ui::ListCtrlSubItem* pSubItem = (ui::ListCtrlSubItem*)args.pEventData;
    ASSERT(pSubItem != nullptr);
    if (pSubItem == nullptr) {
        return;
    }
    if (pSubItem->GetItemCount() > 0) {
        //鼠标划出时：如果存在子控件，则隐藏该子控件
        ui::Control* pDemoControl = pSubItem->GetItemAt(0);
        if ((pDemoControl != nullptr) && pDemoControl->IsVisible()) {
            pDemoControl->SetVisible(false);
        }
    }
}
