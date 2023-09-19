#include "Path_Gdiplus.h"
#include "duilib/RenderGdiPlus/Pen_Gdiplus.h"
#include "duilib/RenderGdiPlus/Matrix_Gdiplus.h"
#include "duilib/RenderGdiPlus/GdiPlusDefs.h"

namespace ui {

Path_Gdiplus::Path_Gdiplus()
{
	path_.reset(new Gdiplus::GraphicsPath());
}

Path_Gdiplus::Path_Gdiplus(const Path_Gdiplus& r)
{
	path_.reset(r.path_->Clone());
}

ui::IPath* Path_Gdiplus::Clone()
{
	return new Path_Gdiplus(*this);
}

void Path_Gdiplus::Reset()
{
	path_->Reset();
}

void Path_Gdiplus::Close()
{
	path_->CloseFigure();
}

void Path_Gdiplus::SetFillType(FillType mode)
{
	path_->SetFillMode((Gdiplus::FillMode)mode);
}

IPath::FillType Path_Gdiplus::GetFillType()
{
	return (IPath::FillType)path_->GetFillMode();
}

void Path_Gdiplus::AddLine(int x1, int y1, int x2, int y2)
{
	path_->AddLine(x1, y1, x2, y2);
}

void Path_Gdiplus::AddLines(const UiPoint* points, int count)
{
	ASSERT(points != nullptr);
	if (points == nullptr) {
		return;
	}
	std::vector<Gdiplus::Point> p;
	for (int i = 0; i < count; i++)
	{
		p.emplace_back(points[i].x, points[i].y);
	}
	if(!p.empty()){
	    path_->AddLines(&p[0], static_cast<INT>(p.size()));
	}
}

void Path_Gdiplus::AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	path_->AddBezier(x1, y1, x2, y2, x3, y3, x4, y4);
}

void Path_Gdiplus::AddBeziers(const UiPoint* points, int count)
{
	ASSERT(points != nullptr);
	if (points == nullptr) {
		return;
	}
	std::vector<Gdiplus::Point> p;
	for (int i = 0; i < count; i++)
	{
		p.emplace_back(points[i].x, points[i].y);
	}
	if (!p.empty()) {
		path_->AddBeziers(&p[0], static_cast<INT>(p.size()));
	}
}

void Path_Gdiplus::AddRect(const UiRect& rect)
{
	path_->AddRectangle(Gdiplus::Rect(rect.left, rect.top, rect.Width(), rect.Height()));
}

void Path_Gdiplus::AddEllipse(const UiRect& rect)
{
	path_->AddEllipse(Gdiplus::Rect(rect.left, rect.top, rect.Width(), rect.Height()));
}

void Path_Gdiplus::AddArc(const UiRect& rect, float startAngle, float sweepAngle)
{
	path_->AddArc(rect.left, rect.top, rect.Width(), rect.Height(), startAngle, sweepAngle);
}

void Path_Gdiplus::AddPolygon(const UiPoint* points, int count)
{
	ASSERT(points != nullptr);
	if (points == nullptr) {
		return;
	}
	std::vector<Gdiplus::Point> p;
	for (int i = 0; i < count; i++)
	{
		p.emplace_back(points[i].x, points[i].y);
	}
	if(!p.empty()){
	    path_->AddPolygon(&p[0], static_cast<INT>(p.size()));
	}
}

void Path_Gdiplus::AddPolygon(const UiPointF* points, int count)
{
	ASSERT(points != nullptr);
	if (points == nullptr) {
		return;
	}
	std::vector<Gdiplus::PointF> p;
	for (int i = 0; i < count; i++)
	{
		p.emplace_back(points[i].x, points[i].y);
	}
	if (!p.empty()) {
		path_->AddPolygon(&p[0], static_cast<INT>(p.size()));
	}
}

void Path_Gdiplus::Transform(IMatrix* pMatrix)
{
	if (pMatrix != nullptr) {
		Matrix_Gdiplus* pGdiplusMatrix = dynamic_cast<Matrix_Gdiplus*>(pMatrix);
		if (pGdiplusMatrix != nullptr) {
			path_->Transform(pGdiplusMatrix->GetMatrix());
		}
	}
}

ui::UiRect Path_Gdiplus::GetBounds(const IPen* pen)
{
	auto p = dynamic_cast<const Pen_GdiPlus*>(pen);
	Gdiplus::Rect rc;
	path_->GetBounds(&rc, NULL, p ? p->GetPen() : NULL);
	return UiRect(rc.X, rc.Y, rc.GetRight(), rc.GetBottom());
}

Gdiplus::GraphicsPath* Path_Gdiplus::GetPath() const
{ 
	return path_.get(); 
}

} // namespace ui
