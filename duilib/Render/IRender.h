#ifndef UI_RENDER_IRENDER_H_
#define UI_RENDER_IRENDER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "duilib/Core/Define.h"
#include "duilib/Render/UiColor.h"
#include "base/callback/callback.h"

namespace ui 
{

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

	/** 该位图是否有透明属性(即透明通道中，含有不是255的数据)
	*/
	virtual bool IsAlphaBitmap() const = 0;

	/** 克隆生成新的的位图
	*@return 返回新生成的位图接口，由调用方释放资源
	*/
	virtual IBitmap* Clone() = 0;
};

class UILIB_API IPen : public virtual nbase::SupportWeakCallback
{
public:

	virtual IPen* Clone() const = 0;

	virtual void SetWidth(int width) = 0;
	virtual int GetWidth() const = 0;
	virtual void SetColor(UiColor color) = 0;
	virtual UiColor GetColor() const = 0;

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
	virtual LineCap GetStartCap() const = 0;
	virtual LineCap GetEndCap() const = 0;
	virtual LineCap GetDashCap() const = 0;

	enum LineJoin
	{
		LineJoinMiter = 0,
		LineJoinBevel = 1,
		LineJoinRound = 2,
		LineJoinMiterClipped = 3
	};
	virtual void SetLineJoin(LineJoin join) = 0;
	virtual LineJoin GetLineJoin() const = 0;

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
	virtual DashStyle GetDashStyle() const = 0;
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

/** 光栅操作代码
*/
enum class UILIB_API RopMode
{
	kSrcCopy,	//对应于 SRCCOPY
	kDstInvert, //对应于 DSTINVERT
	kSrcInvert, //对应于 SRCINVERT
	kSrcAnd     //对应于 SRCAND
};

/** 绘制文本时的格式
*/
enum UILIB_API DrawStringFormat
{
	TEXT_SINGLELINE		= DT_SINGLELINE,
	TEXT_LEFT			= DT_LEFT,
	TEXT_CENTER			= DT_CENTER,
	TEXT_RIGHT			= DT_RIGHT,
	TEXT_TOP			= DT_TOP,
	TEXT_VCENTER		= DT_VCENTER,
	TEXT_BOTTOM			= DT_BOTTOM,
	TEXT_END_ELLIPSIS	= DT_END_ELLIPSIS,
	TEXT_PATH_ELLIPSIS  = DT_PATH_ELLIPSIS,
	TEXT_NOCLIP			= DT_NOCLIP,
};

class UILIB_API IRender : public virtual nbase::SupportWeakCallback
{
public:
	/** 获取画布宽度
	*/
	virtual int	GetWidth() = 0;

	/** 获取画布高度
	*/
	virtual int GetHeight() = 0;

	/** 调整画布大小
	*/
	virtual bool Resize(int width, int height) = 0;

	/** 判断画布是否支持透明
	*/
	virtual bool IsRenderTransparent() const = 0;

	/** 设置画布是否支持透明
	*/
	virtual bool SetRenderTransparent(bool bTransparent) = 0;

	/** 设置窗口视区原点坐标偏移，
	 *  将原视区原点x值增加ptOffset.x后，作为新的视区原点x;
	 *  将原视区原点y值增加ptOffset.y后，作为新的视区原点y;
	 * @param [in] ptOffset 视区原点坐标偏移量
	 *@return 返回原来的视区原点坐标(x,y)
	 */
	virtual UiPoint OffsetWindowOrg(UiPoint ptOffset) = 0;

	/** 将点(pt.x, pt.y)映射到视区原点 (0, 0)
	 *@return 返回原来的视区原点坐标(x,y)
	 */
	virtual UiPoint SetWindowOrg(UiPoint pt) = 0;

	/** 获取视区原点坐标(x,y)
	 * @return 返回当前的视区原点坐标(x,y)
	 */
	virtual UiPoint GetWindowOrg() const = 0;
	
	/** 保存指定设备上下文的当前状态
	* @param [out] 返回保存的设备上下文标志，在RestoreClip的时候，作为参数传入
	*/
	virtual void SaveClip(int& nState) = 0;

	/** 将设备上下文还原到最近一次保存的状态
	* @param [in] 保存的设备上下文标志（由SaveClip返回）
	*/
	virtual void RestoreClip(int nState) = 0;

	/** 设置矩形剪辑区域，并保存当前设备上下文的状态
	* @param [in] rc剪辑区域
	*/
	virtual void SetClip(const UiRect& rc) = 0;

	/** 设置圆角矩形剪辑区域，并保存当前设备上下文的状态
	* @param [in] rcItem 剪辑区域
	* @param [in] width 圆角的宽度
	* @param [in] height 圆角的的度
	*/
	virtual void SetRoundClip(const UiRect& rcItem, int width, int height) = 0;

	/** 清除矩形剪辑区域，并恢复设备上下文到最近一次保存的状态
	*/
	virtual void ClearClip() = 0;

