#ifndef UI_BOX_VIRTUAL_LAYOUT_H_
#define UI_BOX_VIRTUAL_LAYOUT_H_

#pragma once

#include "duilib/Core/UiTypes.h"
#include <vector>

namespace ui 
{
/** 虚表实现的布局接口
*/
class UILIB_API VirtualLayout
{
public:
	VirtualLayout() = default;
	VirtualLayout(const VirtualLayout& r) = delete;
	VirtualLayout& operator=(const VirtualLayout& r) = delete;
	virtual ~VirtualLayout() = default;

public:
    /** 延迟加载展示数据
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    */
    virtual void LazyArrangeChild(UiRect rc) const = 0;

    /** 获取需要展示的真实数据项最大个数（即有Control对象对应的真实数据项）
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    */
    virtual size_t AjustMaxItem(UiRect rc) const = 0;

    /** 得到可见范围内第一个元素的前一个元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @return 返回元素的索引
    */
    virtual size_t GetTopElementIndex(UiRect rc) const = 0 ;

    /** 判断某个元素是否在可见范围内
    * @param[in] iIndex 元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @return 返回 true 表示可见，否则为不可见
    */
    virtual bool IsElementDisplay(UiRect rc, size_t iIndex) const = 0;

    /** 判断是否要重新布局
    */
    virtual bool NeedReArrange() const = 0;

    /** 获取当前所有可见控件的数据元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @param[out] collection 索引列表，范围是：[0, GetElementCount())
    */
    virtual void GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const = 0;

    /** 让控件在可见范围内
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @param[in] iIndex 元素索引号，范围是：[0, GetElementCount())
    * @param[in] bToTop 是否在最上方
    */
    virtual void EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const = 0;
};

} // namespace ui

#endif // UI_BOX_VIRTUAL_LAYOUT_H_
