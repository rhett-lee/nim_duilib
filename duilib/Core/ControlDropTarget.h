#ifndef UI_CORE_CONTROL_DROP_TARGET_H_
#define UI_CORE_CONTROL_DROP_TARGET_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Core/Callback.h"

namespace ui 
{
class Control;

/** 控件的拖放支持
*/
class ControlDropTarget : public virtual SupportWeakCallback
{
public:
    //IDropTarget::DragEnter
    virtual int32_t DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) = 0;

    //IDropTarget::DragOver
    virtual int32_t DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) = 0;

    //IDropTarget::DragLeave
    virtual int32_t DragLeave(void) = 0;

    //IDropTarget::Drop
    virtual int32_t Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) = 0;
};

} // namespace ui

#endif // UI_CORE_CONTROL_DROP_TARGET_H_
