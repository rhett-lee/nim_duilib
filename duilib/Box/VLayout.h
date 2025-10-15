#ifndef UI_BOX_VLAYOUT_H_
#define UI_BOX_VLAYOUT_H_

#include "duilib/Box/Layout.h"

namespace ui 
{

/** 垂直布局：各个子控件横向靠左，垂直方向依次排列
 *  水平方向对齐方式：无，布局时按照子控件本身指定的对齐方式排列子控件
 *  垂直方向对齐方式：默认靠上对齐，子控件本身指定的垂直对齐方式不生效
 */
class UILIB_API VLayout : public Layout
{
    typedef Layout BaseClass;
public:
    VLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::VLayout; }
    
    /** 调整内部所有控件的位置信息
     * @param[in] items 控件列表
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
};

} // namespace ui

#endif // UI_BOX_VLAYOUT_H_
