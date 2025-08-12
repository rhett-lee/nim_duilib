#ifndef UI_CORE_CONTROL_DROP_TARGET_H_
#define UI_CORE_CONTROL_DROP_TARGET_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Core/Callback.h"
#include <vector>

namespace ui 
{
class Control;

/** 控件的拖放支持接口（Windows）
*/
class ControlDropTarget_Windows : public virtual SupportWeakCallback
{
public:
    /** IDropTarget::DragEnter
    */
    virtual int32_t DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) = 0;

    /** IDropTarget::DragOver
    */
    virtual int32_t DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) = 0;

    /** IDropTarget::DragLeave
    */
    virtual int32_t DragLeave(void) = 0;

    /** IDropTarget::Drop
    */
    virtual int32_t Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) = 0;
};

/** 控件的拖放支持接口（SDL）
*/
class ControlDropTarget_SDL : public virtual SupportWeakCallback
{
public:
    /** SDL_EVENT_DROP_BEGIN
    * @param [in] pt 客户区坐标
    * @return 成功返回0，失败返回非0
    */
    virtual int32_t OnDropBegin(const UiPoint& pt) = 0;

    /** SDL_EVENT_DROP_POSITION
    *@param [in] pt 客户区坐标
    */
    virtual void OnDropPosition(const UiPoint& pt) = 0;

    /** SDL_EVENT_DROP_COMPLETE + SDL_EVENT_DROP_TEXT, 后续不会再有OnDropLeave了
    *@param [in] textList 文本内容，容器中每个元素调用代表一行文本
    */
    virtual void OnDropTexts(const std::vector<DString>& textList, const UiPoint& pt) = 0;

    /** SDL_EVENT_DROP_COMPLETE + SDL_EVENT_DROP_FILE, 后续不会再有OnDropLeave了
    *@param [in] source 拖放源
    *@param [in] fileList 文件路径，容器中每个元素调用代表一个文件
    */
    virtual void OnDropFiles(const DString& source, const std::vector<DString>& fileList, const UiPoint& pt) = 0;

    /** SDL_EVENT_DROP_COMPLETE 或者 其他导致离开的消息
    */
    virtual void OnDropLeave() = 0;
};

} // namespace ui

#endif // UI_CORE_CONTROL_DROP_TARGET_H_
