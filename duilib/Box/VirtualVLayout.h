#ifndef UI_BOX_VIRTUAL_VLAYOUT_H_
#define UI_BOX_VIRTUAL_VLAYOUT_H_

#pragma once

#include "duilib/Box/VLayout.h"
#include "duilib/Box/VirtualLayout.h"

namespace ui 
{
/** 虚表实现的瓦片布局
*/
class VirtualListBox;
class UILIB_API VirtualVLayout : public VLayout, public VirtualLayout
{
public:
    VirtualVLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::VirtualVLayout; }

    /** 调整内部所有控件的位置信息
        * @param [in] items 控件列表
        * @param[in] rc 当前容器位置信息, 包含内边距，但不包含外边距
        * @return 返回排列后最终盒子的宽度和高度信息
        */
    virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

    /** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
        * @param[in] items 子控件列表
        * @param [in] szAvailable 可用大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
        * @return 返回排列后最终布局的大小信息（宽度和高度）；
                包含items中子控件的外边距，包含items中子控件的内边距；
                包含Box控件本身的内边距；
                不包含Box控件本身的外边距；
                返回值中不包含拉伸类型的子控件大小。
        */
    virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;

    /** 设置布局属性
     * @param[in] strName 要设置的属性名
     * @param[in] strValue 要设置的属性值
     * @return true 设置成功，false 属性不存在
     */
    virtual bool SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

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
    int64_t GetElementsHeight(UiRect rc, size_t nCount) const;

private:
    /** 获取关联的Box接口
    */
    VirtualListBox* GetOwnerBox() const;

    /** 设置子项大小
     * @param[in] szItem 子项大小数据，该宽度和高度，是包含了控件的外边距和内边距的
     */
    void SetItemSize(UiSize szItem, bool bNeedDpiScale = true);

    /** 获取子项大小，该宽度和高度，是包含了控件的外边距和内边距的
     */
    const UiSize& GetItemSize() const;

private:
    //子项大小, 该宽度和高度，是包含了控件的外边距和内边距的
    UiSize m_szItem;
};
} // namespace ui

#endif // UI_BOX_VIRTUAL_VLAYOUT_H_
