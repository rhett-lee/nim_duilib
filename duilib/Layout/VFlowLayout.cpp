#include "VFlowLayout.h"
#include "duilib/Box/ScrollBox.h"
#include <map>

namespace ui 
{

VFlowLayout::VFlowLayout()
{
    //默认靠上对齐
    SetChildVAlignType(VerAlignType::kAlignTop);
}

UiSize64 VFlowLayout::ArrangeChildInternal(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly, bool bEstimateLayoutSize) const
{
    if (items.empty()) {
        return UiSize64();
    }
    if (!bEstimateOnly) {
        bEstimateLayoutSize = false;
    }
    DeflatePadding(rc);
    const UiRect rcBox = rc; //容器的矩形范围
    const UiSize szAvailable(rc.Width(), rc.Height());

    //需要进行布局处理的所有控件(KEY是控件，VALUE是宽度和高度)
    std::map<Control*, UiEstSize> itemsMap;

    //计算每个控件的宽度和高度，并记录到Map中
    for (auto pControl : items) {
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }

        UiEstSize estSize = pControl->EstimateSize(szAvailable);
        UiSize sz = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
        UiMargin rcMargin = pControl->GetMargin();

        //计算宽度
        if (estSize.cx.IsStretch()) {
            if (bEstimateLayoutSize) {
                //拉伸类型的子控件，不计入， 如果指定最小值，则按最小值计算
                sz.cx = 0;
                if (sz.cx < pControl->GetMinWidth()) {
                    sz.cx = pControl->GetMinWidth();
                }
            }
            else {
                if (estSize.cx.GetStretchPercentValue() >= 100) {
                    //宽度完全拉伸：无法显示
                    ASSERT(0);
                    sz.cx = 0;
                }
                else {
                    sz.cx = (CalcStretchValue(estSize.cx, szAvailable.cx) - rcMargin.left - rcMargin.right);
                }
            }            
            sz.cx = std::max(sz.cx, 0);
        }
        if (sz.cx < pControl->GetMinWidth()) {
            sz.cx = pControl->GetMinWidth();
        }
        if (sz.cx > pControl->GetMaxWidth()) {
            sz.cx = pControl->GetMaxWidth();
        }
        if (sz.cx < 0) {
            sz.cx = 0;
        }
        estSize.cx.SetInt32(sz.cx);//cx是已经计算好的确定数值，不再有拉伸和自动类型值

        //计算高度
        if (estSize.cy.IsStretch()) {
            if (bEstimateLayoutSize) {
                //拉伸类型的子控件，不计入， 如果指定最小值，则按最小值计算
                sz.cy = 0;
                if (sz.cy < pControl->GetMinHeight()) {
                    sz.cy = pControl->GetMinHeight();
                }
            }
            else {
                sz.cy = (CalcStretchValue(estSize.cy, szAvailable.cy) - rcMargin.top - rcMargin.bottom);
            }            
            sz.cy = std::max(sz.cy, 0);
        }
        if (sz.cy < pControl->GetMinHeight()) {
            sz.cy = pControl->GetMinHeight();
        }
        if (sz.cy > pControl->GetMaxHeight()) {
            sz.cy = pControl->GetMaxHeight();
        }
        if (sz.cy < 0) {
            sz.cy = 0;
        }
        estSize.cy.SetInt32(sz.cy);//cy是已经计算好的确定数值，不再有拉伸和自动类型值

        itemsMap[pControl] = estSize;
    }
    struct TColumnControls
    {
        //本列的数据所占的矩形范围
        UiRect m_columnRect;

        //本列的控件列表
        std::vector<Control*> m_pControlList;

        //本列控件的位置和大小
        std::vector<UiRect> m_pControlRects;
    };

    //分列
    int32_t nPosY = rc.top;
    int32_t nPosX = rc.left;
    std::vector<TColumnControls> columnControlList; //每一列中的控件的行高

    //总的行数
    size_t nRowCount = 1;

    const size_t nItemCount = items.size();
    for (size_t nItem = 0; nItem < nItemCount; ++nItem) {
        Control* pControl = items[nItem];
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }

        //对一个控件进行布局
        UiMargin rcMargin = pControl->GetMargin();//子控件的外边距
        nPosY += rcMargin.top;

