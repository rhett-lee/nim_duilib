#include "SpinBox.h"
#include "duilib/Utils/StringUtil.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ui
{

SpinBox::SpinBox(Window* pWindow) :
    BaseClass(pWindow),
    m_decimals(0),
    m_step(1.0),
    m_value(0.0),
    m_minValue(std::numeric_limits<double>::lowest()),
    m_maxValue(std::numeric_limits<double>::max()),
    m_bCustomLimitChars(false)
{
    //步进按钮回调传入的增量只用于判断方向，固定为1即可
    SetSpinStep(1);
    UpdateInputFilter();
}

DString SpinBox::GetType() const
{
    return DUI_CTR_SPIN_BOX;
}

void SpinBox::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("step")) {
        SetStep(StringUtil::StringToDouble(strValue));
    }
    else if (strName == _T("value")) {
        SetValue(StringUtil::StringToDouble(strValue));
    }
    else if (strName == _T("decimals")) {
        SetDecimals(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("prefix")) {
        SetPrefix(strValue);
    }
    else if (strName == _T("suffix")) {
        SetSuffix(strValue);
    }
    else if ((strName == _T("group_separator")) || (strName == _T("thousand_separator"))) {
        SetGroupSeparator(strValue);
    }
    else if ((strName == _T("min_number")) || (strName == _T("min_value"))) {
        SetMinValue(StringUtil::StringToDouble(strValue));
    }
    else if ((strName == _T("max_number")) || (strName == _T("max_value"))) {
        SetMaxValue(StringUtil::StringToDouble(strValue));
    }
    else if (strName == _T("limit_chars")) {
        //调用方自行指定输入字符集时，不再自动覆盖
        m_bCustomLimitChars = true;
        BaseClass::SetAttribute(strName, strValue);
        UpdateInputFilter();
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void SpinBox::AdjustTextNumber(int32_t nDelta)
{
    if ((nDelta == 0) || !IsEnabled() || IsReadOnly()) {
        return;
    }
    AdjustValue((nDelta > 0) ? m_step : -m_step);
}

void SpinBox::SetStep(int32_t nStep)
{
    SetStep(static_cast<double>(nStep));
}

void SpinBox::SetStep(double dStep)
{
    ASSERT(dStep > 0);
    if (dStep <= 0) {
        return;
    }
    m_step = dStep;
    //步进按钮回调传入的增量只用于判断方向；整数步长时保持 GetSpinStep 的可见行为
    const double dRoundedStep = std::round(dStep);
    const int32_t nSpinStep = ((dRoundedStep >= 1.0) && (dRoundedStep <= 2147483647.0)) ?
        static_cast<int32_t>(dRoundedStep) : 1;
    SetSpinStep(nSpinStep);
}

int32_t SpinBox::GetStep() const
{
    return static_cast<int32_t>(std::llround(m_step));
}

double SpinBox::GetStepDouble() const
{
    return m_step;
}

void SpinBox::SetValue(int64_t nValue)
{
    SetValue(static_cast<double>(nValue));
}

void SpinBox::SetValue(double dValue)
{
    if (!std::isfinite(dValue)) {
        ASSERT(false);
        return;
    }
    dValue = ClampValue(dValue);
    dValue = RoundValue(dValue);
    if (dValue == 0.0) {
        //消除 -0.0
        dValue = 0.0;
    }
    m_value = dValue;

    const DString text = FormatValue(dValue);
    if (GetText() != text) {
        SetText(text);
    }
}

int64_t SpinBox::GetValue() const
{
    return static_cast<int64_t>(std::llround(GetValueDouble()));
}

double SpinBox::GetValueDouble() const
{
    double dValue = m_value;
    if (TryParseValue(GetText(), dValue)) {
        return ClampValue(dValue);
    }
    return m_value;
}

void SpinBox::SetRange(int64_t nMin, int64_t nMax)
{
    SetRange(static_cast<double>(nMin), static_cast<double>(nMax));
}

void SpinBox::SetRange(double dMin, double dMax)
{
    ASSERT(dMin <= dMax);
    if (dMin > dMax) {
        std::swap(dMin, dMax);
    }
    m_minValue = dMin;
    m_maxValue = dMax;
    if (!GetText().empty()) {
        SetValue(m_value);
    }
}

void SpinBox::SetMinValue(double dMin)
{
    m_minValue = dMin;
    if (m_maxValue < m_minValue) {
        m_maxValue = m_minValue;
    }
    if (!GetText().empty()) {
        SetValue(m_value);
    }
}

double SpinBox::GetMinValue() const
{
    return m_minValue;
}

void SpinBox::SetMaxValue(double dMax)
{
    m_maxValue = dMax;
    if (m_minValue > m_maxValue) {
        m_minValue = m_maxValue;
    }
    if (!GetText().empty()) {
        SetValue(m_value);
    }
}

double SpinBox::GetMaxValue() const
{
    return m_maxValue;
}

void SpinBox::SetDecimals(int32_t nDecimals)
{
    if (nDecimals < 0) {
        nDecimals = 0;
    }
    else if (nDecimals > 10) {
        nDecimals = 10;
    }
    if (m_decimals == nDecimals) {
        return;
    }
    m_decimals = nDecimals;
    UpdateInputFilter();
    if (!GetText().empty()) {
        SetValue(m_value);
    }
}

int32_t SpinBox::GetDecimals() const
{
    return m_decimals;
}

void SpinBox::SetPrefix(const DString& prefix)
{
    if (m_prefix == prefix) {
        return;
    }
    m_prefix = prefix;
    UpdateInputFilter();
    if (!GetText().empty()) {
        SetValue(m_value);
    }
}

DString SpinBox::GetPrefix() const
{
    return m_prefix;
}

void SpinBox::SetSuffix(const DString& suffix)
{
    if (m_suffix == suffix) {
        return;
    }
    m_suffix = suffix;
    UpdateInputFilter();
    if (!GetText().empty()) {
        SetValue(m_value);
    }
}

DString SpinBox::GetSuffix() const
{
    return m_suffix;
}

void SpinBox::SetGroupSeparator(const DString& separator)
{
    if (m_groupSeparator == separator) {
        return;
    }
    m_groupSeparator = separator;
    UpdateInputFilter();
    if (!GetText().empty()) {
        SetValue(m_value);
    }
}

DString SpinBox::GetGroupSeparator() const
{
    return m_groupSeparator;
}

bool SpinBox::OnKeyDown(const EventArgs& msg)
{
    if (!IsReadOnly() && IsEnabled() && ((msg.vkCode == kVK_UP) || (msg.vkCode == kVK_DOWN))) {
        AdjustValue((msg.vkCode == kVK_UP) ? m_step : -m_step);
        return true;
    }
    return BaseClass::OnKeyDown(msg);
}

bool SpinBox::OnKillFocus(const EventArgs& msg)
{
    const bool bRet = BaseClass::OnKillFocus(msg);
    NormalizeText();
    return bRet;
}

void SpinBox::UpdateInputFilter()
{
    //纯整数模式（无前后缀、无千分位）沿用 RichEdit 的 number_only 输入过滤
    const bool bIntegerMode = (m_decimals == 0) && m_groupSeparator.empty() &&
        m_prefix.empty() && m_suffix.empty();
    SetNumberOnly(bIntegerMode);

    if (m_bCustomLimitChars) {
        //调用方自行设置了 limit_chars，保持其设置
        return;
    }
    if (bIntegerMode) {
        SetLimitChars(DString());
        return;
    }

    //小数/格式化模式：允许数字、符号、小数点、千分位与前后缀中的字符
    DString limitChars = _T("0123456789+-");
    if (m_decimals > 0) {
        limitChars += _T('.');
    }
    limitChars += m_groupSeparator;
    limitChars += m_prefix;
    limitChars += m_suffix;
    SetLimitChars(limitChars);
}

void SpinBox::AdjustValue(double delta)
{
    SetValue(GetValueDouble() + delta);
}

void SpinBox::NormalizeText()
{
    const DString text = GetText();
    if (text.empty()) {
        //保持空文本，不强制填入 0
        return;
    }
    double dValue = m_value;
    if (TryParseValue(text, dValue)) {
        SetValue(dValue);
    }
    else {
        //无法解析时恢复最近一次有效值
        SetValue(m_value);
    }
}

bool SpinBox::TryParseValue(const DString& text, double& dValue) const
{
    DString strValue = text;
    StringUtil::Trim(strValue);

    if (!m_prefix.empty() && (strValue.compare(0, m_prefix.size(), m_prefix) == 0)) {
        strValue = strValue.substr(m_prefix.size());
    }
    if (!m_suffix.empty() && (strValue.size() >= m_suffix.size()) &&
        (strValue.compare(strValue.size() - m_suffix.size(), m_suffix.size(), m_suffix) == 0)) {
        strValue = strValue.substr(0, strValue.size() - m_suffix.size());
    }
    if (!m_groupSeparator.empty()) {
        StringUtil::ReplaceAll(m_groupSeparator, DString(), strValue);
    }
    StringUtil::Trim(strValue);
    if (strValue.empty()) {
        return false;
    }

    size_t index = 0;
    if ((strValue[0] == _T('+')) || (strValue[0] == _T('-'))) {
        index = 1;
    }
    bool bHasDigit = false;
    bool bHasDot = false;
    for (; index < strValue.size(); ++index) {
        const DString::value_type ch = strValue[index];
        if ((ch >= _T('0')) && (ch <= _T('9'))) {
            bHasDigit = true;
        }
        else if ((ch == _T('.')) && !bHasDot) {
            bHasDot = true;
        }
        else {
            return false;
        }
    }
    if (!bHasDigit) {
        return false;
    }

    const double dParsedValue = StringUtil::StringToDouble(strValue);
    if (!std::isfinite(dParsedValue)) {
        return false;
    }
    dValue = dParsedValue;
    return true;
}

DString SpinBox::FormatValue(double dValue) const
{
    DString number = StringUtil::Printf(_T("%.*f"), m_decimals, dValue);
    if (!m_groupSeparator.empty()) {
        const size_t dotPos = number.find(_T('.'));
        const size_t intEnd = (dotPos == DString::npos) ? number.size() : dotPos;
        size_t start = 0;
        if ((intEnd > 0) && ((number[0] == _T('-')) || (number[0] == _T('+')))) {
            start = 1;
        }
        if (intEnd > start) {
            const size_t digitCount = intEnd - start;
            DString grouped;
            grouped.reserve(digitCount + (digitCount / 3) * m_groupSeparator.size());
            for (size_t i = 0; i < digitCount; ++i) {
                if ((i > 0) && (((digitCount - i) % 3) == 0)) {
                    grouped += m_groupSeparator;
                }
                grouped += number[start + i];
            }
            DString result = number.substr(0, start);
            result += grouped;
            if (dotPos != DString::npos) {
                result += number.substr(dotPos);
            }
            number = result;
        }
    }
    return m_prefix + number + m_suffix;
}

double SpinBox::RoundValue(double dValue) const
{
    const double scale = std::pow(10.0, m_decimals);
    const double scaledValue = dValue * scale;
    if (!std::isfinite(scaledValue)) {
        return dValue;
    }
    return std::round(scaledValue) / scale;
}

double SpinBox::ClampValue(double dValue) const
{
    if (dValue < m_minValue) {
        return m_minValue;
    }
    if (dValue > m_maxValue) {
        return m_maxValue;
    }
    return dValue;
}

} // namespace ui
