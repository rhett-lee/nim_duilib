#include "ColorSlider.h"
#include "ColorConvert.h"

namespace ui
{

ColorSlider::ColorSlider():
	m_colorMode(ColorMode::kMode_ARGB)
{
	//TODO: 待实现
	SetColorInfo(UiColor(255, 0, 100, 200), ColorAdjustMode::kMode_ARGB_R);
}

std::wstring ColorSlider::GetType() const { return DUI_CTR_COLOR_SLIDER; }

void ColorSlider::SetColorInfo(const UiColor& color, ColorAdjustMode adjustMode)
{
	m_colorMode = ColorMode::kMode_ARGB;
	m_argbColor = color;
	m_adjustMode = adjustMode;
	m_spBitmap.reset();
	Invalidate();
}

void ColorSlider::SetColorInfo(const ColorHSV& color, ColorAdjustMode adjustMode)
{
	m_colorMode = ColorMode::kMode_HSV;
	m_hsvColor = color;
	m_adjustMode = adjustMode;
	m_spBitmap.reset();
	Invalidate();
}

void ColorSlider::SetColorInfo(const ColorHSL& color, ColorAdjustMode adjustMode)
{
	m_colorMode = ColorMode::kMode_HSL;
	m_hslColor = color;
	m_adjustMode = adjustMode;
	m_spBitmap.reset();
	Invalidate();
}

void ColorSlider::PaintBkImage(IRender* pRender)
{
	__super::PaintBkImage(pRender);
	if (pRender == nullptr) {
		return;
	}
	UiRect rc = GetRect();
	UiRect rcPaint = GetPaintRect();
	IBitmap* pBitmap = GetColorBitmap(rc);
	UiRect rcDest = rc;
	UiRect rcSource;
	rcSource.left = 0;
	rcSource.top = 0;
	rcSource.right = rcSource.left + rc.Width();
	rcSource.bottom = rcSource.top + rc.Height();
	uint8_t uFade = 255;
	IMatrix* pMatrix = nullptr;

	if (pBitmap != nullptr) {
		pRender->DrawImageRect(rcPaint, pBitmap, rcDest, rcSource, uFade, pMatrix);
	}		
}

IBitmap* ColorSlider::GetColorBitmap(const UiRect& rect)
{		
	const int32_t nHeight = rect.Height();
	const int32_t nWidth = rect.Width();
	if ((nHeight <= 0) || (nWidth <= 0)) {
		return nullptr;
	}
	if (m_spBitmap != nullptr) {
		if (((int32_t)m_spBitmap->GetWidth() == nWidth) &&
			((int32_t)m_spBitmap->GetHeight() == nHeight)) {
			//宽度和高度没有变化，不需要重新生成
			return m_spBitmap.get();
		}
		else {
			m_spBitmap.reset();
		}
	}

	IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
	ASSERT(pRenderFactory != nullptr);
	if (pRenderFactory != nullptr) {
		m_spBitmap.reset(pRenderFactory->CreateBitmap());
	}

	if (m_spBitmap != nullptr) {
		BitmapAlphaType alphaType = kOpaque_SkAlphaType;
		if ((m_colorMode == ColorMode::kMode_ARGB) &&
		    (m_adjustMode == ColorAdjustMode::kMode_ARGB_A)) {
			alphaType = kUnpremul_SkAlphaType;
		}
		m_spBitmap->Init(nWidth, nHeight, true, nullptr, alphaType);
		void* pPixelBits = m_spBitmap->LockPixelBits();
		if (pPixelBits != nullptr) {
			uint32_t* pData = (uint32_t*)pPixelBits;
			if (m_colorMode == ColorMode::kMode_ARGB) {
				uint8_t A = m_argbColor.GetA();
				uint8_t R = m_argbColor.GetR();
				uint8_t G = m_argbColor.GetG();
				uint8_t B = m_argbColor.GetB();
				UiColor colorStart;
				UiColor colorEnd;
				if (m_adjustMode == ColorAdjustMode::kMode_ARGB_A) {
					colorStart = UiColor(0, R, G, B);
					colorEnd = UiColor(255, R, G, B);
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						GetARGB(pData, nWidth, colorStart, colorEnd);
						pData += nWidth;
					}
				}
				else if (m_adjustMode == ColorAdjustMode::kMode_ARGB_R) {
					colorStart = UiColor(A, 0, G, B);
					colorEnd = UiColor(A, 255, G, B);						
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						GetARGB(pData, nWidth, colorStart, colorEnd);
						pData += nWidth;
					}
				}
				else if (m_adjustMode == ColorAdjustMode::kMode_ARGB_G) {
					colorStart = UiColor(A, R, 0, B);
					colorEnd = UiColor(A, R, 255, B);
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						GetARGB(pData, nWidth, colorStart, colorEnd);
						pData += nWidth;
					}
				}
				else if (m_adjustMode == ColorAdjustMode::kMode_ARGB_B) {
					colorStart = UiColor(A, R, G, 0);
					colorEnd = UiColor(A, R, G, 255);
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						GetARGB(pData, nWidth, colorStart, colorEnd);
						pData += nWidth;
					}
				}
			}
			else if (m_colorMode == ColorMode::kMode_HSV) {
				double H = m_hsvColor.H * 1.0;
				double S = m_hsvColor.S / 100.0;
				double V = m_hsvColor.V / 100.0;
				if (m_adjustMode == ColorAdjustMode::kMode_HSV_H) {
					//H
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						ColorConvert::HSV_HUE(pData, nWidth, S, V);
						pData += nWidth;
					}
				}
				else if (m_adjustMode == ColorAdjustMode::kMode_HSV_S) {
					//S
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						ColorConvert::HSV_SAT(pData, nWidth, H, V);
						pData += nWidth;
					}
				}
				else {
					//V
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						ColorConvert::HSV_VAL(pData, nWidth, H, S);
						pData += nWidth;
					}
				}
			}
			else if (m_colorMode == ColorMode::kMode_HSL) {
				double H = m_hslColor.H * 1.0;
				double S = m_hslColor.S / 100.0;
				double L = m_hslColor.L / 100.0;
				if (m_adjustMode == ColorAdjustMode::kMode_HSL_H) {
					//H
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						ColorConvert::HSL_HUE(pData, nWidth, S, L);
						pData += nWidth;
					}
				}
				else if (m_adjustMode == ColorAdjustMode::kMode_HSL_S) {
					//S
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						ColorConvert::HSL_SAT(pData, nWidth, H, L);
						pData += nWidth;
					}
				}
				else {
					//L
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						ColorConvert::HSL_LIG(pData, nWidth, H, S);
						pData += nWidth;
					}
				}
			}				
			m_spBitmap->UnLockPixelBits();
		}
	}
	return m_spBitmap.get();
}

