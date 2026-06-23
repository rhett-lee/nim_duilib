#include "AttributeUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

void AttributeUtil::SkipSepChar(wchar_t*& pPtr)
{
    if ((pPtr != nullptr) && (*pPtr != L'\0')) {
        //跳过分隔字符，但避免跳过尾0，防止字符串越界
        ++pPtr;
    }
}

void AttributeUtil::SkipSepChar(char*& pPtr)
{
    if ((pPtr != nullptr) && (*pPtr != '\0')) {
        //跳过分隔字符，但避免跳过尾0，防止字符串越界
        ++pPtr;
    }
}

void AttributeUtil::ParseSizeValue(const wchar_t* strValue, UiSize& size)
{
    size.Clear();
    if ((strValue == nullptr) || (*strValue == L'\0')) {
        return;
    }
    wchar_t* pstr = nullptr;
    int32_t cx = StringUtil::StringToInt32(strValue, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    int32_t cy = StringUtil::StringToInt32(pstr, &pstr, 10);
    size.cx = cx;
    size.cy = cy;
}

void AttributeUtil::ParseSizeValue(const char* strValue, UiSize& size)
{
    size.Clear();
    if ((strValue == nullptr) || (*strValue == '\0')) {
        return;
    }
    char* pstr = nullptr;
    int32_t cx = StringUtil::StringToInt32(strValue, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    int32_t cy = StringUtil::StringToInt32(pstr, &pstr, 10);
    size.cx = cx;
    size.cy = cy;
}

void AttributeUtil::ParsePointValue(const wchar_t* strValue, UiPoint& pt)
{
    UiSize size;
    AttributeUtil::ParseSizeValue(strValue, size);
    pt.x = size.cx;
    pt.y = size.cy;
}

void AttributeUtil::ParsePointValue(const char* strValue, UiPoint& pt)
{
    UiSize size;
    AttributeUtil::ParseSizeValue(strValue, size);
    pt.x = size.cx;
    pt.y = size.cy;
}

void AttributeUtil::ParsePaddingValue(const wchar_t* strValue, UiPadding& padding)
{
    padding.Clear();
    if ((strValue == nullptr) || (*strValue == L'\0')) {
        return;
    }
    wchar_t* pstr = nullptr;
    padding.left = StringUtil::StringToInt32(strValue, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    padding.top = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    padding.right = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    padding.bottom = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    padding.Validate();
}

void AttributeUtil::ParsePaddingValue(const char* strValue, UiPadding& padding)
{
    padding.Clear();
    if ((strValue == nullptr) || (*strValue == '\0')) {
        return;
    }
    char* pstr = nullptr;
    padding.left = StringUtil::StringToInt32(strValue, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    padding.top = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    padding.right = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    padding.bottom = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    padding.Validate();
}

void AttributeUtil::ParseMarginValue(const wchar_t* strValue, UiMargin& margin)
{
    margin.Clear();
    if ((strValue == nullptr) || (*strValue == L'\0')) {
        return;
    }
    wchar_t* pstr = nullptr;
    margin.left = StringUtil::StringToInt32(strValue, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    margin.top = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    margin.right = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    margin.bottom = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    ASSERT((margin.left >= 0) && (margin.top >= 0) && (margin.right >= 0) && (margin.bottom >= 0));
    margin.Validate();
}

void AttributeUtil::ParseMarginValue(const char* strValue, UiMargin& margin)
{
    margin.Clear();
    if ((strValue == nullptr) || (*strValue == '\0')) {
        return;
    }
    char* pstr = nullptr;
    margin.left = StringUtil::StringToInt32(strValue, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    margin.top = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    margin.right = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    margin.bottom = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    ASSERT((margin.left >= 0) && (margin.top >= 0) && (margin.right >= 0) && (margin.bottom >= 0));
    margin.Validate();
}

void AttributeUtil::ParseRectValue(const wchar_t* strValue, UiRect& rect, bool bCheckSize)
{
    rect.Clear();
    if ((strValue == nullptr) || (*strValue == L'\0')) {
        return;
    }
    wchar_t* pstr = nullptr;
    rect.left = StringUtil::StringToInt32(strValue, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    rect.top = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    rect.right = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    rect.bottom = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    if (bCheckSize) {
        ASSERT((rect.Width() >= 0) && (rect.Height() >= 0));
    }    
}

void AttributeUtil::ParseRectValue(const char* strValue, UiRect& rect, bool bCheckSize)
{
    rect.Clear();
    if ((strValue == nullptr) || (*strValue == '\0')) {
        return;
    }
    char* pstr = nullptr;
    rect.left = StringUtil::StringToInt32(strValue, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    rect.top = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    rect.right = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    SkipSepChar(pstr);
    rect.bottom = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
    if (bCheckSize) {
        ASSERT((rect.Width() >= 0) && (rect.Height() >= 0));
    }
}

// 辅助函数：查找字符在字符串中的位置，返回下标（未找到返回-1）
static int32_t FindCharIndexInString(DString::value_type ch, const DString& str)
{
    DString::size_type pos = str.find(ch);
    return (pos != DString::npos) ? static_cast<int>(pos) : -1;
}

bool AttributeUtil::ParseAttributeList(const DString& strList, std::vector<std::pair<DString, DString>>& attributeList,
                                       const DString& seperateStartChars,
                                       const DString& seperateEndChars)
{
    // 清空输出列表，避免残留数据
    attributeList.clear();

    // 示例：normal_image={file='public/button/window-minimize.svg' width='24'} hovered_color="AliceBlue" pressed_color='Lavender'
    DString sName;
    DString sValue;
    const DString::value_type* pstrList = strList.c_str();

    // 失败场景1：空指针/空字符串
    if (pstrList == nullptr || *pstrList == _T('\0')) {
        return false;
    }

    // 失败场景2：起始分隔符为空
    ASSERT(!seperateStartChars.empty());
    if (seperateStartChars.empty()) {
        return false;
    }

    // 失败场景3：起始/结束分隔符长度不匹配
    ASSERT(!seperateEndChars.empty() && (seperateStartChars.size() == seperateEndChars.size()));
    if (seperateEndChars.empty() || seperateStartChars.size() != seperateEndChars.size()) {
        return false;
    }

    // 标记是否至少解析出一个有效属性（避免空解析返回true）
    bool hasValidAttribute = false;

    while (*pstrList != _T('\0')) {
        sName.clear();
        sValue.clear();

        // 1. 读取等号前面的内容，作为属性名
        while (*pstrList != _T('\0') && *pstrList != _T('=')) {
            sName += *pstrList++;
        }

        // 失败场景4：遍历完未找到等号（属性名后无等号）
        if (*pstrList != _T('=')) {
            // 如果已解析出有效属性，返回true；否则返回false
            return hasValidAttribute;
        }

        // 2. 跳到等号后，检查起始分隔符
        pstrList++; // 跳过等号
        // 失败场景5：等号后无内容
        if (*pstrList == _T('\0')) {
            return hasValidAttribute;
        }

        // 3. 查找当前字符是否是支持的起始分隔符，并获取对应结束分隔符
        int32_t sepIndex = FindCharIndexInString(*pstrList, seperateStartChars);
        ASSERT(sepIndex != -1);
        // 失败场景6：不是合法起始分隔符
        if (sepIndex == -1) {
            return hasValidAttribute;
        }
        DString::value_type endSepChar = seperateEndChars[sepIndex];

        // 4. 跳到起始分隔符后，读取属性值（直到遇到对应的结束分隔符）
        pstrList++; // 跳过起始分隔符
        while (*pstrList != _T('\0') && *pstrList != endSepChar) {
            sValue += *pstrList++;
        }

        // 失败场景7：未找到结束分隔符
        ASSERT(*pstrList == endSepChar);
        if (*pstrList != endSepChar) {
            return hasValidAttribute;
        }

        // 5. 处理属性名（去除前后空白），添加到结果列表
        sName = StringUtil::Trim(sName);
        if (!sName.empty()) { // 过滤空属性名
            attributeList.push_back(std::make_pair(sName, sValue));
            hasValidAttribute = true; // 标记有有效属性
        }

        // 6. 跳到结束分隔符后，处理后续字符
        pstrList++; // 跳过结束分隔符

        // 7. 跳过后续的所有空白字符（兼容多空格场景）
        while (*pstrList != _T('\0') && *pstrList == _T(' ')) {
            pstrList++;
        }
    }

    // 正常遍历完成：有有效属性返回true，无则返回false
    return hasValidAttribute;
}

std::tuple<int32_t, float> AttributeUtil::ParseString(const wchar_t* strValue, wchar_t** pEndPtr)
{
    wchar_t* pstr = nullptr;
    if ((strValue == nullptr) || (*strValue == L'\0')) {
        if (pEndPtr != nullptr) {
            *pEndPtr = pstr;
        }
        return std::tuple<int32_t, float>(0, 0.0f);
    }
    int32_t xValue = 0;
    float xPercent = StringUtil::StringToFloat(strValue, &pstr);
    ASSERT(pstr != nullptr);
    if ((pstr != nullptr) && (*pstr == L'%')) {
        //该值是百分比，跳过'%'字符
        pstr++;
    }
    else {
        //不是百分比, 而是整型值
        xPercent = 0.0f;
        xValue = StringUtil::StringToInt32(strValue, &pstr, 10);
        ASSERT(pstr != nullptr);
    }
    if (pEndPtr != nullptr) {
        *pEndPtr = pstr;
    }
    return std::tuple<int32_t, float>(xValue, xPercent);
}

std::tuple<int32_t, float> AttributeUtil::ParseString(const char* strValue, char** pEndPtr)
{
    char* pstr = nullptr;
    if ((strValue == nullptr) || (*strValue == '\0')) {
        if (pEndPtr != nullptr) {
            *pEndPtr = pstr;
        }
        return std::tuple<int32_t, float>(0, 0.0f);
    }
    int32_t xValue = 0;
    float xPercent = StringUtil::StringToFloat(strValue, &pstr);
    ASSERT(pstr != nullptr);
    if ((pstr != nullptr) && (*pstr == '%')) {
        //该值是百分比，跳过'%'字符
        pstr++;
    }
    else {
        //不是百分比, 而是整型值
        xPercent = 0.0f;
        xValue = StringUtil::StringToInt32(strValue, &pstr, 10);
        ASSERT(pstr != nullptr);
    }
    if (pEndPtr != nullptr) {
        *pEndPtr = pstr;
    }
    return std::tuple<int32_t, float>(xValue, xPercent);
}

void AttributeUtil::ParseWindowSize(const Window* pWindow, const DString::value_type* strValue,
                                    UiSize& size,
                                    bool* pScaledCX, bool* pScaledCY,
                                    bool* pPercentCX, bool* pPercentCY)
{
    //支持的格式：size="1200,800",或者size="50%,50%",或者size="1200,50%",size="50%,800"
    //百分比是指屏幕宽度或者高度的百分比
    UiRect rcWork;
    if (pWindow != nullptr) {
        pWindow->GetMonitorWorkRect(rcWork);
    }
    else {
        WindowBase::GetPrimaryMonitorWorkRect(rcWork);
    }
    
    DString::value_type* pstr = nullptr;
    std::tuple<int32_t, float> x = ParseString(strValue, &pstr);
    AttributeUtil::SkipSepChar(pstr);
    std::tuple<int32_t, float> y = ParseString(pstr, &pstr);

    //获取有效的百分比值
    auto GetValidPercent = [](const std::tuple<int32_t, float>& xy) -> float {
        float fPercent = std::get<1>(xy);
        fPercent /= 100.0f;
        ASSERT((fPercent >= 0.0001f) && (fPercent < 1.0001f));
        if ((fPercent < 0.0001f) || (fPercent > 1.0001f)) {
            //如果遇到无效百分比，则默认设置为窗口大小的75%
            fPercent = 0.75f;
        }
        return fPercent;
    };
    if (pPercentCX) {
        *pPercentCX = false;
    }
    if (pPercentCY) {
        *pPercentCY = false;
    }

    bool needScaleCX = true;
    bool needScaleCY = true;
    int cx = std::get<0>(x);
    if (cx <= 0) {
        float fPercent = GetValidPercent(x);
        cx = (int)(rcWork.Width() * fPercent);
        needScaleCX = false;
        if (pPercentCX) {
            *pPercentCX = true;
        }
    }
    int cy = std::get<0>(y);
    if (cy <= 0) {
        float fPercent = GetValidPercent(y);
        cy = (int)(rcWork.Height() * fPercent);
        needScaleCY = false;
        if (pPercentCY) {
            *pPercentCY = true;
        }
    }

    ASSERT((cx > 0) && (cy > 0));
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (needScaleCX && (pWindow != nullptr)) {
        pWindow->Dpi().ScaleWindowSize(cx);
    }
    if (needScaleCY && (pWindow != nullptr)) {
        pWindow->Dpi().ScaleWindowSize(cy);
    }
    if (!rcWork.IsEmpty()) {
        if (cx > rcWork.Width()) {
            cx = rcWork.Width();
        }
        if (cy > rcWork.Height()) {
            cy = rcWork.Height();
        }
    }
    size.cx = cx;
    size.cy = cy;
    if (pScaledCX) {
        *pScaledCX = (pWindow != nullptr) ? true : !needScaleCX;
    }
    if (pScaledCY) {
        *pScaledCY = (pWindow != nullptr) ? true : !needScaleCY;
    }
}

void AttributeUtil::ValidateWindowSize(const Window* pWindow, int32_t& nWindowWidth, int32_t& nWindowHeight)
{
    UiRect rcWork;
    if (pWindow != nullptr) {
        pWindow->GetMonitorWorkRect(rcWork);
    }
    else {
        WindowBase::GetPrimaryMonitorWorkRect(rcWork);
    }
    //如果超过宽度，就按屏幕的95%来确定大小（避免占满整个屏幕，导致该状态与最大化状态无法区分）
    if (nWindowWidth > rcWork.Width()) {
        nWindowWidth = (int32_t)(rcWork.Width() * 0.95);
    }
    if (nWindowHeight > rcWork.Height()) {
        nWindowHeight = (int32_t)(rcWork.Height() * 0.95);
    }
}

}//namespace ui
