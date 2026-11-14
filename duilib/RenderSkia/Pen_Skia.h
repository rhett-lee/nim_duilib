#ifndef UI_RENDER_SKIA_PEN_H_
#define UI_RENDER_SKIA_PEN_H_

#include "duilib/Render/IRender.h"

#include <vector>

namespace ui 
{

class Pen_Skia : public IPen
{
public:
    explicit Pen_Skia(UiColor color, float fWidth = 1.0f);
    Pen_Skia(const Pen_Skia& r);
    Pen_Skia& operator=(const Pen_Skia& r) = delete;

    /** 设置画笔宽度
    */
    virtual void SetWidth(float fWidth) override;

    /** 获取画笔宽度
    */
    virtual float GetWidth() const override;

    /** 设置画笔颜色
    */
    virtual void SetColor(UiColor color) override;

    /** 获取画笔颜色
    */
    virtual UiColor GetColor() const override;

    /** 设置线段起始的笔帽样式
    */
    virtual void SetStartCap(LineCap cap) override;

    /** 获取线段起始的笔帽样式
    */
    virtual LineCap GetStartCap() const override;

    /** 设置线段结束的笔帽样式
    */
    virtual void SetEndCap(LineCap cap) override;

    /** 获取线段结束的笔帽样式
    */
    virtual LineCap GetEndCap() const override;

    /** 设置短划线笔帽的样式
    */
    virtual void SetDashCap(LineCap cap) override;

    /** 获取短划线笔帽的样式
    */
    virtual LineCap GetDashCap() const override;

    /** 设置线段末尾使用的联接样式
    */
    virtual void SetLineJoin(LineJoin join) override;

    /** 获取线段末尾使用的联接样式
    */
    virtual LineJoin GetLineJoin() const override;

    /** 设置绘制的线条样式
    */
    virtual void SetDashStyle(DashStyle style) override;

    /** 获取绘制的线条样式
    */
    virtual DashStyle GetDashStyle() const override;

    /** 设置自定义虚线样式
    */
    virtual void SetDashPattern(const float* intervals, int32_t count) override;

    /** 是否设置了自定义虚线样式
    */
    virtual bool HasDashPattern() const override;

    /** 设置是否启用抗锯齿
    */
    virtual void SetAntiAlias(bool bAntiAlias) override;

    /** 是否启用抗锯齿
    */
    virtual bool IsAntiAlias() const override;

    /** 设置联接处的斜接限制
    */
    virtual void SetMiterLimit(float fMiterLimit) override;

    /** 获取联接处的斜接限制
    */
    virtual float GetMiterLimit() const override;

    /** 获取自定义虚线样式的元素列表（为空表示未设置）
    */
    const std::vector<float>& GetDashPattern() const;

    /** 复制Pen对象
    */
    virtual std::unique_ptr<IPen> Clone() const override;

private:
    /** 画笔颜色
    */
    UiColor m_color;

    /** 画笔宽度
    */
    float m_fWidth;

    /** 线段起始笔帽形状
    */
    LineCap m_startCap;

    /** 短划线笔帽形状
    */
    LineCap m_dashCap;

    /** 线段结束笔帽形状
    */
    LineCap m_endCap;

    /** 线段末尾使用的联接样式，该线段与另一个线段相遇
    */
    LineJoin m_lineJoin;

    /** 短划线样式
    */
    DashStyle m_dashStyle;

    /** 自定义虚线样式（成对的实线长度、空白长度）
    */
    std::vector<float> m_dashPattern;

    /** 是否启用抗锯齿
    */
    bool m_bAntiAlias;

    /** 联接处的斜接限制
    */
    float m_fMiterLimit;
};

} // namespace ui

#endif // UI_RENDER_SKIA_PEN_H_