        UiEstSize estSize = itemsMap[pControl];
        UiRect rcChild;
        rcChild.left = nPosX + rcMargin.left;
        rcChild.top = nPosY;
        rcChild.right = rcChild.left + estSize.cx.GetInt32();
        rcChild.bottom = rcChild.top + estSize.cy.GetInt32();

        nPosY += rcChild.Height();
        nPosY += rcMargin.bottom;
        nPosY += GetChildMarginY();

        //统计数据
        if (nRowCount != columnControlList.size()) {
            columnControlList.resize(nRowCount);
        }
        TColumnControls& controls = columnControlList[nRowCount - 1];
        controls.m_pControlList.push_back(pControl);
        controls.m_pControlRects.push_back(rcChild);

        //实际占用空间(含外边距)
        rcChild.top -= rcMargin.top;
        rcChild.bottom += rcMargin.bottom;
        rcChild.left -= rcMargin.left;
        rcChild.right += rcMargin.right;
        controls.m_columnRect.Union(rcChild);

        bool bNextColumn = false;
        if (nPosY > rc.bottom) {
            bNextColumn = true;
        }
        else {
            //判断下个控件是否能够容纳，如果超过边界，则需要换行
            Control* pNextControl = nullptr;//下一个控件
            size_t nNextItem = nItem + 1;
            while (nNextItem < nItemCount) {
                pNextControl = items[nNextItem];
                if (!pNextControl->IsVisible() || pNextControl->IsFloat()) {
                    pNextControl = nullptr;
                }
                if (pNextControl != nullptr) {
                    break;
                }
                else {
                    ++nNextItem;
                }
            }
            if (pNextControl != nullptr) {
                UiMargin rcNextMargin = pNextControl->GetMargin();//子控件的外边距
                UiEstSize estNextSize = itemsMap[pNextControl];
                int32_t nNextHeight = GetChildMarginY() + rcNextMargin.top + estNextSize.cy.GetInt32() + rcNextMargin.bottom;
                if ((nPosY + nNextHeight) > rc.bottom) {
                    bNextColumn = true;
                }
            }
        }
        
