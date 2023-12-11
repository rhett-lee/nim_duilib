#include "popover.h"
#include "notification.h"
#include "alert.h"
#include "tooltip.h"

#include "duilib/Utils/Macros.h"
#include "duilib/Core/UiColor.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/GlobalManager.h"
#include "base/thread/thread_manager.h"
#include "ui_components/public_define.h"

#include <sstream>

namespace nim_comp {

const int PopoverArrow::kWidth = 12;
const int PopoverArrow::kHeight = 8;
const int PopoverArrow::kMargin = 36;

// PopoverArrow
PopoverArrow::PopoverArrow(int nPlacement) :
    m_nPlacement(nPlacement),
    m_rcArea({ 0,0,0,0 })
{
    ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        m_pPath.reset(pRenderFactory->CreatePath());
        m_pPen.reset(pRenderFactory->CreatePen(ui::UiColor(0xFFFFFFFF), 1));
        m_pBrush.reset(pRenderFactory->CreateBrush(ui::UiColor(0xFFFFFFFF)));
    }
}

PopoverArrow::~PopoverArrow()
{

}

std::wstring PopoverArrow::GetType() const { return L"PopoverArrow"; }

void PopoverArrow::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    SetBkColor(L"white");
    SetFixedWidth(ui::UiFixedInt(kWidth), true, true);
    SetFixedHeight(ui::UiFixedInt(kHeight), true, true);

    SetFloat(true);

    auto horAlignType = GetHorAlignType();
    auto verAlignType = GetVerAlignType();

    // center placement should not have arrow
    //ASSERT(!(m_nPlacement & kPlaceCenter));
    if (m_nPlacement & kPlaceCenter) {
        horAlignType = ui::kHorAlignCenter;
        verAlignType = ui::kVerAlignTop;
    }

    SetHorAlignType(horAlignType);
    SetVerAlignType(verAlignType);

    SetMargin(GetMarginByPlacement(), true);
}

void PopoverArrow::SetArrowArea(ui::UiRect rcArea)
{
    m_rcArea = rcArea;
    Invalidate();
}

ui::HorAlignType PopoverArrow::GetHorAlignType()
{
    // coz arrow is always point to anchor
    if (m_nPlacement & kPlaceTopLeft ||
        m_nPlacement & kPlaceRightTop ||
        m_nPlacement & kPlaceRight ||
        m_nPlacement & kPlaceRightBottom ||
        m_nPlacement & kPlaceBottomLeft)
        return ui::kHorAlignLeft;
    else if (m_nPlacement & kPlaceTop ||
        m_nPlacement & kPlaceBottom)
        return ui::kHorAlignCenter;
    else
        return ui::kHorAlignRight;
}

ui::VerAlignType PopoverArrow::GetVerAlignType()
{
    // coz arrow is always point to anchor
    if (m_nPlacement & kPlaceTopLeft ||
        m_nPlacement & kPlaceTop ||
        m_nPlacement & kPlaceTopRight ||
        m_nPlacement & kPlaceLeftBottom ||
        m_nPlacement & kPlaceRightBottom)
        return ui::kVerAlignBottom;
    else if (m_nPlacement & kPlaceLeft ||
        m_nPlacement & kPlaceRight)
        return ui::kVerAlignCenter;
    else
        return ui::kVerAlignTop;
}

ui::UiMargin PopoverArrow::GetMarginByPlacement()
{
    ui::UiMargin margin{ 0, 0, 0, 0 };

    // coz arrow is always point to anchor
    if (m_nPlacement & kPlaceLeftTop || m_nPlacement & kPlaceLeft || m_nPlacement & kPlaceLeftBottom) {
        auto marginRight = m_rcArea.left - GetFixedWidth().GetInt32() + 1;
        margin = { 0,kMargin,marginRight > 0 ? marginRight : 0,kMargin };
    }

    if (m_nPlacement & kPlaceTopLeft || m_nPlacement & kPlaceTop || m_nPlacement & kPlaceTopRight) {
        auto marginBottom = m_rcArea.top - GetFixedHeight().GetInt32() + 1;
        margin = { kMargin,0,kMargin,marginBottom > 0 ? marginBottom : 0 };
    }

    if (m_nPlacement & kPlaceRightTop || m_nPlacement & kPlaceRight || m_nPlacement & kPlaceRightBottom) {
        auto marginLeft = m_rcArea.right - GetFixedWidth().GetInt32() + 1;
        margin = { marginLeft > 0 ? marginLeft : 0,kMargin,0,kMargin };
    }

    if (m_nPlacement & kPlaceBottomLeft || m_nPlacement & kPlaceBottom || m_nPlacement & kPlaceBottomRight) {
        auto marginTop = m_rcArea.bottom - GetFixedHeight().GetInt32() + 1;
        margin = { kMargin,marginTop > 0 ? marginTop : 0,kMargin,0 };
    }

    // center placement should not have arrow
    //ASSERT(!(m_nPlacement & kPlaceCenter));
    if (m_nPlacement & kPlaceCenter) {
        auto marginTop = m_rcArea.bottom - GetFixedHeight().GetInt32() + 1;
        margin = { kMargin,marginTop > 0 ? marginTop : 0,kMargin,0 };
    }

    return margin;
}

void PopoverArrow::CalcTrianglePoints()
{
    auto rcPos = GetPos();
    auto rcMargin = GetMargin();

    if (m_nPlacement & kPlaceLeftTop || m_nPlacement & kPlaceLeft || m_nPlacement & kPlaceLeftBottom) {
        m_ptTriangle[0] = { rcPos.left,rcPos.top };
        m_ptTriangle[1] = { rcPos.left,rcPos.bottom };
        m_ptTriangle[2] = { rcPos.right,(rcPos.bottom - rcPos.top) / 2 + rcPos.top };
    }

    if (m_nPlacement & kPlaceTopLeft || m_nPlacement & kPlaceTop || m_nPlacement & kPlaceTopRight) {
        m_ptTriangle[0] = { rcPos.left,rcPos.top };
        m_ptTriangle[1] = { rcPos.right,rcPos.top };
        m_ptTriangle[2] = { (rcPos.right - rcPos.left) / 2 + rcPos.left,rcPos.bottom };
    }

    if (m_nPlacement & kPlaceRightTop || m_nPlacement & kPlaceRight || m_nPlacement & kPlaceRightBottom) {
        m_ptTriangle[0] = { rcPos.right,rcPos.top };
        m_ptTriangle[1] = { rcPos.right,rcPos.bottom };
        m_ptTriangle[2] = { rcPos.left,(rcPos.bottom - rcPos.top) / 2 + rcPos.top };
    }

    if (m_nPlacement & kPlaceBottomLeft || m_nPlacement & kPlaceBottom || m_nPlacement & kPlaceBottomRight) {
        m_ptTriangle[0] = { rcPos.left,rcPos.bottom };
        m_ptTriangle[1] = { rcPos.right,rcPos.bottom };
        m_ptTriangle[2] = { (rcPos.right - rcPos.left) / 2 + rcPos.left,rcPos.top };
    }

    // center placement should not have arrow
    //ASSERT(!(m_nPlacement & kPlaceCenter));
    if (m_nPlacement & kPlaceCenter) {
        m_ptTriangle[0] = { rcPos.left,rcPos.bottom };
        m_ptTriangle[1] = { rcPos.right,rcPos.bottom };
        m_ptTriangle[2] = { (rcPos.right - rcPos.left) / 2 + rcPos.left,rcPos.top };
    }
}

