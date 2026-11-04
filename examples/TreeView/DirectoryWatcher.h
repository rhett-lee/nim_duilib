#ifndef EXAMPLES_TREEVIEW_DIRECTORY_WATCHER_H_
#define EXAMPLES_TREEVIEW_DIRECTORY_WATCHER_H_

// duilib
#include "duilib/duilib.h"

#include <memory>

/** 目录内容变化监听器
 *
 * 功能：监听某个目录自身内容的变化（文件或子目录的新增、删除、改名），
 *       内容发生变化时通过回调通知调用方，回调在UI线程中执行（已做防抖处理）。
 *
 * 平台支持：
 *  - Windows 平台：使用 ReadDirectoryChangesW（异步 I/O + 独立监听线程）实现；
 *  - 其他平台：SDL 没有提供文件系统变化监听的接口，不做实现，
 *               Start() 直接返回 false，由调用方退化为手动刷新（F5）。
 */
class DirectoryWatcher final
{
public:
    /** 目录内容发生变化时的回调函数（在UI线程中调用）
     */
    typedef ui::StdClosure ChangeCallback;

public:
    DirectoryWatcher();
    ~DirectoryWatcher();

    DirectoryWatcher(const DirectoryWatcher&) = delete;
    DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;

    /** 开始监听指定目录的内容变化
     *  (正在监听同一个目录时，不会重启监听线程，直接返回true)
     * @param [in] directory 需要监听的目录（绝对路径）
     * @param [in] callback 目录内容发生变化时的回调函数（在UI线程中调用），不能为空
     * @return true 表示监听已经启动，false 表示当前平台不支持或者目录无法监听
     */
    bool Start(const ui::FilePath& directory, const ChangeCallback& callback);

    /** 停止监听（会等待监听线程退出后才返回）
     */
    void Stop();

    /** 是否正在监听
     */
    bool IsWatching() const;

private:
    /** 实现细节（避免在头文件中引入平台相关的定义）
     */
    struct Impl;

    /** 实现对象
     */
    std::unique_ptr<Impl> m_pImpl;
};

#endif // EXAMPLES_TREEVIEW_DIRECTORY_WATCHER_H_
