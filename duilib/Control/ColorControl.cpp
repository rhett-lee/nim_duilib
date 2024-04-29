#include "ColorControl.h"
#include "ColorConvert.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

ColorControl::ColorControl()
{
	m_lastPt.x = -1;
	m_lastPt.y = -1;
	m_bMouseDown = false;
}

std::wstring ColorControl::GetType() const { return DUI_CTR_COLOR_CONTROL; }

void ColorControl::PaintBkImage(IRender* pRender)
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

	//画选择位置的显示标志
	if ((m_lastPt.x >= 0) && m_lastPt.y >= 0) {
		UiPoint centerPt = m_lastPt;
		int32_t radius = GlobalManager::Instance().Dpi().GetScaleInt(6);
		UiColor penColor = UiColor(UiColors::Brown);
		int32_t nWidth = GlobalManager::Instance().Dpi().GetScaleInt(2);
		pRender->DrawCircle(centerPt, radius, penColor, nWidth);
	}		
}

bool ColorControl::ButtonDown(const EventArgs& msg)
{
	bool bRet = __super::ButtonDown(msg);
	UiRect rect = GetRect();
	if (rect.ContainsPt(msg.ptMouse)) {
		m_bMouseDown = true;
		SetMouseCapture(true);
		OnSelectPosChanged(rect, msg.ptMouse);
	}
	return bRet;
}

bool ColorControl::MouseMove(const EventArgs& msg)
{
	bool bRet = __super::MouseMove(msg);		
	if (m_bMouseDown) {
		UiRect rect = GetRect();
		OnSelectPosChanged(rect, msg.ptMouse);
	}
	return bRet;
}

bool ColorControl::ButtonUp(const EventArgs& msg)
{
	m_bMouseDown = false;
	SetMouseCapture(false);
	return __super::ButtonUp(msg);
}

void ColorControl::SelectColor(const UiColor& selColor)
{
	m_lastPt.x = -1;
	m_lastPt.y = -1;
	UiRect rect = GetRect();
	const int32_t nHeight = rect.Height();
	const int32_t nWidth = rect.Width();
	if ((nHeight <= 0) || (nWidth <= 0)) {
		return;
	}
	if ((m_spBitmap != nullptr) && (selColor.GetARGB() != UiColors::Black)) {
		double hue = 0;
		double sat = 0;
		double value = 0;
		ColorConvert::RGB2HSV(selColor.GetR() / 255.0, selColor.GetG() / 255.0,
								selColor.GetB() / 255.0, &hue, &sat, &value);
		m_lastPt.x = static_cast<int32_t>((hue / 360) * nWidth);
		m_lastPt.y = static_cast<int32_t>((1.0 - sat) * nHeight);
		if (m_lastPt.x >= nWidth) {
			m_lastPt.x = nWidth - 1;
		}
		if (m_lastPt.y >= nHeight) {
			m_lastPt.y = nHeight - 1;
		}
		m_lastPt.x = rect.left + m_lastPt.x;
		m_lastPt.y = rect.top + m_lastPt.y;
	}
	Invalidate();
}

IBitmap* ColorControl::GetColorBitmap(const UiRect& rect)
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
			double satStep = 1.0 / (nHeight - 1); //每增加一行，sat的增量值
			double sat = 1.0;//第一个数值是1.0
			for (int32_t nRow = 0; nRow < nHeight; ++nRow) {					
				ColorConvert::HSV_HUE(pData, nWidth, sat, 1.0);
				pData += nWidth;
				sat -= satStep;
				if ((sat < 0.0) || (nRow == (nHeight - 2))){
					//最后一个数值是0.0
					sat = 0.0;
				}
			}
			m_spBitmap->UnLockPixelBits();
		}
	}
	return m_spBitmap.get();
}

void ColorControl::OnSelectPosChanged(const UiRect& rect, const UiPoint& pt)
{
	m_lastPt = pt;
	if (m_lastPt.x < rect.left) {
		m_lastPt.x = rect.left;
	}
	if (m_lastPt.x >= rect.right) {
		m_lastPt.x = rect.right - 1;
	}
	if (m_lastPt.y < rect.top) {
		m_lastPt.y = rect.top;
	}
	if (m_lastPt.y >= rect.bottom) {
		m_lastPt.y = rect.bottom - 1;
	}
	const int32_t nWidth = rect.Width();
	const int32_t nHeight = rect.Height();
	if ((nHeight <= 0) || (nWidth <= 0)) {
		return;
	}

	////
	//if (m_spBitmap != nullptr) {
	//	void* pPixelBits = m_spBitmap->LockPixelBits();
	//	if (pPixelBits != nullptr) {
	//		uint32_t* pData = (uint32_t*)pPixelBits;
	//		::OutputDebugString(L"\n");
	//		for (int32_t nRow = 0; nRow < nHeight; ++nRow) {
	//			for (int32_t nColumn = 0; nColumn < nWidth; ++nColumn) {
	//				int32_t index = nRow * nWidth + nColumn;
	//				uint32_t colorData = pData[index];
	//				UiColor color = UiColor(colorData);
	//				color = UiColor(color.GetR(), color.GetG(), color.GetB());
	//				std::wstring strColor = StringHelper::Printf(L"#%02X%02X%02X ", color.GetR(), color.GetG(), color.GetB());
	//				::OutputDebugString(strColor.c_str());
	//			}
	//			::OutputDebugString(L"\n");
	//		}
	//		::OutputDebugString(L"\n");
	//	}
	//}
	////

	if (m_spBitmap != nullptr) {
		int32_t nRow = m_lastPt.y - rect.top;
		int32_t nColumn = m_lastPt.x - rect.left;
		int32_t colorXY = nWidth * nRow + nColumn; //颜色所在点的坐标位置	
		ASSERT(colorXY < nWidth * nHeight);
		void* pPixelBits = m_spBitmap->LockPixelBits();
		if (pPixelBits != nullptr) {
			uint32_t* pData = (uint32_t*)pPixelBits;
			uint32_t colorValue = pData[colorXY];
			UiColor newColor = UiColor(colorValue);
			newColor = UiColor(newColor.GetR(), newColor.GetG(), newColor.GetB());
			SendEvent(kEventSelectColor, newColor.GetARGB());
		}
	}
	Invalidate();
}

void ColorControl::SetMouseCapture(bool bCapture)
{
	Window* pWindow = GetWindow();
	if (pWindow == nullptr) {
		return;
	}
	if (bCapture) {
		pWindow->SetCapture();
	}
	else {
		pWindow->ReleaseCapture();
	}
}

}//namespace ui

