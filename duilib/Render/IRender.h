#ifndef UI_RENDER_IRENDER_H_
#define UI_RENDER_IRENDER_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/UiTypes.h"
#include "duilib/Core/SharePtr.h"
#include <map>

namespace ui 
{
/** 字体接口
*/
class UILIB_API IFont: public virtual SupportWeakCallback
{
public:
    /** 初始化字体(内部未对字体大小做DPI自适应)
     */
    virtual bool InitFont(const UiFont& fontInfo) = 0;

    /** 获取字体名
    */
    virtual DString FontName() const = 0;

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

/** 字体管理器接口
*/
class UILIB_API IFontMgr : public virtual SupportWeakCallback
{
public:
    /** 获取字体个数
    * @return 返回字体的个数
    */
    virtual uint32_t GetFontCount() const = 0;

    /** 获取字体名称
    * @param [in] nIndex 字体的下标值：[0, GetFontCount())
    * @param [out] fontName 返回字体名称
    * @return 成功返回true，失败返回false
    */
    virtual bool GetFontName(uint32_t nIndex, DString& fontName) const = 0;

    /** 判断是否含有该字体
    * @param [int] fontName 字体名称
    * @return 如果含有该字体名称对应的字体返回true，否则返回false
    */
    virtual bool HasFontName(const DString& fontName) const = 0;

    /** 设置默认字体名称（当需要加载的字体不存在时，使用默认的字体）
    * @param [in] fontName 默认的字体名称
    */
    virtual void SetDefaultFontName(const DString& fontName) = 0;

    /** 加载指定字体文件
    * @param [in] fontFilePath 字体文件的路径（本地绝对路径）
    * @return 成功返回true，失败返回false
    */
    virtual bool LoadFontFile(const DString& fontFilePath) = 0;

    /** 加载指定字体数据
    * @param [in] data 字体文件的内存数据
    * @param [in] length 字体文件的内存数据长度
    * @return 成功返回true，失败返回false
    */
    virtual bool LoadFontFileData(const void* data, size_t length) = 0;
  
    /** 清除已加载的字体文件
    */
    virtual void ClearFontFiles() = 0;

    /** 清除字体缓存
    */
    virtual void ClearFontCache() = 0;
};

/** Skia引擎需要传入Alpha类型
*/
enum UILIB_API BitmapAlphaType: int
{
    kUnknown_SkAlphaType,   //!< uninitialized
    kOpaque_SkAlphaType,    //!< pixel is opaque
    kPremul_SkAlphaType,    //!< pixel components are premultiplied by alpha
    kUnpremul_SkAlphaType   //!< pixel components are independent of alpha
};

/** 位图接口
*/
class UILIB_API IBitmap : public virtual SupportWeakCallback
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

    /** 克隆生成新的的位图
    *@return 返回新生成的位图接口，由调用方释放资源
    */
    virtual IBitmap* Clone() = 0;
};

/** 画笔接口
*/
class UILIB_API IPen : public virtual SupportWeakCallback
{
public:
    /** 设置画笔宽度
    */
    virtual void SetWidth(float fWidth) = 0;

    /** 获取画笔宽度
    */
    virtual float GetWidth() const = 0;

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
        kButt_Cap   = 0,    //平笔帽（默认）
        kRound_Cap  = 1,    //圆笔帽
        kSquare_Cap = 2     //方笔帽
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
        kMiter_Join = 0,    //尖角（默认）
        kBevel_Join = 1,    //平角
        kRound_Join = 2     //圆角        
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
        kDashStyleSolid         = 0,    //实线（默认）
        kDashStyleDash          = 1,    //虚线
        kDashStyleDot           = 2,    //虚线
        kDashStyleDashDot       = 3,    //交替虚线
        kDashStyleDashDotDot    = 4     //交替短划线点点线
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
class UILIB_API IBrush : public virtual SupportWeakCallback
{
public:
    virtual IBrush* Clone() = 0;
    virtual UiColor GetColor() const = 0;
};

/** 路径接口
*/
class IMatrix;
class UILIB_API IPath : public virtual SupportWeakCallback
{
public:    
    /** 填充类型，在路径或曲线相交时如何填充形成的区域
    */
    enum class FillType 
    {
        /** Specifies that "inside" is computed by an odd number of edge crossings
        */
        kEvenOdd        = 0, //FillModeAlternate

