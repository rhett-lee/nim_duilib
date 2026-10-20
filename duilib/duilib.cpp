#include "duilib.h"

// Windows, MacOS adn Linux support CEFControl
// FreeBSD does not support CEFControl
// CMake构建时，可通过 DUILIB_ENABLE_CEF=OFF 关闭CEF功能（此时定义 DUILIB_NO_CEF）
#if !defined(__FreeBSD__) && !defined(DUILIB_NO_CEF)
    #include "duilib_cef.h"
#endif
