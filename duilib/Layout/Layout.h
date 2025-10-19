#ifndef UI_LAYOUT_LAYOUT_H_
#define UI_LAYOUT_LAYOUT_H_

#include "duilib/Core/UiTypes.h"

namespace ui 
{
class Box;
class Control;
class DpiManager;

/** 控件的布局类型
*/
enum class LayoutType
{
    FloatLayout,            //浮动布局

    HLayout,                //横向布局
    VLayout,                //纵向布局
    HTileLayout,            //横向瓦片布局
    VTileLayout,            //纵向瓦片布局
    HFlowLayout,            //横向流式布局
    VFlowLayout,            //纵向流式布局

    VirtualHLayout,         //虚表横向布局
    VirtualVLayout,         //虚表纵向布局
    VirtualHTileLayout,     //虚表横向瓦片布局
    VirtualVTileLayout,     //虚表纵向瓦片布局

    ListCtrlReportLayout    //ListCtrl控件的Report模式布局(仅内部使用)
};

/** 布局管理器基类
 *  该类的控件布局类型为Float方式布局：各个子控件以区域左上角坐标依次堆叠排列（相当于都看成是Float控件）
 *  水平方向对齐方式：无，布局时按照子控件本身指定的对齐方式排列子控件
 *  垂直方向对齐方式：无，布局时按照子控件本身指定的对齐方式排列子控件
 */
class UILIB_API Layout
{
public:
    Layout();
    Layout(const Layout& r) = delete;
    Layout& operator=(const Layout& r) = delete;
    virtual ~Layout() = default;

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const { return LayoutType::FloatLayout; }

    /** 设置布局属性
     * @param [in] strName 要设置的属性名
     * @param [in] strValue 要设置的属性值
     * @param [in] dpiManager DPI管理接口
     * @return true 设置成功，false 属性不存在
     */
    virtual bool SetAttribute(const DString& strName, 
                              const DString& strValue,
                              const DpiManager& dpiManager);

    /** DPI发生变化，更新控件大小和布局
     * @param [in] nOldDpiScale 旧的DPI缩放百分比
     * @param [in] dpiManager DPI缩放管理器
     */
    virtual void ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale);

    /** 是否为纵向布局
    */
    virtual bool IsVLayout() const 
    {
        LayoutType type = GetLayoutType();
        return (type == LayoutType::VLayout) ||
               (type == LayoutType::VFlowLayout) ||
               (type == LayoutType::VTileLayout) ||
               (type == LayoutType::VirtualVLayout) ||
               (type == LayoutType::VirtualVTileLayout) ||
               (type == LayoutType::ListCtrlReportLayout);
    }

    /** 是否为横向布局
    */
    virtual bool IsHLayout() const 
    {
        LayoutType type = GetLayoutType();
        return (type == LayoutType::HLayout) ||
               (type == LayoutType::HFlowLayout) ||
               (type == LayoutType::HTileLayout) ||
               (type == LayoutType::VirtualHLayout) ||
               (type == LayoutType::VirtualHTileLayout);
    }

    /** 是否为瓦片布局(可能为纵向或者横向布局)
    */
    virtual bool IsTileLayout() const
    {
        LayoutType type = GetLayoutType();
        return (type == LayoutType::VTileLayout) ||
               (type == LayoutType::HTileLayout) ||
               (type == LayoutType::VirtualVTileLayout) ||
               (type == LayoutType::VirtualHTileLayout);
    }

    /** 按布局策略调整内部所有子控件的位置和大小
     * @param [in] items 子控件列表
     * @param [in] rc 当前容器位置与大小信息, 包含内边距，但不包含外边距
     * @param [in] bEstimateOnly true表示仅评估不调整控件的位置，false表示调整控件的位置
     * @return 返回排列后最终布局的宽度和高度信息，包含Box容器的内边距，但不包含Box容器本身的外边距(当容器支持滚动条时使用该返回值)
     */
    virtual UiSize64 ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly = false);

    /** 根据内部子控件大小估算容器布局大小（用于评估宽度或者高度为"auto"类型的控件大小，拉伸类型的子控件不计入大小估算）
     * @param [in] items 子控件列表
     * @param [in] szAvailable 容器的可用宽度和高度，包含分配给该容器的内边距，但不包含分配给容器的外边距
     * @return 返回排列后最终布局的大小信息（宽度和高度），包含Box容器本身的内边距，但不包含Box容器本身的外边距；
     */
    virtual UiSize64 EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable);

    /** 当处于ScrollBox中时，是否预先计算实际区域大小，然后再按实际区域大小对子控件进行布局
     *  对于部分布局，当处于ScrollBox中时，必须按实际区域大小布局，而不是按显示区域布局（影响因素：是否有拉伸类型的子控件，水平和垂直对齐方式）
     * @return 返回true表示当处于ScrollBox中时，预先计算实际区域大小，然后再按实际区域大小对子控件进行布局
     *         返回false当处于ScrollBox中时，不需要特殊处理，直接对子控件进行布局
     */
    virtual bool LayoutByActualAreaSize() const;

