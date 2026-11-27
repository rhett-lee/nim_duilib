#ifndef UI_CORE_WINDOW_ATTRIBUTES_H_
#define UI_CORE_WINDOW_ATTRIBUTES_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Core/Shadow.h"
#include "duilib/Render/IRender.h"
#include <vector>

namespace pugi
{
    //XML 解析器相关定义
    class xml_node;
}

namespace ui
{

class DpiManager;
class Window;

/** XML 中窗口(Window 节点)属性的解析结果
*
*   本结构只保存"解析后的原始值"，不做任何默认值推导：
*   - 每个属性都配有 m_bXxxDefined 标志：true 表示 XML 中定义了该属性且取值有效；
*   - 未定义的属性，其取值保持结构体中的默认值，调用方不应读取；
*   - 属性名称的匹配不区分大小写，并支持历史别名（如 sizebox、shadowattached 等）。
*
*   解析结果与 XML 中属性的书写顺序无关，同一份配置只会被解析一次。
*/
struct DUILIB_API WindowAttributeValues
{
    /** 是否使用系统的标题栏
    */
    bool m_bUseSystemCaption = false;
    bool m_bUseSystemCaptionDefined = false;

    /** 窗口四边可拉伸范围信息
    */
    UiRect m_rcSizeBox;
    bool m_bSizeBoxDefined = false;

    /** 标题栏区域信息
    */
    UiRect m_rcCaption;
    bool m_bCaptionDefined = false;

    /** 是否启用贴边布局菜单
    */
    bool m_bEnableSnapLayoutMenu = false;
    bool m_bEnableSnapLayoutMenuDefined = false;

    /** 是否启用系统菜单
    */
    bool m_bEnableSysMenu = false;
    bool m_bEnableSysMenuDefined = false;

    /** 系统菜单区域信息
    */
    UiRect m_rcSysMenuRect;
    bool m_bSysMenuRectDefined = false;

    /** 窗口图标
    */
    DString m_windowIcon;
    bool m_bWindowIconDefined = false;

    /** 窗口标题文本
    */
    DString m_windowText;
    bool m_bWindowTextDefined = false;

    /** 窗口标题的文本ID（多语言）
    */
    DString m_windowTextId;
    bool m_bWindowTextIdDefined = false;

    /** 窗口圆角大小
    */
    UiSize m_szRoundCorner;
    bool m_bRoundCornerDefined = false;

    /** 窗口配置的size是否包含阴影
    */
    bool m_bSizeContainShadow = false;
    bool m_bSizeContainShadowDefined = false;

    /** 是否支持阴影效果（未定义时按"支持阴影"处理，与原有逻辑一致）
    */
    bool m_bShadowAttached = true;
    bool m_bShadowAttachedDefined = false;

    /** 阴影类型（解析失败时保持kShadowDefault，且Defined为false）
    */
    ShadowType m_nShadowType = ShadowType::kShadowDefault;
    bool m_bShadowTypeDefined = false;

    /** 阴影图片属性
    */
    DString m_shadowImage;
    bool m_bShadowImageDefined = false;

    /** 阴影九宫格属性（未经DPI缩放）
    */
    UiPadding m_rcShadowCorner;
    bool m_bShadowCornerDefined = false;

    /** 阴影的圆角大小（未经DPI缩放）
    */
    UiSize m_szShadowBorderRound;
    bool m_bShadowBorderRoundDefined = false;

    /** 阴影的边框大小（未经DPI缩放）
    */
    int32_t m_nShadowBorderSize = 0;
    bool m_bShadowBorderSizeDefined = false;

    /** 阴影的边框颜色
    */
    DString m_shadowBorderColor;
    bool m_bShadowBorderColorDefined = false;

    /** 阴影是否支持窗口贴边操作
    */
    bool m_bEnableShadowSnap = false;
    bool m_bEnableShadowSnapDefined = false;

    /** 是否为层窗口
    */
    bool m_bLayeredWindow = false;
    bool m_bLayeredWindowDefined = false;

    /** 窗口透明度（0 - 255），仅当使用层窗口时有效
    */
    int32_t m_nAlpha = 255;
    bool m_bAlphaDefined = false;

    /** 窗口不透明度（0 - 255），该值在SetLayeredWindowAttributes函数中作为参数使用
    */
    int32_t m_nOpacity = 255;
    bool m_bOpacityDefined = false;

    /** 是否启用窗口的拖放操作
    */
    bool m_bEnableDragDrop = false;
    bool m_bEnableDragDropDefined = false;

    /** 后台绘制方式
    */
    RenderBackendType m_renderBackendType = RenderBackendType::kRaster_BackendType;
    bool m_bRenderBackendTypeDefined = false;

