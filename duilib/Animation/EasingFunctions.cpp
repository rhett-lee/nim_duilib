#include "EasingFunctions.h"
#include <map>
#include <cmath>

////////////////////////////////////////////////////////////////
//效果参考：https://easings.net/

namespace ui
{
// 定义PI常量（如果系统未定义）
#ifndef PI
#define PI 3.14159265358979323846
#endif

/**
 * @brief 线性缓动函数（浮点版，归一化输出）
 * @param t 归一化时间（0=开始，1=结束），函数内部会自动钳位到0~1
 * @return double 缓动后的值（0~1，与输入t完全一致）
 */
static double easeLinear(double t)
{
    // 边界保护：确保t在0~1范围内，避免越界
    t = std::fmax(0.0, std::fmin(1.0, t));
    return t; // 线性缓动核心：输出=输入
}

/**
* @brief 正弦缓入：起始慢，逐渐加速
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInSine(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return 1.0 - cos(t * PI / 2.0); // 标准公式：1 - cos(πt/2)
}

/**
* @brief 正弦缓出：起始快，逐渐减速
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeOutSine(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return sin(t * PI / 2.0); // 标准公式：sin(πt/2)，无需修改原t
}

/**
* @brief 正弦缓入缓出：起始慢、中间快、结尾慢
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInOutSine(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return -0.5 * (cos(PI * t) - 1.0); // 标准公式：-0.5*(cos(πt)-1)
}

/**
* @brief 二次缓入（t²）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInQuad(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return t * t;
}

/**
* @brief 二次缓出（t*(2-t)）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeOutQuad(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return t * (2.0 - t);
}

/**
* @brief 二次缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInOutQuad(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    if (t < 0.5) {
        return 2.0 * t * t;
    }
    else {
        t -= 0.5;
        return 1.0 - 2.0 * t * t; // 标准公式，避免超出0~1范围
    }
}

/**
* @brief 三次缓入（t³）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInCubic(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return t * t * t;
}

/**
* @brief 三次缓出（1+(t-1)³）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeOutCubic(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    double temp = t - 1.0; // 临时变量，不修改原t
    return 1.0 + temp * temp * temp;
}

/**
* @brief 三次缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInOutCubic(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    if (t < 0.5) {
        return 4.0 * t * t * t; // 前半段：4t³
    }
    else {
        double temp = 2.0 * t - 2.0; // 临时变量，不修改原t
        return 1.0 + 0.5 * temp * temp * temp; // 后半段：1 - 4(1-t)³
    }
}

/**
* @brief 四次缓入（t⁴）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInQuart(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    t *= t;
    return t * t;
}

/**
* @brief 四次缓出（1-(t-1)⁴）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeOutQuart(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    double temp = t - 1.0; // 临时变量，不修改原t
    temp *= temp;
    return 1.0 - temp * temp;
}

/**
* @brief 四次缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInOutQuart(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    if (t < 0.5) {
        double temp = 2.0 * t;
        temp *= temp;
        return 0.5 * temp * temp;
    }
    else {
        double temp = 2.0 * (t - 1.0);
        temp *= temp;
        return 1.0 - 0.5 * temp * temp;
    }
}

/**
* @brief 五次缓入（t⁵）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInQuint(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    double t2 = t * t;
    return t * t2 * t2; // t*t²*t² = t⁵
}

/**
* @brief 五次缓出（1+(t-1)⁵）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeOutQuint(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    double temp = t - 1.0; // 临时变量，不修改原t
    double temp2 = temp * temp;
    return 1.0 + temp * temp2 * temp2;
}

/**
* @brief 五次缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInOutQuint(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    if (t < 0.5) {
        double temp = 2.0 * t;
        double temp2 = temp * temp;
        return 0.5 * temp * temp2 * temp2; // 16t⁵ / 2 = 8t⁵
    }
    else {
        double temp = 2.0 * (t - 1.0);
        double temp2 = temp * temp;
        return 1.0 + 0.5 * temp * temp2 * temp2; // 1 - 8(1-t)⁵
    }
}

/**
* @brief 指数缓入（(2^(8t)-1)/255）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInExpo(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return t == 0.0 ? 0.0 : (pow(2.0, 8.0 * t) - 1.0) / 255.0;
}

/**
* @brief 指数缓出（1-2^(-8t)）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeOutExpo(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return t == 1.0 ? 1.0 : 1.0 - pow(2.0, -8.0 * t);
}

/**
* @brief 指数缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInOutExpo(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    if (t == 0.0 || t == 1.0) {
        return t;
    }
    if (t < 0.5) {
        return (pow(2.0, 16.0 * t) - 1.0) / 510.0;
    }
    else {
        return 1.0 - 0.5 * pow(2.0, -16.0 * (t - 0.5));
    }
}

/**
* @brief 圆形缓入（1-sqrt(1-t²)）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInCirc(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return 1.0 - sqrt(1.0 - t * t);
}

/**
* @brief 圆形缓出（sqrt(1-(t-1)²)）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeOutCirc(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    double temp = t - 1.0;
    return sqrt(1.0 - temp * temp);
}

/**
* @brief 圆形缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInOutCirc(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    if (t < 0.5) {
        double temp = 2.0 * t;
        return 0.5 * (1.0 - sqrt(1.0 - temp * temp));
    }
    else {
        double temp = 2.0 * (t - 1.0);
        return 0.5 * (sqrt(1.0 - temp * temp) + 1.0);
    }
}

/**
* @brief 回退缓入（先反向再前进）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（可能短暂<0）
*/
static double easeInBack(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    const double c1 = 1.70158; // 经典回退系数
    const double c3 = c1 + 1.0;
    return c3 * t * t * t - c1 * t * t;
}

/**
* @brief 回退缓出（超过目标再回弹）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（可能短暂>1）
*/
static double easeOutBack(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    const double c1 = 1.70158;
    const double c3 = c1 + 1.0;
    double temp = t - 1.0;
    return 1.0 + c3 * temp * temp * temp + c1 * temp * temp;
}

/**
* @brief 回退缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（可能短暂超出0~1）
*/
static double easeInOutBack(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    const double c1 = 1.70158;
    const double c2 = c1 * 1.525; // 缓入缓出系数
    if (t < 0.5) {
        double temp = 2.0 * t;
        return 0.5 * (temp * temp * ((c2 + 1.0) * temp - c2));
    }
    else {
        double temp = 2.0 * (t - 1.0);
        return 0.5 * (2.0 + temp * temp * ((c2 + 1.0) * temp + c2));
    }
}

/**
* @brief 弹性缓入（模拟弹性拉伸）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（可能有波动）
*/
static double easeInElastic(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    const double c4 = (2.0 * PI) / 3.0; // 弹性系数
    if (t == 0.0 || t == 1.0) {
        return t;
    }
    return -pow(2.0, 10.0 * t - 10.0) * sin((t * 10.0 - 10.75) * c4);
}

/**
* @brief 弹性缓出（模拟弹性回弹）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（可能有波动）
*/
static double easeOutElastic(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    const double c4 = (2.0 * PI) / 3.0;
    if (t == 0.0 || t == 1.0) {
        return t;
    }
    return pow(2.0, -10.0 * t) * sin((t * 10.0 - 0.75) * c4) + 1.0;
}

/**
* @brief 弹性缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（可能有波动）
*/
static double easeInOutElastic(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    const double c5 = (2.0 * PI) / 4.5;
    if (t == 0.0 || t == 1.0) {
        return t;
    }
    if (t < 0.5) {
        return -0.5 * pow(2.0, 20.0 * t - 10.0) * sin((20.0 * t - 11.125) * c5);
    }
    else {
        return pow(2.0, -20.0 * t + 10.0) * sin((20.0 * t - 11.125) * c5) * 0.5 + 1.0;
    }
}

/**
* @brief 回弹缓出（模拟弹跳上升）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeOutBounce(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    const double n1 = 7.5625; // 回弹系数
    const double d1 = 2.75;    // 回弹除数
    if (t < 1.0 / d1) {
        return n1 * t * t;
    }
    else if (t < 2.0 / d1) {
        t -= 1.5 / d1;
        return n1 * t * t + 0.75;
    }
    else if (t < 2.5 / d1) {
        t -= 2.25 / d1;
        return n1 * t * t + 0.9375;
    }
    else {
        t -= 2.625 / d1;
        return n1 * t * t + 0.984375;
    }
}

/**
* @brief 回弹缓入（模拟弹跳落地）
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInBounce(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    return 1.0 - easeOutBounce(1.0 - t); // 复用缓出逻辑，反向实现
}

/**
* @brief 回弹缓入缓出
* @param t 归一化时间（0~1）
* @return 变换后的缓动值（0~1）
*/
static double easeInOutBounce(double t) {
    t = std::fmax(0.0, std::fmin(1.0, t));
    if (t < 0.5) {
        return (1.0 - easeOutBounce(1.0 - 2.0 * t)) * 0.5;
    }
    else {
        return (easeOutBounce(2.0 * t - 1.0) + 1.0) * 0.5;
    }
}

/**
 * @brief 根据枚举值获取缓动函数指针
 * @param function 缓动函数枚举值
 * @return 对应缓动函数指针，未找到则返回nullptr
 */
EasingFunction EasingFunctions::GetEasingFunction(EasingFunctionType function)
{
    // 静态map：只初始化一次，提升性能
    static std::map<EasingFunctionType, EasingFunction> easingFunctions;
    if (easingFunctions.empty()) {
        // 初始化枚举与函数的映射关系
        easingFunctions.insert(std::make_pair(EaseLinear,        easeLinear));
        easingFunctions.insert(std::make_pair(EaseInSine,        easeInSine));
        easingFunctions.insert(std::make_pair(EaseOutSine,       easeOutSine));
        easingFunctions.insert(std::make_pair(EaseInOutSine,     easeInOutSine));
        easingFunctions.insert(std::make_pair(EaseInQuad,        easeInQuad));
        easingFunctions.insert(std::make_pair(EaseOutQuad,       easeOutQuad));
        easingFunctions.insert(std::make_pair(EaseInOutQuad,     easeInOutQuad));
        easingFunctions.insert(std::make_pair(EaseInCubic,       easeInCubic));
        easingFunctions.insert(std::make_pair(EaseOutCubic,      easeOutCubic));
        easingFunctions.insert(std::make_pair(EaseInOutCubic,    easeInOutCubic));
        easingFunctions.insert(std::make_pair(EaseInQuart,       easeInQuart));
        easingFunctions.insert(std::make_pair(EaseOutQuart,      easeOutQuart));
        easingFunctions.insert(std::make_pair(EaseInOutQuart,    easeInOutQuart));
        easingFunctions.insert(std::make_pair(EaseInQuint,       easeInQuint));
        easingFunctions.insert(std::make_pair(EaseOutQuint,      easeOutQuint));
        easingFunctions.insert(std::make_pair(EaseInOutQuint,    easeInOutQuint));
        easingFunctions.insert(std::make_pair(EaseInExpo,        easeInExpo));
        easingFunctions.insert(std::make_pair(EaseOutExpo,       easeOutExpo));
        easingFunctions.insert(std::make_pair(EaseInOutExpo,     easeInOutExpo));
        easingFunctions.insert(std::make_pair(EaseInCirc,        easeInCirc));
        easingFunctions.insert(std::make_pair(EaseOutCirc,       easeOutCirc));
        easingFunctions.insert(std::make_pair(EaseInOutCirc,     easeInOutCirc));
        easingFunctions.insert(std::make_pair(EaseInBack,        easeInBack));
        easingFunctions.insert(std::make_pair(EaseOutBack,       easeOutBack));
        easingFunctions.insert(std::make_pair(EaseInOutBack,     easeInOutBack));
        easingFunctions.insert(std::make_pair(EaseInElastic,     easeInElastic));
        easingFunctions.insert(std::make_pair(EaseOutElastic,    easeOutElastic));
        easingFunctions.insert(std::make_pair(EaseInOutElastic,  easeInOutElastic));
        easingFunctions.insert(std::make_pair(EaseInBounce,      easeInBounce));
        easingFunctions.insert(std::make_pair(EaseOutBounce,     easeOutBounce));
        easingFunctions.insert(std::make_pair(EaseInOutBounce,   easeInOutBounce));
    }

    // 查找对应函数
    auto it = easingFunctions.find(function);
    return it == easingFunctions.end() ? nullptr : it->second;
}

EasingFunctionType EasingFunctions::GetEasingFunctionType(const DString& easingFunctionName)
{
    // 静态map：只初始化一次，提升性能
    static std::map<DString, EasingFunctionType> easingFunctions;
    if (easingFunctions.empty()) {
        // 初始化枚举与函数的映射关系
        easingFunctions.insert(std::make_pair(_T("EaseLinear"), EaseLinear));
        easingFunctions.insert(std::make_pair(_T("EaseInSine"), EaseInSine));
        easingFunctions.insert(std::make_pair(_T("EaseOutSine"), EaseOutSine));
        easingFunctions.insert(std::make_pair(_T("EaseInOutSine"), EaseInOutSine));
        easingFunctions.insert(std::make_pair(_T("EaseInQuad"), EaseInQuad));
        easingFunctions.insert(std::make_pair(_T("EaseOutQuad"), EaseOutQuad));
        easingFunctions.insert(std::make_pair(_T("EaseInOutQuad"), EaseInOutQuad));
        easingFunctions.insert(std::make_pair(_T("EaseInCubic"), EaseInCubic));
        easingFunctions.insert(std::make_pair(_T("EaseOutCubic"), EaseOutCubic));
        easingFunctions.insert(std::make_pair(_T("EaseInOutCubic"), EaseInOutCubic));
        easingFunctions.insert(std::make_pair(_T("EaseInQuart"), EaseInQuart));
        easingFunctions.insert(std::make_pair(_T("EaseOutQuart"), EaseOutQuart));
        easingFunctions.insert(std::make_pair(_T("EaseInOutQuart"), EaseInOutQuart));
        easingFunctions.insert(std::make_pair(_T("EaseInQuint"), EaseInQuint));
        easingFunctions.insert(std::make_pair(_T("EaseOutQuint"), EaseOutQuint));
        easingFunctions.insert(std::make_pair(_T("EaseInOutQuint"), EaseInOutQuint));
        easingFunctions.insert(std::make_pair(_T("EaseInExpo"), EaseInExpo));
        easingFunctions.insert(std::make_pair(_T("EaseOutExpo"), EaseOutExpo));
        easingFunctions.insert(std::make_pair(_T("EaseInOutExpo"), EaseInOutExpo));
        easingFunctions.insert(std::make_pair(_T("EaseInCirc"), EaseInCirc));
        easingFunctions.insert(std::make_pair(_T("EaseOutCirc"), EaseOutCirc));
        easingFunctions.insert(std::make_pair(_T("EaseInOutCirc"), EaseInOutCirc));
        easingFunctions.insert(std::make_pair(_T("EaseInBack"), EaseInBack));
        easingFunctions.insert(std::make_pair(_T("EaseOutBack"), EaseOutBack));
        easingFunctions.insert(std::make_pair(_T("EaseInOutBack"), EaseInOutBack));
        easingFunctions.insert(std::make_pair(_T("EaseInElastic"), EaseInElastic));
        easingFunctions.insert(std::make_pair(_T("EaseOutElastic"), EaseOutElastic));
        easingFunctions.insert(std::make_pair(_T("EaseInOutElastic"), EaseInOutElastic));
        easingFunctions.insert(std::make_pair(_T("EaseInBounce"), EaseInBounce));
        easingFunctions.insert(std::make_pair(_T("EaseOutBounce"), EaseOutBounce));
        easingFunctions.insert(std::make_pair(_T("EaseInOutBounce"), EaseInOutBounce));
    }

    // 查找对应函数
    auto it = easingFunctions.find(easingFunctionName);
    return it == easingFunctions.end() ? EaseLinear : it->second;
}

EasingFunctions::EasingFunctions(int64_t nStartValue, int64_t nEndValue, int32_t nFrameCount, EasingFunctionType easingFunctionType) :
    m_nStartValue(nStartValue),
    m_nEndValue(nEndValue),
    m_nFrameCount(nFrameCount)
{
    m_easingFunction = GetEasingFunction(easingFunctionType);
    ASSERT(m_easingFunction != nullptr);
    if (m_easingFunction == nullptr) {
        m_easingFunction = easeLinear;
    }
    ASSERT(m_nFrameCount > 0);
    if (m_nFrameCount < 1) {
        m_nFrameCount = 1;
    }
}

EasingFunctions::~EasingFunctions()
{

}

/** 缓动插值函数：消除浮点数精度误差，支持可选的精度截断
 * @details 核心功能是将0~1范围内的归一化时间t，通过缓动函数映射到[start, end]的数值范围，
 *          关键优化点：t=1时强制返回end值，彻底消除浮点数运算带来的精度误差。
 *
 * @param start 插值的起始数值（如初始坐标、初始透明度、初始音量等）
 * @param end 插值的目标数值（如目标坐标、目标透明度、目标音量等）
 * @param t 归一化时间值，范围理论上为0~1（0=插值开始，1=插值结束），函数内部会自动钳位到0~1
 * @param func 缓动函数指针，可选参数（默认nullptr），传入时使用指定缓动效果，未传入时使用线性插值
 * @return double 插值后的最终数值，无浮点数精度误差（t=1时精准返回end）
 */
static double InterpolateFunction(double start, double end, double t, EasingFunction func = nullptr)
{
    // 边界钳位：确保t始终在0~1范围内，避免越界导致非预期结果
    t = (t < 0) ? 0 : (t > 1.0) ? 1.0 : t;

    double result = 0.0;
    if (!func) {
        // 无缓动函数时，使用线性插值
        result = start + (end - start) * t;
    }
    else {
        // 有缓动函数时，先计算缓动后的t值，再映射到[start, end]范围
        double easedT = func(t);
        result = start + (end - start) * easedT;
    }

    // 关键优化：t=1时强制返回目标值，消除所有浮点数运算累积的精度误差
    if (t >= 1.0) {
        result = end;
    }
    else if (t <= 0.0) {
        result = start;
    }
    return result;
}

int64_t EasingFunctions::GetEasingValue(int32_t nCurrentFrame) const
{
    int64_t nEasingValue = m_nEndValue;
    ASSERT(nCurrentFrame >= 0);
    if (nCurrentFrame < 0) {
        return nEasingValue;
    }
    if (nCurrentFrame >= m_nFrameCount) {
        return nEasingValue;
    }
    if ((m_nEndValue == m_nStartValue) || (m_nFrameCount <= 0)) {
        return nEasingValue;
    }

    // 定义double能精确表示的int64_t上下限（2^53）
    const int64_t DOUBLE_INT_PRECISION_MAX = 9007199254740992LL;
    const int64_t DOUBLE_INT_PRECISION_MIN = -9007199254740992LL;
    ASSERT((m_nStartValue >= DOUBLE_INT_PRECISION_MIN) && (m_nStartValue <= DOUBLE_INT_PRECISION_MAX));
    ASSERT((m_nEndValue >= DOUBLE_INT_PRECISION_MIN) && (m_nEndValue <= DOUBLE_INT_PRECISION_MAX));
    if (m_nStartValue > DOUBLE_INT_PRECISION_MAX || m_nStartValue < DOUBLE_INT_PRECISION_MIN) {
        return nEasingValue;
    }
    if (m_nEndValue > DOUBLE_INT_PRECISION_MAX || m_nEndValue < DOUBLE_INT_PRECISION_MIN) {
        return nEasingValue;
    }

    double start = static_cast<double>(m_nStartValue);
    double end = static_cast<double>(m_nEndValue);
    double t = static_cast<double>(nCurrentFrame) / static_cast<double>(m_nFrameCount);
    double fValue = InterpolateFunction(start, end, t, m_easingFunction);
    if (!std::isnan(fValue) && !std::isinf(fValue)) {
        nEasingValue = static_cast<int64_t>(std::round(fValue));
    }
    return nEasingValue;
}

int64_t EasingFunctions::GetStartValue() const
{
    return m_nStartValue;
}

int64_t EasingFunctions::GetEndValue() const
{
    return m_nEndValue;
}

int32_t EasingFunctions::GetFrameCount() const
{
    return m_nFrameCount;
}

} //namespace ui
