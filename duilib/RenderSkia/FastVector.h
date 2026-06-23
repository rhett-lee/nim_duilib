#ifndef UI_RENDER_SKIA_FAST_VECTOR_H_
#define UI_RENDER_SKIA_FAST_VECTOR_H_

#include <cstdlib>   // malloc, free
#include <cstring>   // memcpy
#include <cstddef>   // size_t

namespace ui 
{
// 极致性能的通用模板容器，替代 std::vector
// 特性：无异常、无拷贝、无值初始化、纯原生内存、极致访问速度
template <typename T>
class FastVector
{
public:
    // 定义STL兼容的迭代器类型（原生指针，最快）
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = size_t;

public:
    // 默认构造函数：空容器，无内存分配
    FastVector() noexcept
        : m_data(nullptr), m_size(0), m_capacity(0)
    {
    }

    // 析构函数：释放内存
    ~FastVector() noexcept
    {
        if (m_data != nullptr) {
            std::free(m_data);
        }
    }

    // 禁用拷贝（追求极致速度；需要拷贝可自行实现）
    FastVector(const FastVector&) = delete;
    FastVector& operator=(const FastVector&) = delete;

    // 支持移动语义（高性能转移所有权，不拷贝数据）
    FastVector(FastVector&& other) noexcept
        : m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity)
    {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    FastVector& operator=(FastVector&& other) noexcept
    {
        if (this != &other) {
            // 释放自身内存
            std::free(m_data);

            // 接管 other 的资源
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            // 清空 other
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    // reserve：预分配内存，不改变大小，无初始化（最快）
    void reserve(size_t new_capacity) noexcept
    {
        if (new_capacity <= m_capacity) {
            return;
        }

        // 按类型大小分配，保证内存对齐与类型安全
        T* new_data = static_cast<T*>(std::malloc(new_capacity * sizeof(T)));
        if (!new_data) {
            return; // 内存分配失败不处理（极致性能）
        }

        // 拷贝已有数据（仅拷贝有效数据，无冗余）
        if (m_data != nullptr && m_size > 0) {
            std::memcpy(new_data, m_data, m_size * sizeof(T));
            std::free(m_data);
        }

        m_data = new_data;
        m_capacity = new_capacity;
    }

    // push_back：尾部添加元素，无异常，无冗余检查
    inline void push_back(const T& value) noexcept
    {
        // 容量不足时自动扩容（1.5倍扩容，比2倍更省内存，速度接近）
        if (m_size >= m_capacity) {
            const size_t new_cap = (m_capacity == 0) ? 16 : m_capacity * 3 / 2;
            reserve(new_cap);
        }
        // 直接赋值，不调用构造函数（极致性能）
        m_data[m_size++] = value;
    }

    // size：返回当前元素数量（内联，直接读取）
    inline size_t size() const noexcept
    {
        return m_size;
    }

    // capacity：返回当前容量（内联，直接读取）
    inline size_t capacity() const noexcept
    {
        return m_capacity;
    }

    // 快速直接访问（无越界检查，极致速度）
    inline T& operator[](size_t index) noexcept
    {
        return m_data[index];
    }

    // const 快速访问
    inline const T& operator[](size_t index) const noexcept
    {
        return m_data[index];
    }

    // empty：返回当前元素数量（内联，直接读取）
    inline bool empty() const noexcept
    {
        return m_size == 0;
    }

    // 数量清零，保留内存（极快）
    inline void clear() noexcept
    {
        m_size = 0;
    }

    // swap函数
    inline void swap(FastVector& other) noexcept
    {
        // 仅交换指针、大小、容量 → 零拷贝，极致性能
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    // 获取原生指针（兼容C接口）
    inline T* data() noexcept
    {
        return m_data;
    }

    // 获取原生指针（兼容C接口）
    inline const T* data() const noexcept
    {
        return m_data;
    }

    // 重置大小：扩大时不初始化（极致性能），缩小时直接截断
    void resize(size_t new_size) noexcept
    {
        // 大小不变，直接返回
        if (new_size == m_size) {
            return;
        }

        // 缩小：直接改大小，不释放内存
        if (new_size < m_size) {
            m_size = new_size;
            return;
        }

        // 扩大：需要扩容
        if (new_size > m_capacity) {
            reserve(new_size);
        }

        // 只改大小，不初始化新元素（最快）
        m_size = new_size;
    }

    // 版本2：resize 并使用指定值初始化新元素（安全、兼容STL）
    void resize(size_t new_size, const T& default_value) noexcept
    {
        if (new_size == m_size) {
            return;
        }

        // 缩小：直接截断
        if (new_size < m_size) {
            m_size = new_size;
            return;
        }

        // 扩大：先确保容量足够
        if (new_size > m_capacity) {
            reserve(new_size);
        }

        // 将新增的元素全部赋值为 default_value
        for (size_t i = m_size; i < new_size; ++i) {
            m_data[i] = default_value;
        }

        m_size = new_size;
    }

    // --------------------------
    // STL 迭代器支持（核心新增）
    // --------------------------
    inline iterator begin() noexcept
    {
        return m_data;
    }

    inline const_iterator begin() const noexcept
    {
        return m_data;
    }

    inline const_iterator cbegin() const noexcept
    {
        return m_data;
    }

    inline iterator end() noexcept
    {
        return m_data + m_size;
    }

    inline const_iterator end() const noexcept
    {
        return m_data + m_size;
    }

    inline const_iterator cend() const noexcept
    {
        return m_data + m_size;
    }

private:
    T* m_data;      // 类型安全指针
    size_t m_size;      // 当前元素个数
    size_t m_capacity;  // 已分配容量（元素个数）
};
} // namespace ui

#endif // UI_RENDER_SKIA_FAST_VECTOR_H_
