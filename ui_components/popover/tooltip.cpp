#include "tooltip.h"

namespace nim_comp {

TooltipBody::TooltipBody(const std::wstring& content):
  PopoverBody(content)
{
  m_pRichEditContent->SetClass(L"tooltip_content");
}

std::wstring TooltipBody::GetType() const { return L"TooltipBody"; }

ui::UiSize TooltipBody::EstimateSize(ui::UiSize szAvailable)
{
    if (GetFixedWidth() > 0 && GetFixedHeight() > 0 && !IsReEstimateSize()) {
        return GetFixedSize();
    }

  ui::UiSize maxSize = szAvailable;
  maxSize.cy = 0;

  ui::UiRect marginRect = GetMargin();
  maxSize.cx -= marginRect.left + marginRect.right;

  ui::UiRect paddingRect = m_pLayout->GetPadding();
  maxSize.cx -= paddingRect.left + paddingRect.right;

  ui::UiSize editSize{ 0,0 };
  if (m_pRichEditContent) {
    ui::UiRect editMargin = m_pRichEditContent->GetMargin();
    editSize = m_pRichEditContent->EstimateText({ maxSize.cx - editMargin.left - editMargin.right,maxSize.cy });

    m_pRichEditContent->SetFixedHeight(editSize.cy, false);
    m_pRichEditContent->SetFixedWidth(editSize.cx, true, false);

    editSize.cx += editMargin.left + editMargin.right;
    editSize.cy += editMargin.top + editMargin.bottom;
  }

  ui::UiSize fixedSize = editSize;

  fixedSize.cx += paddingRect.left + paddingRect.right;
  fixedSize.cy += paddingRect.top + paddingRect.bottom;

  m_szEstimateSize = fixedSize;
  SetReEstimateSize(false);

  return fixedSize;
}

void TooltipBody::DoInit()
{
  if (m_bInited)
    return;

  m_bInited = true;
}

}