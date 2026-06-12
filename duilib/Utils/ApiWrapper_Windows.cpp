#include "ApiWrapper_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/DllManager_Windows.h"
#include "duilib/Render/IRender.h"
#include <VersionHelpers.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <map>

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

    HMODULE hModAdvapi32 = DllManager::Instance().LoadDll(_T("Advapi32.dll"));
    if (NULL == hModAdvapi32) {
        return false;
    }

    // 获取注册表API的函数地址
    PFUNC_RegOpenKeyExW pfnRegOpenKeyExW = (PFUNC_RegOpenKeyExW)GetProcAddress(hModAdvapi32, "RegOpenKeyExW");
    PFUNC_RegQueryValueExW pfnRegQueryValueExW = (PFUNC_RegQueryValueExW)GetProcAddress(hModAdvapi32, "RegQueryValueExW");
    PFUNC_RegCloseKey pfnRegCloseKey = (PFUNC_RegCloseKey)GetProcAddress(hModAdvapi32, "RegCloseKey");

    if (NULL == pfnRegOpenKeyExW || NULL == pfnRegQueryValueExW || NULL == pfnRegCloseKey) {
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
    return bEnabled;
}

bool IsSystemThemeDarkMode()
{
    bool bDarkMode = false; //默认不是Dark模式
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

    HMODULE hModAdvapi32 = DllManager::Instance().LoadDll(_T("Advapi32.dll"));
    if (NULL == hModAdvapi32) {
        return bDarkMode;
    }

    // 获取注册表API的函数地址
    PFUNC_RegOpenKeyExW pfnRegOpenKeyExW = (PFUNC_RegOpenKeyExW)GetProcAddress(hModAdvapi32, "RegOpenKeyExW");
    PFUNC_RegQueryValueExW pfnRegQueryValueExW = (PFUNC_RegQueryValueExW)GetProcAddress(hModAdvapi32, "RegQueryValueExW");
    PFUNC_RegCloseKey pfnRegCloseKey = (PFUNC_RegCloseKey)GetProcAddress(hModAdvapi32, "RegCloseKey");

    if (NULL == pfnRegOpenKeyExW || NULL == pfnRegQueryValueExW || NULL == pfnRegCloseKey) {
        return bDarkMode;
    }

    HKEY hKey = NULL;
    LONG lResult = ERROR_SUCCESS;

    // 打开注册表项
    lResult = pfnRegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0,
        KEY_READ,
        &hKey
    );

    if (lResult != ERROR_SUCCESS) {
        if (hKey) {
            pfnRegCloseKey(hKey);
        }
        return bDarkMode;
    }

    DWORD dwType = REG_DWORD;
    DWORD value = ~0U;
    DWORD length = sizeof(value);

    //读取注册表键值：1=浅色，0=深色
    lResult = pfnRegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, &dwType, (LPBYTE)&value, &length);
    if (lResult == ERROR_SUCCESS) {
        bDarkMode = (value == 0);
    }

    // 关闭注册表句柄
    pfnRegCloseKey(hKey);
    return bDarkMode;
}

// 辅助函数：将宽度和高度转为唯一键（处理0表示256px的情况）
static DWORD GetIconSizeKey(BYTE bWidth, BYTE bHeight)
{
    // 高位存宽度，低位存高度；0转为256
    UINT width = (bWidth == 0) ? 256 : bWidth;
    UINT height = (bHeight == 0) ? 256 : bHeight;
    return (width << 16) | height;
}

