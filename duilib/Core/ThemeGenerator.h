#ifndef UI_CORE_THEME_GENERATOR_H_
#define UI_CORE_THEME_GENERATOR_H_

#include "duilib/Core/ColorConverter.h"
#include <string>
#include <map>
#include <vector>

namespace ui
{

/** @struct ThemeColorConfig
 *  @brief 主题颜色配置结构体
 *  @details 存储单个颜色项的所有配置信息，包括名称、值、类型、分类等属性
 */
struct ThemeColorConfig
{
    std::string name;           ///< 颜色名称，如"bg_window_main"
    std::string value;          ///< 颜色值，格式为"#AARRGGBB"
    std::string type;           ///< 颜色类型，如"bg_color"、"border_color"等
    std::string category;       ///< 颜色分类，如"bg_color"、"text_color"等
    std::string role;           ///< 颜色角色描述
    std::string derived_from;   ///< 派生来源，指定基于哪个颜色派生
    std::string adjust;         ///< 调整参数，如"lightness:-10,saturation:20"
    bool fixed;                 ///< 是否为固定颜色，固定颜色不会被主题生成器修改
    bool support_accent;        ///< 是否支持强调色
    std::string contrast_bg;    ///< 对比度检查的背景色名称
    std::string comment_cn;     ///< 中文注释
    std::string comment_en;     ///< 英文注释
    int node_order;             ///< 节点顺序索引（用于保持输出顺序与输入一致）

    std::string m_baseName;     ///< 颜色名称的基础名称，如"bg_btn_disabled"的基础名称为"bg_btn"
    std::string m_state;        ///< 颜色名称的状态名称，如"bg_btn_disabled"的状态名称为"disabled"
};

/** @struct ThemeMetaInfo
 *  @brief 主题元信息结构体
 *  @details 存储主题的基本元数据信息
 */
struct ThemeMetaInfo
{
    std::map<std::string, std::string> properties;  ///< 元数据属性键值对
};

/** @class ThemeGenerator
 *  @brief 主题颜色生成器
 *  @details 基于OKLCH色彩空间的主题颜色生成器，支持：
 *          - 从输入XML动态读取颜色配置
 *          - 生成符合WCAG 2.1对比度标准的主题颜色
 *          - 支持Light/Dark两种模式
 *          - 自动处理颜色状态变体（hovered、pressed、selected、disabled）
 *          - 保持输出XML与输入XML的顺序和注释一致
 *          - 可通过SetXXXParams系列函数自定义颜色算法参数
 */
class DUILIB_API ThemeGenerator
{
public:
    /** @brief 构造函数 */
    ThemeGenerator();

    /** @brief 析构函数 */
    ~ThemeGenerator();

    /** @brief 从XML文件加载颜色配置
     *  @param inputXml 输入XML文件路径
     *  @return 加载成功返回true，否则返回false
     *  @details 解析XML文件，提取所有ThemeColor节点配置，并保存原始XML内容
     */
    bool LoadConfigFromXml(const std::string& inputXml);

    /** @brief 生成主题颜色
     *  @param hue 色调值(0-360)
     *  @param base 基础亮度值(0-1)
     *  @param isDark 是否为暗色主题
     *  @return 生成的主题XML字符串
     *  @details 根据指定的色相和亮度生成完整的主题颜色配置，
     *          只修改颜色值，保留原始XML的格式、顺序和注释
     */
    std::string GenerateTheme(double hue, double base, bool isDark);

public:
    // ==================== 颜色算法参数设置函数 ====================
    // 注意: 所有L值(明度)范围为0.0-1.0, C值(色度)范围为0.0-0.4+
    //       base参数范围为0.0-1.0，控制颜色的鲜艳程度

    /** @brief 设置背景色参数（带彩色）
     *  @param bgLightL 浅色模式背景明度基准值 (范围: 0.0-1.0, 典型值: 0.97)
     *  @param bgDarkL 深色模式背景明度基准值 (范围: 0.0-1.0, 典型值: 0.17)
     *  @param bgBaseChroma 背景色基础色度系数 (范围: 0.0-2.0, 典型值: 1.5)
     *  @note baseChroma越大，背景色越鲜艳，base=0.02时建议用1.0，base=1.0时建议用3.0
     */
    void SetBgParams(double bgLightL, double bgDarkL, double bgBaseChroma);

