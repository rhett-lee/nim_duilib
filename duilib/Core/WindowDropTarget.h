#ifndef UI_CORE_WINDOW_DROP_TARGET_H_
#define UI_CORE_WINDOW_DROP_TARGET_H_

#pragma once

#include "duilib/Core/UiTypes.h"

namespace ui 
{

class ControlDropTarget;
class Window;

/** 窗口的拖放支持
*/
class WindowDropTarget
{
public:
	WindowDropTarget();
	WindowDropTarget(const WindowDropTarget& r) = delete;
	WindowDropTarget& operator=(const WindowDropTarget& r) = delete;
	~WindowDropTarget();

public:
	/** 设置容器所属窗口
	 * @param [in] pWindow 窗口指针
	 */
	void SetWindow(Window* pWindow);

	/** 注册一个拖放接口
	*/
	bool RegisterDragDrop(ControlDropTarget* pDropTarget);

	/** 注销一个拖放接口
	*/
	bool UnregisterDragDrop(ControlDropTarget* pDropTarget);

	/** 清理拖放操作的资源, 注销窗口的拖放操作接口
	*/
	void Clear();

private:
	/** 窗口指针
	*/
	Window* m_pWindow;

	/** 注册的控件列表
	*/
	std::vector<ControlDropTarget*> m_dropTargets;

	/** DropTarget 的实现接口
	*/
	friend class WindowDropTargetImpl;
	WindowDropTargetImpl* m_pDropTargetImpl;
};

} // namespace ui

#endif // UI_CORE_WINDOW_DROP_TARGET_H_
