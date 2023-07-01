#include "CircleProgress.h"

namespace ui
{

CircleProgress::CircleProgress() :
	m_bCircular(true),
	m_bClockwise(true),
	m_nCircleWidth(1),
	m_dwBackgroundColor(0),
	m_dwForegroundColor(0),
	m_dwGradientColor(0)
{
}

std::wstring CircleProgress::GetType() const { return DUI_CTR_CIRCLEPROGRESS; }

void CircleProgress::SetAttribute(const std::wstring& srName, const std::wstring& strValue)
{
	if (srName == L"circular") SetCircular(strValue == L"true");
	else if (srName == L"circlewidth") SetCircleWidth(_wtoi(strValue.c_str()));
	else if (srName == L"indicator") SetIndicator(strValue);
	else if (srName == L"clockwise") SetClockwiseRotation(strValue == L"true");
	else if (srName == L"bgcolor") {
		LPCTSTR pValue = strValue.c_str();
		while (*pValue > '\0' && *pValue <= ' ') pValue = ::CharNext(pValue);
		SetBackgroudColor(pValue);
	}
	else if (srName == L"fgcolor") {
		LPCTSTR pValue = strValue.c_str();
		while (*pValue > L'\0' && *pValue <= L' ') pValue = ::CharNext(pValue);
		SetForegroudColor(pValue);
	}
	else if (srName == L"gradientcolor") {
		LPCTSTR pValue = strValue.c_str();
		while (*pValue > L'\0' && *pValue <= L' ') pValue = ::CharNext(pValue);
		SetCircleGradientColor(pValue);
	}
	else Progress::SetAttribute(srName, strValue);
}

void CircleProgress::PaintStatusImage(IRender* pRender)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	Progress::PaintStatusImage(pRender);
	if (!m_bCircular) {
		return;
	}

	IRenderFactory* pRenderFactory = GlobalManager::GetRenderFactory();
	ASSERT(pRenderFactory != nullptr);
	if (pRenderFactory == nullptr) {
		return;
	}
	IPen* bgPen = pRenderFactory->CreatePen(m_dwBackgroundColor, m_nCircleWidth);
	std::unique_ptr<IPen> spBgPen(bgPen);
	ASSERT(bgPen != nullptr);
	if (bgPen == nullptr) {
		return;
	}
	IPen* fgPen = pRenderFactory->CreatePen(m_dwForegroundColor, m_nCircleWidth);
	std::unique_ptr<IPen> spFgPen(fgPen);
	ASSERT(fgPen != nullptr);
	if (fgPen == nullptr) {
		return;
	}

	int direction = m_bClockwise ? 1 : -1;   //旋转方向
	int bordersize = DpiManager::GetInstance()->GetScaledInt(1); //弧度宽度目前使用1像素

	// 圆形中心
	UiPoint center;
	const UiRect& rect = GetRect();
	center.x = rect.left + (rect.right - rect.left) / 2;
	center.y = rect.top + (rect.bottom - rect.top) / 2;

	// 控件矩形内的最大正方形的边界
	int side = std::min(rect.right - rect.left, rect.bottom - rect.top);

	UiRect rcBorder;
	rcBorder.left = center.x - side / 2;
	rcBorder.top = center.y - side / 2;
	rcBorder.right = rcBorder.left + side;
	rcBorder.bottom = rcBorder.top + side;

	//加载图片
	LoadImageData(m_indicatorImage);
	ASSERT(m_indicatorImage.GetImageCache() != nullptr);
	if (m_indicatorImage.GetImageCache() != nullptr) {
		// 假定图片指向上
		//RotateFlip(m_bClockwise ? Rotate90FlipNone : Rotate270FlipNone);
	}

	UiRect outer = rcBorder;
	if (m_indicatorImage.GetImageCache() != nullptr) {
		outer.Inflate(-1 * m_indicatorImage.GetImageCache()->GetWidth() / 2, 
					  -1 * m_indicatorImage.GetImageCache()->GetWidth() / 2);
	}
	else {
		outer.Inflate(-1 * m_nCircleWidth / 2, -1 * m_nCircleWidth / 2);
	}
	int inflateValue = DpiManager::GetInstance()->GetScaledInt(-1);
	outer.Inflate(inflateValue, inflateValue);

