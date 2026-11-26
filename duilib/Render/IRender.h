#ifndef UI_RENDER_IRENDER_H_
#define UI_RENDER_IRENDER_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/UiTypes.h"
#include "duilib/Core/SharePtr.h"
#include <map>
#include <algorithm>

namespace ui 
{
class IFontMgr;

/** 字体度量信息（单位：像素，与字体大小的坐标系一致）
*/
struct DUILIB_API FontMetrics
{
    /** 基线以上的高度（负值，向上为负）
    */
    float fAscent = 0;

    /** 基线以下的高度（正值，向下为正）
    */
    float fDescent = 0;

    /** 行间距（相邻两行之间的推荐间距）
    */
    float fLeading = 0;

    /** 行高（= fDescent - fAscent + fLeading）
    */
    float fLineHeight = 0;
};

/** 字体接口
*/
class DUILIB_API IFont: public SupportWeakCallback
{
public:
    virtual ~IFont() = default;

    /** 初始化字体(内部未对字体大小做DPI自适应)
     */
    virtual bool InitFont(const UiFont& fontInfo) = 0;

    /** 获取字体名
    */
    virtual DString FontName() const = 0;

    /** 获取字体大小(字体高度)
    */
    virtual int32_t FontSize() const = 0;

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

    /** 当前字体是否支持指定的Unicode字符
    * @param [in] unicodeChar UTF32字符
    * @return 支持该字符返回true，否则返回false
    */
    virtual bool IsUnicodeCharSupported(uint32_t unicodeChar) const = 0;

    /** 获取字体所属的字体管理器
    * @return 返回字体管理器接口；如果不支持该操作，则返回nullptr
    */
    virtual IFontMgr* GetFontMgr() const { return nullptr; }

    /** 获取字体的签名（由字体名、字号与字体样式计算，用于字体相关的缓存）
    * @return 返回字体签名；如果不支持该操作，则返回0
    */
    virtual uint64_t GetFontKey() const { return 0; }

    /** 获取字体度量信息（不依赖Render/Canvas）
    * @param [out] metrics 返回字体度量信息
    * @return 成功返回true；如果不支持该操作，返回false（此时metrics内容不变）
    */
    virtual bool GetFontMetrics(FontMetrics& /*metrics*/) const { return false; }

    /** 评估文本的绘制宽度（不依赖Render/Canvas；支持字体回退）
    * @param [in] text 文本内容
    * @return 返回文本的绘制宽度；如果不支持该操作，返回0
    */
    virtual float MeasureTextWidth(const DString& /*text*/) const { return 0.0f; }

    /** 获取创建该字体时使用的字体信息
    * @param [out] fontInfo 返回字体信息
    * @return 成功返回true；如果不支持该操作，返回false（此时fontInfo内容不变）
    */
    virtual bool GetFontInfo(UiFont& /*fontInfo*/) const { return false; }
};

/** 字体回退管理器（当支持的字体无法显示字符时，会查询回退字体管理器，以正确显示文字）
*/
class DUILIB_API IFallbackFontMgr : public SupportWeakCallback
{
public:
    virtual ~IFallbackFontMgr() = default;

    /** 创建指定字体的回退字体接口
    * @param [in] pFont 当前字体接口
    * @param [in] unicodeChar UTF32字符，如果为0表示不校验字符是否支持
    * @return 返回对应的回退字体接口
    */
    virtual IFont* CreateFallbackFont(const IFont* pFont, uint32_t unicodeChar) = 0;
};

/** 字体管理器接口
*/
class DUILIB_API IFontMgr : public SupportWeakCallback
{
public:
    virtual ~IFontMgr() = default;

    /** 获取字体个数
    * @return 返回字体的个数
    */
    virtual uint32_t GetFontCount() const = 0;

    /** 获取字体名称
    * @param [in] nIndex 字体的下标值：[0, GetFontCount())
    * @return 返回字体名称；下标越界时返回空字符串
    */
    virtual DString GetFontName(uint32_t nIndex) const = 0;

    /** 判断是否含有该字体
    * @param [in] fontName 字体名称
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

    /** 设置字体回退管理器
    * @param [in] pFallbackFontMgr 字体回退管理器(生命周期由设置者来管理)
    */
    virtual void SetFallbackFontMgr(IFallbackFontMgr* pFallbackFontMgr) = 0;

    /** 获取字体回退管理器
    * @return 返回字体回退管理器，外部不应存储该指针
    */
    virtual IFallbackFontMgr* GetFallbackFontMgr() const = 0;
};

/** 位图的Alpha类型（描述像素中Alpha通道的存储方式）
*/
enum class DUILIB_API AlphaType: int
{
    kUnknown,   //未知（未初始化）
    kOpaque,    //不透明（像素中不含Alpha通道信息）
    kPremul,    //预乘：像素的颜色分量已与Alpha值相乘
    kUnpremul   //非预乘：像素的颜色分量与Alpha值相互独立
};

/** 位图接口
*/
class DUILIB_API IBitmap : public SupportWeakCallback
{
public:
    virtual ~IBitmap() = default;

    /** 从数据初始化（ARGB格式）
    @param [in] nWidth 宽度（像素）
    @param [in] nHeight 高度（像素）
    @param [in] pPixelBits 位图数据, 如果为nullptr表示窗口空位图，如果不为nullptr，其数据长度为：nHeight * nWidth * 4
    @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值；
               不等于1.0f时，位图的实际尺寸会缩放为 (nWidth, nHeight) * fImageSizeScale
    @param [in] alphaType 位图的Alpha类型
    @return 初始化成功返回true，失败返回false
    */
    virtual bool Init(uint32_t nWidth, uint32_t nHeight,
                      const void* pPixelBits, float fImageSizeScale = 1.0f,
                      AlphaType alphaType = AlphaType::kPremul) = 0;

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

    /** 判断位图是否有效（已成功初始化）
    */
    virtual bool IsValid() const = 0;

    /** 获取每行像素数据的字节数（通常为 GetWidth() * 4）
    */
    virtual uint32_t GetRowBytes() const = 0;

    /** 获取位图的Alpha类型
    */
    virtual AlphaType GetAlphaType() const = 0;

    /** 获取初始化位图时使用的图片缩放比例（1.0f 表示原值）
    */
    virtual float GetImageSizeScale() const = 0;

    /** 锁定位图数据（可读写），数据长度 = GetWidth() * GetHeight() * 4
    * @note 写入像素数据后，必须调用 UnLockPixelBits()，以便同步位图的内部状态
    */
    virtual void* LockPixelBits() = 0;

    /** 释放位图数据
    */
    virtual void UnLockPixelBits() = 0;

    /** 获取位图数据的只读指针，数据长度 = GetWidth() * GetHeight() * 4
    * @return 返回只读的位图数据；位图无效时返回nullptr
    * @note 该函数不加锁，调用后不需要调用 UnLockPixelBits()
    */
    virtual const void* GetPixelBits() const = 0;

    /** 克隆生成新的位图
    *@return 返回新生成的位图接口（所有权归调用方）
    */
    virtual std::unique_ptr<IBitmap> Clone() const = 0;
};

/** 画笔接口
*/
class DUILIB_API IPen : public SupportWeakCallback
{
public:
    virtual ~IPen() = default;

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
    * @note Skia后端只支持一种线帽样式（同时作用于线段两端），绘制时使用起始笔帽样式；
    *       结束笔帽与虚线笔帽的样式会被保存，但不参与绘制
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

    /** 线段连接处的联接样式（两条线段相交时使用）
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

    /** 线条样式（虚线类型）
    *   说明：kDashStyleDash 为虚线，kDashStyleDot 为点线，
    *        kDashStyleDashDot 为点划线，kDashStyleDashDotDot 为双点划线
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

    /** 设置自定义虚线样式（成对的"实线长度、空白长度"，像素值；按DPI缩放后使用）
    *   设置后覆盖 SetDashStyle 的效果；传入的数组会被复制，调用方无需保持数据有效
    * @param [in] intervals 虚实线段长度数组，元素个数必须为偶数
    * @param [in] count 数组元素个数；传0表示清除自定义样式（恢复为 GetDashStyle 的效果）
    */
    virtual void SetDashPattern(const float* intervals, int32_t count) = 0;

    /** 是否设置了自定义虚线样式
    */
    virtual bool HasDashPattern() const = 0;

    /** 设置是否启用抗锯齿（默认启用）
    */
    virtual void SetAntiAlias(bool bAntiAlias) = 0;

    /** 是否启用抗锯齿
    */
    virtual bool IsAntiAlias() const = 0;

    /** 设置联接处的斜接限制（仅对 kMiter_Join 生效，取值需大于等于1，默认4）
    */
    virtual void SetMiterLimit(float fMiterLimit) = 0;

    /** 获取联接处的斜接限制
    */
    virtual float GetMiterLimit() const = 0;

    /** 复制Pen对象
    */
    virtual std::unique_ptr<IPen> Clone() const = 0;
};

/** 画刷接口
*/
class DUILIB_API IBrush : public SupportWeakCallback
{
public:
    virtual ~IBrush() = default;

    /** 画刷类型
    */
    enum class BrushType
    {
        kSolid = 0,         //纯色画刷（默认）
        kLinearGradient,    //线性渐变画刷
        kRadialGradient     //径向渐变画刷
    };

    /** 复制画刷对象
    * @return 返回新生成的画刷接口（所有权归调用方）
    */
    virtual std::unique_ptr<IBrush> Clone() const = 0;

    /** 获取画刷类型
    */
    virtual BrushType GetBrushType() const = 0;

    /** 获取画刷颜色（渐变画刷时为起始颜色）
    */
    virtual UiColor GetColor() const = 0;

    /** 设置画刷为纯色画刷
    */
    virtual void SetSolid(UiColor color) = 0;

    /** 设置画刷为线性渐变：从起始颜色（GetColor）渐变到结束颜色
    * @param [in] endColor 结束颜色
    * @param [in] startPt 渐变起点（画布坐标）
    * @param [in] endPt 渐变终点（画布坐标）
    */
    virtual void SetLinearGradient(UiColor endColor, const UiPointF& startPt, const UiPointF& endPt) = 0;

    /** 设置画刷为径向渐变：从起始颜色（GetColor）渐变到结束颜色
    * @param [in] endColor 结束颜色
    * @param [in] center 渐变中心点（画布坐标）
    * @param [in] radius 渐变半径
    */
    virtual void SetRadialGradient(UiColor endColor, const UiPointF& center, float radius) = 0;
};

/** 路径接口
*/
class IMatrix;
class DUILIB_API IPath : public SupportWeakCallback
{
public:    
    virtual ~IPath() = default;

