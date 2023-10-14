#include "MainForm.h"
#include "FindForm.h"
#include "ReplaceForm.h"
#include "MainThread.h"
#include <ShellApi.h>
#include <commdlg.h>
#include <fstream>

const std::wstring MainForm::kClassName = L"MainForm";

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

std::wstring MainForm::GetSkinFolder()
{
	return L"rich_edit";
}

std::wstring MainForm::GetSkinFile()
{
	return L"rich_edit.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
	return kClassName;
}

void MainForm::OnInitWindow()
{
	ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"test_url"));
	if (pRichEdit != nullptr) {
		pRichEdit->AttachLinkClick([this, pRichEdit](const ui::EventArgs& args) {
				//点击了超级链接
				if (args.pSender == pRichEdit) {
					const wchar_t* pUrl = (const wchar_t*)args.wParam;
					if (pUrl != nullptr) {
						::ShellExecute(GetHWND(), L"open", pUrl, NULL, NULL, SW_SHOWNORMAL);
					}
				}
				return true;
			});
	}
	m_pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"rich_edit"));
	ASSERT(m_pRichEdit != nullptr);
	m_findReplace.SetRichEdit(m_pRichEdit);
	LoadRichEditData();

	//文件操作：打开、保存、另存
	ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(L"open_file"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					this->OnOpenFile();
				}
				return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"save_file"));
	if (pButton != nullptr) {
		m_saveBtnText = pButton->GetText();
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					this->OnSaveFile();
				}
				return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"save_as_file"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					this->OnSaveAsFile();
				}
				return true;
			});
	}

	//编辑操作
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_copy"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Copy();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_cut"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Cut();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_paste"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Paste();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_delete"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Clear();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_sel_all"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->SetSelAll();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_sel_none"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->SetSelNone();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_undo"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Undo();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_redo"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Redo();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}

	//查找操作
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_find_text"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				OnFindText();
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_find_next"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				OnFindNext();
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_replace_text"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				OnReplaceText();
			}
			return true;
			});
	}

	//设置字体
	pButton = dynamic_cast<ui::Button*>(FindControl(L"set_font"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				OnSetFont();
			}
			return true;
			});
	}

	//初始化字体信息
	ui::Combo* pFontNameCombo = dynamic_cast<ui::Combo*>(FindControl(L"combo_font_name")); 
	if (pFontNameCombo != nullptr) {		
		GetSystemFontList(m_fontList);
		for (size_t nIndex = 0; nIndex < m_fontList.size(); ++nIndex) {
			const FontInfo& font = m_fontList[nIndex];
			size_t nItemIndex = pFontNameCombo->AddTextItem(font.lf.lfFaceName);
			if (ui::Box::IsValidItemIndex(nItemIndex)) {
				pFontNameCombo->SetItemData(nItemIndex, nIndex);
			}
		}
		pFontNameCombo->AttachSelect([this, pFontNameCombo](const ui::EventArgs& args) {
			std::wstring fontName = pFontNameCombo->GetText();
			SetFontName(fontName);
			return true;
			});
		pFontNameCombo->AttachWindowClose([this, pFontNameCombo](const ui::EventArgs& args) {
			std::wstring fontName = pFontNameCombo->GetText();
			SetFontName(fontName);
			return true;
			});
	}
	ui::Combo* pFontSizeCombo = dynamic_cast<ui::Combo*>(FindControl(L"combo_font_size"));
	if (pFontSizeCombo != nullptr) {
		GetFontSizeList(m_fontSizeList);
		for (size_t nIndex = 0; nIndex < m_fontSizeList.size(); ++nIndex) {
			const FontSizeInfo& fontSize = m_fontSizeList[nIndex];
			size_t nItemIndex = pFontSizeCombo->AddTextItem(fontSize.fontSizeName);
			if (ui::Box::IsValidItemIndex(nItemIndex)) {
				pFontSizeCombo->SetItemData(nItemIndex, nIndex);
			}
		}
		pFontSizeCombo->AttachSelect([this, pFontSizeCombo](const ui::EventArgs& args) {
			std::wstring fontName = pFontSizeCombo->GetText();
			SetFontSize(fontName);
			return true;
			});
		pFontSizeCombo->AttachWindowClose([this, pFontSizeCombo](const ui::EventArgs& args) {
			std::wstring fontName = pFontSizeCombo->GetText();
			SetFontSize(fontName);
			return true;
			});
	}

	//更新是否粗体
	ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_font_bold"));
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
	pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_font_italic"));
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
	pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_font_underline"));
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
	pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_font_strikeout"));
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
	ui::Button* pFontButton = dynamic_cast<ui::Button*>(FindControl(L"btn_font_size_increase"));
	if (pFontButton != nullptr) {
		pFontButton->AttachClick([this](const ui::EventArgs& args) {
			AdjustFontSize(true);
			return true;
			});
	}
	//减小字体大小
	pFontButton = dynamic_cast<ui::Button*>(FindControl(L"btn_font_size_decrease"));
	if (pFontButton != nullptr) {
		pFontButton->AttachClick([this](const ui::EventArgs& args) {
			AdjustFontSize(false);
			return true;
			});
	}

	//设置颜色
	InitColorCombo();
	ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(L"color_combo_button"));
	if (pColorComboBtn != nullptr) {
		std::wstring textColor;
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
	ui::Button* pZoomButtom = dynamic_cast<ui::Button*>(FindControl(L"btn_zoom_in"));
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
	pZoomButtom = dynamic_cast<ui::Button*>(FindControl(L"btn_zoom_out"));
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
	pZoomButtom = dynamic_cast<ui::Button*>(FindControl(L"btn_zoom_off"));
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
	pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_word_wrap"));
	if ((pCheckBox != nullptr) && (m_pRichEdit != nullptr)) {
		pCheckBox->SetSelected(m_pRichEdit->GetWordWrap());
		pCheckBox->AttachSelect([this](const ui::EventArgs& args) {
			if (m_pRichEdit != nullptr) {
				m_pRichEdit->SetWordWrap(true);
				m_pRichEdit->SetAttribute(L"hscrollbar", L"false");
			}
			return true;
			});
		pCheckBox->AttachUnSelect([this](const ui::EventArgs& args) {
			if (m_pRichEdit != nullptr) {
				m_pRichEdit->SetWordWrap(false);
				m_pRichEdit->SetAttribute(L"hscrollbar", L"true");
			}
			return true;
			});
	}

	//是否支持富文本格式
	pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_rich_text"));
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
			const wchar_t* url = (const wchar_t*)args.wParam;
			if (url != nullptr) {
				::MessageBox(GetHWND(), url, L"RichEdit点击超链接", MB_OK);
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
	ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(L"color_combo_button"));
	if (pColorComboBtn == nullptr) {
		return;
	}
	ui::UiSize boxSize = pColorComboBtn->GetDropBoxSize();
	ui::Box* pComboBox = pColorComboBtn->GetComboBox();
	if (pComboBox == nullptr) {
		return;
	}
	pComboBox->SetWindow(this);
	ui::GlobalManager::Instance().FillBoxWithCache(pComboBox, L"rich_edit/color_combox.xml");
	pComboBox->SetFixedHeight(ui::UiFixedInt(boxSize.cy), false, false);
	pComboBox->SetFixedWidth(ui::UiFixedInt(boxSize.cx), false, false);

	if (pComboBox->GetItemAt(0) != nullptr) {
		pComboBox->GetItemAt(0)->SetFixedHeight(ui::UiFixedInt(boxSize.cy), false, false);
		pComboBox->GetItemAt(0)->SetFixedWidth(ui::UiFixedInt(boxSize.cx), false, false);
	}

	ui::ColorPickerRegular* pColorPicker = dynamic_cast<ui::ColorPickerRegular*>(pComboBox->FindSubControl(L"color_combo_picker"));
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

	ui::Button* pMoreColorButton = dynamic_cast<ui::Button*>(pComboBox->FindSubControl(L"color_combo_picker_more"));
	if (pMoreColorButton != nullptr) {
		pMoreColorButton->AttachClick([this](const ui::EventArgs& args) {
			ShowColorPicker();
			return true;
			});
	}
}

