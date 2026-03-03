#ifndef UI_ANIMATION_EASING_FUNCTIONS_H_
#define UI_ANIMATION_EASING_FUNCTIONS_H_

////////////////////////////////////////////////////////////////
//效果参考：https://easings.net/

#include "duilib/duilib_defs.h"

namespace ui
{
/** 缓动函数类型枚举
 * 涵盖常用的缓动曲线：正弦、二次、三次、四次、五次、指数、圆形、回退、弹性、回弹
 * 每种曲线分为In(缓入)、Out(缓出)、InOut(缓入缓出)三种形式
 */
enum UILIB_API EasingFunctionType: uint8_t
{
    EaseLinear,          // 线性函数

    EaseInSine,          // 正弦缓入
    EaseOutSine,         // 正弦缓出
    EaseInOutSine,       // 正弦缓入缓出
    EaseInQuad,          // 二次缓入
    EaseOutQuad,         // 二次缓出
    EaseInOutQuad,       // 二次缓入缓出
    EaseInCubic,         // 三次缓入
    EaseOutCubic,        // 三次缓出
    EaseInOutCubic,      // 三次缓入缓出
    EaseInQuart,         // 四次缓入
    EaseOutQuart,        // 四次缓出
    EaseInOutQuart,      // 四次缓入缓出
    EaseInQuint,         // 五次缓入
    EaseOutQuint,        // 五次缓出
    EaseInOutQuint,      // 五次缓入缓出
    EaseInExpo,          // 指数缓入
    EaseOutExpo,         // 指数缓出
    EaseInOutExpo,       // 指数缓入缓出
    EaseInCirc,          // 圆形缓入
    EaseOutCirc,         // 圆形缓出
    EaseInOutCirc,       // 圆形缓入缓出
    EaseInBack,          // 回退缓入（先反向再前进）
    EaseOutBack,         // 回退缓出（超过目标再回弹）
    EaseInOutBack,       // 回退缓入缓出
    EaseInElastic,       // 弹性缓入
    EaseOutElastic,      // 弹性缓出
    EaseInOutElastic,    // 弹性缓入缓出
    EaseInBounce,        // 回弹缓入
    EaseOutBounce,       // 回弹缓出
    EaseInOutBounce      // 回弹缓入缓出
};

/** 缓动函数的函数指针类型声明前置
*/
typedef double(*EasingFunction)(double);

/** 缓动函数的插值函数封装(供应用层使用)
*/
class UILIB_API EasingFunctions
{
public:
    /** 根据枚举类型获取对应的缓动函数指针
     * @param function 缓动函数枚举值
     * @return 对应缓动函数的指针，若不存在则返回nullptr
     */
    static EasingFunction GetEasingFunction(EasingFunctionType function);

    /** 根据缓动函数的字符串名称，获取缓动函数类型
    * @param [in] easingFunctionName 缓动函数的名称，字符串名称与EasingFunctionType的枚举值定义完全一致
    */
    static EasingFunctionType GetEasingFunctionType(DString easingFunctionName);

public:
    /** 构造函数
    * @param [in] nStartValue 起始值
    * @param [in] nEndValue 结束值
    * @param [in] nFrameCount 动画总的帧数
    * @param [in] easingFunctionType 使用的缓动函数类型
    */
    EasingFunctions(int32_t nStartValue, int32_t nEndValue, int32_t nFrameCount, EasingFunctionType easingFunctionType);
    ~EasingFunctions();

public:
    /** 获取缓动的值
    * @param [in] nCurrentFrame 当前帧的序号，序号从0开始，有效值的范围是[0, nFrameCount]，闭区间
    * @return 返回缓动的值，有效值的范围是[nStartValue, nEndValue]，闭区间
    */
    int32_t GetEasingValue(int32_t nCurrentFrame) const;

    /** 获取动画播放起始值
    */
    int32_t GetStartValue() const;

    /** 获取动画播放结束值
    */
    int32_t GetEndValue() const;

    /** 获取动画总的帧数
    */
    int32_t GetFrameCount() const;

private:
    /** 起始值（可能是宽度、高度、透明度等）
    */
    int32_t m_nStartValue;

    /** 结束值（可能是宽度、高度、透明度等）
    */
    int32_t m_nEndValue;

    /** 动画总的帧数
    */
    int32_t m_nFrameCount;

    /** 动画使用的缓动函数
    */
    EasingFunction m_easingFunction;
};

} //namespace ui

#endif //#define UI_ANIMATION_EASING_FUNCTIONS_H_
