#include "ColorPickerCustom.h"
#include "duilib/Control/ColorConvert.h"
#include "duilib/Control/ColorPickerRegular.h"

namespace ui
{

ColorPickerCustom::ColorPickerCustom() :
	m_bPickerInited(false),
	m_pRegularPicker(nullptr),
	m_pSpectrumControl(nullptr),
	m_pNewColorEdit(nullptr)
{
}

std::wstring ColorPickerCustom::GetType() const { return DUI_CTR_COLOR_PICKER_CUSTOM; }

void ColorPickerCustom::SelectColor(const UiColor& color)
{
	if (!m_bPickerInited) {
		m_oldColor = color;
		return;
	}
	OnColorChanged(color.GetARGB(), 0, ChangeReason::ColorUpdate);
}

void ColorPickerCustom::SetPos(UiRect rc)
{
	InitPicker();
	__super::SetPos(rc);
}

void ColorPickerCustom::InitPicker()
{
	if (m_bPickerInited) {
		return;
	}
	Window* pWindow = GetWindow();
	if (pWindow == nullptr) {
		return;
	}
			
	m_pRegularPicker = dynamic_cast<ColorPickerRegular*>(pWindow->FindControl(L"color_picker_custom_regular"));
	if (m_pRegularPicker != nullptr) {
		m_pRegularPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			//转发该事件给上层
			OnColorChanged(args.wParam, args.lParam, ChangeReason::ColorRegular);
			return true;
			});
	}

	m_pSpectrumControl = dynamic_cast<ColorControl*>(pWindow->FindControl(L"color_picker_custom_spectrum"));
	if (m_pSpectrumControl != nullptr) {
		m_pSpectrumControl->AttachSelectColor([this](const ui::EventArgs& args) {
			//转发该事件给上层
			OnColorChanged(args.wParam, args.lParam, ChangeReason::ColorSpectrum);
			return true;
			});
	}
		
	m_pNewColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_new_color_edit"));
	if (m_pNewColorEdit != nullptr) {
		m_pNewColorEdit->AttachTextChange([this](const ui::EventArgs& /*args*/) {
			//转发该事件给上层
			std::wstring colorText = m_pNewColorEdit->GetText();
			if (IsValidColorString(colorText)) {
				UiColor newColor = m_pNewColorEdit->GetUiColor(colorText);
				if ((newColor.GetARGB() != 0) && (newColor != m_oldColor)) {
					OnColorChanged(newColor.GetARGB(), m_oldColor.GetARGB(), ChangeReason::NewColorEdit);
				}
			}
			return true;
			});
	}

	m_rgbA.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_RGB_A"));
	m_rgbA.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_RGB_A"));
	m_rgbR.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_RGB_R"));
	m_rgbR.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_RGB_R"));
	m_rgbG.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_RGB_G"));
	m_rgbG.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_RGB_G"));
	m_rgbB.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_RGB_B"));
	m_rgbB.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_RGB_B"));
	InitRGB(m_rgbA, ChangeReason::ColorARGB_A);
	InitRGB(m_rgbR, ChangeReason::ColorARGB_R);
	InitRGB(m_rgbG, ChangeReason::ColorARGB_G);
	InitRGB(m_rgbB, ChangeReason::ColorARGB_B);

	m_hsvH.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_HSV_H"));
	m_hsvH.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_HSV_H"));
	m_hsvS.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_HSV_S"));
	m_hsvS.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_HSV_S"));
	m_hsvV.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_HSV_V"));
	m_hsvV.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_HSV_V"));
	InitHSV(m_hsvH, 359, ChangeReason::ColorHSV_H);
	InitHSV(m_hsvS, 100, ChangeReason::ColorHSV_S);
	InitHSV(m_hsvV, 100, ChangeReason::ColorHSV_V);

	m_hslH.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_HSL_H"));
	m_hslH.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_HSL_H"));
	m_hslS.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_HSL_S"));
	m_hslS.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_HSL_S"));
	m_hslL.m_pColorEdit = dynamic_cast<RichEdit*>(pWindow->FindControl(L"color_picker_edit_HSL_L"));
	m_hslL.m_pColorSlider = dynamic_cast<ColorSlider*>(pWindow->FindControl(L"color_picker_slider_HSL_L"));
	InitHSL(m_hslH, 359, ChangeReason::ColorHSL_H);
	InitHSL(m_hslS, 100, ChangeReason::ColorHSL_S);
	InitHSL(m_hslL, 100, ChangeReason::ColorHSL_L);

	m_bPickerInited = true;
	if (!m_oldColor.IsEmpty()) {
		OnColorChanged(m_oldColor.GetARGB(), 0, ChangeReason::ColorUpdate);
	}
}

