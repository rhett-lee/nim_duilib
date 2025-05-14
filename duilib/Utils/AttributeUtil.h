#ifndef UI_UTILS_ATTRIBUTUTIL_H_
#define UI_UTILS_ATTRIBUTUTIL_H_

#include "duilib/Core/UiTypes.h"
#include <string>
#include <vector>
#include <tuple>

namespace ui
{
class Window;
class UILIB_API AttributeUtil
{
public:
    /** 配置字符串解析：跳过一个分隔符
    */
    static void SkipSepChar(wchar_t*& pPtr);
    static void SkipSepChar(char*& pPtr);

    /** 从配置中解析一个Size结构，输入格式如："800,500"
    */
    static void ParseSizeValue(const wchar_t* strValue, UiSize& size);
    static void ParseSizeValue(const char* strValue, UiSize& size);

    /** 从配置中解析一个Point结构，输入格式如："800,500"
    */
    static void ParsePointValue(const wchar_t* strValue, UiPoint& size);
    static void ParsePointValue(const char* strValue, UiPoint& size);

    /** 从配置中解析一个Rect结构，输入格式如："80,50,60,70"
    */
    static void ParseRectValue(const wchar_t* strValue, UiRect& rect);
    static void ParsePaddingValue(const wchar_t* strValue, UiPadding& padding);
    static void ParseMarginValue(const wchar_t* strValue, UiMargin& margin);

    static void ParseRectValue(const char* strValue, UiRect& rect);
    static void ParsePaddingValue(const char* strValue, UiPadding& padding);
    static void ParseMarginValue(const char* strValue, UiMargin& margin);

    /** 解析属性列表，格式如    : font="system_bold_14" normaltextcolor="white" bkcolor="red"
    *                 或者    : color='black' offset='1,1' blur_radius='2' spread_radius='2'
    *   分隔符可以是双引号或者单引号，由参数传入
    * @param [in] seperateChar 字符串分隔符，可以是 _T('\"') 或者 _T('\')'
    */
    static void ParseAttributeList(const DString& strList,
                                   DString::value_type seperateChar,
                                   std::vector<std::pair<DString, DString>>& attributeList);

    /** 解析一个字符串（格式为："500,"或者"50%,"，逗号可有可无，也可以是其他字符），得到整型值或者浮点数
    * @param [in] strValue 待解析的字符串地址
    * @param [out] pEndPtr 解析完成后，字符串结束地址，用于继续解析后面的内容
    * @return 返回整型值或者百分比，返回值中，0 或者 0.0f 表示无效值
    */
    static std::tuple<int32_t, float> ParseString(const wchar_t* strValue, wchar_t** pEndPtr);
    static std::tuple<int32_t, float> ParseString(const char* strValue, char** pEndPtr);

    /** 获取配置中的窗口大小
    * @param [in] pWindow 关联的窗口，可以为nullptr
    * @param [in] strValue 需要解析的窗口大小字符串
    * @param [out] size 解析出的窗口大小值（像素）
    * @param [out] pScaledCX 返回窗口宽度size.cx值是否执行过DPI缩放
    * @param [out] pScaledCY 返回窗口高度size.cy值是否执行过DPI缩放
    * @param [out] pPercentCX 返回窗口宽度size.cx值的配置是否为屏幕百分比
    * @param [out] pPercentCY 返回窗口高度size.cy值的配置是否为屏幕百分比
    */
    static void ParseWindowSize(const Window* pWindow, const DString::value_type* strValue,
                                UiSize& size,
                                bool* pScaledCX, bool* pScaledCY,
                                bool* pPercentCX, bool* pPercentCY);
};

} //namespace ui

#endif // UI_UTILS_ATTRIBUTUTIL_H_