/** 从图标数据中，查找指定大小的图标资源所在位置
* @param [in] pIconData ico图标数据起始地址（对应于*.ico文件的数据）
* @param [in] nDataSize 图标数据的长度
* @param [in] targetWidth 目标图标的宽度
* @param [in] targetHeight 目标图标的高度
* @param [out] outResSize 返回图标数据的长度
* @return 返回图标资源数据的起始地址
*/
static const BYTE* ExtractIconResource(const BYTE* pIconData, DWORD nDataSize,
                                       int32_t targetWidth, int32_t targetHeight, DWORD& outResSize)
{
#pragma pack(push, 1)
    typedef struct
    {
        WORD idReserved;   // 保留字段，必须为0
        WORD idType;       // 资源类型：1=图标，2=光标
        WORD idCount;      // 图标/光标数量
    } ICONDIR;

    typedef struct
    {
        BYTE bWidth;       // 图标宽度（0表示256px）
        BYTE bHeight;      // 图标高度（0表示256px）
        BYTE bColorCount;  // 颜色数（0表示>=8bpp）
        BYTE bReserved;    // 保留字段，必须为0
        WORD wPlanes;      // 位面数（图标固定为1）
        WORD wBitCount;    // 每像素位数
        DWORD dwBytesInRes;// 该图标资源的字节大小
        DWORD dwImageOffset;// 该图标资源在文件中的偏移量
    } ICONDIRENTRY;
#pragma pack(pop)

    outResSize = 0;
    if ((pIconData == nullptr) || (nDataSize == 0)) {
        return nullptr;
    }

    // 解析ICO文件头
    const ICONDIR* pIconDir = (const ICONDIR*)pIconData;
    if ((pIconDir->idReserved != 0 || pIconDir->idType != 1 || pIconDir->idCount == 0)) {
        return nullptr;
    }

    // 遍历所有图标项，初步筛选图标数据：相同尺寸的图标，只保留位深(wBitCount)最大的图标
    const ICONDIRENTRY* pFirstEntry = (const ICONDIRENTRY*)(pIconData + sizeof(ICONDIR));
    // 用map分组：key=宽度+高度的组合键，value=该尺寸下的所有图标条目
    std::map<DWORD, std::vector<ICONDIRENTRY>> iconGroups;
    for (UINT i = 0; i < pIconDir->idCount; i++) {
        const ICONDIRENTRY* pEntry = &pFirstEntry[i];
        DWORD sizeKey = GetIconSizeKey(pEntry->bWidth, pEntry->bHeight);
        iconGroups[sizeKey].push_back(*pEntry);
    }
    //遍历每个分组，筛选位深最大的图标，剔除其他图标数据，图标按尺寸由小到大排序
    std::vector<ICONDIRENTRY> allIconList;
    for (auto& group : iconGroups) {
        auto& entries = group.second;
        // 找到该分组中wBitCount最大的条目
        auto maxEntryIt = std::max_element(entries.begin(), entries.end(),
            [](const ICONDIRENTRY& a, const ICONDIRENTRY& b) {
                return a.wBitCount < b.wBitCount;
            });

        // 将最大位深的条目加入结果容器
        if (maxEntryIt != entries.end()) {
            allIconList.push_back(*maxEntryIt);
        }
    }
    if (allIconList.empty()) {
        return nullptr;
    }

    //筛选出匹配度最高的那个图标
    ICONDIRENTRY bestEntry = allIconList.back(); //默认选择尺寸最大的图标
    for (size_t nIndex = 0; nIndex < allIconList.size(); ++nIndex) {
        const ICONDIRENTRY& entry = allIconList[nIndex];
        int entryWidth = (entry.bWidth == 0) ? 256 : entry.bWidth;
        int entryHeight = (entry.bHeight == 0) ? 256 : entry.bHeight;
        bool isSizeQualified = (entryWidth >= targetWidth) && (entryHeight >= targetHeight);
        if (!isSizeQualified) {
            continue;
        }
        //尺寸达标
        if ((entryWidth == targetWidth) && (entryHeight == targetHeight)) {
            //尺寸精确满足需要：直接选择
            bestEntry = allIconList[nIndex];
        }
        else if (nIndex == 0) {
            //首个图标满足需要：直接选择
            bestEntry = allIconList[nIndex];
        }
        else {
            //非首个图标满足需要：比较哪个更合适（缩放时图标失真度更小）
            const ICONDIRENTRY& preEntry = allIconList[nIndex - 1];
            int preEntryWidth = (preEntry.bWidth == 0) ? 256 : preEntry.bWidth;
            int preEntryHeight = (preEntry.bHeight == 0) ? 256 : preEntry.bHeight;
            float wRatio = (float)(targetWidth - preEntryWidth) / (float)preEntryWidth;
            float hRatio = (float)(targetHeight - preEntryHeight) / (float)preEntryHeight;
            float preRatio = std::max(wRatio, hRatio);

            wRatio = (float)(entryWidth - targetWidth) / (float)entryWidth;
            hRatio = (float)(entryHeight - targetHeight) / (float)entryHeight;
            float curRatio = std::max(wRatio, hRatio);
            if (curRatio < preRatio) {
                bestEntry = allIconList[nIndex];            //选择尺寸大的
            }
            else {
                const float minRatio = 0.20f; //设置最小放大比例
                if (preRatio < minRatio) {
                    bestEntry = allIconList[nIndex - 1];    //选择尺寸小的
                }
                else {
                    bestEntry = allIconList[nIndex];        //选择尺寸大的
                }
            }
        }
        break;
    }

    // 校验资源偏移和大小
    if ((bestEntry.dwImageOffset + bestEntry.dwBytesInRes) > nDataSize) {
        return nullptr;
    }
    // 输出结果
    outResSize = bestEntry.dwBytesInRes;
    if (outResSize == 0) {
        return nullptr;
    }
    return pIconData + bestEntry.dwImageOffset;
}

