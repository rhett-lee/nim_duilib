#include "ColorPickerStatard.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

ColorPickerStatard::ColorPickerStatard():
	m_radius(0)
{
	InitColorMap();
}

std::wstring ColorPickerStatard::GetType() const { return DUI_CTR_COLOR_PICKER_STANDARD; }

void ColorPickerStatard::SelectColor(const UiColor& color)
{
	m_selectedColor = color;
	Invalidate();
}

void ColorPickerStatard::Paint(IRender* pRender, const UiRect& rcPaint)
{
	__super::Paint(pRender, rcPaint);
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
	ASSERT(pRenderFactory != nullptr);
	if (pRenderFactory == nullptr) {
		return;
	}
		
	UiRect rect = GetRect();
	UiPadding rcPadding = this->GetControlPadding(); //内边距
	rect.Deflate(rcPadding);
	DrawColorMap(pRender, rect);
}

void ColorPickerStatard::DrawColorMap(IRender* pRender, const UiRect& rect)
{
	int32_t rectSize = std::min(rect.Width(), rect.Height());
	int32_t radius = static_cast<int32_t>(rectSize / 13 / 2 / std::cos(30 / 57.2957795f)); //半径
	if (m_radius != radius) {
		m_radius = radius;
	}
	const float distance = radius * std::cos(30 / 57.2957795f); //中心点到边的垂直距离

	UiPointF firstCenterPt = UiPointF((float)rect.CenterX(), (float)rect.CenterY()); //矩形中心点坐标
	firstCenterPt.x = firstCenterPt.x - distance * 2 * 6 * std::sin(30 / 57.2957795f); //第一个六边形中心点X坐标
	firstCenterPt.y = firstCenterPt.y - distance * 2 * 6 * std::cos(30 / 57.2957795f); //第一个六边形中心点Y坐标

	size_t colorIndex = 0;
	for (int32_t y = 0; y < 13; ++y) { //共计13行
		int32_t count = 0;
		if (y < 7) {
			count = 7 + y;
		}
		else {
			count = 7 + (13 - y - 1);
		}
		for (int32_t x = 0; x < count; ++x) {
			UiPointF centerPt = firstCenterPt;
			if (y < 7) {
				centerPt.x += distance * 2 * x - distance * y;
			}
			else {
				centerPt.x += distance * 2 * x - distance * (13 - y - 1);
			}
			centerPt.y += radius * 1.5f * y;

			UiColor penColor = UiColor(UiColors::Orange);
			int32_t penWidth = 0;
			UiColor brushColor = UiColor(UiColors::Salmon);
			if (colorIndex < m_colorMap.size()) {
				brushColor = m_colorMap[colorIndex].color;
				m_colorMap[colorIndex].centerPt = centerPt;
				if (m_selectedColor == brushColor) {
					//当前选择的颜色，边框加粗显示
					penWidth += 3;
				}
			}
			DrawRegularHexagon(pRender, centerPt, radius, penColor, penWidth, brushColor);
			++colorIndex;
		}
	}
}

bool ColorPickerStatard::DrawRegularHexagon(IRender* pRender, const UiPointF& centerPt, int32_t radius,
											const UiColor& penColor, int32_t penWidth, const UiColor& brushColor)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return false;
	}
	IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
	ASSERT(pRenderFactory != nullptr);
	if (pRenderFactory == nullptr) {
		return false;
	}
	ASSERT(radius > 0); //多边形的半径
	if (radius <= 0) {
		return false;
	}

	const int32_t count = 6; //多边形的边数
	//正多边形上任意一个顶点的坐标为： x = r * cos(θ) y = r * sin(θ) 
	std::vector<UiPointF> polygonPoints;
	for (int32_t i = 0; i < count; ++i) {
		int32_t degree = i * 60 + 30;// +30是为了使顶点在中心点的最上方
		float radian = degree / 57.2957795f;
		float x = radius * std::cos(radian) + 0.5f;
		float y = radius * std::sin(radian) + 0.5f;
		polygonPoints.push_back(UiPointF(centerPt.x + x, centerPt.y + y));
	}

	std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
	path->AddPolygon(polygonPoints.data(), (int32_t)polygonPoints.size());
	path->Close();

	bool bRet = false;
	if (brushColor.GetARGB() != 0) {
		std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(brushColor));
		pRender->FillPath(path.get(), brush.get());
		bRet = true;
	}
	if ((penColor.GetARGB() != 0) && (penWidth > 0)) {
		std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(penColor, penWidth));
		pRender->DrawPath(path.get(), pen.get());
		bRet = true;
	}
	return bRet;
}

bool ColorPickerStatard::MouseMove(const EventArgs& msg)
{
	//更新ToolTip信息
	if (GetRect().ContainsPt(msg.ptMouse)) {
		UiColor color;
		if (GetColorInfo(msg.ptMouse, color)) {
			std::wstring colorString = StringHelper::Printf(L"#%02X%02X%02X%02X", color.GetA(), color.GetR(), color.GetG(), color.GetB());
			SetToolTipText(colorString);
		}
		else {
			SetToolTipText(L"");
		}
	}
	return __super::MouseMove(msg);
}