    /** 是否开启垂直同步（仅对OpenGL绘制方式有效）
    */
    bool m_bVsync = true;
    bool m_bVsyncDefined = false;

    /** MSAA采样数（仅对OpenGL绘制方式有效）
    */
    int32_t m_nMsaaSampleCount = 0;
    bool m_bMsaaDefined = false;

    /** 期望的SDL Render名称（可选参数）
    */
    DString m_sdlRenderName;
    bool m_bSdlRenderNameDefined = false;

    /** 窗口的初始大小
    *   m_bSizeScaledCX/CY 表示 m_szSize 的宽/高是否已经过DPI缩放；
    *   m_bSizePercentCX/CY 表示 m_szSize 的宽/高是否为屏幕百分比的计算结果。
    */
    UiSize m_szSize;
    bool m_bSizeDefined = false;
    bool m_bSizeScaledCX = false;
    bool m_bSizeScaledCY = false;
    bool m_bSizePercentCX = false;
    bool m_bSizePercentCY = false;

    /** 窗口最小大小（未经DPI缩放）
    */
    UiSize m_szMinSize;
    bool m_bMinSizeDefined = false;

    /** 窗口最大大小（未经DPI缩放）
    */
    UiSize m_szMaxSize;
    bool m_bMaxSizeDefined = false;

    /** XML 中不认识的属性名称（用于配置错误预警）
    */
    std::vector<DString> m_unknownAttributes;

    /** 配置告警信息（属性值非法或配置冲突时的纠正说明，由调用方统一输出）
    */
    std::vector<DString> m_warnings;
};

/** 单趟解析 XML 中窗口(Window 节点)的全部属性
* @param [in] root Window节点（XML的根节点）
* @param [out] values 解析结果（解析前会被重置）
* @param [in] pWindow 关联的窗口，用于解析size属性（百分比按窗口所在屏幕计算），可以为nullptr
* @return 当root是有效的"Window"节点时返回true，否则返回false
*/
bool DUILIB_API ParseWindowAttributeValues(const pugi::xml_node& root, WindowAttributeValues& values,
                                           const Window* pWindow = nullptr);

/** 阴影推导的输入参数（不读取窗口状态，便于复用与测试）
*/
struct DUILIB_API ShadowResolveParam
{
    /** 窗口是否支持系统阴影（对应 NativeWnd()->IsSystemShadowSupported()）
    */
    bool m_bSystemShadowSupported = false;

    /** 窗口当前是否为分层窗口（用于推导默认阴影类型）
    */
    bool m_bLayeredWindow = false;
};

/** 阴影属性的推导结果
*/
struct DUILIB_API ShadowAttributes
{
    /** 最终是否支持阴影效果
    */
    bool m_bShadowAttached = true;

    /** 最终的阴影类型（不会等于 ShadowType::kShadowDefault）
    */
    ShadowType m_nShadowType = ShadowType::kShadowDefault;

    /** 最终的阴影九宫格属性（未经DPI缩放）
    *   阴影未启用时为全0
    */
    UiPadding m_rcShadowCorner;
};

/** 推导窗口的阴影属性（纯函数，不依赖窗口对象）
*   规则：
*   1. shadow_attached 未定义时默认为true；显式设置为false时，九宫格属性被清空；
*   2. shadow_type 未定义或者配置为"default"时，按默认阴影类型推导；
*   3. shadow_corner 未定义或者解析结果为空时，使用阴影类型对应的默认九宫格属性。
*/
ShadowAttributes DUILIB_API ResolveShadowAttributes(const WindowAttributeValues& values,
                                                    const ShadowResolveParam& param);

/** 层窗口属性推导的结果
*/
struct DUILIB_API LayeredWindowAttributes
{
    /** 最终的层窗口状态
    */
    bool m_bIsLayeredWindow = false;

    /** 层窗口状态是否有效（false表示无需改变窗口的层窗口属性）
    */
    bool m_bIsLayeredWindowDefined = false;

    /** 有效的窗口透明度；m_bAlphaDefined为true表示需要设置该值
    */
    int32_t m_nAlpha = 255;
    bool m_bAlphaDefined = false;

