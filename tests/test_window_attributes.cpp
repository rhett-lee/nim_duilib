// 窗口属性解析与推导（WindowAttributes）的单元测试
//
// 覆盖 duilib/Core/WindowAttributes.h 中的纯逻辑接口：
//   ParseWindowAttributeValues（单趟解析）、ResolveShadowAttributes、
//   ResolveLayeredWindowAttributes、ResolveRenderBackendAttributes、
//   ResolveWindowInitSize。
// 这些接口不创建窗口，可直接在命令行（含 CI）运行。

#include "duilib/Core/WindowAttributes.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/third_party/xml/pugixml.hpp"

#include <functional>
#include <iostream>
#include <string>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const std::function<std::string()>& message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message() << std::endl;
    }
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 判断矩形是否与期望的四个边值一致 */
bool RectEquals(const ui::UiRect& rect, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    return (rect.left == left) && (rect.top == top) && (rect.right == right) && (rect.bottom == bottom);
}

/** 判断内边距是否与期望的四个值一致 */
bool PaddingEquals(const ui::UiPadding& padding, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    return (padding.left == left) && (padding.top == top) && (padding.right == right) && (padding.bottom == bottom);
}

/** 解析XML文本中的Window节点属性 */
bool ParseWindow(const DString& xmlText, ui::WindowAttributeValues& values)
{
    pugi::xml_document xmlDoc;
    pugi::xml_parse_result result = xmlDoc.load_string(xmlText.c_str());
    if (!result) {
        return false;
    }
    return ui::ParseWindowAttributeValues(xmlDoc.first_child(), values, nullptr);
}

/** 使用默认DPI（100%，无缩放）构造尺寸推导参数 */
ui::WindowInitSizeResolveParam MakeSizeParam(const ui::UiPadding& shadowCorner,
                                             bool shadowAttached = true,
                                             bool sizeContainShadow = false)
{
    ui::WindowInitSizeResolveParam param;
    param.m_rcShadowCorner = shadowCorner;
    param.m_bShadowAttached = shadowAttached;
    param.m_bSizeContainShadow = sizeContainShadow;
    return param;
}

