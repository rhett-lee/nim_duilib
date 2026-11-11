#ifndef EXAMPLES_ANIMATION_DEMO_FORM_H_
#define EXAMPLES_ANIMATION_DEMO_FORM_H_

// duilib
#include "duilib/duilib.h"
#include <functional>

/** 控件属性动画的完整演示页面
*
*  演示内容：
*  1. 全局参数：动画总时长、缓动函数类型；
*  2. 属性动画：Alpha / Width / Height / Size 的进场与出场；
*  3. 滑动进出场：上下左右四个方向；
*  4. 缓动曲线对比：同一时刻启动、使用不同缓动函数的动画；
*  5. Retarget：播放过程中修改结束值，从当前值继续播放；
*  6. 物理动画：弹簧（Panel 展开/折叠）与惯性（列表滚动），支持全局开关与自检；
*  7. 弹出层淡入：Popover / Menu / Combo 下拉，支持全局开关；
*  8. 轮播切换缓动：Carousel 的 transition_easing 配置；
*  9. 功能说明。
*/
class AnimationDemoForm : public ui::WindowImplBase
{
public:
    AnimationDemoForm();
    virtual ~AnimationDemoForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

private:
    /** 初始化"全局参数"区域的按钮
    */
    void InitGlobalSettings();

    /** 初始化属性动画的演示控件（Alpha/Width/Height/Size）
    */
    void InitPropertyDemo();

    /** 初始化滑动进出场的演示控件（上下左右四个方向）
    */
    void InitSlideDemo();

    /** 初始化缓动曲线对比的演示控件
    */
    void InitEasingDemo();

    /** 初始化 Retarget 的演示控件
    */
    void InitRetargetDemo();

    /** 初始化物理动画的演示控件（Panel 弹簧展开/折叠、列表惯性滚动、总开关）
    */
    void InitPhysicsDemo();

    /** 初始化弹出层淡入动画的演示控件（Popover / Menu / Combo 下拉、总开关）
    */
    void InitPopupDemo();

    /** 初始化 Carousel 切换缓动的演示控件
    */
    void InitCarouselDemo();

    /** 刷新弹出层动画总开关的状态显示
    */
    void UpdatePopupStateLabel();

    /** 刷新轮播当前缓动曲线的显示
    */
    void UpdateCarouselEasingLabel();

    /** 弹出演示菜单（使用与弹出层动画相同的淡入效果）
    * @param [in] ptMouse 菜单弹出的位置（窗口客户区坐标，内部会转换为屏幕坐标）
    */
    void ShowDemoMenu(const ui::UiPoint& ptMouse);

    /** 刷新物理动画总开关的状态显示
    */
    void UpdatePhysicsStateLabel();

    /** 运行物理动画自检（校验弹簧收敛/回弹/换目标，惯性滑行距离/边界钳制，总开关）
    *  结果直接显示在页面上，无需单元测试框架
    */
    void RunPhysicsSelfTest();

    /** 把当前的全局参数（时长、缓动函数）应用到所有演示控件
    */
    void ApplyGlobalSettings();

    /** 切换控件的显示/隐藏状态（按已配置的动画播放）
    */
    void ToggleControlVisible(const DString& controlName);

    /** 按已配置的动画显示/隐藏所有滑动演示控件
    */
    void SetSlideControlsVisible(bool bVisible);

    /** 同一时刻启动所有缓动曲线对比动画
    */
    void PlayEasingCompare();

    /** 设置 Retarget 演示控件的新宽度（播放中会从当前值继续播放）
    */
    void SetRetargetWidth(int32_t nNewWidth);

    /** 绑定按钮的点击事件
    */
    void BindButtonClick(const DString& buttonName, const std::function<void()>& handler);

private:
    /** 动画总时长（毫秒）
    */
    int32_t m_nTotalMillSeconds;

    /** 缓动函数类型
    */
    ui::EasingFunctionType m_easingType;
};

#endif //EXAMPLES_ANIMATION_DEMO_FORM_H_
