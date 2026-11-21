#include "WindowAttributes.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/third_party/xml/pugixml.hpp"

#include <functional>

namespace ui
{

namespace
{
    /** 推导默认阴影类型（不依赖窗口状态，保持与 Shadow::GetDefaultShadowType 的语义一致）
    */
    ShadowType GetDefaultShadowTypeForResolve(const ShadowResolveParam& param)
    {
        if (param.m_bLayeredWindow) {
            //分层窗口，默认为自绘阴影，圆角
            return ShadowType::kShadowBigRound;
        }
        if (param.m_bSystemShadowSupported) {
            //该窗口支持系统阴影
            return ShadowType::kShadowSystemDefault;
        }
        //未关联窗口的情况（按操作系统版本选择）
        return Shadow::GetDefaultShadowType(nullptr);
    }

    /** 获取Class(样式)中定义的属性列表（支持别名）
    */
    DString GetWindowClassAttributes(const DString& strClassName)
    {
        const GlobalManager& globalManager = GlobalManager::Instance();
        if (!globalManager.IsInUIThread()) {
            //非UI线程（比如单元测试）不查询全局样式，避免触发UI线程断言
            return DString();
        }
        DString classAttributes = globalManager.GetClassAttributes(strClassName);
        if (classAttributes.empty()) {
            //支持通过别名引用样式
            const DString aliasValue = globalManager.GetAliasValue(strClassName);
            if (!aliasValue.empty()) {
                classAttributes = globalManager.GetClassAttributes(aliasValue);
            }
        }
        return classAttributes;
    }
}

bool ParseWindowAttributeValues(const pugi::xml_node& root, WindowAttributeValues& values,
                                const Window* pWindow)
{
    values = WindowAttributeValues();
    if (root.empty()) {
        return false;
    }
    if (!StringUtil::IsEqualNoCase(root.name(), _T("Window"))) {
        return false;
    }

    //应用单个属性到values
    std::function<void(const DString&, const DString&)> applyAttribute;
    applyAttribute = [&values, pWindow](const DString& strName, const DString& strValue) {
        if (StringUtil::IsEqualNoCase(strName, _T("use_system_caption"))) {
            values.m_bUseSystemCaption = StringUtil::IsValueTrue(strValue);
            values.m_bUseSystemCaptionDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("size_box")) ||
                 StringUtil::IsEqualNoCase(strName, _T("sizebox"))) {
            AttributeUtil::ParseRectValue(strValue.c_str(), values.m_rcSizeBox, false);
            values.m_bSizeBoxDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("caption"))) {
            AttributeUtil::ParseRectValue(strValue.c_str(), values.m_rcCaption);
            values.m_bCaptionDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("snap_layout_menu"))) {
            values.m_bEnableSnapLayoutMenu = StringUtil::IsValueTrue(strValue);
            values.m_bEnableSnapLayoutMenuDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("sys_menu"))) {
            values.m_bEnableSysMenu = StringUtil::IsValueTrue(strValue);
            values.m_bEnableSysMenuDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("sys_menu_rect"))) {
            AttributeUtil::ParseRectValue(strValue.c_str(), values.m_rcSysMenuRect);
            values.m_bSysMenuRectDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("icon"))) {
            values.m_windowIcon = strValue;
            values.m_bWindowIconDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("text"))) {
            values.m_windowText = strValue;
            values.m_bWindowTextDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("text_id")) ||
                 StringUtil::IsEqualNoCase(strName, _T("textid"))) {
            values.m_windowTextId = strValue;
            values.m_bWindowTextIdDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("round_corner")) ||
                 StringUtil::IsEqualNoCase(strName, _T("roundcorner"))) {
            AttributeUtil::ParseSizeValue(strValue.c_str(), values.m_szRoundCorner);
            values.m_bRoundCornerDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("size_contain_shadow"))) {
            values.m_bSizeContainShadow = StringUtil::IsValueTrue(strValue);
            values.m_bSizeContainShadowDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("shadow_attached")) ||
                 StringUtil::IsEqualNoCase(strName, _T("shadowattached"))) {
            values.m_bShadowAttached = StringUtil::IsValueTrue(strValue);
            values.m_bShadowAttachedDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("shadow_type"))) {
            ShadowType nShadowType = ShadowType::kShadowDefault;
            if (Shadow::GetShadowType(strValue, nShadowType)) {
                values.m_nShadowType = nShadowType;
                values.m_bShadowTypeDefined = true;
            }
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("shadow_image")) ||
                 StringUtil::IsEqualNoCase(strName, _T("shadowimage"))) {
            values.m_shadowImage = strValue;
            values.m_bShadowImageDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("shadow_corner")) ||
                 StringUtil::IsEqualNoCase(strName, _T("shadowcorner"))) {
            AttributeUtil::ParsePaddingValue(strValue.c_str(), values.m_rcShadowCorner);
            values.m_bShadowCornerDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("shadow_border_round"))) {
            AttributeUtil::ParseSizeValue(strValue.c_str(), values.m_szShadowBorderRound);
            values.m_bShadowBorderRoundDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("shadow_border_size"))) {
            values.m_nShadowBorderSize = StringUtil::StringToInt32(strValue);
            values.m_bShadowBorderSizeDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("shadow_border_color"))) {
            values.m_shadowBorderColor = strValue;
            values.m_bShadowBorderColorDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("shadow_snap"))) {
            values.m_bEnableShadowSnap = StringUtil::IsValueTrue(strValue);
            values.m_bEnableShadowSnapDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("layered_window")) ||
                 StringUtil::IsEqualNoCase(strName, _T("layeredwindow"))) {
            values.m_bLayeredWindow = StringUtil::IsValueTrue(strValue);
            values.m_bLayeredWindowDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("alpha"))) {
            //设置窗口的透明度（0 - 255），仅当使用层窗口时有效
            const int32_t nAlpha = StringUtil::StringToInt32(strValue);
            if ((nAlpha >= 0) && (nAlpha <= 255)) {
                values.m_nAlpha = nAlpha;
                values.m_bAlphaDefined = true;
            }
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("opacity"))) {
            //设置窗口的不透明度（0 - 255），该值在SetLayeredWindowAttributes函数中作为参数使用
            const int32_t nOpacity = StringUtil::StringToInt32(strValue);
            if ((nOpacity >= 0) && (nOpacity <= 255)) {
                values.m_nOpacity = nOpacity;
                values.m_bOpacityDefined = true;
            }
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("drag_drop"))) {
            values.m_bEnableDragDrop = StringUtil::IsValueTrue(strValue);
            values.m_bEnableDragDropDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("render_backend_type"))) {
            RenderBackendType backendType = RenderBackendType::kRaster_BackendType;
            if (StringUtil::IsEqualNoCase(strValue, _T("GL")) ||
                StringUtil::IsEqualNoCase(strValue, _T("GPU"))) {
                backendType = RenderBackendType::kNativeGL_BackendType;
            }
            else if (!StringUtil::IsEqualNoCase(strValue, _T("CPU"))) {
                //无效的配置值：按CPU绘制处理（配置错误由未知属性/日志机制另行提示）
                ASSERT(0);
            }
            values.m_renderBackendType = backendType;
            values.m_bRenderBackendTypeDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("vsync"))) {
            values.m_bVsync = StringUtil::IsValueTrue(strValue);
            values.m_bVsyncDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("msaa"))) {
            int32_t nMsaaSampleCount = StringUtil::StringToInt32(strValue);
            if (nMsaaSampleCount < 0) {
                nMsaaSampleCount = 0;
            }
            values.m_nMsaaSampleCount = nMsaaSampleCount;
            values.m_bMsaaDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("sdl_render_name"))) {
            //期望的SDL Render的名称
            values.m_sdlRenderName = strValue;
            values.m_bSdlRenderNameDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("size"))) {
            AttributeUtil::ParseWindowSize(pWindow, strValue.c_str(), values.m_szSize,
                                           &values.m_bSizeScaledCX, &values.m_bSizeScaledCY,
                                           &values.m_bSizePercentCX, &values.m_bSizePercentCY);
            values.m_bSizeDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("min_size")) ||
                 StringUtil::IsEqualNoCase(strName, _T("mininfo"))) {
            AttributeUtil::ParseSizeValue(strValue.c_str(), values.m_szMinSize);
            values.m_bMinSizeDefined = true;
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("max_size")) ||
                 StringUtil::IsEqualNoCase(strName, _T("maxinfo"))) {
            AttributeUtil::ParseSizeValue(strValue.c_str(), values.m_szMaxSize);
            values.m_bMaxSizeDefined = true;
        }
        else {
            //不认识的属性：记录下来，供调用方统一预警
            values.m_unknownAttributes.push_back(strName);
        }
    };

    //应用Class(样式)及其父Class中定义的属性：父Class先应用，当前Class后应用
    std::vector<DString> resolvedClasses;
    std::function<void(const DString&, int32_t)> applyClassAttributes;
    applyClassAttributes = [&](const DString& strClassList, int32_t nDepth) {
        if (nDepth > 16) {
            //防止Class之间循环引用导致无限递归
            return;
        }
        const std::list<DString> classList = StringUtil::Split(strClassList, _T(" \t\r\n"));
        for (const DString& strClassName : classList) {
            if (strClassName.empty()) {
                continue;
            }
            bool bResolved = false;
            for (const DString& strResolvedName : resolvedClasses) {
                if (StringUtil::IsEqualNoCase(strResolvedName, strClassName)) {
                    bResolved = true;
                    break;
                }
            }
            if (bResolved) {
                continue;
            }
            resolvedClasses.push_back(strClassName);

            const DString classAttributes = GetWindowClassAttributes(strClassName);
            if (classAttributes.empty()) {
                //未找到该Class定义的属性
                continue;
            }
            std::vector<std::pair<DString, DString>> attributeList;
            if (!AttributeUtil::ParseAttributeList(classAttributes, attributeList)) {
                continue;
            }
            //先递归应用父Class，再应用当前Class中的其他属性
            for (const auto& attribute : attributeList) {
                if (StringUtil::IsEqualNoCase(attribute.first, _T("class"))) {
                    applyClassAttributes(attribute.second, nDepth + 1);
                }
            }
            for (const auto& attribute : attributeList) {
                if (StringUtil::IsEqualNoCase(attribute.first, _T("class"))) {
                    continue;
                }
                applyAttribute(attribute.first, attribute.second);
            }
        }
    };

    //先应用Class定义的公共属性，再应用节点自身的属性（节点属性的优先级更高）
    const DString strClassList = root.attribute(_T("class")).as_string();
    if (!strClassList.empty()) {
        applyClassAttributes(strClassList, 0);
    }
    for (pugi::xml_attribute attr : root.attributes()) {
        const DString strName = attr.name();
        if (StringUtil::IsEqualNoCase(strName, _T("class"))) {
            //Class属性已在上面处理
            continue;
        }
        applyAttribute(strName, attr.value());
    }
    return true;
}

