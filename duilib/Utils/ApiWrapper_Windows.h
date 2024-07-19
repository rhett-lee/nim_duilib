#ifndef UI_UTILS_APIWRAPPER_WINDOWS_H_
#define UI_UTILS_APIWRAPPER_WINDOWS_H_

#include "duilib/Utils/Macros_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui
{
    // DPI适配
    //Windows 10版本 1607
    bool GetDpiForSystemWrapper(UINT& dpi);
    bool GetDpiForWindowWrapper(HWND hwnd, UINT& dpi);
    int GetSystemMetricsForDpiWrapper(int nIndex, UINT dpi);

    //Windows 8.1 
    bool GetDpiForMonitorWrapper(HMONITOR hMonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

    //Windows 10 版本 1703 以上
    bool SetProcessDpiAwarenessContextWrapper(PROCESS_DPI_AWARENESS_CONTEXT value);
    bool GetProcessDpiAwarenessContextWrapper(PROCESS_DPI_AWARENESS_CONTEXT& value);
    bool AreDpiAwarenessContextsEqualWrapper(PROCESS_DPI_AWARENESS_CONTEXT dpiContextA, PROCESS_DPI_AWARENESS_CONTEXT dpiContextB);
    
    //Windows 8.1以上
    bool SetProcessDPIAwarenessWrapper(PROCESS_DPI_AWARENESS value);
    bool GetProcessDPIAwarenessWrapper(PROCESS_DPI_AWARENESS& awareness);

    //Windows Vista 以上
    bool SetProcessDPIAwareWrapper();
    bool IsProcessDPIAwareWrapper(bool& bAware);

    // WM_TOUCH
    bool RegisterTouchWindowWrapper(HWND hwnd, ULONG ulFlags);
    bool UnregisterTouchWindowWrapper(HWND hwnd);

    bool GetTouchInputInfoWrapper(HTOUCHINPUT hTouchInput, UINT cInputs, PTOUCHINPUT pInputs, int cbSize);
    bool CloseTouchInputHandleWrapper(HTOUCHINPUT hTouchInput); 

    // WM_POINTER
    bool GetPointerTypeWrapper(UINT32 pointerId, POINTER_INPUT_TYPE *pointerType);
    bool GetPointerInfoWrapper(UINT32 pointerId, POINTER_INFO *pointerInfo);
    bool GetPointerTouchInfoWrapper(UINT32 pointerId, POINTER_TOUCH_INFO *touchInfo);
    bool GetPointerPenInfoWrapper(UINT32 pointerId, POINTER_PEN_INFO *penInfo);
    bool EnableMouseInPointerWrapper(BOOL fEnable);
}

#endif //DUILIB_BUILD_FOR_WIN

#endif //UI_UTILS_APIWRAPPER_WINDOWS_H_
