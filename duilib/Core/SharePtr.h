#ifndef UI_CORE_SHARE_PTR_H_
#define UI_CORE_SHARE_PTR_H_

#include "duilib/duilib_defs.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <type_traits>
#include <utility>

namespace ui
{
/** 引用计数的实现（基于Skia源码skia\include\core\SkRefCnt.h做的修改）
*/
class RefCount
{
public:
    /** 默认构造函数，初始的引用计数为1
    */
    RefCount(): m_nRefCnt(1)
    {
    }

    /** 析构函数
    */
    virtual ~RefCount()
    {
    #ifdef _DEBUG
        ASSERT(this->GetRefCount() == 1);
        // 非法值，用于在delete后再次复用时触发断言
        m_nRefCnt.store(0, std::memory_order_relaxed);
    #endif
    }

    /** 如果调用者是唯一的所有者，可能返回true。
     *  该函数可确保之前所有者的操作均已完成。
     */
    bool Unique() const
    {
        if (1 == m_nRefCnt.load(std::memory_order_acquire)) {
            // 只有当返回true时才真正需要acquire屏障：它可防止依赖Unique()结果的代码
            // 在之前所有所有者完全执行完Release()之前运行。
            return true;
        }
        return false;
    }

    /** 增加引用计数，必须与一次Release()调用配对使用。
    */
    void AddRef() const
    {
#ifdef _DEBUG
        ASSERT(this->GetRefCount() > 0);
#endif
        // 无需内存屏障。
        (void)m_nRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    /** Decrement the reference count. If the reference count is 1 before the
        decrement, then delete the object. Note that if this is the case, then
        the object needs to have been allocated via new, and not on the stack.
    */
    void Release() const {
#ifdef _DEBUG
        ASSERT(this->GetRefCount() > 0);
#endif
        // 此处的release屏障可替代我们在ref()中"本应"执行的所有release操作。
        if (1 == m_nRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            // 与unique()类似，acquire屏障只在成功时才需要，
            // 用于确保decrement之前不会执行OnInternalDispose()中的代码。
            this->OnInternalDispose();
        }
    }

private:

#ifdef _DEBUG
    /** 获取引用计数，仅用于调试。
    */
    int32_t GetRefCount() const {
        return m_nRefCnt.load(std::memory_order_relaxed);
    }
#endif

    /**
     *  当引用计数减为0时被调用。
     */
    virtual void OnInternalDispose() const {
    #ifdef _DEBUG
        ASSERT(0 == this->GetRefCount());
        m_nRefCnt.store(1, std::memory_order_relaxed);
    #endif
        delete this;
    }

    mutable std::atomic<int32_t> m_nRefCnt;

    RefCount(RefCount&&) = delete;
    RefCount(const RefCount&) = delete;
    RefCount& operator=(RefCount&&) = delete;
    RefCount& operator=(const RefCount&) = delete;
};

///////////////////////////////////////////////////////////////////////////////
/** 检查参数是否非空，若非空则调用 obj->AddRef() 并返回 obj。
 */
template <typename T> static inline T* SafeAddRef(T* obj) {
    if (obj) {
        obj->AddRef();
    }
    return obj;
}

/** 检查参数是否非空，若非空则调用 obj->Release()
 */
template <typename T> static inline void SafeRelease(T* obj) {
    if (obj) {
        obj->Release();
    }
}

///////////////////////////////////////////////////////////////////////////////

/** 没有虚函数的引用计数实现
*/
template <typename Derived>
class NVRefCount
{
public:
    NVRefCount() : m_nRefCnt(1) {}
    ~NVRefCount()
    {
        ASSERT(m_nRefCnt.load(std::memory_order_relaxed) == 1);
    }

    // 实现与 SkRefCntBase 基本相同，所需的内存屏障也相同：
    //   - Unique() 在返回true时需要acquire，返回false时不需要屏障；
    //   - AddRef() 不需要任何屏障；
    //   - Release() 需要release屏障，如果要执行delete则需要acquire。
    bool Unique() const { return 1 == m_nRefCnt.load(std::memory_order_acquire); }
    void AddRef() const { (void)m_nRefCnt.fetch_add(+1, std::memory_order_relaxed); }
    void Release() const
    {
        if (1 == m_nRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            // 恢复为1，以满足析构函数中的断言
#ifdef _DEBUG
            m_nRefCnt.store(1, std::memory_order_relaxed);
#endif
            delete (const Derived*)this;
        }
    }

private:
    mutable std::atomic<int32_t> m_nRefCnt;

    NVRefCount(NVRefCount&&) = delete;
    NVRefCount(const NVRefCount&) = delete;
    NVRefCount& operator=(NVRefCount&&) = delete;
    NVRefCount& operator=(const NVRefCount&) = delete;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

/** 智能指针类，用于包装支持 AddRef()/Release() 接口的类。
 *  可用于继承自 RefCount/NVRefCount 的类。
 */
template <typename T>
class SharePtr
{
public:
    using element_type = T;

    constexpr SharePtr() : m_ptr(nullptr) {}
    constexpr SharePtr(std::nullptr_t) : m_ptr(nullptr) {}

    /**
     *  通过调用AddRef()共享底层对象，使参数对象和新建的SharePtr都持有该对象的引用。
     */
    SharePtr(const SharePtr<T>& that) : m_ptr(SafeAddRef(that.get())) {}
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SharePtr(const SharePtr<U>& that) : m_ptr(SafeAddRef(that.get())) {}

