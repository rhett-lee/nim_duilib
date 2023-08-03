#ifndef UI_BOX_VIRTUAL_VTILELAYOUT_H_
#define UI_BOX_VIRTUAL_VTILELAYOUT_H_

#pragma once

#include "duilib/Box/VTileLayout.h"
#include "duilib/Box/VirtualLayout.h"

namespace ui 
{
/** 虚表实现的瓦片布局(纵向布局)
*/
class VirtualListBox;
class UILIB_API VirtualVTileLayout : public VTileLayout, public VirtualLayout
{
public:
    VirtualVTileLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::VirtualVTileLayout; }

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

#endif // UI_BOX_VIRTUAL_VTILELAYOUT_H_