        /** Specifies that "inside" is computed by a non-zero sum of signed edge crossings 
        */
        kWinding        = 1, //FillModeWinding

        /** Same as EvenOdd, but draws outside of the path, rather than inside 
        */
        kInverseEvenOdd    = 2,

        /** Same as Winding, but draws outside of the path, rather than inside 
        */
        kInverseWinding    = 3
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
    *     曲线从 p1 开始，以 p2 结尾。 
    *     曲线不通过控制点，但控制点充当磁铁，将曲线拉向某个方向，并影响曲线的弯曲方式。
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
    *                    序列中的每个后续样条只需要另外三个点：
    *                    前一个样条的终点用作起点，序列中的下两个点是控制点，第三个点是终点。
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
class UILIB_API IMatrix : public SupportWeakCallback
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

/** 渲染接口中使用的DPI转换辅助接口
*/
class UILIB_API IRenderDpi : public SupportWeakCallback
{
public:
    /** 根据界面缩放比来缩放整数
    * @param [in] iValue 整数
    * @return int 缩放后的值
    */
    virtual int32_t GetScaleInt(int32_t iValue) const = 0;

    /** 根据界面缩放比来缩放整数
    * @param [in] fValue 浮点数
    * @return 缩放后的值
    */
    virtual float GetScaleFloat(float fValue) const = 0;
};

typedef std::shared_ptr<IRenderDpi> IRenderDpiPtr;

/** 绘制回调接口
*/
class UILIB_API IRenderPaint
{
public:
    /** 通过回调接口，完成绘制
    * @param [in] rcPaint 需要绘制的区域（客户区坐标）
    */
    virtual bool DoPaint(const UiRect& rcPaint) = 0;

    /** 回调接口，获取当前窗口的透明度值
    */
    virtual uint8_t GetLayeredWindowAlpha() = 0;

    /** 获取界面需要绘制的区域，以实现局部绘制
    * @param [out] rcUpdate 返回需要绘制的区域矩形范围
    * @return 返回true表示支持局部绘制，返回false表示不支持局部绘制
    */
    virtual bool GetUpdateRect(UiRect& rcUpdate) const = 0;
};

/** 光栅操作代码
*/
enum class UILIB_API RopMode
{
    kSrcCopy,    //对应于 SRCCOPY
    kDstInvert, //对应于 DSTINVERT
    kSrcInvert, //对应于 SRCINVERT
    kSrcAnd     //对应于 SRCAND
};

/** 绘制文本时的格式
*/
enum UILIB_API DrawStringFormat
{
    TEXT_LEFT           = 0x0001,   //水平对齐方式：靠左
    TEXT_CENTER         = 0x0002,   //水平对齐方式：居中
    TEXT_RIGHT          = 0x0004,   //水平对齐方式：靠右

    TEXT_TOP            = 0x0010,   //垂直对齐方式：靠上
    TEXT_VCENTER        = 0x0020,   //垂直对齐方式：居中
    TEXT_BOTTOM         = 0x0040,   //垂直对齐方式：靠下

    TEXT_SINGLELINE     = 0x0100,   //单行文本
    TEXT_NOCLIP         = 0x0200,   //绘制的时候，不设置剪辑区域
    TEXT_WORD_WRAP      = 0x0400,   //自动换行（仅在IRender::DrawRichText接口支持此属性，其他文字绘制函数不支持该属性）

    TEXT_PATH_ELLIPSIS  = 0x4000,   //如果绘制区域不足，按显示文件路径的方式，在中间加"..."省略部分文字
    TEXT_END_ELLIPSIS   = 0x8000    //如果绘制区域不足，在结尾加"..."，省略部分文字
};

/** Render类型
*/
enum class RenderType
{
    kRenderType_Skia = 0
};

/** 格式文本数据
*/
class RichTextData
{
public:
    /** 文字内容(由外部负责保证字符串指向内存的生命周期)
    */
    std::wstring_view m_textView;

    /** 文字颜色
    */
    UiColor m_textColor;

    /** 背景颜色
    */
    UiColor m_bgColor;

    /** 字体信息
    */
    SharePtr<UiFontEx> m_pFontInfo;