bool ColorPickerStatard::ButtonDown(const EventArgs& msg)
{
	bool bRet = __super::ButtonDown(msg);
	if (GetRect().ContainsPt(msg.ptMouse)) {
		UiColor color;
		if (GetColorInfo(msg.ptMouse, color)) {
			//选择了当前的颜色
			m_selectedColor = color;
			Invalidate();
			SendEvent(kEventSelectColor, color.GetARGB());
		}
		else {
			Invalidate();
		}
	}
	return bRet;
}

bool ColorPickerStatard::GetColorInfo(const UiPoint& ptMouse, UiColor& ptColor) const
{
	struct ColorPt
	{
		//颜色值
		UiColor color;
		//该颜色值对应的正六边形中心点与ptMouse之间的距离
		float distance;
		//比较函数，距离最近的排在前面
		bool operator < (const ColorPt& r) const
		{
			return distance < r.distance;
		}
	};

	std::vector<ColorPt> maybeColors;
	for (const ColorInfo& colorInfo : m_colorMap) {
		float distance = GetPointsDistance(colorInfo.centerPt,
											UiPointF((float)ptMouse.x, (float)ptMouse.y));
		if (distance <= m_radius) {
			maybeColors.push_back({ colorInfo.color, distance });
		}
	}

	if (maybeColors.empty()) {
		return false;
	}
	else {
		std::sort(maybeColors.begin(), maybeColors.end()); //选取鼠标点距离正六边形中心点最近的那个颜色
		ptColor = maybeColors.front().color;
		return true;
	}		
}

float ColorPickerStatard::GetPointsDistance(const UiPointF& pt1, const UiPointF& pt2) const
{
	float a = std::abs(pt1.x - pt2.x);
	float b = std::abs(pt1.y - pt2.y);
	float c = std::sqrtf(a * a + b * b);
	return c;
}