/** 全部属性都定义时，解析结果应与XML配置一致 */
void TestParseAllAttributes()
{
    ui::WindowAttributeValues values;
    const bool bParsed = ParseWindow(
        _T("<Window use_system_caption=\"true\" size_box=\"1,2,3,4\" caption=\"5,6,7,8\" ")
        _T("snap_layout_menu=\"false\" sys_menu=\"false\" sys_menu_rect=\"0,0,20,20\" ")
        _T("icon=\"icon.png\" text=\"hello\" text_id=\"txt_main\" round_corner=\"3,4\" ")
        _T("size_contain_shadow=\"true\" shadow_attached=\"false\" shadow_type=\"small_round\" ")
        _T("shadow_image=\"shadow.svg\" shadow_corner=\"9,10,11,12\" shadow_border_round=\"2,2\" ")
        _T("shadow_border_size=\"1\" shadow_border_color=\"#FF0000\" shadow_snap=\"true\" ")
        _T("layered_window=\"true\" alpha=\"128\" opacity=\"200\" drag_drop=\"true\" ")
        _T("render_backend_type=\"GL\" vsync=\"false\" msaa=\"4\" sdl_render_name=\"gpu\" ")
        _T("size=\"800,600\" min_size=\"100,120\" max_size=\"1600,1200\"/>"),
        values);
    CHECK(bParsed);
    CHECK(values.m_unknownAttributes.empty());

    CHECK(values.m_bUseSystemCaptionDefined && values.m_bUseSystemCaption);
    CHECK(values.m_bSizeBoxDefined && RectEquals(values.m_rcSizeBox, 1, 2, 3, 4));
    CHECK(values.m_bCaptionDefined && RectEquals(values.m_rcCaption, 5, 6, 7, 8));
    CHECK(values.m_bEnableSnapLayoutMenuDefined && !values.m_bEnableSnapLayoutMenu);
    CHECK(values.m_bEnableSysMenuDefined && !values.m_bEnableSysMenu);
    CHECK(values.m_bSysMenuRectDefined && RectEquals(values.m_rcSysMenuRect, 0, 0, 20, 20));
    CHECK(values.m_bWindowIconDefined && values.m_windowIcon == _T("icon.png"));
    CHECK(values.m_bWindowTextDefined && values.m_windowText == _T("hello"));
    CHECK(values.m_bWindowTextIdDefined && values.m_windowTextId == _T("txt_main"));
    CHECK(values.m_bRoundCornerDefined && (values.m_szRoundCorner.cx == 3) && (values.m_szRoundCorner.cy == 4));
    CHECK(values.m_bSizeContainShadowDefined && values.m_bSizeContainShadow);
    CHECK(values.m_bShadowAttachedDefined && !values.m_bShadowAttached);
    CHECK(values.m_bShadowTypeDefined && (values.m_nShadowType == ui::ShadowType::kShadowSmallRound));
    CHECK(values.m_bShadowImageDefined && values.m_shadowImage == _T("shadow.svg"));
    CHECK(values.m_bShadowCornerDefined && PaddingEquals(values.m_rcShadowCorner, 9, 10, 11, 12));
    CHECK(values.m_bShadowBorderRoundDefined && (values.m_szShadowBorderRound.cx == 2));
    CHECK(values.m_bShadowBorderSizeDefined && (values.m_nShadowBorderSize == 1));
    CHECK(values.m_bShadowBorderColorDefined && values.m_shadowBorderColor == _T("#FF0000"));
    CHECK(values.m_bEnableShadowSnapDefined && values.m_bEnableShadowSnap);
    CHECK(values.m_bLayeredWindowDefined && values.m_bLayeredWindow);
    CHECK(values.m_bAlphaDefined && (values.m_nAlpha == 128));
    CHECK(values.m_bOpacityDefined && (values.m_nOpacity == 200));
    CHECK(values.m_bEnableDragDropDefined && values.m_bEnableDragDrop);
    CHECK(values.m_bRenderBackendTypeDefined && (values.m_renderBackendType == ui::RenderBackendType::kNativeGL_BackendType));
    CHECK(values.m_bVsyncDefined && !values.m_bVsync);
    CHECK(values.m_bMsaaDefined && (values.m_nMsaaSampleCount == 4));
    CHECK(values.m_bSdlRenderNameDefined && values.m_sdlRenderName == _T("gpu"));
    CHECK(values.m_bSizeDefined);
    CHECK((values.m_szSize.cx == 800) && (values.m_szSize.cy == 600));
    CHECK(!values.m_bSizePercentCX && !values.m_bSizePercentCY);
    CHECK(values.m_bMinSizeDefined && (values.m_szMinSize.cx == 100) && (values.m_szMinSize.cy == 120));
    CHECK(values.m_bMaxSizeDefined && (values.m_szMaxSize.cx == 1600) && (values.m_szMaxSize.cy == 1200));
}

/** 未在XML中出现的属性，Defined标志必须为false */
void TestUndefinedAttributes()
{
    ui::WindowAttributeValues values;
    CHECK(ParseWindow(_T("<Window/>"), values));
    CHECK(values.m_unknownAttributes.empty());
    CHECK(!values.m_bUseSystemCaptionDefined);
    CHECK(!values.m_bSizeBoxDefined);
    CHECK(!values.m_bCaptionDefined);
    CHECK(!values.m_bShadowAttachedDefined);
    CHECK(!values.m_bShadowTypeDefined);
    CHECK(!values.m_bShadowCornerDefined);
    CHECK(!values.m_bLayeredWindowDefined);
    CHECK(!values.m_bAlphaDefined);
    CHECK(!values.m_bOpacityDefined);
    CHECK(!values.m_bRenderBackendTypeDefined);
    CHECK(!values.m_bVsyncDefined);
    CHECK(!values.m_bMsaaDefined);
    CHECK(!values.m_bSizeDefined);
    CHECK(!values.m_bMinSizeDefined);
    CHECK(!values.m_bMaxSizeDefined);
    //未定义阴影属性时，阴影默认开启（与原有逻辑一致）
    CHECK(values.m_bShadowAttached);
}