public:
    /** 设置所有者容器接口
    */
    void SetOwner(Box* pOwner);

    /** 获取所有者容器接口
    */
    Box* GetOwner() const { return m_pOwner; }

    /** 获取子控件之间的额外边距（X轴方向）
     * @return 返回额外间距的数值
     */
    int32_t GetChildMarginX() const { return m_nChildMarginX; }

    /** 获取子控件之间的额外边距（Y轴方向）
     * @return 返回额外间距的数值
     */
    int32_t GetChildMarginY() const { return m_nChildMarginY; }

    /** 设置子控件之间的额外边距（X轴方向）
     * @param [in] nMarginX 要设置的边距数值
     */
    void SetChildMarginX(int32_t nMarginX);

    /** 设置子控件之间的额外边距（Y轴方向）
     * @param [in] nMarginY 要设置的边距数值
     */
    void SetChildMarginY(int32_t nMarginY);

    /** 设置子控件之间的额外边距（X轴方向和Y轴方向，均设置为同一个值）
     * @param [in] nMargin 要设置的边距数值
     */
    void SetChildMargin(int32_t nMargin);

    /** 获取子控件的水平对齐方式
     */
    HorAlignType GetChildHAlignType() const { return m_hChildAlignType; }

    /** 设置子控件的水平对齐方式
     * @param [in] hAlignType 要设置的对齐方式
     */
    void SetChildHAlignType(HorAlignType hAlignType);

    /** 获取子控件的垂直对齐方式
     */
    VerAlignType GetChildVAlignType() const { return m_vChildAlignType; }

    /** 设置子控件的垂直对齐方式
     * @param [in] vAlignType 要设置的对齐方式
     */
    void SetChildVAlignType(VerAlignType vAlignType);

public:
    /** 将区域去掉内边距, 并确保rc区域有效
    */
    void DeflatePadding(UiRect& rc) const;

public:
    /** 设置浮动状态下的坐标信息
     * @param [in] pControl 控件句柄
     * @param [in] rcContainer 要设置的矩形区域，不包含容器的内边距
     * @return 返回控件最终的大小信息（宽度和高度）
     */
    static UiSize64 SetFloatPos(Control* pControl, const UiRect& rcContainer);

protected:
    /** 检查配置的宽和高是否正确, 如果发现错误，给予断言
    */
    void CheckConfig(const std::vector<Control*>& items);

    /** 按照控件指定的对齐方式，计算控件的布局位置
    * @param [in] pControl 控件的接口
    * @param [in] rcContainer 目标容器的矩形，包含控件的外边距和内边距
    * @param [in] childSize 控件pControl的大小（宽和高）, 包含内边距，内部不会再计算控件的大小
    * @return 返回控件的位置和大小，不包含外边距，包含内边距
              这个返回值，可用pControl->SetPos(rect)来调整控件位置;
    */
    static UiRect GetFloatPos(const Control* pControl, UiRect rcContainer, UiSize childSize);

private:
    /** 设置浮动状态下的坐标信息
     * @param [in] pControl 控件句柄
     * @param [in] rcContainer 要设置的矩形区域，不包含容器的内边距
     * @param [in] bEstimateOnly true表示仅评估不调整控件的位置，false表示调整控件的位置
     * @return 返回控件最终的大小信息（宽度和高度）
     */
    static UiSize64 SetFloatPosInternal(Control* pControl, const UiRect& rcContainer, bool bEstimateOnly);

private:
    //所属Box对象
    Box* m_pOwner;

    //子控件之间的额外边距: X 轴方向
    uint16_t m_nChildMarginX;

    //子控件之间的额外边距: Y 轴方向
    uint16_t m_nChildMarginY;

    //子控件的水平对齐方式
    HorAlignType m_hChildAlignType;

    //子控件的垂直对齐方式
    VerAlignType m_vChildAlignType;
};

} // namespace ui

#endif // UI_LAYOUT_LAYOUT_H_