void PopoverArrow::PaintBkColor(ui::IRender* pRender)
{

    CalcTrianglePoints();

    m_pPath->Reset();
    m_pPath->AddPolygon(m_ptTriangle, 3);

    pRender->FillPath(m_pPath.get(), m_pBrush.get());
}

// PopoverHeader
PopoverHeader::PopoverHeader(
    const std::wstring& strTitle,
    bool bShowClose,
    PopoverIconType nIconType) :
    m_bUseMaxSize(false),
    m_strTitle(strTitle),
    m_bShowClose(bShowClose),
    m_nIconType(nIconType),
    m_pControlIcon(nullptr),
    m_pRichEditTitle(nullptr),
    m_pButtonClose(nullptr)
{
    SetFixedWidth(ui::UiFixedInt::MakeAuto(), true, true);
    SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);
    GetLayout()->SetChildMargin(12);

    if (m_nIconType != kIconNone) {
        m_pControlIcon = new ui::Control();
        if (m_nIconType == kIconInfo)
            m_pControlIcon->SetClass(L"popover_icon popover_icon_info");
        else if (m_nIconType == kIconWarning)
            m_pControlIcon->SetClass(L"popover_icon popover_icon_warning");
        if (m_nIconType == kIconError)
            m_pControlIcon->SetClass(L"popover_icon popover_icon_error");

        m_pControlIcon->SetFadeVisible(true);
        AddItem(m_pControlIcon);
    }

    if (m_strTitle.length())
    {
        m_pRichEditTitle = new ui::RichEdit();
        m_pRichEditTitle->SetName(L"edit_title");
        m_pRichEditTitle->SetClass(L"popover_label popover_label_title");
        m_pRichEditTitle->SetFixedWidth(ui::UiFixedInt::MakeAuto(), true, true);
        m_pRichEditTitle->SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);
        m_pRichEditTitle->SetText(m_strTitle);

        AddItem(m_pRichEditTitle);
    }

    if (m_bShowClose) {
        m_pButtonClose = new ui::Button();
        m_pButtonClose->SetName(L"btn_close");
        m_pButtonClose->SetClass(L"popover_btn_close");
        m_pButtonClose->AttachClick(ToWeakCallback([this](const ui::EventArgs& args) {
            return OnClose(args);
            }));
        AddItem(m_pButtonClose);
    }
}

PopoverHeader::~PopoverHeader()
{

}

std::wstring PopoverHeader::GetType() const { return L"PopoverHeader"; }

ui::UiEstSize PopoverHeader::EstimateSize(ui::UiSize szAvailable)
{
    if (GetFixedWidth().IsAuto() || GetFixedHeight().IsAuto()) {
        if (!IsReEstimateSize(szAvailable)) {
            return GetEstimateSize();
        }

        ui::UiSize maxSize = szAvailable;

        ui::UiMargin marginRect = GetMargin();
        ui::UiPadding paddingRect = GetPadding();

        maxSize.cx -= marginRect.left + marginRect.right;
        maxSize.cx -= paddingRect.left + paddingRect.right;
        maxSize.cy = 0;

        ui::UiSize editMaxSize = maxSize;
        if (m_pControlIcon) {
            editMaxSize.cx -= m_pControlIcon->GetFixedWidth().GetInt32();
        }

        if (m_pButtonClose) {
            editMaxSize.cx -= m_pButtonClose->GetFixedWidth().GetInt32();
        }

        int childMargin = GetLayout()->GetChildMarginX();
        editMaxSize.cx -= ((int)m_items.size() - 1) * childMargin;

        ui::UiSize editSize{ 0, 0 };
        if (m_pRichEditTitle) {
            ui::UiMargin editMargin = m_pRichEditTitle->GetMargin();
            editSize = m_pRichEditTitle->EstimateText({ editMaxSize.cx - editMargin.left - editMargin.right,editMaxSize.cy });

            if (m_bUseMaxSize) {
                m_pRichEditTitle->SetFixedWidth(ui::UiFixedInt(editMaxSize.cx - editMargin.left - editMargin.right), true, false);
                m_pRichEditTitle->SetFixedHeight(ui::UiFixedInt(editSize.cy), true, false);

                editSize.cx = editMaxSize.cx - editMargin.left - editMargin.right;
                editSize.cy += editMargin.top + editMargin.bottom;
            }
            else {
                m_pRichEditTitle->SetFixedWidth(ui::UiFixedInt(editSize.cx), true, false);
                m_pRichEditTitle->SetFixedHeight(ui::UiFixedInt(editSize.cy), true, false);

                editSize.cx += editMargin.left + editMargin.right;
                editSize.cy += editMargin.top + editMargin.bottom;
            }
        }

        ui::UiSize fixedSize = editSize;

        if (m_pControlIcon) {
            fixedSize.cx += m_pControlIcon->GetFixedWidth().GetInt32();
            fixedSize.cy = std::max(fixedSize.cy, m_pControlIcon->GetFixedHeight().GetInt32());
        }

        if (m_pButtonClose) {
            fixedSize.cx += m_pButtonClose->GetFixedWidth().GetInt32();
            fixedSize.cy = std::max(fixedSize.cy, m_pButtonClose->GetFixedHeight().GetInt32());
        }

        fixedSize.cx += paddingRect.left + paddingRect.right + ((int)m_items.size() - 1) * childMargin;
        fixedSize.cy += paddingRect.top + paddingRect.bottom;

        if (m_bUseMaxSize && fixedSize.cx < maxSize.cx)
            fixedSize.cx = maxSize.cx;

        SetEstimateSize(MakeEstSize(fixedSize), szAvailable);
        SetReEstimateSize(false);
        return MakeEstSize(fixedSize);
    }

    return MakeEstSize(GetFixedSize());
}

void PopoverHeader::UpdateTitle(const std::wstring& strTitle)
{
    if (m_pRichEditTitle)
        m_pRichEditTitle->SetText(strTitle);

    ArrangeAncestor();
}

// PopoverBody
PopoverBody::PopoverBody(const std::wstring& content, const std::wstring& colorId) :
    m_bUseMaxSize(false),
    m_pRichEditContent(nullptr)
{
    SetFixedWidth(ui::UiFixedInt::MakeAuto(), true, true);
    SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);

    if (content.length()) {
        m_pRichEditContent = new ui::RichEdit();
        m_pRichEditContent->SetName(L"edit_content");
        m_pRichEditContent->SetClass(L"popover_label popover_label_content");
        m_pRichEditContent->SetFixedWidth(ui::UiFixedInt::MakeAuto(), true, true);
        m_pRichEditContent->SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);
        m_pRichEditContent->SetText(content);

        AddItem(m_pRichEditContent);
    }

    if (m_pRichEditContent && !colorId.empty())
        m_pRichEditContent->SetTextColor(colorId);
}