    /** 行间距
    */
    float m_fRowSpacingMul = 1.0f;

    /** 绘制文字的属性(包含文本对齐方式等属性，参见 enum DrawStringFormat)
    */
    uint16_t m_textStyle = 0;
};

/** 绘制的字符标记位
*/
enum RichTextCharFlag: uint8_t
{
    kIsIgnoredChar  = 0x01,     //当前字符为未绘制字符
    kIsLowSurrogate = 0x02,     //该字符为低代理字符（由两个Unicode字符构成的字，UTF16编码的字形，每个字占1个或者2个Unicode字符）
    kIsReturn       = 0x04,     //当前字符是否为回车'\r'
    kIsNewLine      = 0x08,     //当前字符是否为换行'\n'
};

/** 绘制的字符属性（共4个字节）
*/
struct RichTextCharInfo
{
    /** 属性标志(读取)
    */
    inline uint8_t CharFlag() const
    {
        uint32_t v = m_value;
        v >>= 24;
        return (uint8_t)v;
    }

    /** 属性标志(设置)
    */
    inline void SetCharFlag(uint8_t flag)
    {
        uint32_t v = flag;
        v <<= 24;
        m_value &= 0x00FFFFFF;
        m_value |= v;
    }

    /** 属性标志(添加)
    */
    inline void AddCharFlag(uint8_t flag)
    {
        uint32_t v = flag;
        v <<= 24;
        m_value |= v;
    }

    /** 字符宽度(读取)
    */
    inline float CharWidth() const
    {
        uint32_t v = m_value & 0x00FFFFFF;
        float fValue = (float)v;
        fValue /= 1000.0f;
        return fValue;
    }

    /** 字符宽度(设置)
    */
    inline void SetCharWidth(float charWidth)
    {
        uint32_t v = (uint32_t)(ui::CEILF(charWidth * 1000.0f));
        ASSERT(v < 0x00FFFFFF);
        v &= 0x00FFFFFF;
        m_value &= 0xFF000000;
        m_value |= v;
    }

    /** 该字符是否为回车
    */
    inline bool IsReturn() const { return CharFlag() & RichTextCharFlag::kIsReturn; }

    /** 该字符是否为换行符
    */
    inline bool IsNewLine() const { return CharFlag() & RichTextCharFlag::kIsNewLine; }

    /** 该字符是否为非绘制字符
    */
    inline bool IsIgnoredChar() const { return CharFlag() & RichTextCharFlag::kIsIgnoredChar; }

    /** 该字符是否为低代理字符
    */
    inline bool IsLowSurrogate() const { return CharFlag() & RichTextCharFlag::kIsLowSurrogate; }

    /** 比较操作符
    */
    inline bool operator == (const RichTextCharInfo& r) const { return m_value == r.m_value; }
    inline bool operator != (const RichTextCharInfo& r) const { return m_value != r.m_value; }

private:
    /** 使用整型存储，减少内存占有量
    */
    uint32_t m_value = 0;
};

/** 逻辑行(矩形区域内显示的行，物理行数据在自动换行的情况下会对应多个逻辑行)的基本信息
*/
struct RichTextRowInfo: public NVRefCount<RichTextRowInfo>
{
    /** 本行中的字符个数，字符属性
    */
    std::vector<RichTextCharInfo> m_charInfo;

    /** 该行的文字所占矩形区域
    */
    UiRectF m_rowRect;

    /** 本行的left坐标偏移量（用于支持居中和靠右对齐）
    */
    int32_t m_xOffset = 0;
};
typedef SharePtr<RichTextRowInfo> RichTextRowInfoPtr;

/** 物理行文本的数据
*/
struct RichTextLineInfo: public NVRefCount<RichTextLineInfo>
{
    /** 文本数据长度
    */
    uint32_t m_nLineTextLen = 0;

    /** 文本数据
    */
    UiStringW m_lineText;

    /** 逻辑行的基本信息
    */
    std::vector<RichTextRowInfoPtr> m_rowInfo;
};
typedef SharePtr<RichTextLineInfo> RichTextLineInfoPtr;

/** 物理行的数据结构
*/
typedef std::vector<RichTextLineInfoPtr> RichTextLineInfoList;

/** 物理行的数据传入参数
*/
struct RichTextLineInfoParam
{
    /** 本次绘制中，关联的物理行的数据起始下标值
    */
    uint32_t m_nStartLineIndex = 0;

