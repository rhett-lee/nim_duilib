#ifndef UI_WEBVIEW2_COM_CALLBACK_H_
#define UI_WEBVIEW2_COM_CALLBACK_H_

#include "duilib/duilib_defs.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include "ComPtr.h"

#include <unknwn.h>
#include <Shlwapi.h>
#include <utility>
#include <memory>
#include <functional>
#include <type_traits>
#include <tuple>

namespace ui
{
    // 前向声明智能指针类（需配合之前实现的 ComPtr）
    template<typename T> class ComPtr;

    // ------------------------------
    // 提取接口中 Invoke 方法的参数类型（支持 STDMETHODCALLTYPE 调用约定）
    // ------------------------------

    // 基础模板：默认无 Invoke 方法
    template <typename TInterface, typename = void>
    struct InvokeTraits {
        static constexpr bool HasInvoke = false;
    };

    // 特化模板：匹配带有 STDMETHODCALLTYPE（__stdcall）的 Invoke 方法
    template <typename TInterface>
    struct InvokeTraits<TInterface, std::void_t<decltype(&TInterface::Invoke)>> {
        static constexpr bool HasInvoke = true;

        // 支持 STDMETHODCALLTYPE（__stdcall）调用约定的成员函数
        template <typename R, typename... Args>
        static std::tuple<Args...> GetArgs(R(STDMETHODCALLTYPE TInterface::*)(Args...));

        // 提取 Invoke 方法的参数类型
        using ArgsTuple = decltype(GetArgs(&TInterface::Invoke));

        // 参数数量
        static constexpr size_t ArgCount = std::tuple_size_v<ArgsTuple>;
    };

    // ------------------------------
    // 回调实现类：按参数数量特化（支持 0-6 个参数）
    // ------------------------------

    // 基础模板（禁用，必须使用特化版本）
    template <typename TInterface, const IID& IID_TInterface, typename TCallback, size_t ArgCount>
    class ComCallbackImpl;

    // 特化：0 个参数的 Invoke 方法
    template <typename TInterface, const IID& IID_TInterface, typename TCallback>
    class ComCallbackImpl<TInterface, IID_TInterface, TCallback, 0> : public TInterface {
    public:
        explicit ComCallbackImpl(TCallback callback)
            : m_callback(std::move(callback)), m_refCount(1) {
        }
        virtual ~ComCallbackImpl() = default;

        // IUnknown 实现
        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
            if (riid == IID_IUnknown || riid == IID_TInterface) {
                *ppv = static_cast<TInterface*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override {
            return InterlockedIncrement(&m_refCount);
        }

        STDMETHOD_(ULONG, Release)() override {
            ULONG ref = InterlockedDecrement(&m_refCount);
            if (ref == 0) {
                delete this;
            }
            return ref;
        }

        // 实现 0 参数 Invoke
        STDMETHOD(Invoke)() override {
            return m_callback();
        }

    private:
        TCallback m_callback;
        LONG m_refCount;
    };

    // 特化：1 个参数的 Invoke 方法
    template <typename TInterface, const IID& IID_TInterface, typename TCallback>
    class ComCallbackImpl<TInterface, IID_TInterface, TCallback, 1> : public TInterface {
    public:
        explicit ComCallbackImpl(TCallback callback)
            : m_callback(std::move(callback)), m_refCount(1) {
        }
        virtual ~ComCallbackImpl() = default;

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
            if (riid == IID_IUnknown || riid == IID_TInterface) {
                *ppv = static_cast<TInterface*>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override { return InterlockedIncrement(&m_refCount); }
        STDMETHOD_(ULONG, Release)() override {
            ULONG ref = InterlockedDecrement(&m_refCount);
            if (ref == 0) {
                delete this;
            }
            return ref;
        }

        // 实现 1 参数 Invoke
        STDMETHOD(Invoke)(
            typename std::tuple_element_t<0, typename InvokeTraits<TInterface>::ArgsTuple> arg1
            ) override {
            return m_callback(arg1);
        }

    private:
        TCallback m_callback;
        LONG m_refCount;
    };

