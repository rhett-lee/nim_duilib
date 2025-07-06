#ifndef UI_WEBVIEW2_COM_PTR_H_
#define UI_WEBVIEW2_COM_PTR_H_

#include "duilib/duilib_defs.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include <cstddef>
#include <utility>

namespace ui
{
/** 轻量级COM智能指针，自动管理COM对象的生命周期
 *  实现了COM对象的引用计数自动管理，避免内存泄漏
 */
template<typename T>
class ComPtr
{
public:
    typedef T Interface;

    /** 默认构造函数，初始化为空指针
     */
    ComPtr(): m_pInterface(nullptr) {}
    
    /** 空指针构造函数
     */
    ComPtr(std::nullptr_t) : m_pInterface(nullptr) {}

    /** 从原始COM指针构造
     * @param pInterface 原始COM接口指针，会增加引用计数
     */
    explicit ComPtr(T* pInterface) : m_pInterface(pInterface)
    {
        if (m_pInterface) {
            m_pInterface->AddRef(); // 获取所有权时增加引用计数
        }
    }

    /** 拷贝构造函数
     * @param other 另一个ComPtr实例，共享同一个COM对象
     */
    ComPtr(const ComPtr& other) : m_pInterface(other.m_pInterface)
    {
        if (m_pInterface) {
            m_pInterface->AddRef(); // 拷贝时增加引用计数
        }
    }

    /** 模板拷贝构造函数，支持从兼容接口类型构造
     * @tparam U 源接口类型，必须能转换为T
     * @param other 源ComPtr实例
     */
    template<typename U>
    ComPtr(const ComPtr<U>& other) : m_pInterface(other.Get())
    {
        if (m_pInterface) {
            m_pInterface->AddRef(); // 跨接口类型拷贝时增加引用计数
        }
    }

    /** 移动构造函数
     * @param other 被移动的ComPtr实例，移动后变为空
     */
    ComPtr(ComPtr&& other) noexcept : m_pInterface(other.m_pInterface)
    {
        other.m_pInterface = nullptr; // 转移所有权，原指针置空
    }

    /** 模板移动构造函数，支持从兼容接口类型移动
     * @tparam U 源接口类型，必须能转换为T
     * @param other 被移动的ComPtr实例
     */
    template<typename U>
    ComPtr(ComPtr<U>&& other) noexcept : m_pInterface(other.Detach()) {}

    /** 析构函数，自动释放持有的COM对象
     */
    ~ComPtr()
    {
        if (m_pInterface) {
            m_pInterface->Release(); // 释放所有权时减少引用计数
        }
    }

    /** 空指针赋值运算符
     */
    ComPtr& operator=(std::nullptr_t)
    {
        Reset();
        return *this;
    }

    /** 原始指针赋值运算符
     * @param pInterface 原始COM接口指针
     */
    ComPtr& operator=(T* pInterface)
    {
        if (m_pInterface != pInterface) {
            ComPtr temp(pInterface);
            Swap(temp); // 使用临时对象实现异常安全的赋值
        }
        return *this;
    }

    /** 拷贝赋值运算符
     * @param other 另一个ComPtr实例
     */
    ComPtr& operator=(const ComPtr& other)
    {
        if (this != &other) {
            ComPtr temp(other);
            Swap(temp); // 使用拷贝并交换技术实现异常安全
        }
        return *this;
    }

    /** 模板拷贝赋值运算符
     * @tparam U 源接口类型
     * @param other 源ComPtr实例
     */
    template<typename U>
    ComPtr& operator=(const ComPtr<U>& other)
    {
        ComPtr temp(other);
        Swap(temp);
        return *this;
    }

    /** 移动赋值运算符
     * @param other 被移动的ComPtr实例
     */
    ComPtr& operator=(ComPtr&& other) noexcept
    {
        ComPtr temp(std::move(other));
        Swap(temp);
        return *this;
    }

    /** 模板移动赋值运算符
     * @tparam U 源接口类型
     * @param other 被移动的ComPtr实例
     */
    template<typename U>
    ComPtr& operator=(ComPtr<U>&& other) noexcept
    {
        ComPtr temp(std::move(other));
        Swap(temp);
        return *this;
    }

    /** 获取指针地址，用于接收COM对象创建函数的输出
     * @note 调用此方法会重置当前指针
     */
    T** GetAddressOf()
    {
        Reset();
        return &m_pInterface;
    }

    /** 获取常量指针地址，用于const对象
     */
    T* const* GetAddressOf() const
    {
        return &m_pInterface;
    }

