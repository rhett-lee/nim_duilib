#include "Path.h"
#include "duilib/Render/Pen.h"
#include "duilib/Render/Matrix.h"
#include "duilib/Image/GdiPlusDefs.h"

namespace ui {

Path_Gdiplus::Path_Gdiplus()
{
	path_.reset(new Gdiplus::GraphicsPath());
}

Path_Gdiplus::Path_Gdiplus(const Path_Gdiplus& r)
	: IPath(r)
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

void Path_Gdiplus::SetFillMode(FillMode mode)
{
	path_->SetFillMode((Gdiplus::FillMode)mode);
}

IPath::FillMode Path_Gdiplus::GetFillMode()
{
	return (IPath::FillMode)path_->GetFillMode();
}

void Path_Gdiplus::StartFigure()
{
	path_->StartFigure();
}

void Path_Gdiplus::CloseFigure()
{
	path_->CloseFigure();
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

void Path_Gdiplus::AddCurve(const UiPoint* points, int count)
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
	    path_->AddCurve(&p[0], static_cast<INT>(p.size()));
	}
}

void Path_Gdiplus::AddRect(int left, int top, int right, int bottom)
{
	path_->AddRectangle(Gdiplus::Rect(left, top, right -left, bottom - top));
}

void Path_Gdiplus::AddRect(const UiRect& rect)
{
	path_->AddRectangle(Gdiplus::Rect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()));
}

void Path_Gdiplus::AddEllipse(int left, int top, int right, int bottom)
{
	path_->AddEllipse(Gdiplus::Rect(left, top, right - left, bottom - top));
}

void Path_Gdiplus::AddEllipse(const UiRect& rect)
{
	path_->AddEllipse(Gdiplus::Rect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()));
}

void Path_Gdiplus::AddArc(int x, int y, int width, int height, float startAngle, float sweepAngle)
{
	path_->AddArc(x, y, width, height, startAngle, sweepAngle);
}

void Path_Gdiplus::AddPie(int x, int y, int width, int height, float startAngle, float sweepAngle)
{
	path_->AddPie(x, y, width, height, startAngle, sweepAngle);
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

ui::UiRect Path_Gdiplus::GetBound(const IPen* pen)
{
	auto p = dynamic_cast<const Pen_GdiPlus*>(pen);
	Gdiplus::Rect rc;
	path_->GetBounds(&rc, NULL, p ? p->GetPen() : NULL);
	return UiRect(rc.X, rc.Y, rc.GetRight(), rc.GetBottom());
}

bool Path_Gdiplus::IsContainsPoint(int x, int y)
{
	return path_->IsVisible(x, y) == TRUE;
}

bool Path_Gdiplus::IsStrokeContainsPoint(int x, int y, const IPen* pen)
{
	ASSERT(pen != nullptr);
	if (pen == nullptr) {
		return false;
	}
	return path_->IsOutlineVisible(x, y, ((Pen_GdiPlus*)pen)->GetPen()) == TRUE;
}

void Path_Gdiplus::Transform(const IMatrix* matrix)
{
	ASSERT(matrix != nullptr);
	if (matrix == nullptr) {
		return;
	}
	auto p = dynamic_cast<const Matrix_Gdiplus*>(matrix);
	ASSERT(p != nullptr);
	if (p) {
		path_->Transform(p->GetMatrix());
	}	
}

Gdiplus::GraphicsPath* Path_Gdiplus::GetPath() const
{ 
	return path_.get(); 
}

} // namespace ui
