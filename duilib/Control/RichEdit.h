#ifndef UI_CONTROL_RICHEDIT_H_
#define UI_CONTROL_RICHEDIT_H_

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    #include "duilib/Control/RichEdit_Windows.h"
#elif defined (DUILIB_BUILD_FOR_SDL)
    #include "duilib/Control/RichEdit_SDL.h"
#endif

#endif // UI_CONTROL_RICHEDIT_H_