    // 特化：2 个参数的 Invoke 方法（WebView2 常用）
    template <typename TInterface, const IID& IID_TInterface, typename TCallback>
    class ComCallbackImpl<TInterface, IID_TInterface, TCallback, 2> : public TInterface {
    public:
        explicit ComCallbackImpl(TCallback callback)
            : m_callback(std::move(callback)), m_refCount(1) {
        }
        virtual ~ComCallbackImpl() = default;

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
            if (riid == IID_IUnknown || riid == IID_TInterface) {
                *ppv = static_cast<TInterface*>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override { return InterlockedIncrement(&m_refCount); }
        STDMETHOD_(ULONG, Release)() override {
            ULONG ref = InterlockedDecrement(&m_refCount);
            if (ref == 0) {
                delete this;
            }
            return ref;
        }

        // 实现 2 参数 Invoke
        STDMETHOD(Invoke)(
            typename std::tuple_element_t<0, typename InvokeTraits<TInterface>::ArgsTuple> arg1,
            typename std::tuple_element_t<1, typename InvokeTraits<TInterface>::ArgsTuple> arg2
            ) override {
            return m_callback(arg1, arg2);
        }

    private:
        TCallback m_callback;
        LONG m_refCount;
    };

    // 特化：3 个参数的 Invoke 方法
    template <typename TInterface, const IID& IID_TInterface, typename TCallback>
    class ComCallbackImpl<TInterface, IID_TInterface, TCallback, 3> : public TInterface {
    public:
        explicit ComCallbackImpl(TCallback callback)
            : m_callback(std::move(callback)), m_refCount(1) {
        }
        virtual ~ComCallbackImpl() = default;

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
            if (riid == IID_IUnknown || riid == IID_TInterface) {
                *ppv = static_cast<TInterface*>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override { return InterlockedIncrement(&m_refCount); }
        STDMETHOD_(ULONG, Release)() override {
            ULONG ref = InterlockedDecrement(&m_refCount);
            if (ref == 0) {
                delete this;
            }
            return ref;
        }

        // 实现 3 参数 Invoke
        STDMETHOD(Invoke)(
            typename std::tuple_element_t<0, typename InvokeTraits<TInterface>::ArgsTuple> arg1,
            typename std::tuple_element_t<1, typename InvokeTraits<TInterface>::ArgsTuple> arg2,
            typename std::tuple_element_t<2, typename InvokeTraits<TInterface>::ArgsTuple> arg3
            ) override {
            return m_callback(arg1, arg2, arg3);
        }

    private:
        TCallback m_callback;
        LONG m_refCount;
    };

    // 特化：4 个参数的 Invoke 方法
    template <typename TInterface, const IID& IID_TInterface, typename TCallback>
    class ComCallbackImpl<TInterface, IID_TInterface, TCallback, 4> : public TInterface {
    public:
        explicit ComCallbackImpl(TCallback callback)
            : m_callback(std::move(callback)), m_refCount(1) {
        }
        virtual ~ComCallbackImpl() = default;

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
            if (riid == IID_IUnknown || riid == IID_TInterface) {
                *ppv = static_cast<TInterface*>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override { return InterlockedIncrement(&m_refCount); }
        STDMETHOD_(ULONG, Release)() override {
            ULONG ref = InterlockedDecrement(&m_refCount);
            if (ref == 0) {
                delete this;
            }
            return ref;
        }

        // 实现 4 参数 Invoke
        STDMETHOD(Invoke)(
            typename std::tuple_element_t<0, typename InvokeTraits<TInterface>::ArgsTuple> arg1,
            typename std::tuple_element_t<1, typename InvokeTraits<TInterface>::ArgsTuple> arg2,
            typename std::tuple_element_t<2, typename InvokeTraits<TInterface>::ArgsTuple> arg3,
            typename std::tuple_element_t<3, typename InvokeTraits<TInterface>::ArgsTuple> arg4
            ) override {
            return m_callback(arg1, arg2, arg3, arg4);
        }

    private:
        TCallback m_callback;
        LONG m_refCount;
    };