PopoverBody::~PopoverBody()
{

}

std::wstring PopoverBody::GetType() const { return L"PopoverBody"; }

ui::UiEstSize PopoverBody::EstimateSize(ui::UiSize szAvailable)
{
    if (GetFixedWidth().IsAuto() || GetFixedHeight().IsAuto()) {
        if (!IsReEstimateSize(szAvailable)) {
            return GetEstimateSize();
        }

        ui::UiSize maxSize = szAvailable;
        maxSize.cy = 0;

        ui::UiMargin marginRect = GetMargin();
        maxSize.cx -= marginRect.left + marginRect.right;

        ui::UiPadding paddingRect = GetPadding();
        maxSize.cx -= paddingRect.left + paddingRect.right;

        ui::UiSize editSize{ 0, 0 };
        if (m_pRichEditContent) {
            ui::UiMargin editMargin = m_pRichEditContent->GetMargin();
            editSize = m_pRichEditContent->EstimateText({ maxSize.cx - editMargin.left - editMargin.right,maxSize.cy });

            m_pRichEditContent->SetFixedHeight(ui::UiFixedInt(editSize.cy), true, false);
            m_pRichEditContent->SetFixedWidth(ui::UiFixedInt(editSize.cx), true, false);

            editSize.cx += editMargin.left + editMargin.right;
            editSize.cy += editMargin.top + editMargin.bottom;


        }

        ui::UiSize fixedSize = editSize;

        fixedSize.cx += paddingRect.left + paddingRect.right;
        fixedSize.cy += paddingRect.top + paddingRect.bottom;
        if (m_bUseMaxSize && fixedSize.cx < maxSize.cx)
            fixedSize.cx = maxSize.cx;

        SetEstimateSize(MakeEstSize(fixedSize), szAvailable);
        SetReEstimateSize(false);
        return MakeEstSize(fixedSize);
    }

    return MakeEstSize(GetFixedSize());
}

void PopoverBody::UpdateContent(const std::wstring& strContent)
{
    if (m_pRichEditContent)
        m_pRichEditContent->SetText(strContent);

    ArrangeAncestor();
}

// PopoverFooter
PopoverFooter::PopoverFooter(const std::wstring& strOk,
    const std::wstring& strCancel) :
    m_bUseMaxSize(false),
    m_pButtonOk(nullptr),
    m_pButtonCancel(nullptr),
    m_strOk(strOk),
    m_strCancel(strCancel)
{
    SetFixedWidth(ui::UiFixedInt::MakeAuto(), true, true);
    SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);
    GetLayout()->SetChildMargin(12);
}

PopoverFooter::~PopoverFooter()
{

}

std::wstring PopoverFooter::GetType() const { return L"PopoverFooter"; }

ui::UiEstSize PopoverFooter::EstimateSize(ui::UiSize szAvailable)
{
    if (GetFixedWidth().IsAuto() || GetFixedHeight().IsAuto()) {
        if (!IsReEstimateSize(szAvailable)) {
            return GetEstimateSize();
        }

        ui::UiSize maxSize = szAvailable;
        maxSize.cy = 0;

        ui::UiMargin marginRect = GetMargin();
        maxSize.cx -= marginRect.left + marginRect.right;

        ui::UiPadding paddingRect = GetPadding();
        maxSize.cx -= paddingRect.left + paddingRect.right;

        ui::UiSize okSize{ 0, 0 };
        if (m_pButtonOk) {
            okSize.cx = m_pButtonOk->GetFixedWidth().GetInt32();
            okSize.cy = m_pButtonOk->GetFixedHeight().GetInt32();
        }

        ui::UiSize cancelSize{ 0, 0 };
        if (m_pButtonCancel) {
            cancelSize.cx = m_pButtonCancel->GetFixedWidth().GetInt32();
            cancelSize.cy = m_pButtonCancel->GetFixedHeight().GetInt32();
        }

        int childMargin = GetLayout()->GetChildMarginX();

        ui::UiSize fixedSize;
        fixedSize.cx = okSize.cx + cancelSize.cx +
            paddingRect.left + paddingRect.right +
            ((int)m_items.size() - 1) * childMargin;

        fixedSize.cy = std::max(okSize.cy, cancelSize.cy) + paddingRect.top + paddingRect.bottom;
        if (m_bUseMaxSize && fixedSize.cx < maxSize.cx)
            fixedSize.cx = maxSize.cx;

        SetReEstimateSize(false);
        SetEstimateSize(MakeEstSize(fixedSize), szAvailable);
        return MakeEstSize(fixedSize);
    }

    return MakeEstSize(GetFixedSize());
}

void PopoverFooter::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    if (m_strCancel.length()) {
        m_pButtonCancel = new ui::Button();
        m_pButtonCancel->SetName(L"btn_cancel");
        m_pButtonCancel->SetClass(L"popover_btn_cancel");
        m_pButtonCancel->SetText(m_strCancel);
        m_pButtonCancel->AttachClick(ToWeakCallback([this](const ui::EventArgs& args) {
            return OnCancel(args);
            }));

        AddItem(m_pButtonCancel);
    }

    if (m_strOk.length()) {
        m_pButtonOk = new ui::Button();
        m_pButtonOk->SetName(L"btn_ok");
        m_pButtonOk->SetClass(L"popover_btn_ok");
        m_pButtonOk->SetText(m_strOk);
        m_pButtonOk->AttachClick(ToWeakCallback([this](const ui::EventArgs& args) {
            return OnOk(args);
            }));

        AddItem(m_pButtonOk);
    }
}

// PopoverRoot
ui::UiEstSize PopoverRoot::EstimateSize(ui::UiSize szAvailable)
{
    if (GetFixedWidth().IsAuto() || GetFixedHeight().IsAuto()) {
        if (!IsReEstimateSize(szAvailable)) {
            return GetEstimateSize();
        }

        ui::UiPadding paddingRect = GetPadding();
        ui::UiMargin marginRect = GetMargin();

        ui::UiSize maxSize{ szAvailable.cx, 0 };
        maxSize.cx -= paddingRect.left + paddingRect.right;
        maxSize.cx -= marginRect.left + marginRect.right;

        ui::UiSize sizeRoot = GetLayout()->EstimateSizeByChild(m_items, maxSize);

        ui::UiSize fixedSize;
        fixedSize.cx = sizeRoot.cx;
        fixedSize.cy = sizeRoot.cy;

        if (fixedSize.cx < GetMinWidth()) {
            fixedSize.cx = GetMinWidth();
        }

        SetReEstimateSize(false);
        for (auto it = m_items.begin(); it != m_items.end(); it++) {
            if (!(*it)->IsVisible()) {
                continue;
            }
            if ((*it)->GetFixedWidth().IsAuto() || (*it)->GetFixedHeight().IsAuto()) {
                if ((*it)->IsReEstimateSize(szAvailable)) {
                    SetReEstimateSize(true);
                    break;
                }
            }
        }

        SetEstimateSize(MakeEstSize(fixedSize), szAvailable);
        return MakeEstSize(fixedSize);
    }

    return MakeEstSize(GetFixedSize());
}