bool ColorPickerCustom::IsValidColorString(const std::wstring& colorText) const
{
	if ((colorText.size() == 9) && (colorText.front() == L'#')) {
		for (size_t i = 1; i < colorText.size(); ++i) {
			wchar_t ch = colorText.at(i);
			bool isValid = (((ch >= L'0') && (ch <= L'9')) ||
						    ((ch >= L'a') && (ch <= L'f')) ||
							((ch >= L'A') && (ch <= L'F')));
			if (!isValid) {
				return false;
			}
		}
		return true;
	}
	return false;
}

void ColorPickerCustom::OnColorChanged(WPARAM wParam, LPARAM lParam, ChangeReason reason)
{
	UiColor newColor((uint32_t)wParam);
	if (reason != ChangeReason::NewColorEdit) {
		if (m_pNewColorEdit != nullptr) {
			std::wstring strColor = StringHelper::Printf(L"#%02X%02X%02X%02X",
				newColor.GetA(), newColor.GetR(), newColor.GetG(), newColor.GetB());
			if (strColor != m_pNewColorEdit->GetText()) {
				m_pNewColorEdit->SetTextNoEvent(strColor);
			}
		}
	}

	if (reason != ChangeReason::ColorSpectrum) {
		//选择当前选中的颜色
		if (m_pSpectrumControl != nullptr) {
			m_pSpectrumControl->SelectColor(newColor);
		}
	}

	if (reason != ChangeReason::ColorRegular) {
		if (m_pRegularPicker != nullptr) {
			UiColor color = m_pRegularPicker->GetSelectedColor();
			if ((color != newColor) && (color.GetARGB() != 0)) {
				//清除选择
				m_pRegularPicker->SelectColor(UiColor());
			}
		}
	}

	//更新RGB
	if (reason != ChangeReason::ColorARGB_A) {
		UpdateRGB(m_rgbA, newColor, 0);			
	}
	if (reason != ChangeReason::ColorARGB_R) {
		UpdateRGB(m_rgbR, newColor, 1);
	}
	if (reason != ChangeReason::ColorARGB_G) {
		UpdateRGB(m_rgbG, newColor, 2);
	}
	if (reason != ChangeReason::ColorARGB_B) {
		UpdateRGB(m_rgbB, newColor, 3);
	}

	//更新HSV
	UpdateHSV(m_hsvH, m_hsvS, m_hsvV, newColor, reason);

	//更新HSL
	UpdateHSL(m_hslH, m_hslS, m_hslL, newColor, reason);

	SendEvent(kEventSelectColor, wParam, lParam);
	m_oldColor = newColor;
}

void ColorPickerCustom::InitRGB(const ColorUI& colorUI, ChangeReason reason)
{
	ASSERT(colorUI.m_pColorEdit != nullptr);
	ASSERT(colorUI.m_pColorSlider != nullptr);
	RichEdit* pRichEdit = colorUI.m_pColorEdit;
	ColorSlider* pColorSlider = colorUI.m_pColorSlider;
	if (colorUI.m_pColorEdit != nullptr) {
		colorUI.m_pColorEdit->SetTextNoEvent(L"");
		colorUI.m_pColorEdit->AttachTextChange([this, pRichEdit, pColorSlider, reason](const ui::EventArgs& /*args*/) {
				if (pRichEdit != nullptr) {
					std::wstring text = pRichEdit->GetText();
					int32_t nValue = _wtoi(text.c_str());
					if (nValue < 0) {
						nValue = 0;
						pRichEdit->SetTextNoEvent(L"0");
					}
					else if (nValue > 255) {
						nValue = 255;
						pRichEdit->SetTextNoEvent(L"255");
					}
					if (pColorSlider != nullptr) {
						pColorSlider->SetValue(nValue);
					}
				}
				//输入框文本变化，触发RGB颜色变化事件
				OnRGBChanged(reason);
				return true;
			});
	}
	if (colorUI.m_pColorSlider != nullptr) {
		colorUI.m_pColorSlider->SetMinValue(0);
		colorUI.m_pColorSlider->SetMaxValue(255);
		colorUI.m_pColorSlider->SetValue(0);
		colorUI.m_pColorSlider->AttachValueChange([this, pRichEdit, reason](const ui::EventArgs& args) {
				int32_t value = (int32_t)args.wParam;
				if (pRichEdit != nullptr) {
					std::wstring text = StringHelper::Printf(L"%d", value);
					if (pRichEdit->GetText() != text) {
						pRichEdit->SetTextNoEvent(text);
					}
				}
				//Slider变化，触发RGB颜色变化事件
				OnRGBChanged(reason);
				return true;
			});
	}
}