    // 特化：5 个参数的 Invoke 方法
    template <typename TInterface, const IID& IID_TInterface, typename TCallback>
    class ComCallbackImpl<TInterface, IID_TInterface, TCallback, 5> : public TInterface {
    public:
        explicit ComCallbackImpl(TCallback callback)
            : m_callback(std::move(callback)), m_refCount(1) {
        }
        virtual ~ComCallbackImpl() = default;

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
            if (riid == IID_IUnknown || riid == IID_TInterface) {
                *ppv = static_cast<TInterface*>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override { return InterlockedIncrement(&m_refCount); }
        STDMETHOD_(ULONG, Release)() override {
            ULONG ref = InterlockedDecrement(&m_refCount);
            if (ref == 0) {
                delete this;
            }
            return ref;
        }

        // 实现 5 参数 Invoke
        STDMETHOD(Invoke)(
            typename std::tuple_element_t<0, typename InvokeTraits<TInterface>::ArgsTuple> arg1,
            typename std::tuple_element_t<1, typename InvokeTraits<TInterface>::ArgsTuple> arg2,
            typename std::tuple_element_t<2, typename InvokeTraits<TInterface>::ArgsTuple> arg3,
            typename std::tuple_element_t<3, typename InvokeTraits<TInterface>::ArgsTuple> arg4,
            typename std::tuple_element_t<4, typename InvokeTraits<TInterface>::ArgsTuple> arg5
            ) override {
            return m_callback(arg1, arg2, arg3, arg4, arg5);
        }

    private:
        TCallback m_callback;
        LONG m_refCount;
    };

    // 特化：6 个参数的 Invoke 方法
    template <typename TInterface, const IID& IID_TInterface, typename TCallback>
    class ComCallbackImpl<TInterface, IID_TInterface, TCallback, 6> : public TInterface {
    public:
        explicit ComCallbackImpl(TCallback callback)
            : m_callback(std::move(callback)), m_refCount(1) {
        }
        virtual ~ComCallbackImpl() = default;

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
            if (riid == IID_IUnknown || riid == IID_TInterface) {
                *ppv = static_cast<TInterface*>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override { return InterlockedIncrement(&m_refCount); }
        STDMETHOD_(ULONG, Release)() override {
            ULONG ref = InterlockedDecrement(&m_refCount);
            if (ref == 0) {
                delete this;
            }
            return ref;
        }

        // 实现 6 参数 Invoke
        STDMETHOD(Invoke)(
            typename std::tuple_element_t<0, typename InvokeTraits<TInterface>::ArgsTuple> arg1,
            typename std::tuple_element_t<1, typename InvokeTraits<TInterface>::ArgsTuple> arg2,
            typename std::tuple_element_t<2, typename InvokeTraits<TInterface>::ArgsTuple> arg3,
            typename std::tuple_element_t<3, typename InvokeTraits<TInterface>::ArgsTuple> arg4,
            typename std::tuple_element_t<4, typename InvokeTraits<TInterface>::ArgsTuple> arg5,
            typename std::tuple_element_t<5, typename InvokeTraits<TInterface>::ArgsTuple> arg6
            ) override {
            return m_callback(arg1, arg2, arg3, arg4, arg5, arg6);
        }

    private:
        TCallback m_callback;
        LONG m_refCount;
    };

    // ------------------------------
    // 工厂函数：创建回调实例
    // ------------------------------

    template <typename TInterface, const IID& IID_TInterface, typename TCallback>
    ComPtr<TInterface> ComCallback(TCallback&& callback) {
        // 验证接口必须包含 Invoke 方法
        static_assert(InvokeTraits<TInterface>::HasInvoke,
            "接口必须定义 STDMETHODCALLTYPE 调用约定的 Invoke 方法");

        // 验证参数数量不超过 6 个
        static_assert(InvokeTraits<TInterface>::ArgCount <= 6,
            "Invoke 方法最多支持 6 个参数");

        // 实例化对应参数数量的实现类
        using Impl = ComCallbackImpl<
            TInterface,
            IID_TInterface,
            std::decay_t<TCallback>,
            InvokeTraits<TInterface>::ArgCount
        >;

        return ComPtr<TInterface>(new Impl(std::forward<TCallback>(callback)));
    }

    // 支持成员函数的版本（匹配 STDMETHODCALLTYPE 调用约定）
    template <typename TInterface, const IID& IID_TInterface, typename T, typename... Args>
    ComPtr<TInterface> ComCallback(T* obj, HRESULT(STDMETHODCALLTYPE T::* method)(Args...)) {
        return ComCallback<TInterface, IID_TInterface>(
            [obj, method](Args... args) -> HRESULT {
                return (obj->*method)(args...);
            }
        );
    }

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2

#endif //UI_WEBVIEW2_COM_CALLBACK_H_