/** 支持ICO格式
*/
static bool CreateIconsFromIcoData(const std::vector<uint8_t>& iconFileData, uint32_t uDpiScaleFactor,
                                   HICON* hSmallIcon, HICON* hBigIcon)
{
    if (iconFileData.empty()) {
        return false;
    }
    if ((hSmallIcon == nullptr) && (hBigIcon == nullptr)) {
        return false;
    }
    if (hSmallIcon != nullptr) {
        *hSmallIcon = nullptr;
    }
    if (hBigIcon != nullptr) {
        *hBigIcon = nullptr;
    }
    //Little Endian Only
    int16_t test = 1;
    bool bLittleEndianHost = (*((char*)&test) == 1);
    ASSERT_UNUSED_VARIABLE(bLittleEndianHost);

    bool bValidIcoFile = false;
    std::vector<uint8_t> fileData = iconFileData;
    fileData.resize(fileData.size() + 1024); //填充空白
    typedef struct tagIconDir {
        uint16_t idReserved;
        uint16_t idType;
        uint16_t idCount;
    } ICONHEADER;
    typedef struct tagIconDirectoryEntry {
        uint8_t  bWidth;
        uint8_t  bHeight;
        uint8_t  bColorCount;
        uint8_t  bReserved;
        uint16_t  wPlanes;
        uint16_t  wBitCount;
        uint32_t dwBytesInRes;
        uint32_t dwImageOffset;
    } ICONDIRENTRY;

    ICONHEADER* icon_header = (ICONHEADER*)fileData.data();
    if ((icon_header->idReserved == 0) && (icon_header->idType == 1)) {
        bValidIcoFile = true;
        for (int32_t c = 0; c < icon_header->idCount; ++c) {
            size_t nDataOffset = sizeof(ICONHEADER) + sizeof(ICONDIRENTRY) * c;
            if (nDataOffset >= fileData.size()) {
                bValidIcoFile = false;
                break;
            }
            ICONDIRENTRY* pIconDir = (ICONDIRENTRY*)((uint8_t*)fileData.data() + nDataOffset);
            if (pIconDir->dwImageOffset >= iconFileData.size()) {
                bValidIcoFile = false;
                break;
            }
            else if ((pIconDir->dwImageOffset + pIconDir->dwBytesInRes) > iconFileData.size()) {
                bValidIcoFile = false;
                break;
            }
        }
    }
    //ASSERT(bValidIcoFile);
    if (!bValidIcoFile) {
        return false;
    }

    if (uDpiScaleFactor == 0) {
        uDpiScaleFactor = 100;
    }
    uint32_t uDpi = DpiManager::MulDiv(uDpiScaleFactor, 96u, 100u);
    struct TWinIconInfo
    {
        BOOL bLargeIcon;
        int32_t cxIcon;
        int32_t cyIcon;
    };
    std::vector<TWinIconInfo> iconInfos;

    //大图标
    if (hBigIcon != nullptr) {
        int32_t cxBestIcon = GetSystemMetricsForDpiWrapper(SM_CXICON, uDpi);
        int32_t cyBestIcon = GetSystemMetricsForDpiWrapper(SM_CYICON, uDpi);
        iconInfos.push_back({ TRUE, cxBestIcon, cyBestIcon });
    }

    //小图标
    if (hSmallIcon != nullptr) {
        int32_t cxBestIcon = GetSystemMetricsForDpiWrapper(SM_CXSMICON, uDpi);
        int32_t cyBestIcon = GetSystemMetricsForDpiWrapper(SM_CYSMICON, uDpi);
        iconInfos.push_back({ FALSE, cxBestIcon, cyBestIcon });
    }

    for (const TWinIconInfo& iconInfo : iconInfos) {
        DWORD nIconDataSize = 0;
        const BYTE* pIconData = ExtractIconResource((const BYTE*)fileData.data(), (DWORD)fileData.size(), iconInfo.cxIcon, iconInfo.cyIcon, nIconDataSize);
        if (pIconData == nullptr) {
            int32_t offset = ::LookupIconIdFromDirectoryEx((PBYTE)fileData.data(), TRUE, iconInfo.cxIcon, iconInfo.cyIcon, LR_DEFAULTCOLOR | LR_SHARED);
            if (offset > 0) {
                pIconData = (PBYTE)fileData.data() + offset;
                nIconDataSize = (DWORD)fileData.size() - (DWORD)offset;
            }
        }
        if (pIconData != nullptr) {
            HICON hIcon = ::CreateIconFromResourceEx((PBYTE)pIconData, nIconDataSize, TRUE, 0x00030000, iconInfo.cxIcon, iconInfo.cyIcon, LR_DEFAULTCOLOR | LR_SHARED);
            ASSERT(hIcon != nullptr);
            if (hIcon != nullptr) {
                if (iconInfo.bLargeIcon) {
                    //大图标
                    ASSERT(hBigIcon != nullptr);
                    if (hBigIcon != nullptr) {
                        *hBigIcon = hIcon;
                    }
                }
                else {
                    //小图标
                    ASSERT(hSmallIcon != nullptr);
                    if (hSmallIcon != nullptr) {
                        *hSmallIcon = hIcon;
                    }
                }
            }
        }
    }
    bool bRet = true;
    if (hSmallIcon != nullptr) {
        if (*hSmallIcon == nullptr) {
            bRet = false;
        }
    }
    if (hBigIcon != nullptr) {
        if (*hBigIcon == nullptr) {
            bRet = false;
        }
    }
    if (!bRet) {
        if (hSmallIcon != nullptr) {
            if (*hSmallIcon != nullptr) {
                ::DestroyIcon(*hSmallIcon);
            }
            *hSmallIcon = nullptr;
        }
        if (hBigIcon != nullptr) {
            if (*hBigIcon != nullptr) {
                ::DestroyIcon(*hBigIcon);
            }
            *hBigIcon = nullptr;
        }
    }
    return bRet;
}