void ColorPickerCustom::InitHSV(const ColorUI& colorUI, int32_t maxValue, ChangeReason reason)
{
	ASSERT(colorUI.m_pColorEdit != nullptr);
	ASSERT(colorUI.m_pColorSlider != nullptr);
	RichEdit* pRichEdit = colorUI.m_pColorEdit;
	ColorSlider* pColorSlider = colorUI.m_pColorSlider;
	if (colorUI.m_pColorEdit != nullptr) {
		colorUI.m_pColorEdit->SetTextNoEvent(L"");
		colorUI.m_pColorEdit->AttachTextChange([this, pRichEdit, pColorSlider, reason](const ui::EventArgs& /*args*/) {
				if (pRichEdit != nullptr) {
					std::wstring text = pRichEdit->GetText();
					int32_t nValue = _wtoi(text.c_str());
					if (nValue < 0) {
						nValue = 0;
						pRichEdit->SetTextNoEvent(L"0");
					}
					else {
						if (reason == ChangeReason::ColorHSV_H) {
							if (nValue > 359) {
								nValue = 359;
								pRichEdit->SetTextNoEvent(L"359");
							}
						}
						else {
							if (nValue > 100) {
								nValue = 100;
								pRichEdit->SetTextNoEvent(L"100");
							}
						}
					}
					if (pColorSlider != nullptr) {
						pColorSlider->SetValue(nValue);
					}
				}
				//输入框文本变化，触发RGB颜色变化事件
				OnRGBChanged(reason);
				return true;
			});
	}
	if (colorUI.m_pColorSlider != nullptr) {
		colorUI.m_pColorSlider->SetMinValue(0);
		colorUI.m_pColorSlider->SetMaxValue(maxValue);
		colorUI.m_pColorSlider->SetValue(0);
		colorUI.m_pColorSlider->AttachValueChange([this, pRichEdit, reason](const ui::EventArgs& args) {
			int32_t value = (int32_t)args.wParam;
			if (pRichEdit != nullptr) {
				std::wstring text = StringHelper::Printf(L"%d", value);
				if (pRichEdit->GetText() != text) {
					pRichEdit->SetTextNoEvent(text);
				}
			}
			//触发HSV颜色变化事件
			OnHSVChanged(reason);
			return true;
			});
	}
}

void ColorPickerCustom::InitHSL(const ColorUI& colorUI, int32_t maxValue, ChangeReason reason)
{
	ASSERT(colorUI.m_pColorEdit != nullptr);
	ASSERT(colorUI.m_pColorSlider != nullptr);
	RichEdit* pRichEdit = colorUI.m_pColorEdit;
	ColorSlider* pColorSlider = colorUI.m_pColorSlider;
	if (colorUI.m_pColorEdit != nullptr) {
		colorUI.m_pColorEdit->SetTextNoEvent(L"");
		colorUI.m_pColorEdit->AttachTextChange([this, pRichEdit, pColorSlider, reason](const ui::EventArgs& /*args*/) {
				if (pRichEdit != nullptr) {
					std::wstring text = pRichEdit->GetText();
					int32_t nValue = _wtoi(text.c_str());
					if (nValue < 0) {
						nValue = 0;
						pRichEdit->SetTextNoEvent(L"0");
					}
					else {
						if (reason == ChangeReason::ColorHSL_H) {
							if (nValue > 359) {
								nValue = 359;
								pRichEdit->SetTextNoEvent(L"359");
							}
						}
						else {
							if (nValue > 100) {
								nValue = 100;
								pRichEdit->SetTextNoEvent(L"100");
							}
						}
					}
					if (pColorSlider != nullptr) {
						pColorSlider->SetValue(nValue);
					}
				}
				//输入框文本变化，触发RGB颜色变化事件
				OnRGBChanged(reason);
				return true;
			});
	}
	if (colorUI.m_pColorSlider != nullptr) {
		colorUI.m_pColorSlider->SetMinValue(0);
		colorUI.m_pColorSlider->SetMaxValue(maxValue);
		colorUI.m_pColorSlider->SetValue(0);
		colorUI.m_pColorSlider->AttachValueChange([this, pRichEdit, reason](const ui::EventArgs& args) {
			int32_t value = (int32_t)args.wParam;
			if (pRichEdit != nullptr) {
				std::wstring text = StringHelper::Printf(L"%d", value);
				if (pRichEdit->GetText() != text) {
					pRichEdit->SetTextNoEvent(text);
				}
			}
			//触发HSL颜色变化事件
			OnHSLChanged(reason);
			return true;
			});
	}
}

