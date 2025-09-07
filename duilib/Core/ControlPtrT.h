#ifndef UI_CORE_CONTROL_PTR_T_H_
#define UI_CORE_CONTROL_PTR_T_H_

#include "duilib/Core/Callback.h"

namespace ui 
{
/** Control指针有效性保护对象，使用方法和原生指针相同，但可以规避使用野指针的问题
*/
template<typename T>
class UILIB_API ControlPtrT
{
public:
    explicit ControlPtrT(std::nullptr_t):
        m_pControl(nullptr)
    {
    }
    explicit ControlPtrT(T* pControl) :
        m_pControl(pControl)
    {
        if (pControl != nullptr) {
            m_weak_flag = pControl->GetWeakFlag();
        }
    }
    ControlPtrT():
        m_pControl(nullptr)
    {
    }
    
    ControlPtrT(const ControlPtrT& r):
        m_pControl(r.get())
    {
        if (m_pControl != nullptr) {
            m_weak_flag = m_pControl->GetWeakFlag();
        }
    }

    ControlPtrT& operator=(const ControlPtrT& r)
    {
        m_pControl = r.get();
        if (m_pControl != nullptr) {
            m_weak_flag = m_pControl->GetWeakFlag();
        }
        else {
            m_weak_flag.reset();
        }
        return *this;
    }

    ~ControlPtrT() = default;

    /** 赋值运算符
    */
    ControlPtrT& operator = (std::nullptr_t)
    {
        m_pControl = nullptr;
        m_weak_flag.reset();
        return *this;
    }

    ControlPtrT& operator = (T* pControl)
    {
        m_pControl = pControl;
        if (pControl != nullptr) {
            m_weak_flag = pControl->GetWeakFlag();
        }
        else {
            m_weak_flag.reset();
        }
        return *this;
    }
    
public:
    /** 判断控件的原始指针是否失效
    @return 返回true表示已经失效，返回false表示未失效
    */
    bool expired() const
    {
        return m_weak_flag.expired();
    }
    
    /** 获取原始指针
    */
    T* get() const
    {
        if (m_weak_flag.expired()) {
            return nullptr;
        }
        return m_pControl;
    }
    
    /** 使用原始指针
    */
    T* operator->() const
    {
        if (m_weak_flag.expired()) {
            return nullptr;
        }
        return m_pControl;
    }

    /** 清空
    */
    void reset()
    {
        m_weak_flag.reset();
        m_pControl = nullptr;
    }

    /** 比较操作符
    */
    bool operator == (std::nullptr_t) const
    {
        return get() == nullptr;
    }

    bool operator == (T* pControl) const
    {
        return get() == pControl;
    }

    /** 比较操作符
    */
    bool operator == (const ControlPtrT& r) const
    {
        return get() == r.get();
    }

    /** 比较操作符
    */
    bool operator != (std::nullptr_t) const
    {
        return get() != nullptr;
    }

    bool operator != (T* pControl) const
    {
        return get() != pControl;
    }

    /** 比较操作符
    */
    bool operator != (const ControlPtrT& r) const
    {
        return get() != r.get();
    }

    /** 比较操作符(全局)
    */
    friend inline bool operator == (std::nullptr_t, const ControlPtrT<T>& r) {
        return r.get() == nullptr;
    }

    friend inline bool operator == (T* pControl, const ControlPtrT<T>& r) {
        return r.get() == pControl;
    }

    /** 比较操作符(全局)
    */
    friend inline bool operator != (std::nullptr_t, const ControlPtrT<T>& r) {
        return r.get() != nullptr;
    }

    friend inline bool operator != (T* pControl, const ControlPtrT<T>& r) {
        return r.get() != pControl;
    }
    
private:
    /** 控件的原始指针
    */
    T* m_pControl;
    
    /** 生命周期防护
    */
    std::weak_ptr<WeakFlag> m_weak_flag;
};

/** 控件的智能指针
*/
class Control;
typedef ControlPtrT<Control> ControlPtr;

/** 容器的智能指针
*/
class Box;
typedef ControlPtrT<Box> BoxPtr;

class Window;
typedef ControlPtrT<Window> WindowPtr;

} // namespace ui

#endif // UI_CORE_CONTROL_PTR_T_H_