/** 支持所有图片格式
*/
static bool CreateIconsFromImageData(const std::vector<uint8_t>& iconFileData,
                                     const FilePath& imageFilePath,
                                     uint32_t uDpiScaleFactor,                                     
                                     HICON* hSmallIcon, HICON* hBigIcon)
{
    if (iconFileData.empty()) {
        return false;
    }
    if ((hSmallIcon == nullptr) && (hBigIcon == nullptr)) {
        return false;
    }
    if (hSmallIcon != nullptr) {
        *hSmallIcon = nullptr;
    }
    if (hBigIcon != nullptr) {
        *hBigIcon = nullptr;
    }

    if (uDpiScaleFactor == 0) {
        uDpiScaleFactor = 100;
    }
    uint32_t uDpi = DpiManager::MulDiv(uDpiScaleFactor, 96u, 100u);
    struct TWinIconInfo
    {
        BOOL bLargeIcon;
        int32_t cxIcon;
        int32_t cyIcon;
    };
    std::vector<TWinIconInfo> iconInfos;

    //大图标
    if (hBigIcon != nullptr) {
        int32_t cxBestIcon = GetSystemMetricsForDpiWrapper(SM_CXICON, uDpi);
        int32_t cyBestIcon = GetSystemMetricsForDpiWrapper(SM_CYICON, uDpi);
        iconInfos.push_back({ TRUE, cxBestIcon, cyBestIcon });
    }

    //小图标
    if (hSmallIcon != nullptr) {
        int32_t cxBestIcon = GetSystemMetricsForDpiWrapper(SM_CXSMICON, uDpi);
        int32_t cyBestIcon = GetSystemMetricsForDpiWrapper(SM_CYSMICON, uDpi);
        iconInfos.push_back({ FALSE, cxBestIcon, cyBestIcon });
    }

    for (const TWinIconInfo& winIconInfo : iconInfos) {
        //按图像数据加载
        ImageDecoderFactory& imageDecoders = GlobalManager::Instance().ImageDecoders();
        float fImageSizeScale = uDpiScaleFactor / 100.0f;
        ImageDecodeParam decodeParam;
        decodeParam.m_imageFilePath = imageFilePath;
        decodeParam.m_fImageSizeScale = fImageSizeScale;
        decodeParam.m_pFileData = std::make_shared<std::vector<uint8_t>>(iconFileData);
        decodeParam.m_rcMaxDestRectSize = UiSize(winIconInfo.cxIcon, winIconInfo.cyIcon);
        std::shared_ptr<IBitmap> pBitmap = imageDecoders.DecodeImageData(decodeParam);
        if (pBitmap == nullptr) {
            continue;
        }
        int32_t nWidth = (int32_t)pBitmap->GetWidth();
        int32_t nHeight = (int32_t)pBitmap->GetHeight();
        if ((nWidth < 1) || (nHeight < 1)) {
            continue;
        }

        void* pPixelBits = pBitmap->LockPixelBits();
        ASSERT(pPixelBits != nullptr);
        if (pPixelBits == nullptr) {
            continue;
        }

        //创建图标
        BITMAPINFO bmpInfo;
        memset(&bmpInfo, 0, sizeof(BITMAPINFO));
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = nWidth;
        bmpInfo.bmiHeader.biHeight = -nHeight; /* Top-down bitmap */
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 32;
        bmpInfo.bmiHeader.biCompression = BI_RGB;

        HDC hdc = ::GetDC(NULL);
        void* pBits = NULL;
        HBITMAP hBitmap = ::CreateDIBSection(hdc, &bmpInfo, DIB_RGB_COLORS, &pBits, NULL, 0);
        if (hBitmap == nullptr) {
            ::ReleaseDC(NULL, hdc);
            continue;
        }
        memcpy(pBits, pPixelBits, nWidth * nHeight * 4);
        HBITMAP hMask = ::CreateBitmap(nWidth, nHeight, 1, 1, NULL);
        if (hMask == nullptr) {
            ::DeleteObject(hBitmap);
            ::ReleaseDC(NULL, hdc);
            continue;
        }

        HDC hdcMem = ::CreateCompatibleDC(hdc);
        HGDIOBJ oldBitmap = ::SelectObject(hdcMem, hMask);

        for (int y = 0; y < nHeight; y++) {
            for (int x = 0; x < nWidth; x++) {
                BYTE* pixel = (BYTE*)pBits + (y * nWidth + x) * 4;
                BYTE alpha = pixel[3];
                COLORREF maskColor = (alpha == 0) ? RGB(0, 0, 0) : RGB(255, 255, 255);
                ::SetPixel(hdcMem, x, y, maskColor);
            }
        }

        ICONINFO iconInfo;
        iconInfo.fIcon = TRUE;
        iconInfo.xHotspot = 0;
        iconInfo.yHotspot = 0;
        iconInfo.hbmMask = hMask;
        iconInfo.hbmColor = hBitmap;

        HICON hIcon = ::CreateIconIndirect(&iconInfo);
        ASSERT(hIcon != nullptr);
        if (hIcon != nullptr) {
            if (winIconInfo.bLargeIcon) {
                //大图标
                ASSERT(hBigIcon != nullptr);
                if (hBigIcon != nullptr) {
                    *hBigIcon = hIcon;
                }
            }
            else {
                //小图标
                ASSERT(hSmallIcon != nullptr);
                if (hSmallIcon != nullptr) {
                    *hSmallIcon = hIcon;
                }
            }
        }

        ::SelectObject(hdcMem, oldBitmap);
        ::DeleteDC(hdcMem);
        ::DeleteObject(hBitmap);
        ::DeleteObject(hMask);
        ::ReleaseDC(NULL, hdc);
    }

    bool bRet = true;
    if (hSmallIcon != nullptr) {
        if (*hSmallIcon == nullptr) {
            bRet = false;
        }
    }
    if (hBigIcon != nullptr) {
        if (*hBigIcon == nullptr) {
            bRet = false;
        }
    }
    if (!bRet) {
        if (hSmallIcon != nullptr) {
            if (*hSmallIcon != nullptr) {
                ::DestroyIcon(*hSmallIcon);
            }
            *hSmallIcon = nullptr;
        }
        if (hBigIcon != nullptr) {
            if (*hBigIcon != nullptr) {
                ::DestroyIcon(*hBigIcon);
            }
            *hBigIcon = nullptr;
        }
    }
    return bRet;
}

