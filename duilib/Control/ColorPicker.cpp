#include "ColorPicker.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Control/ColorPickerRegular.h"
#include "duilib/Control/ColorPickerStatard.h"
#include "duilib/Control/ColorPickerStatardGray.h"
#include "duilib/Control/ColorPickerCustom.h"

namespace ui
{

const std::wstring ColorPicker::kClassName = L"ColorPicker";

ColorPicker::ColorPicker():
	m_pNewColor(nullptr),
	m_pOldColor(nullptr),
	m_pRegularPicker(nullptr),
	m_pStatardPicker(nullptr),
	m_pStatardGrayPicker(nullptr),
	m_pCustomPicker(nullptr)
{
}

ColorPicker::~ColorPicker()
{
}

std::wstring ColorPicker::GetSkinFolder()
{
	return L"public";
}

std::wstring ColorPicker::GetSkinFile()
{
	return L"color/color_picker.xml";
}

std::wstring ColorPicker::GetWindowClassName() const
{
	return kClassName;
}

LRESULT ColorPicker::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	UiColor selectedColor;
	if (wParam == 0) {
		//只有点击"确认"按钮的时候，才保存所选择的颜色
		if (m_pNewColor != nullptr) {
			std::wstring bkColor = m_pNewColor->GetBkColor();
			if (!bkColor.empty()) {
				selectedColor = m_pNewColor->GetUiColor(bkColor);
			}
		}
	}	
	m_selectedColor = selectedColor;
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

void ColorPicker::AttachSelectColor(const EventCallback& callback)
{
	m_colorCallback = callback;
}

void ColorPicker::AttachWindowClose(const EventCallback& callback)
{
	__super::AttachWindowClose(callback);
}

void ColorPicker::OnInitWindow()
{
	m_pNewColor = FindControl(L"color_picker_new_color");
	m_pOldColor = FindControl(L"color_picker_old_color");

	ASSERT(m_pNewColor != nullptr);
	ASSERT(m_pOldColor != nullptr);

	m_pRegularPicker = dynamic_cast<ColorPickerRegular*>(FindControl(L"color_picker_regular"));
	m_pStatardPicker = dynamic_cast<ColorPickerStatard*>(FindControl(L"color_picker_standard"));
	m_pStatardGrayPicker = dynamic_cast<ColorPickerStatardGray*>(FindControl(L"color_picker_standard_gray"));
	m_pCustomPicker = dynamic_cast<ColorPickerCustom*>(FindControl(L"color_picker_custom"));

	if (m_pRegularPicker != nullptr) {
		m_pRegularPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			UiColor newColor((uint32_t)args.wParam);
			OnSelectColor(newColor);
			return true;
			});
	}
	if (m_pStatardPicker != nullptr) {
		m_pStatardPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			UiColor newColor((uint32_t)args.wParam);
			OnSelectColor(newColor);
			if (m_pStatardGrayPicker != nullptr) {
				m_pStatardGrayPicker->SelectColor(UiColor());
			}
			return true;
			});
	}
	if (m_pStatardGrayPicker != nullptr) {
		m_pStatardGrayPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			UiColor newColor((uint32_t)args.wParam);
			OnSelectColor(newColor);
			if (m_pStatardPicker != nullptr) {
				m_pStatardPicker->SelectColor(UiColor());
			}
			return true;
			});
	}
	if (m_pCustomPicker != nullptr) {
		m_pCustomPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			UiColor newColor((uint32_t)args.wParam);
			OnSelectColor(newColor);
			return true;
			});
	}

	TabBox* pTabBox = dynamic_cast<TabBox*>(FindControl(L"color_picker_tab"));
	if (pTabBox != nullptr) {
		pTabBox->AttachTabSelect([this](const ui::EventArgs& args) {
			UiColor selectedColor;
			if (m_pNewColor != nullptr) {
				std::wstring bkColor = m_pNewColor->GetBkColor();
				if (!bkColor.empty()) {
					selectedColor = m_pNewColor->GetUiColor(bkColor);
				}				
			}
			if (args.wParam == 0) {
				//常用颜色
				if (m_pRegularPicker != nullptr) {
					m_pRegularPicker->SelectColor(selectedColor);
				}				
			}
			else if (args.wParam == 1) {
				//标准颜色
				if (m_pStatardPicker != nullptr) {
					m_pStatardPicker->SelectColor(selectedColor);
				}
				if (m_pStatardGrayPicker != nullptr) {
					m_pStatardGrayPicker->SelectColor(selectedColor);
				}
			}
			else if (args.wParam == 2) {
				//自定义颜色
				if (m_pCustomPicker != nullptr) {
					m_pCustomPicker->SelectColor(selectedColor);
				}
			}
			return true;
			});
	}

	//确定按钮
	Button* pButton = dynamic_cast<Button*>(FindControl(L"color_picker_ok"));
	if (pButton != nullptr) {
		pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
			this->CloseWnd(0);
			return true;
			});
	}
	//取消按钮
	pButton = dynamic_cast<Button*>(FindControl(L"color_picker_cancel"));
	if (pButton != nullptr) {
		pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
			this->CloseWnd(2);
			return true;
			});
	}

	//选择：屏幕取色
	pButton = dynamic_cast<Button*>(FindControl(L"color_picker_choose"));
	if (pButton != nullptr) {
		pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
			
			return true;
			});
	}
}

void ColorPicker::OnSelectColor(const UiColor& newColor)
{
	UiColor oldColor;
	if (m_pNewColor != nullptr) {
		std::wstring colorString = m_pNewColor->GetBkColor();
		if (!colorString.empty()) {
			oldColor = m_pNewColor->GetUiColor(colorString);
		}
		m_pNewColor->SetBkColor(newColor);
		m_pNewColor->SetToolTipText(m_pNewColor->GetBkColor());
	}
	if (m_colorCallback != nullptr) {
		EventArgs args;
		args.wParam = newColor.GetARGB();
		args.lParam = oldColor.GetARGB();
		m_colorCallback(args);
	}
}

UiColor ColorPicker::GetSelectedColor() const
{
	return m_selectedColor;
}

} // namespace ui
