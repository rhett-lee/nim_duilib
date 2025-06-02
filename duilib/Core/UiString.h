#ifndef UI_CORE_UISTRING_H_
#define UI_CORE_UISTRING_H_

#include "duilib/Utils/StringUtil.h"

namespace ui
{

/** 控件使用的字符串，用于替代DString，以减少控件的内存占用
*   该类适合用于较低的内存空间来存储字符串，性能方面由于字符串复制偏多，性能偏弱
*/
template<typename T>
class UILIB_API UiStringT
{
    using string_type = std::basic_string<T, std::char_traits<T>, std::allocator<T>>;
    using value_type = typename string_type::value_type;
public:
    UiStringT(): m_pData(nullptr) {}
    UiStringT(const UiStringT& str) : m_pData(nullptr)
    {
        if (!str.empty()) {
            size_t strSize = StringUtil::StringLen(str.c_str());
            m_pData = new value_type[strSize + 1];
            StringUtil::StringCopy(m_pData, strSize + 1, str.c_str());
        }
    }
    UiStringT(const string_type& str) : m_pData(nullptr)
    {
        if (!str.empty()) {
            size_t strSize = StringUtil::StringLen(str.c_str());
            m_pData = new value_type[strSize + 1];
            StringUtil::StringCopy(m_pData, strSize + 1, str.c_str());
        }
    }
    UiStringT(const std::basic_string_view<value_type>& str) : m_pData(nullptr)
    {
        if (!str.empty()) {
            size_t strSize = str.size();
            m_pData = new value_type[strSize + 1];
            StringUtil::StringCopy(m_pData, strSize + 1, str.data());
        }
    }
    UiStringT(const value_type* pstr) : m_pData(nullptr)
    {
        if (pstr != nullptr) {
            std::basic_string_view<value_type> str(pstr);
            if (!str.empty()) {
                size_t strSize = str.size();
                m_pData = new value_type[strSize + 1];
                StringUtil::StringCopy(m_pData, strSize + 1, str.data());
            }
        }
    }
    ~UiStringT()
    { 
        if (m_pData != nullptr) {
            delete[] m_pData;
            m_pData = nullptr;
        }
    }
public:

    bool empty() const { return (m_pData == nullptr) || (m_pData[0] == '\0'); }
    const value_type* data() const { return c_str(); }
    const value_type* c_str() const;
    void clear() 
    {
        if (m_pData != nullptr) {
            delete[] m_pData;
            m_pData = nullptr;
        }
    }

    UiStringT& operator=(const UiStringT& str)
    {
        if (&str == this) {
            return *this;
        }
        if (m_pData != nullptr) {
            delete[] m_pData;
            m_pData = nullptr;
        }
        if (!str.empty()) {
            size_t strSize = StringUtil::StringLen(str.c_str());
            m_pData = new value_type[strSize + 1];
            StringUtil::StringCopy(m_pData, strSize + 1, str.c_str());
        }
        return *this;
    }

    UiStringT& operator=(const string_type& str)
    {
        if (m_pData != nullptr) {
            delete[] m_pData;
            m_pData = nullptr;
        }
        if (!str.empty()) {
            size_t strSize = str.size();
            m_pData = new value_type[strSize + 1];
            StringUtil::StringCopy(m_pData, strSize + 1, str.c_str());
        }
        return *this;
    }

    UiStringT& operator=(const std::basic_string_view<value_type>& str)
    {
        if (m_pData != nullptr) {
            delete[] m_pData;;
            m_pData = nullptr;
        }
        if (!str.empty()) {
            size_t strSize = str.size();
            m_pData = new value_type[strSize + 1];
            StringUtil::StringNCopy(m_pData, strSize + 1, str.data(), str.size());
        }
        return *this;
    }

    UiStringT& operator=(const value_type* pstr)
    {
        if (m_pData != nullptr) {
            delete[] m_pData;;
            m_pData = nullptr;
        }
        if (pstr != nullptr) {
            std::basic_string_view<value_type> str(pstr);
            if (!str.empty()) {
                size_t strSize = str.size();
                m_pData = new value_type[strSize + 1];
                StringUtil::StringNCopy(m_pData, strSize + 1, str.data(), str.size());
            }
        }
        return *this;
    }

    bool equals(const string_type& str) const
    {
        if (str.empty()) {
            return empty();
        }
        else {
            if (m_pData == nullptr) {
                return false;
            }
            else {
                return StringUtil::StringCompare(m_pData, str.c_str()) == 0;
            }
        }
    }

    bool equals(const value_type* str) const
    {
        if ((str == nullptr) || (str[0] == '\0')) {
            return empty();
        }
        else {
            if (m_pData == nullptr) {
                return false;
            }
            else {
                return StringUtil::StringCompare(m_pData, str) == 0;
            }
        }
    }

    bool equals(const UiStringT& str) const
    {
        if (str.empty()) {
            return empty();
        }
        else {
            if (m_pData == nullptr) {
                return false;
            }
            else {
                return StringUtil::StringCompare(m_pData, str.c_str()) == 0;
            }
        }
    }

    friend bool operator==(const UiStringT& a, const UiStringT& b) {
        return a.equals(b);
    }
    friend bool operator==(const UiStringT& a, const string_type& b) {
        return a.equals(b);
    }
    friend bool operator==(const string_type& a, const UiStringT& b) {
        return b.equals(a);
    }
    friend bool operator==(const UiStringT& a, const value_type* b) {
        return a.equals(b);
    }
    friend bool operator==(const value_type* a, const UiStringT& b) {
        return b.equals(a);
    }
    friend bool operator!=(const UiStringT& a, const UiStringT& b) {
        return !a.equals(b);
    }
    friend bool operator!=(const UiStringT& a, const string_type& b) {
        return !a.equals(b);
    }
    friend bool operator!=(const string_type& a, const UiStringT& b) {
        return !b.equals(a);
    }
    friend bool operator!=(const UiStringT& a, const value_type* b) {
        return !a.equals(b);
    }
    friend bool operator!=(const value_type* a, const UiStringT& b) {
        return !b.equals(a);
    }
private:
    //字符串数据
    value_type* m_pData;
};

template <>
inline const DStringW::value_type* UiStringT<DStringW::value_type>::c_str() const
{
    return (m_pData != nullptr) ? m_pData : L"";
}

template <>
inline const DStringA::value_type* UiStringT<DStringA::value_type>::c_str() const
{
    return (m_pData != nullptr) ? m_pData : "";
}

/** 模板类型定义
*/
typedef UiStringT<DString::value_type> UiString;
typedef UiStringT<DStringA::value_type> UiStringA;
typedef UiStringT<DStringW::value_type> UiStringW;

}//namespace ui

#endif // UI_CORE_UISTRING_H_
