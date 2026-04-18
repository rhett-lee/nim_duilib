#ifndef UI_UTILS_TRAY_ICON_H_
#define UI_UTILS_TRAY_ICON_H_

#include "duilib/duilib_defs.h"
#include "duilib/Core/UiTypes.h"
#include <functional>
#include <memory>

namespace ui
{

class Window;

/** 托盘图标消息类型
*/
enum class TrayIconMessageType
{
    kLeftClick,        // 左键单击
    kLeftDoubleClick,  // 左键双击      (非Windows平台，使用SDL时，无此消息)
    kRightClick,       // 右键单击
    kRightDoubleClick, // 右键双击      (非Windows平台，使用SDL时，无此消息)
    kMiddleClick,      // 中键单击
    kMouseMove,        // 鼠标移动      (非Windows平台，使用SDL时，无此消息)
    kShowBalloon,      // 气泡提示被点击 (非Windows平台，使用SDL时，无此消息)
};

/** 托盘图标消息回调函数
 * @param [in] msgType 消息类型
 * @param [in] x 鼠标X坐标（屏幕坐标）
 * @param [in] y 鼠标Y坐标（屏幕坐标）
 */
typedef std::function<void(TrayIconMessageType msgType, int32_t x, int32_t y)> TrayIconMessageCallback;

/** 托盘图标功能
*/
class DUILIB_API TrayIcon
{
protected:
    TrayIcon();
    TrayIcon(const TrayIcon&) = delete;
    TrayIcon& operator=(const TrayIcon&) = delete;

public:
    virtual ~TrayIcon();

    /** 创建托盘图标实例
    * @param [in] pWindow 关联的窗口指针
    * @param [in] iconFilePath 图标文件路径（支持*.ico格式，其他格式也支持，但推荐ICO格式）
    * @param [in] tooltip 托盘提示文本
    * @return 托盘图标实例指针，创建失败返回nullptr
    */
    static std::unique_ptr<TrayIcon> Create(const Window* pWindow, const DString& iconFilePath, const DString& tooltip = _T(""));

public:
    /** 设置托盘图标
    * @param [in] pWindow 关联的窗口指针(用于查找图片资源)
    * @param [in] iconFilePath 图标文件路径（支持*.ico格式，其他格式也支持，但推荐ICO格式）
    * @return 设置成功返回true，失败返回false
    */
    virtual bool SetIcon(const Window* pWindow, const DString& iconFilePath) = 0;

    /** 设置托盘提示文本
    * @param [in] tooltip 提示文本
    * @return 设置成功返回true，失败返回false
    */
    virtual bool SetTooltip(const DString& tooltip) = 0;

    /** 显示气泡提示
    * @param [in] title 气泡标题
    * @param [in] content 气泡内容
    * @param [in] timeoutMs 显示超时时间（毫秒），默认3000ms
    * @return 显示成功返回true，失败返回false
    */
    virtual bool ShowBalloon(const DString& title, const DString& content, uint32_t timeoutMs = 3000) = 0;

    /** 隐藏托盘图标
    * @return 隐藏成功返回true，失败返回false
    */
    virtual bool Hide() = 0;

    /** 显示托盘图标（如果之前隐藏）
    * @return 显示成功返回true，失败返回false
    */
    virtual bool Show() = 0;

    /** 托盘图标是否处于显示状态
    * @return 显示返回true，否则返回false
    */
    virtual bool IsTrayVisible() const = 0;

    /** 删除托盘图标
    * @return 删除成功返回true，失败返回false
    */
    virtual bool Remove() = 0;

    /** 获取内部实现的托盘句柄
    *@return SDL实现时，返回的是SDL_Tray*，Windows SDK实现时，返回的是托盘窗口句柄HWND 
    */
    virtual void* GetTrayHandle() const = 0;

    /** 设置消息回调函数
    * @param [in] callback 回调函数
    */
    void SetMessageCallback(TrayIconMessageCallback callback);

protected:
    /** 触发消息回调
    */
    void NotifyMessage(TrayIconMessageType msgType, int32_t x, int32_t y);

private:
    /** 消息回调函数
    */
    TrayIconMessageCallback m_messageCallback;
};

} //namespace ui

#endif // UI_UTILS_TRAY_ICON_H_
