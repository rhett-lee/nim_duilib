#ifndef UI_RENDER_IRENDER_H_
#define UI_RENDER_IRENDER_H_

#pragma once

#include "base/callback/callback.h"
#include "duilib/Core/UiTypes.h"

namespace ui 
{
/** 字体接口
*/
class UILIB_API IFont: public virtual nbase::SupportWeakCallback
{
public:
	/** 初始化字体(内部未对字体大小做DPI自适应)
 	*/
	virtual bool InitFont(const UiFont& fontInfo) = 0;

	/** 获取字体名
	*/
	virtual const wchar_t* FontName() const = 0;

	/** 获取字体大小(字体高度)
	*/
	virtual int FontSize() const = 0;

	/** 是否为粗体
	*/
	virtual bool IsBold() const = 0;

	/** 字体下划线状态
	*/
	virtual bool IsUnderline() const = 0;

	/** 字体的斜体状态
	*/
	virtual bool IsItalic() const = 0;

	/** 字体的删除线状态
	*/
	virtual bool IsStrikeOut() const = 0;
};

/** Skia引擎需要传入Alpha类型
*/
enum UILIB_API BitmapAlphaType: int
{
	kUnknown_SkAlphaType,	//!< uninitialized
	kOpaque_SkAlphaType,	//!< pixel is opaque
	kPremul_SkAlphaType,	//!< pixel components are premultiplied by alpha
	kUnpremul_SkAlphaType	//!< pixel components are independent of alpha
};

/** 位图接口
*/
class UILIB_API IBitmap : public virtual nbase::SupportWeakCallback
{
public:
	/** 从数据初始化（ARGB格式）
	@param [in] nWidth 宽度
	@param [in] nHeight 高度
	@param [in] flipHeight 是否翻转位图，如果为true，创建位图的时候，以左上角为圆点，图像方向是从上到下的；
	                       如果为false，则以左下角为圆点，图像方向是从下到上。
	@param [in] pPixelBits 位图数据, 如果为nullptr表示窗口空位图，如果不为nullptr，其数据长度为：nWidth*4*nHeight
	@param [in] alphaType 位图的Alpha类型，只有Skia引擎需要此参数
	*/
	virtual bool Init(uint32_t nWidth, uint32_t nHeight, bool flipHeight, 
					  const void* pPixelBits, BitmapAlphaType alphaType = kPremul_SkAlphaType) = 0;

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

/** 画笔接口
*/
class UILIB_API IPen : public virtual nbase::SupportWeakCallback
{
public:
	/** 设置画笔宽度
	*/
	virtual void SetWidth(int width) = 0;

	/** 获取画笔宽度
	*/
	virtual int GetWidth() const = 0;

	/** 设置画笔颜色
	*/
	virtual void SetColor(UiColor color) = 0;

	/** 获取画笔颜色
	*/
	virtual UiColor GetColor() const = 0;

	/** 笔帽样式
	*/
	enum LineCap
	{
		kButt_Cap   = 0,	//平笔帽（默认）
		kRound_Cap  = 1,	//圆笔帽
		kSquare_Cap = 2		//方笔帽
	};

	/** 设置线段起始的笔帽样式
	*/
	virtual void SetStartCap(LineCap cap) = 0;

	/** 获取线段起始的笔帽样式
	*/
	virtual LineCap GetStartCap() const = 0;

	/** 设置线段结束的笔帽样式
	*/
	virtual void SetEndCap(LineCap cap) = 0;

	/** 获取线段结束的笔帽样式
	*/
	virtual LineCap GetEndCap() const = 0;

	/** 设置短划线笔帽的样式
	*/
	virtual void SetDashCap(LineCap cap) = 0;

	/** 获取短划线笔帽的样式
	*/
	virtual LineCap GetDashCap() const = 0;

	/** 线段末尾使用的联接样式，该线段与另一个线段相遇
	*/
	enum LineJoin
	{
		kMiter_Join = 0,	//尖角（默认）
		kBevel_Join = 1,	//平角
		kRound_Join = 2		//圆角		
	};

	/** 设置线段末尾使用的联接样式
	*/
	virtual void SetLineJoin(LineJoin join) = 0;

	/** 获取线段末尾使用的联接样式
	*/
	virtual LineJoin GetLineJoin() const = 0;

	/** 笔绘制的线条的线条样式
	*/
	enum DashStyle
	{
		kDashStyleSolid			= 0,	//实线（默认）
		kDashStyleDash			= 1,	//虚线
		kDashStyleDot			= 2,	//虚线
		kDashStyleDashDot		= 3,	//交替虚线
		kDashStyleDashDotDot	= 4		//交替短划线点点线
	};

