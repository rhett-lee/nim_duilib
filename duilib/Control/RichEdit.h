#ifndef UI_CONTROL_RICHEDIT_H_
#define UI_CONTROL_RICHEDIT_H_

// RichEdit2 支持所有平台
#include "duilib/Control/RichEdit2.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    #include "duilib/Control/RichEdit_Windows.h"
#endif

#endif // UI_CONTROL_RICHEDIT_H_
