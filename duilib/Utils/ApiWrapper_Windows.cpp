#include "ApiWrapper_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include <VersionHelpers.h>

namespace ui
{

UINT GetDpiForWnd(HWND hWnd)
{
    if (!::IsWindow(hWnd)) {
        return 0;
    }
    uint32_t uDPI = 0;
    if ((uDPI == 0) && ::IsWindows8OrGreater()) {
        HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        if (hMonitor != nullptr) {
            uint32_t dpix = 96;
            uint32_t dpiy = 96;
            if (GetDpiForMonitorWrapper(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy)) {
                uDPI = dpix;
            }
        }
    }
    if (uDPI == 0) {
        HDC hDC = ::GetDC(hWnd);
        if (hDC != nullptr) {
            uDPI = (uint32_t)::GetDeviceCaps(hDC, LOGPIXELSX);
            ::ReleaseDC(hWnd, hDC);
        }
    }
    if ((uDPI == 0) && ::IsWindows10OrGreater()) {
        if (!GetDpiForWindowWrapper(hWnd, uDPI)) {
            uDPI = 0;
        }
    }
    return uDPI;
}

bool GetDpiForSystemWrapper(UINT& dpi)
{
    typedef UINT(WINAPI* GetDpiForSystemPtr)();
    static GetDpiForSystemPtr get_dpi_for_system_func = reinterpret_cast<GetDpiForSystemPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetDpiForSystem"));
    dpi = 96;
    if (get_dpi_for_system_func) {
        dpi = get_dpi_for_system_func();
        return true;
    }
    return false;
}

bool GetDpiForMonitorWrapper(HMONITOR hMonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY)
{
    typedef HRESULT(WINAPI *GetDpiForMonitorPtr)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

    static GetDpiForMonitorPtr get_dpi_for_monitor_func = reinterpret_cast<GetDpiForMonitorPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetDpiForMonitorInternal"));
    if (get_dpi_for_monitor_func) {
        if (get_dpi_for_monitor_func(hMonitor, dpiType, dpiX, dpiY) != S_OK) {
            return true;
        }
    }
    return false;
}

bool GetDpiForWindowWrapper(HWND hwnd, UINT& dpi)
{
    typedef UINT(WINAPI* GetDpiForWindowPtr)(HWND hwnd);
    static GetDpiForWindowPtr get_dpi_for_window_func = reinterpret_cast<GetDpiForWindowPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetDpiForWindow"));
    dpi = 96;
    if (get_dpi_for_window_func) {
        dpi = get_dpi_for_window_func(hwnd);
        return true;
    }
    return false;
}

int GetSystemMetricsForDpiWrapper(int nIndex, UINT dpi)
{
    int nMetrics = 0;
    typedef int(WINAPI* GetSystemMetricsForDpiPtr)(int nIndex, UINT dpi);
    static GetSystemMetricsForDpiPtr get_system_metrics_for_dpi_func = reinterpret_cast<GetSystemMetricsForDpiPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetSystemMetricsForDpi"));
    if (get_system_metrics_for_dpi_func) {
        nMetrics = get_system_metrics_for_dpi_func(nIndex, dpi);
    }
    else {
        nMetrics = ::GetSystemMetrics(nIndex);
    }
    return nMetrics;
}

bool SetProcessDpiAwarenessContextWrapper(PROCESS_DPI_AWARENESS_CONTEXT value)
{
    typedef    BOOL (WINAPI *SetProcessDpiAwarenessContextPtr)(PROCESS_DPI_AWARENESS_CONTEXT value);
    static SetProcessDpiAwarenessContextPtr set_process_dpi_awareness_context_func = reinterpret_cast<SetProcessDpiAwarenessContextPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessContext"));
    bool isOk = false;
    if (set_process_dpi_awareness_context_func) {
        isOk = set_process_dpi_awareness_context_func(value) != FALSE;
    }
    return isOk;
}

bool AreDpiAwarenessContextsEqualWrapper(PROCESS_DPI_AWARENESS_CONTEXT dpiContextA, PROCESS_DPI_AWARENESS_CONTEXT dpiContextB)
{
    typedef BOOL (WINAPI *AreDpiAwarenessContextsEqualPtr)(PROCESS_DPI_AWARENESS_CONTEXT dpiContextA, PROCESS_DPI_AWARENESS_CONTEXT dpiContextB);
    static AreDpiAwarenessContextsEqualPtr are_process_dpi_awareness_context_equal_func = reinterpret_cast<AreDpiAwarenessContextsEqualPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "AreDpiAwarenessContextsEqual"));
    bool isOk = false;
    if (are_process_dpi_awareness_context_equal_func) {
        if (are_process_dpi_awareness_context_equal_func(dpiContextA, dpiContextB)) {
            isOk = true;
        }
    }
    return isOk;
}

