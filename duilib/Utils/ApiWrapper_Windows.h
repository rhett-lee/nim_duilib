#ifndef UI_UTILS_APIWRAPPER_WINDOWS_H_
#define UI_UTILS_APIWRAPPER_WINDOWS_H_

#include "duilib/Utils/Macros_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Core/NativeWindowShadow.h"

namespace ui
{
    //获取指定窗口的DPI值，如果失败则返回0
    UINT GetDpiForWnd(HWND hWnd);

    // DPI适配
    //Windows 10版本 1607(该API在进程启动后，如果修改系统DPI，该API返回的依旧是旧值)
    bool GetDpiForSystemWrapper(UINT& dpi);
    //Windows 10版本 1607(该API在进程启动后，如果修改系统DPI，该API返回的依旧是旧值)
    bool GetDpiForWindowWrapper(HWND hwnd, UINT& dpi);
    //Windows 10版本 1607
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

    // 检测「拖动窗口时显示窗口内容」是否开启
    bool IsDragWindowContentsEnabled();

    //获取当前系统主题是否为深色主题：返回true表示深色主题，返回false表示为浅色主题
    bool IsSystemThemeDarkMode();
    
    /** 从图标文件数据，生成两个图标（大图标和小图标）, 图标句柄由外部负责释放
    * @param [in] iconFileData 图标数据，格式为所有支持的图片格式，但最好用ico格式，效果最好
    * @param [in] imageFilePath 图片的路径，用于判断图片类型
    * @param [in] uDpiScaleFactor 当前界面缩放百分比因子（举例：100代表缩放百分比为100%，无缩放）
    * @param [out] hSmallIcon 小图标句柄，图标句柄由外部负责释放
    * @param [out] hBigIcon 小图标句柄，图标句柄由外部负责释放
    */
    bool CreateIconsFromData(const std::vector<uint8_t>& iconFileData,
                             const DString& imageFilePath,
                             uint32_t uDpiScaleFactor,
                             HICON* hSmallIcon, HICON* hBigIcon);

    //判断是否为Windows 11的函数
    bool UiIsWindows11OrGreater();

    //判断是否为Windows 7的函数
    bool UiIsWindows7OrOlder();

    /** 判断DWM服务是否可用
    */
    bool IsDwmCompositionEnabled();

    /** 设置系统的窗口阴影属性，以支持系统阴影
    */
    bool ModifyDwmStyle(HWND hWnd, NativeWindowShadowType nativeShadowType);

    /* 获取窗口 DWM 可见边框厚度
     * @param hWnd 目标窗口句柄
     * @param outThickness 输出边框厚度（UINT 类型，像素值）
     * @return 成功返回 true，失败返回 false
     */
    bool GetDwmVisibleFrameBorderThickness(HWND hWnd, UINT& outThickness);

    /** DWM设置是是否支持透明属性
    */
    bool SetDwmEnableBlurBehindWindow(HWND hWnd, bool bEnable);

    /** 判断操作系统的任务栏是否自动隐藏
    */
    bool IsTaskbarAutoHide();

    // 任务栏位置
    enum TaskbarPosition
    {
        TASKBAR_BOTTOM = 0,
        TASKBAR_LEFT,
        TASKBAR_RIGHT,
        TASKBAR_TOP
    };
    /** 获取任务栏的位置
    */
    TaskbarPosition GetTaskbarPosition();
}

#endif //DUILIB_BUILD_FOR_WIN

#endif //UI_UTILS_APIWRAPPER_WINDOWS_H_
