#ifndef UI_CONTROL_LIST_CTRL_ICON_VIEW_H_
#define UI_CONTROL_LIST_CTRL_ICON_VIEW_H_

#pragma once

#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
/** 列表数据显示和布局功能(Icon类型)
*/
class ListCtrl;
class ListCtrlData;
class ListCtrlIconView : public ListCtrlView, public IListCtrlView
{
public:
    /** 设置是否为List模式
    * @param [in] bListMode true表示列表视图，否则为图标视图
    */
    explicit ListCtrlIconView(bool bListMode);
    virtual ~ListCtrlIconView();

    virtual std::wstring GetType() const override { return L"ListCtrlIconView"; }
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue);
    virtual void HandleEvent(const EventArgs& msg) override;

public:
    /** 设置ListCtrl控件接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 设置数据代理对象
    * @param[in] pProvider 数据代理对象
    */
    virtual void SetDataProvider(VirtualListBoxElement* pProvider) override;

public:
    /** 设置布局方向（默认情况下，Icon视图是纵向布局，List视图是横向布局）
    * @param [in] bHorizontal true表示横向布局，false表示纵向布局
    */
    void SetHorizontalLayout(bool bHorizontal);

    /** 判断当前布局方向是否为横向布局
    */
    bool IsHorizontalLayout() const;

protected:
    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual Control* CreateDataItem() override;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] itemData 数据项（代表行的属性）
    * @param [in] subItemList 数据子项（代表每一列的数据, 第1个是列的ID，第2个是列的数据）
    */
    virtual bool FillDataItem(Control* pControl,
                              size_t nElementIndex,
                              const ListCtrlItemData& itemData,
                              const std::vector<ListCtrlSubItemData2Pair>& subItemList) override;


    /** 获取某列的宽度最大值
    * @param [in] subItemList 数据子项（代表每一列的数据）
    * @return 返回该列宽度的最大值，返回的是DPI自适应后的值； 如果失败返回-1
    */
    virtual int32_t GetMaxDataItemWidth(const std::vector<ListCtrlSubItemData2Ptr>& subItemList) override;

private:
    /** ListCtrl 控件接口
    */
    ListCtrl* m_pListCtrl;

    /** 数据接口
    */
    ListCtrlData* m_pData;

    /** 是否为List模式: true表示列表视图，否则为图标视图 
    */
    const bool m_bListMode;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_ICON_VIEW_H_
