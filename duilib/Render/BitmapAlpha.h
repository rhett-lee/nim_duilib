#ifndef UI_RENDER_BITMAP_ALPHA_H_
#define UI_RENDER_BITMAP_ALPHA_H_

#pragma once

#include "duilib/Core/UiRect.h"

namespace ui
{

/** 对位图数据的Alpha值进行保存和恢复
*/
class BitmapAlpha
{
public:
	BitmapAlpha(uint8_t* pPiexl, int32_t nWidth, int32_t nHeight, int32_t nChannels);
	~BitmapAlpha();

public:
	void ClearAlpha(const UiRect& rcDirty, uint8_t alpha) const;
	void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) const;
	void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding) const;

private:
	/** 图像数据
	*/
	uint8_t* m_pPiexl;

	/** 图像宽度
	*/
	int32_t m_nWidth;

	/** 图像高度
	*/
	int32_t m_nHeight;

	/** 通道数，目前只支持4（ARGB）
	*/
	int32_t m_nChannels;
};
} // namespace ui

#endif // UI_RENDER_BITMAP_ALPHA_H_