bool CreateIconsFromData(const std::vector<uint8_t>& iconFileData,
                         const DString& imageFilePath,
                         uint32_t uDpiScaleFactor,
                         HICON* hSmallIcon, HICON* hBigIcon)
{
    if (CreateIconsFromIcoData(iconFileData, uDpiScaleFactor, hSmallIcon, hBigIcon)) {
        return true;
    }
    return CreateIconsFromImageData(iconFileData, FilePath(imageFilePath), uDpiScaleFactor, hSmallIcon, hBigIcon);
}

//判断是否为Windows 11的函数
bool UiIsWindows11OrGreater()
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
    DWORDLONG const dwlConditionMask = VerSetConditionMask(
        VerSetConditionMask(
            VerSetConditionMask(
                0, VER_MAJORVERSION, VER_GREATER_EQUAL),
            VER_MINORVERSION, VER_GREATER_EQUAL),
        VER_BUILDNUMBER, VER_GREATER_EQUAL);

    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber = 22000; //需要根据Build版本号区分

    return ::VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}

bool UiIsWindows7OrOlder()
{
    return ::IsWindows7OrGreater() && !::IsWindows8OrGreater();
}

bool IsDwmCompositionEnabled()
{
    HMODULE hDwm = DllManager::Instance().LoadDll(_T("dwmapi.dll"));
    if (hDwm == nullptr) {
        return false;
    }
    BOOL bEnabled = FALSE;
    if (hDwm) {
        typedef HRESULT(WINAPI* LPDWMISCOMPOSITIONENABLED)(BOOL*);
        LPDWMISCOMPOSITIONENABLED pfn = (LPDWMISCOMPOSITIONENABLED)GetProcAddress(hDwm, "DwmIsCompositionEnabled");
        if (pfn) {
            pfn(&bEnabled);
        }
    }
    return (bEnabled == TRUE);
}