    /** 释放当前对象并获取指针地址
     * @note 调用此方法会重置当前指针
     */
    T** ReleaseAndGetAddressOf()
    {
        Reset();
        return &m_pInterface;
    }

    /** 分离当前指针，放弃所有权但不释放对象
     * @return 原始COM接口指针
     */
    T* Detach()
    {
        T* pInterface = m_pInterface;
        m_pInterface = nullptr;
        return pInterface;
    }

    /** 重置指针为空，释放持有的COM对象
     */
    void Reset()
    {
        ComPtr().Swap(*this); // 使用临时对象交换实现安全释放
    }

    /** 重置指针为新值，释放原有对象
     * @param pInterface 新的COM接口指针
     */
    void Reset(T* pInterface)
    {
        ComPtr(pInterface).Swap(*this);
    }

    /** 交换两个ComPtr实例的内容
     * @param other 要交换的另一个ComPtr实例
     */
    void Swap(ComPtr& other) noexcept
    {
        T* temp = m_pInterface;
        m_pInterface = other.m_pInterface;
        other.m_pInterface = temp;
    }

    /** 获取原始COM接口指针
     * @return 原始COM接口指针，可能为nullptr
     */
    T* Get() const
    {
        return m_pInterface;
    }

    /** 获取原始COM接口指针的指针
    */
    T** operator&() noexcept {
        if (m_pInterface) {
            m_pInterface->Release();
            m_pInterface = nullptr;
        }
        return &m_pInterface;
    }

    /** 重载箭头运算符，直接访问COM接口方法
     */
    T* operator->() const
    {
        return m_pInterface;
    }

    /** 布尔类型转换，用于条件判断
     */
    explicit operator bool() const
    {
        return m_pInterface != nullptr;
    }

    /** 相等比较运算符
     * @tparam U 比较的接口类型
     */
    template<typename U>
    bool operator==(const ComPtr<U>& other) const
    {
        return m_pInterface == other.Get();
    }

    /** 与空指针比较
     */
    bool operator==(std::nullptr_t) const
    {
        return m_pInterface == nullptr;
    }

    /** 不等比较运算符
     * @tparam U 比较的接口类型
     */
    template<typename U>
    bool operator!=(const ComPtr<U>& other) const
    {
        return !(*this == other);
    }

    /** 与空指针比较
     */
    bool operator!=(std::nullptr_t) const
    {
        return !(*this == nullptr);
    }

    /** 查询接口，获取另一个COM接口
     * @tparam U 目标接口类型
     * @param p 输出参数，接收目标接口指针
     * @return HRESULT表示操作结果
     */
    template<typename U>
    HRESULT As(ComPtr<U>* p) const
    {
        if (!p) {
            return E_POINTER;
        }
        return QueryInterface(IID_PPV_ARGS(p));
    }

    /** 查询接口，获取另一个COM接口
     * @tparam U 目标接口类型
     * @return 包含目标接口的ComPtr实例
     */
    template<typename U>
    ComPtr<U> As() const
    {
        ComPtr<U> result;
        As(&result);
        return result;
    }

    /** 根据IID查询接口
     * @param riid 目标接口的IID
     * @param ppvObject 输出参数，接收目标接口指针
     * @return HRESULT表示操作结果
     */
    template<typename U>
    HRESULT AsIID(REFIID riid, void** ppvObject) const
    {
        if (!ppvObject)
        {
            return E_POINTER;
        }
        *ppvObject = nullptr;
        return m_pInterface ? m_pInterface->QueryInterface(riid, ppvObject) : E_NOINTERFACE;
    }

private:
    template<typename U> friend class ComPtr;
    T* m_pInterface; // 管理的COM接口指针
};

// 辅助函数

/** 从原始指针创建ComPtr实例
 * @tparam T 目标接口类型
 * @tparam U 源接口类型
 * @param p 原始COM接口指针
 * @return ComPtr实例
 */
template<typename T, typename U>
ComPtr<T> MakeComPtr(U* p)
{
    return ComPtr<T>(p);
}

/** 创建ComPtr实例
*/
template<typename T>
ComPtr<T> MakeComPtr()
{
    return ComPtr<T>(new T);
}

/**
 * @brief 原始指针与ComPtr比较的辅助函数
 */
template<typename T, typename U>
bool operator==(U* lhs, const ComPtr<T>& rhs)
{
    return lhs == rhs.Get();
}

/** 原始指针与ComPtr比较的辅助函数
 */
template<typename T, typename U>
bool operator!=(U* lhs, const ComPtr<T>& rhs)
{
    return !(lhs == rhs);
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2

#endif //UI_WEBVIEW2_COM_PTR_H_
