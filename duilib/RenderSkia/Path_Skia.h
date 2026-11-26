#ifndef UI_RENDER_SKIA_PATH_H_
#define UI_RENDER_SKIA_PATH_H_

#include "duilib/Render/IRender.h"

class SkPathBuilder;

namespace ui 
{

/** Path_Skia 是 IPath 的 Skia 实现，用 SkPathBuilder 构建和变换路径，功能要点如下：
 *  1. 支持添加直线、贝塞尔曲线、矩形、椭圆、圆弧、多边形等几何图形
 *  2. 支持设置填充类型，并可通过矩阵对路径进行变换
 *  3. 支持计算路径的边界矩形，以及克隆路径对象
 */
class Path_Skia : public IPath
{
public:
    Path_Skia();
    virtual ~Path_Skia() override;
    Path_Skia(const Path_Skia& r);
    Path_Skia& operator=(const Path_Skia& r) = delete;

    /// 重写父类方法，请参考父类声明：克隆与重置
    virtual std::unique_ptr<IPath> Clone() const override;
    virtual void Reset() override;

    /** 关闭当前绘图
    */
    virtual void Close() override;

    /// 重写父类方法，请参考父类声明：填充类型
    virtual void SetFillType(FillType mode) override;
    virtual FillType GetFillType() const override;

    /// 重写父类方法，请参考父类声明：添加几何图形
    virtual void AddLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2) override;
    virtual void AddLine(float x1, float y1, float x2, float y2) override;
    virtual void AddLines(const UiPoint* points, int32_t count) override;
    virtual void AddLines(const UiPointF* points, int32_t count) override;
    virtual void AddBezier(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t x4, int32_t y4) override;
    virtual void AddBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) override;
    virtual void AddBeziers(const UiPoint* points, int32_t count) override;
    virtual void AddBeziers(const UiPointF* points, int32_t count) override;
    virtual void AddRect(const UiRect& rect) override;
    virtual void AddRect(const UiRectF& rect) override;
    virtual void AddEllipse(const UiRect& rect) override;
    virtual void AddEllipse(const UiRectF& rect) override;
    virtual void AddArc(const UiRect& rect, float startAngle, float sweepAngle) override;
    virtual void AddArc(const UiRectF& rect, float startAngle, float sweepAngle) override;
    virtual void AddPolygon(const UiPoint* points, int32_t count) override;
    virtual void AddPolygon(const UiPointF* points, int32_t count) override;
    /// 重写父类方法，请参考父类声明：路径变换
    virtual void Transform(const IMatrix* pMatrix) override;

    /// 重写父类方法，请参考父类声明：获取路径边界
    virtual UiRect GetBounds(const IPen* pen) const override;

public:
    /** 获取Skia Path
    */
    SkPathBuilder* GetSkPathBuilder() const;

private:
    /** 将绘制起点移动到指定的点(x1,y1)
    */
    void MoveToPoint(int32_t x1, int32_t y1);
    void MoveToPoint(float x1, float y1);

private:
    /** Skia 的 Path 对象
    */
    std::unique_ptr<SkPathBuilder> m_skPathBuilder;
};

} // namespace ui

#endif // UI_RENDER_SKIA_PATH_H_
