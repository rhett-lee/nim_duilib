// Copyright (c) 2010-2011, duilib develop team(www.duilib.com).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or 
// without modification, are permitted provided that the 
// following conditions are met.
//
// Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above 
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials 
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef DUILIB_TYPES_H_
#define DUILIB_TYPES_H_ 1

#pragma once

#include "duilib_config.h"
#include <string>

namespace ui
{
	class UILIB_API UiPoint : public tagPOINT
	{
	public:
		UiPoint()
		{
			x = y = 0;
		}

		explicit UiPoint(const POINT& src)
		{
			x = src.x;
			y = src.y;
		}

		UiPoint(int _x, int _y)
		{
			x = _x;
			y = _y;
		}

		void Offset(int offsetX, int offsetY)
		{
			x += offsetX;
			y += offsetY;
		}

		void Offset(UiPoint offsetPoint)
		{
			x += offsetPoint.x;
			y += offsetPoint.y;
		}

		bool operator==(const UiPoint& dst) {
			return x == dst.x && y == dst.y;
		}

		bool operator!=(const UiPoint& dst) {
			return x != dst.x || y != dst.y;
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API UiSize : public tagSIZE
	{
	public:
		UiSize()
		{
			cx = cy = 0;
		}

		UiSize(const UiSize& src)
		{
			cx = src.cx;
			cy = src.cy;
		}

		UiSize(int _cx, int _cy)
		{
			cx = _cx;
			cy = _cy;
		}

		void Offset(int offsetCX, int offsetCY)
		{
			cx += offsetCX;
			cy += offsetCY;
		}

		void Offset(UiSize offsetPoint)
		{
			cx += offsetPoint.cx;
			cy += offsetPoint.cy;
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API UiRect : public tagRECT
	{
	public:
		UiRect()
		{
			left = top = right = bottom = 0;
		}

		explicit UiRect(const RECT& src)
		{
			left = src.left;
			top = src.top;
			right = src.right;
			bottom = src.bottom;
		}

		UiRect(int iLeft, int iTop, int iRight, int iBottom)
		{
			left = iLeft;
			top = iTop;
			right = iRight;
			bottom = iBottom;
		}

		int GetWidth() const
		{
			return right - left;
		}

		int GetHeight() const
		{
			return bottom - top;
		}

		int CenterX() const
		{
			return (left + right) / 2;
		}

		int CenterY() const
		{
			return (top + bottom) / 2;
		}

		UiPoint Center() const
		{
			return { CenterX(), CenterY() };
		}

		void Clear()
		{
			left = top = right = bottom = 0;
		}

		bool IsRectEmpty() const
		{
			return ::IsRectEmpty(this) == TRUE;
		}

		void ResetOffset()
		{
			::OffsetRect(this, -left, -top);
		}

		void Normalize()
		{
			if (left > right) { int iTemp = left; left = right; right = iTemp; }
			if (top > bottom) { int iTemp = top; top = bottom; bottom = iTemp; }
		}

		void Offset(int cx, int cy)
		{
			::OffsetRect(this, cx, cy);
		}

		void Offset(const UiPoint& offset)
		{
			::OffsetRect(this, offset.x, offset.y);
		}

		void Inflate(int cx, int cy)
		{
			::InflateRect(this, cx, cy);
		}

		void Inflate(const UiRect& rect)
		{
			this->left -= rect.left;
			this->top -= rect.top;
			this->right += rect.right;
			this->bottom += rect.bottom;
		}

		void Deflate(int cx, int cy)
		{
			::InflateRect(this, -cx, -cy);
		}

		void Deflate(const UiRect& rect)
		{
			this->left += rect.left;
			this->top += rect.top;
			this->right -= rect.right;
			this->bottom -= rect.bottom;
		}

		void Union(const UiRect& rc)
		{
			::UnionRect(this, this, &rc);
		}

		void Intersect(const UiRect& rc)
		{
			::IntersectRect(this, this, &rc);
		}

		void Subtract(const UiRect& rc)
		{
			::SubtractRect(this, this, &rc);
		}

		bool IsPointIn(const UiPoint& point) const
		{
			return ::PtInRect(this, point) == TRUE;
		}

		bool Equal(const UiRect& rect) const
		{
			return this->left == rect.left && this->top == rect.top
				&& this->right == rect.right && this->bottom == rect.bottom;
		}
	};

	/** 字体接口的基本数据结构
*/
	class UILIB_API UiFont
	{
	public:
		UiFont() :
			m_fontSize(0),
			m_bBold(false),
			m_bUnderline(false),
			m_bItalic(false),
			m_bStrikeOut(false)
		{}

		/** 字体名称
		*/
		std::wstring m_fontName;

		/** 字体大小（单位：像素）
		*/
		int32_t m_fontSize;

		/** 是否为粗体
		 */
		bool m_bBold;

		/** 字体下划线状态
		 */
		bool m_bUnderline;

		/** 字体的斜体状态
		 */
		bool m_bItalic;

		/** 字体的删除线状态
		 */
		bool m_bStrikeOut;
	};
}// namespace ui

#endif //DUILIB_TYPES_H_