void ColorPickerCustom::UpdateRGB(const ColorUI& colorUI, const UiColor& color, int32_t flag)
{
	ASSERT(colorUI.m_pColorEdit != nullptr);
	ASSERT(colorUI.m_pColorSlider != nullptr);
	int32_t colorValue = 0;
	if (flag == 0) {
		colorValue = color.GetA();
	}
	else if (flag == 1) {
		colorValue = color.GetR();
	}
	else if (flag == 2) {
		colorValue = color.GetG();
	}
	else {
		colorValue = color.GetB();
	}
	if (colorUI.m_pColorEdit != nullptr) {
		std::wstring text = StringHelper::Printf(L"%d", colorValue);
		colorUI.m_pColorEdit->SetTextNoEvent(text);
	}
	if (colorUI.m_pColorSlider != nullptr) {
		colorUI.m_pColorSlider->SetValue(colorValue);
		if (flag == 0) {
			//A
			colorUI.m_pColorSlider->SetColorInfo(color, ColorAdjustMode::kMode_ARGB_A);
		}
		else if (flag == 1) {
			//R
			colorUI.m_pColorSlider->SetColorInfo(color, ColorAdjustMode::kMode_ARGB_R);
		}
		else if (flag == 2) {
			//G
			colorUI.m_pColorSlider->SetColorInfo(color, ColorAdjustMode::kMode_ARGB_G);
		}
		else {
			//B
			colorUI.m_pColorSlider->SetColorInfo(color, ColorAdjustMode::kMode_ARGB_B);
		}
	}
}

void ColorPickerCustom::UpdateHSV(const ColorUI& colorUIH, const ColorUI& colorUIS, const ColorUI& colorUIV, const UiColor& color, ChangeReason reason)
{
	double red = 1.0 * color.GetR() / 255;
	double green = 1.0 * color.GetG() / 255;
	double blue = 1.0 * color.GetB() / 255;
	double hue = 0;
	double sat = 0;
	double value = 0;
	if (ColorConvert::RGB2HSV(red, green, blue, &hue, &sat, &value) != 0) {
		return;
	}

	bool needUpdate = true;
	if ((reason == ChangeReason::ColorHSV_H) ||
		(reason == ChangeReason::ColorHSV_S) ||
		(reason == ChangeReason::ColorHSV_V)) {
		//从控件获取颜色值
		if (colorUIH.m_pColorSlider != nullptr) {
			hue = colorUIH.m_pColorSlider->GetValue();
			if (hue >= 360.0) {
				hue = 359.9;
			}
		}
		if (colorUIS.m_pColorSlider != nullptr) {
			sat = colorUIS.m_pColorSlider->GetValue() / 100.0;
		}
		if (colorUIV.m_pColorSlider != nullptr) {
			value = colorUIV.m_pColorSlider->GetValue() / 100.0;
		}
		needUpdate = false;
	}
	if (needUpdate) {
		if (colorUIH.m_pColorEdit != nullptr) {
			std::wstring text = StringHelper::Printf(L"%d", (int32_t)hue);
			colorUIH.m_pColorEdit->SetTextNoEvent(text);
		}
		if (colorUIH.m_pColorSlider != nullptr) {
			colorUIH.m_pColorSlider->SetValue((int32_t)hue);
		}
	}

	if (needUpdate) {
		if (colorUIS.m_pColorEdit != nullptr) {
			std::wstring text = StringHelper::Printf(L"%d", (int32_t)(sat * 100));
			colorUIS.m_pColorEdit->SetTextNoEvent(text);
		}
		if (colorUIS.m_pColorSlider != nullptr) {
			colorUIS.m_pColorSlider->SetValue((int32_t)(sat * 100));
		}
	}

	if (needUpdate) {
		if (colorUIV.m_pColorEdit != nullptr) {
			std::wstring text = StringHelper::Printf(L"%d", (int32_t)(value * 100));
			colorUIV.m_pColorEdit->SetTextNoEvent(text);
		}
		if (colorUIV.m_pColorSlider != nullptr) {
			colorUIV.m_pColorSlider->SetValue((int32_t)(value * 100));
		}
	}

	//更新背景颜色
	ColorHSV colorHSV;
	colorHSV.H = static_cast<uint16_t>(hue);
	colorHSV.S = static_cast<uint8_t>(sat * 100);
	colorHSV.V = static_cast<uint8_t>(value * 100);
	if (colorUIH.m_pColorSlider != nullptr) {
		colorUIH.m_pColorSlider->SetColorInfo(colorHSV, ColorAdjustMode::kMode_HSV_H);
	}
	if (colorUIS.m_pColorSlider != nullptr) {
		colorUIS.m_pColorSlider->SetColorInfo(colorHSV, ColorAdjustMode::kMode_HSV_S);
	}
	if (colorUIV.m_pColorSlider != nullptr) {
		colorUIV.m_pColorSlider->SetColorInfo(colorHSV, ColorAdjustMode::kMode_HSV_V);
	}
}