	/** 设置绘制的线条样式
	*/
	virtual void SetDashStyle(DashStyle style) = 0;

	/** 获取绘制的线条样式
	*/
	virtual DashStyle GetDashStyle() const = 0;

	/** 复制Pen对象
	*/
	virtual IPen* Clone() const = 0;
};

/** 画刷接口
*/
//目前只支持一个SolidBrush
class UILIB_API IBrush : public virtual nbase::SupportWeakCallback
{
public:
	virtual IBrush* Clone() = 0;
	virtual UiColor GetColor() const = 0;
};

/** 路径接口
*/
class IMatrix;
class UILIB_API IPath : public virtual nbase::SupportWeakCallback
{
public:	
	/** 填充类型，在路径或曲线相交时如何填充形成的区域
	*/
	enum class FillType 
	{
		/** Specifies that "inside" is computed by an odd number of edge crossings
		*/
		kEvenOdd		= 0, //FillModeAlternate

		/** Specifies that "inside" is computed by a non-zero sum of signed edge crossings 
		*/
		kWinding		= 1, //FillModeWinding

		/** Same as EvenOdd, but draws outside of the path, rather than inside 
		*/
		kInverseEvenOdd	= 2,

		/** Same as Winding, but draws outside of the path, rather than inside 
		*/
		kInverseWinding	= 3
	};

	/** 设置填充类型
	*/
	virtual void SetFillType(FillType mode) = 0;

	/** 获取填充类型
	*/
	virtual FillType GetFillType() = 0;

	/** 将一行添加到此路径的当前图中
	* @param [in] x1 线条起点的 x 坐标
	* @param [in] y1 线条起点的 y 坐标
	* @param [in] x2 线条终点的 x 坐标
	* @param [in] y2 线条终点的 y 坐标
	*/
	virtual void AddLine(int x1, int y1, int x2, int y2) = 0;

	/** 向此路径的当前图添加一系列连接线
	* @param [in] points 线条起点和终点的点数组, 数组中的第一个点是第一行的起点，
	                     数组中的最后一个点是最后一行的终点。 
						 其他每个点都用作一行的终点，下一行的起点。
	* @param [in] count 点数组中的元素数
	*/
	virtual void AddLines(const UiPoint* points, int count) = 0;

	/** 将贝塞尔(Bézier)曲线样条添加到此路径的当前图中
	*    贝塞尔自由绘制曲线是一条由四个点指定的曲线：
	*    两个端点（p1[x1,y1] 和 p2[x4,y4]）和两个控制点（c1[x2,y2] 和 c2[x3,y3]）。 
	*	 曲线从 p1 开始，以 p2 结尾。 
	*	 曲线不通过控制点，但控制点充当磁铁，将曲线拉向某个方向，并影响曲线的弯曲方式。
	* @param [in] x1 起点的 x 坐标
	* @param [in] y1 起点的 y 坐标
	* @param [in] x2 第一个控制点的 x 坐标
	* @param [in] y2 第一个控制点的 y 坐标
	* @param [in] x3 第二个控制点的 x 坐标
	* @param [in] y3 第二个控制点的 y 坐标
	* @param [in] x4 终点的 x 坐标
	* @param [in] y4 终点的 y 坐标
	*
	*/
	virtual void AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) = 0;

	/** 将连接的 Bézier 样条序列添加到此路径的当前图中
	* @param [in] points 指向连接的样条的起始点、结束点和控制点数组的指针。 
	*                   第一个样条从第一个点到数组的第四个点构造，并使用第二和第三个点作为控制点。 
	*					序列中的每个后续样条只需要另外三个点：
	*					前一个样条的终点用作起点，序列中的下两个点是控制点，第三个点是终点。
	* @param [in] count 数组中的元素数
	*/
	virtual void AddBeziers(const UiPoint* points, int count) = 0;

	/** 将矩形添加到此路径
	* @param [in] rect 矩形区域
	*/
	virtual void AddRect(const UiRect& rect) = 0;

	/** 将椭圆添加到此路径
	* @param [in] 椭圆的矩形区域 
	*             left 椭圆边框左上角的 x 坐标
	*             top  椭圆边框左上角的 y 坐标
	*             right left + 椭圆边界矩形的宽度
	*             bottom top + 椭圆边界矩形的高度
	*/
	virtual void AddEllipse(const UiRect& rect) = 0;

