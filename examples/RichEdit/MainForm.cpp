#include "MainForm.h"
#include "FindForm.h"
#include "ReplaceForm.h"
#include "MainThread.h"
#include <fstream>

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#include <ShellApi.h>
#include <commdlg.h>
#endif

#ifndef LY_PER_INCH
    #define LY_PER_INCH 1440
#endif

MainForm::MainForm():
    m_pRichEdit(nullptr),
    m_pFindForm(nullptr),
    m_pReplaceForm(nullptr)
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("rich_edit");
}

DString MainForm::GetSkinFile()
{
    return _T("rich_edit.xml");
}

void MainForm::OnInitWindow()
{
    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("test_url")));
    if (pRichEdit != nullptr) {
        pRichEdit->AttachLinkClick([this, pRichEdit](const ui::EventArgs& args) {
                //点击了超级链接
                if (args.GetSender() == pRichEdit) {
                    const DString::value_type* pUrl = (const DString::value_type*)args.wParam;
                    if (pUrl != nullptr) {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
                        ::ShellExecuteW(NativeWnd()->GetHWND(), L"open", ui::StringConvert::TToWString(pUrl).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
                    }
                }
                return true;
            });
    }
    m_pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("rich_edit")));
    ASSERT(m_pRichEdit != nullptr);
    m_findReplace.SetRichEdit(m_pRichEdit);
    LoadRichEditData();

    //文件操作：打开、保存、另存
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("open_file")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
                if (args.GetSender() == pButton) {
                    this->OnOpenFile();
                }
                return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("save_file")));
    if (pButton != nullptr) {
        m_saveBtnText = pButton->GetText();
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
                if (args.GetSender() == pButton) {
                    this->OnSaveFile();
                }
                return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("save_as_file")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
                if (args.GetSender() == pButton) {
                    this->OnSaveAsFile();
                }
                return true;
            });
    }

    //编辑操作
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_copy")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->Copy();
                    UpdateSaveStatus();
                }
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_cut")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->Cut();
                    UpdateSaveStatus();
                }
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_paste")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->Paste();
                    UpdateSaveStatus();
                }
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_delete")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->Clear();
                    UpdateSaveStatus();
                }
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_sel_all")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->SetSelAll();
                    UpdateSaveStatus();
                }
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_sel_none")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->SetSelNone();
                    UpdateSaveStatus();
                }
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_undo")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->Undo();
                    UpdateSaveStatus();
                }
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_redo")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->Redo();
                    UpdateSaveStatus();
                }
            }
            return true;
            });
    }

    //查找操作
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_find_text")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                OnFindText();
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_find_next")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                OnFindNext();
            }
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_replace_text")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                OnReplaceText();
            }
            return true;
            });
    }

    //设置字体
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("set_font")));
    if (pButton != nullptr) {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                OnSetFont();
            }
            return true;
            });
#else
        pButton->SetEnabled(false);