    /** 起始的行号（逻辑行号）
    */
    uint32_t m_nStartRowIndex = 0;

    /** 物理行的数据
    */
    RichTextLineInfoList* m_pLineInfoList = nullptr;
};

/** DrawRichText的绘制缓存
*/
class DrawRichTextCache;

/** 裁剪区域类型
*/
enum class RenderClipType
{
    kEmpty, //空，无裁剪信息
    kRect,  //裁剪区域是个矩形
    kRegion //裁剪区域是个Region
};

/** 后台绘制方式
*/
enum class RenderBackendType
{
    /** 使用CPU绘制
    */
    kRaster_BackendType = 0,

    /** 使用OpenGL的绘制
    *   注意事项：
    *   （1）一个线程内，只允许有一个窗口使用OpenGL绘制，否则会出现导致程序崩溃的问题
    *   （2）OpenGL绘制的窗口，不能是分层窗口（即带有WS_EX_LAYERED属性的窗口）
    *   （3）使用OpenGL的窗口，每次绘制都是绘制整个窗口，不支持局部绘制，所以不一定比使用CPU绘制的情况下性能更好，最好根据实际情况评估使用最佳的绘制方式
    */
    kNativeGL_BackendType = 1
};

/** 渲染接口
*/
class IRenderFactory;
class UILIB_API IRender : public virtual SupportWeakCallback
{
public:
    /** 获取Render实现类型
    */
    virtual RenderType GetRenderType() const = 0;

    /** 获取后台渲染的类型
    */
    virtual RenderBackendType GetRenderBackendType() const = 0;

    /** 获取画布宽度
    */
    virtual int32_t GetWidth() const = 0;

    /** 获取画布高度
    */
    virtual int32_t GetHeight() const = 0;

    /** 调整画布大小
    */
    virtual bool Resize(int32_t width, int32_t height) = 0;

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
    virtual void SaveClip(int32_t& nState) = 0;

    /** 将设备上下文还原到最近一次保存的状态
    * @param [in] 保存的设备上下文标志（由SaveClip返回）
    */
    virtual void RestoreClip(int32_t nState) = 0;

    /** 设置矩形剪辑区域，并保存当前设备上下文的状态
    * @param [in] rc剪辑区域，与当前剪辑区取交集作为新的剪辑区域
    * @param [in] bIntersect ClipOp操作标志，true表示kIntersect操作，false表示kDifference操作
    */
    virtual void SetClip(const UiRect& rc, bool bIntersect = true) = 0;

    /** 设置圆角矩形剪辑区域，并保存当前设备上下文的状态
    * @param [in] rcItem 剪辑区域，与当前剪辑区取交集作为新的剪辑区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的的高度
    * @param [in] bIntersect ClipOp操作标志，true表示kIntersect操作，false表示kDifference操作
    */
    virtual void SetRoundClip(const UiRect& rcItem, float rx, float ry, bool bIntersect = true) = 0;

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
    virtual bool BitBlt(int32_t x, int32_t y, int32_t cx, int32_t cy,
                        IRender* pSrcRender, int32_t xSrc, int32_t ySrc,
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
    virtual bool StretchBlt(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest,
                            IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc,
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
    virtual bool AlphaBlend(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest,
                            IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc,
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
    * @param [in] bWindowShadowMode 九宫格绘制时，不绘制中间部分（比如窗口阴影，只需要绘制边框，不需要绘制中间部分）
    */
    virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap, 
                           const UiRect& rcDest, const UiRect& rcDestCorners,
                           const UiRect& rcSource, const UiRect& rcSourceCorners,
                           uint8_t uFade = 255, bool xtiled = false, bool ytiled = false, 
                           bool fullxtiled = true, bool fullytiled = true, int32_t nTiledMargin = 0,
                           bool bWindowShadowMode = false) = 0;
    /** 绘制图片（采用九宫格方式绘制图片）, 无圆角参数
    */
    virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap, 
                           const UiRect& rcDest,  const UiRect& rcSource, 
                           uint8_t uFade = 255, bool xtiled = false, bool ytiled = false, 
                           bool fullxtiled = true, bool fullytiled = true, int32_t nTiledMargin = 0,
                           bool bWindowShadowMode = false) = 0;

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
    * @param [in] fWidth 画笔的宽度
    */
    virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, float fWidth) = 0;

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

