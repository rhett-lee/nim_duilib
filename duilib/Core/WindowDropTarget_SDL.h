#ifndef UI_CORE_WINDOW_DROP_TARGET_SDL_H_
#define UI_CORE_WINDOW_DROP_TARGET_SDL_H_

#include "duilib/Core/ControlDropTarget.h"

#if defined (DUILIB_BUILD_FOR_SDL)

namespace ui 
{
class NativeWindow_SDL;

/** 窗口的拖放支持
*/
class WindowDropTarget
{
public:
    explicit WindowDropTarget(NativeWindow_SDL* pNativeWindow);
    WindowDropTarget(const WindowDropTarget& r) = delete;
    WindowDropTarget& operator=(const WindowDropTarget& r) = delete;
    virtual ~WindowDropTarget();

public:
    // SDL_EVENT_DROP_BEGIN
    void OnDropBegin();

    /** SDL_EVENT_DROP_POSITION
    *@param [in] pt 客户区坐标
    */
    void OnDropPosition(const UiPoint& pt);

    /** SDL_EVENT_DROP_TEXT
    *@param [in] utf8Text 文本内容，每次调用为一行文本
    */
    void OnDropText(const DStringA& utf8Text);

    /** SDL_EVENT_DROP_FILE
    *@param [in] utf8Source 拖放源
    *@param [in] utf8File 文件路径，每次调用为一个文件
    */
    void OnDropFile(const DStringA& utf8Source, const DStringA& utf8File);
    
    // SDL_EVENT_DROP_COMPLETE
    void OnDropComplete();

protected: 
    /** 找到某个点坐标对应的控件接口
    * @param [in] clientPt 客户区坐标点
    */
    ControlPtrT<ControlDropTarget_SDL> GetControlDropTarget(const UiPoint& clientPt) const;

private:
    /** 当前Hover状态的控件接口
    */
    ControlPtrT<ControlDropTarget_SDL> m_pHoverDropTarget;

    /** 关联的Native窗口实现
    */
    NativeWindow_SDL* m_pNativeWindow;

    /** 拖动操作的客户区坐标
    */
    UiPoint m_dropPt;

    /** 文本数据
    */
    std::vector<DString> m_textList;

    /** 文件数据
    */
    std::vector<DString> m_fileList;

    /** 文件的源
    */
    DString m_fileSource;
};

} // namespace ui

#endif //defined (DUILIB_BUILD_FOR_SDL)

#endif // UI_CORE_WINDOW_DROP_TARGET_SDL_H_