bool GetProcessDpiAwarenessContextWrapper(PROCESS_DPI_AWARENESS_CONTEXT& value)
{
    typedef PROCESS_DPI_AWARENESS_CONTEXT(WINAPI *GetDpiAwarenessContextForProcessPtr)(HANDLE hProcess);
    static GetDpiAwarenessContextForProcessPtr get_process_dpi_awareness_context_func = reinterpret_cast<GetDpiAwarenessContextForProcessPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetDpiAwarenessContextForProcess"));
    bool isOk = false;
    if (get_process_dpi_awareness_context_func) {
        value = get_process_dpi_awareness_context_func(nullptr);
        isOk = true;
    }
    return isOk;
}

bool SetProcessDPIAwarenessWrapper(PROCESS_DPI_AWARENESS value)
{
    typedef BOOL(WINAPI *SetProcessDpiAwarenessPtr)(PROCESS_DPI_AWARENESS);
    static SetProcessDpiAwarenessPtr set_process_dpi_awareness_func = reinterpret_cast<SetProcessDpiAwarenessPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessInternal"));
    if (set_process_dpi_awareness_func) {
        if (set_process_dpi_awareness_func(value)) {
            return true;
        }
    }
    return false;
}

bool GetProcessDPIAwarenessWrapper(PROCESS_DPI_AWARENESS& awareness)
{
    typedef BOOL (WINAPI* GetProcessDpiAwarenessPtr)(HANDLE, PROCESS_DPI_AWARENESS*);
    static GetProcessDpiAwarenessPtr get_process_dpi_awareness_func = reinterpret_cast<GetProcessDpiAwarenessPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetProcessDpiAwarenessInternal"));
    if (get_process_dpi_awareness_func) {
        if (get_process_dpi_awareness_func(nullptr, &awareness)) {
            return true;
        }
    }
    return false;
}

bool SetProcessDPIAwareWrapper()
{
    typedef BOOL(WINAPI *SetProcessDPIAwarePtr)(VOID);
    static SetProcessDPIAwarePtr set_process_dpi_aware_func = reinterpret_cast<SetProcessDPIAwarePtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "SetProcessDPIAware"));
    return set_process_dpi_aware_func && set_process_dpi_aware_func();
}

bool IsProcessDPIAwareWrapper(bool& bAware)
{
    typedef BOOL(WINAPI* IsProcessDPIAwarePtr)();
    static IsProcessDPIAwarePtr is_process_dpi_aware_func = reinterpret_cast<IsProcessDPIAwarePtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "IsProcessDPIAware"));
    if (is_process_dpi_aware_func) {
        bAware = is_process_dpi_aware_func() != FALSE;
        return true;
    }
    return false;
}

bool RegisterTouchWindowWrapper(HWND hwnd, ULONG ulFlags)
{
    typedef BOOL(WINAPI *RegisterTouchWindowPtr)(HWND, ULONG);

    static RegisterTouchWindowPtr register_touch_window_func = reinterpret_cast<RegisterTouchWindowPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "RegisterTouchWindow"));

    if (register_touch_window_func) {
        return (TRUE == register_touch_window_func(hwnd, ulFlags));
    }

    return false;
}