    /** @brief 设置前景色参数
     *  @param fgLightL 浅色模式前景明度基准值 (范围: 0.0-1.0, 典型值: 0.22)
     *  @param fgDarkL 深色模式前景明度基准值 (范围: 0.0-1.0, 典型值: 0.92)
     *  @param fgBaseChroma 前景色基础色度系数 (范围: 0.0-1.0, 典型值: 0.15)
     *  @note 前景色色度通常比背景色低，以保证文本可读性
     */
    void SetFgParams(double fgLightL, double fgDarkL, double fgBaseChroma);

    /** @brief 设置Surface层参数
     *  @param surfaceLightOffset 浅色模式下Surface偏移量 (范围: 0.0-0.2, 典型值: 0.025, 用于使表面变暗)
     *  @param surfaceDarkOffset 深色模式下Surface偏移量 (范围: 0.0-0.2, 典型值: 0.07, 用于使表面变亮)
     *  @param surfaceBaseChroma Surface基础色度系数 (范围: 0.0-2.0, 典型值: 1.0)
     *  @note Surface用于卡片、按钮等元素的背景。浅色模式下offset使表面变暗(L减少)，
     *       深色模式下offset使表面变亮(L增加)，从而形成视觉层次感
     */
    void SetSurfaceParams(double surfaceLightOffset, double surfaceDarkOffset, double surfaceBaseChroma);

    /** @brief 设置Accent颜色参数
     *  @param accentLightL 浅色模式下Accent明度 (范围: 0.0-1.0, 典型值: 0.6204)
     *  @param accentDarkL 深色模式下Accent明度 (范围: 0.0-1.0, 典型值: 0.68)
     *  @param accentC 色度值 (范围: 0.0-0.4, 典型值: 0.195)
     *  @note Accent是主题的强调色，用于按钮、链接等关键元素
     */
    void SetAccentParams(double accentLightL, double accentDarkL, double accentC);

    // ==================== 颜色算法参数获取函数 ====================

    /** @brief 获取背景色参数
     *  @param bgLightL 输出浅色模式背景明度基准值
     *  @param bgDarkL 输出深色模式背景明度基准值
     *  @param bgBaseChroma 输出背景色基础色度系数
     */
    void GetBgParams(double& bgLightL, double& bgDarkL, double& bgBaseChroma) const;

    /** @brief 获取前景色参数
     *  @param fgLightL 输出浅色模式前景明度基准值
     *  @param fgDarkL 输出深色模式前景明度基准值
     *  @param fgBaseChroma 输出前景色基础色度系数
     */
    void GetFgParams(double& fgLightL, double& fgDarkL, double& fgBaseChroma) const;

    /** @brief 获取Surface层参数
     *  @param surfaceLightOffset 输出浅色模式Surface偏移量
     *  @param surfaceDarkOffset 输出深色模式Surface偏移量
     *  @param surfaceBaseChroma 输出Surface基础色度系数
     */
    void GetSurfaceParams(double& surfaceLightOffset, double& surfaceDarkOffset, double& surfaceBaseChroma) const;

    /** @brief 获取Accent颜色参数
     *  @param accentLightL 输出浅色模式Accent明度
     *  @param accentDarkL 输出深色模式Accent明度
     *  @param accentC 输出Accent色度值
     */
    void GetAccentParams(double& accentLightL, double& accentDarkL, double& accentC) const;

    // ==================== 颜色带彩色/不带彩色选项 ====================

    /** @brief 设置背景色是否带彩色
     *  @param colored true=带彩色（应用色相），false=无彩色（灰度）
     *  @note 默认值为true。设为false后，背景色将退化为灰度色
     */
    void SetBgColored(bool colored);

    /** @brief 获取背景色是否带彩色 */
    bool IsBgColored() const;

    /** @brief 设置前景色是否带彩色
     *  @param colored true=带彩色（应用色相），false=无彩色（灰度）
     *  @note 默认值为true。前景通常建议保持带彩色以增强可读性
     */
    void SetFgColored(bool colored);

