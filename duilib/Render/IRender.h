#ifndef UI_RENDER_IRENDER_H_
#define UI_RENDER_IRENDER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "duilib/Core/Define.h"
#include "duilib/Render/UiColor.h"
#include "base/callback/callback.h"

namespace ui 
{

class UILIB_API IClip : public virtual nbase::SupportWeakCallback
{
public:
	virtual void CreateClip(HDC hDC, UiRect rc) = 0;
	virtual void CreateRoundClip(HDC hDC, UiRect rc, int width, int height) = 0;
	virtual void ClearClip(HDC hDC) = 0;
};

class IRender;
class UILIB_API AutoClip
{
public:
	AutoClip(IRender* pRender, const UiRect& rc, bool bClip = true);
	AutoClip(IRender* pRender, const UiRect& rcRound, int width, int height, bool bClip = true);
	~AutoClip();

private:
	IRender *m_pRender;
	bool			m_bClip;
};

class UILIB_API IBitmap : public virtual nbase::SupportWeakCallback
{
public:
	/** 从数据初始化（ARGB格式）
	@param [in] nWidth 宽度
	@param [in] nHeight 高度
	@param [in] flipHeight 是否翻转位图，如果为true，创建位图的时候，以左上角为圆点，图像方向是从上到下的；
	                       如果为false，则以左下角为圆点，图像方向是从下到上。
	@param [in] pPixelBits 位图数据, 如果为nullptr表示窗口空位图，如果不为nullptr，其数据长度为：nWidth*4*nHeight
	*/
	virtual bool Init(uint32_t nWidth, uint32_t nHeight, bool flipHeight, const void* pPixelBits) = 0;

	/** 获取图片宽度
	*/
	virtual uint32_t GetWidth() const = 0;

	/** 获取图片高度
	*/
	virtual uint32_t GetHeight() const = 0;

	/** 获取图片大小
	@return 图片大小
	*/
	virtual UiSize GetSize() const = 0;

	/** 锁定位图数据，数据长度 = GetWidth() * GetHeight() * 4
	*/
	virtual void* LockPixelBits() = 0;

	/** 释放位图数据
	*/
	virtual void UnLockPixelBits() = 0;

	/** 克隆生成新的的位图
	*@return 返回新生成的位图接口，由调用方释放资源
	*/
	virtual IBitmap* Clone() = 0;
};

class UILIB_API IPen : public virtual nbase::SupportWeakCallback
{
public:
	IPen(UiColor color, int width = 1) : color_(color) { (void)width; };
	IPen(const IPen& r) : color_(r.color_) {};
	IPen& operator=(const IPen& r) = delete;

	virtual IPen* Clone() = 0;

	virtual void SetWidth(int width) = 0;
	virtual int GetWidth() = 0;
	virtual void SetColor(UiColor color) = 0;
	virtual UiColor GetColor() { return color_; };

	enum LineCap
	{
		LineCapFlat = 0,
		LineCapSquare = 1,
		LineCapRound = 2,
		LineCapTriangle = 3
	};
	virtual void SetStartCap(LineCap cap) = 0;
	virtual void SetEndCap(LineCap cap) = 0;
	virtual void SetDashCap(LineCap cap) = 0;
	virtual LineCap GetStartCap() = 0;
	virtual LineCap GetEndCap() = 0;
	virtual LineCap GetDashCap() = 0;

	enum LineJoin
	{
		LineJoinMiter = 0,
		LineJoinBevel = 1,
		LineJoinRound = 2,
		LineJoinMiterClipped = 3
	};
	virtual void SetLineJoin(LineJoin join) = 0;
	virtual LineJoin GetLineJoin() = 0;

	enum DashStyle
	{
		DashStyleSolid,          // 0
		DashStyleDash,           // 1
		DashStyleDot,            // 2
		DashStyleDashDot,        // 3
		DashStyleDashDotDot,     // 4
		DashStyleCustom          // 5
	};
	virtual void SetDashStyle(DashStyle style) = 0;
	virtual DashStyle GetDashStyle() = 0;

protected:
	UiColor color_;
};

class UILIB_API IBrush : public virtual nbase::SupportWeakCallback
{
public:
	IBrush(UiColor color) : color_(color){};
	IBrush(const IBrush& r)	: color_(r.color_)	{};
	IBrush& operator=(const IBrush& r) = delete;

	virtual IBrush* Clone() = 0;
	virtual UiColor GetColor() { return color_; };
protected:
	UiColor color_;
};

class UILIB_API IPath : public virtual nbase::SupportWeakCallback
{
public:
	IPath(){};
	IPath(const IPath& r) { (void)r; };
	IPath& operator=(const IPath& r) = delete;

	virtual IPath* Clone() = 0;
	virtual void Reset() = 0;

	enum FillMode
	{
		FillModeAlternate,        // 0
		FillModeWinding           // 1
	};
	virtual void SetFillMode(FillMode mode) = 0;
	virtual FillMode GetFillMode() = 0;

	virtual void StartFigure() = 0;
	virtual void CloseFigure() = 0;

	virtual void AddLine(int x1, int y1, int x2, int y2) = 0;
	virtual void AddLines(const UiPoint* points, int count) = 0;
	virtual void AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) = 0;
	virtual void AddCurve(const UiPoint* points, int count) = 0;
	virtual void AddRect(int left, int top, int right, int bottom) = 0;
	virtual void AddRect(const UiRect& rect) = 0;
	virtual void AddEllipse(int left, int top, int right, int bottom) = 0;
	virtual void AddEllipse(const UiRect& rect) = 0;
	virtual void AddArc(int x, int y, int width, int height, float startAngle, float sweepAngle) = 0;
	virtual void AddPie(int x, int y, int width, int height, float startAngle, float sweepAngle) = 0;
	virtual void AddPolygon(const UiPoint* points, int count) = 0;

