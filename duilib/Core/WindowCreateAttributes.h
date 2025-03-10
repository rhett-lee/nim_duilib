#ifndef UI_CORE_WINDOW_CREATE_ATTRIBUTES_H_
#define UI_CORE_WINDOW_CREATE_ATTRIBUTES_H_

#include "duilib/Core/UiTypes.h"

namespace ui {

/** XML中的窗口的属性
*/
class WindowCreateAttributes
{
public:
    /** 是否使用系统的标题栏
    */
    bool m_bUseSystemCaption = false;
    bool m_bUseSystemCaptionDefined = false;//是否包含有效值

    /** 窗口四边可拉伸范围信息
    */
    UiRect m_rcSizeBox;
    bool m_bSizeBoxDefined = false;//是否包含有效值

    /** 标题栏区域信息
    */
    UiRect m_rcCaption;
    bool m_bCaptionDefined = false;//是否包含有效值

    /** 是否支持阴影效果
    */
    bool m_bShadowAttached = true;
    bool m_bShadowAttachedDefined = false;//是否包含有效值

    /** 是否为层窗口
    */
    bool m_bIsLayeredWindow = true;
    bool m_bIsLayeredWindowDefined = false;//是否包含有效值

    /** 窗口透明度，该值在Windows平台是：UpdateLayeredWindow函数中作为参数使用(BLENDFUNCTION.SourceConstantAlpha)
    */
    uint8_t m_nLayeredWindowAlpha = 255;
    bool m_bLayeredWindowAlphaDefined = false;//是否包含有效值

    /** 窗口不透明度，该值在Windows平台是：SetLayeredWindowAttributes函数中作为参数使用(bAlpha)
    */
    uint8_t m_nLayeredWindowOpacity = 255;
    bool m_bLayeredWindowOpacityDefined = false;//是否包含有效值

    /** 窗口的初始大小
    */
    UiSize m_szInitSize;
    bool m_bInitSizeDefined = false;//是否包含有效值

    /** 期望的SDL Render名称（可选参数）
    */
    DString m_sdlRenderName;
};

} // namespace ui

#endif // UI_CORE_WINDOW_CREATE_ATTRIBUTES_H_

