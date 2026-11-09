#include "DateTimeForm.h"

namespace
{
/** 读取语言资源（代码动态生成的文本必须走语言文件，才能支持中英文切换） */
DString DateTimeLang(const DString& strId)
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}
}//namespace

DateTimeForm::DateTimeForm()
{
}

DateTimeForm::~DateTimeForm()
{
}

DString DateTimeForm::GetSkinFolder()
{
    return _T("controls");
}

DString DateTimeForm::GetSkinFile()
{
    return _T("datetime.xml");
}

void DateTimeForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    const DString controlNames[] = {
        _T("dt_date_up_down"),
        _T("dt_date_time_up_down"),
        _T("dt_minute_up_down"),
        _T("dt_date_calendar"),
        _T("dt_time_calendar"),
        _T("dt_date_time_calendar"),
        _T("dt_limited"),
        _T("dt_disabled"),
        _T("dt_now"),
    };
    for (const DString& name : controlNames) {
        AttachValueChanged(name);
    }
}

void DateTimeForm::AttachValueChanged(const DString& controlName)
{
    ui::DateTime* pDateTime = dynamic_cast<ui::DateTime*>(FindControl(controlName));
    if (pDateTime == nullptr) {
        return;
    }
    pDateTime->AttachEvent(ui::kEventValueChanged, [this, pDateTime, controlName](const ui::EventArgs& /*args*/) {
        UpdateEventText(ui::StringUtil::Printf(DateTimeLang(_T("STRID_DATETIME_EVENT_VALUE")).c_str(),
                                               controlName.c_str(), pDateTime->GetText().c_str()));
        return true;
        }, 0);
}

bool DateTimeForm::OnLanguageChanged()
{
    const bool bRet = BaseClass::OnLanguageChanged();
    UpdateEventText(DateTimeLang(_T("STRID_DATETIME_EVENT_DEFAULT")));
    return bRet;
}

void DateTimeForm::UpdateEventText(const DString& text)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("datetime_event_label")));
    if (pLabel != nullptr) {
        pLabel->SetText(text);
    }
}