    /** 填充类型，在路径或曲线相交时如何填充形成的区域
    */
    enum class FillType 
    {
        /** 按边相交次数的奇偶性判断内部区域（奇偶填充规则）
        */
        kEvenOdd        = 0, //FillModeAlternate

        /** 按有符号边相交次数累加后的非零值判断内部区域（非零填充规则）
        */
        kWinding        = 1, //FillModeWinding

        /** 与 kEvenOdd 相同，但绘制路径外部而非内部
        */
        kInverseEvenOdd    = 2,

        /** 与 kWinding 相同，但绘制路径外部而非内部
        */
        kInverseWinding    = 3
    };

    /** 设置填充类型
    */
    virtual void SetFillType(FillType mode) = 0;

    /** 获取填充类型
    */
    virtual FillType GetFillType() const = 0;

    /** 将一行添加到此路径的当前图中
    * @param [in] x1 线条起点的 x 坐标
    * @param [in] y1 线条起点的 y 坐标
    * @param [in] x2 线条终点的 x 坐标
    * @param [in] y2 线条终点的 y 坐标
    */
    virtual void AddLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2) = 0;
    virtual void AddLine(float x1, float y1, float x2, float y2) = 0;

    /** 向此路径的当前图添加一系列连接线
    * @param [in] points 线条起点和终点的点数组, 数组中的第一个点是第一行的起点，
                         数组中的最后一个点是最后一行的终点。 
                         其他每个点都用作一行的终点，下一行的起点。
    * @param [in] count 点数组中的元素数
    */
    virtual void AddLines(const UiPoint* points, int32_t count) = 0;
    virtual void AddLines(const UiPointF* points, int32_t count) = 0;

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
    */
    virtual void AddBezier(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t x4, int32_t y4) = 0;
    virtual void AddBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) = 0;

    /** 将连接的 Bézier 样条序列添加到此路径的当前图中
    * @param [in] points 指向连接的样条的起始点、结束点和控制点数组的指针。 
    *                   第一个样条从第一个点到数组的第四个点构造，并使用第二和第三个点作为控制点。 
    *                    序列中的每个后续样条只需要另外三个点：
    *                    前一个样条的终点用作起点，序列中的下两个点是控制点，第三个点是终点。
    * @param [in] count 数组中的元素数，必须满足：count == 1 + 3 * n（n为样条个数，n >= 1）
    */
    virtual void AddBeziers(const UiPoint* points, int32_t count) = 0;
    virtual void AddBeziers(const UiPointF* points, int32_t count) = 0;

    /** 将矩形添加到此路径
    * @param [in] rect 矩形区域
    */
    virtual void AddRect(const UiRect& rect) = 0;
    virtual void AddRect(const UiRectF& rect) = 0;

    /** 将椭圆添加到此路径
    * @param [in] rect 椭圆的矩形区域 
    *             left 椭圆边框左上角的 x 坐标
    *             top  椭圆边框左上角的 y 坐标
    *             right left + 椭圆边界矩形的宽度
    *             bottom top + 椭圆边界矩形的高度
    */
    virtual void AddEllipse(const UiRect& rect) = 0;
    virtual void AddEllipse(const UiRectF& rect) = 0;

    /** 将椭圆弧添加到此路径
    * @param [in] rect 椭圆的矩形区域 
    * @param [in] startAngle 椭圆水平轴与弧线起点之间的顺时针角度（以度为单位）
    * @param [in] sweepAngle 起点 (startAngle) 和弧的终点之间的顺时针角度（以度为单位）
    */
    virtual void AddArc(const UiRect& rect, float startAngle, float sweepAngle) = 0;
    virtual void AddArc(const UiRectF& rect, float startAngle, float sweepAngle) = 0;

    /** 将多边形添加到此路径
    * @param [in] points 指定多边形顶点的点数组
    * @param [in] count 数组中的元素数
    */
    virtual void AddPolygon(const UiPoint* points, int32_t count) = 0;
    virtual void AddPolygon(const UiPointF* points, int32_t count) = 0;

    /** 对路径进行矩阵变换，可以进行旋转等操作
    * @param [in] pMatrix 矩阵接口
    */
    virtual void Transform(const IMatrix* pMatrix) = 0;

    /** 获取此路径的边界矩形
    * @param [in] pen 关联的Pen对象，可以为nullptr；不为nullptr时，边界会向外扩展线宽的一半
    * @return 返回路径的边界矩形（向外取整，确保包含路径的全部像素）
    */
    virtual UiRect GetBounds(const IPen* pen) const = 0;

    /** 关闭当前绘图
    */
    virtual void Close() = 0;

    /** 重置Path数据
    */
    virtual void Reset() = 0;

    /** 复制Path对象
    */
    virtual std::unique_ptr<IPath> Clone() const = 0;
};

/** 3x3 矩阵接口
*/
class DUILIB_API IMatrix : public SupportWeakCallback
{
public:
    virtual ~IMatrix() = default;

    /** 平移操作（设置当前矩阵为纯平移矩阵，覆盖之前的变换）
    * 注意：本接口以及下面的 Scale/Rotate/Skew 都是"设置"矩阵，不是"叠加"变换；
    *       需要组合多种变换时，请分别创建矩阵，并使用 Concat 按顺序叠加。
    * @param [in] offsetX X轴方向平移的偏移量
    * @param [in] offsetY Y轴方向平移的偏移量
    */
    virtual void Translate(float offsetX, float offsetY) = 0;

    /** 缩放操作（以原点为中心，设置当前矩阵，覆盖之前的变换）
    * @param [in] scaleX X轴方向缩放比例
    * @param [in] scaleY Y轴方向缩放比例
    */
    virtual void Scale(float scaleX, float scaleY) = 0;

    /** 缩放操作（以 (px, py) 为中心，设置当前矩阵，覆盖之前的变换）
    * @param [in] scaleX X轴方向缩放比例
    * @param [in] scaleY Y轴方向缩放比例
    * @param [in] px X轴方向的中心点
    * @param [in] py Y轴方向的中心点
    */
    virtual void Scale(float scaleX, float scaleY, float px, float py) = 0;

    /** 旋转操作（以原点为中心，设置当前矩阵，覆盖之前的变换）
    * @param [in] angle 旋转的角度，正数为顺时针操作，负数为逆时针操作
    */
    virtual void Rotate(float angle) = 0;

    /** 旋转操作，以坐标(px,py)为中心点做旋转操作（设置当前矩阵，覆盖之前的变换）
    * @param [in] angle 旋转的角度，正数为顺时针操作，负数为逆时针操作
    * @param [in] px 旋转的中心点X坐标值
    * @param [in] py 旋转的中心点Y坐标值
    */
    virtual void RotateAt(float angle, float px, float py) = 0;

    /** 错切操作（以原点为中心，设置当前矩阵，覆盖之前的变换）
    * @param [in] kx X 轴错切因子
    * @param [in] ky Y 轴错切因子
    */
    virtual void Skew(float kx, float ky) = 0;

    /** 错切操作（以 (px, py) 为中心，设置当前矩阵，覆盖之前的变换）
    * @param [in] kx X 轴错切因子
    * @param [in] ky Y 轴错切因子
    * @param [in] px X轴方向的中心点
    * @param [in] py Y轴方向的中心点
    */
    virtual void Skew(float kx, float ky, float px, float py) = 0;

    /** 重置为单位矩阵（取消所有变换）
    */
    virtual void Reset() = 0;

    /** 在当前变换的基础上叠加另一个矩阵的变换（先执行当前变换，再执行pMatrix）
    * @param [in] pMatrix 需要叠加的矩阵，不可为nullptr
    */
    virtual void Concat(const IMatrix* pMatrix) = 0;

    /** 求逆矩阵（就地取逆）
    * @return 成功返回true；矩阵不可逆时返回false，此时矩阵内容不变
    */
    virtual bool Invert() = 0;

    /** 判断是否为单位矩阵
    */
    virtual bool IsIdentity() const = 0;

    /** 对点坐标应用当前变换
    * @param [in] pt 原始点坐标
    * @return 返回变换后的点坐标
    */
    virtual UiPointF MapPoint(const UiPointF& pt) const = 0;
};

/** 渲染接口中使用的DPI转换辅助接口
*/
class DUILIB_API IRenderDpi : public SupportWeakCallback
{
public:
    virtual ~IRenderDpi() = default;

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
class DUILIB_API IRenderPaint
{
public:
    /** 虚析构函数：本类是多态基类（通过 IRenderPaint* 传递派生类对象），
    *   必须提供虚析构函数，避免通过基类指针释放派生类对象时析构不完整
    */
    virtual ~IRenderPaint() = default;

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

    /** 获取界面需要绘制的区域列表，以实现更精细的局部绘制（多个不连续的脏矩形）
    *   默认实现：退化为 GetUpdateRect 返回的单个矩形，保证兼容性
    * @param [out] rcUpdateList 返回需要绘制的矩形列表（客户区坐标），矩形之间允许重叠
    * @return 返回true表示支持局部绘制；返回false表示不支持局部绘制（需要绘制整个客户区）
    */
    virtual bool GetUpdateRects(std::vector<UiRect>& rcUpdateList) const
    {
        rcUpdateList.clear();
        UiRect rcUpdate;
        if (!GetUpdateRect(rcUpdate) || rcUpdate.IsEmpty()) {
            return false;
        }
        rcUpdateList.push_back(rcUpdate);
        return true;
    }

