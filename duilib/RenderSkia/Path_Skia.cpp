#include "Path_Skia.h"

#pragma warning (push)
#pragma warning (disable: 4244 4201)

#include "include/core/SkPath.h"

#pragma warning (pop)

#include "duilib/RenderSkia/Matrix_Skia.h"

namespace ui {

Path_Skia::Path_Skia()
{
	m_skPath = std::make_unique<SkPath>();
}

Path_Skia::Path_Skia(const Path_Skia& r)
{
	if (r.m_skPath.get() != nullptr) {
		m_skPath = std::make_unique<SkPath>(*r.m_skPath.get());
	}
	else {
		m_skPath = std::make_unique<SkPath>();
	}
}

ui::IPath* Path_Skia::Clone()
{
	return new Path_Skia(*this);
}

void Path_Skia::Reset()
{
	m_skPath->reset();
}

void Path_Skia::Close()
{
	m_skPath->close();
}

void Path_Skia::SetFillType(FillType mode)
{
	switch(mode) {
	case FillType::kEvenOdd:
		m_skPath->setFillType(SkPathFillType::kEvenOdd);
		break;
	case FillType::kWinding:
		m_skPath->setFillType(SkPathFillType::kWinding);
		break;
	case FillType::kInverseEvenOdd:
		m_skPath->setFillType(SkPathFillType::kInverseEvenOdd);
		break;
	case FillType::kInverseWinding:
		m_skPath->setFillType(SkPathFillType::kInverseWinding);
		break;
	default:
		break;
	}
}

IPath::FillType Path_Skia::GetFillType()
{
	IPath::FillType fillType;
	SkPathFillType skPathFillType = m_skPath->getFillType();
	switch (skPathFillType) {
	case SkPathFillType::kEvenOdd:
		fillType = FillType::kEvenOdd;
		break;
	case SkPathFillType::kWinding:
		fillType = FillType::kWinding;
		break;
	case SkPathFillType::kInverseEvenOdd:
		fillType = FillType::kInverseEvenOdd;
		break;
	case SkPathFillType::kInverseWinding:
		fillType = FillType::kInverseWinding;
		break;
	default:
		fillType = FillType::kWinding;
	}
	return fillType;
}

void Path_Skia::MoveToPoint(int x1, int y1)
{
	SkPoint lastPt = SkPoint::Make(0, 0);
	m_skPath->getLastPt(&lastPt);
	if (lastPt != SkPoint::Make(x1, y1)) {
		//如果不相等才调用moveTo函数，否则影响路径的闭合逻辑
		m_skPath->moveTo(SkPoint::Make(x1, y1));
	}
}

void Path_Skia::AddLine(int x1, int y1, int x2, int y2)
{
	MoveToPoint(x1, y1);
	m_skPath->lineTo(SkPoint::Make(x2, y2));
}

void Path_Skia::AddLines(const UiPoint* points, int count)
{
	SkASSERT(points != nullptr);
	SkASSERT(count >= 2);
	if ((count < 2) || (points == nullptr)) {
		return;
	}
	MoveToPoint(points[0].x, points[0].y);
	for (int i = 1; i < count; ++i) {
		m_skPath->lineTo(SkPoint::Make(points[i].x, points[i].y));
	}
}

void Path_Skia::AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	MoveToPoint(x1, y1);
	m_skPath->cubicTo(SkPoint::Make(x2, y2), SkPoint::Make(x3, y3), SkPoint::Make(x4, y4));
}

void Path_Skia::AddBeziers(const UiPoint* points, int count)
{
	SkASSERT(points != nullptr);
	SkASSERT(count >= 4);
	if ((count < 4) || (points == nullptr)) {
		return;
	}
	MoveToPoint(points[0].x, points[0].y);
	int i = 1;
	for (; i < count; i += 3) {
		m_skPath->cubicTo(SkPoint::Make(points[i].x, points[i].y),
						  SkPoint::Make(points[i + 1].x, points[i + 1].y), 
						  SkPoint::Make(points[i + 2].x, points[i + 2].y));
	}
	SkASSERT(i == (count - 1));
}

void Path_Skia::AddRect(const UiRect& rect)
{
	m_skPath->addRect(SkRect::MakeLTRB(rect.left, rect.top, rect.right, rect.bottom));
}

void Path_Skia::AddEllipse(const UiRect& rect)
{
	m_skPath->addOval(SkRect::MakeLTRB(rect.left, rect.top, rect.right, rect.bottom));
}

void Path_Skia::AddArc(const UiRect& rect, float startAngle, float sweepAngle)
{
	m_skPath->arcTo(SkRect::MakeLTRB(rect.left, rect.top, rect.right, rect.bottom), startAngle, sweepAngle, false);
}

void Path_Skia::AddPolygon(const UiPoint* points, int count)
{
	SkASSERT(points != nullptr);
	SkASSERT(count >= 1);
	if ((count < 1) || (points == nullptr)) {
		return;
	}
	SkPoint* skPts = new SkPoint[count];
	for (int i = 0; i < count; ++i)	{
		skPts[i].fX = (float)points[i].x;
		skPts[i].fY = (float)points[i].y;
	}
	m_skPath->addPoly(skPts, count, false);
	delete[] skPts;
}

void Path_Skia::AddPolygon(const UiPointF* points, int count)
{
	SkASSERT(points != nullptr);
	SkASSERT(count >= 1);
	if ((count < 1) || (points == nullptr)) {
		return;
	}
	SkPoint* skPts = new SkPoint[count];
	for (int i = 0; i < count; ++i) {
		skPts[i].fX = points[i].x;
		skPts[i].fY = points[i].y;
	}
	m_skPath->addPoly(skPts, count, false);
	delete[] skPts;
}

void Path_Skia::Transform(IMatrix* pMatrix)
{
	if (pMatrix != nullptr) {
		Matrix_Skia* pSkMatrix = dynamic_cast<Matrix_Skia*>(pMatrix);
		if (pSkMatrix != nullptr) {
			m_skPath->transform(*pSkMatrix->GetMatrix());
		}
	}
}

ui::UiRect Path_Skia::GetBounds(const IPen* /*pen*/)
{
	SkRect bounds = m_skPath->getBounds();
	return ui::UiRect(SkScalarTruncToInt(bounds.fLeft), 
					  SkScalarTruncToInt(bounds.fTop), 
					  SkScalarTruncToInt(bounds.fRight), 
					  SkScalarTruncToInt(bounds.fBottom));
}

SkPath* Path_Skia::GetSkPath() const
{
	return m_skPath.get();
}

} // namespace ui
