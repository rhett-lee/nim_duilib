#include "MainForm.h"
#include "FindForm.h"
#include "ReplaceForm.h"
#include "MainThread.h"
#include <ShellApi.h>
#include <commdlg.h>
#include <fstream>

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
                        //TODO: 平台
                        ::ShellExecuteW(NativeWnd()->GetHWND(), L"open", ui::StringUtil::TToUTF16(pUrl).c_str(), NULL, NULL, SW_SHOWNORMAL);
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
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                OnSetFont();
            }
            return true;
            });
    }

    //初始化字体信息
    ui::Combo* pFontNameCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_name"))); 
    if (pFontNameCombo != nullptr) {        
        GetSystemFontList(m_fontList);
        for (size_t nIndex = 0; nIndex < m_fontList.size(); ++nIndex) {
            const FontInfo& font = m_fontList[nIndex];
            size_t nItemIndex = pFontNameCombo->AddTextItem(ui::StringUtil::UTF16ToT(font.lf.lfFaceName));
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

    //缩放比例
    struct ZoomInfo
    {
        int32_t nNum;
        int32_t nDen;

        bool operator < (const ZoomInfo& r)
        {
            if ((nDen == 0) || (r.nDen == 0)) {
                return false;
            }
            return (1.0f * nNum / nDen) < (1.0f * r.nNum / r.nDen);
        }
    };
    std::vector<ZoomInfo> zoomInfoList;
    for (int32_t nDen = 64; nDen >= 1; --nDen) {
        for (int32_t nNum = 1; nNum <= 64; ++nNum) {
            zoomInfoList.push_back({ nNum , nDen });
        }
    }
    zoomInfoList.pop_back();//不包括64/1
    zoomInfoList.erase(zoomInfoList.begin()); //不包括1/64
    std::sort(zoomInfoList.begin(), zoomInfoList.end());//排序：升序

    UpdateZoomValue();
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->AttachZoom([this](const ui::EventArgs& args) {
            UpdateZoomValue();
            return true;
            });
    }
    ui::Button* pZoomButtom = dynamic_cast<ui::Button*>(FindControl(_T("btn_zoom_in")));
    if (pZoomButtom != nullptr) {
        pZoomButtom->AttachClick([this, zoomInfoList](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                //放大：每次放大10%
                int32_t nNum = 0;
                int32_t nDen = 0;
                m_pRichEdit->GetZoom(nNum, nDen);
                if ((nNum > 0) && (nDen > 0)) {
                    float zoomValue = nNum * 100.0f / nDen;
                    zoomValue *= 1.10f;
                    bool bFound = false;
                    for (const ZoomInfo& zoomInfo : zoomInfoList) {
                        if ((zoomInfo.nNum * 100.0f / zoomInfo.nDen) >= zoomValue){
                            m_pRichEdit->SetZoom(zoomInfo.nNum, zoomInfo.nDen);
                            bFound = true;
                            break;
                        }
                    }
                    if (!bFound) {
                        ZoomInfo zoomInfo = zoomInfoList.back();
                        m_pRichEdit->SetZoom(zoomInfo.nNum, zoomInfo.nDen);
                    }
                }
                else {
                    //100%
                    m_pRichEdit->SetZoom(11, 10);
                }
                UpdateZoomValue();
            }
            return true;
            });
    }
    pZoomButtom = dynamic_cast<ui::Button*>(FindControl(_T("btn_zoom_out")));
    if (pZoomButtom != nullptr) {
        pZoomButtom->AttachClick([this, zoomInfoList](const ui::EventArgs& args) {
            if (m_pRichEdit != nullptr) {
                //缩小：每次缩小10%
                int32_t nNum = 0;
                int32_t nDen = 0;
                m_pRichEdit->GetZoom(nNum, nDen);
                if ((nNum > 0) && (nDen > 0)) {
                    float zoomValue = nNum * 100.0f / nDen;
                    zoomValue *= 0.90f;
                    bool bFound = false;
                    int32_t zoomCount = (int32_t)zoomInfoList.size();
                    for (int32_t index = zoomCount - 1; index >= 0; --index) {
                        const ZoomInfo& zoomInfo = zoomInfoList[index];
                        if ((zoomInfo.nNum * 100.0f / zoomInfo.nDen) <= zoomValue) {
                            m_pRichEdit->SetZoom(zoomInfo.nNum, zoomInfo.nDen);
                            bFound = true;
                            break;
                        }
                    }
                    if (!bFound) {
                        ZoomInfo zoomInfo = zoomInfoList.front();
                        m_pRichEdit->SetZoom(zoomInfo.nNum, zoomInfo.nDen);
                    }
                }
                else {
                    //90%
                    m_pRichEdit->SetZoom(9, 10);
                }
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
                m_pRichEdit->SetZoomOff();
                UpdateZoomValue();
            }
            return true;
            });
    }

    //是否自动换行
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_word_wrap")));
    if ((pCheckBox != nullptr) && (m_pRichEdit != nullptr)) {
        pCheckBox->SetSelected(m_pRichEdit->GetWordWrap());
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
    }

    //更新字体按钮的状态
    UpdateFontStatus();

    //超链接
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->AttachLinkClick([this](const ui::EventArgs& args) {
            const DString::value_type* url = (const DString::value_type*)args.wParam;
            if (url != nullptr) {
                //TODO: 平台相关
                ::MessageBoxW(NativeWnd()->GetHWND(), ui::StringUtil::TToUTF16(url).c_str(), L"RichEdit点击超链接", MB_OK);
            }
            return true;
            });
    }
    //RichEdit文本选择变化
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->AttachSelChange([this](const ui::EventArgs& args) {
            if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
                UpdateFontStatus();
            }
            return true;
            });
    }
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
    pComboBox->SetFixedHeight(ui::UiFixedInt(boxSize.cy), false, false);
    pComboBox->SetFixedWidth(ui::UiFixedInt(boxSize.cx), false, false);

    if (pComboBox->GetItemAt(0) != nullptr) {
        pComboBox->GetItemAt(0)->SetFixedHeight(ui::UiFixedInt(boxSize.cy), false, false);
        pComboBox->GetItemAt(0)->SetFixedWidth(ui::UiFixedInt(boxSize.cx), false, false);
    }

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
    createParam.m_dwExStyle = ui::kWS_EX_TOOLWINDOW;
    createParam.m_windowTitle = _T("ColorPicker");
    pColorPicker->CreateWnd(nullptr, createParam);
    pColorPicker->CenterWindow();
    pColorPicker->ShowModalFake();

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
            if ((args.wParam == 0) && !newColor.IsEmpty()) {
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
    LOGFONTW logFont = {};
    GetRichEditLogFont(logFont);

    //更新字体名称
    ui::Combo* pFontNameCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_name")));
    if (pFontNameCombo != nullptr) {
        pFontNameCombo->SelectTextItem(ui::StringUtil::UTF16ToT(logFont.lfFaceName));
    }

    //更新字体大小
    UpdateFontSizeStatus();

    //更新是否粗体
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_bold")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(logFont.lfWeight >= FW_BOLD);
    }

    //更新是否斜体
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_italic")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(logFont.lfItalic != FALSE);
    }

    //更新是否下划线
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_underline")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(logFont.lfUnderline != FALSE);
    }

    //更新是否删除线
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_strikeout")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(logFont.lfStrikeOut != FALSE);
    }
}

