#ifndef UI_UTILS_APIWRAPPER_WINDOWS_H_
#define UI_UTILS_APIWRAPPER_WINDOWS_H_

#include "duilib/Utils/Macros_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Core/NativeWindowShadow.h"

namespace ui
{
    /** 获取指定窗口的DPI值
    * @param [in] hWnd 目标窗口的句柄
    * @return 返回窗口的DPI值（96 表示无缩放）；句柄无效或获取失败时返回 0
    */
    UINT GetDpiForWnd(HWND hWnd);

    // DPI适配接口：优先动态加载系统API，系统不支持时返回失败
    /** 获取系统的DPI值（Windows 10 版本 1607 及以上支持）
    * @note 该API在进程启动后，如果修改系统DPI，返回的依旧是旧值
    * @param [out] dpi 返回系统DPI值（API不可用时为默认值 96）
    * @return 返回 true 表示调用成功，否则返回 false
    */
    bool GetDpiForSystemWrapper(UINT& dpi);

    /** 获取指定窗口的DPI值（Windows 10 版本 1607 及以上支持）
    * @note 该API在进程启动后，如果修改系统DPI，返回的依旧是旧值
    * @param [in] hwnd 目标窗口的句柄
    * @param [out] dpi 返回窗口的DPI值（API不可用时为默认值 96）
    * @return 返回 true 表示调用成功，否则返回 false
    */
    bool GetDpiForWindowWrapper(HWND hwnd, UINT& dpi);

    /** 按指定的DPI值获取系统度量值（Windows 10 版本 1607 及以上支持）
    * @param [in] nIndex 系统度量值的索引（如 SM_CXICON、SM_CXSMICON）
    * @param [in] dpi 用于换算的DPI值
    * @return 返回对应的度量值（像素）；API不可用时退化为 GetSystemMetrics 的返回值
    */
    int GetSystemMetricsForDpiWrapper(int nIndex, UINT dpi);

