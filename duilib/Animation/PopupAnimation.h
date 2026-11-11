#ifndef UI_ANIMATION_POPUP_ANIMATION_H_
#define UI_ANIMATION_POPUP_ANIMATION_H_

#include "duilib/Animation/EasingFunctions.h"
#include "duilib/duilib_defs.h"
#include <memory>

namespace ui
{

class AnimationPlayer;
class WindowBase;

/** 弹出层动画的全局配置与总开关
*
*  用于弹出层窗口（Flyout / Popover / Popconfirm、Menu 及子菜单、Combo 下拉窗口等）的显示过渡：
*  窗口显示时把分层窗口的 Alpha 值从 0 渐变到 255，形成整体淡入效果。
*
*  说明：
*  1. 默认关闭；关闭时弹出层按原有方式瞬时显示，行为与开启前完全一致；
*  2. 所有弹出层共用同一套参数，单个弹出层不需要单独配置；
*  3. 仅对分层窗口（Layered Window）生效，非分层窗口会自动跳过动画。
*/
class DUILIB_API PopupAnimation final
{
public:
    PopupAnimation() = delete;

    /** 设置弹出层动画总开关
    */
    static void SetEnabled(bool bEnabled);

    /** 弹出层动画总开关是否开启
    */
    static bool IsEnabled();

    /** 设置淡入时长（毫秒，默认 120；<=0 表示不播放动画）
    */
    static void SetTotalMillSeconds(int32_t nTotalMillSeconds);

    /** 获取淡入时长（毫秒）
    */
    static int32_t GetTotalMillSeconds();

    /** 设置淡入使用的缓动函数（默认 EaseOutCubic）
    */
    static void SetEasingFunctionType(EasingFunctionType easingFunctionType);

    /** 获取淡入使用的缓动函数
    */
    static EasingFunctionType GetEasingFunctionType();

    /** 播放弹出层淡入动画
    * @param [in] pWindow 弹出层窗口（需要已经创建，且为分层窗口）
    * @param [in,out] pPlayer 动画播放器：内部创建并赋值；调用方负责保存该对象（通常作为窗口的成员），
    *                         重复调用时会先停止上一次动画
    * @note 全局开关关闭、时长为 0 或窗口不是分层窗口时，直接把 Alpha 置为 255，不播放动画
    */
    static void PlayFadeIn(WindowBase* pWindow, std::unique_ptr<AnimationPlayer>& pPlayer);
};

} // namespace ui

#endif // UI_ANIMATION_POPUP_ANIMATION_H_
