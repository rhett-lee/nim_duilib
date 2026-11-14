#ifndef UI_RENDER_SKIA_BRUSH_H_
#define UI_RENDER_SKIA_BRUSH_H_

#include "duilib/Render/IRender.h"

class SkPaint;
class SkBitmap;

namespace ui 
{

class Brush_Skia : public IBrush
{
public:
    explicit Brush_Skia(UiColor color);
    Brush_Skia(const Brush_Skia& r);
    Brush_Skia& operator=(const Brush_Skia& r) = delete;

    virtual std::unique_ptr<IBrush> Clone() const override;

    /** 获取画刷类型
    */
    virtual BrushType GetBrushType() const override;

    /** 获取画刷颜色（渐变画刷时为起始颜色）
    */
    virtual UiColor GetColor() const override;

    /** 设置画刷为纯色画刷
    */
    virtual void SetSolid(UiColor color) override;

    /** 设置线性渐变
    */
    virtual void SetLinearGradient(UiColor endColor, const UiPointF& startPt, const UiPointF& endPt) override;

    /** 设置径向渐变
    */
    virtual void SetRadialGradient(UiColor endColor, const UiPointF& center, float radius) override;

public:
    /** 根据画刷类型，把对应的着色器设置到Paint对象（非纯色画刷时生效）
    * @param [in,out] skPaint 目标Paint对象
    * @param [in] offsetX 画布原点在X轴的偏移量
    * @param [in] offsetY 画布原点在Y轴的偏移量
    */
    void ApplyShader(SkPaint& skPaint, float offsetX, float offsetY) const;

private:
    /** 画刷颜色（渐变画刷时为起始颜色）
    */
    UiColor m_color;

    /** 画刷类型
    */
    BrushType m_brushType;

    /** 渐变结束颜色
    */
    UiColor m_endColor;

    /** 渐变起点/渐变中心点
    */
    UiPointF m_startPt;

    /** 渐变终点
    */
    UiPointF m_endPt;

    /** 径向渐变的半径
    */
    float m_fRadius;
};

} // namespace ui

#endif // UI_RENDER_SKIA_BRUSH_H_
