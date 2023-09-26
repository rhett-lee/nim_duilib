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
		m_spBitmap->Init(nWidth, nHeight, true, nullptr, kOpaque_SkAlphaType);
		void* pPixelBits = m_spBitmap->LockPixelBits();
		if (pPixelBits != nullptr) {
			uint32_t* pData = (uint32_t*)pPixelBits;
			if (m_colorMode == ColorMode::kMode_ARGB) {
				UiColor colorStart;
				UiColor colorEnd;
				if (m_adjustMode == ColorAdjustMode::kMode_ARGB_A) {
					colorStart = UiColor(0, m_argbColor.GetR(), m_argbColor.GetG(), m_argbColor.GetB());
					colorEnd = UiColor(255, m_argbColor.GetR(), m_argbColor.GetG(), m_argbColor.GetB());
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						GetARGB(pData, nWidth, colorStart, colorEnd);
						pData += nWidth;
					}
				}
				else if (m_adjustMode == ColorAdjustMode::kMode_ARGB_R) {
					colorStart = UiColor(m_argbColor.GetA(), 0, m_argbColor.GetG(), m_argbColor.GetB());
					colorEnd = UiColor(m_argbColor.GetA(), 255, m_argbColor.GetG(), m_argbColor.GetB());						
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						GetARGB(pData, nWidth, colorStart, colorEnd);
						pData += nWidth;
					}
				}
				else if (m_adjustMode == ColorAdjustMode::kMode_ARGB_G) {
					colorStart = UiColor(m_argbColor.GetA(), m_argbColor.GetR(), 0, m_argbColor.GetB());
					colorEnd = UiColor(m_argbColor.GetA(), m_argbColor.GetR(), 255, m_argbColor.GetB());
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						GetARGB(pData, nWidth, colorStart, colorEnd);
						pData += nWidth;
					}
				}
				else if (m_adjustMode == ColorAdjustMode::kMode_ARGB_B) {
					colorStart = UiColor(m_argbColor.GetA(), m_argbColor.GetR(), m_argbColor.GetG(), 0);
					colorEnd = UiColor(m_argbColor.GetA(), m_argbColor.GetR(), m_argbColor.GetG(), 255);
					for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
						GetARGB(pData, nWidth, colorStart, colorEnd);
						pData += nWidth;
					}
				}
			}
			else if (m_colorMode == ColorMode::kMode_HSV) {
				double satStep = 1.0 / (nHeight - 1); //每增加一行，sat的增量值
				double sat = 1.0;//第一个数值是1.0
				for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
					ColorConvert::HSV_HUE(pData, nWidth, sat, 1.0);
					pData += nWidth;
					sat -= satStep;
					if ((sat < 0.0) || (nRow == (nHeight - 2))) {
						//最后一个数值是0.0
						sat = 0.0;
					}
				}
			}
			else if (m_colorMode == ColorMode::kMode_HSL) {

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