void MainForm::UpdateFontSizeStatus()
{
    LOGFONTW logFont = {};
    GetRichEditLogFont(logFont);

    ui::Combo* pFontSizeCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_size")));
    if (pFontSizeCombo != nullptr) {
        int32_t fHeight = -logFont.lfHeight;
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
            if (fHeight == fontSize.fDpiFontSize) {
                if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName) != ui::Box::InvalidIndex) {
                    bSelected = true;
                }
                break;
            }
        }
        if (!bSelected) {
            for (size_t nIndex = 0; nIndex < maxItemIndex; ++nIndex) {
                //选择数字的字号
                const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
                if (fontSize.fDpiFontSize >= fHeight) {
                    if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName) != ui::Box::InvalidIndex) {
                        bSelected = true;
                        break;
                    }
                }
            }
        }
    }
}

void MainForm::SetFontName(const DString& fontName)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    DStringW fontNameW = ui::StringUtil::TToUTF16(fontName);
    for (const FontInfo& fontInfo : m_fontList) {
        if (fontNameW == fontInfo.lf.lfFaceName) {
            CHARFORMAT2W charFormat = {};
            GetCharFormat(charFormat);
            if (fontNameW != charFormat.szFaceName) {
                ui::StringUtil::StringCopy(charFormat.szFaceName, fontNameW.c_str());
                charFormat.dwMask = CFM_FACE;
                SetCharFormat(charFormat);
            }        
            break;
        }
    }
}