#endif
    }

    //初始化字体信息
    ui::Combo* pFontNameCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_name"))); 
    if (pFontNameCombo != nullptr) {
        m_fontList.clear();
        ui::GlobalManager::Instance().Font().GetFontNameList(m_fontList);
        for (size_t nIndex = 0; nIndex < m_fontList.size(); ++nIndex) {
            const DString& fontName = m_fontList[nIndex];
            size_t nItemIndex = pFontNameCombo->AddTextItem(fontName);
            if (ui::Box::IsValidItemIndex(nItemIndex)) {
                pFontNameCombo->SetItemData(nItemIndex, nIndex);
            }
        }
        pFontNameCombo->AttachSelect([this, pFontNameCombo](const ui::EventArgs& args) {
            DString fontName = pFontNameCombo->GetText();
            SetFontName(fontName);
            return true;
            });
        pFontNameCombo->AttachWindowClose([this, pFontNameCombo](const ui::EventArgs& args) {
            DString fontName = pFontNameCombo->GetText();
            SetFontName(fontName);
            return true;
            });
    }
    ui::Combo* pFontSizeCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_size")));
    if (pFontSizeCombo != nullptr) {
        ui::GlobalManager::Instance().Font().GetFontSizeList(Dpi(), m_fontSizeList);
        for (size_t nIndex = 0; nIndex < m_fontSizeList.size(); ++nIndex) {
            const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
            size_t nItemIndex = pFontSizeCombo->AddTextItem(fontSize.fontSizeName);
            if (ui::Box::IsValidItemIndex(nItemIndex)) {
                pFontSizeCombo->SetItemData(nItemIndex, nIndex);
            }
        }
        pFontSizeCombo->AttachSelect([this, pFontSizeCombo](const ui::EventArgs& args) {
            DString fontName = pFontSizeCombo->GetText();
            SetFontSize(fontName);
            return true;
            });
        pFontSizeCombo->AttachWindowClose([this, pFontSizeCombo](const ui::EventArgs& args) {
            DString fontName = pFontSizeCombo->GetText();
            SetFontSize(fontName);
            return true;
            });
    }

    //更新是否粗体
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_bold")));
    if (pCheckBox != nullptr) {
        pCheckBox->AttachSelect([this, pCheckBox](const ui::EventArgs& args) {
            SetFontBold(pCheckBox->IsSelected());
            return true;
            });
        pCheckBox->AttachUnSelect([this, pCheckBox](const ui::EventArgs& args) {
            SetFontBold(pCheckBox->IsSelected());
            return true;
            });
    }

    //更新是否斜体
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_italic")));
    if (pCheckBox != nullptr) {
        pCheckBox->AttachSelect([this, pCheckBox](const ui::EventArgs& args) {
            SetFontItalic(pCheckBox->IsSelected());
            return true;
            });
        pCheckBox->AttachUnSelect([this, pCheckBox](const ui::EventArgs& args) {
            SetFontItalic(pCheckBox->IsSelected());
            return true;
            });
    }

    //更新是否下划线
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_underline")));
    if (pCheckBox != nullptr) {
        pCheckBox->AttachSelect([this, pCheckBox](const ui::EventArgs& args) {
            SetFontUnderline(pCheckBox->IsSelected());
            return true;
            });
        pCheckBox->AttachUnSelect([this, pCheckBox](const ui::EventArgs& args) {
            SetFontUnderline(pCheckBox->IsSelected());
            return true;
            });
    }

    //更新是否删除线
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_strikeout")));
    if (pCheckBox != nullptr) {
        pCheckBox->AttachSelect([this, pCheckBox](const ui::EventArgs& args) {
            SetFontStrikeOut(pCheckBox->IsSelected());
            return true;
            });
        pCheckBox->AttachUnSelect([this, pCheckBox](const ui::EventArgs& args) {
            SetFontStrikeOut(pCheckBox->IsSelected());
            return true;
            });
    }

    //增加字体大小
    ui::Button* pFontButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_font_size_increase")));
    if (pFontButton != nullptr) {
        pFontButton->AttachClick([this](const ui::EventArgs& args) {
            AdjustFontSize(true);
            return true;
            });
    }
    //减小字体大小
    pFontButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_font_size_decrease")));
    if (pFontButton != nullptr) {
        pFontButton->AttachClick([this](const ui::EventArgs& args) {
            AdjustFontSize(false);
            return true;
            });
    }

    //设置颜色
    InitColorCombo();
    ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(_T("color_combo_button")));
    if (pColorComboBtn != nullptr) {
        DString textColor;
        if (m_pRichEdit != nullptr) {
            textColor = m_pRichEdit->GetTextColor();
        }
        //设置选择后的颜色
        ui::Label* pLeftColorLabel = pColorComboBtn->GetLabelBottom();
        if (pLeftColorLabel != nullptr) {
            pLeftColorLabel->SetBkColor(textColor);
        }

        //左侧按钮点击事件
        pColorComboBtn->AttachClick([this, pLeftColorLabel](const ui::EventArgs& args) {
            if (pLeftColorLabel != nullptr) {
                SetTextColor(pLeftColorLabel->GetBkColor());
            }
            return true;
            });
    }

    UpdateZoomValue();
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->AttachZoom([this](const ui::EventArgs& args) {
            UpdateZoomValue();
            return true;
            });
    }
    ui::Button* pZoomButtom = dynamic_cast<ui::Button*>(FindControl(_T("btn_zoom_in")));
    if (pZoomButtom != nullptr) {
        pZoomButtom->AttachClick([this](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                //放大：每次放大10%
                uint32_t nZoomPercent = GetNextZoomPercent(m_pRichEdit->GetZoomPercent(), true);
                m_pRichEdit->SetZoomPercent(nZoomPercent);
                UpdateZoomValue();
            }
            return true;
            });
    }
    pZoomButtom = dynamic_cast<ui::Button*>(FindControl(_T("btn_zoom_out")));
    if (pZoomButtom != nullptr) {
        pZoomButtom->AttachClick([this](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                //缩小：每次缩小10%
                uint32_t nZoomPercent = GetNextZoomPercent(m_pRichEdit->GetZoomPercent(), false);
                m_pRichEdit->SetZoomPercent(nZoomPercent);
                UpdateZoomValue();
            }
            return true;
            });
    }
    pZoomButtom = dynamic_cast<ui::Button*>(FindControl(_T("btn_zoom_off")));
    if (pZoomButtom != nullptr) {
        pZoomButtom->AttachClick([this](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                //恢复
                m_pRichEdit->SetZoomPercent(100);
                UpdateZoomValue();
            }
            return true;
            });
    }

    //是否自动换行
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_word_wrap")));
    if ((pCheckBox != nullptr) && (m_pRichEdit != nullptr)) {
        pCheckBox->SetSelected(m_pRichEdit->IsWordWrap());
        pCheckBox->AttachSelect([this](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetWordWrap(true);
                m_pRichEdit->SetAttribute(_T("hscrollbar"), _T("false"));
            }
            return true;
            });
        pCheckBox->AttachUnSelect([this](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetWordWrap(false);
                m_pRichEdit->SetAttribute(_T("hscrollbar"), _T("true"));
            }
            return true;
            });
    }

    //是否支持富文本格式
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_rich_text")));
    if ((pCheckBox != nullptr) && (m_pRichEdit != nullptr)) {
        pCheckBox->SetSelected(m_pRichEdit->IsRichText());
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        pCheckBox->AttachSelect([this](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetRichText(true);
            }
            return true;
            });
        pCheckBox->AttachUnSelect([this](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetRichText(false);
            }
            return true;
            });
#else
        //SDL实现的时候，不支持富文本格式
        pCheckBox->SetEnabled(false);
#endif
    }

#ifdef DUILIB_BUILD_FOR_SDL
    ui::Control* pRowSpacingTips = FindControl(_T("row_spacing_tips"));
    if (pRowSpacingTips != nullptr) {
        pRowSpacingTips->SetVisible(false);
    }