    /** 是否只需要重新提交窗口的后备缓冲内容（不重新绘制界面）
    *   用于窗口暴露、遮挡恢复、最小化还原等场景：此时后备缓冲的内容仍然有效，
    *   只需把缓冲内容重新提交到窗口，无需重新光栅化整棵控件树。
    *   默认返回false，保证与已有实现的兼容性。
    */
    virtual bool IsPresentOnly() const
    {
        return false;
    }
};

/** 判断是否可以只重新提交窗口的后备缓冲内容（不重新绘制界面）
* @param [in] bBackBufferValid 后备缓冲的内容是否完整有效
* @param [in] bContentDirty 界面内容是否发生了变化（需要重新绘制）
* @param [in] bHasUpdateRect 是否存在待绘制的脏区
* @return 返回true表示可以直接提交后备缓冲，跳过界面重绘
*/
inline bool CanPresentBackBufferOnly(bool bBackBufferValid, bool bContentDirty, bool bHasUpdateRect)
{
    return bBackBufferValid && !bContentDirty && !bHasUpdateRect;
}

/** 光栅操作代码
 *
 *  各模式的行为描述与限制（不同渲染后端的实现细节可能不同，但语义保持一致）：
 *   kSrcCopy   用源图像覆盖目标区域（完整支持，源图像中的Alpha通道参与混合）
 *   kDstInvert 将目标区域颜色取反（255 - dst）；源图像仅用于确定绘制区域，
 *              其像素颜色不参与运算（完整支持）
 *   kSrcInvert 源图像与目标区域做异或运算（近似实现：Skia后端使用 kXor 混合模式，
 *              其效果是按Alpha覆盖率做异或合成，并不等价于逐通道的颜色按位异或，
 *              不保证与GDI的像素级结果完全一致）
 *   kSrcAnd    仅保留目标区域覆盖范围内的源图像（近似实现：与逐位"与"运算的
 *              结果不完全等价，不保证逐像素精确）
 */
enum class DUILIB_API RopMode
{
    kSrcCopy,    //用源图像覆盖目标区域
    kDstInvert,  //目标区域颜色取反（源图像仅作为绘制区域使用）
    kSrcInvert,  //源图像与目标区域做异或运算（近似实现，见上方说明）
    kSrcAnd      //仅保留目标区域覆盖范围内的源图像（近似实现，见上方说明）
};

/** 绘制文本时的格式
*/
enum DUILIB_API DrawStringFormat
{
    TEXT_LEFT           = 0x0001,   //水平对齐方式：靠左
    TEXT_HCENTER        = 0x0002,   //水平对齐方式：居中
    TEXT_RIGHT          = 0x0004,   //水平对齐方式：靠右
    TEXT_HJUSTIFY       = 0x0008,   //水平对齐方式：两端对齐
    TEXT_HALIGN_ALL     = TEXT_LEFT | TEXT_HCENTER | TEXT_RIGHT | TEXT_HJUSTIFY,

    TEXT_TOP            = 0x0010,   //垂直对齐方式：靠上
    TEXT_VCENTER        = 0x0020,   //垂直对齐方式：居中
    TEXT_BOTTOM         = 0x0040,   //垂直对齐方式：靠下
    TEXT_VJUSTIFY       = 0x0080,   //垂直对齐方式：两端对齐
    TEXT_VALIGN_ALL     = TEXT_TOP | TEXT_VCENTER | TEXT_BOTTOM | TEXT_VJUSTIFY,

    TEXT_SINGLELINE     = 0x0100,   //单行文本
    TEXT_NOCLIP         = 0x0200,   //绘制的时候，不设置剪辑区域
    TEXT_WORD_WRAP      = 0x0400,   //自动换行（仅在IRender::DrawRichText接口支持此属性，其他文字绘制函数不支持该属性）

    TEXT_VERTICAL       = 0x0800,   //纵向绘制文本，文本绘制方向为从上到下，从右到左

    TEXT_PATH_ELLIPSIS  = 0x4000,   //如果绘制区域不足，按显示文件路径的方式，在中间加"..."省略部分文字
    TEXT_END_ELLIPSIS   = 0x8000    //如果绘制区域不足，在结尾加"..."，省略部分文字
};

/** 内联图片的信息（用于 <img> 标签）
*/
struct DUILIB_API RichTextImageInfo
{
    /** 图片的位图
    */
    std::shared_ptr<IBitmap> m_pImage;

    /** 显示宽度（像素），0表示使用位图的原始宽度
    */
    int32_t m_nWidth = 0;

    /** 显示高度（像素），0表示使用位图的原始高度
    */
    int32_t m_nHeight = 0;
};

/** 格式化文本的属性索引表：保存文本片段引用的字体、颜色与内联图片
 *
 *  RichTextData 中只保存索引（uint16_t），实际的字体对象、颜色值与图片保存在本表中，
 *  由数据提供方（RichEdit2 / RichText）与绘制缓存共同持有。
 *  这样每个文本片段不再重复保存 SharePtr<UiFontEx>（16B）与两个 UiColor（8B），
 *  避免同一字体/颜色被每个文本片段重复保存。
 */
class DUILIB_API RichTextAttrTable
{
public:
    RichTextAttrTable() = default;

    /** 添加字体（按字体属性去重）
     * @param [in] pFont 字体属性，可以为nullptr
     * @return 返回字体索引，索引从1开始；返回0表示未设置字体
     */
    uint16_t AddFont(const UiFontEx* pFont)
    {
        if (pFont == nullptr) {
            return 0;
        }
        for (size_t nIndex = 0; nIndex < m_fontList.size(); ++nIndex) {
            const UiFontEx* pExistFont = m_fontList[nIndex].get();
            if ((pExistFont != nullptr) && (*pExistFont == *pFont)) {
                return (uint16_t)(nIndex + 1);
            }
        }
        SharePtr<UiFontEx> spFontInfo(new UiFontEx);
        spFontInfo->CopyFrom(*pFont);
        m_fontList.push_back(spFontInfo);
        return (uint16_t)m_fontList.size();
    }

    /** 添加颜色（按颜色值去重）
     * @param [in] color 颜色值
     * @return 返回颜色索引，索引从1开始；返回0表示未设置颜色（空颜色）
     */
    uint16_t AddColor(const UiColor& color)
    {
        if (color.IsEmpty()) {
            return 0;
        }
        for (size_t nIndex = 0; nIndex < m_colorList.size(); ++nIndex) {
            if (m_colorList[nIndex] == color) {
                return (uint16_t)(nIndex + 1);
            }
        }
        m_colorList.push_back(color);
        return (uint16_t)m_colorList.size();
    }

    /** 添加内联图片（按图片指针与显示大小去重）
     * @param [in] pImage 图片的位图
     * @param [in] nWidth 显示宽度，0表示使用原始宽度
     * @param [in] nHeight 显示高度，0表示使用原始高度
     * @return 返回图片索引，索引从1开始；返回0表示无图片
     */
    uint16_t AddImage(const std::shared_ptr<IBitmap>& pImage, int32_t nWidth, int32_t nHeight)
    {
        if (pImage == nullptr) {
            return 0;
        }
        for (size_t nIndex = 0; nIndex < m_imageList.size(); ++nIndex) {
            const RichTextImageInfo& existImage = m_imageList[nIndex];
            if ((existImage.m_pImage == pImage) &&
                (existImage.m_nWidth == nWidth) &&
                (existImage.m_nHeight == nHeight)) {
                return (uint16_t)(nIndex + 1);
            }
        }
        RichTextImageInfo imageInfo;
        imageInfo.m_pImage = pImage;
        imageInfo.m_nWidth = nWidth;
        imageInfo.m_nHeight = nHeight;
        m_imageList.push_back(imageInfo);
        return (uint16_t)m_imageList.size();
    }

    /** 获取字体
     * @param [in] nIndex 字体索引
     * @return 返回字体属性，索引无效时返回nullptr
     */
    const UiFontEx* GetFont(uint16_t nIndex) const
    {
        if ((nIndex == 0) || ((size_t)nIndex > m_fontList.size())) {
            return nullptr;
        }
        return m_fontList[nIndex - 1].get();
    }

    /** 获取颜色
     * @param [in] nIndex 颜色索引
     * @return 返回颜色值，索引无效时返回空颜色
     */
    UiColor GetColor(uint16_t nIndex) const
    {
        if ((nIndex == 0) || ((size_t)nIndex > m_colorList.size())) {
            return UiColor();
        }
        return m_colorList[nIndex - 1];
    }

    /** 获取内联图片的信息
     * @param [in] nIndex 图片索引
     * @return 返回图片信息，索引无效时返回空信息（m_pImage为nullptr）
     */
    const RichTextImageInfo& GetImage(uint16_t nIndex) const
    {
        static const RichTextImageInfo g_emptyImageInfo;
        if ((nIndex == 0) || ((size_t)nIndex > m_imageList.size())) {
            return g_emptyImageInfo;
        }
        return m_imageList[nIndex - 1];
    }

    /** 判断两个表中的字体是否相同（用于比较绘制缓存的数据）
     */
    static bool IsFontEqual(const RichTextAttrTable& first, uint16_t nFirstIndex,
                            const RichTextAttrTable& second, uint16_t nSecondIndex)
    {
        const UiFontEx* pFirstFont = first.GetFont(nFirstIndex);
        const UiFontEx* pSecondFont = second.GetFont(nSecondIndex);
        if (pFirstFont == pSecondFont) {
            return true;
        }
        if ((pFirstFont == nullptr) || (pSecondFont == nullptr)) {
            return false;
        }
        return *pFirstFont == *pSecondFont;
    }

    /** 判断两个表中的颜色是否相同
     */
    static bool IsColorEqual(const RichTextAttrTable& first, uint16_t nFirstIndex,
                             const RichTextAttrTable& second, uint16_t nSecondIndex)
    {
        return first.GetColor(nFirstIndex) == second.GetColor(nSecondIndex);
    }

    /** 判断两个表中的图片是否相同
     */
    static bool IsImageEqual(const RichTextAttrTable& first, uint16_t nFirstIndex,
                             const RichTextAttrTable& second, uint16_t nSecondIndex)
    {
        const RichTextImageInfo& firstImage = first.GetImage(nFirstIndex);
        const RichTextImageInfo& secondImage = second.GetImage(nSecondIndex);
        return (firstImage.m_pImage == secondImage.m_pImage) &&
               (firstImage.m_nWidth == secondImage.m_nWidth) &&
               (firstImage.m_nHeight == secondImage.m_nHeight);
    }

    /** 清空所有数据
     */
    void Clear()
    {
        m_fontList.clear();
        m_colorList.clear();
        m_imageList.clear();
    }

    /** 清空并释放容量（用于需要归还内存的场景，如文本清空、控件隐藏）
    */
    void Release()
    {
        std::vector<SharePtr<UiFontEx>>().swap(m_fontList);
        std::vector<UiColor>().swap(m_colorList);
        std::vector<RichTextImageInfo>().swap(m_imageList);
    }

    /** 交换两个表的数据
     */
    void Swap(RichTextAttrTable& other)
    {
        m_fontList.swap(other.m_fontList);
        m_colorList.swap(other.m_colorList);
        m_imageList.swap(other.m_imageList);
    }

    /** 字体数量（不含索引0）
     */
    size_t GetFontCount() const { return m_fontList.size(); }

private:
    /** 字体列表（索引0不使用）
    */
    std::vector<SharePtr<UiFontEx>> m_fontList;

    /** 颜色列表（索引0不使用）
    */
    std::vector<UiColor> m_colorList;

    /** 内联图片列表（索引0不使用）
    */
    std::vector<RichTextImageInfo> m_imageList;
};

/** 格式文本数据
*/
class RichTextData
{
public:
    /** 文字内容(由外部负责保证字符串指向内存的生命周期)
    */
    std::wstring_view m_textView;

    /** 字体索引（RichTextAttrTable::AddFont 的返回值，0表示未设置字体）
    */
    uint16_t m_nFontIndex = 0;

    /** 文字颜色索引（0表示未设置颜色，使用默认颜色）
    */
    uint16_t m_nTextColorIndex = 0;