/** 属性名称不区分大小写，并支持历史别名 */
void TestAliasesAndCaseInsensitive()
{
    ui::WindowAttributeValues values;
    CHECK(ParseWindow(
        _T("<Window SIZeBox=\"1,2,3,4\" ShadowAttached=\"true\" shadowcorner=\"5,6,7,8\" ")
        _T("LAYEREDWINDOW=\"true\" textid=\"id\" roundcorner=\"9,10\" shadowimage=\"x\" ")
        _T("MININFO=\"100,200\" MAXINFO=\"300,400\"/>"),
        values));
    CHECK(values.m_unknownAttributes.empty());
    CHECK(values.m_bSizeBoxDefined && RectEquals(values.m_rcSizeBox, 1, 2, 3, 4));
    CHECK(values.m_bShadowAttachedDefined && values.m_bShadowAttached);
    CHECK(values.m_bShadowCornerDefined && PaddingEquals(values.m_rcShadowCorner, 5, 6, 7, 8));
    CHECK(values.m_bLayeredWindowDefined && values.m_bLayeredWindow);
    CHECK(values.m_bWindowTextIdDefined && values.m_windowTextId == _T("id"));
    CHECK(values.m_bRoundCornerDefined && (values.m_szRoundCorner.cx == 9) && (values.m_szRoundCorner.cy == 10));
    CHECK(values.m_bShadowImageDefined && values.m_shadowImage == _T("x"));
    CHECK(values.m_bMinSizeDefined && (values.m_szMinSize.cx == 100) && (values.m_szMinSize.cy == 200));
    CHECK(values.m_bMaxSizeDefined && (values.m_szMaxSize.cx == 300) && (values.m_szMaxSize.cy == 400));
}

/** Window节点支持class属性：class本身不算未知属性，且不影响节点自身属性的解析 */
void TestWindowClassAttribute()
{
    ui::WindowAttributeValues values;
    CHECK(ParseWindow(
        _T("<Window class=\"popup_window\" render_backend_type=\"GL\" size=\"320,240\"/>"),
        values));
    CHECK(values.m_unknownAttributes.empty());
    CHECK(values.m_bRenderBackendTypeDefined && (values.m_renderBackendType == ui::RenderBackendType::kNativeGL_BackendType));
    CHECK(values.m_bSizeDefined && (values.m_szSize.cx == 320) && (values.m_szSize.cy == 240));
}

/** 未知属性被记录，已识别属性不被记录 */
void TestUnknownAttributes()
{
    ui::WindowAttributeValues values;
    CHECK(ParseWindow(_T("<Window foo=\"1\" text=\"t\" bar=\"2\"/>"), values));
    CHECK(values.m_unknownAttributes.size() == 2);
    if (values.m_unknownAttributes.size() == 2) {
        CHECK(values.m_unknownAttributes[0] == _T("foo"));
        CHECK(values.m_unknownAttributes[1] == _T("bar"));
    }
    CHECK(values.m_bWindowTextDefined);
}

/** 属性书写顺序不影响解析结果 */
void TestAttributeOrderIndependent()
{
    ui::WindowAttributeValues values1;
    ui::WindowAttributeValues values2;
    CHECK(ParseWindow(
        _T("<Window shadow_attached=\"true\" shadow_type=\"small_round\" opacity=\"128\" ")
        _T("size=\"800,600\" shadow_corner=\"1,2,3,4\" layered_window=\"false\"/>"),
        values1));
    CHECK(ParseWindow(
        _T("<Window layered_window=\"false\" shadow_corner=\"1,2,3,4\" size=\"800,600\" ")
        _T("opacity=\"128\" shadow_type=\"small_round\" shadow_attached=\"true\"/>"),
        values2));

    ui::ShadowResolveParam shadowParam;
    shadowParam.m_bLayeredWindow = true;
    const ui::ShadowAttributes shadow1 = ui::ResolveShadowAttributes(values1, shadowParam);
    const ui::ShadowAttributes shadow2 = ui::ResolveShadowAttributes(values2, shadowParam);
    CHECK(shadow1.m_bShadowAttached == shadow2.m_bShadowAttached);
    CHECK(shadow1.m_nShadowType == shadow2.m_nShadowType);
    CHECK(shadow1.m_rcShadowCorner == shadow2.m_rcShadowCorner);

    const ui::LayeredWindowAttributes layered1 = ui::ResolveLayeredWindowAttributes(values1, shadowParam);
    const ui::LayeredWindowAttributes layered2 = ui::ResolveLayeredWindowAttributes(values2, shadowParam);
    CHECK(layered1.m_bIsLayeredWindow == layered2.m_bIsLayeredWindow);
    CHECK(layered1.m_bIsLayeredWindowDefined == layered2.m_bIsLayeredWindowDefined);
    CHECK(layered1.m_bOpacityDefined == layered2.m_bOpacityDefined);
    CHECK(layered1.m_nOpacity == layered2.m_nOpacity);

    ui::DpiManager dpi;
    const ui::WindowInitSizeResolveParam sizeParam = MakeSizeParam(ui::UiPadding(1, 2, 3, 4));
    const ui::UiSize size1 = ui::ResolveWindowInitSize(values1, sizeParam, dpi);
    const ui::UiSize size2 = ui::ResolveWindowInitSize(values2, sizeParam, dpi);
    CHECK(size1 == size2);
}

