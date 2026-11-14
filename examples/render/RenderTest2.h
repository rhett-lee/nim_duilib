#ifndef EXAMPLES_RENDER_TEST2_H_
#define EXAMPLES_RENDER_TEST2_H_

// duilib
#include "duilib/duilib.h"

namespace ui {

/** 图形绘制测试控件：以表格形式展示 IRender 中图形绘制接口的各种用法
*/
class RenderTest2 : public ui::Control
{
    typedef ui::Control BaseClass;
public:
    explicit RenderTest2(ui::Window* pWindow);
    virtual ~RenderTest2() override;

public:
    /** 绘制控件自身
    * @param[in] pRender 渲染接口
    * @param[in] rcPaint 指定绘制区域
    */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

    /** 计算控件大小：返回展示全部绘制示例所需的大小（用于配合纵向滚动条）
    * @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
    */
    virtual UiEstSize EstimateSize(UiSize szAvailable) override;

private:
    /** 绘制项定义
    */
    struct DrawItem
    {
        const char* pCaption;   //绘制项的标题
        int32_t nColumns;       //占用的列数
        int32_t nRows;          //占用的行数
    };

    /** 绘制项的布局结果
    */
    struct ItemLayout
    {
        UiRect rcDraw;          //图形绘制区域（相对坐标，绘制时需要加上控件的左上角坐标）
        UiRect rcCaption;       //标题绘制区域（相对坐标，绘制时需要加上控件的左上角坐标）
    };

    /** 布局参数（各值均为DPI缩放后的像素值）
    */
    struct LayoutInfo
    {
        int32_t nColumns = 6;       //总列数
        int32_t nCellWidth = 0;     //单个单元格的宽度（图形绘制区域的宽度）
        int32_t nCellHeight = 0;    //单个单元格的高度（图形绘制区域的高度）
        int32_t nCaptionHeight = 0; //单元格标题的高度
        int32_t nGap = 0;           //单元格之间的间距
        int32_t nMargin = 0;        //内容区域的外边距
        int32_t nRowCount = 0;      //总行数
        int32_t nContentWidth = 0;  //内容总宽度
        int32_t nContentHeight = 0; //内容总高度
    };

private:
    /** 获取绘制项的列表
    */
    static const std::vector<DrawItem>& GetDrawItems();

    /** 计算布局参数
    * @param [out] itemLayouts 返回每个绘制项的布局结果（相对坐标）
    */
    LayoutInfo CalcLayout(std::vector<ItemLayout>& itemLayouts) const;

    /** 绘制一个图形绘制项
    * @param [in] pRender 渲染接口
    * @param [in] pRenderFactory 渲染接口工厂
    * @param [in] nIndex 绘制项的索引
    * @param [in] rcDraw 图形绘制区域（客户区坐标）
    */
    void PaintItem(IRender* pRender, IRenderFactory* pRenderFactory, int32_t nIndex, const UiRect& rcDraw);

    /** 绘制单元格的标题
    */
    void PaintCaption(IRender* pRender, const char* pCaption, const UiRect& rcCaption);

    /** 绘制正六边形（通过多边形接口）
    */
    static bool DrawRegularHexagon(IRender* pRender, const UiPointF& centerPt, int32_t radius,
                                   const UiColor& penColor, int32_t penWidth, const UiColor& brushColor);

    /** 绘制正六边形（通过路径接口，按区块分别填充）
    */
    static bool DrawRegularHexagon3(IRender* pRender, const UiPoint& centerPt, int32_t radius,
                                    const UiColor& penColor, int32_t penWidth, const UiColor& brushColor);

    /** 用正六边形拼接一个复杂图形
    */
    static void DrawColorMap(IRender* pRender, const UiRect& rect);

    /** 创建一个圆角四边形路径（用于展示路径绘制）
    */
    static std::unique_ptr<IPath> CreateRoundRectPath(IRenderFactory* pRenderFactory, const UiRect& rc, int32_t nRoundSize);

    /** 按DPI缩放整数值
    */
    int32_t DpiScaledInt(int32_t nValue) const;

    /** 按DPI缩放整数值（返回浮点数）
    */
    float DpiScaledFloat(int32_t nValue) const;
};

} //end of namespace ui
#endif //EXAMPLES_RENDER_TEST2_H_