        if (bNextColumn) {
            //换列
            nRowCount += 1;
            nPosY = rc.top;
            nPosX += controls.m_columnRect.Width();
            nPosX += GetChildMarginX();
        }
    }

    UiRect childrenRect;
    for (const TColumnControls& control : columnControlList) {
        childrenRect.Union(control.m_columnRect);
    }
    const int32_t cxNeeded = childrenRect.Width(); //需要的总宽度
    const int32_t cyNeeded = childrenRect.Height();//需要的总高度
    UiSize64 szChildren((int64_t)cxNeeded, (int64_t)cyNeeded);
    UiPadding rcPadding;
    if (GetOwner() != nullptr) {
        rcPadding = GetOwner()->GetPadding();
    }
    if (szChildren.cx > 0) {
        szChildren.cx += ((int64_t)rcPadding.left + rcPadding.right);
    }
    if (szChildren.cy > 0) {
        szChildren.cy += ((int64_t)rcPadding.top + rcPadding.bottom);
    }

    if (bEstimateLayoutSize) {
        //评估模式, 直接返回结果（容器的宽度和高度设置为"auto"的情况）
        return szChildren;
    }

    //处理总体的对齐方式（在Box容器布局上设置的子控件对齐方式）
    if (cxNeeded < rcBox.Width()) {
        //在ScrollBox中，仅当不出现纵向滚动条时，垂直对齐方式才生效
        UiRect rcRealBox;
        bool bEnableHScrollBar = false;
        ScrollBox* pScrollBox = dynamic_cast<ScrollBox*>(GetOwner());
        if (pScrollBox != nullptr) {
            bEnableHScrollBar = pScrollBox->GetHScrollBar() != nullptr;
            rcRealBox = pScrollBox->GetPos();
        }
        if (!bEnableHScrollBar || (cxNeeded < rcRealBox.Width())) {
            //水平方向的对齐方式: 把所有项目当作一个整体排列
            int32_t nOffsetX = 0;
            const HorAlignType hAlignType = GetChildHAlignType();
            if (hAlignType == HorAlignType::kAlignCenter) {
                //垂直居中对齐
                nOffsetX = (rcBox.Width() - cxNeeded) / 2;
            }
            else if (hAlignType == HorAlignType::kAlignRight) {
                //靠右对齐
                nOffsetX = rcBox.Width() - cxNeeded;
            }
            if (nOffsetX != 0) {
                for (TColumnControls& control : columnControlList) {
                    control.m_columnRect.Offset(nOffsetX, 0);
                    for (UiRect& rcChild : control.m_pControlRects) {
                        rcChild.Offset(nOffsetX, 0);
                    }
                    //返回的区域大小，需要包含向右调整的偏移量
                    childrenRect.Union(control.m_columnRect);
                }
            }
        }
    }

    if ((GetChildVAlignType() == VerAlignType::kAlignCenter) || (GetChildVAlignType() == VerAlignType::kAlignBottom)) {
        //垂直方向的对齐方式: 按列设置对齐
        const VerAlignType vAlignType = GetChildVAlignType();
        for (TColumnControls& control : columnControlList) {
            UiRect& rowRect = control.m_columnRect;
            if (rowRect.Height() < rcBox.Height()) {
                int32_t nOffsetY = 0;
                if (vAlignType == VerAlignType::kAlignCenter) {
                    //垂直居中对齐
                    nOffsetY = (rcBox.Height() - rowRect.Height()) / 2;
                }
                else if (vAlignType == VerAlignType::kAlignBottom) {
                    //靠下对齐
                    nOffsetY = rcBox.Height() - rowRect.Height();
                }
                if (nOffsetY != 0) {
                    rowRect.Offset(0, nOffsetY);
                    for (UiRect& rcChild : control.m_pControlRects) {
                        rcChild.Offset(0, nOffsetY);
                    }

                    //返回的区域大小，需要包含向下调整的偏移量
                    childrenRect.Union(control.m_columnRect);
                }
            }
        }
    }

    //调整子控件的布局
    if (!bEstimateOnly) {
        //调整子控件的布局（非浮动控件）
        for (TColumnControls& control : columnControlList) {
            ASSERT(control.m_pControlRects.size() == control.m_pControlList.size());
            if (control.m_pControlRects.size() != control.m_pControlList.size()) {
                //错误
                return szChildren;
            }
            const UiRect& rowRect = control.m_columnRect;
            const size_t nCount = control.m_pControlList.size();
            for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
                Control* pControl = control.m_pControlList[nIndex];
                UiRect& rcChild = control.m_pControlRects[nIndex];

                //处理控件的对齐方式（控件本身设置的对齐方式）
                //只需要处理控件的水平方向对齐方式
                const HorAlignType hAlignType = pControl->GetHorAlignType();
                if ((hAlignType == HorAlignType::kAlignCenter) || (hAlignType == HorAlignType::kAlignRight)) {
                    UiRect rcFullRect = rowRect;
                    UiMargin rcMargin = pControl->GetMargin();//子控件的外边距
                    rcFullRect.left += rcMargin.left;
                    rcFullRect.right -= rcMargin.right;
                    if (rcChild.Width() < rcFullRect.Width()) {
                        int32_t nOffset = rcFullRect.Width() - rcChild.Width();
                        if (hAlignType == HorAlignType::kAlignCenter) {
                            rcChild.Offset(nOffset / 2, 0);
                        }
                        else if (hAlignType == HorAlignType::kAlignRight) {
                            rcChild.Offset(nOffset, 0);
                        }
                    }
                }

                //调整子控件的布局
                pControl->SetPos(rcChild);
            }
        }

        //调整子控件的布局（浮动控件）
        for (auto pControl : items) {
            if ((pControl == nullptr) || !pControl->IsVisible()) {
                continue;
            }
            if (pControl->IsFloat()) {
                //浮动控件（容器本身的对齐方式不生效）
                SetFloatPos(pControl, rc);
            }
        }
    }

    //需要重新计算，因为对齐方式引起区域的变化
    szChildren.cx = (int64_t)childrenRect.Width();
    szChildren.cy = (int64_t)childrenRect.Height();
    if (szChildren.cx > 0) {
        szChildren.cx += ((int64_t)rcPadding.left + rcPadding.right);
    }
    if (szChildren.cy > 0) {
        szChildren.cy += ((int64_t)rcPadding.top + rcPadding.bottom);
    }
    return szChildren;
}

UiSize64 VFlowLayout::ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly)
{
    return ArrangeChildInternal(items, rc, bEstimateOnly, false);
}

UiSize64 VFlowLayout::EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable)
{
    UiRect rc(0, 0, szAvailable.cx, szAvailable.cy);
    return ArrangeChildInternal(items, rc, true, true);
}

} // namespace ui