void ColorPickerCustom::UpdateHSL(const ColorUI& colorUIH, const ColorUI& colorUIS, const ColorUI& colorUIL, const UiColor& color, ChangeReason reason)
{
	double red = 1.0 * color.GetR() / 255;
	double green = 1.0 * color.GetG() / 255;
	double blue = 1.0 * color.GetB() / 255;
	double hue = 0;
	double sat = 0;
	double lightness = 0;
	if (ColorConvert::RGB2HSL(red, green, blue, &hue, &sat, &lightness) != 0) {
		return;
	}

	bool needUpdate = true;
	if ((reason == ChangeReason::ColorHSL_H) ||
		(reason == ChangeReason::ColorHSL_S) ||
		(reason == ChangeReason::ColorHSL_L)) {
		//从控件获取颜色值
		if (colorUIH.m_pColorSlider != nullptr) {
			hue = colorUIH.m_pColorSlider->GetValue();
			if (hue >= 360.0) {
				hue = 359.9;
			}
		}
		if (colorUIS.m_pColorSlider != nullptr) {
			sat = colorUIS.m_pColorSlider->GetValue() / 100.0;
		}
		if (colorUIL.m_pColorSlider != nullptr) {
			lightness = colorUIL.m_pColorSlider->GetValue() / 100.0;
		}
		needUpdate = false;
	}

	if (needUpdate) {
		if (colorUIH.m_pColorEdit != nullptr) {
			std::wstring text = StringHelper::Printf(L"%d", (int32_t)hue);
			colorUIH.m_pColorEdit->SetTextNoEvent(text);
		}
		if (colorUIH.m_pColorSlider != nullptr) {
			colorUIH.m_pColorSlider->SetValue((int32_t)hue);
		}
	}

	if (needUpdate) {
		if (colorUIS.m_pColorEdit != nullptr) {
			std::wstring text = StringHelper::Printf(L"%d", (int32_t)(sat * 100));
			colorUIS.m_pColorEdit->SetTextNoEvent(text);
		}
		if (colorUIS.m_pColorSlider != nullptr) {
			colorUIS.m_pColorSlider->SetValue((int32_t)(sat * 100));
		}
	}

	if (needUpdate) {
		if (colorUIL.m_pColorEdit != nullptr) {
			std::wstring text = StringHelper::Printf(L"%d", (int32_t)(lightness * 100));
			colorUIL.m_pColorEdit->SetTextNoEvent(text);
		}
		if (colorUIL.m_pColorSlider != nullptr) {
			colorUIL.m_pColorSlider->SetValue((int32_t)(lightness * 100));
		}
	}

	//更新背景颜色
	ColorHSL colorHSL;
	colorHSL.H = static_cast<uint16_t>(hue);
	colorHSL.S = static_cast<uint8_t>(sat * 100);
	colorHSL.L = static_cast<uint8_t>(lightness * 100);
	if (colorUIH.m_pColorSlider != nullptr) {
		colorUIH.m_pColorSlider->SetColorInfo(colorHSL, ColorAdjustMode::kMode_HSL_H);
	}
	if (colorUIS.m_pColorSlider != nullptr) {
		colorUIS.m_pColorSlider->SetColorInfo(colorHSL, ColorAdjustMode::kMode_HSL_S);
	}
	if (colorUIL.m_pColorSlider != nullptr) {
		colorUIL.m_pColorSlider->SetColorInfo(colorHSL, ColorAdjustMode::kMode_HSL_L);
	}
}