void ColorPickerStatard::InitColorMap()
{
	m_colorMap = {
		{UiColor(0xFF003366),UiPointF()},
		{UiColor(0xFF336699),UiPointF()},
		{UiColor(0xFF3366CC),UiPointF()},
		{UiColor(0xFF003399),UiPointF()},
		{UiColor(0xFF000099),UiPointF()},
		{UiColor(0xFF0000CC),UiPointF()},
		{UiColor(0xFF000066),UiPointF()},
		{UiColor(0xFF006666),UiPointF()},
		{UiColor(0xFF006699),UiPointF()},
		{UiColor(0xFF0099CC),UiPointF()},
		{UiColor(0xFF0066CC),UiPointF()},
		{UiColor(0xFF0033CC),UiPointF()},
		{UiColor(0xFF0000FF),UiPointF()},
		{UiColor(0xFF3333FF),UiPointF()},
		{UiColor(0xFF333399),UiPointF()},
		{UiColor(0xFF669999),UiPointF()},
		{UiColor(0xFF009999),UiPointF()},
		{UiColor(0xFF33CCCC),UiPointF()},
		{UiColor(0xFF00CCFF),UiPointF()},
		{UiColor(0xFF0099FF),UiPointF()},
		{UiColor(0xFF0066FF),UiPointF()},
		{UiColor(0xFF3366FF),UiPointF()},
		{UiColor(0xFF3333CC),UiPointF()},
		{UiColor(0xFF666699),UiPointF()},
		{UiColor(0xFF339966),UiPointF()},
		{UiColor(0xFF00CC99),UiPointF()},
		{UiColor(0xFF00FFCC),UiPointF()},
		{UiColor(0xFF00FFFF),UiPointF()},
		{UiColor(0xFF33CCFF),UiPointF()},
		{UiColor(0xFF3399FF),UiPointF()},
		{UiColor(0xFF6699FF),UiPointF()},
		{UiColor(0xFF6666FF),UiPointF()},
		{UiColor(0xFF6600FF),UiPointF()},
		{UiColor(0xFF6600CC),UiPointF()},
		{UiColor(0xFF339933),UiPointF()},
		{UiColor(0xFF00CC66),UiPointF()},
		{UiColor(0xFF00FF99),UiPointF()},
		{UiColor(0xFF66FFCC),UiPointF()},
		{UiColor(0xFF66FFFF),UiPointF()},
		{UiColor(0xFF66CCFF),UiPointF()},
		{UiColor(0xFF99CCFF),UiPointF()},
		{UiColor(0xFF9999FF),UiPointF()},
		{UiColor(0xFF9966FF),UiPointF()},
		{UiColor(0xFF9933FF),UiPointF()},
		{UiColor(0xFF9900FF),UiPointF()},
		{UiColor(0xFF006600),UiPointF()},
		{UiColor(0xFF00CC00),UiPointF()},
		{UiColor(0xFF00FF00),UiPointF()},
		{UiColor(0xFF66FF99),UiPointF()},
		{UiColor(0xFF99FFCC),UiPointF()},
		{UiColor(0xFFCCFFFF),UiPointF()},
		{UiColor(0xFFCCCCFF),UiPointF()},
		{UiColor(0xFFCC99FF),UiPointF()},
		{UiColor(0xFFCC66FF),UiPointF()},
		{UiColor(0xFFCC33FF),UiPointF()},
		{UiColor(0xFFCC00FF),UiPointF()},
		{UiColor(0xFF9900CC),UiPointF()},
		{UiColor(0xFF003300),UiPointF()},
		{UiColor(0xFF009933),UiPointF()},
		{UiColor(0xFF33CC33),UiPointF()},
		{UiColor(0xFF66FF66),UiPointF()},
		{UiColor(0xFF99FF99),UiPointF()},
		{UiColor(0xFFCCFFCC),UiPointF()},
		{UiColor(0xFFFFFFFF),UiPointF()},
		{UiColor(0xFFFFCCFF),UiPointF()},
		{UiColor(0xFFFF99FF),UiPointF()},
		{UiColor(0xFFFF66FF),UiPointF()},
		{UiColor(0xFFFF00FF),UiPointF()},
		{UiColor(0xFFCC00CC),UiPointF()},
		{UiColor(0xFF660066),UiPointF()},
		{UiColor(0xFF336600),UiPointF()},
		{UiColor(0xFF009900),UiPointF()},
		{UiColor(0xFF66FF33),UiPointF()},
		{UiColor(0xFF99FF66),UiPointF()},
		{UiColor(0xFFCCFF99),UiPointF()},
		{UiColor(0xFFFFFFCC),UiPointF()},
		{UiColor(0xFFFFCCCC),UiPointF()},
		{UiColor(0xFFFF99CC),UiPointF()},
		{UiColor(0xFFFF66CC),UiPointF()},
		{UiColor(0xFFFF33CC),UiPointF()},
		{UiColor(0xFFCC0099),UiPointF()},
		{UiColor(0xFF993399),UiPointF()},
		{UiColor(0xFF333300),UiPointF()},
		{UiColor(0xFF669900),UiPointF()},
		{UiColor(0xFF99FF33),UiPointF()},
		{UiColor(0xFFCCFF66),UiPointF()},
		{UiColor(0xFFFFFF99),UiPointF()},
		{UiColor(0xFFFFCC99),UiPointF()},
		{UiColor(0xFFFF9999),UiPointF()},
		{UiColor(0xFFFF6699),UiPointF()},
		{UiColor(0xFFFF3399),UiPointF()},
		{UiColor(0xFFCC3399),UiPointF()},
		{UiColor(0xFF990099),UiPointF()},
		{UiColor(0xFF666633),UiPointF()},
		{UiColor(0xFF99CC00),UiPointF()},
		{UiColor(0xFFCCFF33),UiPointF()},
		{UiColor(0xFFFFFF66),UiPointF()},
		{UiColor(0xFFFFCC66),UiPointF()},
		{UiColor(0xFFFF9966),UiPointF()},
		{UiColor(0xFFFF6666),UiPointF()},
		{UiColor(0xFFFF0066),UiPointF()},
		{UiColor(0xFFCC6699),UiPointF()},
		{UiColor(0xFF993366),UiPointF()},
		{UiColor(0xFF999966),UiPointF()},
		{UiColor(0xFFCCCC00),UiPointF()},
		{UiColor(0xFFFFFF00),UiPointF()},
		{UiColor(0xFFFFCC00),UiPointF()},
		{UiColor(0xFFFF9933),UiPointF()},
		{UiColor(0xFFFF6600),UiPointF()},
		{UiColor(0xFFFF5050),UiPointF()},
		{UiColor(0xFFCC0066),UiPointF()},
		{UiColor(0xFF660033),UiPointF()},
		{UiColor(0xFF996633),UiPointF()},
		{UiColor(0xFFCC9900),UiPointF()},
		{UiColor(0xFFFF9900),UiPointF()},
		{UiColor(0xFFCC6600),UiPointF()},
		{UiColor(0xFFFF3300),UiPointF()},
		{UiColor(0xFFFF0000),UiPointF()},
		{UiColor(0xFFCC0000),UiPointF()},
		{UiColor(0xFF990033),UiPointF()},
		{UiColor(0xFF663300),UiPointF()},
		{UiColor(0xFF996600),UiPointF()},
		{UiColor(0xFFCC3300),UiPointF()},
		{UiColor(0xFF993300),UiPointF()},
		{UiColor(0xFF990000),UiPointF()},
		{UiColor(0xFF800000),UiPointF()},
		{UiColor(0xFF993333),UiPointF()}
	};
	ASSERT(m_colorMap.size() == 127);
}

}//namespace ui
