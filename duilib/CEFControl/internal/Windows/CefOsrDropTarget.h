#ifndef UI_CEF_CONTROL_OSR_DROP_TARGET_H_
#define UI_CEF_CONTROL_OSR_DROP_TARGET_H_

#include "duilib/Core/ControlDropTarget.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_CEF)

namespace client {
    class DropTargetWin;
}

namespace ui {

/** CEF离屏渲染模式下的拖动目标（把控件的拖放事件转发给 CEF 浏览器）
 */
class DUILIB_API CefOsrDropTarget : public ControlDropTarget_Windows
{
public:
    /** 构造函数
    * @param [in] pDropTargetWin Windows平台的拖放目标实现
    */
    explicit CefOsrDropTarget(const std::shared_ptr<client::DropTargetWin>& pDropTargetWin);
    virtual ~CefOsrDropTarget() override;

public:
    //IDropTarget::DragEnter
    virtual int32_t DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

    //IDropTarget::DragOver
    virtual int32_t DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

    //IDropTarget::DragLeave
    virtual int32_t DragLeave(void) override;

    //IDropTarget::Drop
    virtual int32_t Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

private:
    /** Windows平台下的拖放目标接口实现
    */
    std::shared_ptr<client::DropTargetWin> m_pDropTargetWin;
};

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN/DUILIB_BUILD_FOR_CEF

#endif //UI_CEF_CONTROL_OSR_DROP_TARGET_H_