    /** @brief 获取前景色是否带彩色 */
    bool IsFgColored() const;

    /** @brief 设置Surface层是否带彩色
     *  @param colored true=带彩色（应用色相），false=无彩色（灰度）
     *  @note 默认值为true
     */
    void SetSurfaceColored(bool colored);

    /** @brief 获取Surface层是否带彩色 */
    bool IsSurfaceColored() const;

    /** @brief 重置所有参数为默认值 */
    void ResetParams();

public:
    /** @brief 获取生成的指定颜色
     *  @param colorName 颜色名称
     *  @return 颜色的ARGB字符串，如"#FF123456"
     */
    std::string GetGeneratedColor(const std::string& colorName) const;

    /** @brief 获取已加载的颜色配置
     *  @return 颜色配置映射表
     */
    std::map<std::string, ThemeColorConfig> GetLoadedConfigs() const { return m_loadedConfigs; }

private:

    /** @brief 根据色相获取基础背景颜色
     *  @param hue 色调值(0-360)
     *  @param base 基础亮度值(0-1)
     *  @param isDark 是否为暗色主题
     *  @return 基础背景颜色的ARGB字符串
     */
    std::string GetBackgroundColor(double hue, double base, bool isDark);

    /** @brief 获取前景色（与背景色互补）
     *  @param hue 色调值(0-360)
     *  @param base 基础亮度值(0-1)
     *  @param isDark 是否为暗色主题
     *  @return 前景色的ARGB字符串
     */
    std::string GetForegroundColor(double hue, double base, bool isDark);

    /** @brief 获取表面层颜色
     *  @param hue 色调值(0-360)
     *  @param base 基础亮度值(0-1)
     *  @param isDark 是否为暗色主题
     *  @param surfaceLevel 表面层级(0-3)
     *  @return 表面层颜色的ARGB字符串
     *  @details 根据层级递推颜色明暗，浅色主题逐层变暗，深色主题逐层变亮
     */
    std::string GetSurfaceColor(double hue, double base, bool isDark, int surfaceLevel);

    /** @brief 获取状态变体颜色
     *  @param baseColor 基础颜色
     *  @param state 状态类型："hovered"、"pressed"、"selected"、"disabled"
     *  @param isDark 是否为暗色主题
     *  @return 状态变体颜色的ARGB字符串
     *  @details 根据状态调整亮度和饱和度：
     *          - hovered: 浅色变暗(+0.06L)，深色变亮(-0.06L)
     *          - pressed: 浅色变暗(+0.10L)，深色变亮(-0.10L)
     *          - selected: 浅色变暗(+0.09L)，深色变亮(-0.09L)
     *          - disabled: 降低饱和度并设置半透明
     */
    std::string GetStateColor(const std::string& baseColor, const std::string& state, bool isDark) const;

    /** @brief 应用颜色调整参数
     *  @param baseColor 基础颜色
     *  @param adjustStr 调整参数字符串，格式："lightness:10,saturation:-5,hue:30"
     *  @return 调整后的颜色
     *  @details 支持的调整类型：
     *          - lightness: 亮度调整（百分比）
     *          - saturation: 饱和度调整（百分比）
     *          - hue: 色相调整（度数）
     *          - alpha: 透明度调整（绝对值或相对值）
     */
    std::string ApplyAdjustments(const std::string& baseColor, const std::string& adjustStr) const;

    /** @brief 检测颜色名称中的状态标识
     *  @param colorName 颜色名称
     *  @return pair<状态, 基础名称>
     *  @details 从颜色名称中提取状态后缀，如"bg_btn_hovered"返回("hovered", "bg_btn")
     */
    std::pair<std::string, std::string> DetectColorState(const std::string& colorName) const;

    /** @brief 生成主题核心颜色集
     *  @param hue 色调值(0-360)
     *  @param base 基础亮度值(0-1)
     *  @param isDark 是否为暗色主题
     *  @details 生成所有核心主题颜色，包括：
     *          - 背景色、前景色、表面层颜色
     *          - 强调色及其前景色
     *          - 链接颜色及其状态变体
     *          - 中性色渐变
     */
    void GenerateThemeColors(double hue, double base, bool isDark);

