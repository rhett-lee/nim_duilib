#include "cef_control.h"
#include "ui_components/cef_control/util/util.h"

namespace nim_comp {

void CefControl::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList& /*dirtyRects*/, const std::string* buffer, int width, int height)
{
	//必须不使用缓存，否则绘制异常
	ASSERT(IsUseCache() == false);

	if (NULL == buffer)
		return;

	if (type == PET_VIEW)
	{
		if (dc_cef_.GetWidth() != width || dc_cef_.GetHeight() != height)
			dc_cef_.Init(GetWindow()->GetPaintDC(), width, height);

		LPBYTE pDst = (LPBYTE)dc_cef_.GetBits();
		if (pDst)
			memcpy(pDst, (char*)buffer->c_str(), height * width * 4);
	}
	else if (type == PET_POPUP && dc_cef_.IsValid() && rect_popup_.width > 0 && rect_popup_.height > 0)
	{
		// 单独保存popup窗口的位图
		if (dc_cef_popup_.GetWidth() != width || dc_cef_popup_.GetHeight() != height)
			dc_cef_popup_.Init(GetWindow()->GetPaintDC(), width, height);

		LPBYTE pDst = (LPBYTE)dc_cef_popup_.GetBits();
		if (pDst)
			memcpy(pDst, (char*)buffer->c_str(), width * height * 4);
	}

	this->Invalidate();
}

void CefControl::ClientToControl(POINT &pt)
{
	auto offset = GetScrollOffsetInScrollBox();
	pt.x = pt.x + offset.x - GetRect().left;
	pt.y = pt.y + offset.y - GetRect().top;
}

void CefControl::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
	if (!show)
	{
		// 当popup窗口隐藏时，刷新popup区域
		CefRect rect_dirty = rect_popup_;
		rect_popup_.Set(0, 0, 0, 0);
		browser->GetHost()->Invalidate(PET_VIEW);
	}
}

void CefControl::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
	if (rect.width <= 0 || rect.height <= 0)
		return;

	rect_popup_ = rect;
}

}