#ifndef UI_RENDER_GDIPLUS_PATH_H_
#define UI_RENDER_GDIPLUS_PATH_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace Gdiplus
{
	class GraphicsPath;
}

namespace ui 
{

class UILIB_API Path_Gdiplus : public IPath
{
public:
	Path_Gdiplus();
	Path_Gdiplus(const Path_Gdiplus& r);
	Path_Gdiplus& operator=(const Path_Gdiplus& r) = delete;

	virtual IPath* Clone() override;
	virtual void Reset() override;
	virtual void Close() override;

	virtual void SetFillType(FillType mode) override;
	virtual FillType GetFillType() override;

	virtual void AddLine(int x1, int y1, int x2, int y2) override;
	virtual void AddLines(const UiPoint* points, int count) override;
	virtual void AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) override;
	virtual void AddBeziers(const UiPoint* points, int count) override;
	virtual void AddRect(const UiRect& rect) override;
	virtual void AddEllipse(const UiRect& rect) override;
	virtual void AddArc(const UiRect& rect, float startAngle, float sweepAngle) override;
	virtual void AddPolygon(const UiPoint* points, int count) override;
	virtual void AddPolygon(const UiPointF* points, int count) override;
	virtual void Transform(IMatrix* pMatrix) override;

	virtual UiRect GetBounds(const IPen* pen) override;

	Gdiplus::GraphicsPath* GetPath() const;
protected:
	std::unique_ptr<Gdiplus::GraphicsPath> path_;
};

} // namespace ui

#endif // UI_RENDER_GDIPLUS_PATH_H_