    /** @brief 确保文本颜色与背景色的对比度达标
     *  @param textColor 文本颜色
     *  @param bgColor 背景颜色
     *  @param minContrast 最低对比度要求，默认4.5（WCAG AA标准）
     *  @return 调整后的文本颜色
     *  @details 如果对比度不足，通过迭代调整亮度分量直到达到目标对比度
     */
    std::string EnsureContrast(const std::string& textColor, const std::string& bgColor, double minContrast = 4.5) const;

    /** @brief 生成主题颜色
     *  @param isDark 是否为暗色主题
     *  @return 生成的主题XML字符串
     *  @details 根据指定的色相和亮度生成完整的主题颜色配置，
     *          只修改颜色值，保留原始XML的格式、顺序和注释
     */
    std::string GenerateThemeXml(bool isDark) const;

private:
    double m_hue;                                            ///< 当前色调值 (0-360)
    double m_base;                                           ///< 当前基础亮度值 (0.0-1.0，控制颜色鲜艳程度)
    bool m_isDark;                                           ///< 当前是否为暗色主题
    std::map<std::string, std::string> m_generatedColors;    ///< 生成的核心颜色集
    std::map<std::string, ThemeColorConfig> m_loadedConfigs; ///< 从XML加载的颜色配置
    std::string m_originalXmlContent;                        ///< 原始XML完整内容
    ThemeMetaInfo m_themeMeta;                               ///< 主题元信息

    // ============================================================================
    // 颜色算法参数 (所有L值范围0.0-1.0, 所有Offset/C系数范围根据具体参数确定)
    // ============================================================================

    // 背景色参数
    double m_bgLightL;          ///< 浅色模式背景明度基准值 (范围: 0.0-1.0, 默认: 0.97)
    double m_bgLightLScale;     ///< 浅色模式背景明度随base变化系数 (范围: -2.0到0, 默认: -0.95)
    double m_bgDarkL;           ///< 深色模式背景明度基准值 (范围: 0.0-1.0, 默认: 0.17)
    double m_bgDarkLScale;      ///< 深色模式背景明度随base变化系数 (范围: 0到2.0, 默认: 0.78)
    double m_bgBaseChroma;      ///< 背景色度系数 (范围: 0.0-3.0, 默认: 1.0)

    // 前景色参数
    double m_fgLightL;          ///< 浅色模式前景明度基准值 (范围: 0.0-1.0, 默认: 0.22)
    double m_fgLightLScale;     ///< 浅色模式前景明度随base变化系数 (范围: 0到2.0, 默认: 0.72)
    double m_fgDarkL;           ///< 深色模式前景明度基准值 (范围: 0.0-1.0, 默认: 0.92)
    double m_fgDarkLScale;      ///< 深色模式前景明度随base变化系数 (范围: -2.0到0, 默认: -0.75)
    double m_fgBaseChroma;      ///< 前景色度系数 (范围: 0.0-1.0, 默认: 0.15)

    // Surface层参数
    double m_surfaceLightOffset; ///< 浅色模式Surface偏移量 (范围: 0.0-0.2, 默认: 0.025, 使表面变暗)
    double m_surfaceDarkOffset;  ///< 深色模式Surface偏移量 (范围: 0.0-0.2, 默认: 0.07, 使表面变亮)
    double m_surfaceBaseChroma;  ///< Surface色度系数 (范围: 0.0-3.0, 默认: 1.5)

    // Accent参数
    double m_accentLightL;      ///< 浅色模式Accent明度 (范围: 0.0-1.0, 默认: 0.6204)
    double m_accentDarkL;       ///< 深色模式Accent明度 (范围: 0.0-1.0, 默认: 0.68)
    double m_accentC;           ///< Accent色度 (范围: 0.0-0.4, 默认: 0.195)

    // 带彩色/无彩色选项 (默认全部为true，即带彩色)
    bool m_bgColored;           ///< 背景色是否带彩色 (true=带彩色, false=无彩色)
    bool m_fgColored;           ///< 前景色是否带彩色 (true=带彩色, false=无彩色)
    bool m_surfaceColored;      ///< Surface层是否带彩色 (true=带彩色, false=无彩色)
};

}

#endif //UI_CORE_THEME_GENERATOR_H_
