#include "tooltip.h"

namespace nim_comp {

TooltipBody::TooltipBody(const std::wstring& content) :
    PopoverBody(content)
{
    m_pRichEditContent->SetClass(L"tooltip_content");
}

std::wstring TooltipBody::GetType() const { return L"TooltipBody"; }

ui::UiEstSize TooltipBody::EstimateSize(ui::UiSize szAvailable)
{
    if (GetFixedWidth().GetInt32() > 0 && GetFixedHeight().GetInt32() > 0 && !IsReEstimateSize(szAvailable)) {
        return MakeEstSize(GetFixedSize());
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

    ui::UiEstSize estSize = ui::MakeEstSize(fixedSize);
    SetEstimateSize(estSize, szAvailable);
    SetReEstimateSize(false);
    return estSize;
}

}