#endif

    //更新字体按钮的状态
    UpdateFontStatus();

    //超链接
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->AttachLinkClick([this](const ui::EventArgs& args) {
            const DString::value_type* url = (const DString::value_type*)args.wParam;
            if (url != nullptr) {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
                ::MessageBoxW(NativeWnd()->GetHWND(), ui::StringConvert::TToWString(url).c_str(), L"RichEdit点击超链接", MB_OK);
#endif
            }
            return true;
            });
    }
    //RichEdit文本选择变化
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->AttachSelChanged([this](const ui::EventArgs& args) {
            if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
                UpdateFontStatus();
            }
            return true;
            });
    }
}

uint32_t MainForm::GetNextZoomPercent(uint32_t nOldZoomPercent, bool bZoomIn) const
{
    //缩放百分比的最大值
    const uint32_t MAX_ZOOM_PERCENT = 800;

    //通过查表的方式设置缩放比例(放大和缩小的时候，能够按照原来的比例复原)
    std::vector<uint32_t> zoomPercentList;
    uint32_t nZoomPercent = 100;
    while (nZoomPercent > 1) {
        nZoomPercent = (uint32_t)(nZoomPercent * 0.90f);
        zoomPercentList.insert(zoomPercentList.begin(), nZoomPercent);
    }
    nZoomPercent = 100;
    while (nZoomPercent < MAX_ZOOM_PERCENT) {
        zoomPercentList.insert(zoomPercentList.end(), nZoomPercent);
        nZoomPercent = (uint32_t)(nZoomPercent * 1.10f);
    }
    zoomPercentList.insert(zoomPercentList.end(), MAX_ZOOM_PERCENT);

    bool bFound = false;
    nZoomPercent = nOldZoomPercent;
    const size_t nPercentCount = zoomPercentList.size();
    for (size_t nPercentIndex = 0; nPercentIndex < nPercentCount; ++nPercentIndex) {
        if ((zoomPercentList[nPercentIndex] > nZoomPercent) || (nPercentIndex == (nPercentCount - 1))) {
            if (nPercentIndex <= 1) {
                size_t nCurrentIndex = 0;//当前是第一个元素
                if (bZoomIn) {
                    //放大
                    nZoomPercent = zoomPercentList[nCurrentIndex + 1];
                }
                else {
                    //缩小（已经到达最小值，无法再缩小）
                    nZoomPercent = zoomPercentList[nCurrentIndex];
                }
            }
            else if (zoomPercentList[nPercentIndex] > nZoomPercent) {
                size_t nCurrentIndex = nPercentIndex - 1;//中间元素
                if (bZoomIn) {
                    //放大
                    nZoomPercent = zoomPercentList[nCurrentIndex + 1];
                }
                else {
                    //缩小
                    nZoomPercent = zoomPercentList[nCurrentIndex - 1];
                }
            }
            else if (nPercentIndex == (nPercentCount - 1)) {
                size_t nCurrentIndex = nPercentCount - 1;//当前是最后一个元素
                if (bZoomIn) {
                    //放大（已经到达最大值，无法再放大）
                    nZoomPercent = zoomPercentList[nCurrentIndex];
                }
                else {
                    //缩小
                    nZoomPercent = zoomPercentList[nCurrentIndex - 1];
                }
            }
            bFound = true;
            break;
        }
    }

    if (!bFound) {
        //如果查表得不到结果，则按比例放大或者缩小
        if (bZoomIn) {
            //放大
            nZoomPercent = (uint32_t)(nZoomPercent * 1.10f);
            if (nZoomPercent == nOldZoomPercent) {
                //避免数值过小时无法放大
                ++nZoomPercent;
            }
        }
        else {
            //缩小
            nZoomPercent = (uint32_t)(nZoomPercent * 0.91f);
        }
    }
    if (nZoomPercent < 1) {
        nZoomPercent = 1;
    }
    return nZoomPercent;
}

void MainForm::InitColorCombo()
{
    ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(_T("color_combo_button")));
    if (pColorComboBtn == nullptr) {
        return;
    }
    ui::UiSize boxSize = pColorComboBtn->GetDropBoxSize();
    ui::Box* pComboBox = pColorComboBtn->GetComboBox();
    if (pComboBox == nullptr) {
        return;
    }
    pComboBox->SetWindow(this);
    ui::GlobalManager::Instance().FillBoxWithCache(pComboBox, ui::FilePath(_T("rich_edit/color_combox.xml")));
    ui::ColorPickerRegular* pColorPicker = dynamic_cast<ui::ColorPickerRegular*>(pComboBox->FindSubControl(_T("color_combo_picker")));
    if (pColorPicker != nullptr) {
        //响应选择颜色事件
        pColorPicker->AttachSelectColor([this, pColorComboBtn](const ui::EventArgs& args) {
            ui::UiColor newColor((uint32_t)args.wParam);
            //设置选择后的颜色
            ui::Label* pLeftColorLabel = pColorComboBtn->GetLabelBottom();
            if (pLeftColorLabel != nullptr) {
                pLeftColorLabel->SetBkColor(newColor);
                SetTextColor(pLeftColorLabel->GetBkColor());
            }
            return true;
            });
    }

    ui::Button* pMoreColorButton = dynamic_cast<ui::Button*>(pComboBox->FindSubControl(_T("color_combo_picker_more")));
    if (pMoreColorButton != nullptr) {
        pMoreColorButton->AttachClick([this](const ui::EventArgs& args) {
            ShowColorPicker();
            return true;
            });
    }
}