    /** 有效的窗口不透明度；m_bOpacityDefined为true表示需要设置该值
    */
    int32_t m_nOpacity = 255;
    bool m_bOpacityDefined = false;
};

/** 推导窗口的层窗口与透明度属性（纯函数，不依赖窗口对象）
*   规则（与创建窗口时的推导保持一致）：
*   1. 显式配置的 layered_window 优先；
*   2. 未配置时，opacity < 255 且未使用系统标题栏，则默认开启层窗口；
*   3. 未配置时，使用系统标题栏，则默认关闭层窗口；
*   4. 未配置时，启用阴影且阴影类型需要层窗口，则默认开启层窗口；
*   5. 系统标题栏与层窗口互斥，同时配置时层窗口被关闭（并在pWarnings中记录该冲突）；
*   6. SDL构建下，未显式配置 layered_window 时默认开启（与原有行为一致）；
*   7. 非分层窗口不支持透明度/不透明度，这些属性会被忽略（并在pWarnings中记录）。
* @param [out] pWarnings 配置冲突的告警信息，可以为nullptr
*/
LayeredWindowAttributes DUILIB_API ResolveLayeredWindowAttributes(const WindowAttributeValues& values,
                                                                  const ShadowResolveParam& param,
                                                                  std::vector<DString>* pWarnings = nullptr);

/** 渲染后端推导的输入参数（窗口的默认配置或全局配置）
*/
struct DUILIB_API RenderBackendResolveParam
{
    /** 默认的后台绘制方式
    */
    RenderBackendType m_defaultBackendType = RenderBackendType::kRaster_BackendType;

    /** 默认的MSAA采样数
    */
    int32_t m_nDefaultMsaaSampleCount = 0;

    /** 默认是否关闭垂直同步
    */
    bool m_bDefaultDisableVsync = kDefaultDisableVsync;
};

/** 渲染后端属性的推导结果
*/
struct DUILIB_API RenderBackendAttributes
{
    /** 最终的绘制方式
    */
    RenderBackendType m_backendType = RenderBackendType::kRaster_BackendType;

    /** 最终的MSAA采样数
    */
    int32_t m_nMsaaSampleCount = 0;

    /** 最终是否关闭垂直同步
    */
    bool m_bDisableVsync = kDefaultDisableVsync;

    /** 是否因"OpenGL不支持层窗口"而降级为CPU绘制（调用方负责输出日志）
    */
    bool m_bFallbackToRaster = false;
};

/** 推导窗口的渲染后端属性（纯函数，不依赖窗口对象）
*   规则：
*   1. XML中配置的 render_backend_type/vsync/msaa 优先，否则使用 param 中的默认值；
*   2. 要求设置不透明度(opacity < 255)或显式配置layered_window="true"时，OpenGL绘制方式会降级为CPU绘制
*      （SDL构建模式不支持OpenGL绘制方式，始终为CPU绘制）；
*      使用默认配置（XML未配置 render_backend_type）降级时，MSAA与垂直同步同时复位。
* @param [out] pWarnings 配置冲突的告警信息，可以为nullptr
*/
RenderBackendAttributes DUILIB_API ResolveRenderBackendAttributes(const WindowAttributeValues& values,
                                                                  const RenderBackendResolveParam& param,
                                                                  std::vector<DString>* pWarnings = nullptr);

/** 窗口初始大小推导的输入参数
*/
struct DUILIB_API WindowInitSizeResolveParam
{
    /** 有效的最小窗口大小（未经DPI缩放）
    */
    UiSize m_szMinSize;

    /** 有效的最大窗口大小（未经DPI缩放）
    */
    UiSize m_szMaxSize;

    /** min/max是否已经过DPI缩放
    *   false：min/max直接来自XML，需要按DPI缩放；
    *   true：min/max来自窗口的当前设置，已经是缩放后的值（应用属性时使用）。
    */
    bool m_bMinMaxScaled = false;

    /** 有效的阴影九宫格属性（未经DPI缩放）
    */
    UiPadding m_rcShadowCorner;

    /** 是否支持阴影效果
    */
    bool m_bShadowAttached = true;

    /** 窗口配置的size是否包含阴影
    */
    bool m_bSizeContainShadow = false;

    /** 窗口是否处于最大化状态
    */
    bool m_bWindowMaximized = false;

    /** 关联的窗口，用于校验窗口大小的上限，可以为nullptr（此时按主显示器计算）
    */
    const Window* m_pWindow = nullptr;
};

/** 推导窗口的初始大小（纯函数）
*   规则（与创建窗口时的计算保持一致）：
*   1. min/max 尚未缩放时先按DPI缩放，然后对 size 做夹取；
*   2. size 未做DPI缩放时进行缩放；
*   3. size 不包含阴影时，按阴影九宫格向四周扩展（百分比宽度/高度不扩展）；
*   4. 最后校验窗口大小，避免超出所在屏幕的工作区。
* @param [out] pWarnings 配置冲突的告警信息，可以为nullptr
* @return 返回最终的窗口大小；当XML未配置size属性时，返回空的大小
*/
UiSize DUILIB_API ResolveWindowInitSize(const WindowAttributeValues& values,
                                        const WindowInitSizeResolveParam& param,
                                        const DpiManager& dpi,
                                        std::vector<DString>* pWarnings = nullptr);

} // namespace ui

#endif // UI_CORE_WINDOW_ATTRIBUTES_H_
