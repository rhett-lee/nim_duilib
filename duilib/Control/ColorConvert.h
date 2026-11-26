/////////////////////////////////////////////////////////////////////////////////////
// 颜色选择器辅助类，来源：                                                                //
// codeproject.com/Articles/2577/Xguiplus-A-set-of-Photoshop-s-like-color-pickers  //
//                                                                                   //
/////////////////////////////////////////////////////////////////////////////////////
#ifndef UI_CONTROL_COLOR_CONVERT_H_
#define UI_CONTROL_COLOR_CONVERT_H_

#include <math.h>
#include <stdint.h>
#include "duilib/Core/UiColor.h"

namespace ui
{

/** 颜色类型（RGB/HSV/HSL）转换类
*   提供 HSV/HSL 与 RGB 之间的相互转换，以及为颜色选择器生成渐变色采样数据。
*/
class ColorConvert
{
public:
    /** HSV 转换为 RGB（输出归一化的分量）
    * @param [in] hue 色相（0~360）
    * @param [in] sat 饱和度（0~1）
    * @param [in] value 明度（0~1）
    * @param [out] red 返回红色分量（0~1）
    * @param [out] green 返回绿色分量（0~1）
    * @param [out] blue 返回蓝色分量（0~1）
    * @return 返回 0 表示成功，-1 表示参数非法
    */
    static int HSV2RGB(double hue, double sat, double value, double* red, double* green, double* blue);

    /** HSV 转换为 RGB（输出 0~255 的分量）
    * @param [in] hue 色相（0~360）
    * @param [in] sat 饱和度（0~1）
    * @param [in] value 明度（0~1）
    * @param [out] red 返回红色分量（0~255）
    * @param [out] green 返回绿色分量（0~255）
    * @param [out] blue 返回蓝色分量（0~255）
    * @return 返回 0 表示成功，-1 表示参数非法
    */
    static int HSV2RGB(double hue, double sat, double value, uint8_t& red, uint8_t& green, uint8_t& blue);

    /** RGB 转换为 HSV
    * @param [in] red 红色分量（0~1）
    * @param [in] green 绿色分量（0~1）
    * @param [in] blue 蓝色分量（0~1）
    * @param [out] hue 返回色相（0~360）
    * @param [out] sat 返回饱和度（0~1）
    * @param [out] value 返回明度（0~1）
    * @return 返回 0 表示成功，-1 表示参数非法
    */
    static int RGB2HSV(double red, double green, double blue, double* hue, double* sat, double* value);

    /** HSL 转换为 RGB（输出归一化的分量）
    * @param [in] hue 色相（0~360）
    * @param [in] sat 饱和度（0~1）
    * @param [in] lightness 亮度（0~1）
    * @param [out] red 返回红色分量（0~1）
    * @param [out] green 返回绿色分量（0~1）
    * @param [out] blue 返回蓝色分量（0~1）
    * @return 返回 0 表示成功，-1 表示参数非法
    */
    static int HSL2RGB(double hue, double sat, double lightness, double* red, double* green, double* blue);

    /** HSL 转换为 RGB（输出 0~255 的分量）
    * @param [in] hue 色相（0~360）
    * @param [in] sat 饱和度（0~1）
    * @param [in] lightness 亮度（0~1）
    * @param [out] red 返回红色分量（0~255）
    * @param [out] green 返回绿色分量（0~255）
    * @param [out] blue 返回蓝色分量（0~255）
    * @return 返回 0 表示成功，-1 表示参数非法
    */
    static int HSL2RGB(double hue, double sat, double lightness, uint8_t& red, uint8_t& green, uint8_t& blue);

    /** RGB 转换为 HSL
    * @param [in] red 红色分量（0~1）
    * @param [in] green 绿色分量（0~1）
    * @param [in] blue 蓝色分量（0~1）
    * @param [out] hue 返回色相（0~360）
    * @param [out] sat 返回饱和度（0~1）
    * @param [out] lightness 返回亮度（0~1）
    * @return 返回 0 表示成功，-1 表示参数非法
    */
    static int RGB2HSL(double red, double green, double blue, double* hue, double* sat, double* lightness);

    /** 在起始颜色与结束颜色之间做线性插值，填充颜色采样数据
    * @param [out] buffer 输出缓冲区（存放 BGR 顺序的颜色值）
    * @param [in] samples 采样点数量
    * @param [in] start 起始颜色
    * @param [in] end 结束颜色
    */
    static void GetRGB(uint32_t* buffer, int samples, UiColorRef start, UiColorRef end);

    /** 生成色相渐变（固定饱和度与明度）
    * @param [out] buffer 输出缓冲区
    * @param [in] samples 采样点数量
    * @param [in] sat 固定的饱和度
    * @param [in] val 固定的明度
    */
    static void HSV_HUE(uint32_t* buffer, int samples, double sat, double val);

    /** 生成饱和度渐变（固定色相与明度）
    * @param [out] buffer 输出缓冲区
    * @param [in] samples 采样点数量
    * @param [in] hue 固定的色相
    * @param [in] val 固定的明度
    */
    static void HSV_SAT(uint32_t* buffer, int samples, double hue, double val);

    /** 生成明度渐变（固定色相与饱和度）
    * @param [out] buffer 输出缓冲区
    * @param [in] samples 采样点数量
    * @param [in] hue 固定的色相
    * @param [in] sat 固定的饱和度
    */
    static void HSV_VAL(uint32_t* buffer, int samples, double hue, double sat);

    /** 生成色相渐变（固定饱和度与亮度，HSL 空间）
    * @param [out] buffer 输出缓冲区
    * @param [in] samples 采样点数量
    * @param [in] sat 固定的饱和度
    * @param [in] lightness 固定的亮度
    */
    static void HSL_HUE(uint32_t* buffer, int samples, double sat, double lightness);

    /** 生成饱和度渐变（固定色相与亮度，HSL 空间）
    * @param [out] buffer 输出缓冲区
    * @param [in] samples 采样点数量
    * @param [in] hue 固定的色相
    * @param [in] lightness 固定的亮度
    */
    static void HSL_SAT(uint32_t* buffer, int samples, double hue, double lightness);

    /** 生成亮度渐变（固定色相与饱和度，HSL 空间）
    * @param [out] buffer 输出缓冲区
    * @param [in] samples 采样点数量
    * @param [in] hue 固定的色相
    * @param [in] sat 固定的饱和度
    */
    static void HSL_LIG(uint32_t* buffer, int samples, double hue, double sat);
};

} //namespace ui

#endif //UI_CONTROL_COLOR_CONVERT_H_