// Popover
Popover::Popover(ui::Control* pAnchor,
    int nPlacement,
    int nShowType,
    int nDisappearType,
    bool bRemoveOnClose,
    int nShowTimeouts,
    int nDisappearTimeouts,
    PopoverHeader* header,
    PopoverBody* body,
    PopoverFooter* footer,
    PopoverArrow* arrow) :
    m_strTag(),
    m_pAnchor(pAnchor),
    m_nPlacement(nPlacement),
    m_nShowType(nShowType),
    m_nDisappearType(nDisappearType),
    m_bRemoveOnClose(bRemoveOnClose),
    m_nShowTimeouts(nShowTimeouts),
    m_nDisappearTimeouts(nDisappearTimeouts),
    m_nShowTriggerIndex(0),
    m_nDisappearTriggerIndex(0),
    m_pPopoverRoot(nullptr),
    m_pPopoverHeader(header),
    m_pPopoverBody(body),
    m_pPopoverFooter(footer),
    m_pPopoverArrow(arrow),
    m_cbResult(),
    m_cbClose()
{
    std::wstringstream ss;
    ss << L"popover_" << this;
    SetName(ss.str());

    // set width height 
    SetFixedWidth(ui::UiFixedInt::MakeAuto(), true, true);
    SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);

    // show shadow
    SetClass(L"popover_shadow");

    // create popover root
    m_pPopoverRoot = new PopoverRoot();
    m_pPopoverRoot->SetClass(L"popover_root");
    AddItem(m_pPopoverRoot);

    // hide popover first
    SetFadeVisible(false);

    // DO NOT DELAY DESTROY
    SetDelayedDestroy(false);

    // attach last event of anchor
    if (m_pAnchor) {
        m_pAnchor->AttachLastEvent(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
            nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this]() {TriggerResult({ kResultNone }, true); }));
            return true;
            }));
    }
}

Popover::~Popover()
{
    m_cbClose({ kResultNone });
}

std::wstring Popover::GetType() const { return L"Popover"; }

ui::UiEstSize Popover::EstimateSize(ui::UiSize szAvailable)
{
    if (GetFixedWidth().IsAuto() || GetFixedHeight().IsAuto()) {
        if (!IsReEstimateSize(szAvailable)) {
            return GetEstimateSize();
        }

        ui::UiPadding paddingRect = GetPadding();

        ui::UiSize maxSize{ 0, 0 };
        maxSize.cx = GetMaxWidth();
        maxSize.cx -= paddingRect.left + paddingRect.right;
        ui::UiSize sizeRoot = GetLayout()->EstimateSizeByChild({ m_pPopoverRoot }, maxSize);

        ui::UiSize fixedSize;
        fixedSize.cx = sizeRoot.cx;
        fixedSize.cy = sizeRoot.cy;

        if (fixedSize.cx < GetMinWidth()) {
            fixedSize.cx = GetMinWidth();
        }

        SetReEstimateSize(false);
        for (auto it = m_items.begin(); it != m_items.end(); it++) {
            if (!(*it)->IsVisible()) {
                continue;
            }
            if ((*it)->GetFixedWidth().IsAuto() || (*it)->GetFixedHeight().IsAuto()) {
                if ((*it)->IsReEstimateSize(szAvailable)) {
                    SetReEstimateSize(true);
                    break;
                }
            }
        }

        SetEstimateSize(MakeEstSize(fixedSize), szAvailable);
        return MakeEstSize(fixedSize);
    }

    return MakeEstSize(GetFixedSize());
}

void Popover::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    InitializeElements();
    InitializePosition();
    InitializeShowTriggers(kTriggerNone, m_nShowType, m_nShowTimeouts);
    InitializeDisappearTriggers(kTriggerNone, m_nDisappearType, m_nDisappearTimeouts);
}

void Popover::SetPos(ui::UiRect rc)
{
    if (m_pAnchor) {
        rc = ReCalcByAnchorPos(rc);
    }

    ui::Control::SetPos(rc);
    ui::UiSize64 requiredSize;
    if (m_items.size() == 0) {
        requiredSize.cx = 0;
        requiredSize.cy = 0;
    }
    else {
        requiredSize = GetLayout()->ArrangeChild(m_items, rc);
    }
}

void Popover::Update(PopoverHeader* header, PopoverBody* body, PopoverFooter* footer)
{
    m_nDisappearTriggerIndex++;

    if (header) {
        if (m_pPopoverHeader) {
            m_pPopoverRoot->RemoveItem(m_pPopoverHeader);
        }

        m_pPopoverHeader = header;

        m_pPopoverHeader->AttachCloseClicked(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
            nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this]() {TriggerResult({ kResultNone }); }));
            return true;
            }));
        m_pPopoverRoot->AddItemAt(m_pPopoverHeader, 0);
    }

    if (body) {
        size_t index = GetItemCount() - 1;
        if (m_pPopoverBody) {
            index = GetItemIndex(m_pPopoverBody);
            m_pPopoverRoot->RemoveItem(m_pPopoverBody);
        }

        m_pPopoverBody = body;
        m_pPopoverRoot->AddItemAt(m_pPopoverBody, (index == Box::InvalidIndex) ? 0 : index);
    }

    if (footer) {
        size_t index = GetItemCount() - 1;
        if (m_pPopoverFooter) {
            index = GetItemIndex(m_pPopoverFooter);
            m_pPopoverRoot->RemoveItem(m_pPopoverFooter);
        }

        m_pPopoverFooter = footer;
        m_pPopoverFooter->AttachCancelClicked(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
            nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this]() {TriggerResult({ kResultCancel }); }));
            return true;
            }));
        m_pPopoverFooter->AttachOkClicked(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
            nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this]() {TriggerResult({ kResultOk }); }));
            return true;
            }));
        m_pPopoverRoot->AddItemAt(m_pPopoverFooter, (index == Box::InvalidIndex) ? 0 : index);
    }

    ArrangeSelf();
}

void Popover::UpdateContent(const std::wstring& content)
{
    if (!m_pPopoverFooter)
        return;

    m_pPopoverBody->UpdateContent(content);
}

void Popover::UpdateNotification(const std::wstring& strTitle, const std::wstring& strContent)
{
    m_nDisappearTriggerIndex++;

    if (m_pPopoverHeader)
        m_pPopoverHeader->UpdateTitle(strTitle);

    if (m_pPopoverBody)
        m_pPopoverBody->UpdateContent(strContent);
}

void Popover::UpdateTooltip(const std::wstring& tooltip)
{
    m_nDisappearTriggerIndex++;

    if (!m_pPopoverBody)
        return;

    m_pPopoverBody->UpdateContent(tooltip);
}

void Popover::UpdateShowType(int nType, int nTimeouts)
{
    InitializeShowTriggers(m_nShowType, nType, nTimeouts);

    m_nShowType = nType;
}

void Popover::UpdateDisappearType(int nType, int nTimeouts)
{
    InitializeDisappearTriggers(m_nDisappearType, nType, nTimeouts);

    m_nDisappearType = nType;
}

