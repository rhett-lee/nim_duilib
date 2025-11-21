#ifndef UI_LAYOUT_VIRTUAL_HLAYOUT_H_
#define UI_LAYOUT_VIRTUAL_HLAYOUT_H_

#include "duilib/Layout/HLayout.h"
#include "duilib/Layout/VirtualLayout.h"

namespace ui 
{
/** 虚表实现的瓦片布局(只有1行的纵向布局)
 *  水平方向对齐方式：靠左对齐，按控件依次排列
 *  垂直方向对齐方式：默认居中对齐
 *  在该布局中，子控件本身指定的对齐方式不生效
 */
class VirtualListBox;
class UILIB_API VirtualHLayout : public HLayout, public VirtualLayout
{
    typedef HLayout BaseClass;
public:
    VirtualHLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::VirtualHLayout; }

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

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] dpiManager DPI缩放管理器
    */
    virtual void ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale) override;

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

public:
    /** 设置子项大小
     * @param [in] szItem 子项大小数据，该宽度和高度，是包含了控件的外边距和内边距的
     * @param [in] bArrange 当变化的时候，是否需要重排
     */
    void SetItemSize(UiSize szItem, bool bArrange = true);

    /** 获取子项大小，该宽度和高度，是包含了控件的外边距和内边距的
     */
    const UiSize& GetItemSize() const;

    /** 设置是否自动计算子项的高度（仅当设置为固定行时有效）
    */
    void SetAutoCalcItemHeight(bool bAutoCalcItemHeight);

    /** 是否自动计算子项的高度
    */
    bool IsAutoCalcItemHeight() const;

private:
    /** 获取数据项的宽度
    * @param [in] nCount 数据项个数，如果为Box::InvalidIndex，则获取所有数据项的高度总和
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    * @return 返回 nCount 个数据项的高度总和
    */
    int64_t GetElementsWidth(UiRect rc, size_t nCount) const;

private:
    /** 获取关联的Box接口
    */
    VirtualListBox* GetOwnerBox() const;

private:
    //子项大小, 该宽度和高度，是包含了控件的外边距和内边距的
    UiSize m_szItem;

    //是否自动计算子项的高度（根据父控件总体高度自动适应，仅当设置为固定行时有效）
    bool m_bAutoCalcItemHeight;
};
} // namespace ui

#endif // UI_LAYOUT_VIRTUAL_HLAYOUT_H_