	/** 函数执行与从指定源设备上下文到目标设备上下文中的像素矩形对应的颜色数据的位块传输
	* @param [in] x 目标矩形左上角的 x 坐标
	* @param [in] y 目标矩形左上角的 y 坐标
	* @param [in] cx 源矩形和目标矩形的宽度
	* @param [in] cy 源和目标矩形的高度
	* @param [in] pSrcBitmap 源图片接口
	* @param [in] pSrcRender 源Render对象
	* @param [in] xSrc 源矩形左上角的 x 坐标
	* @param [in] ySrc 源矩形左上角的 y 坐标
	* @param [in] rop 光栅操作代码
	*/
	virtual bool BitBlt(int x, int y, int cx, int cy, 
		                IBitmap* pSrcBitmap, int xSrc, int ySrc,
		                RopMode rop) = 0;
	virtual bool BitBlt(int x, int y, int cx, int cy, 
		                IRender* pSrcRender, int xSrc, int ySrc,
		                RopMode rop) = 0;

	/** 函数将一个位图从源矩形复制到目标矩形中，并拉伸或压缩位图以适应目标矩形的尺寸（如有必要）。 
	    系统根据当前在目标设备上下文中设置的拉伸模式拉伸或压缩位图。
	* @param [in] xDest 目标矩形左上角的 x 坐标
	* @param [in] yDest 目标矩形左上角的 y 坐标
	* @param [in] widthDest 目标矩形的宽度
	* @param [in] heightDest 目标矩形的高度
	* @param [in] pSrcRender 源Render对象
	* @param [in] xSrc 源矩形左上角的 x 坐标
	* @param [in] ySrc 源矩形左上角的 y 坐标
	* @param [in] widthSrc 源矩形的宽度
	* @param [in] heightSrc 源矩形的高度
	* @param [in] rop 光栅操作代码
	*/
	virtual bool StretchBlt(int xDest, int yDest, int widthDest, int heightDest,
		                    IRender* pSrcRender, int xSrc, int ySrc, int widthSrc, int heightSrc,
		                    RopMode rop) = 0;


	/** 显示具有透明或半透明像素的位图，如果源矩形和目标矩形的大小不相同，则会拉伸源位图以匹配目标矩形。
	* @param [in] xDest 目标矩形左上角的 x 坐标
	* @param [in] yDest 目标矩形左上角的 y 坐标
	* @param [in] widthDest 目标矩形的宽度
	* @param [in] heightDest 目标矩形的高度
	* @param [in] pSrcRender 源Render对象
	* @param [in] xSrc 源矩形左上角的 x 坐标
	* @param [in] ySrc 源矩形左上角的 y 坐标
	* @param [in] widthSrc 源矩形的宽度
	* @param [in] heightSrc 源矩形的高度
	* @param [in] alpha 透明度 alpha 值（0 - 255）
	*/
	virtual bool AlphaBlend(int xDest, int yDest, int widthDest, int heightDest,
		                    IRender* pSrcRender, int xSrc, int ySrc, int widthSrc, int heightSrc,
		                    uint8_t alpha = 255) = 0;

	/** 绘制图片（采用九宫格方式绘制图片）
	* @param [in] rcPaint 当前全部可绘制区域（用于避免非可绘制区域的绘制，以提高绘制性能）
	* @param [in] pBitmap 用于绘制的位图接口
	* @param [in] rcImageDest 绘制的目标区域
	* @param [in] rcImageSource 绘制的源图片区域
	* @param [in] rcImageCorners 绘制源图片的边角信息，用于九宫格绘制
	* @param [in] bBitmapDpiScaled 位图尺寸是否已经做过DPI适应
	* @param [in] uFade 透明度（0 - 255）
	* @param [in] xtiled 横向平铺
	* @param [in] ytiled 纵向平铺
	* @param [in] fullxtiled 如果为true，横向平铺绘制时，确保是完整绘制图片，该参数仅当xtiled为true时有效
	* @param [in] fullytiled 如果为true，纵向平铺绘制时，确保是完整绘制图片，该参数仅当ytiled为true时有效
	* @param [in] nTiledMargin 平铺绘制时，图片的横向、纵向间隔，该参数仅当xtiled为true或者ytiled为true时有效
	*/
	virtual void DrawImage(const UiRect& rcPaint, 
						   IBitmap* pBitmap, 
						   const UiRect& rcImageDest, 
						   UiRect rcImageSource, 
						   UiRect rcImageCorners,
						   bool bBitmapDpiScaled = false,
						   uint8_t uFade = 255,
						   bool xtiled = false, 
						   bool ytiled = false, 
						   bool fullxtiled = true, 
						   bool fullytiled = true, 
						   int nTiledMargin = 0) = 0;

	/** 绘制直线
	* @param [in] pt1 起始点坐标
	* @param [in] pt2 终止点坐标
	* @param [in] penColor 画笔的颜色值
	* @param [in] nWidth 画笔的宽度
	*/
	virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int nWidth) = 0;

	/** 绘制矩形
	* @param [in] rc 矩形区域
	* @param [in] penColor 画笔的颜色值
	* @param [in] nWidth 画笔的宽度
	*/
	virtual void DrawRect(const UiRect& rc, UiColor penColor, int nWidth) = 0;