void MainForm::ShowColorPicker()
{
    ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(_T("color_combo_button")));
    if (pColorComboBtn == nullptr) {
        return;
    }
    ui::Label* pLeftColorLabel = pColorComboBtn->GetLabelBottom();
    if (pLeftColorLabel == nullptr) {
        return;
    }
    DString oldTextColor = pLeftColorLabel->GetBkColor();

    ui::ColorPicker* pColorPicker = new ui::ColorPicker;
    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("ColorPicker");
    createParam.m_bCenterWindow = true;
#ifdef DUILIB_BUILD_FOR_WIN
    pColorPicker->CreateWnd(nullptr, createParam);
    pColorPicker->ShowModalFake();
#else
    pColorPicker->CreateWnd(this, createParam);
    pColorPicker->ShowModalFake();
    pColorPicker->SetWindowForeground();
#endif

    ui::RichEdit* pEdit = m_pRichEdit;
    if (pEdit != nullptr) {
        if (!oldTextColor.empty()) {
            pColorPicker->SetSelectedColor(pEdit->GetUiColor(oldTextColor));
        }
        //如果在界面选择颜色，则临时更新RichEdit控件文本的颜色
        pColorPicker->AttachSelectColor([this, pEdit, pLeftColorLabel](const ui::EventArgs& args) {
            ui::UiColor newColor = ui::UiColor((uint32_t)args.wParam);
            pLeftColorLabel->SetBkColor(newColor);
            SetTextColor(pEdit->GetColorString(newColor));
            return true;
            });

        //窗口关闭事件
        pColorPicker->AttachWindowClose([this, pColorPicker, pEdit, oldTextColor, pLeftColorLabel](const ui::EventArgs& args) {
            ui::UiColor newColor = pColorPicker->GetSelectedColor();
            if ((args.wParam == ui::kWindowCloseOK) && !newColor.IsEmpty()) {
                //如果是"确认"，则设置RichEdit控件的文本颜色
                pLeftColorLabel->SetBkColor(newColor);
                SetTextColor(pEdit->GetColorString(newColor));
            }
            else {
                //如果是"取消"或者关闭窗口，则恢复原来的颜色
                pLeftColorLabel->SetBkColor(newColor);
                SetTextColor(oldTextColor);
            }
            return true;
            });
    }
}

void MainForm::UpdateFontStatus()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    const ui::UiFont fontInfo = pRichEdit->GetFontInfo();

    //更新字体名称
    ui::Combo* pFontNameCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_name")));
    if (pFontNameCombo != nullptr) {
        pFontNameCombo->SelectTextItem(fontInfo.m_fontName.c_str(), false);
    }

    //更新字体大小
    UpdateFontSizeStatus();

    //更新是否粗体
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_bold")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(fontInfo.m_bBold);
    }

    //更新是否斜体
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_italic")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(fontInfo.m_bItalic);
    }

    //更新是否下划线
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_underline")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(fontInfo.m_bUnderline);
    }

    //更新是否删除线
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_strikeout")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(fontInfo.m_bStrikeOut);
    }
}

void MainForm::UpdateFontSizeStatus()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    ui::Combo* pFontSizeCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_size")));
    if (pFontSizeCombo == nullptr) {
        return;
    }
    const ui::UiFont fontInfo = pRichEdit->GetFontInfo();
    if (fontInfo.m_fontSize == 0) {
        //不含有效字体大小信息
        pFontSizeCombo->SetCurSel(ui::Box::InvalidIndex);
        return;
    }
   
    size_t maxItemIndex = 0;
    for (size_t nIndex = 0; nIndex < m_fontSizeList.size(); ++nIndex) {
        if (nIndex == (m_fontSizeList.size() - 1)) {
            break;
        }
        if (m_fontSizeList[nIndex].fFontSize > m_fontSizeList[nIndex + 1].fFontSize) {
            maxItemIndex = nIndex;
            break;
        }
    }

    bool bSelected = false;
    for (size_t nIndex = maxItemIndex; nIndex < m_fontSizeList.size(); ++nIndex) {
        //优先选择汉字的字号
        const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
        if (fontInfo.m_fontSize == (int32_t)std::roundf(fontSize.fDpiFontSize)) {
            if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName, false) != ui::Box::InvalidIndex) {
                bSelected = true;
            }
            break;
        }
    }
    if (!bSelected) {
        for (size_t nIndex = 0; nIndex <= maxItemIndex; ++nIndex) {
            //选择数字的字号
            const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
            if ((int32_t)std::roundf(fontSize.fDpiFontSize) >= fontInfo.m_fontSize) {
                if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName, false) != ui::Box::InvalidIndex) {
                    bSelected = true;
                    break;
                }
            }
        }
    }
    if (!bSelected) {
        for (size_t nIndex = 0; nIndex <= maxItemIndex; ++nIndex) {
            //选择数字的字号
            const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
            int32_t nFontSize = Dpi().GetScaleInt((int32_t)std::roundf(fontSize.fFontSize));
            if (nFontSize >= fontInfo.m_fontSize) {
                if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName, false) != ui::Box::InvalidIndex) {
                    bSelected = true;
                    break;
                }
            }
        }
    }
}

void MainForm::SetFontName(const DString& fontName)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_fontName = fontName;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetFontSize(const DString& fontSize)
{
    for (const ui::FontSizeInfo& fontSizeInfo : m_fontSizeList) {
        if (fontSize == fontSizeInfo.fontSizeName) {
            if (m_pRichEdit != nullptr) {
                ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
                fontInfo.m_fontSize = (int32_t)(std::roundf(fontSizeInfo.fDpiFontSize));
                m_pRichEdit->SetFontInfo(fontInfo);
            }
            break;
        }
    }
}