// 动态加载并初始化 DWM 样式（以支持系统的窗口阴影）
bool ModifyDwmStyle(HWND hWnd, NativeWindowShadowType nativeShadowType)
{
    if (!::IsWindow(hWnd)) {
        return false;
    }
    HMODULE hDwm = DllManager::Instance().LoadDll(_T("dwmapi.dll"));
    if (hDwm == nullptr) {
        return false;
    }

    // 动态 DWM API 定义
    typedef HRESULT(WINAPI* DWM_SET_WINDOW_ATTRIBUTE)(HWND, DWORD, LPCVOID, DWORD);
    typedef HRESULT(WINAPI* DWM_EXTEND_FRAME_INTO_CLIENT_AREA)(HWND, const MARGINS*);
    //typedef HRESULT(WINAPI* DWM_ENABLE_BLUR_BEHIND_WINDOW)(HWND, const DWM_BLURBEHIND*);

    DWM_SET_WINDOW_ATTRIBUTE DwmSetWindowAttribute = (DWM_SET_WINDOW_ATTRIBUTE)GetProcAddress(hDwm, "DwmSetWindowAttribute");
    DWM_EXTEND_FRAME_INTO_CLIENT_AREA DwmExtendFrameIntoClientArea = (DWM_EXTEND_FRAME_INTO_CLIENT_AREA)GetProcAddress(hDwm, "DwmExtendFrameIntoClientArea");
    //DWM_ENABLE_BLUR_BEHIND_WINDOW DwmEnableBlurBehindWindow = (DWM_ENABLE_BLUR_BEHIND_WINDOW)GetProcAddress(hDwm, "DwmEnableBlurBehindWindow");

    // Win11+：设置圆角
    if (UiIsWindows11OrGreater() && DwmSetWindowAttribute) {
        DWORD corner = DWMWCP_DEFAULT;
        switch (nativeShadowType) {
        case NativeWindowShadowType::kShadowSystemDoNotRound:
            corner = DWMWCP_DONOTROUND;
            break;
        case NativeWindowShadowType::kShadowSystemRound:
            corner = DWMWCP_ROUND;
            break;
        case NativeWindowShadowType::kShadowSystemSmallRound:
            corner = DWMWCP_ROUNDSMALL;
            break;
        default:
            break;
        }
        HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(DWORD));
        ASSERT_UNUSED_VARIABLE(SUCCEEDED(hr));
    }

    //// 启用模糊后景（全版本兼容）
    //if (DwmEnableBlurBehindWindow) {
    //    DWM_BLURBEHIND bb = { 0 };
    //    bb.dwFlags = DWM_BB_ENABLE;
    //    bb.fEnable = TRUE;
    //    DwmEnableBlurBehindWindow(hWnd, &bb);
    //}

    // 扩展框架到整个客户区（全版本兼容, 该函数为关键函数）
    HRESULT hr = E_FAIL;
    if (DwmExtendFrameIntoClientArea) {
        MARGINS margins = { -1, -1, -1, -1 };
        if (nativeShadowType == NativeWindowShadowType::kShadowSystemDisabled) {
            margins = { 0, 0, 0, 0 }; //关闭
        }        
        hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
        ASSERT_UNUSED_VARIABLE(SUCCEEDED(hr));
    }
    return SUCCEEDED(hr);
}

