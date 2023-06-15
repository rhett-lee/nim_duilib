#include "Clip_GDI.h"

namespace ui {

Clip_GDI::Clip_GDI()
{

}

Clip_GDI::~Clip_GDI()
{

}

void Clip_GDI::CreateClip(HDC hDC, UiRect rcItem)
{
	if (hDC != NULL) {
		UiPoint ptWinOrg;
		GetWindowOrgEx(hDC, &ptWinOrg);
		rcItem.Offset(-ptWinOrg.x, -ptWinOrg.y);

		HRGN hRgn = ::CreateRectRgnIndirect(&rcItem);
		::SaveDC(hDC);
		::ExtSelectClipRgn(hDC, hRgn, RGN_AND);
		::DeleteObject(hRgn);
	}
}

void Clip_GDI::CreateRoundClip(HDC hDC, UiRect rcItem, int width, int height)
{
	if (hDC != NULL) {
		UiPoint ptWinOrg;
		GetWindowOrgEx(hDC, &ptWinOrg);
		rcItem.Offset(-ptWinOrg.x, -ptWinOrg.y);

		HRGN hRgn = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, width, height);
		::SaveDC(hDC);
		::ExtSelectClipRgn(hDC, hRgn, RGN_AND);
		::DeleteObject(hRgn);
	}
}

void Clip_GDI::ClearClip(HDC hDC)
{
	if (hDC != NULL) {
		ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
		::RestoreDC(hDC, -1);
	}
}

} // namespace ui