void MainForm::AdjustFontSize(bool bIncreaseFontSize)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    std::map<int32_t, int32_t> fontSizeMap;
    for (const ui::FontSizeInfo& fontSizeInfo : m_fontSizeList) {
        fontSizeMap[(int32_t)(std::roundf(fontSizeInfo.fDpiFontSize))] = (int32_t)(std::roundf(fontSizeInfo.fFontSize));
    }
    std::vector<int32_t> fontSizeList;
    for (auto fontSize : fontSizeMap) {
        fontSizeList.push_back(fontSize.second);
    }
    auto pos = std::unique(fontSizeList.begin(), fontSizeList.end());
    if (pos != fontSizeList.end()) {
        fontSizeList.erase(pos);
    }

    ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
    //转换回原值
    auto iter = fontSizeMap.find(fontInfo.m_fontSize);
    if (iter != fontSizeMap.end()) {
        fontInfo.m_fontSize = iter->second;
    }
    else {
        bool bFound = false;
        if (bIncreaseFontSize) {
            for (iter = fontSizeMap.begin(); iter != fontSizeMap.end(); ++iter) {
                if (iter->first > fontInfo.m_fontSize) {
                    fontInfo.m_fontSize = iter->second;
                    bFound = true;
                    break;
                }
            }
        }
        else {
            auto pos = fontSizeMap.rbegin();
            for (; pos != fontSizeMap.rend(); ++pos) {
                if (pos->first < fontInfo.m_fontSize) {
                    fontInfo.m_fontSize = pos->second;
                    bFound = true;
                    break;
                }
            }
        }
        if (!bFound) {
            Dpi().UnscaleInt(fontInfo.m_fontSize);
        }
    }

    const size_t fontCount = fontSizeList.size();
    for (size_t index = 0; index < fontCount; ++index) {
        if (fontInfo.m_fontSize == fontSizeList[index]) {
            //匹配到当前字体大小
            if (bIncreaseFontSize) {
                //增加字体
                if (index < (fontCount - 1)) {                  
                    fontInfo.m_fontSize = Dpi().GetScaleInt(fontSizeList[index + 1]);
                    if (m_pRichEdit->SetFontInfo(fontInfo)) {
                        UpdateFontSizeStatus();
                    }
                }
            }
            else {
                //减小字体
                if (index > 0) {
                    fontInfo.m_fontSize = Dpi().GetScaleInt(fontSizeList[index - 1]);
                    if (m_pRichEdit->SetFontInfo(fontInfo)) {
                        UpdateFontSizeStatus();
                    }
                }
            }
            break;
        }
    }
}

void MainForm::SetFontBold(bool bBold)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bBold = bBold;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetFontItalic(bool bItalic)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bItalic = bItalic;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetFontUnderline(bool bUnderline)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bUnderline = bUnderline;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetFontStrikeOut(bool bStrikeOut)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bStrikeOut = bStrikeOut;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetTextColor(const DString& newColor)
{
    if (m_pRichEdit != nullptr) {
        if (m_pRichEdit->IsRichText()) {
            m_pRichEdit->SetSelectionTextColor(newColor);
        }
        else {
            m_pRichEdit->SetTextColor(newColor);
        }        
    }
}

void MainForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMsg(0);
}

LRESULT MainForm::OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    bool bControlDown = ui::Keyboard::IsKeyDown(ui::kVK_CONTROL);
    bool bShiftDown = ui::Keyboard::IsKeyDown(ui::kVK_SHIFT);
    if (bControlDown) {
        if ((vkCode == 'O') && !bShiftDown){
            //打开
            OnOpenFile();
        }
        else if (vkCode == 'S') {
            if (bShiftDown) {
                //另存为
                OnSaveAsFile();                
            }
            else {
                //保存
                OnSaveFile();
            }
        }
        if (!bShiftDown) {
            if (vkCode == 'F') {
                //查找
                OnFindText();
            }
            else if (vkCode == 'H') {
                //替换
                OnReplaceText();
            }
        }
    }
    if (!bControlDown && !bShiftDown && (vkCode == ui::kVK_F3)) {
        //查找下一个
        OnFindNext();
    }
    return lResult;
}

LRESULT MainForm::OnKeyUpMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKeyUpMsg(vkCode, modifierKey, nativeMsg, bHandled);
    UpdateSaveStatus();
    return lResult;
}

void MainForm::UpdateSaveStatus()
{
    if (m_pRichEdit != nullptr) {
        ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("save_file")));
        if (m_pRichEdit->GetModify()) {
            if (pButton != nullptr) {
                pButton->SetText(m_saveBtnText + _T("*"));
            }
        }
        else {
            if (pButton != nullptr) {
                pButton->SetText(m_saveBtnText);
            }
        }
    }
}

void MainForm::LoadRichEditData()
{
    std::streamoff length = 0;
    std::string xml;
    ui::FilePath controls_xml = ui::GlobalManager::Instance().GetResourcePath();
    controls_xml += GetResourcePath();
    controls_xml += GetSkinFile();

    std::ifstream ifs(controls_xml.NativePath().c_str(), std::ios::binary);
    if (ifs.is_open()) {
        ifs.seekg(0, std::ios_base::end);
        length = ifs.tellg();
        ifs.seekg(0, std::ios_base::beg);

        xml.resize(static_cast<unsigned int>(length));
        ifs.read(&xml[0], length);
        ifs.close();
    }
    DString xmlU = ui::StringConvert::UTF8ToT(xml);

    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetText(xmlU);
        m_pRichEdit->SetFocus();
        m_pRichEdit->HomeUp();
        m_pRichEdit->SetModify(false);
        m_filePath = controls_xml;
    }
}

