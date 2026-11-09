#include "CalendarWeekdayUtil.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

uint32_t ParseDisabledWeekdays(const DString& disabledWeekdays)
{
    //统一小写，并把常见分隔符都归一为逗号
    DString normalized = StringUtil::MakeLowerString(disabledWeekdays);
    for (auto& ch : normalized) {
        if ((ch == _T(';')) || (ch == _T('|')) || (ch == _T(' '))) {
            ch = _T(',');
        }
    }

    uint32_t mask = 0;
    size_t startPos = 0;
    while (startPos <= normalized.size()) {
        size_t endPos = normalized.find(_T(','), startPos);
        if (endPos == DString::npos) {
            endPos = normalized.size();
        }
        DString token = normalized.substr(startPos, endPos - startPos);
        token = StringUtil::Trim(token);

        int32_t dayOfWeek = -1;
        if ((token == _T("sunday")) || (token == _T("sun"))) {
            dayOfWeek = 0;
        }
        else if ((token == _T("monday")) || (token == _T("mon"))) {
            dayOfWeek = 1;
        }
        else if ((token == _T("tuesday")) || (token == _T("tue"))) {
            dayOfWeek = 2;
        }
        else if ((token == _T("wednesday")) || (token == _T("wed"))) {
            dayOfWeek = 3;
        }
        else if ((token == _T("thursday")) || (token == _T("thu"))) {
            dayOfWeek = 4;
        }
        else if ((token == _T("friday")) || (token == _T("fri"))) {
            dayOfWeek = 5;
        }
        else if ((token == _T("saturday")) || (token == _T("sat"))) {
            dayOfWeek = 6;
        }
        else if (!token.empty()) {
            //只接受纯数字（0~6）：StringToInt32 对非数字文本会返回 0，
            //若不校验字符，非法项会被当成周日而误禁用
            bool bNumeric = true;
            for (auto ch : token) {
                if ((ch < _T('0')) || (ch > _T('9'))) {
                    bNumeric = false;
                    break;
                }
            }
            if (bNumeric) {
                dayOfWeek = StringUtil::StringToInt32(token);
                if ((dayOfWeek < 0) || (dayOfWeek > 6)) {
                    dayOfWeek = -1;
                }
            }
        }
        if ((dayOfWeek >= 0) && (dayOfWeek <= 6)) {
            mask |= (1u << dayOfWeek);
        }
        if (endPos >= normalized.size()) {
            break;
        }
        startPos = endPos + 1;
    }
    return mask;
}

bool IsCalendarWeekdayDisabled(uint32_t weekdayMask, int32_t dayOfWeek)
{
    if ((dayOfWeek < 0) || (dayOfWeek > 6)) {
        return false;
    }
    return (weekdayMask & (1u << dayOfWeek)) != 0;
}

}//namespace ui
