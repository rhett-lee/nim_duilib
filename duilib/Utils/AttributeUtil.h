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

	/** 从配置中解析一个Size结构，输入格式如："800,500"
	*/
	static void ParseSizeValue(const wchar_t* strValue, UiSize& size);

	/** 从配置中解析一个Point结构，输入格式如："800,500"
	*/
	static void ParsePointValue(const wchar_t* strValue, UiPoint& size);

	/** 从配置中解析一个Rect结构，输入格式如："80,50,60,70"
	*/
	static void ParseRectValue(const wchar_t* strValue, UiRect& rect);
	static void ParsePaddingValue(const wchar_t* strValue, UiPadding& padding);
	static void ParseMarginValue(const wchar_t* strValue, UiMargin& margin);

	/** 解析属性列表，格式如	: font="system_bold_14" normaltextcolor="white" bkcolor="red"
	*                 或者	: color='black' offset='1,1' blur_radius='2' spread_radius='2'
	*   分隔符可以是双引号或者单引号，由参数传入
	* @param [in] seperateChar 字符串分隔符，可以是 L'\"' 或者 L'\''
	*/
	static void ParseAttributeList(const std::wstring& strList,
		                           wchar_t seperateChar,
		                           std::vector<std::pair<std::wstring, std::wstring>>& attributeList);

	/** 解析一个字符串（格式为："500,"或者"50%,"，逗号可有可无，也可以是其他字符），得到整型值或者浮点数
    * @param [in] strValue 待解析的字符串地址
    * @param [out] pEndPtr 解析完成后，字符串结束地址，用于继续解析后面的内容
    * @return 返回整型值或者百分比，返回值中，0 或者 0.0f 表示无效值
    */
	static std::tuple<int32_t, float> ParseString(const wchar_t* strValue, wchar_t** pEndPtr);

    /** 获取配置中的窗口大小
    */
	static void ParseWindowSize(Window* pWindow, const wchar_t* strValue, UiSize& size);
};

} //namespace ui

#endif // UI_UTILS_ATTRIBUTUTIL_H_