    /** 绘制直线，支持各种线形
    * @param [in] pt1 起始点坐标
    * @param [in] pt2 终止点坐标
    * @param [in] pen 画笔的接口
    */
    virtual void DrawLine(const UiPointF& pt1, const UiPointF& pt2, IPen* pen) = 0;

    /** 绘制矩形
    * @param [in] rc 矩形区域
    * @param [in] penColor 画笔的颜色值
    * @param [in] nWidth 画笔的宽度
    * @param [in] bLineInRect 如果为true，表示确保画出的线条严格限制在rc矩形内部，否则线的中心点是与rc边线对齐的，线条会有部分超出rc矩形范围
    */
    virtual void DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool bLineInRect = false) = 0;
    virtual void DrawRect(const UiRectF& rc, UiColor penColor, int32_t nWidth, bool bLineInRect = false) = 0;

    /** 绘制矩形
    * @param [in] rc 矩形区域
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    * @param [in] bLineInRect 如果为true，表示确保画出的线条严格限制在rc矩形内部，否则线的中心点是与rc边线对齐的，线条会有部分超出rc矩形范围
    */
    virtual void DrawRect(const UiRect& rc, UiColor penColor, float fWidth, bool bLineInRect = false) = 0;
    virtual void DrawRect(const UiRectF& rc, UiColor penColor, float fWidth, bool bLineInRect = false) = 0;

    /** 绘制矩形，支持各种线形
    * @param [in] rc 矩形区域
    * @param [in] pen 画笔的接口
    * @param [in] bLineInRect 如果为true，表示确保画出的线条严格限制在rc矩形内部，否则线的中心点是与rc边线对齐的，线条会有部分超出rc矩形范围
    */
    virtual void DrawRect(const UiRect& rc, IPen* pen, bool bLineInRect = false) = 0;
    virtual void DrawRect(const UiRectF& rc, IPen* pen, bool bLineInRect = false) = 0;

    /** 用颜色填充矩形
    * @param [in] rc 目标矩形区域
    * @param [in] dwColor 颜色值
    * @param [in] uFade 透明度（0 - 255）
    */
    virtual void FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade = 255) = 0;
    virtual void FillRect(const UiRectF& rc, UiColor dwColor, uint8_t uFade = 255) = 0;

    /** 用渐变颜色填充矩形（支持渐变颜色）
    * @param [in] rc 目标矩形区域
    * @param [in] dwColor 第一颜色值
    * @param [in] dwColor2 第二颜色值
    * @param [in] nColor2Direction 渐变颜色的渐变方向，"1": 左->右，"2": 上->下，"3": 左上->右下，"4": 右上->左下
    * @param [in] uFade 透明度（0 - 255）
    */
    virtual void FillRect(const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) = 0;
    virtual void FillRect(const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) = 0;

    /** 绘制圆角矩形
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] penColor 画笔的颜色值
    * @param [in] nWidth 画笔的宽度
    */
    virtual void DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor penColor, int32_t nWidth) = 0;
    virtual void DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, int32_t nWidth) = 0;

