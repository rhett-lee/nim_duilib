#ifndef UI_CONTROL_LIST_CTRL_SUB_ITEM_H_
#define UI_CONTROL_LIST_CTRL_SUB_ITEM_H_

#include "duilib/Core/ImageList.h"
#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
/** 列表项的子项
*/
class ListCtrlItem;
class ListCtrlSubItem : public ListCtrlLabel
{
    typedef ListCtrlLabel BaseClass;
public:
    explicit ListCtrlSubItem(Window* pWindow);

    /** 获取控件类型
    */
    virtual DString GetType() const override;

    /** 设置属性
    */
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 是否支持勾选模式（目前是TreeView/ListCtrl在使用这个模式）
        勾选模式是指：
        （1）只有点击在CheckBox图片上的时候，勾选框图片才是选择状态（非勾选模式下，是点击在控件矩形内就选择）
        （2）勾选状态和选择状态分离，是两个不同的状态
    */
    virtual bool SupportCheckMode() const override;

public:
    /** 设置关联的Item接口
    */
    void SetListCtrlItem(ListCtrlItem* pItem);

    /** 获取关联的Item接口
    */
    ListCtrlItem* GetListCtrlItem() const;

    /** 设置是否在该列的数据项显示CheckBox
    * @param [in] bShow true表示在行首显示CheckBox，false表示不显示
    */
    bool SetShowCheckBox(bool bShow);

    /** 判断当前该列的数据项CheckBox是否处于显示状态
    @return 返回true表示CheckBox存在，并且可见； 如果不含CheckBox，返回false
    */
    bool IsShowCheckBox() const;

    /** 判断当前该列的数据项CheckBox是否处于勾选状态
    */
    bool IsCheckBoxChecked() const;

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

    /** 获取CheckBox的图片宽度
    */
    int32_t GetCheckBoxImageWidth();

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
