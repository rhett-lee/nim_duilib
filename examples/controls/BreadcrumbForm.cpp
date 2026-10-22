#include "BreadcrumbForm.h"

BreadcrumbForm::BreadcrumbForm()
{
}

BreadcrumbForm::~BreadcrumbForm()
{
}

DString BreadcrumbForm::GetSkinFolder()
{
    return _T("controls");
}

DString BreadcrumbForm::GetSkinFile()
{
    return _T("breadcrumb.xml");
}

void BreadcrumbForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    ui::Breadcrumb* pBreadcrumb = dynamic_cast<ui::Breadcrumb*>(FindControl(_T("breadcrumb_demo")));
    if (pBreadcrumb != nullptr) {
        //初始回显
        UpdateCurrentText(pBreadcrumb);

        //当前项变化事件：索引通过 pEventData 携带（BreadcrumbValue）
        pBreadcrumb->AttachCurrentChanged([this](const ui::EventArgs& args) {
            ui::Breadcrumb* p = dynamic_cast<ui::Breadcrumb*>(args.GetSender());
            if (p != nullptr) {
                UpdateCurrentText(p);
            }
            return true;
            });
    }

    //上一级
    ui::Button* pUpBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_bc_up")));
    if ((pUpBtn != nullptr) && (pBreadcrumb != nullptr)) {
        pUpBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Breadcrumb* p = dynamic_cast<ui::Breadcrumb*>(FindControl(_T("breadcrumb_demo")));
            if ((p != nullptr) && (p->GetCurrentIndex() > 0)) {
                p->GoTo(p->GetCurrentIndex() - 1);
            }
            return true;
            });
    }

    //跳到首页
    ui::Button* pRootBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_bc_root")));
    if ((pRootBtn != nullptr) && (pBreadcrumb != nullptr)) {
        pRootBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Breadcrumb* p = dynamic_cast<ui::Breadcrumb*>(FindControl(_T("breadcrumb_demo")));
            if (p != nullptr) {
                p->GoTo(0);
            }
            return true;
            });
    }

    //追加一项
    ui::Button* pAddBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_bc_add")));
    if ((pAddBtn != nullptr) && (pBreadcrumb != nullptr)) {
        pAddBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Breadcrumb* p = dynamic_cast<ui::Breadcrumb*>(FindControl(_T("breadcrumb_demo")));
            if (p != nullptr) {
                const DString strNewItem = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_BREADCRUMB_NEW_ITEM"));
                p->AddItem(strNewItem);
                UpdateCurrentText(p);
            }
            return true;
            });
    }

    //删除末项
    ui::Button* pRemoveBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_bc_remove")));
    if ((pRemoveBtn != nullptr) && (pBreadcrumb != nullptr)) {
        pRemoveBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Breadcrumb* p = dynamic_cast<ui::Breadcrumb*>(FindControl(_T("breadcrumb_demo")));
            if ((p != nullptr) && (p->GetItemCount() > 1)) {
                p->RemoveItemAt(p->GetItemCount() - 1);
                UpdateCurrentText(p);
            }
            return true;
            });
    }
}

void BreadcrumbForm::UpdateCurrentText(ui::Breadcrumb* pBreadcrumb)
{
    if (pBreadcrumb == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("breadcrumb_event")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_BREADCRUMB_EVENT_PREFIX"));
    pLabel->SetText(strPrefix + ui::StringUtil::Printf(_T("%d / %d"),
                   static_cast<int>(pBreadcrumb->GetCurrentIndex() + 1),
                   static_cast<int>(pBreadcrumb->GetItemCount())));
}
