#ifndef UI_CEF_CONTROL_CEF_DRAG_DROP_WINDOWS_H_
#define UI_CEF_CONTROL_CEF_DRAG_DROP_WINDOWS_H_

#include "duilib/duilib_config.h"
#include <memory>

#ifdef DUILIB_BUILD_FOR_WIN

//Windows平台的DropTarget实现
namespace client {
    class DropTargetWin;
}

namespace ui
{
/** CEF控件的拖拽操作支持（仅在离屏渲染模式时使用）
 */
class CefDragDrop
{
public:
    CefDragDrop();
    CefDragDrop(const CefDragDrop&) = delete;
    CefDragDrop& operator=(const CefDragDrop&) = delete;

    /** 单例对象
    */
    static CefDragDrop& GetInstance();

    /** 清理
    */
    void Clear();

private:
    ~CefDragDrop();

public:
    /** 获取某个窗口对应的DropTarget，用于浏览器控件的拖动功能
    */
    std::shared_ptr<client::DropTargetWin> GetDropTarget(HWND hwnd);

private:
    class TImpl;
    TImpl* m_pImpl;
};
}

#endif //DUILIB_BUILD_FOR_WIN

#endif //UI_CEF_CONTROL_CEF_DRAG_DROP_WINDOWS_H_
