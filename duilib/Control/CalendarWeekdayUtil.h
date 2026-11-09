#ifndef UI_CONTROL_CALENDAR_WEEKDAY_UTIL_H_
#define UI_CONTROL_CALENDAR_WEEKDAY_UTIL_H_

#include "duilib/duilib_defs.h"

namespace ui
{
/** 解析"每周禁用的星期"列表，返回星期掩码（bit0=周日, bit1=周一, ... bit6=周六）
* @param [in] disabledWeekdays 列表文本，多个项用 ',' 或 ';' 分隔（也接受 '|' 和空格）；
*            每项为 sunday/sun ... saturday/sat，或数字 0(周日) ... 6(周六)；非法项忽略
* @return 星期掩码，未禁用任何星期时返回 0
*/
DUILIB_API uint32_t ParseDisabledWeekdays(const DString& disabledWeekdays);

/** 判断某个星期是否被禁用
* @param [in] weekdayMask ParseDisabledWeekdays 返回的掩码
* @param [in] dayOfWeek 星期（0=周日 ... 6=周六），取值非法时返回 false
*/
DUILIB_API bool IsCalendarWeekdayDisabled(uint32_t weekdayMask, int32_t dayOfWeek);

}//namespace ui

#endif //UI_CONTROL_CALENDAR_WEEKDAY_UTIL_H_