int32_t MainForm::ConvertToFontHeight(int32_t fontSize) const
{
    if (m_pRichEdit == nullptr) {
        return fontSize;
    }
    bool bGetDC = false;
    HDC hDC = m_pRichEdit->GetWindowDC();
    if (hDC == nullptr) {
        hDC = ::GetDC(nullptr);
        bGetDC = true;
    }
    LONG yPixPerInch = ::GetDeviceCaps(hDC, LOGPIXELSY);
    if (bGetDC && (hDC != nullptr)) {
        ::ReleaseDC(nullptr, hDC);
        hDC = nullptr;
    }
    if (yPixPerInch == 0) {
        yPixPerInch = 96;
    }
    int32_t lfHeight = fontSize * LY_PER_INCH / yPixPerInch;
    return lfHeight;
}

void MainForm::SetFontSize(const DString& fontSize)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    for (const ui::FontSizeInfo& fontSizeInfo : m_fontSizeList) {
        if (fontSize == fontSizeInfo.fontSizeName) {
            CHARFORMAT2W charFormat = {};
            GetCharFormat(charFormat);
            LONG lfHeight = ConvertToFontHeight((int32_t)fontSizeInfo.fDpiFontSize);
            charFormat.cbSize = sizeof(CHARFORMAT2W);
            charFormat.dwMask = CFM_SIZE;
            if (charFormat.yHeight != lfHeight) {
                charFormat.yHeight = lfHeight;
                SetCharFormat(charFormat);
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
        fontSizeMap[(int32_t)fontSizeInfo.fDpiFontSize] = (int32_t)fontSizeInfo.fDpiFontSize;
    }
    std::vector<int32_t> fontSizeList;
    for (auto fontSize : fontSizeMap) {
        fontSizeList.push_back(fontSize.first);
    }
    
    CHARFORMAT2W charFormat = {};
    charFormat.cbSize = sizeof(CHARFORMAT2W);
    GetCharFormat(charFormat);
    const size_t fontCount = fontSizeList.size();
    for (size_t index = 0; index < fontCount; ++index) {
        LONG lfHeight = ConvertToFontHeight(fontSizeList[index]);
        if (lfHeight == charFormat.yHeight) {
            //匹配到当前字体大小
            if (bIncreaseFontSize) {
                //增加字体
                if (index < (fontCount - 1)) {
                    int32_t newFontSize = fontSizeList[index + 1];
                    lfHeight = ConvertToFontHeight(newFontSize);
                    charFormat.cbSize = sizeof(CHARFORMAT2W);
                    charFormat.dwMask = CFM_SIZE;
                    if (charFormat.yHeight != lfHeight) {
                        charFormat.yHeight = lfHeight;
                        SetCharFormat(charFormat);
                        UpdateFontSizeStatus();
                    }
                }
            }
            else {
                //减小字体
                if (index > 0) {
                    int32_t newFontSize = fontSizeList[index - 1];
                    lfHeight = ConvertToFontHeight(newFontSize);
                    charFormat.cbSize = sizeof(CHARFORMAT2W);
                    charFormat.dwMask = CFM_SIZE;
                    if (charFormat.yHeight != lfHeight) {
                        charFormat.yHeight = lfHeight;
                        SetCharFormat(charFormat);
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
    if (m_pRichEdit == nullptr) {
        return;
    }
    CHARFORMAT2W charFormat = {};
    GetCharFormat(charFormat);
    charFormat.dwMask = CFM_BOLD;
    if (bBold) {
        charFormat.dwEffects |= CFE_BOLD;
    }
    else {
        charFormat.dwEffects &= ~CFE_BOLD;
    }    
    SetCharFormat(charFormat);
}

void MainForm::SetFontItalic(bool bItalic)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    CHARFORMAT2W charFormat = {};
    GetCharFormat(charFormat);
    charFormat.dwMask = CFM_ITALIC;
    if (bItalic) {
        charFormat.dwEffects |= CFE_ITALIC;
    }
    else {
        charFormat.dwEffects &= ~CFE_ITALIC;
    }
    SetCharFormat(charFormat);
}

void MainForm::SetFontUnderline(bool bUnderline)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    CHARFORMAT2W charFormat = {};
    GetCharFormat(charFormat);
    charFormat.dwMask = CFM_UNDERLINE;
    if (bUnderline) {
        charFormat.dwEffects |= CFE_UNDERLINE;
    }
    else {
        charFormat.dwEffects &= ~CFE_UNDERLINE;
    }
    SetCharFormat(charFormat);
}

void MainForm::SetFontStrikeOut(bool bStrikeOut)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    CHARFORMAT2W charFormat = {};
    GetCharFormat(charFormat);
    charFormat.dwMask = CFM_STRIKEOUT;
    if (bStrikeOut) {
        charFormat.dwEffects |= CFE_STRIKEOUT;
    }
    else {
        charFormat.dwEffects &= ~CFE_STRIKEOUT;
    }
    SetCharFormat(charFormat);
}

void MainForm::SetTextColor(const DString& newColor)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    ui::UiColor dwColor = m_pRichEdit->GetUiColor(newColor);
    CHARFORMAT2W charFormat = {};
    GetCharFormat(charFormat);
    charFormat.dwMask = CFM_COLOR;
    charFormat.crTextColor = dwColor.ToCOLORREF();
    charFormat.dwEffects &= ~CFE_AUTOCOLOR;
    SetCharFormat(charFormat);
}

void MainForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMessage(0L);
}

LRESULT MainForm::OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = __super::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
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
    if (!bControlDown && !bShiftDown && ui::Keyboard::IsKeyDown(ui::kVK_F3)) {
        //查找下一个
        OnFindNext();
    }
    return lResult;
}