ShadowAttributes ResolveShadowAttributes(const WindowAttributeValues& values,
                                         const ShadowResolveParam& param)
{
    ShadowAttributes shadow;
    //shadow_attached未定义时，默认支持阴影
    shadow.m_bShadowAttached = !values.m_bShadowAttachedDefined || values.m_bShadowAttached;

    //推导有效的阴影类型
    shadow.m_nShadowType = values.m_bShadowTypeDefined ? values.m_nShadowType : ShadowType::kShadowDefault;
    if (shadow.m_nShadowType == ShadowType::kShadowDefault) {
        shadow.m_nShadowType = GetDefaultShadowTypeForResolve(param);
    }
    //不支持系统阴影时，转换为实际支持的自绘阴影类型
    shadow.m_nShadowType = Shadow::GetSupportedShadowType(shadow.m_nShadowType, param.m_bSystemShadowSupported);

    if (!shadow.m_bShadowAttached) {
        //阴影被禁用：九宫格属性清空（阴影图片不再参与计算）
        shadow.m_rcShadowCorner.Clear();
        return shadow;
    }
    if (!values.m_rcShadowCorner.IsEmpty()) {
        //XML中配置了有效的九宫格属性，直接使用
        shadow.m_rcShadowCorner = values.m_rcShadowCorner;
        return shadow;
    }

    //未配置九宫格属性（或者解析结果为空）：按阴影类型获取默认的九宫格属性
    UiSize szBorderRound;
    DString shadowImage;
    Shadow::GetShadowParam(nullptr, shadow.m_nShadowType,
                           szBorderRound, shadow.m_rcShadowCorner, shadowImage);
    return shadow;
}

