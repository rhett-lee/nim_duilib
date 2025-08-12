#ifndef UI_CORE_CONTROL_DROP_TARGET_IMPL_SDL_H_
#define UI_CORE_CONTROL_DROP_TARGET_IMPL_SDL_H_

#include "duilib/Core/ControlDropTarget.h"

#ifdef DUILIB_BUILD_FOR_SDL

namespace ui 
{
/** 控件的拖放支持接口的实现（SDL）
*/
class ControlDropTargetImpl_SDL : public ControlDropTarget_SDL
{
public:
    explicit ControlDropTargetImpl_SDL(Control* pControl);
    virtual ~ControlDropTargetImpl_SDL();

public:
    /** SDL_EVENT_DROP_BEGIN
    * @param [in] pt 客户区坐标
    * @return 成功返回0，失败返回非0
    */
    virtual int32_t OnDropBegin(const UiPoint& pt) override;

    /** SDL_EVENT_DROP_POSITION
    *@param [in] pt 客户区坐标
    */
    virtual void OnDropPosition(const UiPoint& pt) override;

    /** SDL_EVENT_DROP_COMPLETE + SDL_EVENT_DROP_TEXT, 后续不会再有OnDropLeave了
    *@param [in] textList 文本内容，容器中每个元素调用代表一行文本
    */
    virtual void OnDropTexts(const std::vector<DString>& textList, const UiPoint& pt) override;

    /** SDL_EVENT_DROP_COMPLETE + SDL_EVENT_DROP_FILE, 后续不会再有OnDropLeave了
    *@param [in] source 拖放源
    *@param [in] fileList 文件路径，容器中每个元素调用代表一个文件
    */
    virtual void OnDropFiles(const DString& source, const std::vector<DString>& fileList, const UiPoint& pt) override;

    /** SDL_EVENT_DROP_COMPLETE 或者 其他导致离开的消息
    */
    virtual void OnDropLeave() override;

private:
    /** 关联的控件
    */
    ControlPtr m_pControl;
};

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL

#endif // UI_CORE_CONTROL_DROP_TARGET_IMPL_SDL_H_