	if (m_dwGradientColor.GetARGB() == 0) {
		//不使用渐变色，直接用前景色铺满
		pRender->DrawArc(outer, 270, 360, false, bgPen);

		float sweepAngle = static_cast<float>(direction * 360 * (m_nValue - m_nMin) / (m_nMax - m_nMin));
		pRender->DrawArc(outer, 270, sweepAngle, false, fgPen);
	}
	else {
		//不使用渐变色，直接用前景色铺满
		pRender->DrawArc(outer, 270, 360, false, bgPen);

		float sweepAngle = static_cast<float>(direction * 360 * (m_nValue - m_nMin) / (m_nMax - m_nMin));
		pRender->DrawArc(outer, 270, sweepAngle, false, fgPen, &m_dwGradientColor, &rcBorder);
	}
	
	if (m_indicatorImage.GetImageCache() != nullptr) {

		std::unique_ptr<IMatrix> spMatrix(pRenderFactory->CreateMatrix());
		if ((spMatrix != nullptr) && ((m_nMax - m_nMin) != 0)){
			float angle = direction * 360 * ((float)m_nValue - m_nMin) / (m_nMax - m_nMin);
			spMatrix->RotateAt(angle, UiPoint(center.x, center.y));
		}

		UiRect imageRect;
		imageRect.left = center.x - m_indicatorImage.GetImageCache()->GetWidth() / 2;
		imageRect.top = outer.top + bordersize / 2 - m_indicatorImage.GetImageCache()->GetHeight() / 2;
		imageRect.right = imageRect.left + m_indicatorImage.GetImageCache()->GetWidth();
		imageRect.bottom = imageRect.top + m_indicatorImage.GetImageCache()->GetHeight();
		imageRect.Offset(-GetRect().left, -GetRect().top);
		std::wstring imageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", 
			imageRect.left, imageRect.top, imageRect.right, imageRect.bottom);
		DrawImage(pRender, m_indicatorImage, imageModify, -1, spMatrix.get());
	}
}

void CircleProgress::ClearImageCache()
{
	__super::ClearImageCache();
	m_indicatorImage.ClearImageCache();
}

void CircleProgress::SetCircular(bool bCircular /*= true*/)
{
	m_bCircular = bCircular;
	Invalidate();
}

void CircleProgress::SetClockwiseRotation(bool bClockwise /*= true*/)
{
	m_bClockwise = bClockwise;
}

void CircleProgress::SetCircleWidth(int nCircleWidth)
{
	m_nCircleWidth = nCircleWidth;
	DpiManager::GetInstance()->ScaleInt(m_nCircleWidth);
	Invalidate();
}


void CircleProgress::SetBackgroudColor(const std::wstring& strColor)
{
	m_dwBackgroundColor = GlobalManager::GetTextColor(strColor);
	ASSERT(m_dwBackgroundColor.GetARGB() != 0);
	Invalidate();
}

void CircleProgress::SetForegroudColor(const std::wstring& strColor)
{
	m_dwForegroundColor = GlobalManager::GetTextColor(strColor);
	ASSERT(m_dwForegroundColor.GetARGB() != 0);
	Invalidate();
}

void CircleProgress::SetIndicator(const std::wstring& sIndicatorImage)
{
	if (m_indicatorImage.GetImageString() != sIndicatorImage) {
		m_indicatorImage.ClearImageCache();
		m_indicatorImage.SetImageString(sIndicatorImage);
		Invalidate();		
	}
}

void CircleProgress::SetCircleGradientColor(const std::wstring& strColor)
{
	m_dwGradientColor = GlobalManager::GetTextColor(strColor);
	ASSERT(m_dwGradientColor.GetARGB() != 0);
	Invalidate();
}

}