LayeredWindowAttributes ResolveLayeredWindowAttributes(const WindowAttributeValues& values,
                                                       const ShadowResolveParam& param)
{
    LayeredWindowAttributes result;
    //XML中是否显式配置了层窗口属性（SDL下用于判断是否应用默认值）
    const bool bXmlLayeredDefined = values.m_bLayeredWindowDefined;

    const bool bUseSystemCaption = values.m_bUseSystemCaptionDefined && values.m_bUseSystemCaption;
    const bool bShadowAttached = !values.m_bShadowAttachedDefined || values.m_bShadowAttached;
    //opacity < 255 时要求窗口必须是层窗口
    const bool bNeedLayeredByOpacity = values.m_bOpacityDefined && (values.m_nOpacity < 255);

    bool bDefined = values.m_bLayeredWindowDefined;
    bool bLayered = values.m_bLayeredWindow;
    if (!bDefined && !bUseSystemCaption && bNeedLayeredByOpacity) {
        //使用Opacity属性，默认需要开启分层窗口
        bDefined = true;
        bLayered = true;
    }
    else if (!bDefined && bUseSystemCaption) {
        //使用系统标题栏属性，默认不开启分层窗口
        bDefined = true;
        bLayered = false;
    }
    if (!bDefined && bShadowAttached && !bUseSystemCaption) {
        //阴影启用时，若阴影类型需要层窗口，则默认开启分层窗口
        ShadowType nShadowType = values.m_bShadowTypeDefined ? values.m_nShadowType : ShadowType::kShadowDefault;
        if (nShadowType == ShadowType::kShadowDefault) {
            nShadowType = GetDefaultShadowTypeForResolve(param);
        }
        if (Shadow::IsShadowTypeNeedLayeredWindow(nShadowType)) {
            bDefined = true;
            bLayered = true;
        }
    }
    if (bUseSystemCaption && bLayered) {
        //系统标题栏与分层窗口属性互斥，使用系统标题栏时必须关闭分层窗口属性
        bLayered = false;
    }

#if defined (DUILIB_BUILD_FOR_SDL)
    //默认开启支持透明度(SDL在Windows系统中，未使用分层窗口属性)
    if (!bXmlLayeredDefined) {
        bDefined = true;
        bLayered = true;
    }
#else
    //仅SDL构建使用该变量（XML中是否定义了分层窗口属性），非SDL构建时显式标记为未使用
    UNUSED_VARIABLE(bXmlLayeredDefined);
#endif

    result.m_bIsLayeredWindow = bLayered;
    result.m_bIsLayeredWindowDefined = bDefined;

    //透明度/不透明度：只记录需要设置的有效值（与创建窗口时的记录规则一致）
    if (values.m_bAlphaDefined && (values.m_nAlpha < 255)) {
        result.m_nAlpha = values.m_nAlpha;
        result.m_bAlphaDefined = true;
    }
    if (values.m_bOpacityDefined && (values.m_nOpacity < 255)) {
        result.m_nOpacity = values.m_nOpacity;
        result.m_bOpacityDefined = true;
    }
    return result;
}

