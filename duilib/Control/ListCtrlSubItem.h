#ifndef UI_CONTROL_LIST_CTRL_SUB_ITEM_H_
#define UI_CONTROL_LIST_CTRL_SUB_ITEM_H_

#pragma once

#include "duilib/Core/ImageList.h"
#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
/** 列表项的子项
*/
class ListCtrlItem;
class ListCtrlCheckBox;
class ListCtrlSubItem : public ListCtrlLabel
{
public:
    ListCtrlSubItem();

    /** 获取控件类型
    */
    virtual std::wstring GetType() const override;

    /** 设置属性
    */
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

public:
    /** 设置关联的Item接口
    */
    void SetListCtrlItem(ListCtrlItem* pItem);

    /** 获取关联的Item接口
    */
    ListCtrlItem* GetListCtrlItem() const;

    /** 设置是否显示CheckBox
    * @param [in] bVisible true表示显示，false表示隐藏
    */
    bool SetCheckBoxVisible(bool bVisible);

    /** 判断当前CheckBox是否处于显示状态
    @return 返回true表示CheckBox存在，并且可见； 如果不含CheckBox，返回false
    */
    bool IsCheckBoxVisible() const;

    /** 设置CheckBox的勾选状态
    * @param [in] bSelected true表示勾选，false表示不勾选
    * @param [in] 如果bSelected和bPartSelect同时为true，表示部分选择
    */
    bool SetCheckBoxSelect(bool bSelected, bool bPartSelect);

    /** 获取CheckBox的勾选状态
    * @param [out] bSelected true表示勾选，false表示不勾选
    * @param [out] 如果bSelected和bPartSelect同时为true，表示部分选择
    */
    bool GetCheckBoxSelect(bool& bSelected, bool& bPartSelect) const;

    /** 获取CheckBox接口
    */
    ListCtrlCheckBox* GetCheckBox() const;

    /** 设置关联图标Id, 如果为-1表示不显示图标，图标显示在文本前面
    */
    void SetImageId(int32_t imageId);

    /** 获取关联图标Id
    */
    int32_t GetImageId() const;

    /** 设置文字与图标之间的间隔（像素）
    */
    void SetIconSpacing(int32_t nIconSpacing, bool bNeedDpiScale);

    /** 获取文字与图标之间的间隔（像素）
    */
    int32_t GetIconSpacing() const;

protected:
    /** 绘制文字
    */
    virtual void PaintText(IRender* pRender) override;

    /** 计算文本区域大小（宽和高）
     *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
     *  @return 控件的文本估算大小，包含内边距(Box)，不包含外边距
     */
    virtual UiSize EstimateText(UiSize szAvailable) override;

    /** 加载图标资源
    */
    ImagePtr LoadItemImage() const;

    /** 使得目标区域纵向对齐
    */
    void VAlignRect(UiRect& rc, uint32_t textStyle, int32_t nImageHeight);

private:
    /** 关联的Item接口
    */
    ListCtrlItem* m_pItem;

    /** 关联图标Id, 如果为-1表示不显示图标，图标显示在文本前面
    */
    int32_t m_imageId;

    /** 文字与图标之间的间隔
    */
    int32_t m_nIconSpacing;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_SUB_ITEM_H_