	/** 用颜色填充矩形
	* @param [in] rc 目标矩形区域
	* @param [in] dwColor 颜色值
	* @param [in] uFade 透明度（0 - 255）
	*/
	virtual void FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade = 255) = 0;

	/** 绘制圆角矩形
	* @param [in] rc 矩形区域
	* @param [in] roundSize 圆角的宽和高
	* @param [in] penColor 画笔的颜色值
	* @param [in] nWidth 画笔的宽度
	*/
	virtual void DrawRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor penColor, int nWidth) = 0;

	/** 用颜色填充圆角矩形
	* @param [in] rc 矩形区域
	* @param [in] roundSize 圆角的宽和高
	* @param [in] dwColor 颜色值
	* @param [in] uFade 透明度（0 - 255）
	*/
	virtual void FillRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor dwColor, uint8_t uFade = 255) = 0;

	virtual void DrawPath(const IPath* path, const IPen* pen) = 0;
	virtual void FillPath(const IPath* path, const IBrush* brush) = 0;

	/** 计算指定文本字符串的宽度和高度
	* @param [in] strText 文字内容
	* @param [in] strFontId 文字的字体ID，字体属性在全局配置中设置
	* @param [in] uFormat 文字的格式，参见 enum DrawStringFormat 类型定义
	* @param [in] width 当前区域的限制宽度
	* @return 返回文本字符串的宽度和高度，以矩形表示结果
	*/
	virtual UiRect MeasureString(const std::wstring& strText, 
		                         const std::wstring& strFontId, 
		                         uint32_t uFormat,
		                         int width = DUI_NOSET_VALUE) = 0;
	/** 绘制文字
	* @param [in] 矩形区域
	* @param [in] strText 文字内容
	* @param [in] dwTextColor 文字颜色值
	* @param [in] strFontId 文字的字体ID，字体属性在全局配置中设置
	* @param [in] uFormat 文字的格式，参见 enum DrawStringFormat 类型定义
	* @param [in] uFade 透明度（0 - 255）
	*/
	virtual void DrawString(const UiRect& rc,
		                    const std::wstring& strText,
		                    UiColor dwTextColor,
		                    const std::wstring& strFontId, 
		                    uint32_t uFormat,
		                    uint8_t uFade = 255) = 0;

	/** 在指定矩形周围绘制阴影（高斯模糊）
	* @param [in] rc 矩形区域
	* @param [in] roundSize 阴影的圆角宽度和高度
	* @param [in] cpOffset 设置阴影偏移量（offset-x 和 offset-y）
	*                      <offset-x> 设置水平偏移量，如果是负值则阴影位于矩形左边。 
	*                      <offset-y> 设置垂直偏移量，如果是负值则阴影位于矩形上面。
	* @param [in] nBlurRadius 模糊半径，半径必须介于 0 到 255 的范围内。
	*                         值越大，模糊面积越大，阴影就越大越淡。 不能为负值。默认为0，此时阴影边缘锐利。
	* @param [in] nSpreadRadius 扩展半径，即模糊区域距离rc矩形边缘多少个像素。
	*                         取正值时，阴影扩大；取负值时，阴影收缩。默认为0，此时阴影与元素同样大。
	* @param [in] dwColor 阴影的颜色值
	* @param [in] bExclude 当为true的时候表示阴影在矩形边框外面，为false的时候表示阴影在边框矩形内部
	*/
	virtual void DrawBoxShadow(const UiRect& rc, 
		                       const UiSize& roundSize, 
		                       const UiPoint& cpOffset, 
		                       int nBlurRadius, 
		                       int nSpreadRadius,
		                       UiColor dwColor, 
		                       bool bExclude) = 0;


	/** 分离位图
	*@return 返回位图接口，返回后由调用方管理资源（包括释放资源等）
	*/
	virtual IBitmap* DetachBitmap() = 0;

	virtual void ClearAlpha(const UiRect& rcDirty, int alpha = 0) = 0;
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, int alpha) = 0;
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding = UiRect()) = 0;

#ifdef UILIB_IMPL_WINSDK
	/** 获取DC句柄，当不使用后，需要调用ReleaseDC接口释放资源
	*/
	virtual HDC GetDC() = 0;

	/** 释放DC资源
	* @param [in] hdc 需要释放的DC句柄
	*/
	virtual void ReleaseDC(HDC hdc) = 0;
#endif

public:

	/** 清除位图数据，使其变成全透明位图（位图数据全部清零）
	*/
	virtual void Clear() = 0;

	/** 克隆一个新的对象
	*/
	virtual std::unique_ptr<IRender> Clone() = 0;
};

class UILIB_API IRenderFactory
{
public:
	virtual ~IRenderFactory() = default;
	virtual IPen* CreatePen(UiColor color, int width = 1) = 0;
	virtual IBrush* CreateBrush(UiColor corlor) = 0;
	virtual IPath* CreatePath() = 0;
	virtual IBitmap* CreateBitmap() = 0;
	virtual IRender* CreateRender() = 0;
};

} // namespace ui

#endif // UI_RENDER_IRENDER_H_