LRESULT MainForm::OnKeyUpMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = __super::OnKeyUpMsg(vkCode, modifierKey, nativeMsg, bHandled);
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

    std::ifstream ifs(controls_xml.NativePath().c_str());
    if (ifs.is_open()) {
        ifs.seekg(0, std::ios_base::end);
        length = ifs.tellg();
        ifs.seekg(0, std::ios_base::beg);

        xml.resize(static_cast<unsigned int>(length) + 1);
        ifs.read(&xml[0], length);
        ifs.close();
    }
    DString xmlU = ui::StringUtil::UTF8ToT(xml);

    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetText(xmlU);
        m_pRichEdit->SetFocus();
        m_pRichEdit->HomeUp();
        m_pRichEdit->SetModify(false);
        m_filePath = controls_xml;
    }
}

void MainForm::OnOpenFile()
{
    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({ _T("所有文件 (*.*)"), _T("*.*")});
    fileTypes.push_back({ _T("文本文件 (*.txt)"), _T("*.txt") });
    fileTypes.push_back({ _T("RTF文件 (*.rtf)"), _T("*.rtf") });

    DString defaultExt;
    int32_t nFileTypeIndex = 1;
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
        nFileTypeIndex = 2;
    }
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
    fileTypes.push_back({ _T("RTF文件 (*.rtf)"), _T("*.rtf") });

    DString defaultExt;
    int32_t nFileTypeIndex = 1;
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
        nFileTypeIndex = 2;
    }
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