/** 阴影属性的推导：默认值、显式禁用、显式九宫格 */
void TestResolveShadow()
{
    ui::ShadowResolveParam param;
    //分层窗口时，默认阴影类型为自绘大圆角阴影
    param.m_bLayeredWindow = true;
    param.m_bSystemShadowSupported = false;

    {
        //未配置阴影属性：默认开启，使用默认阴影类型与默认九宫格
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window/>"), values));
        const ui::ShadowAttributes shadow = ui::ResolveShadowAttributes(values, param);
        CHECK(shadow.m_bShadowAttached);
        CHECK(shadow.m_nShadowType == ui::ShadowType::kShadowBigRound);
        CHECK(PaddingEquals(shadow.m_rcShadowCorner, 30, 30, 34, 36));
    }
    {
        //显式禁用阴影：九宫格被清空
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window shadow_attached=\"false\" shadow_type=\"big\" shadow_corner=\"9,9,9,9\"/>"), values));
        const ui::ShadowAttributes shadow = ui::ResolveShadowAttributes(values, param);
        CHECK(!shadow.m_bShadowAttached);
        CHECK(shadow.m_rcShadowCorner.IsEmpty());
    }
    {
        //显式配置阴影类型：使用该类型对应的默认九宫格
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window shadow_attached=\"true\" shadow_type=\"small\"/>"), values));
        const ui::ShadowAttributes shadow = ui::ResolveShadowAttributes(values, param);
        CHECK(shadow.m_bShadowAttached);
        CHECK(shadow.m_nShadowType == ui::ShadowType::kShadowSmall);
        CHECK(PaddingEquals(shadow.m_rcShadowCorner, 24, 24, 28, 30));
    }
    {
        //显式配置九宫格：优先使用配置值
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window shadow_corner=\"1,2,3,4\"/>"), values));
        const ui::ShadowAttributes shadow = ui::ResolveShadowAttributes(values, param);
        CHECK(PaddingEquals(shadow.m_rcShadowCorner, 1, 2, 3, 4));
    }
    {
        //配置为全0的九宫格：视为未配置，使用默认值
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window shadow_type=\"menu\" shadow_corner=\"0,0,0,0\"/>"), values));
        const ui::ShadowAttributes shadow = ui::ResolveShadowAttributes(values, param);
        CHECK(shadow.m_nShadowType == ui::ShadowType::kShadowMenu);
        CHECK(PaddingEquals(shadow.m_rcShadowCorner, 24, 24, 28, 30));
    }
    {
        //系统阴影：不支持系统阴影的窗口由调用方传入标志，这里只验证系统阴影本身没有九宫格
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window shadow_type=\"system_default\"/>"), values));
        ui::ShadowResolveParam systemParam;
        systemParam.m_bLayeredWindow = false;
        systemParam.m_bSystemShadowSupported = true;
        const ui::ShadowAttributes shadow = ui::ResolveShadowAttributes(values, systemParam);
        CHECK(shadow.m_nShadowType == ui::ShadowType::kShadowSystemDefault);
        CHECK(shadow.m_rcShadowCorner.IsEmpty());
    }
}