    /** 背景颜色索引（0表示无背景颜色）
    */
    uint16_t m_nBgColorIndex = 0;

    /** 内联图片索引（0表示无图片）
    */
    uint16_t m_nImageIndex = 0;

    /** 行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距
    */
    float m_fRowSpacingMul = 1.0f;

    /** 行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素）
    */
    float m_fRowSpacingAdd = 0.0f;

    /** 字间距（像素），0表示使用字体默认间距
    */
    float m_fLetterSpacing = 0.0f;

    /** 基线偏移（像素）：负数向上（上标），正数向下（下标），用于 <sup>/<sub>
    */
    int32_t m_nBaselineOffset = 0;

    /** 首行缩进（像素）：仅对段落的第一行生效
    */
    int32_t m_nFirstLineIndent = 0;

    /** 最多显示的行数（0表示不限制）
    */
    int32_t m_nMaxLines = 0;

    /** 绘制文字的属性(包含文本对齐方式等属性，参见 enum DrawStringFormat)
    */
    uint16_t m_textStyle = 0;

    /** 超出最大行数时是否显示省略号：true显示省略号，false直接裁剪
    */
    bool m_bTextOverflowEllipsis = true;

    /** 绘制缓存是否按文本的完整宽度构建（不按可视宽度裁剪）
        默认false，缓存只包含可视宽度内的内容；
        支持横向滚动的控件（如RichEdit2）需要设置为true，否则缓存不包含首屏之外的文本，
        横向滚动后新出现的区域会显示为空白
    */
    bool m_bCacheFullTextWidth = false;

    /** 是否绘制水平分割线（用于 <hr> 标签）
    */
    bool m_bHorizontalRule = false;
};

/** 格式文本的绘制数据：文本片段列表 + 字体/颜色/图片的索引表
*/
class DUILIB_API RichTextDataList
{
public:
    /** 文本片段列表
    */
    std::vector<RichTextData> m_data;

    /** 文本片段引用的字体/颜色/图片
    */
    RichTextAttrTable m_attrs;

public:
    /** 清空
    */
    void Clear()
    {
        m_data.clear();
        m_attrs.Clear();
    }

    /** 清空并释放容量（用于需要归还内存的场景，如文本清空、控件隐藏）
    */
    void Release()
    {
        std::vector<RichTextData>().swap(m_data);
        m_attrs.Release();
    }

    /** 是否为空
    */
    bool IsEmpty() const { return m_data.empty(); }

    /** 文本片段个数
    */
    size_t Size() const { return m_data.size(); }

    /** 预留容量
    */
    void Reserve(size_t nCount) { m_data.reserve(nCount); }

    /** 添加一个文本片段
    */
    void PushBack(const RichTextData& data) { m_data.push_back(data); }

    /** 交换两个数据对象（文本片段与属性索引表一起交换）
    */
    void Swap(RichTextDataList& other)
    {
        m_data.swap(other.m_data);
        m_attrs.Swap(other.m_attrs);
    }

    /** 获取文本片段
    */
    RichTextData& operator[](size_t nIndex) { return m_data[nIndex]; }
    const RichTextData& operator[](size_t nIndex) const { return m_data[nIndex]; }
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

/** 行内字符的排版信息：每个字符保存"相对本行左边界的 x 偏移量"和 4 位属性标志
*   存储方式：
*   （1）常规行：x 偏移用 uint16 保存（单位 1/8 像素，单行宽度的可表示上限为 8191 像素），约 2B/字符；
*   （2）超宽行：构建过程中一旦超出上述范围，本行自动切换为 32 位偏移（两个 uint16 保存），
*        可表示约 5.36 亿像素的行宽，**不限制单行宽度**，约 4B/字符（与压缩前相同，不产生回退）；
*   属性标志采用稀疏存储：只记录属性不为 0 的字符（4B/个），普通文本每行通常只有末尾的换行符。
*   说明：偏移量是"绝对位置"的四舍五入值，误差不随字符数累积（单个字符的误差 ≤ 1/16 像素）。
*/
class DUILIB_API RichTextCharLayout
{
public:
    /** x 偏移的存储单位：1/8 像素 */
    static constexpr uint32_t kOffsetUnit = 8;

    /** 常规行 x 偏移的最大原始值（uint16 可表示的范围） */
    static constexpr uint32_t kOffsetMaxValue = 0xFFFF;

public:
    /** 字符个数
    */
    size_t CharCount() const
    {
        return m_bWide ? (m_xOffsets.size() / 2) : m_xOffsets.size();
    }

    /** 是否为空
    */
    bool Empty() const { return m_xOffsets.empty(); }

    /** 是否已切换为超宽行存储
    */
    bool IsWideRow() const { return m_bWide; }

    /** 清空
    */
    void Clear()
    {
        m_xOffsets.clear();
        m_charFlags.clear();
        m_fLastCharWidth = 0.0f;
        m_bWide = false;
    }

    /** 预留容量
    * @param [in] nCharCount 本行的字符个数
    */
    void Reserve(size_t nCharCount)
    {
        //x 偏移必须预留：几乎每个字符都会写入
        m_xOffsets.reserve(nCharCount * (m_bWide ? 2 : 1));
        //属性标志是稀疏的（普通文本每行通常只有末尾的换行符），不做预留：
        //预留会在每一行产生一次固定大小的堆分配，抵消压缩带来的收益
    }

    /** 添加一个字符
    * @param [in] xOffset 该字符相对本行左边界的 x 偏移量（像素）
    * @param [in] charWidth 该字符的宽度（像素）
    * @param [in] charFlag 该字符的属性标志（RichTextCharFlag）
    */
    void PushBack(float xOffset, float charWidth, uint8_t charFlag)
    {
        const uint32_t nRawOffset = ToRawOffset(xOffset);
        if (!m_bWide && (nRawOffset > kOffsetMaxValue)) {
            ConvertToWideRow();
        }
        if (m_bWide) {
            m_xOffsets.push_back((uint16_t)(nRawOffset & 0xFFFF));
            m_xOffsets.push_back((uint16_t)((nRawOffset >> 16) & 0xFFFF));
        }
        else {
            m_xOffsets.push_back((uint16_t)nRawOffset);
        }
        m_fLastCharWidth = charWidth;
        if (charFlag != 0) {
            const uint32_t nIndex = (uint32_t)(CharCount() - 1);
            m_charFlags.push_back((nIndex << 8) | (uint32_t)charFlag);
        }
    }

    /** 获取字符相对本行左边界的 x 偏移量（像素）
    * @param [in] nIndex 字符下标；取值 CharCount() 时返回本行的宽度（最后一个字符的右边界）
    */
    float CharX(size_t nIndex) const
    {
        const size_t nCount = CharCount();
        if (nCount == 0) {
            return 0.0f;
        }
        if (nIndex >= nCount) {
            return ((float)GetRawOffset(nCount - 1) / (float)kOffsetUnit) + m_fLastCharWidth;
        }
        return (float)GetRawOffset(nIndex) / (float)kOffsetUnit;
    }

    /** 获取字符宽度（像素）
    */
    float CharWidth(size_t nIndex) const
    {
        const size_t nCount = CharCount();
        if (nIndex >= nCount) {
            return 0.0f;
        }
        if (nIndex + 1 >= nCount) {
            return m_fLastCharWidth;
        }
        return (float)(GetRawOffset(nIndex + 1) - GetRawOffset(nIndex)) / (float)kOffsetUnit;
    }

    /** 获取字符的属性标志
    */
    uint8_t CharFlag(size_t nIndex) const
    {
        if (m_charFlags.empty()) {
            return 0;
        }
        const uint32_t nKey = (uint32_t)nIndex;
        auto iter = std::lower_bound(m_charFlags.begin(), m_charFlags.end(), nKey,
                                     [](uint32_t nValue, uint32_t nFind) {
                                         return (nValue >> 8) < nFind;
                                     });
        if ((iter != m_charFlags.end()) && (((*iter) >> 8) == nKey)) {
            return (uint8_t)((*iter) & 0xFF);
        }
        return 0;
    }

    /** 该字符是否为回车
    */
    bool IsReturn(size_t nIndex) const { return (CharFlag(nIndex) & RichTextCharFlag::kIsReturn) != 0; }

    /** 该字符是否为换行符
    */
    bool IsNewLine(size_t nIndex) const { return (CharFlag(nIndex) & RichTextCharFlag::kIsNewLine) != 0; }

    /** 该字符是否为非绘制字符
    */
    bool IsIgnoredChar(size_t nIndex) const { return (CharFlag(nIndex) & RichTextCharFlag::kIsIgnoredChar) != 0; }

    /** 该字符是否为低代理字符
    */
    bool IsLowSurrogate(size_t nIndex) const { return (CharFlag(nIndex) & RichTextCharFlag::kIsLowSurrogate) != 0; }

    /** 比较两个字符表的内容是否相同
    */
    bool operator==(const RichTextCharLayout& r) const
    {
        return (m_bWide == r.m_bWide) &&
               (m_xOffsets == r.m_xOffsets) &&
               (m_charFlags == r.m_charFlags) &&
               (m_fLastCharWidth == r.m_fLastCharWidth);
    }
    bool operator!=(const RichTextCharLayout& r) const { return !((*this) == r); }

private:
    /** 像素值转换为内部存储的原始偏移值
    */
    uint32_t ToRawOffset(float xOffset) const
    {
        //防御异常输入：浮点负值转换为无符号整数属于未定义行为，统一按 0 处理
        if (!(xOffset > 0.0f)) { //同时处理 NaN 的情况
            return 0;
        }
        const float fValue = (xOffset * (float)kOffsetUnit) + 0.5f;
        if (fValue >= 4294967040.0f) { //防止超出 uint32 的表示范围（float 在该值以上无法精确表示）
            return 0xFFFFFFFF;
        }
        return (uint32_t)fValue;
    }

    /** 获取内部存储的原始偏移值
    */
    uint32_t GetRawOffset(size_t nIndex) const
    {
        if (m_bWide) {
            return (uint32_t)m_xOffsets[nIndex * 2] | ((uint32_t)m_xOffsets[nIndex * 2 + 1] << 16);
        }
        return m_xOffsets[nIndex];
    }

    /** 切换为超宽行的存储方式（每个字符使用 32 位偏移，每行最多触发一次）
    */
    void ConvertToWideRow()
    {
        const size_t nCount = CharCount();
        std::vector<uint16_t> xOffsets;
        xOffsets.resize(nCount * 2);
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            const uint32_t nRawOffset = m_xOffsets[nIndex];
            xOffsets[nIndex * 2] = (uint16_t)(nRawOffset & 0xFFFF);
            xOffsets[nIndex * 2 + 1] = (uint16_t)((nRawOffset >> 16) & 0xFFFF);
        }
        m_xOffsets.swap(xOffsets);
        m_bWide = true;
    }

private:
    /** 字符的 x 偏移量（常规行：1个 uint16/字符；超宽行：2个 uint16/字符）
    */
    std::vector<uint16_t> m_xOffsets;

