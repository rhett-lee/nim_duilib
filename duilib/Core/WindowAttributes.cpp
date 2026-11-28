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
    /** 推导默认阴影类型（不依赖窗口状态）
    *   规则：
    *   1. 窗口已经使用分层窗口（或SDL透明窗口）时，使用自绘默认阴影，避免系统阴影与分层窗口混用；
    *   2. 系统支持系统阴影时，使用系统阴影；
    *   3. 否则使用自绘默认阴影（圆角）。
    */
    ShadowType GetDefaultShadowTypeForResolve(const ShadowResolveParam& param)
    {
        if (param.m_bLayeredWindow && (param.m_bLayeredRequiredForSelfDrawnShadow || param.m_bSdlTransparent)) {
            //分层窗口，默认为自绘阴影，圆角
            return ShadowType::kShadowBigRound;
        }
        if (param.m_bSystemShadowSupported) {
            //该窗口支持系统阴影
            return ShadowType::kShadowSystemDefault;
        }
        //系统不支持系统阴影：使用自绘默认阴影（圆角）
        return ShadowType::kShadowBigRound;
    }

    /** 阴影请求是否要求开启分层窗口（Windows非SDL下自绘阴影必须使用分层窗口）
    */
    bool IsShadowRequestNeedLayeredWindow(const WindowAttributeValues& values, const ShadowResolveParam& param)
    {
        if (!param.m_bLayeredRequiredForSelfDrawnShadow || param.m_bGlBackendActive) {
            return false;
        }
        if (values.m_bShadowAttachedDefined && !values.m_bShadowAttached) {
            return false;
        }
        ShadowType nShadowType = values.m_bShadowTypeDefined ? values.m_nShadowType : ShadowType::kShadowDefault;
        if (nShadowType == ShadowType::kShadowDefault) {
            nShadowType = GetDefaultShadowTypeForResolve(param);
        }
        nShadowType = Shadow::GetSupportedShadowType(nShadowType, param.m_bSystemShadowSupported);
        return Shadow::IsShadowTypeNeedLayeredWindow(nShadowType);
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

    /** 记录窗口配置的告警信息（属性值非法或配置冲突时的纠正说明，由调用方统一输出）
    */
    void AddAttributeWarning(std::vector<DString>* pWarnings, const DString& strAttribute,
                             const DString& strValue, const DString& strReason)
    {
        if (pWarnings == nullptr) {
            return;
        }
        pWarnings->push_back(StringUtil::Printf(_T("attribute=\"%s\", value=\"%s\": %s"),
                                                strAttribute.c_str(), strValue.c_str(), strReason.c_str()));
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
    applyAttribute = [&values, pWindow](const DString& strName, const DString& strValue2) {
        //窗口属性支持变量展开（与控件属性一致），如：caption="0,0,0,${SIZE_TITLEBAR_HEIGHT}"
        const DString strValue = GlobalManager::Instance().GetExpandVarStrings(strValue2);
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
            if ((values.m_szRoundCorner.cx == 0) != (values.m_szRoundCorner.cy == 0)) {
                //两个参数要么同时等于0，要么同时大于0
                AddAttributeWarning(&values.m_warnings, strName, strValue,
                                    _T("the two values must be both zero or both positive, ignored"));
            }
            else {
                values.m_bRoundCornerDefined = true;
            }
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
            else {
                AddAttributeWarning(&values.m_warnings, strName, strValue, _T("invalid value, ignored"));
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
            else {
                AddAttributeWarning(&values.m_warnings, strName, strValue, _T("value out of range [0,255], ignored"));
            }
        }
        else if (StringUtil::IsEqualNoCase(strName, _T("opacity"))) {
            //设置窗口的不透明度（0 - 255），该值在SetLayeredWindowAttributes函数中作为参数使用
            const int32_t nOpacity = StringUtil::StringToInt32(strValue);
            if ((nOpacity >= 0) && (nOpacity <= 255)) {
                values.m_nOpacity = nOpacity;
                values.m_bOpacityDefined = true;
            }
            else {
                AddAttributeWarning(&values.m_warnings, strName, strValue, _T("value out of range [0,255], ignored"));
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
                //无效的配置值：按CPU绘制处理，并记录告警
                ASSERT(0);
                AddAttributeWarning(&values.m_warnings, strName, strValue,
                                    _T("invalid value, use CPU rendering"));
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
                AddAttributeWarning(&values.m_warnings, strName, strValue,
                                    _T("negative value, reset to 0"));
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
        const std::vector<DString> classList = StringUtil::Split(strClassList, _T(" \t\r\n"));
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
                                         const ShadowResolveParam& param,
                                         std::vector<DString>* pWarnings)
{
    ShadowAttributes shadow;

    //使用系统标题栏时，有效阴影必须关闭（请求值保留）
    const bool bUseSystemCaption = values.m_bUseSystemCaptionDefined && values.m_bUseSystemCaption;
    shadow.m_bShadowAttached = (!values.m_bShadowAttachedDefined || values.m_bShadowAttached) && !bUseSystemCaption;
    if (bUseSystemCaption && (!values.m_bShadowAttachedDefined || values.m_bShadowAttached)) {
        AddAttributeWarning(pWarnings, _T("shadow_attached"), _T("true"),
                            _T("conflicts with use_system_caption=\"true\", shadow is temporarily disabled, "
                               "the configured value is preserved"));
    }

    //推导有效的阴影类型
    shadow.m_nShadowType = values.m_bShadowTypeDefined ? values.m_nShadowType : ShadowType::kShadowDefault;
    if (shadow.m_nShadowType == ShadowType::kShadowDefault) {
        shadow.m_nShadowType = GetDefaultShadowTypeForResolve(param);
    }
    //不支持系统阴影时，转换为实际支持的自绘阴影类型
    shadow.m_nShadowType = Shadow::GetSupportedShadowType(shadow.m_nShadowType, param.m_bSystemShadowSupported);

    //GL有效时，不允许使用自绘阴影：切换为系统阴影（支持时）或None
    if (shadow.m_bShadowAttached && param.m_bGlBackendActive &&
        Shadow::IsShadowTypeNeedLayeredWindow(shadow.m_nShadowType)) {
        shadow.m_nShadowType = param.m_bSystemShadowSupported ? ShadowType::kShadowSystemDefault
                                                              : ShadowType::kShadowNone;
        AddAttributeWarning(pWarnings, _T("shadow_type"), _T("self_drawn"),
                            _T("conflicts with render_backend_type=\"GL\", switched to a supported shadow type, "
                               "the configured value is preserved"));
    }

    //Windows非SDL：分层窗口被关闭时，自绘阴影无法绘制，有效阴影关闭（请求值保留）
    if (shadow.m_bShadowAttached && param.m_bLayeredRequiredForSelfDrawnShadow && !param.m_bLayeredWindow &&
        Shadow::IsShadowTypeNeedLayeredWindow(shadow.m_nShadowType)) {
        shadow.m_bShadowAttached = false;
        AddAttributeWarning(pWarnings, _T("shadow_attached"), _T("true"),
                            _T("requires a layered window, shadow is temporarily disabled, "
                               "the configured value is preserved"));
    }

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
                                                       const ShadowResolveParam& param,
                                                       std::vector<DString>* pWarnings)
{
    LayeredWindowAttributes result;
    const bool bSdl = param.m_bSdlTransparent;
    const bool bUseSystemCaption = values.m_bUseSystemCaptionDefined && values.m_bUseSystemCaption;
    const bool bAlphaRequest = values.m_bAlphaDefined && (values.m_nAlpha < 255);
    const bool bOpacityRequest = values.m_bOpacityDefined && (values.m_nOpacity < 255);

    bool bDefined = values.m_bLayeredWindowDefined;
    bool bLayered = values.m_bLayeredWindow;
    if (!bDefined) {
        if (bSdl) {
            //SDL：layered_window 表示 SDL_WINDOW_TRANSPARENT，未显式配置时默认开启
            bDefined = true;
            bLayered = param.m_bSdlTransparentSupported;
            if (!bLayered) {
                AddAttributeWarning(pWarnings, _T("layered_window"), _T("true"),
                                    _T("the current platform or renderer does not support transparent windows, "
                                       "the configured value is preserved"));
            }
        }
        else if (param.m_bGlBackendActive) {
            //GL有效：不允许分层窗口
            bDefined = true;
            bLayered = false;
        }
        else if (bUseSystemCaption) {
            //使用系统标题栏：默认不开启分层窗口
            bDefined = true;
            bLayered = false;
        }
        else if (bOpacityRequest || bAlphaRequest) {
            //使用opacity/alpha：默认需要开启分层窗口
            bDefined = true;
            bLayered = true;
        }
        else if (IsShadowRequestNeedLayeredWindow(values, param)) {
            //阴影类型需要分层窗口
            bDefined = true;
            bLayered = true;
        }
    }

    //冲突处理：高优先级条件关闭分层窗口的有效值，请求值保留
    if (!bSdl && bUseSystemCaption && bLayered) {
        if (values.m_bLayeredWindowDefined && values.m_bLayeredWindow) {
            AddAttributeWarning(pWarnings, _T("layered_window"), _T("true"),
                                _T("conflicts with use_system_caption=\"true\", layered window is disabled, "
                                   "the configured value is preserved"));
        }
        bLayered = false;
    }
    if (!bSdl && param.m_bGlBackendActive && bLayered) {
        AddAttributeWarning(pWarnings, _T("layered_window"), _T("true"),
                            _T("conflicts with render_backend_type=\"GL\", layered window is disabled, "
                               "the configured value is preserved"));
        bLayered = false;
    }
    if (bSdl && bLayered && !param.m_bSdlTransparentSupported) {
        AddAttributeWarning(pWarnings, _T("layered_window"), _T("true"),
                            _T("transparent windows are not supported, the configured value is preserved"));
        bLayered = false;
    }

    result.m_bIsLayeredWindow = bLayered;
    result.m_bIsLayeredWindowDefined = bDefined;

    //透明度/不透明度：只记录需要设置的有效值
    bool bAlphaEffective = bAlphaRequest;
    bool bOpacityEffective = bOpacityRequest;
    if (bSdl) {
        //SDL：opacity直接映射SDL_SetWindowOpacity，不依赖透明窗口；alpha依赖透明窗口
        if (!bLayered && bAlphaEffective) {
            AddAttributeWarning(pWarnings, _T("alpha"), StringUtil::Printf(_T("%d"), values.m_nAlpha),
                                _T("requires a transparent window, ignored, the configured value is preserved"));
            bAlphaEffective = false;
        }
    }
    else if (!bLayered) {
        //Windows非SDL：透明度/不透明度都要求分层窗口
        if (bAlphaEffective) {
            AddAttributeWarning(pWarnings, _T("alpha"), StringUtil::Printf(_T("%d"), values.m_nAlpha),
                                _T("requires a layered window, ignored, the configured value is preserved"));
            bAlphaEffective = false;
        }
        if (bOpacityEffective) {
            AddAttributeWarning(pWarnings, _T("opacity"), StringUtil::Printf(_T("%d"), values.m_nOpacity),
                                _T("requires a layered window, ignored, the configured value is preserved"));
            bOpacityEffective = false;
        }
    }
    if (bAlphaEffective) {
        result.m_nAlpha = values.m_nAlpha;
        result.m_bAlphaDefined = true;
    }
    if (bOpacityEffective) {
        result.m_nOpacity = values.m_nOpacity;
        result.m_bOpacityDefined = true;
    }
    return result;
}

RenderBackendAttributes ResolveRenderBackendAttributes(const WindowAttributeValues& values,
                                                       const RenderBackendResolveParam& param,
                                                       std::vector<DString>* pWarnings)
{
    RenderBackendAttributes result;
    //逐字段解析：XML > 默认配置（全局配置）> 内置默认值
    result.m_backendType = values.m_bRenderBackendTypeDefined ? values.m_renderBackendType
                                                              : param.m_defaultBackendType;
    result.m_nMsaaSampleCount = values.m_bMsaaDefined ? values.m_nMsaaSampleCount
                                                      : param.m_nDefaultMsaaSampleCount;
    //XML中的vsync属性为true表示开启垂直同步
    result.m_bDisableVsync = values.m_bVsyncDefined ? !values.m_bVsync
                                                    : param.m_bDefaultDisableVsync;

    //平台/构建不支持GL时，在Resolve阶段归一为CPU，并记录告警（避免在SetRenderBackendType中静默修改）
    if ((result.m_backendType == RenderBackendType::kNativeGL_BackendType) && !param.m_bGlSupported) {
        result.m_backendType = RenderBackendType::kRaster_BackendType;
        result.m_nMsaaSampleCount = 0;
        result.m_bDisableVsync = false;
        result.m_bFallbackToRaster = true;
        AddAttributeWarning(pWarnings, _T("render_backend_type"), _T("GL"),
                            _T("GL backend is not supported in this build/platform, use CPU rendering, "
                               "the configured value is preserved"));
    }
    return result;
}

ResolvedWindowAttributes ResolveWindowAttributes(const WindowAttributeValues& values,
                                                 const RenderBackendResolveParam& renderParam,
                                                 const ShadowResolveParam& param,
                                                 std::vector<DString>* pWarnings)
{
    ResolvedWindowAttributes resolved;

    //【步骤1/2】渲染后端：最高优先级的有效结论，后续冲突以它为准
    resolved.m_render = ResolveRenderBackendAttributes(values, renderParam, pWarnings);

    ShadowResolveParam shadowParam = param;
    shadowParam.m_bGlBackendActive = (resolved.m_render.m_backendType == RenderBackendType::kNativeGL_BackendType);

    //【步骤2】分层窗口：按请求值与渲染后端确定有效值
    resolved.m_layered = ResolveLayeredWindowAttributes(values, shadowParam, pWarnings);

    //【步骤3】阴影：使用最终的分层窗口状态判断自绘阴影是否可用
    shadowParam.m_bLayeredWindow = resolved.m_layered.m_bIsLayeredWindow;
    resolved.m_shadow = ResolveShadowAttributes(values, shadowParam, pWarnings);
    return resolved;
}

UiSize ResolveWindowInitSize(const WindowAttributeValues& values,
                             const WindowInitSizeResolveParam& param,
                             const DpiManager& dpi,
                             std::vector<DString>* pWarnings)
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

    //min_size大于max_size属于配置冲突：忽略max_size，保证最小尺寸约束不被突破
    if ((minSize.cx > 0) && (maxSize.cx > 0) && (minSize.cx > maxSize.cx)) {
        AddAttributeWarning(pWarnings, _T("max_size"), StringUtil::Printf(_T("%d"), maxSize.cx),
                            _T("is smaller than min_size, ignored"));
        maxSize.cx = 0;
    }
    if ((minSize.cy > 0) && (maxSize.cy > 0) && (minSize.cy > maxSize.cy)) {
        AddAttributeWarning(pWarnings, _T("max_size"), StringUtil::Printf(_T("%d"), maxSize.cy),
                            _T("is smaller than min_size, ignored"));
        maxSize.cy = 0;
    }

    bool bClamped = false;
    if ((minSize.cx > 0) && (cx < minSize.cx)) {
        cx = minSize.cx;
        bClamped = true;
    }
    if ((maxSize.cx > 0) && (cx > maxSize.cx)) {
        cx = maxSize.cx;
        bClamped = true;
    }
    if ((minSize.cy > 0) && (cy < minSize.cy)) {
        cy = minSize.cy;
        bClamped = true;
    }
    if ((maxSize.cy > 0) && (cy > maxSize.cy)) {
        cy = maxSize.cy;
        bClamped = true;
    }
    if (bClamped) {
        AddAttributeWarning(pWarnings, _T("size"), StringUtil::Printf(_T("%d,%d"), values.m_szSize.cx, values.m_szSize.cy),
                            _T("clamped to the min_size/max_size range"));
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