/** 层窗口属性的推导：opacity、系统标题栏互斥、阴影类型、SDL默认值 */
void TestResolveLayeredWindow()
{
    ui::ShadowResolveParam param;
    param.m_bLayeredWindow = false;
    param.m_bSystemShadowSupported = false;

    {
        //opacity < 255：默认开启层窗口
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window opacity=\"128\"/>"), values));
        const ui::LayeredWindowAttributes layered = ui::ResolveLayeredWindowAttributes(values, param);
        CHECK(layered.m_bIsLayeredWindowDefined);
        CHECK(layered.m_bIsLayeredWindow);
        CHECK(layered.m_bOpacityDefined && (layered.m_nOpacity == 128));
        CHECK(!layered.m_bAlphaDefined);
    }
    {
        //opacity = 255：不需要层窗口（SDL构建默认开启层窗口）
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window opacity=\"255\" shadow_attached=\"false\"/>"), values));
        const ui::LayeredWindowAttributes layered = ui::ResolveLayeredWindowAttributes(values, param);
#if !defined (DUILIB_BUILD_FOR_SDL)
        CHECK(!layered.m_bIsLayeredWindowDefined);
        CHECK(!layered.m_bIsLayeredWindow);
#else
        CHECK(layered.m_bIsLayeredWindowDefined);
        CHECK(layered.m_bIsLayeredWindow);
#endif
        CHECK(!layered.m_bOpacityDefined);
    }
    {
        //系统标题栏与层窗口互斥：显式配置层窗口时被关闭
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window use_system_caption=\"true\" layered_window=\"true\"/>"), values));
        const ui::LayeredWindowAttributes layered = ui::ResolveLayeredWindowAttributes(values, param);
        CHECK(layered.m_bIsLayeredWindowDefined);
        CHECK(!layered.m_bIsLayeredWindow);
    }
    {
        //系统标题栏：非SDL构建默认关闭层窗口
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window use_system_caption=\"true\" shadow_attached=\"false\"/>"), values));
        const ui::LayeredWindowAttributes layered = ui::ResolveLayeredWindowAttributes(values, param);
#if !defined (DUILIB_BUILD_FOR_SDL)
        CHECK(layered.m_bIsLayeredWindowDefined);
        CHECK(!layered.m_bIsLayeredWindow);
#endif
    }
    {
        //自绘阴影需要层窗口
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window shadow_type=\"big_round\"/>"), values));
        const ui::LayeredWindowAttributes layered = ui::ResolveLayeredWindowAttributes(values, param);
#if !defined (DUILIB_BUILD_FOR_SDL)
        CHECK(layered.m_bIsLayeredWindowDefined);
        CHECK(layered.m_bIsLayeredWindow);
#endif
    }
    {
        //系统阴影不需要层窗口
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window shadow_attached=\"true\" shadow_type=\"system_default\"/>"), values));
        const ui::LayeredWindowAttributes layered = ui::ResolveLayeredWindowAttributes(values, param);
#if !defined (DUILIB_BUILD_FOR_SDL)
        CHECK(!layered.m_bIsLayeredWindowDefined);
        CHECK(!layered.m_bIsLayeredWindow);
#endif
    }
    {
        //alpha/opacity 只有小于255时才需要设置
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window alpha=\"128\" opacity=\"255\" layered_window=\"true\"/>"), values));
        const ui::LayeredWindowAttributes layered = ui::ResolveLayeredWindowAttributes(values, param);
        CHECK(layered.m_bAlphaDefined && (layered.m_nAlpha == 128));
        CHECK(!layered.m_bOpacityDefined);
    }
    {
        //alpha/opacity 取值越界时不生效
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window alpha=\"300\" opacity=\"-1\" layered_window=\"true\"/>"), values));
        CHECK(!values.m_bAlphaDefined);
        CHECK(!values.m_bOpacityDefined);
        const ui::LayeredWindowAttributes layered = ui::ResolveLayeredWindowAttributes(values, param);
        CHECK(!layered.m_bAlphaDefined);
        CHECK(!layered.m_bOpacityDefined);
    }
}