ui::HorAlignType Popover::GetHorAlignType()
{
    if (m_nPlacement & kPlaceLeft ||
        m_nPlacement & kPlaceLeftTop ||
        m_nPlacement & kPlaceLeftBottom ||
        m_nPlacement & kPlaceTopLeft ||
        m_nPlacement & kPlaceBottomLeft)
        return ui::kHorAlignLeft;

    if (m_nPlacement & kPlaceRightTop ||
        m_nPlacement & kPlaceRight ||
        m_nPlacement & kPlaceRightBottom ||
        m_nPlacement & kPlaceTopRight ||
        m_nPlacement & kPlaceBottomRight)
        return ui::kHorAlignRight;

    return ui::kHorAlignCenter;
}

ui::VerAlignType Popover::GetVerAlignType()
{
    if (m_nPlacement & kPlaceTopLeft ||
        m_nPlacement & kPlaceTop ||
        m_nPlacement & kPlaceTopRight ||
        m_nPlacement & kPlaceRightTop ||
        m_nPlacement & kPlaceLeftTop)
        return ui::kVerAlignTop;

    if (m_nPlacement & kPlaceBottomLeft ||
        m_nPlacement & kPlaceBottom ||
        m_nPlacement & kPlaceBottomRight ||
        m_nPlacement & kPlaceLeftBottom ||
        m_nPlacement & kPlaceRightBottom)
        return ui::kVerAlignBottom;

    return ui::kVerAlignCenter;
}

ui::UiRect Popover::ReCalcByAnchorPos(ui::UiRect rc)
{
    auto anchorPos = m_pAnchor->GetPos();
    auto estSize = EstimateSize({ 0,0 });
    ui::UiSize size(estSize.cx.GetInt32(), estSize.cy.GetInt32());

    int verOffset = 0;
    int horOffset = 0;

    bool bHasArrow = (m_pPopoverArrow != nullptr);
    bool bIsInner = (m_nPlacement & kPlaceInner);

    if (bIsInner) {
        if (m_nPlacement & kPlaceLeftTop ||
            m_nPlacement & kPlaceLeft ||
            m_nPlacement & kPlaceLeftBottom) {
            rc.left = anchorPos.left;
        }
        else if (m_nPlacement & kPlaceRightTop ||
            m_nPlacement & kPlaceRight ||
            m_nPlacement & kPlaceRightBottom) {
            rc.left = anchorPos.right - size.cx;
        }
        else if (m_nPlacement & kPlaceTopLeft ||
            m_nPlacement & kPlaceTop ||
            m_nPlacement & kPlaceTopRight) {
            rc.top = anchorPos.top;
        }
        else if (m_nPlacement & kPlaceBottomLeft ||
            m_nPlacement & kPlaceBottom ||
            m_nPlacement & kPlaceBottomRight) {
            rc.top = anchorPos.bottom - size.cy;
        }

        if (m_nPlacement & kPlaceLeftTop ||
            m_nPlacement & kPlaceRightTop)
            rc.top = anchorPos.top;
        else if (m_nPlacement & kPlaceLeft ||
            m_nPlacement & kPlaceRight)
            rc.top = anchorPos.top + (anchorPos.Height() - size.cy) / 2;
        else if (m_nPlacement & kPlaceLeftBottom ||
            m_nPlacement & kPlaceRightBottom)
            rc.top = anchorPos.bottom - size.cy;
        else if (m_nPlacement & kPlaceTopLeft ||
            m_nPlacement & kPlaceBottomLeft)
            rc.left = anchorPos.left;
        else if (m_nPlacement & kPlaceTop ||
            m_nPlacement & kPlaceBottom)
            rc.left = anchorPos.left + (anchorPos.Width() - size.cx) / 2;
        else if (m_nPlacement & kPlaceTopRight ||
            m_nPlacement & kPlaceBottomRight)
            rc.left = anchorPos.right - size.cx;

        if (m_nPlacement & kPlaceCenter) {
            rc.left = anchorPos.left + (anchorPos.Width() - size.cx) / 2;
            rc.right = rc.left + size.cx;
            rc.top = anchorPos.top + (anchorPos.Height() - size.cy) / 2;
            rc.bottom = rc.top + size.cy;
        }
    }
    else {
        if (m_nPlacement & kPlaceLeftTop ||
            m_nPlacement & kPlaceLeft ||
            m_nPlacement & kPlaceLeftBottom) {
            rc.left = anchorPos.left - size.cx;
            if (bHasArrow)
                rc.top = anchorPos.top + anchorPos.Height() / 2;
            else
                rc.top = anchorPos.top - (size.cy - anchorPos.Height()) / 2;
        }
        else if (m_nPlacement & kPlaceRightTop ||
            m_nPlacement & kPlaceRight ||
            m_nPlacement & kPlaceRightBottom) {
            rc.left = anchorPos.right;
            if (bHasArrow)
                rc.top = anchorPos.top + anchorPos.Height() / 2;
            else
                rc.top = anchorPos.top - (size.cy - anchorPos.Height()) / 2;
        }
        else if (m_nPlacement & kPlaceTopLeft ||
            m_nPlacement & kPlaceTop ||
            m_nPlacement & kPlaceTopRight) {
            rc.top = anchorPos.top - size.cy;
            if (bHasArrow)
                rc.left = anchorPos.left + anchorPos.Width() / 2;
            else
                rc.left = anchorPos.left - (size.cx - anchorPos.Width()) / 2;
        }
        else if (m_nPlacement & kPlaceBottomLeft ||
            m_nPlacement & kPlaceBottom ||
            m_nPlacement & kPlaceBottomRight) {
            rc.top = anchorPos.bottom;
            if (bHasArrow)
                rc.left = anchorPos.left + anchorPos.Width() / 2;
            else
                rc.left = anchorPos.left - (size.cx - anchorPos.Width()) / 2;
        }



        if (bHasArrow) {
            if (m_nPlacement & kPlaceLeftTop ||
                m_nPlacement & kPlaceRightTop) {
                int offset = PopoverArrow::kMargin + PopoverArrow::kHeight / 2;
                verOffset -= ui::GlobalManager::Instance().Dpi().ScaleInt(offset);
            }
            else if (m_nPlacement & kPlaceLeft ||
                m_nPlacement & kPlaceRight) {
                verOffset -= size.cy / 2;
            }
            else if (m_nPlacement & kPlaceLeftBottom ||
                m_nPlacement & kPlaceRightBottom) {
                int offset = (PopoverArrow::kMargin + PopoverArrow::kHeight / 2);
                verOffset -= size.cy - ui::GlobalManager::Instance().Dpi().ScaleInt(offset);
            }

            if (m_nPlacement & kPlaceTopLeft ||
                m_nPlacement & kPlaceBottomLeft) {
                int offset = PopoverArrow::kMargin + PopoverArrow::kWidth / 2;
                horOffset -= ui::GlobalManager::Instance().Dpi().ScaleInt(offset);
            }
            else if (m_nPlacement & kPlaceTop ||
                m_nPlacement & kPlaceBottom) {
                horOffset -= size.cx / 2;
            }
            else if (m_nPlacement & kPlaceTopRight ||
                m_nPlacement & kPlaceBottomRight) {
                int offset = (PopoverArrow::kMargin + PopoverArrow::kWidth / 2);
                horOffset -= size.cx - ui::GlobalManager::Instance().Dpi().ScaleInt(offset);
            }
        }
    }

    rc.top += verOffset;
    rc.bottom = rc.top + size.cy;
    rc.left += horOffset;
    rc.right = rc.left + size.cx;

    return rc;
}