    /** 获取指定显示器的DPI值（Windows 8.1 及以上支持）
    * @param [in] hMonitor 显示器句柄
    * @param [in] dpiType 需要获取的DPI类型（如 MDT_EFFECTIVE_DPI）
    * @param [out] dpiX 返回水平方向的DPI值
    * @param [out] dpiY 返回垂直方向的DPI值
    * @return 返回 true 表示调用成功；失败或系统不支持时返回 false
    */
    bool GetDpiForMonitorWrapper(HMONITOR hMonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

    /** 设置进程的DPI感知上下文（Windows 10 版本 1703 及以上支持）
    * @param [in] value DPI感知上下文，如 PROCESS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
    * @return 返回 true 表示设置成功；失败或系统不支持时返回 false
    */
    bool SetProcessDpiAwarenessContextWrapper(PROCESS_DPI_AWARENESS_CONTEXT value);

    /** 获取当前进程的DPI感知上下文（Windows 10 版本 1703 及以上支持）
    * @param [out] value 返回当前进程的DPI感知上下文
    * @return 返回 true 表示获取成功；失败或系统不支持时返回 false
    */
    bool GetProcessDpiAwarenessContextWrapper(PROCESS_DPI_AWARENESS_CONTEXT& value);

    /** 判断两个DPI感知上下文是否相等（Windows 10 版本 1703 及以上支持）
    * @param [in] dpiContextA 第一个DPI感知上下文
    * @param [in] dpiContextB 第二个DPI感知上下文
    * @return 返回 true 表示两者相等；不相等或系统不支持时返回 false
    */
    bool AreDpiAwarenessContextsEqualWrapper(PROCESS_DPI_AWARENESS_CONTEXT dpiContextA, PROCESS_DPI_AWARENESS_CONTEXT dpiContextB);
    
    /** 设置进程的DPI感知级别（Windows 8.1 及以上支持）
    * @param [in] value DPI感知级别，参见 PROCESS_DPI_AWARENESS
    * @return 返回 true 表示设置成功；失败或系统不支持时返回 false
    */
    bool SetProcessDPIAwarenessWrapper(PROCESS_DPI_AWARENESS value);

    /** 获取进程的DPI感知级别（Windows 8.1 及以上支持）
    * @param [out] awareness 返回当前进程的DPI感知级别
    * @return 返回 true 表示获取成功；失败或系统不支持时返回 false
    */
    bool GetProcessDPIAwarenessWrapper(PROCESS_DPI_AWARENESS& awareness);

    /** 将进程设置为DPI感知（Windows Vista 及以上支持）
    * @return 返回 true 表示设置成功；失败或系统不支持时返回 false
    */
    bool SetProcessDPIAwareWrapper();

    /** 判断当前进程是否为DPI感知（Windows Vista 及以上支持）
    * @param [out] bAware 返回 true 表示进程为DPI感知，false 表示不是
    * @return 返回 true 表示查询成功；失败或系统不支持时返回 false
    */
    bool IsProcessDPIAwareWrapper(bool& bAware);

    // WM_TOUCH 触摸消息处理接口
    /** 注册窗口接收 WM_TOUCH 触摸消息
    * @param [in] hwnd 目标窗口的句柄
    * @param [in] ulFlags 注册标志，参见 TWF_FINETOUCH、TWF_WANTPALM
    * @return 返回 true 表示注册成功，否则返回 false
    */
    bool RegisterTouchWindowWrapper(HWND hwnd, ULONG ulFlags);

    /** 注销窗口对 WM_TOUCH 触摸消息的接收
    * @param [in] hwnd 目标窗口的句柄
    * @return 返回 true 表示注销成功，否则返回 false
    */
    bool UnregisterTouchWindowWrapper(HWND hwnd);

    /** 获取触摸输入的详细信息
    * @param [in] hTouchInput 触摸输入句柄（来自 WM_TOUCH 消息）
    * @param [in] cInputs pInputs 数组的元素个数
    * @param [out] pInputs 接收触摸输入信息的数组
    * @param [in] cbSize TOUCHINPUT 结构体的大小（字节）
    * @return 返回 true 表示获取成功，否则返回 false
    */
    bool GetTouchInputInfoWrapper(HTOUCHINPUT hTouchInput, UINT cInputs, PTOUCHINPUT pInputs, int cbSize);

    /** 关闭触摸输入句柄，释放相关资源
    * @param [in] hTouchInput 触摸输入句柄
    * @return 返回 true 表示关闭成功，否则返回 false
    */
    bool CloseTouchInputHandleWrapper(HTOUCHINPUT hTouchInput); 

    // WM_POINTER 指针输入消息处理接口
    /** 获取指针输入的类型
    * @param [in] pointerId 指针ID（来自 WM_POINTER* 消息）
    * @param [out] pointerType 返回指针输入的类型（如 PT_TOUCH、PT_PEN、PT_MOUSE）
    * @return 返回 true 表示获取成功，否则返回 false
    */
    bool GetPointerTypeWrapper(UINT32 pointerId, POINTER_INPUT_TYPE *pointerType);

    /** 获取指针输入的通用信息
    * @param [in] pointerId 指针ID
    * @param [out] pointerInfo 返回指针输入的信息
    * @return 返回 true 表示获取成功，否则返回 false
    */
    bool GetPointerInfoWrapper(UINT32 pointerId, POINTER_INFO *pointerInfo);

    /** 获取触摸指针输入的详细信息
    * @param [in] pointerId 指针ID
    * @param [out] touchInfo 返回触摸指针的详细信息
    * @return 返回 true 表示获取成功，否则返回 false
    */
    bool GetPointerTouchInfoWrapper(UINT32 pointerId, POINTER_TOUCH_INFO *touchInfo);

    /** 获取画笔指针输入的详细信息
    * @param [in] pointerId 指针ID
    * @param [out] penInfo 返回画笔指针的详细信息
    * @return 返回 true 表示获取成功，否则返回 false
    */
    bool GetPointerPenInfoWrapper(UINT32 pointerId, POINTER_PEN_INFO *penInfo);

    /** 设置是否将鼠标输入作为指针输入处理（Windows 8 及以上支持）
    * @param [in] fEnable TRUE 表示把鼠标当作指针输入，FALSE 表示不做转换
    * @return 返回 true 表示设置成功，否则返回 false
    */
    bool EnableMouseInPointerWrapper(BOOL fEnable);

    /** 检测系统「拖动窗口时显示窗口内容」选项是否开启
    * @return 返回 true 表示开启；该选项关闭或注册表读取失败时返回 false
    */
    bool IsDragWindowContentsEnabled();

    /** 获取当前系统主题是否为深色主题
    * @return 返回 true 表示深色主题，false 表示浅色主题（读取失败时也返回 false）
    */
    bool IsSystemThemeDarkMode();
    
    /** 从图标文件数据，生成两个图标（大图标和小图标）, 图标句柄由外部负责释放
    * @param [in] iconFileData 图标数据，格式为所有支持的图片格式，但最好用ico格式，效果最好
    * @param [in] imageFilePath 图片的路径，用于判断图片类型
    * @param [in] uDpiScaleFactor 当前界面缩放百分比因子（举例：100代表缩放百分比为100%，无缩放；为 0 时按 100 处理）
    * @param [out] hSmallIcon 小图标句柄，图标句柄由外部负责释放；可为 nullptr，表示不需要生成小图标
    * @param [out] hBigIcon 大图标句柄，图标句柄由外部负责释放；可为 nullptr，表示不需要生成大图标
    * @return 返回 true 表示生成成功；生成失败时，已生成的图标会被销毁并将句柄置为 nullptr
    */
    bool CreateIconsFromData(const std::vector<uint8_t>& iconFileData,
                             const DString& imageFilePath,
                             uint32_t uDpiScaleFactor,
                             HICON* hSmallIcon, HICON* hBigIcon);

    /** 判断当前系统是否为 Windows 11 或更高版本
    * @return 返回 true 表示是 Windows 11 及以上版本，否则返回 false
    */
    bool UiIsWindows11OrGreater();

    /** 判断当前系统是否为 Windows 7 或更低版本
    * @return 返回 true 表示是 Windows 7 及更低版本，否则返回 false
    * @note DLL 模式下示例程序直接使用，因此声明为导出接口
    */
    DUILIB_API bool UiIsWindows7OrOlder();

    /** 判断DWM服务是否可用
    * @return 返回 true 表示DWM服务可用，否则返回 false
    */
    bool IsDwmCompositionEnabled();

    /** 设置系统的窗口阴影属性，以支持系统阴影
    * @param [in] hWnd 目标窗口句柄
    * @param [in] nativeShadowType 系统阴影的类型（关闭阴影/圆角/直角等）
    * @return 返回 true 表示设置成功，否则返回 false
    */
    bool ModifyDwmStyle(HWND hWnd, NativeWindowShadowType nativeShadowType);

    /** 获取窗口 DWM 可见边框厚度
    * @param [in] hWnd 目标窗口句柄
    * @param [out] outThickness 输出边框厚度（UINT 类型，像素值；失败时会被置为 0）
    * @return 返回 true 表示获取成功，失败或系统不支持时返回 false
    */
    bool GetDwmVisibleFrameBorderThickness(HWND hWnd, UINT& outThickness);

    /** DWM设置是否支持透明属性（启用/禁用窗口的模糊效果）
    * @param [in] hWnd 目标窗口句柄
    * @param [in] bEnable true 表示启用模糊效果，false 表示禁用
    * @return 返回 true 表示设置成功，失败或系统不支持时返回 false
    */
    bool SetDwmEnableBlurBehindWindow(HWND hWnd, bool bEnable);

    /** 判断操作系统的任务栏是否自动隐藏
    * @return 返回 true 表示任务栏自动隐藏；未自动隐藏或查询失败时返回 false
    */
    bool IsTaskbarAutoHide();

    /** 任务栏在屏幕中的位置
    */
    enum TaskbarPosition
    {
        TASKBAR_BOTTOM = 0,
        TASKBAR_LEFT,
        TASKBAR_RIGHT,
        TASKBAR_TOP
    };

    /** 获取任务栏的位置
    * @return 返回任务栏的位置；查询失败时返回 TASKBAR_BOTTOM
    */
    TaskbarPosition GetTaskbarPosition();
}

#endif //DUILIB_BUILD_FOR_WIN

#endif //UI_UTILS_APIWRAPPER_WINDOWS_H_