    /** 字符的属性标志，按字符下标升序排列，每个元素为：(字符下标 << 8) | 属性标志
    */
    std::vector<uint32_t> m_charFlags;

    /** 最后一个字符的宽度（像素），用于计算最后一个字符的宽度和行宽
    */
    float m_fLastCharWidth = 0.0f;

    /** 是否已切换为超宽行存储
    */
    bool m_bWide = false;
};

/** 逻辑行(矩形区域内显示的行，物理行数据在自动换行的情况下会对应多个逻辑行)的基本信息
*/
struct RichTextRowInfo
{
    /** 本行中的字符个数，字符属性（紧凑存储：每个字符只保存行内 x 偏移与稀疏属性位）
    */
    RichTextCharLayout m_charInfo;

    /** 该行的文字所占矩形区域
    */
    UiRectF m_rowRect;

    /** 本行的left坐标偏移量（用于支持居中和靠右对齐）
    */
    int32_t m_xOffset = 0;
};

/** 物理行的文本数据：共享整篇文本的存储，只记录本行文本在整篇文本中的偏移量
*   说明：为避免每行都分配一份字符串，行文本统一共享整篇文本的存储；
*        行文本不保证以'\0'结尾（长度由 RichTextLineInfo::m_nLineTextLen 记录），
*        因此不提供 c_str() 接口，使用方只能通过 data() + 长度访问，避免误用。
*/
class DUILIB_API RichTextLineText
{
public:
    RichTextLineText() = default;
    ~RichTextLineText() = default;
    RichTextLineText(const RichTextLineText&) = default;
    RichTextLineText& operator=(const RichTextLineText&) = default;

    /** 共享整篇文本的存储，并指定本行文本的起始偏移量
    * @param [in] spText 整篇文本的存储（生命周期由共享指针保证）
    * @param [in] nOffset 本行文本在整篇文本中的起始偏移量
    */
    void SetShareText(const std::shared_ptr<const UiStringW>& spText, uint32_t nOffset)
    {
        m_spText = spText;
        m_nOffset = nOffset;
    }

    /** 设置本行文本的内容（文本内容复制一份，独立存储）
    */
    RichTextLineText& operator=(const std::wstring_view& textView)
    {
        m_spText = std::make_shared<UiStringW>(DStringW(textView));
        m_nOffset = 0;
        return *this;
    }

    /** 本行文本数据的起始地址（不保证以'\0'结尾，长度由调用方记录）
    */
    const DStringW::value_type* data() const
    {
        if (m_spText == nullptr) {
            return L"";
        }
        return m_spText->data() + m_nOffset;
    }

private:
    /** 整篇文本的存储（各行文本共享使用）
    */
    std::shared_ptr<const UiStringW> m_spText;

    /** 本行文本在整篇文本中的起始偏移量
    */
    uint32_t m_nOffset = 0;
};

/** 物理行文本的数据
*/
struct RichTextLineInfo: public NVRefCount<RichTextLineInfo>
{
    /** 文本数据长度
    */
    uint32_t m_nLineTextLen = 0;

    /** 文本数据（共享整篇文本的存储，只记录偏移量）
    */
    RichTextLineText m_lineText;

    /** 逻辑行的基本信息（按值保存，避免每个逻辑行一次堆分配与指针开销）
    */
    std::vector<RichTextRowInfo> m_rowInfo;

    /** 获取本行文本的视图
    */
    std::wstring_view GetLineTextView() const
    {
        return std::wstring_view(m_lineText.data(), m_nLineTextLen);
    }
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

/** 绘制缓存的构建窗口：只构建可视区域附近的行，避免为全文建立绘制数据
*/
struct RichTextDrawWindow
{
    /** 窗口对应的第一个逻辑行号（全局行号） */
    uint32_t m_nFirstRowIndex = 0;

    /** 窗口对应的第一个物理行号 */
    uint32_t m_nFirstLineNumber = 0;

    /** 窗口第一行的顶部Y坐标（内容坐标，以文本绘制区域的左上角为原点） */
    int32_t m_nFirstRowTop = 0;

    /** 窗口的内容底部Y坐标：构建位置到达该值后停止（内容坐标）
        （用于限制单行文本被折行成很多逻辑行时的构建范围） */
    int32_t m_nBottom = 0;

    /** 全文逻辑行总数（用于判断文本的最后一行） */
    uint32_t m_nTotalRowCount = 0;
};

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

/** GL绘制方式下，是否默认禁用垂直同步（true表示禁用，false表示启用）
*   注意：这是全局唯一的默认值定义，所有涉及vsync默认值的位置都必须引用该常量，
*   后续调整默认vsync行为时只需修改这里
*/
constexpr bool kDefaultDisableVsync = true;

/** 计算指定文本字符串的宽度和高度使用的参数
*/
struct MeasureStringParam
{
    int32_t limitWidth = -1;            //横向文本的限制宽度（像素），-1表示不限制
                                        //多行文本：应设置合理值，如果不设置则文本不会自动换行；单行文本：忽略该值
    int32_t limitHeight = -1;           //纵向文本的限制高度（像素），-1表示不限制
                                        //多行文本：应设置合理值，如果不设置则文本不会自动换行；单行文本：忽略该值

    IFont* pFont = nullptr;             //文字的字体数据接口, 不可为nullptr
    uint32_t uFormat = 0;               //文字的格式，参见 enum DrawStringFormat 类型定义
    float fSpacingMul = 1.0f;           //行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距
    float fSpacingAdd = 0;              //行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素）
    float fLetterSpacing = 0;           //设置两个相邻的字符之间的间隔（像素），与RichTextData::m_fLetterSpacing语义一致

    bool bUseFontHeight = true;         //纵向绘制时，使用字体的默认高度，而不是每个字体的高度（显示时所有字体等高）
    bool bRotate90ForAscii = true;      //纵向绘制时，对于字母数字等，旋转90度显示
};

/** 绘制文字使用的参数
*/
struct DrawStringParam
{
    UiRect textRect;        //文字绘制的矩形区域
    UiColor dwTextColor;    //文字颜色值
    uint8_t uFade = 255;    //文字的透明度[0 - 255]，与文字颜色的Alpha值相乘

    IFont* pFont = nullptr;             //文字的字体
    uint32_t uFormat = 0;               //文字的格式，参见 enum DrawStringFormat 类型定义    
    float fSpacingMul = 1.0f;           //行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距
    float fSpacingAdd = 0;              //行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素）
    float fLetterSpacing = 0;           //设置两个相邻的字符之间的间隔（像素），与RichTextData::m_fLetterSpacing语义一致

    bool bUseFontHeight = true;         //纵向绘制时，使用字体的默认高度，而不是每个字体的高度（显示时所有字体等高）
    bool bRotate90ForAscii = true;      //纵向绘制时，对于字母数字等，旋转90度显示
};

/** 图片缩放时的采样质量
*/
enum class ImageSamplingQuality
{
    /** 最近邻采样：速度最快，图片缩放时锯齿明显（默认值，保持原有绘制效果）
    */
    kNearest = 0,

    /** 双线性采样：质量与速度均衡，适合大多数缩放场景
    */
    kLinear = 1,

    /** 三次卷积采样：质量最好，速度最慢，适合高质量缩略图
    */
    kCubic = 2,
};

/** 绘制图片的参数（除位图、目标区域、源区域之外的可选参数）
*/
struct ImageDrawParam
{
    /** 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    uint8_t uFade = 255;

    /** 九宫格绘制的边距（目标区域与源图片共用同一组边距）
    *   四个值分别代表四条边的内缩量，不代表矩形区域；
    *   四个值都为0时，表示按普通方式拉伸绘制
    */
    UiMargin rcImageBorder;

    /** 平铺绘制相关参数，不平铺绘制时可传nullptr（仅对九宫格绘制的中间区域有效）
    */
    const TiledDrawParam* pTiledDrawParam = nullptr;

    /** 九宫格绘制时，不绘制中间部分（比如窗口阴影，只需要绘制边框，不需要绘制中间部分）
    *   仅当 rcImageBorder 不全为0时有效
    */
    bool bWindowShadowMode = false;

    /** 绘制时的变换矩阵，nullptr表示不使用矩阵（与九宫格、平铺、窗口阴影模式互斥）
    */
    const IMatrix* pMatrix = nullptr;

    /** 当前全部可绘制区域（可选，仅用于避免非可绘制区域的绘制，以提高绘制性能）。
    *   传 UiRect() 表示不限制，此时渲染接口使用当前裁剪区域兜底；
    *   传值时需覆盖本图片所有可能被绘制的区域，否则会漏画
    */
    UiRect rcPaint;
};

/** 混合模式（用于整组绘制时的合成效果）
*/
enum class BlendMode
{
    kSrcOver = 0,   //源覆盖目标（默认）
    kClear,         //清除目标区域
    kMultiply,      //正片叠底
    kScreen,        //滤色
    kXor,           //异或
    kPlus           //相加（提亮）
};

/** 渲染接口
*/
class IRenderFactory;
class DUILIB_API IRender : public SupportWeakCallback
{
public:
    virtual ~IRender() = default;

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
     * @return 返回原来的视区原点坐标(x,y)
     * @note 视区原点用于滚动等场景：设置后，所有绘制接口传入的客户区坐标都会整体平移，
     *       使客户区坐标 pt 显示在设备坐标 (pt - 视区原点) 处
     */
    virtual UiPoint OffsetWindowOrg(UiPoint ptOffset) = 0;

    /** 将点(pt.x, pt.y)映射到视区原点 (0, 0)
     * @return 返回原来的视区原点坐标(x,y)
     */
    virtual UiPoint SetWindowOrg(UiPoint pt) = 0;

    /** 获取视区原点坐标(x,y)
     * @return 返回当前的视区原点坐标(x,y)
     */
    virtual UiPoint GetWindowOrg() const = 0;

    /** 设置画布的变换矩阵（替换当前变换）
    * @param [in] pMatrix 变换矩阵；传nullptr表示恢复为单位矩阵
    */
    virtual void SetTransform(const IMatrix* pMatrix) = 0;

    /** 在当前画布变换的基础上叠加一个变换矩阵（先执行当前变换，再执行pMatrix）
    * @param [in] pMatrix 变换矩阵；传nullptr时不做任何操作
    */
    virtual void ConcatMatrix(const IMatrix* pMatrix) = 0;

