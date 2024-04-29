#include "ColorPickerStatardGray.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

ColorPickerStatardGray::ColorPickerStatardGray()
{
	InitColorMap();
}

std::wstring ColorPickerStatardGray::GetType() const { return DUI_CTR_COLOR_PICKER_STANDARD_GRAY; }

void ColorPickerStatardGray::SelectColor(const UiColor& color)
{
	m_selectedColor = color;
	Invalidate();
}

void ColorPickerStatardGray::Paint(IRender* pRender, const UiRect& rcPaint)
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

UiEstSize ColorPickerStatardGray::EstimateSize(UiSize szAvailable)
{
	UiFixedSize fixedSize = GetFixedSize();
	if (!fixedSize.cx.IsAuto() && !fixedSize.cy.IsAuto()) {
		//如果宽高都不是auto属性，则直接返回
		return MakeEstSize(fixedSize);
	}
	szAvailable.Validate();
	if (!IsReEstimateSize(szAvailable)) {
		//使用缓存中的估算结果
		return GetEstimateSize();
	}

	const UiPadding rcPadding = this->GetControlPadding(); //内边距
	szAvailable.cx -= (rcPadding.left + rcPadding.right);
		
	const float cosRadius = std::cos(30 / 57.2957795f);//边长的30度余弦长度
	int32_t bigRadius = static_cast<int32_t>(szAvailable.cy / 2);
	int32_t smallRadius = bigRadius / 2;
	const int32_t marginX = ui::GlobalManager::Instance().Dpi().GetScaleInt(10);
	int32_t totalWidth = static_cast<int32_t>(smallRadius * 23 * cosRadius + marginX * 2);
	if (totalWidth > szAvailable.cx) {
		smallRadius = static_cast<int32_t>((szAvailable.cx - marginX * 2) / 23 / cosRadius);
		bigRadius = smallRadius * 2;
	}
	totalWidth = static_cast<int32_t>(smallRadius * 23 * cosRadius + marginX * 2);

	UiSize colorSize;
	colorSize.cx = totalWidth;
	colorSize.cy = bigRadius * 2;

	if (colorSize.cx > 0) {
		colorSize.cx += (rcPadding.left + rcPadding.right);
	}
	if (colorSize.cy > 0) {
		colorSize.cy += (rcPadding.top + rcPadding.bottom);
	}

	//选取图片和文本区域高度和宽度的最大值
	if (fixedSize.cx.IsAuto()) {
		fixedSize.cx.SetInt32(colorSize.cx);
	}
	if (fixedSize.cy.IsAuto()) {
		fixedSize.cy.SetInt32(colorSize.cy);
	}
	//保持结果到缓存，避免每次都重新估算
	UiEstSize estSize = MakeEstSize(fixedSize);
	SetEstimateSize(estSize, szAvailable);
	SetReEstimateSize(false);
	return estSize;
}

