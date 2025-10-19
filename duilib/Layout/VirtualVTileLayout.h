#ifndef UI_LAYOUT_VIRTUAL_VTILELAYOUT_H_
#define UI_LAYOUT_VIRTUAL_VTILELAYOUT_H_

#include "duilib/Layout/VTileLayout.h"
#include "duilib/Layout/VirtualLayout.h"

namespace ui 
{
/** 虚表实现的瓦片布局(纵向布局)
 *  水平方向对齐方式：默认居中对齐
 *  垂直方向对齐方式：靠上对齐，按控件依次排列
 *  在瓦片布局中，子控件本身指定的对齐方式不生效
 */
class VirtualListBox;
class UILIB_API VirtualVTileLayout : public VTileLayout, public VirtualLayout
{
    typedef VTileLayout BaseClass;
public:
    VirtualVTileLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::VirtualVTileLayout; }

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

public:
    /** 延迟加载展示数据
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    */
    virtual void LazyArrangeChild(UiRect rc) const override;

    /** 获取需要展示的真实数据项最大个数（即有Control对象对应的真实数据项）
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    */
    virtual size_t AjustMaxItem(UiRect rc) const override;

    /** 得到可见范围内第一个元素的前一个元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @return 返回元素的索引
    */
    virtual size_t GetTopElementIndex(UiRect rc) const override;

    /** 判断某个元素是否在可见范围内
    * @param[in] iIndex 元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @return 返回 true 表示可见，否则为不可见
    */
    virtual bool IsElementDisplay(UiRect rc, size_t iIndex) const override;

    /** 判断是否要重新布局
    */
    virtual bool NeedReArrange() const override;

    /** 获取当前所有可见控件的数据元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @param[out] collection 索引列表，范围是：[0, GetElementCount())
    */
    virtual void GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const override;

    /** 让控件在可见范围内
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @param[in] iIndex 元素索引号，范围是：[0, GetElementCount())
    * @param[in] bToTop 是否在最上方
    */
    virtual void EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const override;

private:
    /** 获取数据项的高度
    * @param [in] nCount 数据项个数，如果为Box::InvalidIndex，则获取所有数据项的高度总和
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    * @return 返回 nCount 个数据项的高度总和
    */
    int64_t GetElementsHeight(const UiRect& rc, size_t nCount) const;

    /** 计算列数
    *@param [in] rcWidth 可用区域宽度
    *@return 计算得到的列数, 大于或等于1
    */
    int32_t CalcTileColumns(int32_t rcWidth) const;

private:
    /** 获取关联的Box接口
    */
    VirtualListBox* GetOwnerBox() const;
};
} // namespace ui

#endif // UI_LAYOUT_VIRTUAL_VTILELAYOUT_H_