bool UnregisterTouchWindowWrapper(HWND hwnd)
{
    typedef BOOL(WINAPI *UnregisterTouchWindowPtr)(HWND);

    static UnregisterTouchWindowPtr unregister_touch_window_func = reinterpret_cast<UnregisterTouchWindowPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "UnregisterTouchWindow"));

    if (unregister_touch_window_func) {
        return (TRUE == unregister_touch_window_func(hwnd));
    }

    return false;
}

bool GetTouchInputInfoWrapper(HTOUCHINPUT hTouchInput, UINT cInputs, PTOUCHINPUT pInputs, int cbSize)
{
    typedef BOOL(WINAPI *GetTouchInputInfoPtr)(HTOUCHINPUT, UINT, PTOUCHINPUT, int);

    static GetTouchInputInfoPtr get_touch_input_info_func = reinterpret_cast<GetTouchInputInfoPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetTouchInputInfo"));
    
    if (get_touch_input_info_func) {
        return (TRUE == get_touch_input_info_func(hTouchInput, cInputs, pInputs, cbSize));
    }

    return false;
}


bool CloseTouchInputHandleWrapper(HTOUCHINPUT hTouchInput)
{
    typedef BOOL(WINAPI *CloseTouchInputHandlePtr)(HTOUCHINPUT);

    static CloseTouchInputHandlePtr close_touch_input_handle_func = reinterpret_cast<CloseTouchInputHandlePtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "CloseTouchInputHandle"));

    if (close_touch_input_handle_func) {
        return (TRUE == close_touch_input_handle_func(hTouchInput));
    }

    return false;
}

bool GetPointerTypeWrapper(UINT32 pointerId, POINTER_INPUT_TYPE *pointerType)
{
    typedef BOOL(WINAPI* GetPointerTypePtr)(UINT32 pointerId, POINTER_INPUT_TYPE *pointerType);
    static GetPointerTypePtr get_pointer_type = reinterpret_cast<GetPointerTypePtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetPointerType"));

    if (get_pointer_type) {
        return (TRUE == get_pointer_type(pointerId, pointerType));
    }

    return false;
}

bool GetPointerInfoWrapper(UINT32 pointerId, POINTER_INFO *pointerInfo)
{
    typedef BOOL(WINAPI* GetPointerInfoPtr)(UINT32 pointerId, POINTER_INFO *pointerInfo);
    static GetPointerInfoPtr get_pointer_info = reinterpret_cast<GetPointerInfoPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetPointerInfo"));

    if (get_pointer_info) {
        return (TRUE == get_pointer_info(pointerId, pointerInfo));
    }

    return false;
}

bool GetPointerTouchInfoWrapper(UINT32 pointerId, POINTER_TOUCH_INFO *touchInfo)
{
    typedef BOOL(WINAPI* GetPointerTouchInfoPtr)(UINT32 pointerId, POINTER_TOUCH_INFO *touchInfo);
    static GetPointerTouchInfoPtr get_pointer_touch_info = reinterpret_cast<GetPointerTouchInfoPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetPointerTouchInfo"));

    if (get_pointer_touch_info) {
        return (TRUE == get_pointer_touch_info(pointerId, touchInfo));
    }

    return false;
}

bool GetPointerPenInfoWrapper(UINT32 pointerId, POINTER_PEN_INFO *penInfo)
{
    typedef BOOL(WINAPI* GetPointerPenInfoPtr)(UINT32 pointerId, POINTER_PEN_INFO *penInfo);
    static GetPointerPenInfoPtr get_pointer_pen_info = reinterpret_cast<GetPointerPenInfoPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "GetPointerPenInfo"));

    if (get_pointer_pen_info) {
        return (TRUE == get_pointer_pen_info(pointerId, penInfo));
    }

    return false;
}

