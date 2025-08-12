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
class UILIB_API RefCount
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
        // illegal value, to catch us if we reuse after delete
        m_nRefCnt.store(0, std::memory_order_relaxed);
    #endif
    }

    /** May return true if the caller is the only owner.
     *  Ensures that all previous owner's actions are complete.
     */
    bool Unique() const
    {
        if (1 == m_nRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It
            // prevents code conditioned on the result of Unique() from running
            // until previous owners are all totally done calling Release().
            return true;
        }
        return false;
    }

    /** Increment the reference count. Must be balanced by a call to Release().
    */
    void AddRef() const
    {
#ifdef _DEBUG
        ASSERT(this->GetRefCount() > 0);
#endif
        // No barrier required.
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
        // A release here acts in place of all releases we "should" have been doing in ref().
        if (1 == m_nRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            // Like unique(), the acquire is only needed on success, to make sure
            // code in internal_dispose() doesn't happen before the decrement.
            this->OnInternalDispose();
        }
    }

private:

#ifdef _DEBUG
    /** Return the reference count. Use only for debugging. */
    int32_t GetRefCount() const {
        return m_nRefCnt.load(std::memory_order_relaxed);
    }
#endif

    /**
     *  Called when the ref count goes to 0.
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
/** Check if the argument is non-null, and if so, call obj->AddRef() and return obj.
 */
template <typename T> static inline T* SafeAddRef(T* obj) {
    if (obj) {
        obj->AddRef();
    }
    return obj;
}

/** Check if the argument is non-null, and if so, call obj->unref()
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

    // Implementation is pretty much the same as SkRefCntBase. All required barriers are the same:
    //   - Unique() needs acquire when it returns true, and no barrier if it returns false;
    //   - AddRef() doesn't need any barrier;
    //   - Release() needs a release barrier, and an acquire if it's going to call delete.
    bool Unique() const { return 1 == m_nRefCnt.load(std::memory_order_acquire); }
    void AddRef() const { (void)m_nRefCnt.fetch_add(+1, std::memory_order_relaxed); }
    void Release() const
    {
        if (1 == m_nRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            // restore the 1 for our destructor's assert
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

/**
 *  Shared pointer class to wrap classes that support a AddRef()/Release() interface.
 *  This can be used for classes inheriting from RefCount/NVRefCount
 */
template <typename T>
class UILIB_API SharePtr
{
public:
    using element_type = T;

    constexpr SharePtr() : m_ptr(nullptr) {}
    constexpr SharePtr(std::nullptr_t) : m_ptr(nullptr) {}

    /**
     *  Shares the underlying object by calling AddRef(), so that both the argument and the newly
     *  created SharePtr both have a reference to it.
     */
    SharePtr(const SharePtr<T>& that) : m_ptr(SafeAddRef(that.get())) {}
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SharePtr(const SharePtr<U>& that) : m_ptr(SafeAddRef(that.get())) {}

    /**
     *  Move the underlying object from the argument to the newly created SharePtr. Afterwards only
     *  the new SharePtr will have a reference to the object, and the argument will point to null.
     *  No call to ref() or unref() will be made.
     */
    SharePtr(SharePtr<T>&& that) : m_ptr(that.release()) {}
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    SharePtr(SharePtr<U>&& that) : m_ptr(that.release()) {}

    /**
     *  Adopt the bare pointer into the newly created SharePtr.
     *  No call to ref() or unref() will be made.
     */
    explicit SharePtr(T* obj) : m_ptr(obj) {}

    /**
     *  Calls Release() on the underlying object pointer.
     */
    ~SharePtr() {
        SafeRelease(m_ptr);
#ifdef _DEBUG
        m_ptr = nullptr;
#endif // _DEBUG
    }

    SharePtr<T>& operator=(std::nullptr_t) { this->reset(); return *this; }

    /**
     *  Shares the underlying object referenced by the argument by calling ref() on it. If this
     *  SharePtr previously had a reference to an object (i.e. not null) it will call unref() on that
     *  object.
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
     *  Move the underlying object from the argument to the SharePtr. If the SharePtr previously held
     *  a reference to another object, unref() will be called on that object. No call to ref()
     *  will be made.
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
     *  Adopt the new bare pointer, and call unref() on any previously held object (if not null).
     *  No call to ref() will be made.
     */
    void reset(T* ptr = nullptr) {
        // Calling m_ptr->Release() may call this->~() or this->reset(T*).
        // http://wg21.cmeerw.net/lwg/issue998
        // http://wg21.cmeerw.net/lwg/issue2262
        T* oldPtr = m_ptr;
        m_ptr = ptr;
        SafeRelease(oldPtr);
    }

    /**
     *  Return the bare pointer, and set the internal object pointer to nullptr.
     *  The caller must assume ownership of the object, and manage its reference count directly.
     *  No call to unref() will be made.
     */
    [[nodiscard]] T* release() {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    void swap(SharePtr<T>& that) /*noexcept*/ {
        using std::swap;
        swap(m_ptr, that.m_ptr);
    }

private:
    T* m_ptr;
};

template <typename T> inline void swap(SharePtr<T>& a, SharePtr<T>& b) /*noexcept*/ {
    a.swap(b);
}

template <typename T, typename U> inline bool operator==(const SharePtr<T>& a, const SharePtr<U>& b) {
    return a.get() == b.get();
}
template <typename T> inline bool operator==(const SharePtr<T>& a, std::nullptr_t) /*noexcept*/ {
    return !a;
}
template <typename T> inline bool operator==(std::nullptr_t, const SharePtr<T>& b) /*noexcept*/ {
    return !b;
}

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

template <typename T, typename... Args>
SharePtr<T> make_share_ptr(Args&&... args) {
    return SharePtr<T>(new T(std::forward<Args>(args)...));
}

/*
 *  Returns a SharePtr wrapping the provided ptr AND calls ref on it (if not null).
 *
 *  This is different than the semantics of the constructor for SharePtr, which just wraps the ptr,
 *  effectively "adopting" it.
 */
template <typename T> SharePtr<T> ref_share_ptr(T* obj) {
    return SharePtr<T>(SafeAddRef(obj));
}

template <typename T> SharePtr<T> ref_share_ptr(const T* obj) {
    return SharePtr<T>(const_cast<T*>(SafeAddRef(obj)));
}

} // namespace ui

#endif //UI_CORE_SHARE_PTR_H_