    /** 绘制圆角矩形
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    */
    virtual void DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor penColor, float fWidth) = 0;
    virtual void DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, float fWidth) = 0;

    /** 绘制圆角矩形，支持各种线形
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] pen 画笔的接口
    */
    virtual void DrawRoundRect(const UiRect& rc, float rx, float ry, IPen* pen) = 0;
    virtual void DrawRoundRect(const UiRectF& rc, float rx, float ry, IPen* pen) = 0;

    /** 用颜色填充圆角矩形
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] dwColor 颜色值
    * @param [in] uFade 透明度（0 - 255）
    */
    virtual void FillRoundRect(const UiRect& rc, float rx, float ry, UiColor dwColor, uint8_t uFade = 255) = 0;
    virtual void FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, uint8_t uFade = 255) = 0;

    /** 用颜色填充圆角矩形(支持渐变颜色)
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] dwColor 颜色值
    * @param [in] dwColor2 第二颜色值
    * @param [in] nColor2Direction 渐变颜色的渐变方向，"1": 左->右，"2": 上->下，"3": 左上->右下，"4": 右上->左下
    * @param [in] uFade 透明度（0 - 255）
    */
    virtual void FillRoundRect(const UiRect& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) = 0;
    virtual void FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) = 0;

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
    virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, int32_t nWidth) = 0;

    /** 绘制圆形
    * @param [in] centerPt 圆心坐标点
    * @param [in] radius 圆的半径
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    */
    virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, float fWidth) = 0;

    /** 绘制圆形，支持各种线形
    * @param [in] centerPt 圆心坐标点
    * @param [in] radius 圆的半径
    * @param [in] pen 画笔的接口
    */
    virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, IPen* pen) = 0;

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

    /** 填充路径（支持背景颜色渐变）
    * @param [in] path 路径的接口
    * @param [in] rc 矩形区域
    * @param [in] dwColor 填充路径使用的第一个颜色
    * @param [in] dwColor2 填充路径使用的第二个颜色
    * @param [in] nColor2Direction 渐变颜色的渐变方向，"1": 左->右，"2": 上->下，"3": 左上->右下，"4": 右上->左下
    */
    virtual void FillPath(const IPath* path, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) = 0;

    /** 计算指定文本字符串的宽度和高度
    * @param [in] strText 文字内容
    * @param [in] pFont 文字的字体数据接口
    * @param [in] uFormat 文字的格式，参见 enum DrawStringFormat 类型定义
    * @param [in] width 当前区域的限制宽度
    * @return 返回文本字符串的宽度和高度，以矩形表示结果
    */
    virtual UiRect MeasureString(const DString& strText, 
                                 IFont* pFont, 
                                 uint32_t uFormat,
                                 int32_t width = DUI_NOSET_VALUE) = 0;
    /** 绘制文字
    * @param [in] textRect 文字绘制的矩形区域
    * @param [in] strText 文字内容
    * @param [in] dwTextColor 文字颜色值
    * @param [in] pFont 文字的字体数据接口
    * @param [in] uFormat 文字的格式，参见 enum DrawStringFormat 类型定义
    * @param [in] uFade 透明度（0 - 255）
    */
    virtual void DrawString(const UiRect& textRect,
                            const DString& strText,
                            UiColor dwTextColor,
                            IFont* pFont, 
                            uint32_t uFormat,
                            uint8_t uFade = 255) = 0;

    /** 计算格式文本的宽度和高度
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] pRenderFactory 渲染接口，用于创建字体
    * @param [in] richTextData 格式化文字内容，返回文字绘制的区域
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    */
    virtual void MeasureRichText(const UiRect& textRect,
                                 const UiSize& szScrollOffset,
                                 IRenderFactory* pRenderFactory, 
                                 const std::vector<RichTextData>& richTextData,
                                 std::vector<std::vector<UiRect>>* pRichTextRects) = 0;

    /** 计算格式文本的宽度和高度, 并计算每个字符的位置
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] pRenderFactory 渲染接口，用于创建字体
    * @param [in] richTextData 格式化文字内容
    * @param [in,out] pLineInfoParam 如果不为nullptr，则计算每个字符的区域
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    */
    virtual void MeasureRichText2(const UiRect& textRect,
                                  const UiSize& szScrollOffset,
                                  IRenderFactory* pRenderFactory, 
                                  const std::vector<RichTextData>& richTextData,
                                  RichTextLineInfoParam* pLineInfoParam,
                                  std::vector<std::vector<UiRect>>* pRichTextRects) = 0;

    /** 计算格式文本的宽度和高度, 并计算每个字符的位置，并创建绘制缓存
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] pRenderFactory 渲染接口，用于创建字体
    * @param [in] richTextData 格式化文字内容
    * @param [in,out] pLineInfoParam 如果不为nullptr，则计算每个字符的区域
    * @param [out] spDrawRichTextCache 返回绘制缓存
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    */
    virtual void MeasureRichText3(const UiRect& textRect,
                                  const UiSize& szScrollOffset,
                                  IRenderFactory* pRenderFactory, 
                                  const std::vector<RichTextData>& richTextData,
                                  RichTextLineInfoParam* pLineInfoParam,
                                  std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                  std::vector<std::vector<UiRect>>* pRichTextRects) = 0;

    /** 绘制格式文本
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] pRenderFactory 渲染接口，用于创建字体
    * @param [in] richTextData 格式化文字内容
    * @param [in] uFade 透明度（0 - 255）
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    */
    virtual void DrawRichText(const UiRect& textRect,
                              const UiSize& szScrollOffset,
                              IRenderFactory* pRenderFactory, 
                              const std::vector<RichTextData>& richTextData,
                              uint8_t uFade = 255,
                              std::vector<std::vector<UiRect>>* pRichTextRects = nullptr) = 0;

    /** 创建RichText的绘制缓存
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] pRenderFactory 渲染接口，用于创建字体
    * @param [in] richTextData 格式化文字内容
    * @param [out] spDrawRichTextCache 返回绘制缓存
    */
    virtual bool CreateDrawRichTextCache(const UiRect& textRect,
                                         const UiSize& szScrollOffset,
                                         IRenderFactory* pRenderFactory,
                                         const std::vector<RichTextData>& richTextData,
                                         std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache) = 0;

    /** 判断RichText的绘制缓存是否有效
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] richTextData 格式化文字内容，返回文字绘制的区域
    * @param [out] spDrawRichTextCache 返回绘制缓存
    */
    virtual bool IsValidDrawRichTextCache(const UiRect& textRect,
                                          const std::vector<RichTextData>& richTextData,
                                          const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache) = 0;

    /** 更新RichText的绘制缓存(增量计算)
    * @param [in] spOldDrawRichTextCache 需要更新的缓存
    * @param [in] spUpdateDrawRichTextCache 增量绘制的缓存
    * @param [in,out] richTextDataNew 最新的完整数据, 数据会交换给内部容器
    * @param [in] nStartLine 重新计算的起始行号
    * @param [in] modifiedLines 有修改的行号
    * @param [in] nModifiedRows 修改后的文本，计算后切分为几行（逻辑行）
    * @param [in] deletedLines 删除的行
    * @param [in] nDeletedRows 删除了几个逻辑行
    * @param [in] rowRectTopList 每个逻辑行的top坐标，用于更新行的坐标(下标值为逻辑行，从0开始编号)
    */
    virtual bool UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>& spOldDrawRichTextCache,
                                         const std::shared_ptr<DrawRichTextCache>& spUpdateDrawRichTextCache,
                                         std::vector<RichTextData>& richTextDataNew,
                                         size_t nStartLine,
                                         const std::vector<size_t>& modifiedLines,
                                         size_t nModifiedRows,
                                         const std::vector<size_t>& deletedLines,
                                         size_t nDeletedRows,
                                         const std::vector<int32_t>& rowRectTopList) = 0;

    /** 比较两个绘制缓存的数据是否一致
    */
    virtual bool IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const = 0;

    /** 绘制RichText的缓存中的内容（绘制前，需要使用IsValidDrawRichTextCache判断缓存是否失效）
    * @param [in] spDrawRichTextCache 缓存的数据
    * @param [in] rcNewTextRect 绘制文本的矩形区域
    * @param [in] szNewScrollOffset 新的滚动条位置
    * @param [in] rowXOffset 每行的横向偏移列表（逻辑行）
    * @param [in] uFade 透明度（0 - 255）
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    */
    virtual void DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,                                       
                                       const UiRect& textRect,
                                       const UiSize& szNewScrollOffset,
                                       const std::vector<int32_t>& rowXOffset,
                                       uint8_t uFade,
                                       std::vector<std::vector<UiRect>>* pRichTextRects = nullptr) = 0;

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
                               int32_t nBlurRadius, 
                               int32_t nSpreadRadius,
                               UiColor dwColor) = 0;


    /** 分离位图
    *@return 返回位图接口，返回后由调用方管理资源（包括释放资源等）
    */
    virtual IBitmap* MakeImageSnapshot() = 0;

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