	virtual UiRect GetBound(const IPen* pen) = 0;
	virtual bool IsContainsPoint(int x, int y) = 0;
	virtual bool IsStrokeContainsPoint(int x, int y, const IPen* pen) = 0;
};

class UILIB_API IRender : public virtual nbase::SupportWeakCallback
{
public:
	virtual HDC GetDC() = 0;
	virtual bool Resize(int width, int height) = 0;
	virtual void Clear() = 0;
	virtual std::unique_ptr<IRender> Clone() = 0;

	/** 分离位图
	*@return 返回位图接口，返回后由调用方管理资源（包括释放资源等）
	*/
	virtual IBitmap* DetachBitmap() = 0;

	virtual int	GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual void ClearAlpha(const UiRect& rcDirty, int alpha = 0) = 0;
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, int alpha) = 0;
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding = UiRect()) = 0;

	virtual bool IsRenderTransparent() const = 0;
	virtual bool SetRenderTransparent(bool bTransparent) = 0;

	virtual void Save() = 0;
	virtual void Restore() = 0;
	virtual UiPoint OffsetWindowOrg(UiPoint ptOffset) = 0;
	virtual UiPoint SetWindowOrg(UiPoint ptOffset) = 0;
	virtual UiPoint GetWindowOrg() const = 0;

	virtual void SetClip(const UiRect& rc) = 0;
	virtual void SetRoundClip(const UiRect& rcItem, int width, int height) = 0;
	virtual void ClearClip() = 0;

	virtual HRESULT BitBlt(int x, int y, int cx, int cy, HDC hdcSrc, int xSrc = 0, int yScr = 0, DWORD rop = SRCCOPY) = 0;
	virtual bool StretchBlt(int xDest, int yDest, int widthDest, int heightDest, HDC hdcSrc, int xSrc, int yScr, int widthSrc, int heightSrc, DWORD rop = SRCCOPY) = 0;
	virtual bool AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, HDC hdcSrc, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade = 255) = 0;

	/** 绘制图片（采用九宫格方式绘制图片）
	* @param [in] rcPaint 当前全部可绘制区域（用于避免非可绘制区域的绘制，以提高绘制性能）
	* @param [in] pBitmap 用于绘制的位图接口
	* @param [in] bAlphaChannel 图片是否含有Alpha通道
	* @param [in] rcImageDest 绘制的目标区域
	* @param [in] rcImageSource 绘制的源图片区域
	* @param [in] rcImageCorners 绘制源图片的边角信息，用于九宫格绘制
	* @param [in] bBitmapDpiScale 位图尺寸是否已经做过DPI适应
	* @param [in] uFade 透明度（0 - 255）
	* @param [in] xtiled 横向平铺
	* @param [in] ytiled 纵向平铺
	* @param [in] fullxtiled 如果为true，横向平铺绘制时，确保是完整绘制图片，该参数仅当xtiled为true时有效
	* @param [in] fullytiled 如果为true，纵向平铺绘制时，确保是完整绘制图片，该参数仅当ytiled为true时有效
	* @param [in] nTiledMargin 平铺绘制时，图片的横向、纵向间隔，该参数仅当xtiled为true或者ytiled为true时有效
	*/
	virtual void DrawImage(const UiRect& rcPaint, 
						   IBitmap* pBitmap, 
						   bool bAlphaChannel,
						   const UiRect& rcImageDest, 
						   const UiRect& rcImageSource, 
						   UiRect rcImageCorners,
						   bool bBitmapDpiScale = false,
						   uint8_t uFade = 255,
						   bool xtiled = false, 
						   bool ytiled = false, 
						   bool fullxtiled = true, 
						   bool fullytiled = true, 
						   int nTiledMargin = 0) = 0;

	virtual void DrawColor(const UiRect& rc, UiColor dwColor, BYTE uFade = 255) = 0;
	virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int nWidth) = 0;
	virtual void DrawRect(const UiRect& rc, UiColor penColor, int nWidth) = 0;
	virtual void DrawRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor penColor, int nWidth) = 0;
	virtual void DrawPath(const IPath* path, const IPen* pen) = 0;
	virtual void FillPath(const IPath* path, const IBrush* brush) = 0;

	virtual void DrawText(const UiRect& rc, const std::wstring& strText, UiColor dwTextColor, const std::wstring& strFontId, UINT uStyle, BYTE uFade = 255, bool bLineLimit = false, bool bFillPath = false) = 0;
	virtual UiRect MeasureText(const std::wstring& strText, const std::wstring& strFontId, UINT uStyle, int width = DUI_NOSET_VALUE) = 0;

	virtual void DrawBoxShadow(const UiRect& rc, const UiSize& roundSize, const UiPoint& cpOffset, int nBlurRadius, int nBlurSize, int nSpreadSize, UiColor dwColor, bool bExclude) = 0;
};

class UILIB_API IRenderFactory
{
public:
	virtual ~IRenderFactory() = default;
	virtual ui::IPen* CreatePen(UiColor color, int width = 1) = 0;
	virtual ui::IBrush* CreateBrush(UiColor corlor) = 0;
	virtual ui::IPath* CreatePath() = 0;
	virtual ui::IBitmap* CreateBitmap() = 0;
	virtual ui::IRender* CreateRender() = 0;
};

} // namespace ui

#endif // UI_RENDER_IRENDER_H_