bool GetDwmVisibleFrameBorderThickness(HWND hWnd, UINT& outThickness)
{
    // 初始化输出值
    outThickness = 0;

    // 句柄无效直接返回
    if (!::IsWindow(hWnd)) {
        return false;
    }

    // 动态加载 dwmapi.dll（使用 DllManager，不重复加载）
    HMODULE hDwm = DllManager::Instance().LoadDll(_T("dwmapi.dll"));
    if (hDwm == nullptr) {
        return false;
    }

    // 获取函数地址
    typedef HRESULT(WINAPI* DWM_GET_WINDOW_ATTRIBUTE)(HWND, DWORD, PVOID, DWORD);
    DWM_GET_WINDOW_ATTRIBUTE pDwmGetWindowAttribute = reinterpret_cast<DWM_GET_WINDOW_ATTRIBUTE>(::GetProcAddress(hDwm, "DwmGetWindowAttribute"));
    if (pDwmGetWindowAttribute == nullptr) {
        return false;
    }
    HRESULT hr = pDwmGetWindowAttribute(hWnd, DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &outThickness, sizeof(UINT));
    return SUCCEEDED(hr);
}

bool SetDwmEnableBlurBehindWindow(HWND hWnd, bool bEnable)
{
    // 句柄无效直接返回
    if (!::IsWindow(hWnd)) {
        return false;
    }

    // 动态加载 dwmapi.dll（使用 DllManager，不重复加载）
    HMODULE hDwm = DllManager::Instance().LoadDll(_T("dwmapi.dll"));
    if (hDwm == nullptr) {
        return false;
    }
    typedef HRESULT(WINAPI* DWM_ENABLE_BLUR_BEHIND_WINDOW)(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind);
    DWM_ENABLE_BLUR_BEHIND_WINDOW pDwmEnableBlurBehindWindow = reinterpret_cast<DWM_ENABLE_BLUR_BEHIND_WINDOW>(::GetProcAddress(hDwm, "DwmEnableBlurBehindWindow"));
    if (pDwmEnableBlurBehindWindow == nullptr) {
        return false;
    }

    HRGN rgn = CreateRectRgn(-1, -1, 0, 0);
    DWM_BLURBEHIND bb;
    bb.dwFlags = (DWM_BB_ENABLE | DWM_BB_BLURREGION);
    bb.fEnable = bEnable ? TRUE : FALSE;
    bb.hRgnBlur = rgn;
    bb.fTransitionOnMaximized = FALSE;
    HRESULT hr = pDwmEnableBlurBehindWindow(hWnd, &bb);
    DeleteObject(rgn);
    return SUCCEEDED(hr);
}