void Popover::InitializeElements()
{
    if (m_pPopoverHeader) {
        m_pPopoverHeader->AttachCloseClicked(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
            nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this]() {TriggerResult({ kResultNone }); }));
            return true;
            }));
        m_pPopoverRoot->AddItem(m_pPopoverHeader);
    }

    if (m_pPopoverBody) {
        m_pPopoverRoot->AddItem(m_pPopoverBody);
    }

    if (m_pPopoverFooter) {
        m_pPopoverFooter->AttachCancelClicked(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
            nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this]() {TriggerResult({ kResultCancel }); }));
            return true;
            }));
        m_pPopoverFooter->AttachOkClicked(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
            nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this]() {TriggerResult({ kResultOk }); }));
            return true;
            }));

        m_pPopoverRoot->AddItem(m_pPopoverFooter);
    }

    if (m_pPopoverArrow) {
        ui::UiMargin rootMargin = m_pPopoverRoot->GetMargin();
        ui::UiRect arrowArea(rootMargin.left, rootMargin.top, rootMargin.right, rootMargin.bottom);
        m_pPopoverArrow->SetArrowArea(arrowArea);
        AddItem(m_pPopoverArrow);
    }
}

void Popover::InitializePosition()
{
    if (m_pAnchor) {
        m_pAnchor->AttachResize(nbase::Bind(&Popover::OnAnchorResize, this, std::placeholders::_1));
        m_pAnchor->AttachVisibleChange(nbase::Bind(&Popover::OnAnchorVisibleChange, this, std::placeholders::_1));
        m_pAnchor->AttachLastEvent(nbase::Bind(&Popover::OnAnchorLastEvent, this, std::placeholders::_1));
    }
    else {
        SetFloat(true);
        SetHorAlignType(GetHorAlignType());
        SetVerAlignType(GetVerAlignType());
    }
}

void Popover::InitializeShowTriggers(int nOldType, int nNewType, int nTimeouts)
{
    if (m_pAnchor) {
        if (!HasTrigger(nOldType, kTriggerSetFocus) && HasTrigger(nNewType, kTriggerSetFocus))
            m_pAnchor->AttachSetFocus(nbase::Bind(&Popover::OnAnchorSetFocus, this, std::placeholders::_1));
        if (!HasTrigger(nOldType, kTriggerClick) && HasTrigger(nNewType, kTriggerClick))
            m_pAnchor->AttachButtonDown(nbase::Bind(&Popover::OnAnchorMouseClick, this, std::placeholders::_1));
        if (!HasTrigger(nOldType, kTriggerEnter) && HasTrigger(nNewType, kTriggerEnter))
            m_pAnchor->AttachMouseEnter(nbase::Bind(&Popover::OnAnchorMouseEnter, this, std::placeholders::_1));
        if (!HasTrigger(nOldType, kTriggerHover) && HasTrigger(nNewType, kTriggerHover))
            m_pAnchor->AttachMouseHover(nbase::Bind(&Popover::OnAnchorMouseHover, this, std::placeholders::_1));
    }

    if (!HasTrigger(nOldType, kTriggerImmediately) && HasTrigger(nNewType, kTriggerImmediately)) {
        SetFadeVisible(true);
    }

    if (HasTrigger(nNewType, kTriggerTimeout)) {
        int index = ++m_nShowTriggerIndex;
        m_nShowTimeouts = nTimeouts;

        ASSERT(m_nShowTimeouts);
        if (!HasTrigger(nOldType, kTriggerTimeout))
            nbase::ThreadManager::PostDelayedTask(kThreadUI,
                nbase::Bind(&Popover::OnShowTriggerTimeout, this, index),
                nbase::TimeDelta::FromMilliseconds(m_nShowTimeouts));
    }
}

void Popover::InitializeDisappearTriggers(int nOldType, int nNewType, int nTimeouts)
{
    if (m_pAnchor) {
        if (!HasTrigger(nOldType, kTriggerKillFocus) && HasTrigger(nNewType, kTriggerKillFocus))
            m_pAnchor->AttachKillFocus(nbase::Bind(&Popover::OnAnchorKillFocus, this, std::placeholders::_1));
        if (!HasTrigger(nOldType, kTriggerLeave) && HasTrigger(nNewType, kTriggerLeave))
            m_pAnchor->AttachMouseLeave(nbase::Bind(&Popover::OnAnchorMouseLeave, this, std::placeholders::_1));
    }

    if (HasTrigger(nNewType, kTriggerTimeout)) {
        int index = ++m_nDisappearTriggerIndex;
        m_nDisappearTimeouts = nTimeouts;

        ASSERT(m_nDisappearTimeouts);
        if (!HasTrigger(nOldType, kTriggerTimeout))
            nbase::ThreadManager::PostDelayedTask(kThreadUI,
                nbase::Bind(&Popover::OnDisappearTriggerTimeout, this, index),
                nbase::TimeDelta::FromMilliseconds(m_nDisappearTimeouts));
    }
}

void Popover::OnShowTriggerTimeout(int index)
{
    if (index != m_nShowTriggerIndex) {
        index = m_nShowTriggerIndex;
        nbase::ThreadManager::PostDelayedTask(kThreadUI,
            nbase::Bind(&Popover::OnShowTriggerTimeout, this, index),
            nbase::TimeDelta::FromMilliseconds(m_nShowTimeouts));
        return;
    }

    if (m_nShowType & kTriggerTimeout) {
        SetFadeVisible(true);
        m_nShowType |= ~kTriggerTimeout;
    }
}

void Popover::OnDisappearTriggerTimeout(int index)
{
    if (index != m_nDisappearTriggerIndex) {
        index = m_nDisappearTriggerIndex;
        nbase::ThreadManager::PostDelayedTask(kThreadUI,
            nbase::Bind(&Popover::OnDisappearTriggerTimeout, this, index),
            nbase::TimeDelta::FromMilliseconds(m_nDisappearTimeouts));
        return;
    }

    if (m_nDisappearType & kTriggerTimeout) {
        nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback(
            [this]() {
                TriggerResult({ kResultNone }, true);
            }
        ));

        m_nDisappearType |= ~kTriggerTimeout;
    }
}

void Popover::TriggerResult(const PopoverResult& result, bool bForceClose)
{
    m_cbResult(result);

    if ((m_nDisappearType & kTriggerConfirm) || bForceClose)
        TriggerClose();
}

void Popover::TriggerClose()
{
    this->SetFadeVisible(false);

    if (m_bRemoveOnClose) {
        if (GetParent())
            GetParent()->RemoveItem(this);
        else
            delete this;
    }
}

