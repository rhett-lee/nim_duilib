#ifndef UI_CORE_UISTRING_H_
#define UI_CORE_UISTRING_H_

#include "duilib/Utils/StringUtil.h"

namespace ui
{

/** 控件使用的字符串，用于替代DString，以减少控件的内存占用
*   该类适合用于较低的内存空间来存储字符串，性能方面由于字符串复制偏多，性能偏弱
*/
class UILIB_API UiString
{
public:
    UiString(): m_pData(nullptr) {}
    UiString(const UiString& str) : m_pData(nullptr)
    {
        if (!str.empty()) {
            size_t strSize = StringUtil::StringLen(str.c_str());
            m_pData = new DString::value_type[strSize + 1];
            StringUtil::StringCopy(m_pData, strSize + 1, str.c_str());
        }
    }
    UiString& operator=(const UiString& str)
    {
        if(&str == this){
            return *this;
        }
        if (m_pData != nullptr) {
            delete m_pData;
            m_pData = nullptr;
        }
        if (!str.empty()) {
            size_t strSize = StringUtil::StringLen(str.c_str());
            m_pData = new DString::value_type[strSize + 1];
            StringUtil::StringCopy(m_pData, strSize + 1, str.c_str());
        }
        return *this;
    }
    ~UiString()
    { 
        if (m_pData != nullptr) {
            delete m_pData;
            m_pData = nullptr;
        }
    }
public:

    bool empty() const { return (m_pData == nullptr) || (m_pData[0] == _T('\0')); }
    const DString::value_type* data() const { return c_str(); }
    const DString::value_type* c_str() const { return (m_pData != nullptr) ? m_pData : _T(""); }
    void clear() 
    {
        if (m_pData != nullptr) {
            delete m_pData;
            m_pData = nullptr;
        }
    }

    UiString& operator=(const DString& str)
    {
        if (m_pData != nullptr) {
            delete m_pData;
            m_pData = nullptr;
        }
        if (!str.empty()) {
            size_t strSize = str.size();
            m_pData = new DString::value_type[strSize + 1];
            StringUtil::StringCopy(m_pData, strSize + 1, str.c_str());
        }
        return *this;
    }

    bool equals(const DString& str) const
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

    bool equals(const UiString& str) const
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

    friend bool operator==(const UiString& a, const UiString& b) {
        return a.equals(b);
    }
    friend bool operator==(const UiString& a, const DString& b) {
        return a.equals(b);
    }
    friend bool operator==(const DString& a, const UiString& b) {
        return b.equals(a);
    }
    friend bool operator!=(const UiString& a, const UiString& b) {
        return !a.equals(b);
    }
    friend bool operator!=(const UiString& a, const DString& b) {
        return !a.equals(b);
    }
    friend bool operator!=(const DString& a, const UiString& b) {
        return !b.equals(a);
    }
private:
    //字符串数据
    DString::value_type* m_pData;
};

}//namespace ui

#endif // UI_CORE_UISTRING_H_
