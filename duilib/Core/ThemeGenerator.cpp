#include "ThemeGenerator.h"
#include "duilib/Utils/StringConvert.h"

#include "duilib/third_party/xml/pugixml.hpp"

#include <sstream>
#include <iomanip>
#include <fstream>

namespace ui
{

ThemeGenerator::ThemeGenerator()
    : m_hue(143.24)
    , m_base(0.0143)
    , m_isDark(false)
    // 背景色参数
    , m_bgLightL(0.97)
    , m_bgLightLScale(-0.95)
    , m_bgDarkL(0.17)
    , m_bgDarkLScale(0.78)
    , m_bgBaseChroma(1.0)
    // 前景色参数
    , m_fgLightL(0.22)
    , m_fgLightLScale(0.72)
    , m_fgDarkL(0.92)
    , m_fgDarkLScale(-0.75)
    , m_fgBaseChroma(0.15)
    // Surface层参数
    , m_surfaceLightOffset(0.025)
    , m_surfaceDarkOffset(0.07)
    , m_surfaceBaseChroma(1.0)
    // Accent参数
    , m_accentLightL(0.6204)
    , m_accentDarkL(0.68)
    , m_accentC(0.195)
    // 带彩色选项（默认全部为true）
    , m_bgColored(true)
    , m_fgColored(true)
    , m_surfaceColored(true)
{
}

ThemeGenerator::~ThemeGenerator()
{
}

void ThemeGenerator::SetBgParams(double bgLightL, double bgDarkL, double bgBaseChroma)
{
    m_bgLightL = bgLightL;
    m_bgDarkL = bgDarkL;
    m_bgBaseChroma = bgBaseChroma;
}

void ThemeGenerator::SetFgParams(double fgLightL, double fgDarkL, double fgBaseChroma)
{
    m_fgLightL = fgLightL;
    m_fgDarkL = fgDarkL;
    m_fgBaseChroma = fgBaseChroma;
}

void ThemeGenerator::SetSurfaceParams(double surfaceLightOffset, double surfaceDarkOffset, double surfaceBaseChroma)
{
    m_surfaceLightOffset = surfaceLightOffset;
    m_surfaceDarkOffset = surfaceDarkOffset;
    m_surfaceBaseChroma = surfaceBaseChroma;
}

void ThemeGenerator::SetAccentParams(double accentLightL, double accentDarkL, double accentC)
{
    m_accentLightL = accentLightL;
    m_accentDarkL = accentDarkL;
    m_accentC = accentC;
}

void ThemeGenerator::GetBgParams(double& bgLightL, double& bgDarkL, double& bgBaseChroma) const
{
    bgLightL = m_bgLightL;
    bgDarkL = m_bgDarkL;
    bgBaseChroma = m_bgBaseChroma;
}

void ThemeGenerator::GetFgParams(double& fgLightL, double& fgDarkL, double& fgBaseChroma) const
{
    fgLightL = m_fgLightL;
    fgDarkL = m_fgDarkL;
    fgBaseChroma = m_fgBaseChroma;
}

void ThemeGenerator::GetSurfaceParams(double& surfaceLightOffset, double& surfaceDarkOffset, double& surfaceBaseChroma) const
{
    surfaceLightOffset = m_surfaceLightOffset;
    surfaceDarkOffset = m_surfaceDarkOffset;
    surfaceBaseChroma = m_surfaceBaseChroma;
}

void ThemeGenerator::GetAccentParams(double& accentLightL, double& accentDarkL, double& accentC) const
{
    accentLightL = m_accentLightL;
    accentDarkL = m_accentDarkL;
    accentC = m_accentC;
}

void ThemeGenerator::SetBgColored(bool colored)
{
    m_bgColored = colored;
}

bool ThemeGenerator::IsBgColored() const
{
    return m_bgColored;
}

void ThemeGenerator::SetFgColored(bool colored)
{
    m_fgColored = colored;
}

bool ThemeGenerator::IsFgColored() const
{
    return m_fgColored;
}

void ThemeGenerator::SetSurfaceColored(bool colored)
{
    m_surfaceColored = colored;
}

bool ThemeGenerator::IsSurfaceColored() const
{
    return m_surfaceColored;
}

void ThemeGenerator::ResetParams()
{
    // 重置所有参数为默认值
    SetBgParams(0.97, 0.17, 1.0);
    SetFgParams(0.22, 0.92, 0.15);
    SetSurfaceParams(0.025, 0.07, 1.0);
    SetAccentParams(0.6204, 0.68, 0.195);
    // 重置带彩色选项为默认值
    m_bgColored = true;
    m_fgColored = true;
    m_surfaceColored = true;
}

std::string ThemeGenerator::GetBackgroundColor(double hue, double base, bool isDark)
{
    // 使用成员变量计算背景色
    double baseChroma = m_bgColored ? (base * m_bgBaseChroma) : 0.0;
    // 实际使用的色相值，无彩色时色相不影响结果
    double effectiveHue = m_bgColored ? hue : 0.0;

    double bgL;
    if (isDark) {
        bgL = m_bgDarkL + base * m_bgDarkLScale;
    }
    else {
        bgL = m_bgLightL + base * m_bgLightLScale;
    }

    return ColorConverter::OKLCHToARGB(bgL, baseChroma, effectiveHue, 255);
}

std::string ThemeGenerator::GetForegroundColor(double hue, double base, bool isDark)
{
    // 使用成员变量计算前景色
    double fgChroma = m_fgColored ? (base * m_fgBaseChroma) : 0.0;
    double effectiveHue = m_fgColored ? hue : 0.0;

    double fgL;
    if (isDark) {
        fgL = m_fgDarkL + base * m_fgDarkLScale;
    }
    else {
        fgL = m_fgLightL + base * m_fgLightLScale;
    }

    return ColorConverter::OKLCHToARGB(fgL, fgChroma, effectiveHue, 255);
}

std::string ThemeGenerator::GetSurfaceColor(double hue, double base, bool isDark, int surfaceLevel)
{
    // 使用成员变量计算Surface颜色
    double surfaceChroma = m_surfaceColored ? (base * m_surfaceBaseChroma) : 0.0;
    double effectiveHue = m_surfaceColored ? hue : 0.0;

    double bgL;
    if (isDark) {
        bgL = m_bgDarkL + base * m_bgDarkLScale;
    }
    else {
        bgL = m_bgLightL + base * m_bgLightLScale;
    }

    // 浅色模式: offset为负，表面色随层级增加而变暗
    // 深色模式: offset为正，表面色随层级增加而变亮
    double offset = isDark ? m_surfaceDarkOffset : -m_surfaceLightOffset;
    double sfL = bgL + offset * surfaceLevel;

    return ColorConverter::OKLCHToARGB(sfL, surfaceChroma, effectiveHue, 255);
}

std::string ThemeGenerator::GetStateColor(const std::string& baseColor, const std::string& state, bool isDark) const
{
    if (baseColor.empty()) {
        return baseColor;
    }

    uint8_t alpha, r, g, b;
    if (!ColorConverter::ParseHexColor(baseColor, alpha, r, g, b)) {
        return baseColor;
    }

    double L, C, H;
    if (!ColorConverter::RGBToOKLCH(r, g, b, L, C, H)) {
        return baseColor;
    }

    if (state == "hovered") {
        if (!isDark) {
            L = std::max(0.0, std::min(1.0, L - 0.06));
        }
        else {
            L = std::max(0.0, std::min(1.0, L + 0.06));
        }
    }
    else if (state == "pressed") {
        if (!isDark) {
            L = std::max(0.0, std::min(1.0, L - 0.10));
        }
        else {
            L = std::max(0.0, std::min(1.0, L + 0.10));
        }
    }
    else if (state == "selected") {
        if (!isDark) {
            L = std::max(0.0, std::min(1.0, L - 0.09));
        }
        else {
            L = std::max(0.0, std::min(1.0, L + 0.09));
        }
    }
    else if ((state == "disabled") || (state == "prompt")) {
        if (!isDark) {
            L = std::max(0.0, std::min(1.0, L + 0.05));
            C = std::max(0.0, C - 0.4);
        }
        else {
            L = std::max(0.0, std::min(1.0, L - 0.05));
            C = std::max(0.0, C - 0.2);
        }
    }
    else if (state == "focused") {
        //强调色
        return GetGeneratedColor("--accent");
    }
    else {
        return baseColor;
    }

    return ColorConverter::OKLCHToARGB(L, C, H, alpha);
}

std::string ThemeGenerator::ApplyAdjustments(const std::string& baseColor, const std::string& adjustStr) const
{
    // 非法颜色或公式为空时直接返回原颜色
    if (baseColor.empty() || adjustStr.empty()) {
        return baseColor;
    }

    // 解析 ARGB 格式
    uint8_t alpha, r, g, b;
    if (!ColorConverter::ParseHexColor(baseColor, alpha, r, g, b)) {
        return baseColor;
    }

    // 解析 alpha 为 0~1 范围
    double currentAlpha = alpha / 255.0;

    // RGB 转 HSL（注意：使用 HSL 而不是 OKLCH，与 JS 算法保持一致）
    double h, s, l;
    ColorConverter::RGBToHSL(r, g, b, h, s, l);

    // 当前 RGB 值（用于 invert 操作）
    uint8_t currentR = r, currentG = g, currentB = b;

    // 解析公式，按逗号分隔得到多个操作
    std::istringstream adjustStream(adjustStr);
    std::string adjustment;
    while (std::getline(adjustStream, adjustment, ',')) {
        // 去除首尾空白
        size_t start = adjustment.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        size_t end = adjustment.find_last_not_of(" \t");
        adjustment = adjustment.substr(start, end - start + 1);

        if (adjustment.empty()) continue;

        // 解析属性名（lightness / saturation / hue / alpha / invert）
        // 和运算符（+ / - / * / =）以及百分比数值
        char op = '+';
        std::string property;
        std::string valueStr;

        size_t colonPos = adjustment.find(':');
        if (colonPos != std::string::npos) {
            // 格式: "property:opvalue" 或 "property: value"
            property = adjustment.substr(0, colonPos);
            valueStr = adjustment.substr(colonPos + 1);

            // 去除属性名前后空白
            size_t pStart = property.find_first_not_of(" \t");
            size_t pEnd = property.find_last_not_of(" \t");
            if (pStart != std::string::npos) {
                property = property.substr(pStart, pEnd - pStart + 1);
            }

            // 去除数值前后空白
            size_t vStart = valueStr.find_first_not_of(" \t");
            size_t vEnd = valueStr.find_last_not_of(" \t");
            if (vStart != std::string::npos) {
                valueStr = valueStr.substr(vStart, vEnd - vStart + 1);
            }

            // 提取运算符
            if (!valueStr.empty()) {
                op = valueStr[0];
                if (op == '+' || op == '-' || op == '*' || op == '=') {
                    valueStr = valueStr.substr(1);
                    // 去除数值前空白
                    size_t vvStart = valueStr.find_first_not_of(" \t");
                    if (vvStart != std::string::npos) {
                        valueStr = valueStr.substr(vvStart);
                    }
                }
            }
        }
        else {
            // 没有冒号，可能是 "invert" 这样的无值属性
            property = adjustment;
        }

        if (property.empty()) continue;

        // invert 是无参数操作，直接反转 RGB 三通道
        if (property == "invert") {
            currentR = 255 - currentR;
            currentG = 255 - currentG;
            currentB = 255 - currentB;
            // 重新计算 HSL
            ColorConverter::RGBToHSL(currentR, currentG, currentB, h, s, l);
            continue;
        }

        // 其他属性需要数值
        double percent = 0.0;
        if (!valueStr.empty()) {
            try {
                percent = std::stod(valueStr);
            }
            catch (...) {
                continue;
            }
        }

        // 归一化为 0~1 范围
        double delta = percent / 100.0;

        // 应用操作（+ - * =）
        if (property == "lightness") {
            // 亮度：0~100% → 内部 0~1
            double newValue = l;
            switch (op) {
                case '+': newValue = l + delta; break;
                case '-': newValue = l - delta; break;
                case '*': newValue = l * delta; break;
                case '=': newValue = delta; break;
            }
            l = std::max(0.0, std::min(1.0, newValue));
        }
        else if (property == "saturation") {
            // 饱和度：0~100% → 内部 0~1
            double newValue = s;
            switch (op) {
                case '+': newValue = s + delta; break;
                case '-': newValue = s - delta; break;
                case '*': newValue = s * delta; break;
                case '=': newValue = delta; break;
            }
            s = std::max(0.0, std::min(1.0, newValue));
        }
        else if (property == "alpha") {
            // 透明度：0~100% → 内部 0~1
            double newValue = currentAlpha;
            switch (op) {
                case '+': newValue = currentAlpha + delta; break;
                case '-': newValue = currentAlpha - delta; break;
                case '*': newValue = currentAlpha * delta; break;
                case '=': newValue = delta; break;
            }
            currentAlpha = std::max(0.0, std::min(1.0, newValue));
        }
        else if (property == "hue") {
            // 色相：0~100% → 内部 0~360
            // 公式 0~100% → 内部映射为 0~360°
            double hueNormalized = h / 360.0;
            double newValue = hueNormalized;
            switch (op) {
                case '+': newValue = hueNormalized + delta; break;
                case '-': newValue = hueNormalized - delta; break;
                case '*': newValue = hueNormalized * delta; break;
                case '=': newValue = delta; break;
            }
            // 强制夹紧到 [0, 1)
            newValue = std::max(0.0, std::min(0.999999, newValue));
            h = newValue * 360.0;
        }
    }

    // 转换回 RGB
    uint8_t finalR, finalG, finalB;
    ColorConverter::HSLToRGB(h, s, l, finalR, finalG, finalB);

    // Alpha 限制在 0~1（对应 0~255）
    currentAlpha = std::max(0.0, std::min(1.0, currentAlpha));
    uint8_t finalAlpha = static_cast<uint8_t>(std::round(currentAlpha * 255.0));

    return ColorConverter::RGBToHex(finalAlpha, finalR, finalG, finalB);
}

std::pair<std::string, std::string> ThemeGenerator::DetectColorState(const std::string& colorName) const
{
    std::vector<std::string> states = {"_default", "_normal", "_disabled", "_pressed", "_hovered", "_selected", "_prompt" , "_focused" };
    for (const auto& state : states) {
        size_t pos = colorName.rfind(state);
        if (pos != std::string::npos) {
            std::string baseName = colorName.substr(0, pos);
            std::string actualState = state.substr(1);
            return std::make_pair(actualState, baseName);
        }
    }
    return std::make_pair("", colorName);
}

std::string ThemeGenerator::EnsureContrast(const std::string& textColor, const std::string& bgColor, double minContrast) const
{
    uint8_t alpha, r, g, b;
    if (!ColorConverter::ParseHexColor(textColor, alpha, r, g, b)) {
        return textColor;
    }

    double L, C, H;
    if (!ColorConverter::RGBToOKLCH(r, g, b, L, C, H)) {
        return textColor;
    }

    uint8_t bgAlpha, bgR, bgG, bgB;
    if (!ColorConverter::ParseHexColor(bgColor, bgAlpha, bgR, bgG, bgB)) {
        return textColor;
    }

    double bgLuminance = ColorConverter::GetRelativeLuminance(bgR, bgG, bgB);

    double contrast = ColorConverter::CalculateContrastRatio(textColor, bgColor);
    if (contrast < 0.01) {
        return textColor;
    }

    if (contrast >= minContrast) {
        return textColor;
    }

    // 根据背景明度确定调整方向：
    // 浅色背景(bgLuminance > 0.5)：需要让文字更暗(L减小)
    // 深色背景(bgLuminance <= 0.5)：需要让文字更亮(L增大)
    const bool bgIsLight = (bgLuminance > 0.5);
    const double LStep = bgIsLight ? -0.05 : 0.05;

    const int maxIterations = 50;
    int iteration = 0;

    while (iteration < maxIterations) {
        // 调整L值
        L = L + LStep;

        // 钳制到 [0, 1] 范围
        if (L < 0.0) L = 0.0;
        if (L > 1.0) L = 1.0;

        // 如果已经达到目标方向极端值，停止迭代
        if (bgIsLight && L <= 0.0) break;
        if (!bgIsLight && L >= 1.0) break;

        std::string testColor = ColorConverter::OKLCHToARGB(L, C, H, alpha);
        contrast = ColorConverter::CalculateContrastRatio(testColor, bgColor);

        if (contrast >= minContrast) {
            return testColor;
        }

        // 如果已无法再继续调整（到达极限），停止
        if ((bgIsLight && L <= 0.0) || (!bgIsLight && L >= 1.0)) {
            break;
        }

        iteration++;
    }

    std::string finalColor = ColorConverter::OKLCHToARGB(L, C, H, alpha);
    return finalColor;
}

void ThemeGenerator::GenerateThemeColors(double hue, double base, bool isDark)
{
    m_hue = hue;
    m_base = base;
    m_isDark = isDark;
    m_generatedColors.clear();

    // =========================================================================
    // 核心颜色计算
    // =========================================================================
    // 遵循《UI主题颜色生成规范》：
    //   1) 主题采用7级背景色等级，浅色主题从最浅(#FFFFFF)到最深(#D6D9DD)【深色主题则相反】
    //   2) 数字越小颜色越浅，数字越大颜色越深
    //   3) 同一等级内所有控件/区域使用完全相同的背景色
    //   4) 编辑框最浅，标题栏最深
    //   5) 深色主题沿用相同等级，颜色深度方向相反（数字越大颜色越浅）
    // =========================================================================

    // Surface层（7级背景色等级）
    // 浅色模式：level越大颜色越深；深色模式：level越大颜色越浅
    std::string surface1 = GetSurfaceColor(hue, base, isDark, -3); //比窗口颜色浅的更多
    std::string surface2 = GetSurfaceColor(hue, base, isDark, -1); //比窗口颜色浅
    std::string surface3 = GetBackgroundColor(hue, base, isDark);  //窗口背景，为标准色
    std::string surface4 = GetSurfaceColor(hue, base, isDark, 1);
    std::string surface5 = GetSurfaceColor(hue, base, isDark, 2);
    std::string surface6 = GetSurfaceColor(hue, base, isDark, 3);
    std::string surface7 = GetSurfaceColor(hue, base, isDark, 4);

    // 等级3（主窗口/对话框客户区背景、Tab页面内容区域）
    std::string windowBg = surface3;

    // 前景色：与最深背景色形成对比的文字颜色
    std::string foregroundColor = GetForegroundColor(hue, base, isDark);

    // Accent颜色（强调色）：用于按钮、链接等关键元素
    double accentL = isDark ? m_accentDarkL : m_accentLightL;
    std::string accentColor = ColorConverter::OKLCHToARGB(accentL, m_accentC, hue, 255);

    // Accent前景色：浅色模式接近白色，深色模式接近黑色
    double accentFgL = isDark ? 0.14 : 0.99;
    std::string accentForeground = ColorConverter::OKLCHToARGB(accentFgL, 0, hue, 255);

    // =========================================================================
    // 写入颜色到 m_generatedColors
    // =========================================================================

    // 7级背景色
    m_generatedColors["--surface_1"] = surface1;
    m_generatedColors["--surface_2"] = surface2;
    m_generatedColors["--surface_3"] = surface3;
    m_generatedColors["--surface_4"] = surface4;
    m_generatedColors["--surface_5"] = surface5;
    m_generatedColors["--surface_6"] = surface6;
    m_generatedColors["--surface_7"] = surface7;
    m_generatedColors["--background"] = windowBg;                //窗口的被景色
    m_generatedColors["--foreground"] = foregroundColor;         //窗口的前景色
    m_generatedColors["--accent"] = accentColor;                 //强调色
    m_generatedColors["--accent_foreground"] = accentForeground; //强调色的前景色
    m_generatedColors["--success"] = ColorConverter::OKLCHToARGB(0.65, 0.16, 152, 255);  // 成功色
    m_generatedColors["--warning"] = ColorConverter::OKLCHToARGB(0.68, 0.18, 80, 255);   // 警告色
    m_generatedColors["--error"] = ColorConverter::OKLCHToARGB(0.65, 0.18, 25, 255);     // 失败/错误/危险色

    // =========================================================================
    // 筛选出需要处理的颜色（去除基础色/派生色/固定色）
    // =========================================================================
    std::map<std::string, ThemeColorConfig> generatedConfigs;
    for (const auto& pair : m_loadedConfigs) {
        const std::string& colorName = pair.first;
        const ThemeColorConfig& attrs = pair.second;
        if (attrs.fixed) {
            //固定属性，不需要生成颜色
            //continue;
        }
        if (attrs.category == "basic_color") {
            //基础色，不需要生成颜色
            continue;
        }
        if (!attrs.derived_from.empty()) {
            //继承自其他颜色的颜色，不需要生成颜色
            continue;
        }
        if (!attrs.m_state.empty()) {
            //有状态的颜色，按BaseName生成
            generatedConfigs[attrs.m_baseName] = attrs;
            if ((attrs.m_state == "normal") || (attrs.m_state == "default")) {
                generatedConfigs[colorName] = attrs; //仅保留正常状态的颜色值
            }
        }
        else {
            generatedConfigs[colorName] = attrs;
        }
    }

    // 根据规则生成颜色(对于含有状态的颜色，只生成BaseName的颜色值)
    for (const auto& pair : generatedConfigs) {
        const std::string& colorName = pair.first;
        const ThemeColorConfig& attrs = pair.second;
        std::string colorValue;
        if (attrs.support_accent) {
            //支持强调色的，使用强调色
            colorValue = accentColor;
        }
        else if (attrs.category == "bg_color") {
            //未明确分类的背景色，默认使用主窗口背景
            colorValue = windowBg;
        }
        else if (attrs.category == "border_color") {
            //未明确分类的边框色，使用等级4
            colorValue = surface4;
        }
        else if (attrs.category == "text_color") {
            //文本颜色使用前景色
            colorValue = foregroundColor;
        }
        if (!colorValue.empty()) {
            if (!attrs.m_state.empty()) {
                m_generatedColors[attrs.m_baseName] = colorValue;
            }
            else {
                m_generatedColors[colorName] = colorValue;
            }            
        }
    }

    // =========================================================================
    // 根据7级规范分配背景色到具体控件(对于含有状态的颜色，只生成BaseName的颜色值)
    // =========================================================================
    // 规范等级    颜色变量     典型色值     控件/区域
    // ----------------------------------------------------------------
    // 等级1(最浅) surface0    #FFFFFF     编辑框 Edit/LineEdit, RichEdit, ComboBox编辑区, CheckBox方框, RadioButton圆圈
    // 等级2       surface1    #F8F9FA     列表 ListCtrl/ListView, 树 TreeView, 列表框 ListBox, 数据表格 Grid/DataGrid
    // 等级3       surface2    #F0F0F0     主窗口/对话框客户区背景, Tab页面内容区域
    // 等级4       surface3    #EDEFF2     通用容器面板 Panel/Widget, GroupBox内部区域
    // 等级5       surface4    #E5E7EB     Button默认态, Switch, Progress背景, Slider槽, ComboBox整体背景, StatusBar
    // 等级6       surface5    #DFE1E5     MenuBar, ToolBar, Tab标签条背景
    // 等级7(最深)  surface6    #D6D9DD     窗口标题栏 TitleBar/Caption
    // ----------------------------------------------------------------

    const size_t nGeneratedColorsCount = m_generatedColors.size();

    // 等级1：编辑框区域（最浅，#FFFFFF）
    m_generatedColors["bg_richedit"] = surface1;

    // 等级2：列表/树/表格
    m_generatedColors["bg_list"] = surface2;
    m_generatedColors["bg_list_item"] = surface2;

    m_generatedColors["bg_list_ctrl"] = surface2;
    m_generatedColors["bg_list_ctrl_view"] = surface2;
    m_generatedColors["bg_list_ctrl_item"] = surface2;

    m_generatedColors["bg_tree_view"] = surface2;
    m_generatedColors["bg_tree_view_node"] = surface2;

    // 等级3：主窗口/对话框客户区背景、Tab页面内容区域
    m_generatedColors["bg_window_main"] = windowBg;
    m_generatedColors["bg_window_card"] = surface2; //比标准窗口的颜色浅

    m_generatedColors["bg_container"] = surface3;
    m_generatedColors["bg_content"] = surface3;
    m_generatedColors["bg_overlay"] = surface3;
    m_generatedColors["bg_address_bar"] = surface3;

    // 等级4：通用容器面板 Panel/Widget, GroupBox内部区域
    m_generatedColors["bg_header"] = surface4;


    // 等级5：按钮、Switch、Progress、Slider、ComboBox整体、StatusBar
    m_generatedColors["bg_btn_window"] = surface5;
    m_generatedColors["bg_btn"] = surface5;
    m_generatedColors["border_btn"] = surface7; //按钮边框色
    m_generatedColors["bg_combo"] = surface5;
    m_generatedColors["bg_progress_track"] = surface5;

    // 等级6：MenuBar、ToolBar、Tab标签条背景
    m_generatedColors["bg_menu_bar"] = surface6;
    m_generatedColors["bg_tab_ctrl"] = surface6;
    m_generatedColors["bg_property_grid"] = surface6;

    // 等级7（最深）：窗口标题栏 / 窗口边框
    m_generatedColors["bg_titlebar"] = surface7;
    m_generatedColors["border_window"] = surface7;

    // 边框色：通常使用比背景色深一级的颜色
    m_generatedColors["border_control"] = surface4;
    m_generatedColors["border_group_box"] = surface4;

    // 分割线：颜色比边框线更深一些
    m_generatedColors["bg_split"] = surface6;
    m_generatedColors["border_split_level1"] = surface6;
    m_generatedColors["border_split_level2"] = surface7;

    // 窗口关闭按钮 - 使用错误色
    m_generatedColors["bg_btn_window_close"] = m_generatedColors["--error"];

    //确保颜色值正确(生成的颜色个数不应增加，因为新增加的是无法识别的)
    ASSERT_UNUSED_VARIABLE(nGeneratedColorsCount == m_generatedColors.size());
}

std::string ThemeGenerator::GetGeneratedColor(const std::string& colorName) const
{
    auto iter = m_generatedColors.find(colorName);
    if (iter != m_generatedColors.end()) {
        return iter->second;
    }
    return std::string();
}

bool ThemeGenerator::LoadConfigFromXml(const std::string& inputXml)
{
    ASSERT(!inputXml.empty());
    if (inputXml.empty()) {
        return false;
    }
    pugi::xml_document doc;
    pugi::xml_parse_result result;

    if (inputXml.front() == _T('<')) {
        //按文件数据加载
        m_originalXmlContent = inputXml; // 保存原始XML内容，用于后续精确替换value值
        result = doc.load_buffer(inputXml.c_str(), inputXml.size());
    }
    else {
        //按文件路径加载
        // 保存原始XML内容，用于后续精确替换value值
        std::ifstream file(inputXml, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        m_originalXmlContent = buffer.str();
        file.close();

        result = doc.load_file(inputXml.c_str());
    }

    ASSERT(result);
    if (!result) {
        return false;
    }

    m_loadedConfigs.clear();
    m_themeMeta.properties.clear();

    pugi::xml_node root = doc.root().first_child();
    if (DString(root.name()) != _T("Global")) {
        return false;
    }

    int orderIndex = 0;
    for (pugi::xml_node child : root.children()) {
        DString nodeName = child.name();

        if (nodeName == _T("Theme")) {
            m_themeMeta.properties["theme_name"] = StringConvert::TToUTF8(child.attribute(_T("name")).as_string());
            m_themeMeta.properties["theme_type"] = StringConvert::TToUTF8(child.attribute(_T("type")).as_string());
            m_themeMeta.properties["theme_style"] = StringConvert::TToUTF8(child.attribute(_T("style")).as_string());
        }
        else if (nodeName == _T("ThemeMeta")) {
            for (pugi::xml_node prop : child.children()) {
                if (DString(prop.name()) == _T("Property")) {
                    std::string propName = StringConvert::TToUTF8(prop.attribute(_T("name")).as_string());
                    std::string propValue = StringConvert::TToUTF8(prop.attribute(_T("value")).as_string());
                    m_themeMeta.properties[propName] = propValue;
                }
            }
        }
        else if (nodeName == _T("ThemeColor")) {
            ThemeColorConfig config;
            config.name = StringConvert::TToUTF8(child.attribute(_T("name")).as_string());
            config.value = StringConvert::TToUTF8(child.attribute(_T("value")).as_string());
            config.type = StringConvert::TToUTF8(child.attribute(_T("type")).as_string());
            config.category = StringConvert::TToUTF8(child.attribute(_T("category")).as_string());
            config.role = StringConvert::TToUTF8(child.attribute(_T("role")).as_string());
            config.derived_from = StringConvert::TToUTF8(child.attribute(_T("derived_from")).as_string());
            config.adjust = StringConvert::TToUTF8(child.attribute(_T("adjust")).as_string());
            config.fixed = (DString(child.attribute(_T("fixed")).as_string(_T("false"))) == _T("true"));
            config.support_accent = (DString(child.attribute(_T("support_accent")).as_string(_T("false"))) == _T("true"));
            config.contrast_bg = StringConvert::TToUTF8(child.attribute(_T("contrast_bg")).as_string());
            config.comment_cn = StringConvert::TToUTF8(child.attribute(_T("comment_cn")).as_string());
            config.comment_en = StringConvert::TToUTF8(child.attribute(_T("comment_en")).as_string());
            config.node_order = orderIndex++;

            std::pair<std::string, std::string> statePair = DetectColorState(config.name);
            if (!statePair.first.empty()) {
                config.m_state = statePair.first;
                config.m_baseName = statePair.second;
            }

            if (!config.name.empty()) {
                m_loadedConfigs[config.name] = config;
            }
        }
    }
    return true;
}

std::string ThemeGenerator::GenerateTheme(double hue, double base, bool isDark)
{
    if (m_loadedConfigs.empty()) {
        return std::string();
    }
    GenerateThemeColors(hue, base, isDark);
    return GenerateThemeXml(isDark);
}

std::string ThemeGenerator::GenerateThemeXml(bool isDark) const
{
    std::map<std::string, std::string> processedColors;

    // 优先使用生成的核心颜色
    for (const auto& pair : m_loadedConfigs) {
        const std::string& colorName = pair.first;
        auto iter = m_generatedColors.find(colorName);
        if (iter != m_generatedColors.end()) {
            processedColors[colorName] = iter->second;
        }
    }

    // 第1步：处理所有颜色配置的变体颜色
    for (const auto& pair : m_loadedConfigs) {
        const std::string& colorName = pair.first;

        //已经生成的，不重复计算
        auto iter = processedColors.find(colorName);
        if (iter != processedColors.end()) {
            continue;
        }

        //处理状态变体颜色
        auto [state, baseName] = DetectColorState(colorName);
        if (!state.empty()) {
            std::string baseColor;//基准色（用于计算状态变体颜色）
            if (processedColors.find(baseName) != processedColors.end()) {
                baseColor = processedColors[baseName];
            }
            else if (m_generatedColors.find(baseName) != m_generatedColors.end()) {
                baseColor = m_generatedColors.find(baseName)->second;
            }
            if (!baseColor.empty()) {
                processedColors[colorName] = GetStateColor(baseColor, state, isDark);
            }
        }
    }

    //第2步：处理派生颜色
    for (const auto& pair : m_loadedConfigs) {
        const std::string& colorName = pair.first;
        const ThemeColorConfig& attrs = pair.second;

        std::string derivedFrom = attrs.derived_from;
        if (derivedFrom.empty()) {
            //非派生颜色
            continue;
        }

        //处理派生颜色
        std::string baseColor;//基准色（用于计算派生颜色）
        if (processedColors.find(derivedFrom) != processedColors.end()) {
            baseColor = processedColors[derivedFrom];
        }
        else if (m_generatedColors.find(derivedFrom) != m_generatedColors.end()) {
            baseColor = m_generatedColors.find(derivedFrom)->second;
        }
        if (baseColor.empty()) {
            auto [derivedFromState, derivedFromBaseName] = DetectColorState(derivedFrom);
            if (!derivedFromState.empty()) {
                if (processedColors.find(derivedFromBaseName) != processedColors.end()) {
                    baseColor = processedColors[derivedFromBaseName];
                }
                else if (m_generatedColors.find(derivedFromBaseName) != m_generatedColors.end()) {
                    baseColor = m_generatedColors.find(derivedFromBaseName)->second;
                }
            }
        }

        if (!baseColor.empty()) {
            std::string adjust = attrs.adjust;
            if (!adjust.empty()) {
                processedColors[colorName] = ApplyAdjustments(baseColor, adjust);
            }
            else {
                processedColors[colorName] = baseColor;
            }
        }
    }

    // 第3步：保留原颜色的透明度
    for (const auto& pair : m_loadedConfigs) {
        const std::string& colorName = pair.first;
        auto iter = processedColors.find(colorName);
        if (iter != processedColors.end()) {
            std::string oldColor = pair.second.value;
            std::string newColor = iter->second;
            uint8_t alpha, r, g, b;
            if (ColorConverter::ParseHexColor(newColor, alpha, r, g, b)) {
                if (alpha == 255) {
                    uint8_t alpha0, r0, g0, b0;
                    if (ColorConverter::ParseHexColor(oldColor, alpha0, r0, g0, b0)) {
                        if (alpha0 != alpha) {
                            iter->second = ColorConverter::RGBToHex(alpha0, r, g, b);
                        }
                    }
                }
            }
        }
    }

    // 第4步：对比度检查和修正
    // 对所有processedColors中的颜色进行检查（只要在loadedConfigs中配置了contrast_bg）
    for (const auto& pair : processedColors) {
        const std::string& colorName = pair.first;

        auto itLoaded = m_loadedConfigs.find(colorName);
        if (itLoaded == m_loadedConfigs.end()) {
            continue;
        }

        const ThemeColorConfig& attrs = itLoaded->second;
        if (attrs.contrast_bg.empty()) {
            continue;
        }

        std::list<std::string> contrastBgList = StringUtil::Split(attrs.contrast_bg, ",");
        for (std::string& contrastBg : contrastBgList) {
            StringUtil::Trim(contrastBg);
            if (!contrastBg.empty()) {
                auto itBg = processedColors.find(contrastBg);
                auto itColor = processedColors.find(colorName);
                if (itBg != processedColors.end() && itColor != processedColors.end()) {
                    std::string textColor = itColor->second;
                    std::string bgColor = itBg->second;
                    std::string corrected = EnsureContrast(textColor, bgColor, 4.5);
                    if (!corrected.empty()) {
                        processedColors[colorName] = corrected;
                    }
                }
            }
        }
    }

    //颜色修正
    processedColors["bg_split_disabled"] = GetGeneratedColor("--surface_4");

    // 第5步：补充缺失的颜色
    for (const auto& pair : m_loadedConfigs) {
        const std::string& colorName = pair.first;
        if (processedColors.find(colorName) == processedColors.end()) {
            processedColors[colorName] = pair.second.value;
        }
    }

    // 第6步：精确替换原始XML中的value值（保持原始格式和顺序）
    std::string result = m_originalXmlContent;

    for (const auto& pair : processedColors) {
        const std::string& colorName = pair.first;
        const std::string& newValue = pair.second;

        if (colorName.empty() || newValue.empty()) {
            continue;
        }

        size_t namePos = result.find("name=\"" + colorName + "\"");
        if (namePos == std::string::npos) {
            continue;
        }

        size_t valuePos = result.find("value=\"", namePos);
        if (valuePos == std::string::npos) {
            continue;
        }

        size_t valueStart = valuePos + 7;
        size_t valueEnd = result.find("\"", valueStart);
        if (valueEnd == std::string::npos) {
            continue;
        }

        result = result.substr(0, valueStart) + newValue + result.substr(valueEnd);
    }

    return result;
}

}
