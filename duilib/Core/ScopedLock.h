#ifndef UI_CORE_SCOPED_LOCK_H_
#define UI_CORE_SCOPED_LOCK_H_

#include <mutex>

namespace ui 
{
/** 自动解锁类的封装
*/
class ScopedLock
{
public:
    // 构造函数获取锁
    explicit ScopedLock(std::mutex& mutex)
        : m_mutex(mutex), m_locked(true)
    {
        m_mutex.lock();
    }

    // 析构函数自动释放锁
    ~ScopedLock()
    {
        Unlock();
    }

    // 手动解锁方法
    void Unlock()
    {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

    // 禁止拷贝
    ScopedLock(const ScopedLock&) = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;

private:
    std::mutex& m_mutex;
    bool m_locked;
};

} // namespace ui

#endif // UI_CORE_SCOPED_LOCK_H_