bool Popover::OnAnchorResize(const ui::EventArgs& /*args*/)
{
    SetPos(GetPos());

    return true;
}

bool Popover::OnAnchorSetFocus(const ui::EventArgs& /*args*/)
{
    if (m_nShowType & kTriggerSetFocus)
        SetFadeVisible(true);

    return true;
}

bool Popover::OnAnchorKillFocus(const ui::EventArgs& /*args*/)
{
    if (m_nDisappearType & kTriggerKillFocus)
        nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback(
            [this]() {
                TriggerResult({ kResultNone }, true);
            }
    ));

    return true;
}

bool Popover::OnAnchorMouseClick(const ui::EventArgs& /*args*/)
{
    if (m_nShowType & kTriggerClick)
        SetFadeVisible(true);

    return true;
}

bool Popover::OnAnchorMouseEnter(const ui::EventArgs& /*args*/)
{
    if (m_nShowType & kTriggerEnter)
        SetFadeVisible(true);

    return true;
}

bool Popover::OnAnchorMouseHover(const ui::EventArgs& /*args*/)
{
    if (m_nShowType & kTriggerHover)
        SetFadeVisible(true);

    return true;
}

bool Popover::OnAnchorMouseLeave(const ui::EventArgs& /*args*/)
{
    if (m_nDisappearType & kTriggerLeave)
        nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this]() {TriggerResult({ kResultNone }, true); }));

    return true;
}

bool Popover::OnAnchorLastEvent(const ui::EventArgs& /*args*/)
{
    this->SetFadeVisible(false);

    if (GetParent())
        GetParent()->RemoveItem(this);
    else
        delete this;

    return true;
}

bool Popover::OnAnchorVisibleChange(const ui::EventArgs& /*args*/)
{
    bool is_visible = m_pAnchor->IsVisible();
    if (is_visible && m_nShowType & kTriggerVisible)
        SetFadeVisible(true);

    if (is_visible)
        SetPos(GetPos());

    if (!is_visible && m_nDisappearType & kTriggerVisible)
        nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback(
            [this]() {
                TriggerResult({ kResultNone }, true);
            }
    ));

    return true;
}

// PopoverHolderLayer
const std::wstring PopoverHolderLayer::kHolderTypeNotification = L"NotificationHolder";
const std::wstring PopoverHolderLayer::kHolderTypePopover = L"PopoverHolder";
const std::wstring PopoverHolderLayer::kHolderTypeAlert = L"AlertHolder";

// PopoverLayer
PopoverLayer::PopoverLayer() :
    m_bShowMask(false),
    m_bDisableWindow(false),
    alert_count(0),
    m_pPopoverLayer(nullptr),
    m_pAlertLayer(nullptr)
{
    SetFloat(true);
    SetShowMask(m_bShowMask);
    SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, true);
    SetFixedHeight(ui::UiFixedInt::MakeStretch(), true, true);

    // DO NOT DELAY DESTROY
    SetDelayedDestroy(false);
}

PopoverLayer::~PopoverLayer()
{
    if (GetWindow())
        GetWindow()->RemoveMessageFilter(this);
}

void PopoverLayer::ShowAlert(Popover* popover)
{
    if (m_pAlertLayer && popover) {
        popover->AttachVisibleChange(ToWeakCallback(
            [this](const ui::EventArgs& args) {
                if (args.pSender->IsVisible()) {
                    alert_count++;
                    m_bDisableWindow = true;
                    m_pAlertLayer->SetMouseEnabled(true);
                }
                else {
                    alert_count--;
                    if (!alert_count) {
                        m_bDisableWindow = false;
                        m_pAlertLayer->SetMouseEnabled(false);
                    }
                }
                return true;
            }
        ));

        m_pAlertLayer->AddItem(popover);
    }
}

void PopoverLayer::ShowPopover(Popover* popover)
{
    if (m_pPopoverLayer && popover)
        m_pPopoverLayer->AddItem(popover);
}

void PopoverLayer::ShowTooltip(Popover* popover)
{
    if (m_pPopoverLayer && popover)
        m_pPopoverLayer->AddItem(popover);
}

void PopoverLayer::ShowNotification(Popover* popover)
{
    auto pAnchor = popover->GetAnchor();
    if (pAnchor == nullptr)
        pAnchor = GetParent();

    auto itr = m_NotifyLayerMap.find(pAnchor);
    if (itr != m_NotifyLayerMap.end()) {
        itr->second->AddItem(popover);
        return;
    }

    auto pHolderLayer = new NotificationHolderLayer(pAnchor);
    m_NotifyLayerMap[pAnchor] = pHolderLayer;
    this->AddItemAt(pHolderLayer, 0);

    pHolderLayer->AddItem(popover);
}

bool PopoverLayer::RemoveItem(Control* pControl)
{
    if (pControl->GetParent() == this)
        return __super::RemoveItem(pControl);

    auto pHolderLayer = static_cast<PopoverHolderLayer*>(pControl->GetParent());
    if (pHolderLayer) {
        // alert popover depends visible event to disable or enable mouse
        pControl->SetFadeVisible(false);
        return pHolderLayer->RemoveItem(pControl);
    }

    return false;
}

void PopoverLayer::ClearAll()
{
    for (auto pair : m_NotifyLayerMap)
        pair.second->RemoveAllItems();

    m_pAlertLayer->RemoveAllItems();
    m_pPopoverLayer->RemoveAllItems();
}

std::wstring PopoverLayer::GetType() const { return L"PopoverLayer"; }

void PopoverLayer::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    auto pHolderLayer = new NotificationHolderLayer(GetParent());
    m_NotifyLayerMap[GetParent()] = pHolderLayer;
    AddItem(pHolderLayer);

    m_pPopoverLayer = new PopoverHolderLayer(PopoverHolderLayer::kHolderTypePopover);
    AddItem(m_pPopoverLayer);

    m_pAlertLayer = new PopoverHolderLayer(PopoverHolderLayer::kHolderTypeAlert);
    AddItem(m_pAlertLayer);

    if (GetWindow())
        GetWindow()->AddMessageFilter(this);
}