bool IsTaskbarAutoHide()
{
    HMODULE hShell32 = DllManager::Instance().LoadDll(_T("shell32.dll"));
    if (!hShell32) {
        return false;
    }

    using FuncSHAppBarMessage = UINT(WINAPI*)(UINT, PAPPBARDATA);
    auto pSHAppBarMessage = (FuncSHAppBarMessage)GetProcAddress(hShell32, "SHAppBarMessage");
    if (!pSHAppBarMessage) {
        return false;
    }
    APPBARDATA abd{};
    abd.cbSize = sizeof(APPBARDATA);
    // 获取任务栏状态
    UINT state = pSHAppBarMessage(ABM_GETSTATE, &abd);
    // ABS_AUTOHIDE：自动隐藏标记
    return (state & ABS_AUTOHIDE) != 0;
}

TaskbarPosition GetTaskbarPosition()
{
    HMODULE hShell32 = DllManager::Instance().LoadDll(_T("shell32.dll"));
    if (!hShell32) {
        return TASKBAR_BOTTOM;
    }

    using FuncSHAppBarMessage = UINT(WINAPI*)(UINT, PAPPBARDATA);
    FuncSHAppBarMessage pSHAppBarMessage = (FuncSHAppBarMessage)GetProcAddress(hShell32, "SHAppBarMessage");
    if (!pSHAppBarMessage) {
        return TASKBAR_BOTTOM;
    }

    // 调用 API 获取任务栏位置
    APPBARDATA abd{};
    abd.cbSize = sizeof(APPBARDATA);
    abd.uEdge = TASKBAR_BOTTOM;
    pSHAppBarMessage(ABM_GETTASKBARPOS, &abd);
    TaskbarPosition pos = TASKBAR_BOTTOM;
    switch (abd.uEdge)
    {
    case ABE_BOTTOM: pos = TASKBAR_BOTTOM; break;
    case ABE_LEFT:   pos = TASKBAR_LEFT;   break;
    case ABE_RIGHT:  pos = TASKBAR_RIGHT;  break;
    case ABE_TOP:    pos = TASKBAR_TOP;   break;
    default: break;
    }
    return pos;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
