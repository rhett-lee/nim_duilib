#ifndef UI_CORE_WINDOW_CREATE_PARAM_H_
#define UI_CORE_WINDOW_CREATE_PARAM_H_

#include "duilib/duilib_defs.h"
#include <string>

namespace ui {

/** 窗口类的风格
*   Windows平台可参考：https://learn.microsoft.com/zh-cn/windows/win32/winmsg/window-class-styles
*/
enum WindowClassStyle : uint32_t
{
    kCS_VREDRAW = 0x0001,   //如果移动或大小调整更改了工作区的高度，则重新绘制整个窗口
    kCS_HREDRAW = 0x0002,   //如果移动或大小调整更改了工作区的宽度，则重绘整个窗口
    kCS_DBLCLKS = 0x0008    //当用户在光标位于属于 类的窗口中时双击鼠标时，将双击消息发送到窗口过程
};

/** 窗口风格
*   Windows平台可参考：https://learn.microsoft.com/zh-cn/windows/win32/winmsg/window-styles
*/
enum WindowStyle : uint32_t
{
    kWS_OVERLAPPED      = 0x00000000L, //窗口是重叠的窗口, 重叠的窗口带有标题栏和边框
    kWS_POPUP           = 0x80000000L, //窗口是弹出窗口
    kWS_VISIBLE         = 0x10000000L, //该窗口最初是可见的
    kWS_DISABLED        = 0x08000000L, //窗口最初处于禁用状态, 已禁用的窗口无法接收用户的输入
    kWS_CAPTION         = 0x00C00000L, /* kWS_BORDER | kWS_DLGFRAME */ //窗口具有标题栏，(包含 kWS_BORDER 样式) 。
    kWS_BORDER          = 0x00800000L, //窗口具有细线边框
    kWS_DLGFRAME        = 0x00400000L, //窗口具有通常与对话框一起使用的样式的边框。
    kWS_THICKFRAME      = 0x00040000L, //窗口具有调整大小边框
    kWS_MINIMIZEBOX     = 0x00020000L, //窗口有一个“最大化”按钮。必须指定 WS_SYSMENU 样式。
    kWS_MAXIMIZEBOX     = 0x00010000L, //窗口有一个最小化按钮。必须指定 WS_SYSMENU 样式。
    kWS_SYSMENU	        = 0x00080000L, //窗口的标题栏上有一个窗口菜单。 还必须指定 WS_CAPTION 样式。

    //重叠窗口，窗口具有标题栏、调整边框大小、窗口菜单以及最小化和最大化按钮。
    kWS_OVERLAPPEDWINDOW    = (kWS_OVERLAPPED |  kWS_CAPTION |  kWS_SYSMENU |  kWS_THICKFRAME |  kWS_MINIMIZEBOX | kWS_MAXIMIZEBOX),
    //弹出窗口
    kWS_POPUPWINDOW         = (kWS_POPUP | kWS_BORDER | kWS_SYSMENU)
};

/** 窗口扩展风格
*   Windows平台可参考：https://docs.microsoft.com/en-us/windows/desktop/winmsg/window-class-styles
*/
enum WindowExStyle : uint32_t
{
    kWS_EX_TOPMOST      = 0x00000008L, //窗口应放置在所有非最顶部窗口的上方，并且应保持在窗口上方，即使窗口已停用也是如此。 
    kWS_EX_ACCEPTFILES  = 0x00000010L, //窗口接受拖放文件
    kWS_EX_TRANSPARENT  = 0x00000020L, //该窗口显示为透明
    kWS_EX_TOOLWINDOW   = 0x00000080L, //该窗口旨在用作浮动工具栏。 工具窗口具有短于普通标题栏的标题栏和使用较小的字体绘制的窗口标题。 工具窗口不会显示在任务栏中，也不会显示在用户按 Alt+TAB 时显示的对话框中。 如果工具窗口具有系统菜单，则其图标不会显示在标题栏上。 
    kWS_EX_LAYERED      = 0x00080000L, //该窗口是一个分层窗口
    kWS_EX_NOACTIVATE   = 0x08000000L  //用户单击时，使用此样式创建的顶级窗口不会成为前台窗口。 当用户最小化或关闭前台窗口时，系统不会将此窗口带到前台。
};

/** 窗口的位置和大小默认值标志
*/
enum WindowDefaultSize
{
    kCW_USEDEFAULT  = ((int32_t)0x80000000)
};

/** 创建窗口所需的参数
*/
class WindowCreateParam
{
public:
    /** 默认构造函数
    */
    WindowCreateParam();

    /** 提供窗口标题的构造函数
    * @param [in] windowTitle 窗口标题
    * @param [in] windowId 窗口ID，如果为空，内部会生成一个窗口ID
    */
    explicit WindowCreateParam(const DString& windowTitle, const DString& windowId = _T(""));

    /** 提供窗口标题, 窗口居中的构造函数
    * @param [in] windowTitle 窗口标题
    * @param [in] bCenterWindow 窗口初始位置居中显示
    * @param [in] windowId 窗口ID，如果为空，内部会生成一个窗口ID
    */
    WindowCreateParam(const DString& windowTitle, bool bCenterWindow, const DString& windowId = _T(""));

public:
    /** 平台相关数据（可选参数，如不填写则使用默认值：nullptr）
    * Windows平台：是资源所在模块句柄（HMODULE），如果为nullptr，则使用所在exe的句柄（可选参数）
    */
    void* m_platformData;

    /** 窗口类名（可选参数，如不填写则使用默认值）
    */
    DString m_className;

    /** 窗口类的风格（可选参数，如不填写则使用默认值）, 参考上方定义：enum WindowClassStyle
    */
    uint32_t m_dwClassStyle;

public:
    /** 窗口风格（可选参数，如不填写则使用默认值）, 参考上方定义：enum WindowStyle
    */
    uint32_t m_dwStyle;

    /** 窗口扩展风格（可选参数，如不填写则使用默认值）, 参考上方定义：enum WindowExStyle
    */
    uint32_t m_dwExStyle;

    /** 窗口的标题（可选参数，默认为空）
    */
    DString m_windowTitle;

    /** 窗口ID（可以为空，如果为空的话，内部会自动生成一个唯一ID）
    */
    DString m_windowId;

public:
    /** 窗口的左上角X坐标(如果不设置，则使用默认值)
    */
    int32_t m_nX;

    /** 窗口的左上角Y坐标(如果不设置，则使用默认值)
    */
    int32_t m_nY;

    /** 窗口的宽度(如果不设置，则使用默认值)
    */
    int32_t m_nWidth;

    /** 窗口的宽度(如果不设置，则使用默认值)
    */
    int32_t m_nHeight;

    /** 初始窗口是否居中显示(默认为false)
    */
    bool m_bCenterWindow;
};

} // namespace ui

#endif // UI_CORE_WINDOW_CREATE_PARAM_H_

