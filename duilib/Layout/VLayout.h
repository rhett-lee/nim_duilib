#ifndef UI_LAYOUT_VLAYOUT_H_
#define UI_LAYOUT_VLAYOUT_H_

#include "duilib/Layout/Layout.h"

namespace ui 
{

/** 垂直布局：各个子控件横向靠左，垂直方向依次排列
 *  水平方向对齐方式：无，布局时按照子控件本身指定的对齐方式排列子控件
 *  垂直方向对齐方式：默认靠上对齐，子控件本身指定的垂直对齐方式不生效
 */
class UILIB_API VLayout : public Layout
{
    typedef Layout BaseClass;
public:
    VLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::VLayout; }
    
    /** 按布局策略调整内部所有子控件的位置和大小
     * @param [in] items 子控件列表
     * @param [in] rc 当前容器位置与大小信息, 包含内边距，但不包含外边距
     * @param [in] bEstimateOnly true表示仅评估不调整控件的位置，false表示调整控件的位置
     * @return 返回排列后最终布局的宽度和高度信息，包含Box容器的内边距，但不包含Box容器本身的外边距(当容器支持滚动条时使用该返回值)
     */
    virtual UiSize64 ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly = false) override;

    /** 根据内部子控件大小估算容器布局大小（用于评估宽度或者高度为"auto"类型的控件大小，拉伸类型的子控件不计入大小估算）
     * @param [in] items 子控件列表
     * @param [in] szAvailable 容器的可用宽度和高度，包含分配给该容器的内边距，但不包含分配给容器的外边距
     * @return 返回排列后最终布局的大小信息（宽度和高度），包含Box容器本身的内边距，但不包含Box容器本身的外边距；
     */
    virtual UiSize64 EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable) override;
};

} // namespace ui

#endif // UI_LAYOUT_VLAYOUT_H_
