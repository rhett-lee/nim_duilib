#ifndef DUILIB_CORE_COLORCONVERTER_H_
#define DUILIB_CORE_COLORCONVERTER_H_

#include "duilib/Core/UiTypes.h"

/**
 * @file ColorConverter.h
 * @brief 颜色空间转换工具类
 *
 * ============================================================================
 * 参数取值范围说明
 * ============================================================================
 *
 * 【OKLCH色彩空间】
 * - L (Lightness/明度): 0.0 - 1.0
 *   * 0.0 表示黑色，1.0 表示白色
 *   * 0.5 是中性灰
 *
 * - C (Chroma/色度): 0.0 - ~0.4+
 *   * 0.0 表示无彩色（灰度）
 *   * 值越大颜色越鲜艳
 *   * 最大值取决于L和H，实际可达0.3-0.4以上
 *
 * - H (Hue/色相): 0 - 360
 *   * 0/360 = 红色 (Red)
 *   * 60 = 黄色 (Yellow)
 *   * 120 = 绿色 (Green)
 *   * 180 = 青色 (Cyan)
 *   * 240 = 蓝色 (Blue)
 *   * 300 = 品红色 (Magenta)
 *
 * 【RGB色彩空间】
 * - 各分量: 0 - 255 (字节) 或 0.0 - 1.0 (双精度)
 *
 * 【HSL色彩空间】
 * - H (Hue/色相): 0 - 360
 *   * 0/360 = 红色 (Red)
 *   * 60 = 黄色 (Yellow)
 *   * 120 = 绿色 (Green)
 *   * 180 = 青色 (Cyan)
 *   * 240 = 蓝色 (Blue)
 *   * 300 = 品红色 (Magenta)
 *
 * - S (Saturation/饱和度): 0.0 - 1.0
 *   * 0.0 = 灰度（无彩色）
 *   * 1.0 = 最高饱和度
 *
 * - L (Lightness/明度): 0.0 - 1.0
 *   * 0.0 = 黑色
 *   * 1.0 = 白色
 *   * 0.5 = 颜色最鲜艳
 *
 * 【Hex格式】
 * - 格式: "#AARRGGBB" 或 "#RRGGBB"
 *   * AA = Alpha (00-FF, 透明-不透明)
 *   * RR = Red (00-FF)
 *   * GG = Green (00-FF)
 *   * BB = Blue (00-FF)
 *
 * 【相对亮度】
 * - 值域: 0.0 - 1.0
 *   * 0.0 表示最暗（纯黑）
 *   * 1.0 表示最亮（纯白）
 *
 * 【对比度】
 * - 值域: 1.0 - 21.0
 *   * 1.0 = 无对比度（相同颜色）
 *   * 4.5 = WCAG AA标准（普通文本）
 *   * 7.0 = WCAG AAA标准（普通文本）
 *   * 3.0 = WCAG AA标准（大文本）
 *   * 21.0 = 最大对比度（黑与白）
 *
 * ============================================================================
 * 转换公式
 * ============================================================================
 *
 * 【sRGB到Linear RGB】
 * - 如果 x <= 0.04045: linear = x / 12.92
 * - 否则: linear = ((x + 0.055) / 1.055) ^ 2.4
 *
 * 【Linear RGB到sRGB】
 * - 如果 x <= 0.0031308: srgb = 12.92 * x
 * - 否则: srgb = 1.055 * (x ^ (1/2.4)) - 0.055
 *
 * 【RGB到相对亮度】
 * - Y = 0.2126 * R_linear + 0.7152 * G_linear + 0.0722 * B_linear
 *
 * 【对比度计算】
 * - contrast = (L1 + 0.05) / (L2 + 0.05)
 * - 其中L1是较亮颜色的相对亮度，L2是较暗颜色的相对亮度
 */

namespace ui {

/** @class ColorConverter
 *  @brief 颜色空间转换工具类
 *  @details 提供OKLCH、RGB、Hex等颜色格式之间的相互转换功能（所有函数为静态函数）
 */
class DUILIB_API ColorConverter
{
public:
    // ==================== Hex <-> RGB ====================

    /** @brief 解析Hex颜色字符串
     *  @param colorStr Hex颜色字符串，格式如"#FFAABBCC"
     *  @param alpha 输出alpha值(0-255)
     *  @param r 输出红色值(0-255)
     *  @param g 输出绿色值(0-255)
     *  @param b 输出蓝色值(0-255)
     *  @return 解析成功返回true，否则返回false
     */
    static bool ParseHexColor(const std::string& colorStr, uint8_t& alpha, uint8_t& r, uint8_t& g, uint8_t& b);

    /** @brief 将RGB转换为Hex字符串
     *  @param alpha Alpha值(0-255)
     *  @param r 红色值(0-255)
     *  @param g 绿色值(0-255)
     *  @param b 蓝色值(0-255)
     *  @return Hex颜色字符串，如"#FFAABBCC"
     */
    static std::string RGBToHex(uint8_t alpha, uint8_t r, uint8_t g, uint8_t b);