void MainForm::OnFindText()
{
    if (m_pFindForm == nullptr) {
        m_pFindForm = new FindForm(this);
        ui::WindowCreateParam createParam;
        createParam.m_dwStyle = ui::kWS_POPUP;
        createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
        createParam.m_windowTitle = _T("FindForm");
        createParam.m_bCenterWindow = true;
        m_pFindForm->CreateWnd(this, createParam);
        m_pFindForm->ShowWindow(ui::kSW_SHOW);
        m_pFindForm->AttachWindowClose([this](const ui::EventArgs& args) {
                m_pFindForm = nullptr;
                return true;
            });
    }
    else {
        if (m_pFindForm->IsWindowMinimized()) {
            m_pFindForm->ShowWindow(ui::kSW_RESTORE);
        }
        else {
            m_pFindForm->ShowWindow(ui::kSW_SHOW);
        }
    }
}

void MainForm::OnFindNext()
{
    m_findReplace.FindNext();
}

void MainForm::OnReplaceText()
{
    if (m_pReplaceForm == nullptr) {
        m_pReplaceForm = new ReplaceForm(this);
        ui::WindowCreateParam createParam;
        createParam.m_dwStyle = ui::kWS_POPUP;
        createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
        createParam.m_windowTitle = _T("ReplaceForm");
        createParam.m_bCenterWindow = true;
        m_pReplaceForm->CreateWnd(this, createParam);
        m_pReplaceForm->ShowWindow(ui::kSW_SHOW);
        m_pReplaceForm->AttachWindowClose([this](const ui::EventArgs& args) {
                m_pReplaceForm = nullptr;
                return true;
            });
    }
    else {
        if (m_pReplaceForm->IsWindowMinimized()) {
            m_pReplaceForm->ShowWindow(ui::kSW_RESTORE);
        }
        else {
            m_pReplaceForm->ShowWindow(ui::kSW_SHOW);
        }
    }
}

void MainForm::FindRichText(const DString& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
{
    m_findReplace.FindRichText(findText, bFindDown, bMatchCase, bMatchWholeWord, pWndDialog);
}

void MainForm::ReplaceRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
{
    if (m_findReplace.ReplaceRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, pWndDialog)) {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetModify(true);
            UpdateSaveStatus();
        }
    }
}

void MainForm::ReplaceAllRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
{
    if (m_findReplace.ReplaceAllRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, pWndDialog)) {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetModify(true);
            UpdateSaveStatus();
        }
    }
}

ui::RichEdit* MainForm::GetRichEdit() const
{
    return m_pRichEdit;
}

void MainForm::UpdateZoomValue()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }

    ui::Label* pZoomLabel = dynamic_cast<ui::Label*>(FindControl(_T("lavel_zoom_value")));
    if (pZoomLabel != nullptr) {
        uint32_t nZoomPercent = pRichEdit->GetZoomPercent();
        DString strZoom = ui::StringUtil::Printf(_T("%u%%"), nZoomPercent);
        pZoomLabel->SetText(strZoom);
    }
}

void MainForm::OnOpenFile()
{
    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({ _T("所有文件 (*.*)"), _T("*.*")});
    fileTypes.push_back({ _T("文本文件 (*.txt)"), _T("*.txt") });
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    fileTypes.push_back({ _T("RTF文件 (*.rtf)"), _T("*.rtf") });
#endif

    DString defaultExt;
    int32_t nFileTypeIndex = 1;
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
        nFileTypeIndex = 2;
    }
#endif
    DString fileName = m_filePath.GetFileName();

    ui::FilePath filePath;
    ui::FileDialog openFileDlg;
    if (openFileDlg.BrowseForFile(this, filePath, true, fileTypes, nFileTypeIndex, defaultExt, fileName)) {
        if (LoadFile(filePath)) {
            m_filePath = filePath;
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetModify(false);
                UpdateSaveStatus();
            }
        }        
    }
}

void MainForm::OnSaveFile()
{
    if (m_pRichEdit != nullptr) {
        if (m_pRichEdit->GetModify()) {
            if (SaveFile(m_filePath)) {
                m_pRichEdit->SetModify(false);
                UpdateSaveStatus();
            }
        }
    }
}

void MainForm::OnSaveAsFile()
{
    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({ _T("所有文件 (*.*)"), _T("*.*") });
    fileTypes.push_back({ _T("文本文件 (*.txt)"), _T("*.txt") });
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    fileTypes.push_back({ _T("RTF文件 (*.rtf)"), _T("*.rtf") });
#endif

    DString defaultExt;
    int32_t nFileTypeIndex = 1;
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
        nFileTypeIndex = 2;
    }
#endif
    DString fileName = m_filePath.GetFileName();

    ui::FilePath filePath;
    ui::FileDialog openFileDlg;
    if (openFileDlg.BrowseForFile(this, filePath, false, fileTypes, nFileTypeIndex, defaultExt, fileName)) {
        if (SaveFile(filePath)) {
            m_filePath = filePath;
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetModify(false);
                UpdateSaveStatus();
            }
        }        
    }
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