    /** 设置后续绘制使用的混合模式（默认 kSrcOver）
    * @param [in] blendMode 混合模式
    */
    virtual void SetBlendMode(BlendMode blendMode) = 0;

    /** 获取当前绘制使用的混合模式
    */
    virtual BlendMode GetBlendMode() const = 0;
    
    /** 保存当前设备上下文的状态（包括剪辑区域、变换矩阵等）
    * @param [out] nState 返回保存的状态标志，在调用 RestoreClip 时作为参数传入
    */
    virtual void SaveClip(int32_t& nState) = 0;

    /** 将设备上下文还原到指定标志对应的状态（清除该状态之后设置的剪辑区域）
    * @param [in] nState 状态标志（由 SaveClip / ClipRect / ClipRoundRect / ClipPath 返回）
    * @note 支持多层嵌套：可以传回任意一次保存时返回的标志，此时该标志之后的所有保存状态都会被丢弃；
    *       标志无效（<=0）时不做任何处理
    */
    virtual void RestoreClip(int32_t nState) = 0;

    /** 设置矩形剪辑区域，并保存当前设备上下文的状态
    * @param [in] rc 剪辑区域（客户区坐标，会按视区原点换算到设备坐标）
    * @param [in] bIntersect ClipOp操作标志，true表示与当前剪辑区取交集（kIntersect），false表示从当前剪辑区中减去该区域（kDifference）
    * @return 返回当前设备上下文标志，在调用RestoreClip的时候需要传回该返回值；画布不可用时返回-1
    * @note 传入空矩形时，交集操作得到的剪辑区域为空（不会绘制任何内容），但依然返回有效的状态标志
    */
    virtual int32_t ClipRect(const UiRect& rc, bool bIntersect = true) = 0;

    /** 设置圆角矩形剪辑区域，并保存当前设备上下文的状态
    * @param [in] rcItem 剪辑区域（客户区坐标，会按视区原点换算到设备坐标）
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的的高度
    * @param [in] bIntersect ClipOp操作标志，true表示与当前剪辑区取交集（kIntersect），false表示从当前剪辑区中减去该区域（kDifference）
    * @return 返回当前设备上下文标志，在调用RestoreClip的时候需要传回该返回值；画布不可用时返回-1
    */
    virtual int32_t ClipRoundRect(const UiRect& rcItem, float rx, float ry, bool bIntersect = true) = 0;

    /** 设置路径剪辑区域，并保存当前设备上下文的状态（用于圆形头像、异形卡片等非矩形裁剪）
    * @param [in] path 剪辑路径（客户区坐标，会按视区原点换算到设备坐标）；不可为nullptr
    * @param [in] bIntersect ClipOp操作标志，true表示与当前剪辑区取交集（kIntersect），false表示从当前剪辑区中减去该区域（kDifference）
    * @return 返回当前设备上下文标志，在调用RestoreClip的时候需要传回该返回值；参数无效（path为nullptr或类型不匹配）或画布不可用时返回-1
    */
    virtual int32_t ClipPath(const IPath* path, bool bIntersect = true) = 0;

    /** 保存当前设备上下文状态，并启用离屏图层（用于整组淡出/离屏合成）
    *   图层内容绘制完成后，需调用 RestoreLayer 把图层合成到画布（按 alpha 整体淡化）
    * @param [in] rc 图层的矩形区域，传 UiRect() 表示整个画布
    * @param [in] alpha 图层合成时的整体透明度（0 - 255）
    */
    virtual void SaveLayer(const UiRect& rc, uint8_t alpha = 255) = 0;

    /** 结束离屏图层，并把图层内容合成到画布（与 SaveLayer 配对使用）
    */
    virtual void RestoreLayer() = 0;

    /** 函数执行与从指定源设备上下文到目标设备上下文中的像素矩形对应的颜色数据的位块传输
    * @param [in] x 目标矩形左上角的 x 坐标
    * @param [in] y 目标矩形左上角的 y 坐标
    * @param [in] cx 源矩形和目标矩形的宽度
    * @param [in] cy 源和目标矩形的高度
    * @param [in] pSrcRender 源Render对象
    * @param [in] xSrc 源矩形左上角的 x 坐标
    * @param [in] ySrc 源矩形左上角的 y 坐标
    * @param [in] rop 光栅操作代码
    */
    virtual bool BitBlt(int32_t x, int32_t y, int32_t cx, int32_t cy,
                        const IRender* pSrcRender, int32_t xSrc, int32_t ySrc,
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
                            const IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc,
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

    /** 绘制图片（将整个源图片绘制到目标区域）
    * @param [in] pBitmap 用于绘制的位图接口
    * @param [in] rcDest 绘制的目标区域
    */
    virtual void DrawImage(const IBitmap* pBitmap, const UiRect& rcDest) = 0;

    /** 绘制图片
    * @param [in] pBitmap 用于绘制的位图接口
    * @param [in] rcDest 绘制的目标区域
    * @param [in] rcSource 绘制的源图片区域
    * @param [in] drawParam 绘制参数（九宫格、平铺、变换矩阵、透明度、脏区等），按需设置
    */
    virtual void DrawImage(const IBitmap* pBitmap,
                           const UiRect& rcDest, const UiRect& rcSource,
                           const ImageDrawParam& drawParam = ImageDrawParam()) = 0;

    /** 绘制直线
    * @param [in] pt1 起始点坐标
    * @param [in] pt2 终止点坐标
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    */
    virtual void DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth) = 0;

    /** 绘制直线，支持各种线形
    * @param [in] pt1 起始点坐标
    * @param [in] pt2 终止点坐标
    * @param [in] pen 画笔的接口
    */
    virtual void DrawLine(const UiPointF& pt1, const UiPointF& pt2, IPen* pen) = 0;

    /** 绘制矩形
    * @param [in] rc 矩形区域
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    * @param [in] bLineInRect 如果为true，表示确保画出的线条严格限制在rc矩形内部，否则线的中心点是与rc边线对齐的，线条会有部分超出rc矩形范围
    */
    virtual void DrawRect(const UiRectF& rc, UiColor penColor, float fWidth, bool bLineInRect = false) = 0;

    /** 绘制矩形，支持各种线形
    * @param [in] rc 矩形区域
    * @param [in] pen 画笔的接口
    * @param [in] bLineInRect 如果为true，表示确保画出的线条严格限制在rc矩形内部，否则线的中心点是与rc边线对齐的，线条会有部分超出rc矩形范围
    */
    virtual void DrawRect(const UiRectF& rc, IPen* pen, bool bLineInRect = false) = 0;

    /** 用颜色填充矩形
    * @param [in] rc 目标矩形区域
    * @param [in] dwColor 颜色值
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    virtual void FillRect(const UiRectF& rc, UiColor dwColor, uint8_t uFade = 255) = 0;

    /** 用渐变颜色填充矩形（支持渐变颜色）
    * @param [in] rc 目标矩形区域
    * @param [in] dwColor 第一颜色值
    * @param [in] dwColor2 第二颜色值
    * @param [in] nColor2Direction 渐变颜色的渐变方向，"1": 左->右，"2": 上->下，"3": 左上->右下，"4": 右上->左下
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    virtual void FillRect(const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) = 0;

    /** 绘制圆角矩形
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    */
    virtual void DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, float fWidth) = 0;

    /** 绘制圆角矩形，支持各种线形
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] pen 画笔的接口
    */
    virtual void DrawRoundRect(const UiRectF& rc, float rx, float ry, IPen* pen) = 0;

    /** 用颜色填充圆角矩形
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] dwColor 颜色值
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    virtual void FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, uint8_t uFade = 255) = 0;

    /** 用颜色填充圆角矩形(支持渐变颜色)
    * @param [in] rc 矩形区域
    * @param [in] rx 圆角的宽度
    * @param [in] ry 圆角的高度
    * @param [in] dwColor 颜色值
    * @param [in] dwColor2 第二颜色值
    * @param [in] nColor2Direction 渐变颜色的渐变方向，"1": 左->右，"2": 上->下，"3": 左上->右下，"4": 右上->左下
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    virtual void FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) = 0;

    /** 绘制曲线（椭圆的一部分）
    * @param [in] rc 包含圆弧的椭圆的矩形边界区域
    * @param [in] startAngle  x轴与弧起点之间的角度
    * @param [in] sweepAngle  圆弧起点和终点之间的角度，正数是顺时针方向，负数是逆时针方向
    * @param [in] useCenter 如果为true，则绘制扇形（包含椭圆的中心点）；否则只绘制弧线
    * @param [in] pen 画笔的接口，设置画笔颜色和画笔宽度
    * @param [in] gradientColor 可选参数，渐变颜色
    * @param [in] gradientRect 可选参数，渐变颜色的矩形区域设置，仅当gradientColor不为nullptr时有效
    */
    virtual void DrawArc(const UiRectF& rc, float startAngle, float sweepAngle, bool useCenter, 
                         const IPen* pen, 
                         const UiColor* gradientColor = nullptr, const UiRectF* gradientRect = nullptr) = 0;

    /** 填充圆弧（椭圆的一部分）
    * @param [in] rc 包含圆弧的椭圆的矩形边界区域
    * @param [in] startAngle x轴与弧起点之间的角度
    * @param [in] sweepAngle 圆弧起点和终点之间的角度，正数是顺时针方向，负数是逆时针方向
    * @param [in] useCenter 如果为true，则填充扇形（包含椭圆的中心点）；否则填充弦与弧围成的区域
    * @param [in] dwColor 填充颜色值
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    virtual void FillArc(const UiRectF& rc, float startAngle, float sweepAngle, bool useCenter,
                         UiColor dwColor, uint8_t uFade = 255) = 0;

    /** 绘制圆形
    * @param [in] centerPt 圆心坐标点
    * @param [in] radius 圆的半径
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    */
    virtual void DrawCircle(const UiPointF& centerPt, float radius, UiColor penColor, float fWidth) = 0;

    /** 绘制圆形，支持各种线形
    * @param [in] centerPt 圆心坐标点
    * @param [in] radius 圆的半径
    * @param [in] pen 画笔的接口
    */
    virtual void DrawCircle(const UiPointF& centerPt, float radius, IPen* pen) = 0;

    /** 填充圆形
    * @param [in] centerPt 圆心坐标点
    * @param [in] radius 圆的半径
    * @param [in] dwColor 颜色值
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    virtual void FillCircle(const UiPointF& centerPt, float radius, UiColor dwColor, uint8_t uFade = 255) = 0;

    /** 绘制椭圆
    * @param [in] rc 椭圆的矩形边界区域
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    */
    virtual void DrawEllipse(const UiRectF& rc, UiColor penColor, float fWidth) = 0;

