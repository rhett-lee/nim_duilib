#ifndef UI_CORE_CALLBACK_H_
#define UI_CORE_CALLBACK_H_

#include "duilib/duilib_defs.h"
#include <memory>
#include <functional>

namespace ui
{

/** 回调函数的原型
*/
typedef std::function<void(void)> StdClosure;

/** weak_ptr智能指针的类型标志
*/
class UILIB_API WeakFlag
{
};

/** 基于weak_ptr的回调类
*/
template<typename T>
class UILIB_API WeakCallback
{
public:
    WeakCallback(const std::weak_ptr<WeakFlag>& weak_flag, const T& t) :
        m_weak_flag(weak_flag),
        m_func(t)
    {
    }

    WeakCallback(const std::weak_ptr<WeakFlag>& weak_flag, T&& t) :
        m_weak_flag(weak_flag),
        m_func(std::move(t))
    {

    }

    template<class WeakType>
    explicit WeakCallback(const WeakType& weak_callback) :
        m_weak_flag(weak_callback.GetFlag()),
        m_func(weak_callback.GetFunc())
    {
    }

    template<class... Args>
    auto operator ()(Args && ... args) const
    {
        if (!m_weak_flag.expired()) {
            return m_func(std::forward<Args>(args)...);
        }
        else {
            return decltype(m_func(std::forward<Args>(args)...))();
        }
    }

    bool Expired() const
    {
        return m_weak_flag.expired();
    }

    const std::weak_ptr<WeakFlag>& GetFlag() const
    {
        return m_weak_flag;
    }

    const T& GetFunc() const
    {
        return m_func;
    }

private:
    std::weak_ptr<WeakFlag> m_weak_flag;
    mutable T m_func;
};

/** 支持基于weak_ptr的回调类的基类
*/
class UILIB_API SupportWeakCallback
{
public:
    typedef std::weak_ptr<WeakFlag> _TyWeakFlag;
public:
    SupportWeakCallback() = default;
    virtual ~SupportWeakCallback() = default;

    template<typename CallbackType>
    auto ToWeakCallback(const CallbackType& closure)
        ->WeakCallback<CallbackType>
    {
        return WeakCallback<CallbackType>(GetWeakFlag(), closure);
    }

    std::weak_ptr<WeakFlag> GetWeakFlag()
    {
        if (m_weakFlag.use_count() == 0) {
            m_weakFlag.reset((WeakFlag*)nullptr);
        }
        return m_weakFlag;
    }

private:
    template<typename ReturnValue, typename... Param, typename WeakFlag>
    static std::function<ReturnValue(Param...)> ConvertToWeakCallback(
        const std::function<ReturnValue(Param...)>& callback, std::weak_ptr<WeakFlag> expiredFlag)
    {
        auto weakCallback = [expiredFlag, callback](Param... p) {
            if (!expiredFlag.expired()) {
                return callback(p...);
            }
            return ReturnValue();
        };

        return weakCallback;
    }

protected:
    std::shared_ptr<WeakFlag> m_weakFlag;
};

// WeakCallbackFlag 一般作为类成员变量使用，要继承，可使用不带 Cancel() 函数的 SupportWeakCallback
// 这里禁止继承，主要担心误用。当使用这个类的功能，打包出多个支持 Weak 语义的 Callback 时，一旦错误的调用了 Cancel，
// 将会取消所有 Callback，这种情况可能不是用户希望的。此时，应该使用多个带有 Cancel() 函数的 WeakCallbackFlag 类型的成员变量，
// 每个对应一个 Callback，一一对应的控制每个支持 Weak 语义的 Callback。
class UILIB_API WeakCallbackFlag final : public SupportWeakCallback
{
public:
    void Cancel()
    {
        m_weakFlag.reset();
    }

    bool HasUsed()
    {
        return m_weakFlag.use_count() != 0;
    }
};

// global function 
template<class F, class... Args, class = typename std::enable_if<!std::is_member_function_pointer<F>::value>::type>
auto UiBind(F && f, Args && ... args)
    ->decltype(std::bind(f, args...))
{
    return std::bind(f, args...);
}

// global function using this
template<class C, class F, class... Args, class = typename std::enable_if<!std::is_member_function_pointer<F>::value>::type>
auto UiBind(C* p, F&& f, Args && ... args)
    -> WeakCallback<decltype(std::bind(f, args...))>
{
    std::weak_ptr<WeakFlag> weak_flag = p->GetWeakFlag();
    auto bind_obj = std::bind(f, args...);
    static_assert(std::is_base_of<SupportWeakCallback, C>::value, "SupportWeakCallback should be base of C");
    WeakCallback<decltype(bind_obj)> weak_callback(weak_flag, std::move(bind_obj));
    return weak_callback;
}

// const class member function 
template<class R, class C, class... DArgs, class P, class... Args>
auto UiBind(R(C::*f)(DArgs...) const, P && p, Args && ... args)
    ->WeakCallback<decltype(std::bind(f, p, args...))>
{
    std::weak_ptr<WeakFlag> weak_flag = p->GetWeakFlag();
    auto bind_obj = std::bind(f, p, args...);
    static_assert(std::is_base_of<SupportWeakCallback, C>::value, "SupportWeakCallback should be base of C");
    WeakCallback<decltype(bind_obj)> weak_callback(weak_flag, std::move(bind_obj));
    return weak_callback;
}

// non-const class member function 
template<class R, class C, class... DArgs, class P, class... Args>
auto UiBind(R(C::*f)(DArgs...), P && p, Args && ... args)
    ->WeakCallback<decltype(std::bind(f, p, args...))>
{
    std::weak_ptr<WeakFlag> weak_flag = p->GetWeakFlag();
    auto bind_obj = std::bind(f, p, args...);
    static_assert(std::is_base_of<SupportWeakCallback, C>::value, "SupportWeakCallback should be base of C");
    WeakCallback<decltype(bind_obj)> weak_callback(weak_flag, std::move(bind_obj));
    return weak_callback;
}

} // namespace ui

#endif  // UI_CORE_CALLBACK_H_