void MainForm::ShowColorPicker()
{
	ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(L"color_combo_button"));
	if (pColorComboBtn == nullptr) {
		return;
	}
	ui::Label* pLeftColorLabel = pColorComboBtn->GetLabelBottom();
	if (pLeftColorLabel == nullptr) {
		return;
	}
	std::wstring oldTextColor = pLeftColorLabel->GetBkColor();

	ui::ColorPicker* pColorPicker = new ui::ColorPicker;
	pColorPicker->CreateWnd(GetHWND(), ui::ColorPicker::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
	pColorPicker->CenterWindow();
	pColorPicker->ShowModalFake(this->GetHWND());

	ui::RichEdit* pEdit = m_pRichEdit;
	if (pEdit != nullptr) {
		if (!oldTextColor.empty() && (pColorPicker != nullptr)) {
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
	LOGFONT logFont = {};
	GetRichEditLogFont(logFont);

	//更新字体名称
	ui::Combo* pFontNameCombo = dynamic_cast<ui::Combo*>(FindControl(L"combo_font_name"));
	if (pFontNameCombo != nullptr) {
		pFontNameCombo->SelectTextItem(logFont.lfFaceName);
	}

	//更新字体大小
	UpdateFontSizeStatus();

	//更新是否粗体
	ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_font_bold"));
	if (pCheckBox != nullptr) {
		pCheckBox->SetSelected(logFont.lfWeight >= FW_BOLD);
	}

	//更新是否斜体
	pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_font_italic"));
	if (pCheckBox != nullptr) {
		pCheckBox->SetSelected(logFont.lfItalic != FALSE);
	}

	//更新是否下划线
	pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_font_underline"));
	if (pCheckBox != nullptr) {
		pCheckBox->SetSelected(logFont.lfUnderline != FALSE);
	}

	//更新是否删除线
	pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(L"btn_font_strikeout"));
	if (pCheckBox != nullptr) {
		pCheckBox->SetSelected(logFont.lfStrikeOut != FALSE);
	}
}

void MainForm::UpdateFontSizeStatus()
{
	LOGFONT logFont = {};
	GetRichEditLogFont(logFont);

	ui::Combo* pFontSizeCombo = dynamic_cast<ui::Combo*>(FindControl(L"combo_font_size"));
	if (pFontSizeCombo != nullptr) {
		int32_t fHeight = -logFont.lfHeight;
		size_t maxItemIndex = 0;
		for (size_t nIndex = 0; nIndex < m_fontSizeList.size(); ++nIndex) {
			if (nIndex == (m_fontSizeList.size() - 1)) {
				break;
			}
			if (m_fontSizeList[nIndex].fontSize > m_fontSizeList[nIndex + 1].fontSize) {
				maxItemIndex = nIndex;
				break;
			}
		}

		bool bSelected = false;
		for (size_t nIndex = maxItemIndex; nIndex < m_fontSizeList.size(); ++nIndex) {
			//优先选择汉字的字号
			const FontSizeInfo& fontSize = m_fontSizeList[nIndex];
			if (fHeight == fontSize.fontSize) {
				if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName) != ui::Box::InvalidIndex) {
					bSelected = true;
				}
				break;
			}
		}
		if (!bSelected) {
			for (size_t nIndex = 0; nIndex < maxItemIndex; ++nIndex) {
				//选择数字的字号
				const FontSizeInfo& fontSize = m_fontSizeList[nIndex];
				if (fontSize.fontSize >= fHeight) {
					if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName) != ui::Box::InvalidIndex) {
						bSelected = true;
						break;
					}
				}
			}
		}
	}
}