bool MainForm::LoadFile(const ui::FilePath& filePath)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DString filePathLocal = filePath.NativePath();
    HANDLE hFile = ::CreateFile(filePathLocal.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    EDITSTREAM es;
    es.dwCookie = (DWORD_PTR)hFile;
    es.dwError = 0;
    es.pfnCallback = StreamReadCallback;
    UINT nFormat = SF_TEXT;
    if (m_pRichEdit->IsRichText()) {
        nFormat = IsRtfFile(filePathLocal) ? SF_RTF : SF_TEXT;
    }
    m_pRichEdit->StreamIn(nFormat, es);
    ::CloseHandle(hFile);
    return !(BOOL)es.dwError;
}

bool MainForm::SaveFile(const ui::FilePath& filePath)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DString filePathLocal = filePath.NativePath();
    HANDLE hFile = ::CreateFile(filePathLocal.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    EDITSTREAM es;
    es.dwCookie = (DWORD_PTR)hFile;
    es.dwError = 0;
    es.pfnCallback = StreamWriteCallback;
    UINT nFormat = SF_TEXT;
    if (m_pRichEdit->IsRichText()) {
        nFormat = IsRtfFile(filePathLocal) ? SF_RTF : SF_TEXT;
    }
    m_pRichEdit->StreamOut(nFormat, es);
    ::CloseHandle(hFile);
    return !(BOOL)es.dwError;
}

bool MainForm::IsRtfFile(const DString& filePath) const
{
    DString fileExt;
    size_t pos = filePath.find_last_of(_T("."));
    if (pos != DString::npos) {
        fileExt = filePath.substr(pos);
        fileExt = ui::StringUtil::MakeLowerString(fileExt);
    }
    return fileExt == _T(".rtf");
}

DWORD MainForm::StreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
    ASSERT(dwCookie != 0);
    ASSERT(pcb != nullptr);

    return !::ReadFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, nullptr);
}

DWORD MainForm::StreamWriteCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
    ASSERT(dwCookie != 0);
    ASSERT(pcb != nullptr);

    return !::WriteFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, nullptr);
}

int32_t MainForm::ConvertToFontHeight(int32_t fontSize) const
{
    if (m_pRichEdit != nullptr) {
        return m_pRichEdit->ConvertToFontHeight(fontSize);
    }
    return fontSize;
}

bool MainForm::GetRichEditLogFont(LOGFONTW& lf) const
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return false;
    }

    CHARFORMAT2W cf = {};
    GetCharFormat(cf);

    if (cf.dwMask & CFM_SIZE) {
        HWND hWnd = NativeWnd()->GetHWND();
        HDC hDC = ::GetDC(hWnd);
        lf.lfHeight = -MulDiv(cf.yHeight, ::GetDeviceCaps(hDC, LOGPIXELSY), LY_PER_INCH);
        lf.lfWidth = 0;
        ::ReleaseDC(hWnd, hDC);
    }

    if (cf.dwMask & CFM_BOLD) {
        lf.lfWeight = (cf.dwEffects & CFE_BOLD) ? FW_BOLD : 0;
    }

    if (cf.dwMask & CFM_WEIGHT) {
        lf.lfWeight = cf.wWeight;
    }

    if (cf.dwMask & CFM_ITALIC) {
        lf.lfItalic = (cf.dwEffects & CFE_ITALIC) ? TRUE : FALSE;
    }

    if (cf.dwMask & CFM_UNDERLINE) {
        lf.lfUnderline = (cf.dwEffects & CFE_UNDERLINE) ? TRUE : FALSE;
    }

    if (cf.dwMask & CFM_STRIKEOUT) {
        lf.lfStrikeOut = (cf.dwEffects & CFE_STRIKEOUT) ? TRUE : FALSE;
    }

    if (cf.dwMask & CFM_CHARSET) {
        lf.lfCharSet = cf.bCharSet;
    }

    if (cf.dwMask & CFM_FACE) {
        lf.lfPitchAndFamily = cf.bPitchAndFamily;

        //替换为系统字体名称
        DStringW fontName = cf.szFaceName;
        ui::StringUtil::StringCopy(lf.lfFaceName, fontName.c_str());
    }
    return true;
}

void MainForm::InitCharFormat(const LOGFONTW& lf, CHARFORMAT2W& charFormat) const
{
    //字体字号需要转换, 否则字体大小显示异常
    LONG lfHeight = ConvertToFontHeight(lf.lfHeight);

    charFormat.cbSize = sizeof(CHARFORMAT2W);
    charFormat.dwMask = CFM_SIZE | CFM_OFFSET | CFM_FACE | CFM_CHARSET | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
    charFormat.yHeight = -lfHeight;
    if (lf.lfWeight >= FW_BOLD) {
        charFormat.dwEffects |= CFE_BOLD;
    }
    else {
        charFormat.dwEffects &= ~CFE_BOLD;
    }
    if (lf.lfItalic) {
        charFormat.dwEffects |= CFE_ITALIC;
    }
    else {
        charFormat.dwEffects &= ~CFE_ITALIC;
    }
    if (lf.lfUnderline) {
        charFormat.dwEffects |= CFE_UNDERLINE;
    }
    else {
        charFormat.dwEffects &= ~CFE_UNDERLINE;
    }
    if (lf.lfStrikeOut) {
        charFormat.dwEffects |= CFE_STRIKEOUT;
    }
    else {
        charFormat.dwEffects &= ~CFE_STRIKEOUT;
    }
    charFormat.bCharSet = lf.lfCharSet;
    charFormat.bPitchAndFamily = lf.lfPitchAndFamily;
    ui::StringUtil::StringCopy(charFormat.szFaceName, lf.lfFaceName);
}