RenderBackendAttributes ResolveRenderBackendAttributes(const WindowAttributeValues& values,
                                                       const RenderBackendResolveParam& param)
{
    RenderBackendAttributes result;
    if (values.m_bRenderBackendTypeDefined) {
        //XML中配置了绘制方式：MSAA与垂直同步只取XML配置值（未配置时使用默认值）
        result.m_backendType = values.m_renderBackendType;
        result.m_nMsaaSampleCount = values.m_bMsaaDefined ? values.m_nMsaaSampleCount : 0;
        //XML中的vsync属性为true表示开启垂直同步；未配置时使用默认值
        result.m_bDisableVsync = values.m_bVsyncDefined ? !values.m_bVsync : kDefaultDisableVsync;
    }
    else {
        //XML中未配置绘制方式：使用传入的默认配置
        result.m_backendType = param.m_defaultBackendType;
        result.m_nMsaaSampleCount = param.m_nDefaultMsaaSampleCount;
        result.m_bDisableVsync = param.m_bDefaultDisableVsync;
    }

    //GL绘制方式不支持分层窗口：XML要求设置不透明度时，降级为CPU绘制（否则窗口内容无法显示）
    const bool bNeedLayeredWindowOpacity = values.m_bOpacityDefined && (values.m_nOpacity < 255);
    if ((result.m_backendType == RenderBackendType::kNativeGL_BackendType) && bNeedLayeredWindowOpacity) {
        result.m_backendType = RenderBackendType::kRaster_BackendType;
        if (!values.m_bRenderBackendTypeDefined) {
            //使用默认配置降级时，MSAA与垂直同步同时复位（降级为CPU绘制后不再需要这些配置）
            result.m_nMsaaSampleCount = 0;
            result.m_bDisableVsync = false;
        }
        result.m_bFallbackToRaster = true;
    }
    return result;
}

