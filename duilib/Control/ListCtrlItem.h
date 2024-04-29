#ifndef UI_CONTROL_LIST_CTRL_ITEM_H_
#define UI_CONTROL_LIST_CTRL_ITEM_H_

#pragma once

#include "duilib/Box/ListBox.h"
#include "duilib/Control/Label.h"
#include "duilib/Core/ImageList.h"
#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
/** ListCtrl列表数据项UI控件（行）
*    基本结构: <ListCtrlItem> <ListCtrlSubItem/> ... <ListCtrlSubItem/>  </ListCtrlItem>
     附加说明: 1. ListCtrlItem 是HBox的子类;   
              2. 每一列，放置一个ListCtrlSubItem控件
              3. ListCtrlSubItem 是LabelBox的子类
*/
class ListCtrl;
class ListCtrlSubItem;
class ListCtrlItem : public ListCtrlItemBaseH
{
    friend class ListCtrlReportView;//开放部分protect接口
public:
    ListCtrlItem();
    virtual ~ListCtrlItem();

    /** 获取控件类型
    */
    virtual std::wstring GetType() const override;

    /** 设置属性
    */
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

    /** 事件处理函数
    */
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 判断控件类型是否为可选择的
     * @return 默认返回false
     */
    virtual bool IsSelectableType() const override;

    /** 设置是否可以选择(置顶数据不可选择，其他数据可选择)
    */
    void SetSelectableType(bool bSelectable);

public:
    /** 设置关联的ListCtrl接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 获取关联的ListCtrl接口
    */
    ListCtrl* GetListCtrl() const;

    /** 获取子控件的个数
    */
    size_t GetSubItemCount() const;

    /** 获取第columnIndex个子控件
    * @param [in] columnIndex 列索引序号：[0, GetSubItemCount())
    */
    ListCtrlSubItem* GetSubItem(size_t columnIndex) const;

    /** 获取鼠标所在位置的子控件
    * @param [in] ptMouse 鼠标所在的位置，屏幕坐标点
    */
    ListCtrlSubItem* GetSubItem(const UiPoint& ptMouse) const;

    /** 获取鼠标所在位置的子控件的列索引序号(哪一列)
    * @param [in] ptMouse 鼠标所在的位置，屏幕坐标点
    */
    size_t GetSubItemIndex(const UiPoint& ptMouse) const;

    /** 获取子控件的列索引序号(哪一列)
    * @param [in] pSubItem 子控件的接口
    */
    size_t GetSubItemIndex(ListCtrlSubItem* pSubItem) const;

    /** 设置是否在行首显示CheckBox
    * @param [in] bShow true表示在行首显示CheckBox，false表示不显示
    */
    bool SetShowCheckBox(bool bShow);

    /** 判断是否行首显示了CheckBox
    */
    bool IsShowCheckBox() const;

    /** 设置关联图标Id, 如果为-1表示不显示图标，图标显示在文本前面
    */
    void SetImageId(int32_t imageId);

    /** 获取关联图标Id
    */
    int32_t GetImageId() const;

    /** 设置图标之间的间隔（像素）
    */
    void SetIconSpacing(int32_t nIconSpacing, bool bNeedDpiScale);

    /** 获取图标之间的间隔（像素）
    */
    int32_t GetIconSpacing() const;

protected:
    /** 获取关联图标/CheckBox等所占的宽度(左侧)
    */
    int32_t GetItemPaddingLeft();

    /** 加载图标资源
    */
    ImagePtr LoadItemImage() const;

    /** 使得目标区域纵向对齐
    */
    void VAlignRect(UiRect& rc, uint32_t textStyle, int32_t nImageHeight);

    /** 获取CheckBox的图片宽度
    */
    int32_t GetCheckBoxImageWidth();

protected:
    
    /** 鼠标左键弹起事件
    */
    virtual bool ButtonUp(const EventArgs& msg) override;

    /** 是否支持勾选模式（目前是TreeView/ListCtrl在使用这个模式）
        勾选模式是指：
        （1）只有点击在CheckBox图片上的时候，勾选框图片才是选择状态（非勾选模式下，是点击在控件矩形内就选择）
        （2）勾选状态和选择状态分离，是两个不同的状态
    */
    virtual bool SupportCheckedMode() const override;

    /** 绘制函数
    */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

private:
    /** 是否可以选择（影响方向键切换选择项）
    */
    bool m_bSelectable;

    /** 关联的ListCtrl接口
    */
    ListCtrl* m_pListCtrl;

    /** 关联图标Id, 如果为-1表示不显示图标，图标显示在文本前面
    */
    int32_t m_imageId;

    /** CheckBox所占宽度
    */
    int32_t m_nCheckBoxWidth;

    /** 图标之间的间隔
    */
    int32_t m_nIconSpacing;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_ITEM_H_
