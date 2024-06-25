#ifndef UI_CORE_THREAD_MESSAGE_H_
#define UI_CORE_THREAD_MESSAGE_H_

#include "duilib/Core/Callback.h"

namespace ui {

/** 消息回调函数原型：void FunctionName(uint32_t msgID, WPARAM wParam, LPARAM lParam);
*/
typedef std::function<void(uint32_t msgID, WPARAM wParam, LPARAM lParam)> ThreadMessageCallback;

/** 与主线程通信的辅助类
*/
class ThreadMessage
{
public:
    ThreadMessage();
    ThreadMessage(const ThreadMessage& r) = delete;
    ThreadMessage& operator = (const ThreadMessage& r) = delete;
    ~ThreadMessage();

public:
    /** 初始化
    * @param [in] platformData 平台相关数据（可选参数，如不填写则使用默认值：nullptr）
    * Windows平台：是资源所在模块句柄（HMODULE），如果为nullptr，则使用所在exe的句柄（可选参数）
    */
    void Initialize(void* platformData);

    /** 设置消息回调函数
    * @param [in] msgId 消息ID
    * @param [in] callback 回调函数
    */
    void SetMessageCallback(uint32_t msgId, const ThreadMessageCallback& callback);

    /** 发送一个消息
    * @param [in] msgId 消息ID
    * @param [in] wParam 消息的第1个参数
    * @param [in] lParam 消息的第2个参数
    */
    bool PostMsg(uint32_t msgId, WPARAM wParam, LPARAM lParam);

    /** 从消息队列里面移除多余的消息
    * @param [in] msgId 消息ID
    */
    void RemoveDuplicateMsg(uint32_t msgId);

    /** 清理资源
    */
    void Clear();

private:
    /** 消息窗口函数
    */
    void OnUserMessage(uint32_t msgId, WPARAM wParam, LPARAM lParam);

private:
    /** 内部实现
    */
    class TImpl;
    TImpl* m_impl;
};

} // namespace ui

#endif // UI_CORE_THREAD_MESSAGE_H_
