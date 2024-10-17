#ifndef UI_CORE_MESSAGE_LOOP_SDL_H_
#define UI_CORE_MESSAGE_LOOP_SDL_H_

#include "duilib/duilib_defs.h"
#include <functional>
#include <unordered_map>

#if defined(DUILIB_BUILD_FOR_SDL)

union SDL_Event;

namespace ui {

class NativeWindow_SDL;

/** 自定义消息回调函数原型：void FunctionName(uint32_t msgID, WPARAM wParam, LPARAM lParam);
*/
typedef std::function<void(uint32_t msgID, WPARAM wParam, LPARAM lParam)> SDLUserMessageCallback;

/** 主线程的消息循环
*/
class MessageLoop_SDL
{
public:
    MessageLoop_SDL();
    MessageLoop_SDL(const MessageLoop_SDL& r) = delete;
    MessageLoop_SDL& operator = (const MessageLoop_SDL& r) = delete;
    ~MessageLoop_SDL();

public:
    /** 运行消息循环
    */
    int32_t Run();

    /** 运行模态窗口的消息循环，直到窗口退出
    * @param [in] nativeWindow 需要等待退出的窗口
    * @param [in] bCloseByEsc 按ESC键的时候，是否关闭窗口
    * @param [in] bCloseByEnter 按Enter键的时候，是否关闭窗口
    */
    void RunDoModal(NativeWindow_SDL& nativeWindow, bool bCloseByEsc = true, bool bCloseByEnter = false);

    /** 运行一个用户消息循环，直到达到退出条件
    * @param [in] bTerminate 为true表示退出消息循环，为false表示一直运行消息循环，初始值应为false
    */
    void RunUserLoop(bool& bTerminate);

public:
    /** 从消息队列里面移除多余的消息
    * @param [in] msgId 消息ID
    */
    static void RemoveDuplicateMsg(uint32_t msgId);

    /** 向消息队列中发送一个消息
    * @param [in] msgId 消息ID, 该ID必须位于SDL_EVENT_USER与SDL_EVENT_LAST之间
    * @param [in] wParam 消息的第1个参数
    * @param [in] lParam 消息的第2个参数
    */
    static bool PostUserEvent(uint32_t msgId, WPARAM wParam, LPARAM lParam);

    /** 设置自定义消息回调函数
    * @param [in] msgId 消息ID
    * @param [in] callback 回调函数
    */
    static void AddUserMessageCallback(uint32_t msgId, const SDLUserMessageCallback& callback);

    /** 删除自定义消息回调函数
    * @param [in] msgId 消息ID
    */
    static void RemoveUserMessageCallback(uint32_t msgId);

    /** 向队列中放入一个空消息, 返回到等待队列的处理函数
    */
    static void PostNoneEvent();

    /** 初始化SDL
    * @param [in] videoDriverName 显示驱动的名称, 有效值是：
      Windows平台："windows"
      Linux平台："X11" 或者 "wayland" 或者 "wayland,X11" 或者 "X11,wayland"
    */
    static bool CheckInitSDL(const DString& videoDriverName = _T(""));

private:
    /** 处理用户自定义消息
    */
    static void OnUserEvent(const SDL_Event& sdlEvent);

private:
    /** 自定义消息映射
    */
    static std::unordered_map<uint32_t, SDLUserMessageCallback> s_userMsgCallbacks;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL

#endif // UI_CORE_MESSAGE_LOOP_SDL_H_
