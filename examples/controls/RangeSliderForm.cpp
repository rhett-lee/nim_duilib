#include "RangeSliderForm.h"

RangeSliderForm::RangeSliderForm()
{
}

RangeSliderForm::~RangeSliderForm()
{
}

DString RangeSliderForm::GetSkinFolder()
{
    return _T("controls");
}

DString RangeSliderForm::GetSkinFile()
{
    return _T("rangeslider.xml");
}

void RangeSliderForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    ui::RangeSlider* pRange = dynamic_cast<ui::RangeSlider*>(FindControl(_T("price_range")));
    if (pRange != nullptr) {
        //初始回显
        UpdateRangeText(pRange);

        //范围变化事件：下限与上限通过 pEventData 携带（RangeValue）
        pRange->AttachRangeChanged([this](const ui::EventArgs& args) {
            ui::RangeSlider* pSlider = dynamic_cast<ui::RangeSlider*>(args.GetSender());
            if (pSlider != nullptr) {
                UpdateRangeText(pSlider);
            }
            return true;
            });
    }

    //重置区间：恢复到 200 ~ 800
    ui::Button* pResetBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_range_reset")));
    if ((pResetBtn != nullptr) && (pRange != nullptr)) {
        pResetBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::RangeSlider* pSlider = dynamic_cast<ui::RangeSlider*>(FindControl(_T("price_range")));
            if (pSlider != nullptr) {
                pSlider->SetRange(200, 800);
            }
            return true;
            });
    }

    //全选区间：下限取 min，上限取 max
    ui::Button* pFullBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_range_full")));
    if ((pFullBtn != nullptr) && (pRange != nullptr)) {
        pFullBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::RangeSlider* pSlider = dynamic_cast<ui::RangeSlider*>(FindControl(_T("price_range")));
            if (pSlider != nullptr) {
                pSlider->SetRange((double)pSlider->GetMinValue(), (double)pSlider->GetMaxValue());
            }
            return true;
            });
    }
}

void RangeSliderForm::UpdateRangeText(ui::RangeSlider* pRange)
{
    if (pRange == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("rs_event")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_RANGESLIDER_EVENT_PREFIX"));
    pLabel->SetText(strPrefix + ui::StringUtil::Printf(_T("%.0f ~ %.0f"), pRange->GetLowValue(), pRange->GetHighValue()));
}
