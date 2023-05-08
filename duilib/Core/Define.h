#ifndef UI_CORE_DEFINE_H_
#define UI_CORE_DEFINE_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <string>

namespace ui
{
    class Control;

    struct EventArgs
    {
        EventArgs() :
            Type(kEventNone),
            pSender(nullptr),
            dwTimestamp(0),
            chKey(0),
            wParam(0),
            lParam(0)
        {
            ptMouse.x = 0;
            ptMouse.y = 0;
        }

        EventType Type;
        Control* pSender;
        DWORD dwTimestamp;
        POINT ptMouse;
        TCHAR chKey;
        WPARAM wParam;
        LPARAM lParam;
    };

    EventType StringToEnum(const std::wstring& messageType);

}// namespace ui

#endif // UI_CORE_DEFINE_H_