/** 渲染后端的推导：XML优先、默认值、GL与opacity的降级 */
void TestResolveRenderBackend()
{
    {
        //XML配置完整：直接使用
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window render_backend_type=\"GL\" vsync=\"false\" msaa=\"4\"/>"), values));
        ui::RenderBackendResolveParam param;
        const ui::RenderBackendAttributes result = ui::ResolveRenderBackendAttributes(values, param);
        CHECK(result.m_backendType == ui::RenderBackendType::kNativeGL_BackendType);
        CHECK(result.m_nMsaaSampleCount == 4);
        CHECK(result.m_bDisableVsync);
        CHECK(!result.m_bFallbackToRaster);
    }
    {
        //"GPU"与"GL"等价
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window render_backend_type=\"GPU\"/>"), values));
        ui::RenderBackendResolveParam param;
        const ui::RenderBackendAttributes result = ui::ResolveRenderBackendAttributes(values, param);
        CHECK(result.m_backendType == ui::RenderBackendType::kNativeGL_BackendType);
    }
    {
        //XML配置GL + opacity < 255：降级为CPU绘制，XML中的MSAA配置保留
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window render_backend_type=\"GL\" msaa=\"4\" opacity=\"128\"/>"), values));
        ui::RenderBackendResolveParam param;
        const ui::RenderBackendAttributes result = ui::ResolveRenderBackendAttributes(values, param);
        CHECK(result.m_backendType == ui::RenderBackendType::kRaster_BackendType);
        CHECK(result.m_nMsaaSampleCount == 4);
        CHECK(result.m_bFallbackToRaster);
    }
    {
        //使用默认配置GL + opacity < 255：降级时MSAA与垂直同步复位
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window opacity=\"128\"/>"), values));
        ui::RenderBackendResolveParam param;
        param.m_defaultBackendType = ui::RenderBackendType::kNativeGL_BackendType;
        param.m_nDefaultMsaaSampleCount = 8;
        param.m_bDefaultDisableVsync = true;
        const ui::RenderBackendAttributes result = ui::ResolveRenderBackendAttributes(values, param);
        CHECK(result.m_backendType == ui::RenderBackendType::kRaster_BackendType);
        CHECK(result.m_nMsaaSampleCount == 0);
        CHECK(!result.m_bDisableVsync);
        CHECK(result.m_bFallbackToRaster);
    }
    {
        //XML未配置：使用传入的默认值
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window/>"), values));
        ui::RenderBackendResolveParam param;
        param.m_defaultBackendType = ui::RenderBackendType::kNativeGL_BackendType;
        param.m_nDefaultMsaaSampleCount = 2;
        param.m_bDefaultDisableVsync = true;
        const ui::RenderBackendAttributes result = ui::ResolveRenderBackendAttributes(values, param);
        CHECK(result.m_backendType == ui::RenderBackendType::kNativeGL_BackendType);
        CHECK(result.m_nMsaaSampleCount == 2);
        CHECK(result.m_bDisableVsync);
        CHECK(!result.m_bFallbackToRaster);
    }
    {
        //XML配置为CPU：即使默认值是GL，也使用CPU绘制
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window render_backend_type=\"CPU\"/>"), values));
        ui::RenderBackendResolveParam param;
        param.m_defaultBackendType = ui::RenderBackendType::kNativeGL_BackendType;
        const ui::RenderBackendAttributes result = ui::ResolveRenderBackendAttributes(values, param);
        CHECK(result.m_backendType == ui::RenderBackendType::kRaster_BackendType);
        CHECK(!result.m_bFallbackToRaster);
    }
    {
        //负数MSAA按0处理
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window msaa=\"-4\"/>"), values));
        CHECK(values.m_bMsaaDefined && (values.m_nMsaaSampleCount == 0));
    }
}