void ColorPickerCustom::OnRGBChanged(ChangeReason reason)
{
	uint8_t rgbA = 0;
	uint8_t rgbR = 0;
	uint8_t rgbG = 0;
	uint8_t rgbB = 0;
	if (m_rgbA.m_pColorSlider != nullptr) {
		rgbA = static_cast<uint8_t>(m_rgbA.m_pColorSlider->GetValue());
	}
	if (m_rgbR.m_pColorSlider != nullptr) {
		rgbR = static_cast<uint8_t>(m_rgbR.m_pColorSlider->GetValue());
	}
	if (m_rgbG.m_pColorSlider != nullptr) {
		rgbG = static_cast<uint8_t>(m_rgbG.m_pColorSlider->GetValue());
	}
	if (m_rgbB.m_pColorSlider != nullptr) {
		rgbB = static_cast<uint8_t>(m_rgbB.m_pColorSlider->GetValue());
	}

	UiColor newColor = UiColor(rgbA, rgbR, rgbG, rgbB);
	OnColorChanged(newColor.GetARGB(), m_oldColor.GetARGB(), reason);
}

void ColorPickerCustom::OnHSVChanged(ChangeReason reason)
{
	uint32_t hsvH = 0;
	uint32_t hsvS = 0;
	uint32_t hsvV = 0;
	if (m_hsvH.m_pColorSlider != nullptr) {
		hsvH = static_cast<uint32_t>(m_hsvH.m_pColorSlider->GetValue());
	}
	if (m_hsvS.m_pColorSlider != nullptr) {
		hsvS = static_cast<uint32_t>(m_hsvS.m_pColorSlider->GetValue());
	}
	if (m_hsvV.m_pColorSlider != nullptr) {
		hsvV = static_cast<uint32_t>(m_hsvV.m_pColorSlider->GetValue());
	}

	uint8_t rgbA = 0;
	uint8_t rgbR = 0;
	uint8_t rgbG = 0;
	uint8_t rgbB = 0;
	if (m_rgbA.m_pColorSlider != nullptr) {
		rgbA = static_cast<uint8_t>(m_rgbA.m_pColorSlider->GetValue());
	}

	int nRet = ColorConvert::HSV2RGB(hsvH / 1.0, hsvS / 100.0, hsvV / 100.0, rgbR, rgbG, rgbB);
	ASSERT(nRet == 0);
	if (nRet != 0) {
		return;
	}
	UiColor newColor = UiColor(rgbA, rgbR, rgbG, rgbB);
	OnColorChanged(newColor.GetARGB(), m_oldColor.GetARGB(), reason);
}

void ColorPickerCustom::OnHSLChanged(ChangeReason reason)
{
	uint32_t hslH = 0;
	uint32_t hslS = 0;
	uint32_t hslL = 0;
	if (m_hslH.m_pColorSlider != nullptr) {
		hslH = static_cast<uint32_t>(m_hslH.m_pColorSlider->GetValue());
	}
	if (m_hslS.m_pColorSlider != nullptr) {
		hslS = static_cast<uint32_t>(m_hslS.m_pColorSlider->GetValue());
	}
	if (m_hslL.m_pColorSlider != nullptr) {
		hslL = static_cast<uint32_t>(m_hslL.m_pColorSlider->GetValue());
	}

	uint8_t rgbA = 0;
	uint8_t rgbR = 0;
	uint8_t rgbG = 0;
	uint8_t rgbB = 0;
	if (m_rgbA.m_pColorSlider != nullptr) {
		rgbA = static_cast<uint8_t>(m_rgbA.m_pColorSlider->GetValue());
	}

	int nRet = ColorConvert::HSL2RGB(hslH / 1.0, hslS / 100.0, hslL / 100.0, rgbR, rgbG, rgbB);
	ASSERT(nRet == 0);
	if (nRet != 0) {
		return;
	}
	UiColor newColor = UiColor(rgbA, rgbR, rgbG, rgbB);
	OnColorChanged(newColor.GetARGB(), m_oldColor.GetARGB(), reason);
}

}//namespace ui