void ColorSlider::GetARGB(uint32_t* buffer, int32_t samples,
						  const UiColor& start, 
						  const UiColor& end) const
{
	const int32_t int_extend = 20;

	int32_t alpha = (int32_t)start.GetA() << int_extend;
	int32_t red   = (int32_t)start.GetR() << int_extend;
	int32_t green = (int32_t)start.GetG() << int_extend;
	int32_t blue  = (int32_t)start.GetB() << int_extend;
	int32_t alpha_adv = (((int32_t)end.GetA() << int_extend) - alpha) / (samples - 1);
	int32_t red_adv   = (((int32_t)end.GetR() << int_extend) - red) / (samples - 1);
	int32_t green_adv = (((int32_t)end.GetG() << int_extend) - green) / (samples - 1);
	int32_t blue_adv  = (((int32_t)end.GetB() << int_extend) - blue) / (samples - 1);
		
	while (samples--) {
		// set current pixel (in DIB bitmap format is BGR, not RGB!)
		*buffer++ = UiColor( (uint8_t)(alpha >> int_extend),
							 (uint8_t)(red   >> int_extend),
				             (uint8_t)(green >> int_extend),
					         (uint8_t)(blue  >> int_extend) ).GetARGB();
		// advance color values to the next pixel
		alpha += alpha_adv;
		red += red_adv;
		green += green_adv;
		blue += blue_adv;
	}
}

}//namespace ui
