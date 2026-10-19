#include "CarouselForm.h"
#include "duilib/Control/Carousel.h"
#include "duilib/Control/Pagination.h"

CarouselForm::CarouselForm()
{
}

CarouselForm::~CarouselForm()
{
}

DString CarouselForm::GetSkinFolder()
{
    return _T("controls");
}

DString CarouselForm::GetSkinFile()
{
    return _T("carousel.xml");
}

void CarouselForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //自动轮播：索引变化事件（新索引/旧索引通过 pEventData 携带，CarouselValue）
    ui::Carousel* pAuto = dynamic_cast<ui::Carousel*>(FindControl(_T("carousel_auto")));
    if (pAuto != nullptr) {
        UpdateCarouselText(pAuto);
        pAuto->AttachCarouselChanged([this](const ui::EventArgs& args) {
            ui::Carousel* p = dynamic_cast<ui::Carousel*>(args.GetSender());
            if (p != nullptr) {
                UpdateCarouselText(p);
            }
            return true;
            });
    }

    //手动轮播：上一张 / 下一张 / 回到第一张
    ui::Carousel* pManual = dynamic_cast<ui::Carousel*>(FindControl(_T("carousel_manual")));
    if (pManual != nullptr) {
        UpdateCarouselText(pManual);
        pManual->AttachCarouselChanged([this](const ui::EventArgs& args) {
            ui::Carousel* p = dynamic_cast<ui::Carousel*>(args.GetSender());
            if (p != nullptr) {
                UpdateCarouselText(p);
            }
            return true;
            });
    }
    ui::Button* pPrevBtn = dynamic_cast<ui::Button*>(FindControl(_T("carousel_prev")));
    if (pPrevBtn != nullptr) {
        pPrevBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (ui::Carousel* p = dynamic_cast<ui::Carousel*>(FindControl(_T("carousel_manual")))) {
                p->Prev();
            }
            return true;
            });
    }
    ui::Button* pNextBtn = dynamic_cast<ui::Button*>(FindControl(_T("carousel_next")));
    if (pNextBtn != nullptr) {
        pNextBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (ui::Carousel* p = dynamic_cast<ui::Carousel*>(FindControl(_T("carousel_manual")))) {
                p->Next();
            }
            return true;
            });
    }
    ui::Button* pGotoFirstBtn = dynamic_cast<ui::Button*>(FindControl(_T("carousel_goto_first")));
    if (pGotoFirstBtn != nullptr) {
        pGotoFirstBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (ui::Carousel* p = dynamic_cast<ui::Carousel*>(FindControl(_T("carousel_manual")))) {
                p->GoTo(0);
            }
            return true;
            });
    }

    //分页器：页码变化事件（新页码/旧页码通过 pEventData 携带，PaginationValue）
    ui::Pagination* pPagination = dynamic_cast<ui::Pagination*>(FindControl(_T("pagination_basic")));
    if (pPagination != nullptr) {
        UpdatePaginationText(pPagination);
        pPagination->AttachPageChanged([this](const ui::EventArgs& args) {
            ui::Pagination* p = dynamic_cast<ui::Pagination*>(args.GetSender());
            if (p != nullptr) {
                UpdatePaginationText(p);
            }
            return true;
            });
    }
    ui::Button* pPagePrevBtn = dynamic_cast<ui::Button*>(FindControl(_T("pagination_prev")));
    if (pPagePrevBtn != nullptr) {
        pPagePrevBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (ui::Pagination* p = dynamic_cast<ui::Pagination*>(FindControl(_T("pagination_basic")))) {
                p->Prev();
            }
            return true;
            });
    }
    ui::Button* pPageNextBtn = dynamic_cast<ui::Button*>(FindControl(_T("pagination_next")));
    if (pPageNextBtn != nullptr) {
        pPageNextBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (ui::Pagination* p = dynamic_cast<ui::Pagination*>(FindControl(_T("pagination_basic")))) {
                p->Next();
            }
            return true;
            });
    }
}

void CarouselForm::UpdateCarouselText(ui::Carousel* pCarousel)
{
    if (pCarousel == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("carousel_event")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CAROUSEL_EVENT_PREFIX"));
    pLabel->SetText(strPrefix + ui::StringUtil::Printf(_T("%d / %d"),
                   pCarousel->GetCurrentIndex(), pCarousel->GetItemCount()));
}

void CarouselForm::UpdatePaginationText(ui::Pagination* pPagination)
{
    if (pPagination == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("pagination_event")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_PAGINATION_EVENT_PREFIX"));
    pLabel->SetText(strPrefix + ui::StringUtil::Printf(_T("%d / %d"),
                   pPagination->GetCurrentPage(), pPagination->GetPageCount()));
}