bool EnableMouseInPointerWrapper(BOOL fEnable)
{
    typedef BOOL(WINAPI* EnableMouseInPointerPtr)(BOOL);
    static EnableMouseInPointerPtr enable_mouse_in_pointer = reinterpret_cast<EnableMouseInPointerPtr>(GetProcAddress(GetModuleHandleA("user32.dll"), "EnableMouseInPointer"));

    if (enable_mouse_in_pointer) {
        return (TRUE == enable_mouse_in_pointer(fEnable));
    }

    return false;
}

// 检测「拖动窗口时显示窗口内容」是否开启 - 动态加载注册表API版本
bool IsDragWindowContentsEnabled()
{
    typedef LONG(WINAPI* PFUNC_RegOpenKeyExW)(
        HKEY hKey,
        LPCWSTR lpSubKey,
        DWORD ulOptions,
        REGSAM samDesired,
        PHKEY phkResult
        );

    typedef LONG(WINAPI* PFUNC_RegQueryValueExW)(
        HKEY hKey,
        LPCWSTR lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData
        );

    typedef LONG(WINAPI* PFUNC_RegCloseKey)(
        HKEY hKey
        );

    HMODULE hModAdvapi32 = LoadLibraryW(L"Advapi32.dll");
    if (NULL == hModAdvapi32) {
        return false;
    }

    // 获取注册表API的函数地址
    PFUNC_RegOpenKeyExW pfnRegOpenKeyExW = (PFUNC_RegOpenKeyExW)GetProcAddress(hModAdvapi32, "RegOpenKeyExW");
    PFUNC_RegQueryValueExW pfnRegQueryValueExW = (PFUNC_RegQueryValueExW)GetProcAddress(hModAdvapi32, "RegQueryValueExW");
    PFUNC_RegCloseKey pfnRegCloseKey = (PFUNC_RegCloseKey)GetProcAddress(hModAdvapi32, "RegCloseKey");

    if (NULL == pfnRegOpenKeyExW || NULL == pfnRegQueryValueExW || NULL == pfnRegCloseKey) {
        FreeLibrary(hModAdvapi32);
        return false;
    }

    HKEY hKey = NULL;
    LONG lResult = ERROR_SUCCESS;
    DWORD dwValueType = 0;
    wchar_t szValueBuffer[8] = { 0 };
    DWORD dwValueBuffer = 0;
    DWORD dwBufferSize = 0;

    // 打开注册表项
    lResult = pfnRegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Control Panel\\Desktop",
        0,
        KEY_READ,
        &hKey
    );

    if (lResult != ERROR_SUCCESS) {
        if (hKey) {
            pfnRegCloseKey(hKey);
        }
        FreeLibrary(hModAdvapi32);      // 释放DLL句柄
        return false;
    }

    bool bEnabled = false;
    // 先尝试读取为字符串类型（Win10/11主流场景）
    dwBufferSize = sizeof(szValueBuffer);
    lResult = pfnRegQueryValueExW(
        hKey,
        L"DragFullWindows",
        NULL,
        &dwValueType,
        (LPBYTE)szValueBuffer,
        &dwBufferSize
    );

    if (lResult == ERROR_SUCCESS) {
        if (dwValueType == REG_SZ) {
            bEnabled = (wcscmp(szValueBuffer, L"1") == 0);
        }
        else if (dwValueType == REG_DWORD) {
            dwValueBuffer = *(DWORD*)szValueBuffer;
            bEnabled = (dwValueBuffer == 1);
        }
    }
    else {
        // 字符串读取失败，尝试按DWORD类型读取（兼容Win7）
        dwBufferSize = sizeof(dwValueBuffer);
        lResult = pfnRegQueryValueExW(
            hKey,
            L"DragFullWindows",
            NULL,
            NULL,
            (LPBYTE)&dwValueBuffer,
            &dwBufferSize
        );
        if (lResult == ERROR_SUCCESS) {
            bEnabled = (dwValueBuffer == 1);
        }
    }

    // 关闭注册表句柄
    pfnRegCloseKey(hKey);

    //释放已加载的DLL
    FreeLibrary(hModAdvapi32);
    return bEnabled;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
