#ifndef UI_CORE_WINDOW_MANAGER_H_
#define UI_CORE_WINDOW_MANAGER_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Core/ControlPtrT.h"

namespace ui 
{
class Window;
class WindowBase;

/** 窗口管理器，用于管理所有窗口的生命周期
 */
class UILIB_API WindowManager
{
public:
    WindowManager();
    ~WindowManager();
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator = (const WindowManager&) = delete;

public:
    /** 添加一个窗口接口（主要用于换肤、切换语言之后的重绘、资源同步等操作）
    * @param [in] pWindow 窗口的接口
    */
    void AddWindow(Window* pWindow);

    /** 移除一个窗口
    * @param [in] pWindow 窗口的接口
    */
    void RemoveWindow(Window* pWindow);

    /** 判断当前是否含有窗口
    * @param [in] pWindow 窗口的接口
    */
    bool HasWindow(Window* pWindow) const;
    bool HasWindowBase(WindowBase* pWindowBase) const;

    /** 获取所有窗口列表
    */
    std::vector<WindowPtr> GetAllWindowList() const;

    /** 获取指定窗口类下的所有窗口
    * @param [in] windowClassName 创建窗口时传入的窗口类名
    */
    std::vector<WindowPtr> GetAllWindowList(const DString& windowClassName) const;

    /** 获取指定窗口ID对应的窗口
    * @param [in] windowId 窗口ID, 理论上该Id是唯一的
    * @return 返回该窗口ID对应的窗口，如果存在多个，则返回第一个匹配的窗口
    */
    WindowPtr GetWindowById(const DString& windowId) const;

    /** 清空
    */
    void Clear();

private:
    /** 窗口列表
    */
    std::vector<WindowPtr> m_windowList;
};

} //namespace ui 

#endif //UI_CORE_WINDOW_MANAGER_H_
