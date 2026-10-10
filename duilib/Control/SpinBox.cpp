#include "SpinBox.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

SpinBox::SpinBox(Window* pWindow) :
    BaseClass(pWindow)
{
    SetNumberOnly(true);
}

DString SpinBox::GetType() const
{
    return DUI_CTR_SPIN_BOX;
}

void SpinBox::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("step")) {
        SetStep(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("value")) {
        SetValue(StringUtil::StringToInt64(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void SpinBox::SetStep(int32_t nStep)
{
    SetSpinStep(nStep);
}

int32_t SpinBox::GetStep() const
{
    return GetSpinStep();
}

void SpinBox::SetValue(int64_t nValue)
{
    if ((GetMinNumber() != INT64_MIN) || (GetMaxNumber() != INT64_MAX)) {
        if (nValue > GetMaxNumber()) {
            nValue = GetMaxNumber();
        }
        else if (nValue < GetMinNumber()) {
            nValue = GetMinNumber();
        }
    }
    SetTextNumber(nValue);
}

int64_t SpinBox::GetValue() const
{
    return GetTextNumber();
}

void SpinBox::SetRange(int64_t nMin, int64_t nMax)
{
    ASSERT(nMin <= nMax);
    SetMinNumber(nMin);
    SetMaxNumber(nMax);
}

} // namespace ui