LRESULT PopoverLayer::FilterMessage(UINT uMsg,
    WPARAM /*wParam*/,
    LPARAM lParam,
    bool& bHandled)
{
    bHandled = false;

    if (uMsg == WM_LBUTTONDOWN ||
        uMsg == WM_LBUTTONDBLCLK ||
        uMsg == WM_RBUTTONDOWN ||
        uMsg == WM_RBUTTONDBLCLK ||
        uMsg == WM_MBUTTONDOWN ||
        uMsg == WM_MBUTTONDBLCLK) {
        OnMouseEventButtonDown({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
    }
    else if (uMsg == WM_KEYDOWN || uMsg == WM_CHAR) {
        // this will stop any key down or char message to other controls
        // but will not stop other keyboard event lik Escape
        if (m_bDisableWindow)
            bHandled = true;
    }

    return 0;
}

void PopoverLayer::SetShowMask(bool show)
{
    m_bShowMask = show;
    if (m_bShowMask)
        SetBkColor(L"maskbkblack");
    else
        SetBkColor(L"");

    SetMouseEnabled(m_bShowMask);
    SetKeyboardEnabled(m_bShowMask);
}

void PopoverLayer::OnMouseEventButtonDown(ui::UiPoint pt)
{
    auto trigger_loop = [this](PopoverHolderLayer* holder, ui::UiPoint pt) {
        for (size_t index = 0; index < holder->GetItemCount(); ++index) {
            auto popover = static_cast<Popover*>(holder->GetItemAt(index));
            if (!popover)
                continue;

            auto control = GetWindow()->FindSubControlByPoint(GetWindow()->GetRoot(), pt);

            auto pos = popover->GetPos();
            if (!pos.ContainsPt(pt) &&
                (!(popover->m_nShowType & kTriggerClick) || popover->GetAnchor() != control) &&
                popover->m_nDisappearType & kTriggerClickLayer) {
                nbase::ThreadManager::PostTask(kThreadUI,
                    ToWeakCallback([this, popover]() {popover->TriggerResult({ kResultNone }, true); }));
            }
        }
    };

    if (m_pAlertLayer)
        trigger_loop(m_pAlertLayer, pt);

    for (auto pair : m_NotifyLayerMap)
        trigger_loop(pair.second, pt);

    if (m_pPopoverLayer)
        trigger_loop(m_pPopoverLayer, pt);
}

Popover* PopoverLayer::CreatePopover(ui::Control* pAnchor,
    const std::wstring& strTitle,
    const std::wstring& strContent,
    const std::wstring& strOk,
    const std::wstring& strCancel,
    int nPlacement,
    int nButtons,
    int nShowType,
    int nDisappearType,
    bool bShowArrow,
    bool bRemoveOnClose,
    int nShowTimeouts,
    int nDisappearTimeouts,
    PopoverHeader* pCustomHeader,
    PopoverBody* pCustomBody,
    PopoverFooter* pCustomFooter,
    PopoverArrow* pCustomArrow)
{
    PopoverHeader* header = pCustomHeader;
    PopoverBody* body = pCustomBody;
    PopoverFooter* footer = pCustomFooter;
    PopoverArrow* arrow = pCustomArrow;

    // Create header
    if (!header && (strTitle.length() || nButtons & kButtonClose)) {
        header = new PopoverHeader(strTitle, nButtons & kButtonClose);
    }

    // Create body
    if (!body && strContent.length()) {
        body = new PopoverBody(strContent, header ? L"" : L"popover_content_color1");
    }

    // Create footer
    if (!footer && (nButtons & kButtonOk | nButtons & kButtonCancel)) {
        footer = new PopoverFooter(
            nButtons & kButtonOk ? (strOk.length() ? strOk : ui::GlobalManager::Instance().Lang().GetStringViaID(L"STRING_OK")) : L"",
            nButtons & kButtonCancel ? (strCancel.length() ? strCancel : ui::GlobalManager::Instance().Lang().GetStringViaID(L"STRING_CANCEL")) : L"");
        footer->SetClass(L"popover_footer");
    }

    if (!arrow && !(nPlacement & kPlaceCenter) && bShowArrow) {
        arrow = new PopoverArrow(nPlacement);
    }

    auto popover = new Popover(pAnchor,
        nPlacement,
        nShowType,
        nDisappearType,
        bRemoveOnClose,
        nShowTimeouts,
        nDisappearTimeouts,
        header,
        body,
        footer,
        arrow);

    popover->SetClass(L"popover_popover");

    return popover;
}

Popover* PopoverLayer::CreateAlert(ui::Control* pAnchor,
    const std::wstring& strTitle,
    const std::wstring& strContent,
    const std::wstring& strOk,
    const std::wstring& strCancel,
    int nPlacement,
    int nButtons,
    int nShowType,
    int nDisappearType,
    bool bRemoveOnClose,
    PopoverHeader* pCustomHeader,
    PopoverBody* pCustomBody,
    PopoverFooter* pCustomFooter)
{
    PopoverFooter* footer = pCustomFooter;

    // Create footer
    if (!footer && (nButtons & kButtonOk | nButtons & kButtonCancel)) {
        footer = new AlertFooter(
            nButtons & kButtonOk ? (strOk.length() ? strOk : ui::GlobalManager::Instance().Lang().GetStringViaID(L"STRING_OK")) : L"",
            nButtons & kButtonCancel ? (strCancel.length() ? strCancel : ui::GlobalManager::Instance().Lang().GetStringViaID(L"STRING_CANCEL")) : L"");
        footer->SetClass(L"alert_footer");
    }

    auto popover = CreatePopover(pAnchor,
        strTitle,
        strContent,
        strOk,
        strCancel,
        nPlacement,
        nButtons,
        nShowType,
        nDisappearType,
        false,
        bRemoveOnClose,
        0,
        0,
        pCustomHeader,
        pCustomBody,
        footer,
        nullptr);

    popover->GetRoot()->SetClass(L"alert_root");
    popover->SetClass(L"popover_alert");

    return popover;
}

Popover* PopoverLayer::CreateNotification(ui::Control* pAnchor,
    const std::wstring& strTitle,
    const std::wstring& strContent,
    PopoverIconType nIconType,
    int nPlacement,
    int nButtons,
    int nShowType,
    int nDisappearType,
    bool bRemoveOnClose,
    int nShowTimeouts,
    int nDisappearTimeouts,
    PopoverHeader* pCustomHeader,
    PopoverBody* pCustomBody)
{
    PopoverHeader* header = pCustomHeader;
    PopoverBody* body = pCustomBody;

    ASSERT(strTitle.length() || strContent.length() || pCustomHeader || pCustomBody);

    // Create header
    if (!header && (strTitle.length() || nButtons & kButtonClose)) {
        header = static_cast<PopoverHeader*>(new NotificationHeader(strTitle, nButtons & kButtonClose, nIconType));
    }

    // Create body
    if (!body && strContent.length()) {
        body = static_cast<PopoverBody*>(new NotificationBody(strContent));
    }

    auto popover = new Notification(pAnchor,
        nPlacement,
        nShowType,
        nDisappearType,
        bRemoveOnClose,
        nShowTimeouts,
        nDisappearTimeouts,
        header,
        body);

    popover->SetClass(L"popover_notification");

    return popover;
}

Popover* PopoverLayer::CreateToolltip(ui::Control* pAnchor,
    const std::wstring& strContent,
    int nPlacement, int nShowType,
    int nDisappearType,
    PopoverBody* pCustomBody)
{
    ASSERT(strContent.length());

    PopoverBody* body = pCustomBody;

    // Create body
    if (!body && strContent.length()) {
        body = static_cast<PopoverBody*>(new TooltipBody(strContent));
    }

    auto popover = CreatePopover(pAnchor,
        L"",
        strContent,
        L"",
        L"",
        nPlacement,
        kButtonNone,
        nShowType,
        nDisappearType,
        false,
        false,
        0,
        0,
        nullptr,
        body,
        nullptr,
        nullptr);

    popover->GetRoot()->SetClass(L"tooltip_root");
    popover->SetClass(L"popover_tooltip");

    return popover;
}

}