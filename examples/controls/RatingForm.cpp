#include "RatingForm.h"
#include "duilib/Control/Rating.h"

RatingForm::RatingForm()
{
}

RatingForm::~RatingForm()
{
}

DString RatingForm::GetSkinFolder()
{
    return _T("controls");
}

DString RatingForm::GetSkinFile()
{
    return _T("rating.xml");
}

void RatingForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //交互打分：值变化事件（新值/旧值通过 pEventData 携带，RatingValue）
    ui::Rating* pRating = dynamic_cast<ui::Rating*>(FindControl(_T("rating_demo")));
    if (pRating != nullptr) {
        UpdateRatingText(pRating);
        pRating->AttachRatingChanged([this](const ui::EventArgs& args) {
            ui::Rating* p = dynamic_cast<ui::Rating*>(args.GetSender());
            if (p != nullptr) {
                UpdateRatingText(p);
            }
            return true;
            });
    }

    //清零
    ui::Button* pClearBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rating_clear")));
    if ((pClearBtn != nullptr) && (pRating != nullptr)) {
        pClearBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Rating* p = dynamic_cast<ui::Rating*>(FindControl(_T("rating_demo")));
            if (p != nullptr) {
                p->SetValue(0);
            }
            return true;
            });
    }

    //设为 4.5（对整数模式的 rating_demo 会按步长对齐为 5）
    ui::Button* pSet45Btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rating_set45")));
    if (pSet45Btn != nullptr) {
        pSet45Btn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Rating* pDemo = dynamic_cast<ui::Rating*>(FindControl(_T("rating_demo")));
            if (pDemo != nullptr) {
                pDemo->SetValue(4.5);
            }
            ui::Rating* pHalf = dynamic_cast<ui::Rating*>(FindControl(_T("rating_half")));
            if (pHalf != nullptr) {
                pHalf->SetValue(4.5);
            }
            return true;
            });
    }
}

void RatingForm::UpdateRatingText(ui::Rating* pRating)
{
    if (pRating == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("rating_event")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_RATING_EVENT_PREFIX"));
    pLabel->SetText(strPrefix + ui::StringUtil::Printf(_T("%g / %d"),
                   pRating->GetValue(), pRating->GetMax()));
}