	/** 将椭圆弧添加到此路径
	* @param [in] 椭圆的矩形区域 
	* @param [in] startAngle 椭圆水平轴与弧线起点之间的顺时针角度（以度为单位）
	* @param [in] sweepAngle 起点 (startAngle) 和弧的终点之间的顺时针角度（以度为单位）
	*/
	virtual void AddArc(const UiRect& rect, float startAngle, float sweepAngle) = 0;

	/** 将多边形添加到此路径
	* @param [in] points 指定多边形顶点的点数组
	* @param [in] count 数组中的元素数
	*/
	virtual void AddPolygon(const UiPoint* points, int count) = 0;
	virtual void AddPolygon(const UiPointF* points, int count) = 0;

	/** 对路径进行矩阵变换，可以进行旋转等操作
	* @param [in] pMatrix 矩阵接口
	*/
	virtual void Transform(IMatrix* pMatrix) = 0;

	/** 获取此路径的边界矩形
	* @param [in] pen 关联的Pen对象，可以为nullptr
	*/
	virtual UiRect GetBounds(const IPen* pen) = 0;

	/** 关闭当前绘图
	*/
	virtual void Close() = 0;

	/** 重置Path数据
	*/
	virtual void Reset() = 0;

	/** 复制Path对象
	*/
	virtual IPath* Clone() = 0;
};

/** 3x3 矩阵接口
*/
class UILIB_API IMatrix : public nbase::SupportWeakCallback
{
public:
	/** 平移操作
	* @param [in] offsetX X轴方向平移的偏移量
	* @param [in] offsetY Y轴方向平移的偏移量
	*/
	virtual void Translate(int offsetX, int offsetY) = 0;

	/** 缩放操作
	* @param [in] scaleX X轴方向缩放比例
	* @param [in] scaleY Y轴方向缩放比例
	*/
	virtual void Scale(float scaleX, float scaleY) = 0;

	/** 旋转操作，以源点坐标(0,0)为中心点做旋转操作
	* @param [in] angle 旋转的角度，正数为顺时针操作，负数为逆时针操作
	*/
	virtual void Rotate(float angle) = 0;

	/** 旋转操作，以坐标(center)为中心点做旋转操作
	* @param [in] angle 旋转的角度，正数为顺时针操作，负数为逆时针操作
	* @param [in] center 旋转的中心点坐标值
	*/
	virtual void RotateAt(float angle, const UiPoint& center) = 0;
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

/** Render类型
*/
enum class RenderType
{
	kRenderType_Skia = 0,
	kRenderType_GdiPlus = 1
};

/** 格式文本数据
*/
class RichTextData
{
public:
	/** 文字内容
	*/
	UiString m_text;

	/** 文字颜色
	*/
	UiColor m_textColor;

	/** 背景颜色
	*/
	UiColor m_bgColor;

	/** 字体信息
	*/
	UiFont m_fontInfo;

	/** 行间距
	*/
	float m_fRowSpacingMul = 1.0f;

	/** 对象绘制区域(输出参数)
	*/
	std::vector<UiRect> m_textRects;
};

/** 渲染接口
*/
class UILIB_API IRender : public virtual nbase::SupportWeakCallback
{
public:
	/** 获取Render实现类型
	*/
	virtual RenderType GetRenderType() const = 0;

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
	* @param [in] rc剪辑区域，与当前剪辑区取交集作为新的剪辑区域
	* @param [in] bIntersect ClipOp操作标志，true表示kIntersect操作，false表示kDifference操作
	*/
	virtual void SetClip(const UiRect& rc, bool bIntersect = true) = 0;