void MainForm::SetFontName(const std::wstring& fontName)
{
	if (m_pRichEdit == nullptr) {
		return;
	}
	for (const FontInfo& fontInfo : m_fontList) {
		if (fontName == fontInfo.lf.lfFaceName) {
			CHARFORMAT2 charFormat = {};
			GetCharFormat(charFormat);
			if (fontName != charFormat.szFaceName) {
				wcscpy_s(charFormat.szFaceName, fontName.c_str());
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
	HDC hDC = nullptr;
	if (m_pRichEdit != nullptr) {
		hDC = m_pRichEdit->GetWindowDC();
	}
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

void MainForm::SetFontSize(const std::wstring& fontSize)
{
	if (m_pRichEdit == nullptr) {
		return;
	}
	for (const FontSizeInfo& fontSizeInfo : m_fontSizeList) {
		if (fontSize == fontSizeInfo.fontSizeName) {
			CHARFORMAT2 charFormat = {};
			GetCharFormat(charFormat);
			LONG lfHeight = ConvertToFontHeight(fontSizeInfo.fontSize);
			charFormat.cbSize = sizeof(CHARFORMAT2);
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
	for (const FontSizeInfo& fontSizeInfo : m_fontSizeList) {
		fontSizeMap[fontSizeInfo.fontSize] = fontSizeInfo.fontSize;
	}
	std::vector<int32_t> fontSizeList;
	for (auto fontSize : fontSizeMap) {
		fontSizeList.push_back(fontSize.first);
	}
	
	CHARFORMAT2 charFormat = {};
	charFormat.cbSize = sizeof(CHARFORMAT2);
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
					charFormat.cbSize = sizeof(CHARFORMAT2);
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
					charFormat.cbSize = sizeof(CHARFORMAT2);
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
	CHARFORMAT2 charFormat = {};
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
	CHARFORMAT2 charFormat = {};
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
	CHARFORMAT2 charFormat = {};
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
	CHARFORMAT2 charFormat = {};
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

void MainForm::SetTextColor(const std::wstring& newColor)
{
	if (m_pRichEdit == nullptr) {
		return;
	}
	ui::UiColor dwColor = m_pRichEdit->GetUiColor(newColor);
	CHARFORMAT2 charFormat = {};
	GetCharFormat(charFormat);
	charFormat.dwMask = CFM_COLOR;
	charFormat.crTextColor = dwColor.ToCOLORREF();
	SetCharFormat(charFormat);
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

LRESULT MainForm::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	LRESULT lResult = __super::OnKeyUp(uMsg, wParam, lParam, bHandled);
	bool bControlDown = ::GetKeyState(VK_CONTROL) < 0;
	bool bShiftDown = ::GetKeyState(VK_SHIFT) < 0;
	if (bControlDown) {
		if ((wParam == 'O') && !bShiftDown){
			//打开
			OnOpenFile();
		}
		else if (wParam == 'S') {
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
			if (wParam == 'F') {
				//查找
				OnFindText();
			}
			else if (wParam == 'H') {
				//替换
				OnReplaceText();
			}
		}
	}
	if (!bControlDown && !bShiftDown && (::GetKeyState(VK_F3) < 0)) {
		//查找下一个
		OnFindNext();
	}
	return lResult;
}

LRESULT MainForm::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	LRESULT lResult = __super::OnKeyUp(uMsg, wParam, lParam, bHandled);
	UpdateSaveStatus();
	return lResult;
}

void MainForm::UpdateSaveStatus()
{
	if (m_pRichEdit != nullptr) {
		ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(L"save_file"));
		if (m_pRichEdit->GetModify()) {
			if (pButton != nullptr) {
				pButton->SetText(m_saveBtnText + L"*");
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
	std::wstring controls_xml = ui::GlobalManager::Instance().GetResourcePath() + GetResourcePath() + GetSkinFile();

	std::ifstream ifs(controls_xml.c_str());
	if (ifs.is_open()) {
		ifs.seekg(0, std::ios_base::end);
		length = ifs.tellg();
		ifs.seekg(0, std::ios_base::beg);

		xml.resize(static_cast<unsigned int>(length) + 1);
		ifs.read(&xml[0], length);
		ifs.close();
	}
	std::wstring xmlU;
	ui::StringHelper::MBCSToUnicode(xml.c_str(), xmlU, CP_UTF8);

	if (m_pRichEdit != nullptr) {
		m_pRichEdit->SetText(xmlU);
		m_pRichEdit->SetFocus();
		m_pRichEdit->HomeUp();
		m_pRichEdit->SetModify(false);
		m_filePath = controls_xml;
	}
}

LPCTSTR g_defualtFilter = L"所有文件 (*.*)\0*.*\0"
						  L"文本文件 (*.txt)\0*.txt\0"
						  L"RTF文件 (*.rtf)\0*.rtf\0"
						  L"";

void MainForm::OnOpenFile()
{
	TCHAR szFileTitle[_MAX_FNAME] = {0,};   // contains file title after return
	TCHAR szFileName[_MAX_PATH] = {0,};     // contains full path name after return

	OPENFILENAME ofn = {0, };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetHWND();

	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = _MAX_FNAME;
	ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
	ofn.lpstrFilter = g_defualtFilter;
	ofn.hInstance = ::GetModuleHandle(NULL);

	BOOL bRet = ::GetOpenFileName(&ofn);
	if (bRet) {
		if (LoadFile(szFileName)) {
			m_filePath = szFileName;
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
	TCHAR szFileTitle[_MAX_FNAME] = { 0, };   // contains file title after return
	TCHAR szFileName[_MAX_PATH] = { 0, };     // contains full path name after return
	wcscpy_s(szFileName, m_filePath.c_str());

	OPENFILENAME ofn = { 0, };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetHWND();

	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = _MAX_FNAME;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLESIZING;
	ofn.lpstrFilter = g_defualtFilter;
	ofn.hInstance = ::GetModuleHandle(NULL);

	BOOL bRet = ::GetSaveFileName(&ofn);
	if (bRet) {
		if (SaveFile(szFileName)) {
			m_filePath = szFileName;
			if (m_pRichEdit != nullptr) {
				m_pRichEdit->SetModify(false);
				UpdateSaveStatus();
			}
		}
	}
}

bool MainForm::LoadFile(const std::wstring& filePath)
{
	if (m_pRichEdit == nullptr) {
		return false;
	}
	HANDLE hFile = ::CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)hFile;
	es.dwError = 0;
	es.pfnCallback = StreamReadCallback;
	UINT nFormat = IsRtfFile(filePath) ? SF_RTF : SF_TEXT;
	m_pRichEdit->StreamIn(nFormat, es);
	::CloseHandle(hFile);
	return !(BOOL)es.dwError;
}

bool MainForm::SaveFile(const std::wstring& filePath)
{
	if (m_pRichEdit == nullptr) {
		return false;
	}
	HANDLE hFile = ::CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)hFile;
	es.dwError = 0;
	es.pfnCallback = StreamWriteCallback;
	UINT nFormat = IsRtfFile(filePath) ? SF_RTF : SF_TEXT;
	m_pRichEdit->StreamOut(nFormat, es);
	::CloseHandle(hFile);
	return !(BOOL)es.dwError;
}

bool MainForm::IsRtfFile(const std::wstring& filePath) const
{
	std::wstring fileExt;
	size_t pos = filePath.find_last_of(L".");
	if (pos != std::wstring::npos) {
		fileExt = filePath.substr(pos);
		fileExt = ui::StringHelper::MakeLowerString(fileExt);
	}
	return fileExt == L".rtf";
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
		m_pFindForm->CreateWnd(GetHWND(), FindForm::kClassName, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED);
		m_pFindForm->CenterWindow();
		m_pFindForm->ShowWindow();

		m_pFindForm->AttachWindowClose([this](const ui::EventArgs& args) {
				m_pFindForm = nullptr;
				return true;
			});
	}
	else {
		m_pFindForm->ActiveWindow();
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
		m_pReplaceForm->CreateWnd(GetHWND(), ReplaceForm::kClassName, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED);
		m_pReplaceForm->CenterWindow();
		m_pReplaceForm->ShowWindow();
		m_pReplaceForm->AttachWindowClose([this](const ui::EventArgs& args) {
				m_pReplaceForm = nullptr;
				return true;
			});
	}
	else {
		m_pReplaceForm->ActiveWindow();
	}
}

void MainForm::FindRichText(const std::wstring& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	m_findReplace.FindRichText(findText, bFindDown, bMatchCase, bMatchWholeWord, hWndDialog);
}

void MainForm::ReplaceRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	if (m_findReplace.ReplaceRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, hWndDialog)) {
		if (m_pRichEdit != nullptr) {
			m_pRichEdit->SetModify(true);
			UpdateSaveStatus();
		}
	}
}

void MainForm::ReplaceAllRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	if (m_findReplace.ReplaceAllRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, hWndDialog)) {
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
	LOGFONT logfont = {};
	logfont.lfCharSet = DEFAULT_CHARSET;
	logfont.lfFaceName[0] = L'\0';
	logfont.lfPitchAndFamily = 0;
	::EnumFontFamiliesEx(pRichEdit->GetWindowDC(), &logfont, EnumFontFamExProc, (LPARAM)&fontList, 0);

	//字体名称列表
	std::map<std::wstring, FontInfo> fontMap;
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

void MainForm::GetFontSizeList(std::vector<FontSizeInfo>& fontSizeList) const
{
	fontSizeList.clear();
	fontSizeList.push_back({ L"8",  8.0f, 0});
	fontSizeList.push_back({ L"9",  9.0f, 0 });
	fontSizeList.push_back({ L"10", 10.0f, 0 });
	fontSizeList.push_back({ L"11", 11.0f, 0 });
	fontSizeList.push_back({ L"12", 12.0f, 0 });
	fontSizeList.push_back({ L"14", 14.0f, 0 });
	fontSizeList.push_back({ L"16", 16.0f, 0 });
	fontSizeList.push_back({ L"18", 18.0f, 0 });
	fontSizeList.push_back({ L"20", 20.0f, 0 });
	fontSizeList.push_back({ L"22", 22.0f, 0 });
	fontSizeList.push_back({ L"24", 24.0f, 0 });
	fontSizeList.push_back({ L"26", 26.0f, 0 });
	fontSizeList.push_back({ L"28", 28.0f, 0 });
	fontSizeList.push_back({ L"32", 32.0f, 0 });
	fontSizeList.push_back({ L"36", 36.0f, 0 });
	fontSizeList.push_back({ L"48", 48.0f, 0 });
	fontSizeList.push_back({ L"72", 72.0f, 0 });
	fontSizeList.push_back({ L"1英寸", 95.6f, 0 });
	fontSizeList.push_back({ L"大特号", 83.7f, 0 });
	fontSizeList.push_back({ L"特号", 71.7f, 0 });
	fontSizeList.push_back({ L"初号", 56.0f, 0 });
	fontSizeList.push_back({ L"小初", 48.0f, 0 });
	fontSizeList.push_back({ L"一号", 34.7f, 0 });
	fontSizeList.push_back({ L"小一", 32.0f, 0 });
	fontSizeList.push_back({ L"二号", 29.3f, 0 });
	fontSizeList.push_back({ L"小二", 24.0f, 0 });
	fontSizeList.push_back({ L"三号", 21.3f, 0 });
	fontSizeList.push_back({ L"小三", 20.0f, 0 });
	fontSizeList.push_back({ L"四号", 18.7f, 0 });
	fontSizeList.push_back({ L"小四", 16.0f, 0 });
	fontSizeList.push_back({ L"五号", 14.0f, 0 });
	fontSizeList.push_back({ L"小五", 12.0f, 0 });
	fontSizeList.push_back({ L"六号", 10.0f, 0 });
	fontSizeList.push_back({ L"小六", 8.7f, 0 });
	fontSizeList.push_back({ L"七号", 7.3f, 0 });
	fontSizeList.push_back({ L"八号", 6.7f, 0 });

	//更新DPI自适应值
	for (FontSizeInfo& fontSize : fontSizeList) {
		int32_t nSize = static_cast<int32_t>(fontSize.fFontSize * 1000);
		ui::GlobalManager::Instance().Dpi().ScaleInt(nSize);
		fontSize.fontSize = nSize / 1000;
	}
}

bool MainForm::GetRichEditLogFont(LOGFONT& lf) const
{
	ui::RichEdit* pRichEdit = GetRichEdit();
	if (pRichEdit == nullptr) {
		return false;
	}

	CHARFORMAT2 cf = {};
	GetCharFormat(cf);

	if (cf.dwMask & CFM_SIZE) {
		lf.lfHeight = -MulDiv(cf.yHeight, ::GetDeviceCaps(pRichEdit->GetWindowDC(), LOGPIXELSY), LY_PER_INCH);
		lf.lfWidth = 0;
	}

	if (cf.dwMask & CFM_BOLD) {
		lf.lfWeight = cf.dwEffects & CFE_BOLD ? FW_BOLD : 0;
	}

	if (cf.dwMask & CFM_WEIGHT) {
		lf.lfWeight = cf.wWeight;
	}

	if (cf.dwMask & CFM_ITALIC) {
		lf.lfItalic = cf.dwEffects & CFE_ITALIC ? TRUE : FALSE;
	}

	if (cf.dwMask & CFM_UNDERLINE) {
		lf.lfUnderline = cf.dwEffects & CFE_UNDERLINE ? TRUE : FALSE;
	}

	if (cf.dwMask & CFM_STRIKEOUT) {
		lf.lfStrikeOut = cf.dwEffects & CFE_STRIKEOUT ? TRUE : FALSE;
	}

	if (cf.dwMask & CFM_CHARSET) {
		lf.lfCharSet = cf.bCharSet;
	}

	if (cf.dwMask & CFM_FACE) {
		lf.lfPitchAndFamily = cf.bPitchAndFamily;

		//替换为系统字体名称
		std::wstring fontName = ui::FontManager::GetFontSystemName(cf.szFaceName);
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, fontName.c_str());
	}
	return true;
}

void MainForm::InitCharFormat(const LOGFONT& lf, CHARFORMAT2& charFormat) const
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
	wcscpy_s(charFormat.szFaceName, lf.lfFaceName);
}

void MainForm::OnSetFont()
{
	ui::RichEdit* pRichEdit = GetRichEdit();
	if (pRichEdit == nullptr) {
		return;
	}
	//文本颜色
	ui::UiColor textColor = pRichEdit->GetUiColor(pRichEdit->GetTextColor());

	LOGFONT logFont = {};
	GetRichEditLogFont(logFont);

	CHOOSEFONT cf;
	TCHAR szStyleName[64];  // contains style name after return
	LOGFONT lf;			    // default LOGFONT to store the info

	HWND hWndParent = GetHWND();
	LPLOGFONT lplfInitial = &logFont;
	DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS;
	memset(&cf, 0, sizeof(cf));
	memset(&lf, 0, sizeof(lf));
	memset(&szStyleName, 0, sizeof(szStyleName));

	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = hWndParent;
	cf.rgbColors = textColor.ToCOLORREF();
	cf.lpszStyle = (LPTSTR)&szStyleName;
	cf.Flags = dwFlags;

	if (lplfInitial != NULL) {
		cf.lpLogFont = lplfInitial;
		cf.Flags |= CF_INITTOLOGFONTSTRUCT;
		lf = *lplfInitial;
	}
	else {
		cf.lpLogFont = &lf;
	}

	BOOL bRet = ::ChooseFont(&cf);
	if (bRet) {
		memcpy_s(&lf, sizeof(lf), cf.lpLogFont, sizeof(lf));

		//设置RichEdit字体
		CHARFORMAT2 charFormat = {};
		GetCharFormat(charFormat);
		InitCharFormat(lf, charFormat);

		//设置字体颜色
		charFormat.dwMask |= CFM_COLOR;
		charFormat.crTextColor = cf.rgbColors;

		SetCharFormat(charFormat);

		//更新颜色
		ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(L"color_combo_button"));
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

int MainForm::EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* /*lpntme*/, DWORD fontType, LPARAM lParam)
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

	ui::Label* pZoomLabel = dynamic_cast<ui::Label*>(FindControl(L"lavel_zoom_value"));
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
		std::wstring strZoom = ui::StringHelper::Printf(L"%.01f%%", zoomValue / 10.0);
		pZoomLabel->SetText(strZoom);
	}
}

void MainForm::GetCharFormat(CHARFORMAT2& charFormat) const
{
	charFormat = {};
	charFormat.cbSize = sizeof(CHARFORMAT2);
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

void MainForm::SetCharFormat(CHARFORMAT2& charFormat)
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