    /**
     *  将底层对象从参数对象移动到新建的SharePtr。此后只有新的SharePtr持有该对象的引用，
     *  参数对象将指向空。不会调用ref()或unref()。
     */
    SharePtr(SharePtr<T>&& that) : m_ptr(that.release()) {}
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SharePtr(SharePtr<U>&& that) : m_ptr(that.release()) {}

    /**
     *  将裸指针交由新建的SharePtr接管。
     *  不会调用ref()或unref()。
     */
    explicit SharePtr(T* obj) : m_ptr(obj) {}

    /**
     *  对底层对象指针调用Release()。
     */
    ~SharePtr() {
        SafeRelease(m_ptr);
#ifdef _DEBUG
        m_ptr = nullptr;
#endif // _DEBUG
    }

    SharePtr<T>& operator=(std::nullptr_t) { this->reset(); return *this; }

    /**
     *  通过调用ref()共享参数对象所引用的底层对象。如果该SharePtr此前持有某个对象的引用
     *  （即不为空），则会对该对象调用unref()。
     */
    SharePtr<T>& operator=(const SharePtr<T>& that) {
        if (this != &that) {
            this->reset(SafeAddRef(that.get()));
        }
        return *this;
    }
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SharePtr<T>& operator=(const SharePtr<U>& that) {
        this->reset(SafeAddRef(that.get()));
        return *this;
    }

    /**
     *  将底层对象从参数对象移动到该SharePtr。如果该SharePtr此前持有另一个对象的引用，
     *  则会对该对象调用unref()。不会调用ref()。
     */
    SharePtr<T>& operator=(SharePtr<T>&& that) {
        this->reset(that.release());
        return *this;
    }
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SharePtr<T>& operator=(SharePtr<U>&& that) {
        this->reset(that.release());
        return *this;
    }

    T& operator*() const {
        ASSERT(this->get() != nullptr);
        return *this->get();
    }

    explicit operator bool() const { return this->get() != nullptr; }

    T* get() const { return m_ptr; }
    T* operator->() const { return m_ptr; }

    /**
     *  接管新的裸指针，并对此前持有的对象（如果不为空）调用unref()。
     *  不会调用ref()。
     */
    void reset(T* ptr = nullptr) {
        // 调用 m_ptr->Release() 可能触发 this->~() 或 this->reset(T*)。
        // http://wg21.cmeerw.net/lwg/issue998
        // http://wg21.cmeerw.net/lwg/issue2262
        T* oldPtr = m_ptr;
        m_ptr = ptr;
        SafeRelease(oldPtr);
    }

    /**
     *  返回裸指针，并将内部对象指针置为nullptr。
     *  调用者必须接管该对象的所有权，并自行管理其引用计数。
     *  不会调用unref()。
     */
    [[nodiscard]] T* release() {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    /** 与另一个SharePtr交换所持有的对象指针
    */
    void swap(SharePtr<T>& that) /*noexcept*/ {
        using std::swap;
        swap(m_ptr, that.m_ptr);
    }

private:
    T* m_ptr;
};

/** 交换两个SharePtr对象所持有的对象指针
*/
template <typename T> inline void swap(SharePtr<T>& a, SharePtr<T>& b) /*noexcept*/ {
    a.swap(b);
}

/** 判断两个SharePtr是否指向同一个对象
*/
template <typename T, typename U> inline bool operator==(const SharePtr<T>& a, const SharePtr<U>& b) {
    return a.get() == b.get();
}
template <typename T> inline bool operator==(const SharePtr<T>& a, std::nullptr_t) /*noexcept*/ {
    return !a;
}
template <typename T> inline bool operator==(std::nullptr_t, const SharePtr<T>& b) /*noexcept*/ {
    return !b;
}

/** 判断两个SharePtr是否指向不同的对象
*/
template <typename T, typename U> inline bool operator!=(const SharePtr<T>& a, const SharePtr<U>& b) {
    return a.get() != b.get();
}
template <typename T> inline bool operator!=(const SharePtr<T>& a, std::nullptr_t) /*noexcept*/ {
    return static_cast<bool>(a);
}
template <typename T> inline bool operator!=(std::nullptr_t, const SharePtr<T>& b) /*noexcept*/ {
    return static_cast<bool>(b);
}

template <typename C, typename CT, typename T>
auto operator<<(std::basic_ostream<C, CT>& os, const SharePtr<T>& sp) -> decltype(os << sp.get()) {
    return os << sp.get();
}

/** 创建SharePtr对象（使用new创建T类型的对象，并交由SharePtr接管）
* @param [in] args 构造T类型对象所需的参数
* @return 返回新创建的SharePtr对象
*/
template <typename T, typename... Args>
SharePtr<T> make_share_ptr(Args&&... args) {
    return SharePtr<T>(new T(std::forward<Args>(args)...));
}

/**
 *  返回包装指定指针的SharePtr，并对该指针（如果不为空）调用ref。
 *
 *  这与SharePtr构造函数的语义不同：构造函数只是包装指针，相当于"接管"它。
 */
template <typename T> SharePtr<T> ref_share_ptr(T* obj) {
    return SharePtr<T>(SafeAddRef(obj));
}

template <typename T> SharePtr<T> ref_share_ptr(const T* obj) {
    return SharePtr<T>(const_cast<T*>(SafeAddRef(obj)));
}

} // namespace ui

#endif //UI_CORE_SHARE_PTR_H_