UiSize ResolveWindowInitSize(const WindowAttributeValues& values,
                             const WindowInitSizeResolveParam& param,
                             const DpiManager& dpi)
{
    UiSize initSize;
    if (!values.m_bSizeDefined) {
        return initSize;
    }
    int32_t cx = values.m_szSize.cx;
    int32_t cy = values.m_szSize.cy;

    //min/max属性未做DPI缩放时，先按DPI缩放，再做夹取
    UiSize minSize = param.m_szMinSize;
    UiSize maxSize = param.m_szMaxSize;
    if (!param.m_bMinMaxScaled && values.m_bSizeScaledCX) {
        dpi.ScaleWindowSize(minSize.cx);
        dpi.ScaleWindowSize(maxSize.cx);
    }
    if (!param.m_bMinMaxScaled && values.m_bSizeScaledCY) {
        dpi.ScaleWindowSize(minSize.cy);
        dpi.ScaleWindowSize(maxSize.cy);
    }
    if ((minSize.cx > 0) && (cx < minSize.cx)) {
        cx = minSize.cx;
    }
    if ((maxSize.cx > 0) && (cx > maxSize.cx)) {
        cx = maxSize.cx;
    }
    if ((minSize.cy > 0) && (cy < minSize.cy)) {
        cy = minSize.cy;
    }
    if ((maxSize.cy > 0) && (cy > maxSize.cy)) {
        cy = maxSize.cy;
    }

    //size的值未做DPI缩放时，进行DPI缩放
    if (!values.m_bSizeScaledCX) {
        dpi.ScaleWindowSize(cx);
    }
    if (!values.m_bSizeScaledCY) {
        dpi.ScaleWindowSize(cy);
    }

    if (!param.m_bSizeContainShadow && param.m_bShadowAttached && !param.m_bWindowMaximized) {
        //XML配置中指定的窗口大小，如果设置的是固定值，则不包含阴影部分
        if (!values.m_bSizePercentCX) {
            UiPadding rcShadowCorner = param.m_rcShadowCorner;
            dpi.ScaleWindowSize(rcShadowCorner.left);
            dpi.ScaleWindowSize(rcShadowCorner.right);
            cx += rcShadowCorner.left + rcShadowCorner.right;
        }
        if (!values.m_bSizePercentCY) {
            UiPadding rcShadowCorner = param.m_rcShadowCorner;
            dpi.ScaleWindowSize(rcShadowCorner.top);
            dpi.ScaleWindowSize(rcShadowCorner.bottom);
            cy += rcShadowCorner.top + rcShadowCorner.bottom;
        }
    }
    AttributeUtil::ValidateWindowSize(param.m_pWindow, cx, cy);
    initSize.cx = cx;
    initSize.cy = cy;
    return initSize;
}

} // namespace ui
