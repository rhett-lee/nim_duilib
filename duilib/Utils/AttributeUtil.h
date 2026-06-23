#ifndef UI_UTILS_ATTRIBUTUTIL_H_
#define UI_UTILS_ATTRIBUTUTIL_H_

#include "duilib/Core/UiTypes.h"
#include <string>
#include <vector>
#include <tuple>

namespace ui
{
class Window;
class DUILIB_API AttributeUtil
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
    static void ParseRectValue(const wchar_t* strValue, UiRect& rect, bool bCheckSize = true);
    static void ParsePaddingValue(const wchar_t* strValue, UiPadding& padding);
    static void ParseMarginValue(const wchar_t* strValue, UiMargin& margin);

    static void ParseRectValue(const char* strValue, UiRect& rect, bool bCheckSize = true);
    static void ParsePaddingValue(const char* strValue, UiPadding& padding);
    static void ParseMarginValue(const char* strValue, UiMargin& margin);

    /** 解析属性列表，有效的格式示例
     * (1) 使用双引号: font="system_bold_14" normal_text_color="white" bkcolor="red"
     * (2) 使用单引号: color='black' offset='1,1' blur_radius='2' spread_radius='2'
     * (3) 使用单引号和双引号混合：color="black" offset='1,1' blur_radius="2" spread_radius='2'
     * (4) 使用花括号(当无法用单引号和双引号时，可适当使用花括号):
     *     combo_button_class="height='stretch' hovered_image={file='test/arrow_hot.svg' valign='center'}"
     * @param [in] strList 需要解析的属性列表，格式如上说明
     * @param [out] attributeList 返回解析结果的name和value列表
     * @param [in] seperateStartChars 起始分隔符集合（如_T("\"'{["）
     * @param [in] seperateEndChars 结束分隔符集合，必须与起始分隔符集合配对（如_T("\"'}]"));
     * @return 当解析出至少一个属性-值时，返回true，否则返回false
     */
    static bool ParseAttributeList(const DString& strList, std::vector<std::pair<DString, DString>>& attributeList,
                                   const DString& seperateStartChars = _T("\"'{["),
                                   const DString& seperateEndChars = _T("\"'}]"));

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

    /** 对窗口大小进行校验，避免超出窗口所在屏幕的区域
    * @param [in] pWindow 关联的窗口，可以为nullptr
    * @param [in,out] nWindowWidth 窗口宽度
    * @param [in,out] nWindowHeight 窗口高度
    */
    static void ValidateWindowSize(const Window* pWindow, int32_t& nWindowWidth, int32_t& nWindowHeight);
};

} //namespace ui

#endif // UI_UTILS_ATTRIBUTUTIL_H_