void MainForm::OnSetFont()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    //文本颜色
    ui::UiColor textColor;
    if (pRichEdit->IsRichText()) {
        textColor = pRichEdit->GetUiColor(pRichEdit->GetSelectionTextColor());
    }
    else {
        textColor = pRichEdit->GetUiColor(pRichEdit->GetTextColor());
    }

    LOGFONTW logFont = {};
    GetRichEditLogFont(logFont);

    CHOOSEFONTW cf;
    WCHAR szStyleName[64];  // contains style name after return
    LOGFONTW lf;                // default LOGFONTW to store the info

    HWND hWndParent = NativeWnd()->GetHWND();
    LPLOGFONTW lplfInitial = &logFont;
    DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS;
    memset(&cf, 0, sizeof(cf));
    memset(&lf, 0, sizeof(lf));
    memset(&szStyleName, 0, sizeof(szStyleName));

    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = hWndParent;
    cf.rgbColors = textColor.ToCOLORREF();
    cf.lpszStyle = (LPWSTR)&szStyleName;
    cf.Flags = dwFlags;

    if (lplfInitial != nullptr) {
        cf.lpLogFont = lplfInitial;
        cf.Flags |= CF_INITTOLOGFONTSTRUCT;
        lf = *lplfInitial;
    }
    else {
        cf.lpLogFont = &lf;
    }

    BOOL bRet = ::ChooseFontW(&cf);
    if (bRet) {
        memcpy_s(&lf, sizeof(lf), cf.lpLogFont, sizeof(lf));

        //设置RichEdit字体
        CHARFORMAT2W charFormat = {};
        GetCharFormat(charFormat);
        InitCharFormat(lf, charFormat);

        //设置字体颜色
        charFormat.dwMask |= CFM_COLOR;
        charFormat.crTextColor = cf.rgbColors;
        charFormat.dwEffects &= ~CFE_AUTOCOLOR;

        SetCharFormat(charFormat);

        //更新颜色
        ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(_T("color_combo_button")));
        if (pColorComboBtn != nullptr) {
            if (pColorComboBtn->GetLabelBottom() != nullptr) {
                ui::UiColor textColor;
                textColor.SetFromCOLORREF(charFormat.crTextColor);
                pColorComboBtn->GetLabelBottom()->SetBkColor(textColor);
            }
        }
    }

    //更新字体按钮的状态
    UpdateFontStatus();
}

void MainForm::GetCharFormat(CHARFORMAT2W& charFormat) const
{
    charFormat = {};
    charFormat.cbSize = sizeof(CHARFORMAT2W);
    ui::RichEdit* pRichEdit = GetRichEdit();
    ASSERT(pRichEdit != nullptr);
    if (pRichEdit != nullptr) {
        if (pRichEdit->IsRichText()) {
            pRichEdit->GetSelectionCharFormat(charFormat);
        }
        else {
            pRichEdit->GetDefaultCharFormat(charFormat);
        }
    }
}

void MainForm::SetCharFormat(CHARFORMAT2W& charFormat)
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    ASSERT(pRichEdit != nullptr);
    if (pRichEdit != nullptr) {
        if (pRichEdit->IsRichText()) {
            pRichEdit->SetSelectionCharFormat(charFormat);
        }
        else {
            pRichEdit->SetDefaultCharFormat(charFormat);
        }
    }
}

#else //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

bool MainForm::LoadFile(const ui::FilePath& filePath)
{
    //打开文件
    if (m_pRichEdit == nullptr) {
        return false;
    }
    bool bRet = false;
    std::vector<uint8_t> fileData;
    ui::FileUtil::ReadFileData(filePath, fileData);
    if (!fileData.empty()) {
        DStringW text;
        if (ui::StringCharset::GetDataAsString((const char*)fileData.data(), (uint32_t)fileData.size(), text)) {
            m_pRichEdit->SetText(text);
            bRet = true;
        }
    }
    return bRet;
}

bool MainForm::SaveFile(const ui::FilePath& filePath)
{
    //保存
    if (m_pRichEdit == nullptr) {
        return false;
    }    
    DStringW text = m_pRichEdit->GetTextW();
    if (text.empty()) {
        return false;
    }

    bool bRet = false;
    ui::CharsetType charsetType = ui::CharsetType::UNKNOWN;
    std::vector<uint8_t> fileData;
    ui::FileUtil::ReadFileData(filePath, fileData);
    if (!fileData.empty()) {
        //检测原来文件的编码类型，尽量保持一致
        charsetType = ui::StringCharset::GetDataCharsetByBOM((const char*)fileData.data(), (uint32_t)fileData.size());
        if (charsetType == ui::CharsetType::UNKNOWN) {
            charsetType = ui::StringCharset::GetDataCharset((const char*)fileData.data(), (uint32_t)fileData.size());
        }
        fileData.clear();
    }
    if (charsetType == ui::CharsetType::UTF16_LE) {
        bRet = ui::FileUtil::WriteFileData(filePath, text);
    }
    else if (charsetType == ui::CharsetType::ANSI) {
#ifdef DUILIB_BUILD_FOR_WIN
        DStringA textA = ui::StringConvert::UnicodeToMBCS(text);
        bRet = ui::FileUtil::WriteFileData(filePath, textA);
#else
        //按UTF8保存
        DStringA textA = ui::StringConvert::WStringToUTF8(text);
        bRet = ui::FileUtil::WriteFileData(filePath, textA);
#endif        
    }
    else {
        //按UTF8保存
        DStringA textA = ui::StringConvert::WStringToUTF8(text);
        bRet = ui::FileUtil::WriteFileData(filePath, textA);
    }
    return bRet;
}

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
