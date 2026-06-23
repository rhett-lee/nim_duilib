#ifndef UI_CONTROL_RICHEDIT_DEFS_H_
#define UI_CONTROL_RICHEDIT_DEFS_H_

/** RichEdit与RichEdit2类的公共部分定义
*/
#include "duilib/Box/ScrollBox.h"
#include "duilib/Image/Image.h"

namespace ui
{
/** 字符的索引号范围
*/
struct TextCharRange
{
    int32_t cpMin = -1; //字符的起始索引值
    int32_t cpMax = -1; //字符的结束索引值
};

/** 字符查找的参数
*/
struct FindTextParam
{
    bool bMatchCase = true;      //查找时是否区分大小写
    bool bMatchWholeWord = true; //查找时，是否按词匹配
    bool bFindDown = true;       //是否向后查找，为true表示向后查找，false表示反向查找
    TextCharRange chrg;          //字符的查找范围
    DString findText;            //查找的文本
};

} // namespace ui

#endif // UI_CONTROL_RICHEDIT_DEFS_H_
