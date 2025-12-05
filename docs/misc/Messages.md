## Window消息注解
| 消息          | 参数说明     | 资料     |
| :---          | :---     | :---     |
| WM_SIZE       | wParam（具体值）, lParam（新的宽度和高度）          | https://learn.microsoft.com/zh-cn/windows/win32/winmsg/wm-size|
| WM_MOVE       | lParam（窗口客户端区域左上角的 x 坐标和 y 坐标）    |  https://learn.microsoft.com/zh-cn/windows/win32/winmsg/wm-move   |
| WM_PAINT      | 无参数    |https://learn.microsoft.com/zh-cn/windows/win32/gdi/wm-paint|
| WM_SETFOCUS   | wParam 已失去键盘焦点的窗口的句柄 | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-setfocus|
| WM_KILLFOCUS  | wParam 接收键盘焦点的窗口的句柄 | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-killfocus|
| WM_CHAR       | wParam 键的字符代码，lParam 扫描代码、扩展键标志等  | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-char |
| WM_KEYDOWN    | wParam virtual-key code, lParam 扫描代码、扩展键标志等  | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-keydown|
| WM_KEYUP      | 同WM_KEYDOWN | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-keyup|
| WM_SYSKEYDOWN | 同WM_KEYDOWN | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-syskeydown|
| WM_SYSKEYUP   | 同WM_KEYDOWN | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-syskeyup|
| WM_HOTKEY     | wParam，lParam 三个值  |https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-hotkey|
| WM_IME_STARTCOMPOSITION |  无参数   |  https://learn.microsoft.com/zh-cn/windows/win32/intl/wm-ime-startcomposition   |
| WM_IME_ENDCOMPOSITION   |  无参数   |  https://learn.microsoft.com/zh-cn/windows/win32/intl/wm-ime-endcomposition   |
| WM_SETCURSOR  | wParam 窗口句柄，lParam 触发事件等   | https://learn.microsoft.com/zh-cn/windows/win32/menurc/wm-setcursor    |
| -WM_SYSCOMMAND | wParam 请求的系统命令的类型 | https://learn.microsoft.com/zh-cn/windows/win32/menurc/wm-syscommand    |
| WM_CONTEXTMENU| wParam 窗口句柄，lParam光标坐标x,y | https://learn.microsoft.com/zh-cn/windows/win32/menurc/wm-contextmenu?redirectedfrom=MSDN|
| WM_MOUSELEAVE | 无参数 | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-mouseleave    |
| WM_MOUSEHOVER | wParam 指示各种虚拟键是否已按下，lParam光标坐标x,y  | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-mousehover    |
| WM_MOUSEMOVE  | wParam 指示各种虚拟键是否已按下，lParam光标坐标x,y  | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-mousemove    |
| WM_MOUSEWHEEL | wParam WHEEL_DELTA + 虚拟键是否已按下，lParam光标坐标x,y| https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-mousewheel    |
| WM_LBUTTONDOWN| wParam 指示各种虚拟键是否已按下，lParam光标坐标x,y  |https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-lbuttondown     |
| WM_LBUTTONUP  | wParam 指示各种虚拟键是否已按下，lParam光标坐标x,y  | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-lbuttonup    |
| WM_LBUTTONDBLCLK |  相同   |https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-lbuttondblclk |
| WM_RBUTTONDOWN   |  相同   |https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-rbuttondown     |
| WM_RBUTTONUP     |  相同   |https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-rbuttonup     |
| WM_RBUTTONDBLCLK |  相同   |https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-rbuttondblclk    |
| WM_CAPTURECHANGED|  消息发送到丢失鼠标捕获的窗口，lParam 获取鼠标捕获的窗口的句柄   | https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-capturechanged |
| WM_CLOSE    |     |     |
|     |     |     |
| WM_TASKBARBUTTONCREATED |  示例使用，无参数   |     |
| WM_SHOWWINDOW| wParam 指示是否显示窗口。 如果 wParam 为 TRUE，则显示窗口。 如果 wParam 为 FALSE，则表示窗口处于隐藏状态。| https://learn.microsoft.com/zh-cn/windows/win32/winmsg/wm-showwindow?redirectedfrom=MSDN
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
|     |     |     |
