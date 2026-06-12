#ifndef UI_CORE_NATIVE_WINDOW_SHADOW_H_
#define UI_CORE_NATIVE_WINDOW_SHADOW_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** 系统阴影类型（具体实现，不同的操作系统平台是不同的）
*/
enum class NativeWindowShadowType
{
    kShadowSystemDisabled   = -1,   //禁用操作系统的阴影
    kShadowSystemDefault    =  0,   //操作系统的默认阴影（默认圆角）
    kShadowSystemDoNotRound =  1,   //操作系统的阴影，直角
    kShadowSystemRound      =  2,   //操作系统的阴影，圆角
    kShadowSystemSmallRound =  3,   //操作系统的阴影，小圆角
};

} // namespace ui

#endif // UI_CORE_NATIVE_WINDOW_SHADOW_H_