    /** 绘制椭圆，支持各种线形
    * @param [in] rc 椭圆的矩形边界区域
    * @param [in] pen 画笔的接口
    */
    virtual void DrawEllipse(const UiRectF& rc, const IPen* pen) = 0;

    /** 填充椭圆
    * @param [in] rc 椭圆的矩形边界区域
    * @param [in] dwColor 颜色值
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    virtual void FillEllipse(const UiRectF& rc, UiColor dwColor, uint8_t uFade = 255) = 0;

    /** 绘制多边形（按顶点顺序连接并闭合）
    * @param [in] points 顶点坐标数组
    * @param [in] count 顶点个数，必须大于等于3
    * @param [in] penColor 画笔的颜色值
    * @param [in] fWidth 画笔的宽度
    */
    virtual void DrawPolygon(const UiPointF* points, int32_t count, UiColor penColor, float fWidth) = 0;

    /** 绘制多边形（按顶点顺序连接并闭合），支持各种线形
    * @param [in] points 顶点坐标数组
    * @param [in] count 顶点个数，必须大于等于3
    * @param [in] pen 画笔的接口
    */
    virtual void DrawPolygon(const UiPointF* points, int32_t count, const IPen* pen) = 0;

    /** 填充多边形
    * @param [in] points 顶点坐标数组
    * @param [in] count 顶点个数，必须大于等于3
    * @param [in] dwColor 颜色值
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    */
    virtual void FillPolygon(const UiPointF* points, int32_t count, UiColor dwColor, uint8_t uFade = 255) = 0;

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
    virtual void FillPath(const IPath* path, const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) = 0;

    /** 计算指定文本字符串的宽度和高度
    * @param [in] strText 文字内容
    * @param [in] measureParam 评估相关的参数
    * @return 返回文本字符串的宽度和高度，以矩形表示结果
    */
    virtual UiRect MeasureString(const DString& strText, const MeasureStringParam& measureParam) = 0;

    /** 计算单行文本的宽度和高度（默认格式：单行、不换行、不限制宽高）
    * @param [in] strText 文字内容
    * @param [in] pFont 文字的字体数据接口，不可为nullptr
    * @param [in] uFormat 文字的格式，参见 enum DrawStringFormat 类型定义
    * @return 返回文本字符串的宽度和高度，以矩形表示结果
    */
    virtual UiRect MeasureString(const DString& strText, IFont* pFont, uint32_t uFormat = 0) = 0;

    /** 绘制文字
    * @param [in] strText 文字内容
    * @param [in] drawParam 文字绘制相关的参数
    */
    virtual void DrawString(const DString& strText, const DrawStringParam& drawParam) = 0;

    /** 计算格式文本的宽度和高度，可选输出每个字符的位置与绘制缓存
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] richTextData 格式化文字内容
    * @param [in,out] pLineInfoParam 如果不为nullptr，则计算每个字符的区域
    * @param [out] spDrawRichTextCache 如果不为nullptr，则返回绘制缓存
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    * @return 返回文字的宽度和高度（宽度为内容的最大行宽，高度为内容总高度）
    */
    virtual UiRect MeasureRichText(const UiRect& textRect,
                                   const UiSize& szScrollOffset,
                                   const RichTextDataList& richTextData,
                                   RichTextLineInfoParam* pLineInfoParam = nullptr,
                                   std::shared_ptr<DrawRichTextCache>* spDrawRichTextCache = nullptr,
                                   std::vector<std::vector<UiRect>>* pRichTextRects = nullptr) = 0;

    /** 绘制格式文本
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] richTextData 格式化文字内容
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    */
    virtual void DrawRichText(const UiRect& textRect,
                              const UiSize& szScrollOffset,
                              const RichTextDataList& richTextData,
                              uint8_t uFade = 255,
                              std::vector<std::vector<UiRect>>* pRichTextRects = nullptr) = 0;

    /** 创建RichText的绘制缓存
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] richTextData 格式化文字内容
    * @param [out] spDrawRichTextCache 返回绘制缓存
    */
    virtual bool CreateDrawRichTextCache(const UiRect& textRect,
                                         const UiSize& szScrollOffset,
                                         const RichTextDataList& richTextData,
                                         std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache) = 0;

    /** 创建RichText的绘制缓存（只构建指定物理行窗口内的行，用于按可视区域懒构建缓存）
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] szScrollOffset 绘制文本的矩形区域所占的滚动条位置
    * @param [in] richTextData 窗口内的格式化文字内容（与 drawWindow 指定的物理行范围对应）
    * @param [in] drawWindow 构建窗口的参数
    * @param [out] spDrawRichTextCache 返回绘制缓存
    * @return 返回true表示创建成功
    */
    virtual bool CreateDrawRichTextCacheInRange(const UiRect& textRect,
                                                const UiSize& szScrollOffset,
                                                const RichTextDataList& richTextData,
                                                const RichTextDrawWindow& drawWindow,
                                                std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
    {
        //默认实现：忽略窗口参数，构建全部行（保证接口兼容性）
        UNUSED_VARIABLE(drawWindow);
        return CreateDrawRichTextCache(textRect, szScrollOffset, richTextData, spDrawRichTextCache);
    }

    /** 判断RichText的绘制缓存是否有效
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] richTextData 格式化文字内容，返回文字绘制的区域
    * @param [out] spDrawRichTextCache 返回绘制缓存
    * @param [in] bCompareTextColor 是否比较文字颜色；为false时忽略颜色差异（颜色在绘制时覆盖，
    *             用于悬停/按下等仅改变颜色的场景，避免重新排版）
    */
    virtual bool IsValidDrawRichTextCache(const UiRect& textRect,
                                          const RichTextDataList& richTextData,
                                          const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                          bool bCompareTextColor = true) = 0;

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
                                         RichTextDataList& richTextDataNew,
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
    * @param [in] uFade 透明度（0 - 255），与颜色值（或图像）自身的Alpha相乘
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    * @param [in] pTextColors 每个数据绘制时使用的文字颜色（下标与richTextData对应）；
    *             不为nullptr时覆盖缓存中的颜色，用于悬停/按下等仅改变颜色的场景
    */
    virtual void DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                       const UiRect& textRect,
                                       const UiSize& szNewScrollOffset,
                                       const std::vector<int32_t>& rowXOffset,
                                       uint8_t uFade,
                                       std::vector<std::vector<UiRect>>* pRichTextRects = nullptr,
                                       const std::vector<UiColor>* pTextColors = nullptr) = 0;

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
    * @param [in] uAlpha 透明度（0 - 255），与阴影颜色的Alpha值相乘
    */
    virtual void DrawBoxShadow(const UiRect& rc, 
                               const UiSize& roundSize, 
                               const UiPoint& cpOffset, 
                               int32_t nBlurRadius, 
                               int32_t nSpreadRadius,
                               UiColor dwColor,
                               uint8_t uAlpha) = 0;


    /** 分离位图
    * @return 返回位图接口，返回后由调用方管理资源（包括释放资源等）
    */
    virtual IBitmap* MakeImageSnapshot() = 0;

    /** 分离指定区域的位图（用于局部缓存）
    * @param [in] rc 需要分离的矩形区域（画布坐标），超出画布的部分会被裁剪
    * @return 返回位图接口，返回后由调用方管理资源；区域无效时返回nullptr
    */
    virtual IBitmap* MakeImageSnapshot(const UiRect& rc) = 0;

    /** 将矩形区域内的图像Alpha设定为指定值alpha(0 - 255)，同时清除图像数据，也设置为alpha
    * @param [in] rcDirty 矩形区域（超出图像范围的部分会被裁剪）
    * @param [in] alpha 需要设定的Alpha值（0 - 255），图像数据（RGB通道）同时被清除为该值
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

public:
    /** 清除位图数据，填充指定颜色（若要使位图数据全部清零，可传入UiColor()参数）
    * @param [in] uiColor 需要填充的颜色值（非预乘格式，内部会按画布的Alpha类型做预乘处理）
    */
    virtual void Clear(const UiColor& uiColor) = 0;

    /** 清除位图指定区域的数据，填充指定颜色（若要使位图数据全部清零，可传入UiColor()参数）
    * @param [in] rcDirty 需要清除的区域（客户区坐标）
    * @param [in] uiColor 需要填充的颜色值（非预乘格式，内部会按画布的Alpha类型做预乘处理）
    */
    virtual void ClearRect(const UiRect& rcDirty, const UiColor& uiColor) = 0;

    /** 克隆一个新的对象
    */
    virtual std::unique_ptr<IRender> Clone() const = 0;

    /** 读取指定矩形范围内的位图数据（数据是从目标数据复制一份）
    * @param [in] rc 在Render中的矩形范围（客户区坐标，会按视区原点换算到设备坐标）；
    *                必须完全位于画布内，否则返回false
    * @param [in] dstPixels 读取的目标缓冲区起始地址
    * @param [in] dstPixelsLen 目标dstPixels的缓冲区长度, 长度需要满足要求：dstPixelsLen >= (rc.Width() * rc.Height() * sizeof(uint32_t))
    * @return 读取成功返回true；区域超出画布、画布不可用或缓冲区长度不足时返回false
    */
    virtual bool ReadPixels(const UiRect& rc, void* dstPixels, size_t dstPixelsLen) = 0;

    /** 将数据写入到位图中去（数据是复制一份到目标数据）
    * @param [in] srcPixels 源数据的缓冲区起始地址
    * @param [in] srcPixelsLen 源数据srcPixels缓冲区长度, 长度需要满足要求：srcPixelsLen == (rc.Width() * rc.Height() * sizeof(uint32_t))
    * @param [in] rc 在Render中的矩形范围（客户区坐标），将位图数据写入到此矩形内，rc.Width()代表图像数据的宽度，rc.Height()代表图像数据的高度；
    *                必须完全位于画布内，否则返回false
    * @return 写入成功返回true；区域超出画布、画布不可用或缓冲区长度不匹配时返回false
    */
    virtual bool WritePixels(const void* srcPixels, size_t srcPixelsLen, const UiRect& rc) = 0;

    /** 将数据写入到位图中去(数据是复制一份到目标数据，仅复制绘制部分)
    * @param [in] srcPixels 源数据的缓冲区起始地址
    * @param [in] srcPixelsLen 源数据srcPixels缓冲区长度，长度需要满足要求：srcPixelsLen == (rc.Width() * rc.Height() * sizeof(uint32_t))
    * @param [in] rc 在Render中的矩形范围（客户区坐标），rc.Width()代表图像数据的宽度，rc.Height()代表图像数据的高度；
    *                可以部分位于画布外（例如容器通过平移视区原点实现滚动时，控件可能只有部分可见），
    *                超出画布的部分会被裁剪，不会导致写入失败
    * @param [in] rcPaint 绘制的部分矩形范围（与rc同一坐标系），代表脏区域，仅写入rcPaint与rc交集的部分区域；
    *                     该区域超出画布的部分同样会被裁剪
    * @return 写入成功返回true（脏区域全部位于画布外时，没有需要写入的数据，也返回true）；
    *         画布不可用或缓冲区长度不匹配时返回false
    */
    virtual bool WritePixels(const void* srcPixels, size_t srcPixelsLen, const UiRect& rc, const UiRect& rcPaint) = 0;

