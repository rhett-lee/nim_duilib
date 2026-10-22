#ifndef UI_LAYOUT_VFLOWLAYOUT_H_
#define UI_LAYOUT_VFLOWLAYOUT_H_

#include "duilib/Layout/Layout.h"

namespace ui 
{

/** 垂直流式布局: 各个子控件在垂直方向依次排列，超过目标区域高度时，自动换列
 *  水平方向对齐方式：默认无，布局时按照子控件本身指定的对齐方式排列子控件（同一列内）
 *  垂直方向对齐方式：默认靠上对齐，子控件本身指定的垂直对齐方式不生效
 */
class DUILIB_API VFlowLayout : public Layout
{
    typedef Layout BaseClass;
public:
    /** 主轴（垂直方向）的分布方式
    */
    enum class JustifyContent
    {
        kNotSet,        //未设置：沿用 child_valign 的原有行为
        kStart,         //起始端（顶部）对齐
        kCenter,        //居中
        kEnd,           //末端（底部）对齐
        kSpaceBetween,  //两端对齐，相邻子项之间均分剩余空间
        kSpaceAround,   //每个子项两侧均分剩余空间（首尾各半个间距）
        kSpaceEvenly    //所有间隙（含首尾）相等
    };

public:
    VFlowLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::VFlowLayout; }

    /** 获取主轴分布方式
    */
    JustifyContent GetJustifyContent() const { return m_justifyContent; }

    /** 设置主轴分布方式（值变化且存在Owner时触发重新布局）
    */
    void SetJustifyContent(JustifyContent justifyContent);

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

    /** 设置布局属性
     * @param [in] strName 要设置的属性名
     * @param [in] strValue 要设置的属性值
     * @param [in] dpiManager DPI管理接口
     * @return true 设置成功，false 属性不存在
     */
    virtual bool SetAttribute(const DString& strName,
                              const DString& strValue,
                              const DpiManager& dpiManager) override;

private:
    /** 调整内部所有控件的位置信息
     * @param [in] items 控件列表
     * @param [in] rc 当前容器位置信息, 包含内边距，但不包含外边距
     * @param [in] bEstimateOnly true表示仅评估不调整控件的位置，false表示调整控件的位置
     * @param [in] bEstimateLayoutSize 当前是否用于估算控件的大小("auto"属性)，这种情况下，对拉伸控件的处理逻辑不同
     * @return 返回排列后最终盒子的宽度和高度信息，包含Owner Box的内边距，不包含外边距
     */
    UiSize64 ArrangeChildInternal(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly, bool bEstimateLayoutSize) const;

private:
    /** 主轴（垂直方向）的分布方式
    */
    JustifyContent m_justifyContent;
};

} // namespace ui

#endif // UI_LAYOUT_VFLOWLAYOUT_H_