	/** 设置圆角矩形剪辑区域，并保存当前设备上下文的状态
	* @param [in] rcItem 剪辑区域，与当前剪辑区取交集作为新的剪辑区域
	* @param [in] width 圆角的宽度
	* @param [in] height 圆角的的度
	* @param [in] bIntersect ClipOp操作标志，true表示kIntersect操作，false表示kDifference操作
	*/
	virtual void SetRoundClip(const UiRect& rcItem, int width, int height, bool bIntersect = true) = 0;

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
	* @param [in] rcDest 绘制的目标区域
	* @param [in] rcDestCorners 绘制的目标区域的边角信息，用于九宫格绘制
	* @param [in] rcSource 绘制的源图片区域
	* @param [in] rcSourceCorners 绘制源图片的边角信息，用于九宫格绘制
	* @param [in] uFade 透明度（0 - 255）
	* @param [in] xtiled 横向平铺
	* @param [in] ytiled 纵向平铺
	* @param [in] fullxtiled 如果为true，横向平铺绘制时，确保是完整绘制图片，该参数仅当xtiled为true时有效
	* @param [in] fullytiled 如果为true，纵向平铺绘制时，确保是完整绘制图片，该参数仅当ytiled为true时有效
	* @param [in] nTiledMargin 平铺绘制时，图片的横向、纵向间隔，该参数仅当xtiled为true或者ytiled为true时有效
	*/
	virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap, 
						   const UiRect& rcDest, const UiRect& rcDestCorners,
						   const UiRect& rcSource, const UiRect& rcSourceCorners,
						   uint8_t uFade = 255, bool xtiled = false, bool ytiled = false, 
						   bool fullxtiled = true, bool fullytiled = true, int nTiledMargin = 0) = 0;
	/** 绘制图片（采用九宫格方式绘制图片）, 无圆角参数
	*/
	virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap, 
						   const UiRect& rcDest,  const UiRect& rcSource, 
						   uint8_t uFade = 255, bool xtiled = false, bool ytiled = false, 
						   bool fullxtiled = true, bool fullytiled = true, int nTiledMargin = 0) = 0;

	/** 绘制图片
	* @param [in] rcPaint 当前全部可绘制区域（用于避免非可绘制区域的绘制，以提高绘制性能）
	* @param [in] pBitmap 用于绘制的位图接口
	* @param [in] rcDest 绘制的目标区域
	* @param [in] rcSource 绘制的源图片区域
	* @param [in] uFade 透明度（0 - 255）
	* @param [in] pMatrix 绘制时的变换矩阵接口
	*/
	virtual void DrawImageRect(const UiRect& rcPaint, IBitmap* pBitmap,
						       const UiRect& rcDest, const UiRect& rcSource,
						       uint8_t uFade = 255, IMatrix* pMatrix = nullptr) = 0;