    /** 获取当前的裁剪区域
    * @param [out] clipRects 返回裁剪区域的矩形数据，矩形区域坐标为客户区坐标（可直接用于绘制代码）
                             如果是 RenderClipType::kRect类型，容器中只有一个元素，
                             如果是RenderClipType::kRegion类型，容器中有多个元素，用于构建Region
    * @return 返回裁剪区域类型；没有设置裁剪时返回 RenderClipType::kEmpty
    */
    virtual RenderClipType GetClipInfo(std::vector<UiRect>& clipRects) const = 0;

    /** 判断裁剪区域是否为空(如果为空不需要绘制)
    */
    virtual bool IsClipEmpty() const = 0;

    /** 开始局部绘制：设置本次绘制的脏区列表，并把画布的裁剪区域设置为这些脏矩形的并集
    *   说明：脏区列表用于控件绘制时的精确剔除（见 IntersectPaintRect），裁剪区域用于像素级限制，
    *         两者都设置为脏区后，脏区之外的内容不会被重新光栅化，也不会覆盖已有的正确内容
    * @param [in] rcPaintList 本次需要绘制的脏区矩形列表（客户区坐标）
    * @return 返回保存的裁剪状态值，可传给 EndPaintRects 恢复裁剪状态（<0 表示无需恢复）
    */
    virtual int32_t BeginPaintRects(const std::vector<UiRect>& rcPaintList) = 0;

    /** 结束局部绘制：清除脏区列表，并恢复 BeginPaintRects 保存的裁剪状态
    * @param [in] nClipState BeginPaintRects 返回的状态值
    */
    virtual void EndPaintRects(int32_t nClipState) = 0;

    /** 计算控件矩形与本次绘制脏区的交集，用于精确剔除不需要绘制的控件
    * @param [in] rcItem 控件矩形（当前绘制坐标系）
    * @param [in] rcPaint 本次绘制的区域（当前绘制坐标系）；未设置脏区列表时，作为脏区使用
    * @param [out] rcResult 返回交集区域的边界（多个脏矩形与rcItem交集的并集边界，与rcItem同坐标系）
    * @return 返回false表示无交集，不需要绘制该控件
    * @note 绘制原点可能被偏移（如滚动容器的滚动偏移、控件动画偏移），此时脏区列表是设置时的
    *       客户区坐标，实现内部会自动换算到当前绘制坐标系，调用方只需保证rcItem与rcPaint一致
    */
    virtual bool IntersectPaintRect(const UiRect& rcItem, const UiRect& rcPaint, UiRect& rcResult) const = 0;

    /** 是否为空（宽度或者高度为0）
    */
    virtual bool IsEmpty() const = 0;

    /** 设置Render使用的DPI转换接口
    */
    virtual void SetRenderDpi(const IRenderDpiPtr& spRenderDpi) = 0;

    /** 设置创建本Render的渲染工厂接口（由 IRenderFactory::CreateWindowRender / CreateOffscreenRender 在创建时注入）
    * @param [in] pRenderFactory 渲染工厂接口，用于创建字体等内部资源；生命周期由调用方保证长于本Render
    */
    virtual void SetRenderFactory(IRenderFactory* pRenderFactory) = 0;

    /** 设置图片缩放时的采样质量（对后续的图片绘制生效）
    * @param [in] quality 采样质量
    */
    virtual void SetImageSamplingQuality(ImageSamplingQuality quality) = 0;

    /** 获取图片缩放时的采样质量
    */
    virtual ImageSamplingQuality GetImageSamplingQuality() const = 0;
};

/** 窗口渲染接口：在通用渲染接口的基础上，增加窗口刷新、平台DC与窗口形状能力
*   说明：IRender 只保留与窗口无关的绘制能力，离屏渲染实现不实现本接口。
*/
class DUILIB_API IRenderWindow : public virtual IRender
{
public:
    /** 绘制并刷新到屏幕（Render的实现已经与窗口关联）, 同步完成
    * @param [in] pRenderPaint 界面绘制所需的回调接口
    */
    virtual bool PaintAndSwapBuffers(IRenderPaint* pRenderPaint) = 0;

    /** 窗口的后备缓冲内容是否完整有效
    *   有效表示后备缓冲保存了当前客户区的完整图像，可以直接提交到窗口而不重新绘制。
    *   窗口尺寸变化、DPI变化、缓冲重建等场景必须返回false。
    *   默认返回false，保证与已有实现的兼容性。
    */
    virtual bool IsBackBufferValid() const
    {
        return false;
    }

#ifdef DUILIB_BUILD_FOR_WIN
    /** 获取DC句柄，当不使用后，需要调用ReleaseDC接口释放资源（仅Windows平台支持）
    * @param [in] hWnd 窗口句柄
    * @return 返回窗口句柄关联的DC句柄
    */
    virtual HDC GetRenderDC(HWND hWnd) = 0;

    /** 释放DC资源（仅Windows平台支持）
    * @param [in] hdc 需要释放的DC句柄
    */
    virtual void ReleaseRenderDC(HDC hdc) = 0;

    /** 设置窗口的形状为圆角矩形（仅Windows平台支持）
    * @param [in] rcWnd 需要设置RGN的区域，坐标为屏幕坐标
    * @param [in] rx 圆角的宽度，其值不能为0
    * @param [in] ry 圆角的高度，其值不能为0
    * @param [in] bRedraw 是否重绘
    * @return 设置成功返回true；平台不支持或参数无效时返回false
    */
    virtual bool SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw) = 0;

    /** 设置窗口的形状为直角矩形（仅Windows平台支持）
    * @param [in] rcWnd 需要设置RGN的区域，坐标为屏幕坐标
    * @param [in] bRedraw 是否重绘
    * @return 设置成功返回true；平台不支持或参数无效时返回false
    */
    virtual bool SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw) = 0;

    /** 清除窗口的形状设置, 恢复为系统默认形状（仅Windows平台支持）
    * @param [in] bRedraw 是否重绘
    */
    virtual void ClearWindowRgn(bool bRedraw) = 0;
#endif
};

/** 渲染接口管理，用于创建Font、Pen、Brush、Path、Matrix、Bitmap、Render等渲染实现对象
*/
class DUILIB_API IRenderFactory
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
    virtual IBrush* CreateBrush(UiColor color) = 0;

    /** 创建一个Path对象
    */
    virtual IPath* CreatePath() = 0;

    /** 创建一个Matrix对象
    */
    virtual IMatrix* CreateMatrix() = 0;

    /** 创建一个Bitmap对象
    */
    virtual IBitmap* CreateBitmap() = 0;

    /** 创建一个指定大小与Alpha类型的位图对象（像素数据未初始化）
    * @param [in] nWidth 位图宽度（像素）
    * @param [in] nHeight 位图高度（像素）
    * @param [in] alphaType 位图的Alpha类型
    * @return 返回位图接口，返回后由调用方管理资源；创建或初始化失败时返回nullptr
    */
    virtual IBitmap* CreateBitmap(uint32_t nWidth, uint32_t nHeight, AlphaType alphaType = AlphaType::kPremul) = 0;

    /** 创建一个直接绘制到指定位图上的Render对象（零拷贝）
    * @param [in] pBitmap 绘制目标的位图（必须由本渲染后端创建，且已初始化成功）
    * @param [in] spRenderDpi 关联的DPI转换接口（用于线宽、字体等缩放）
    * @return 成功返回渲染对象；位图无效或后端不支持时返回nullptr
    * @note 绘制内容直接写入位图的像素数据，不产生拷贝；
    *       Render对象不拥有位图的所有权，位图的生命周期必须长于Render对象
    */
    virtual std::unique_ptr<IRender> CreateBitmapRender(IBitmap* pBitmap,
                                                        const IRenderDpiPtr& spRenderDpi = nullptr) = 0;

    /** 查询指定后端是否支持离屏渲染
    * @param [in] backendType 待查询的后端类型
    * @return 支持返回true，否则返回false
    * @note kRaster_BackendType 恒为true；GPU后端取决于当前平台与运行环境（如Windows ARM64不支持GL）
    */
    virtual bool IsOffscreenBackendSupported(RenderBackendType backendType) const = 0;

    /** 创建一个离屏渲染对象（不关联任何窗口）
    * @param [in] nWidth 画布宽度（物理像素），可以为0（表示创建后再Resize）
    * @param [in] nHeight 画布高度（物理像素），可以为0（表示创建后再Resize）
    * @param [in] spRenderDpi 关联的DPI转换接口（用于线宽、字体等缩放）
    * @param [in] backendType 离屏渲染后端；不支持时返回nullptr，不做静默回退
    * @param [in] alphaType 画布的Alpha类型
    * @return 成功返回离屏渲染对象；后端不可用、尺寸非法或内存不足时返回nullptr
    */
    virtual std::unique_ptr<IRender> CreateOffscreenRender(uint32_t nWidth, uint32_t nHeight,
                                                           const IRenderDpiPtr& spRenderDpi = nullptr,
                                                           RenderBackendType backendType = RenderBackendType::kRaster_BackendType,
                                                           AlphaType alphaType = AlphaType::kPremul) = 0;

    /** 创建一个窗口Render对象
    * @param [in] spRenderDpi 关联的DPI转换接口
    * @param [in] platformData 平台相关的数据，Windows平台该值是窗口句柄
    * @param [in] backendType 后台绘制方式
    * @param [in] nMsaaSampleCount MSAA的采样数（0表示使用默认值；最小值1，非法值按1处理）
    * @param [in] bDisableVsync 是否禁用垂直同步（true表示禁用，false表示启用）
    * @return 返回窗口渲染接口，返回后由调用方管理资源；创建失败时返回nullptr
    */
    virtual IRenderWindow* CreateWindowRender(const IRenderDpiPtr& spRenderDpi,
                                              void* platformData,
                                              RenderBackendType backendType = RenderBackendType::kRaster_BackendType,
                                              int32_t nMsaaSampleCount = 0,
                                              bool bDisableVsync = kDefaultDisableVsync) = 0;

    /** 获取字体管理器接口（每个factory共享一个对象）
    */
    virtual IFontMgr* GetFontMgr() const = 0;
};

} // namespace ui

#endif // UI_RENDER_IRENDER_H_