bool MainForm::LoadFile(const ui::FilePath& filePath)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DString filePathLocal = filePath.NativePath();
    HANDLE hFile = ::CreateFile(filePathLocal.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
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
    HANDLE hFile = ::CreateFile(filePathLocal.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
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
    ASSERT(pcb != NULL);

    return !::ReadFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, NULL);
}

DWORD MainForm::StreamWriteCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
    ASSERT(dwCookie != 0);
    ASSERT(pcb != NULL);

    return !::WriteFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, NULL);
}

void MainForm::OnFindText()
{
    if (m_pFindForm == nullptr) {
        m_pFindForm = new FindForm(this);
        ui::WindowCreateParam createParam;
        createParam.m_dwStyle = ui::kWS_POPUP;
        createParam.m_dwExStyle = ui::kWS_EX_TOOLWINDOW;
        createParam.m_windowTitle = _T("FindForm");
        m_pFindForm->CreateWnd(this, createParam);
        m_pFindForm->CenterWindow();
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
        createParam.m_dwExStyle = ui::kWS_EX_TOOLWINDOW;
        createParam.m_windowTitle = _T("ReplaceForm");
        m_pReplaceForm->CreateWnd(this, createParam);
        m_pReplaceForm->CenterWindow();
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

void MainForm::GetSystemFontList(std::vector<FontInfo>& fontList) const
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }

    fontList.clear();
    LOGFONTW logfont = {};
    logfont.lfCharSet = DEFAULT_CHARSET;
    logfont.lfFaceName[0] = L'\0';
    logfont.lfPitchAndFamily = 0;
    ::EnumFontFamiliesExW(pRichEdit->GetWindowDC(), &logfont, EnumFontFamExProc, (LPARAM)&fontList, 0);

    //字体名称列表
    std::map<DStringW, FontInfo> fontMap;
    for (auto font : fontList) {
        if (font.lf.lfWeight != FW_NORMAL) {
            continue;
        }
        if (font.lf.lfFaceName[0] == L'@') {
            continue;
        }
        fontMap[font.lf.lfFaceName] = font;
    }

    fontList.clear();
    for (auto iter : fontMap) {
        fontList.push_back(iter.second);
    }
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
        lf.lfHeight = -MulDiv(cf.yHeight, ::GetDeviceCaps(pRichEdit->GetWindowDC(), LOGPIXELSY), LY_PER_INCH);
        lf.lfWidth = 0;
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
    ui::UiColor textColor = pRichEdit->GetUiColor(pRichEdit->GetTextColor());

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

    if (lplfInitial != NULL) {
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

int MainForm::EnumFontFamExProc(const LOGFONTW* lpelfe, const TEXTMETRICW* /*lpntme*/, DWORD fontType, LPARAM lParam)
{
    std::vector<FontInfo>* pFontList = (std::vector<FontInfo>*)lParam;
    if (pFontList != nullptr) {
        FontInfo fontInfo;
        if (lpelfe != nullptr) {
            fontInfo.lf = *lpelfe;
        }
        else {
            fontInfo.lf = {};
        }
        fontInfo.fontType = fontType;
        pFontList->emplace_back(std::move(fontInfo));
        return 1;
    }
    else {
        return 0;
    }
}

void MainForm::UpdateZoomValue()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }

    ui::Label* pZoomLabel = dynamic_cast<ui::Label*>(FindControl(_T("lavel_zoom_value")));
    if (pZoomLabel != nullptr) {
        int32_t nNum = 0;
        int32_t nDen = 0;
        int32_t zoomValue = 100;
        pRichEdit->GetZoom(nNum, nDen);
        if ((nNum <= 0) || (nDen <= 0)) {
            zoomValue = 1000;
        }
        else {
            zoomValue = nNum * 1000 / nDen;
        }
        DString strZoom = ui::StringUtil::Printf(_T("%.01f%%"), zoomValue / 10.0);
        pZoomLabel->SetText(strZoom);
    }
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