void ColorPickerStatardGray::DrawColorMap(IRender* pRender, const UiRect& rect)
{
	if (m_colorMap.size() != 16) {
		return;
	}
	const float cosRadius = std::cos(30 / 57.2957795f);//边长的30度余弦长度
	int32_t bigRadius = static_cast<int32_t>(rect.Height() / 2);
	int32_t smallRadius = bigRadius / 2;
	const int32_t marginX = ui::GlobalManager::Instance().Dpi().GetScaleInt(10);
	int32_t totalWidth = static_cast<int32_t>(smallRadius * 23 * cosRadius + marginX * 2);
	if (totalWidth > rect.Width()) {
		smallRadius = static_cast<int32_t>((rect.Width() - marginX * 2) / 23 / cosRadius);
		bigRadius = smallRadius * 2;
	}
	totalWidth = static_cast<int32_t>(smallRadius * 23 * cosRadius + marginX * 2);

	UiPointF firstCenterPt((float)rect.left + bigRadius * cosRadius, (float)rect.top + bigRadius);
	if (rect.Width() > totalWidth) {
		firstCenterPt.x += ((rect.Width() - totalWidth) / 2);
	}
	if (rect.Height() > (bigRadius * 2)) {
		firstCenterPt.y += ((rect.Height() - bigRadius * 2) / 2);
	}
		
	//画第一个大的
	if (!m_colorMap.empty()) {
		UiPointF centerPt = firstCenterPt;
		ColorInfo& colorInfo = m_colorMap[0];
		UiColor penColor = UiColor(UiColors::Orange);
		int32_t penWidth = 0;
		UiColor brushColor = colorInfo.color;
		colorInfo.m_radius = bigRadius;
		colorInfo.centerPt = centerPt;
		if (m_selectedColor == brushColor) {
			//当前选择的颜色，边框加粗显示
			penWidth += 3;
		}
		DrawRegularHexagon(pRender, centerPt, bigRadius, penColor, penWidth, brushColor);
	}

	//画中间小的: 第一排
	UiPointF centerPt = firstCenterPt;
	centerPt.y -= smallRadius;
	centerPt.x += (bigRadius * cosRadius + marginX + smallRadius * 2 * cosRadius);
	size_t startIndex = 1;
	for (size_t index = startIndex; index < (startIndex + 7); ++index) {
		ColorInfo& colorInfo = m_colorMap[index];
		UiColor penColor = UiColor(UiColors::Orange);
		int32_t penWidth = 0;
		UiColor brushColor = colorInfo.color;
		colorInfo.m_radius = smallRadius;
		colorInfo.centerPt = centerPt;
		if (m_selectedColor == brushColor) {
			//当前选择的颜色，边框加粗显示
			penWidth += 3;
		}
		DrawRegularHexagon(pRender, centerPt, smallRadius, penColor, penWidth, brushColor);

		//下一个正六边形的圆心坐标
		centerPt.x += smallRadius * 2 * cosRadius;
	}

	//第二排
	centerPt = firstCenterPt;
	centerPt.y += (smallRadius * 0.5f);
	centerPt.x += (bigRadius * cosRadius + marginX + smallRadius * 1 * cosRadius);
	startIndex = 8;
	for (size_t index = startIndex; index < (startIndex + 7); ++index) {
		ColorInfo& colorInfo = m_colorMap[index];
		UiColor penColor = UiColor(UiColors::Orange);
		int32_t penWidth = 0;
		UiColor brushColor = colorInfo.color;
		colorInfo.m_radius = smallRadius;
		colorInfo.centerPt = centerPt;
		if (m_selectedColor == brushColor) {
			//当前选择的颜色，边框加粗显示
			penWidth += 3;
		}
		DrawRegularHexagon(pRender, centerPt, smallRadius, penColor, penWidth, brushColor);

		//下一个正六边形的圆心坐标
		centerPt.x += smallRadius * 2 * cosRadius;
	}

	//画最后一个大的
	if (!m_colorMap.empty()) {
		centerPt = firstCenterPt;
		centerPt.x += (bigRadius * 2 * cosRadius + marginX * 2 + smallRadius * 15 * cosRadius);
		ColorInfo& colorInfo = m_colorMap[m_colorMap.size() - 1];
		UiColor penColor = UiColor(UiColors::Orange);
		int32_t penWidth = 0;
		UiColor brushColor = colorInfo.color;
		colorInfo.m_radius = bigRadius;
		colorInfo.centerPt = centerPt;
		if (m_selectedColor == brushColor) {
			//当前选择的颜色，边框加粗显示
			penWidth += 3;
		}
		DrawRegularHexagon(pRender, centerPt, bigRadius, penColor, penWidth, brushColor);
	}
}

bool ColorPickerStatardGray::DrawRegularHexagon(IRender* pRender, const UiPointF& centerPt, int32_t radius,
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

bool ColorPickerStatardGray::MouseMove(const EventArgs& msg)
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

bool ColorPickerStatardGray::ButtonDown(const EventArgs& msg)
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

bool ColorPickerStatardGray::GetColorInfo(const UiPoint& ptMouse, UiColor& ptColor) const
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
		if (distance <= colorInfo.m_radius) {
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

float ColorPickerStatardGray::GetPointsDistance(const UiPointF& pt1, const UiPointF& pt2) const
{
	float a = std::abs(pt1.x - pt2.x);
	float b = std::abs(pt1.y - pt2.y);
	float c = std::sqrtf(a * a + b * b);
	return c;
}

void ColorPickerStatardGray::InitColorMap()
{
	m_colorMap = {
		{UiColor(0xFFFFFFFF),UiPointF(),0},
		{UiColor(0xFFDDDDDD),UiPointF(),0},
		{UiColor(0xFFB2B2B2),UiPointF(),0},
		{UiColor(0xFF808080),UiPointF(),0},
		{UiColor(0xFF5F5F5F),UiPointF(),0},
		{UiColor(0xFF333333),UiPointF(),0},
		{UiColor(0xFF1C1C1C),UiPointF(),0},
		{UiColor(0xFF080808),UiPointF(),0},
		{UiColor(0xFFEAEAEA),UiPointF(),0},
		{UiColor(0xFFC0C0C0),UiPointF(),0},
		{UiColor(0xFF969696),UiPointF(),0},
		{UiColor(0xFF777777),UiPointF(),0},
		{UiColor(0xFF4D4D4D),UiPointF(),0},
		{UiColor(0xFF292929),UiPointF(),0},
		{UiColor(0xFF111111),UiPointF(),0},
		{UiColor(0xFF000000),UiPointF(),0}
	};
	ASSERT(m_colorMap.size() == 16);
}

}//namespace ui