	/** 绘制直线
	* @param [in] pt1 起始点坐标
	* @param [in] pt2 终止点坐标
	* @param [in] penColor 画笔的颜色值
	* @param [in] nWidth 画笔的宽度
	*/
	virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int32_t nWidth) = 0;

	/** 绘制直线
	* @param [in] pt1 起始点坐标
	* @param [in] pt2 终止点坐标
	* @param [in] penColor 画笔的颜色值
	* @param [in] nWidth 画笔的宽度
	*/
	virtual void DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth) = 0;

	/** 绘制直线，支持各种线形
	* @param [in] pt1 起始点坐标
	* @param [in] pt2 终止点坐标
	* @param [in] pen 画笔的接口
	*/
	virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, IPen* pen) = 0;

	/** 绘制矩形
	* @param [in] rc 矩形区域
	* @param [in] penColor 画笔的颜色值
	* @param [in] nWidth 画笔的宽度
	* @param [in] bLineInRect 如果为true，表示确保画出的线条严格限制在rc矩形内部，否则线的中心点是与rc边线对齐的，线条会有部分超出rc矩形范围
	*/
	virtual void DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool bLineInRect = false) = 0;

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

	/** 绘制曲线（椭圆的一部分）
	* @param [in] rc 包含圆弧的椭圆的矩形边界区域
	* @param [in] startAngle  x轴与弧起点之间的角度
	* @param [in] sweepAngle  圆弧起点和终点之间的角度，正数是顺时针方向，负数是逆时针方向
	* @param [in] useCenter 如果为true，则包含椭圆的中心点（仅Skia引擎使用）
	* @param [in] pen 画笔的接口，设置画笔颜色和画笔宽度
	* @param [in] gradientColor 可选参数，渐变颜色
	* @param [in] gradientRect 可选参数，渐变颜色的矩形区域设置，仅当gradientColor不为nullptr时有效
	*/
	virtual void DrawArc(const UiRect& rc, float startAngle, float sweepAngle, bool useCenter, 
						 const IPen* pen, 
		                 UiColor* gradientColor = nullptr, const UiRect* gradientRect = nullptr) = 0;

	/** 绘制圆形
	* @param [in] centerPt 圆心坐标点
	* @param [in] radius 圆的半径
	* @param [in] penColor 画笔的颜色值
	* @param [in] nWidth 画笔的宽度
	*/
	virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, int nWidth) = 0;

	/** 填充圆形
	* @param [in] centerPt 圆心坐标点
	* @param [in] radius 圆的半径
	* @param [in] dwColor 颜色值
	* @param [in] uFade 透明度（0 - 255）
	*/
	virtual void FillCircle(const UiPoint& centerPt, int32_t radius, UiColor dwColor, uint8_t uFade = 255) = 0;

	/** 绘制路径
	* @param [in] path 路径的接口
	* @param [in] pen 绘制路径使用的画笔
	*/
	virtual void DrawPath(const IPath* path, const IPen* pen) = 0;

	/** 填充路径
	* @param [in] path 路径的接口
	* @param [in] brush 填充路径使用的画刷
	*/
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

	/** 绘制格式文本
	* @param [in] 矩形区域
	* @param [in,out] richTextData 格式化文字内容，返回文字绘制的区域
	* @param [in] uFormat 文字的格式，参见 enum DrawStringFormat 类型定义
	* @param [in] bMeasureOnly 如果为true，仅评估绘制文字所需区域，不执行文字绘制
	* @param [in] uFade 透明度（0 - 255）
	*/
	virtual void DrawRichText(const UiRect& rc,
		                      std::vector<RichTextData>& richTextData,
		                      uint32_t uFormat = 0,
		                      bool bMeasureOnly = false,
		                      uint8_t uFade = 255) = 0;

	/** 在指定矩形周围绘制阴影（高斯模糊, 只支持外部阴影，不支持内部阴影）
	* @param [in] rc 矩形区域
	* @param [in] roundSize 阴影的圆角宽度和高度
	* @param [in] cpOffset 设置阴影偏移量（offset-x 和 offset-y）
	*                      <offset-x> 设置水平偏移量，如果是负值则阴影位于矩形左边。 
	*                      <offset-y> 设置垂直偏移量，如果是负值则阴影位于矩形上面。
	* @param [in] nBlurRadius 模糊半径，值越大，模糊面积越大，阴影就越大越淡, 如果为0，此时阴影边缘锐利，无模糊效果，不能为负值。
	* @param [in] nSpreadRadius 扩展半径，即模糊区域距离rc矩形边缘多少个像素。
	*                           取正值时，阴影扩大；取负值时，阴影收缩。
	* @param [in] dwColor 阴影的颜色值
	*/
	virtual void DrawBoxShadow(const UiRect& rc, 
		                       const UiSize& roundSize, 
		                       const UiPoint& cpOffset, 
		                       int nBlurRadius, 
		                       int nSpreadRadius,
		                       UiColor dwColor) = 0;


	/** 分离位图
	*@return 返回位图接口，返回后由调用方管理资源（包括释放资源等）
	*/
	virtual IBitmap* DetachBitmap() = 0;

	/** 将矩形区域内的图像Alpha设定为指定值alpha(0 - 255)
	* @param [in] rcDirty 矩形区域
	* @param [in] alpha 需要设定的Aplpa值
	*/
	virtual void ClearAlpha(const UiRect& rcDirty, uint8_t alpha = 0) = 0;

	/** 恢复矩形区域内的图像Alpha值为alpha(0 - 255)
	* @param [in] rcDirty 矩形区域
	* @param [in] rcShadowPadding 阴影边距（分别对应矩形的左/右/上/下边距的Padding值）
	* @param [in] alpha 需要恢复的Alpha值（需要与ClearAlpha时传入的alpha值相同）
	*/
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) = 0;

	/** 恢复矩形区域内的图像Alpha值为255
	* @param [in] rcDirty 矩形区域
	* @param [in] rcShadowPadding 阴影边距（分别对应矩形的左/右/上/下边距的Padding值）
	*/
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding = UiPadding()) = 0;

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

/** 渲染接口管理，用于创建Font、Pen、Brush、Path、Matrix、Bitmap、Render等渲染实现对象
*/
class UILIB_API IRenderFactory
{
public:
	virtual ~IRenderFactory() = default;

	/** 创建一个Font对象
	*/
	virtual IFont* CreateIFont() = 0;

	/** 创建一个Pen对象
	*/
	virtual IPen* CreatePen(UiColor color, int width = 1) = 0;

	/** 创建一个Brush对象
	*/
	virtual IBrush* CreateBrush(UiColor corlor) = 0;

	/** 创建一个Path对象
	*/
	virtual IPath* CreatePath() = 0;

	/** 创建一个Matrix对象
	*/
	virtual IMatrix* CreateMatrix() = 0;

	/** 创建一个Bitmap对象
	*/
	virtual IBitmap* CreateBitmap() = 0;

	/** 创建一个Render对象
	*/
	virtual IRender* CreateRender() = 0;
};

} // namespace ui

#endif // UI_RENDER_IRENDER_H_