    /** @brief 将RGB转换为HSL色彩空间
     *  @param r 红色值(0-255)
     *  @param g 绿色值(0-255)
     *  @param b 蓝色值(0-255)
     *  @param h 输出色相(0-360)
     *  @param s 输出饱和度(0.0-1.0)
     *  @param l 输出明度(0.0-1.0)
     *  @note HSL是更直观的颜色表示，便于按比例调整
     */
    static void RGBToHSL(uint8_t r, uint8_t g, uint8_t b, double& h, double& s, double& l);

    /** @brief 将HSL色彩空间转换为RGB
     *  @param h 色相(0-360)
     *  @param s 饱和度(0.0-1.0)
     *  @param l 明度(0.0-1.0)
     *  @param r 输出红色值(0-255)
     *  @param g 输出绿色值(0-255)
     *  @param b 输出蓝色值(0-255)
     */
    static void HSLToRGB(double h, double s, double l, uint8_t& r, uint8_t& g, uint8_t& b);

    // ==================== 相对亮度 ====================

    /** @brief 计算RGB颜色的相对亮度
     *  @param r 红色值(0-255)
     *  @param g 绿色值(0-255)
     *  @param b 蓝色值(0-255)
     *  @return 相对亮度值(0-1)
     */
    static double GetRelativeLuminance(uint8_t r, uint8_t g, uint8_t b);

    /** @brief 计算两个颜色的对比度
     *  @param color1 颜色1的Hex字符串
     *  @param color2 颜色2的Hex字符串
     *  @return 对比度值，失败返回0.0
     */
    static double CalculateContrastRatio(const std::string& color1, const std::string& color2);

    // ==================== RGB <-> OKLCH ====================

    /** @brief 将RGB转换为OKLCH
     *  @param r 红色值(0-255)
     *  @param g 绿色值(0-255)
     *  @param b 蓝色值(0-255)
     *  @param L 输出明度值(0-1)
     *  @param C 输出色度值(0~)
     *  @param H 输出色相值(0-360)
     *  @return 转换成功返回true，否则返回false
     */
    static bool RGBToOKLCH(uint8_t r, uint8_t g, uint8_t b, double& L, double& C, double& H);

    /** @brief 将RGB(双精度)转换为OKLCH
     *  @param red 红色值(0.0-1.0)
     *  @param green 绿色值(0.0-1.0)
     *  @param blue 蓝色值(0.0-1.0)
     *  @param L 输出明度值(0-1)
     *  @param C 输出色度值(0~)
     *  @param H 输出色相值(0-360)
     *  @return 成功返回0，失败返回-1
     */
    static int RGBToOKLCH(double red, double green, double blue, double* L, double* C, double* H);

    /** @brief 将OKLCH转换为ARGB十六进制字符串
     *  @param L 明度值(0-1，超出范围会被自动钳制)
     *  @param C 色度值(负值会被自动钳制为0)
     *  @param H 色相值(0-360，超出范围会自动归一化)
     *  @param alpha Alpha值(0-255)
     *  @return ARGB十六进制字符串，如"#FFAABBCC"
     */
    static std::string OKLCHToARGB(double L, double C, double H, uint8_t alpha);

    /** @brief 将OKLCH转换为RGB
     *  @param L 明度值(0-1，超出范围会被自动钳制)
     *  @param C 色度值(负值会被自动钳制为0)
     *  @param H 色相值(0-360，超出范围会自动归一化)
     *  @param red 输出红色值(0.0-1.0)
     *  @param green 输出绿色值(0.0-1.0)
     *  @param blue 输出蓝色值(0.0-1.0)
     *  @return 成功返回0，失败返回-1
     */
    static int OKLCHToRGB(double L, double C, double H, double* red, double* green, double* blue);

    /** @brief 将OKLCH转换为RGB(字节)
     *  @param L 明度值(0-1)
     *  @param C 色度值
     *  @param H 色相值(0-360)
     *  @param red 输出红色值(0-255)
     *  @param green 输出绿色值(0-255)
     *  @param blue 输出蓝色值(0-255)
     *  @return 成功返回0，失败返回-1
     */
    static int OKLCHToRGB(double L, double C, double H, uint8_t& red, uint8_t& green, uint8_t& blue);

private:
    /** @brief sRGB传输函数(Gamma校正) */
    static double sRGBTransferFunction(double x);

    /** @brief 逆向sRGB传输函数 */
    static double InverseSRGBTransferFunction(double x);

    /** @brief 立方根函数 */
    static double Cbrt(double x);

    static constexpr double PI = 3.14159265358979323846;  ///< 圆周率常量
};

}  // namespace ui

#endif  // DUILIB_CORE_COLORCONVERTER_H_