#ifdef DUILIB_BUILD_FOR_WIN
    /** 获取DC句柄，当不使用后，需要调用ReleaseDC接口释放资源
    */
    virtual HDC GetRenderDC(HWND hWnd) = 0;

    /** 释放DC资源
    * @param [in] hdc 需要释放的DC句柄
    */
    virtual void ReleaseRenderDC(HDC hdc) = 0;
#endif

public:
    /** 清除位图数据，填充指定颜色（若要使位图数据全部清零，可传入UiColor()参数）
    * @param [in] uiColor 需要填充的颜色值
    */
    virtual void Clear(const UiColor& uiColor) = 0;

    /** 清除位图指定区域的数据，填充指定颜色（若要使位图数据全部清零，可传入UiColor()参数）
    * @param [in] rcDirty 需要清除的区域
    * @param [in] uiColor 需要填充的颜色值
    */
    virtual void ClearRect(const UiRect& rcDirty, const UiColor& uiColor) = 0;

    /** 克隆一个新的对象
    */
    virtual std::unique_ptr<IRender> Clone() = 0;

    /** 读取指定矩形范围内的位图数据（数据是从目标数据复制一份）
    * @param [in] rc 在Render中的矩形范围
    * @param [in] dstPixels 读取的目标缓冲区起始地址
    * @param [in] dstPixelsLen 目标dstPixels的缓冲区长度, 长度需要满足要求：dstPixelsLen >= (rc.Width() * rc.Height() * sizeof(uint32_t))
    */
    virtual bool ReadPixels(const UiRect& rc, void* dstPixels, size_t dstPixelsLen) = 0;

    /** 将数据写入到位图中去（数据是复制一份到目标数据）
    * @param [in] srcPixels 源数据的缓冲区起始地址
    * @param [in] srcPixelsLen 源数据srcPixels缓冲区长度, 长度需要满足要求：srcPixelsLen >= (rc.Width() * rc.Height() * sizeof(uint32_t))
    * @param [in] rc 在Render中的矩形范围，将位图数据写入到此矩形内
    */
    virtual bool WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc) = 0;

    /** 将数据写入到位图中去(数据是复制一份到目标数据，仅复制绘制部分)
    * @param [in] srcPixels 源数据的缓冲区起始地址
    * @param [in] srcPixelsLen 源数据srcPixels缓冲区长度，长度需要满足要求：srcPixelsLen >= (rc.Width() * rc.Height() * sizeof(uint32_t))
    * @param [in] rc 在Render中的矩形范围
    * @param [in] rcPaint 绘制的部分矩形范围，将绘制的位图数据写入到此矩形内
    */
    virtual bool WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc, const UiRect& rcPaint) = 0;

    /** 获取当前的裁剪区域
    * @param [out] clipRects 返回裁剪区域的矩形数据，矩形区域坐标为客户区坐标
                             如果是 RenderClipType::kRect类型，容器中只有一个元素，
                             如果是RenderClipType::kRegion类型，容器中有多个元素，用于构建Region
    * @return 返回裁剪区域类型
    */
    virtual RenderClipType GetClipInfo(std::vector<UiRect>& clipRects) = 0;

    /** 判断裁剪区域是否为空(如果为空不需要绘制)
    */
    virtual bool IsClipEmpty() const = 0;

    /** 是否为空（宽度或者高度为0）
    */
    virtual bool IsEmpty() const = 0;

    /** 设置Render使用的DPI转换接口
    */
    virtual void SetRenderDpi(const IRenderDpiPtr& spRenderDpi) = 0;

    /** 绘制并刷新到屏幕（Render的实现已经与窗口关联）, 同步完成
    * @param [in] pRenderPaint 界面绘制所需的回调接口
    */
    virtual bool PaintAndSwapBuffers(IRenderPaint* pRenderPaint) = 0;

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
    virtual IPen* CreatePen(UiColor color, float fWidth = 1) = 0;

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
    * @param [in] spRenderDpi 关联的DPI转换接口
    * @param [in] platformData 平台相关的数据，Windows平台该值是窗口句柄
    * @parma [in] backendType 后台绘制方式
    */
    virtual IRender* CreateRender(const IRenderDpiPtr& spRenderDpi,
                                  void* platformData = nullptr,
                                  RenderBackendType backendType = RenderBackendType::kRaster_BackendType) = 0;

    /** 获取字体管理器接口（每个factory共享一个对象）
    */
    virtual IFontMgr* GetFontMgr() const = 0;
};

} // namespace ui

#endif // UI_RENDER_IRENDER_H_