/** 初始大小的推导：min/max夹取、阴影扩展、百分比与边界条件 */
void TestResolveInitSize()
{
    ui::DpiManager dpi;//默认缩放为100%，无DPI缩放

    {
        //size不包含阴影：按九宫格向四周扩展
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window size=\"800,600\"/>"), values));
        const ui::UiSize size = ui::ResolveWindowInitSize(values, MakeSizeParam(ui::UiPadding(10, 20, 30, 40)), dpi);
        CHECK((size.cx == 840) && (size.cy == 660));
    }
    {
        //size包含阴影：不扩展
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window size=\"800,600\" size_contain_shadow=\"true\"/>"), values));
        const ui::UiSize size = ui::ResolveWindowInitSize(
            values, MakeSizeParam(ui::UiPadding(10, 20, 30, 40), true, true), dpi);
        CHECK((size.cx == 800) && (size.cy == 600));
    }
    {
        //阴影被禁用：不扩展
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window size=\"800,600\"/>"), values));
        const ui::UiSize size = ui::ResolveWindowInitSize(
            values, MakeSizeParam(ui::UiPadding(10, 20, 30, 40), false), dpi);
        CHECK((size.cx == 800) && (size.cy == 600));
    }
    {
        //min_size夹取后再扩展阴影
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window size=\"800,600\" min_size=\"1000,700\"/>"), values));
        ui::WindowInitSizeResolveParam param = MakeSizeParam(ui::UiPadding(10, 20, 30, 40));
        param.m_szMinSize = values.m_szMinSize;
        const ui::UiSize size = ui::ResolveWindowInitSize(values, param, dpi);
        CHECK((size.cx == 1040) && (size.cy == 760));
    }
    {
        //max_size夹取后再扩展阴影
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window size=\"800,600\" max_size=\"700,500\"/>"), values));
        ui::WindowInitSizeResolveParam param = MakeSizeParam(ui::UiPadding(10, 20, 30, 40));
        param.m_szMaxSize = values.m_szMaxSize;
        const ui::UiSize size = ui::ResolveWindowInitSize(values, param, dpi);
        CHECK((size.cx == 740) && (size.cy == 560));
    }
    {
        //百分比配置：不扩展阴影
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window size=\"50%,50%\"/>"), values));
        CHECK(values.m_bSizePercentCX && values.m_bSizePercentCY);
        const ui::UiSize size = ui::ResolveWindowInitSize(values, MakeSizeParam(ui::UiPadding(10, 20, 30, 40)), dpi);
        CHECK((size.cx == values.m_szSize.cx) && (size.cy == values.m_szSize.cy));
        CHECK((size.cx > 0) && (size.cy > 0));
    }
    {
        //未配置size：返回空值
        ui::WindowAttributeValues values;
        CHECK(ParseWindow(_T("<Window/>"), values));
        const ui::UiSize size = ui::ResolveWindowInitSize(values, MakeSizeParam(ui::UiPadding(10, 20, 30, 40)), dpi);
        CHECK((size.cx == 0) && (size.cy == 0));
    }
    {
        //固定值不包含DPI缩放标记，百分比包含
        ui::WindowAttributeValues fixedValues;
        CHECK(ParseWindow(_T("<Window size=\"800,600\"/>"), fixedValues));
        CHECK(!fixedValues.m_bSizeScaledCX && !fixedValues.m_bSizeScaledCY);
        ui::WindowAttributeValues percentValues;
        CHECK(ParseWindow(_T("<Window size=\"50%,600\"/>"), percentValues));
        CHECK(percentValues.m_bSizeScaledCX && !percentValues.m_bSizeScaledCY);
    }
}

/** 非Window节点或空节点解析失败 */
void TestParseInvalidNode()
{
    ui::WindowAttributeValues values;
    CHECK(!ParseWindow(_T("<Global/>"), values));
    CHECK(!ParseWindow(_T("<Window"), values));//XML语法错误
    CHECK(values.m_unknownAttributes.empty());
}

} // namespace

int main()
{
    TestParseAllAttributes();
    TestUndefinedAttributes();
    TestAliasesAndCaseInsensitive();
    TestWindowClassAttribute();
    TestUnknownAttributes();
    TestAttributeOrderIndependent();
    TestResolveShadow();
    TestResolveLayeredWindow();
    TestResolveRenderBackend();
    TestResolveInitSize();
    TestParseInvalidNode();

    std::cout << "window attributes tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
