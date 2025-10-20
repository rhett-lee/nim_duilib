#include "HFlowLayout.h"
#include "duilib/Box/ScrollBox.h"
#include <unordered_map>

namespace ui 
{

HFlowLayout::HFlowLayout()
{
    //默认靠左对齐
    SetChildHAlignType(HorAlignType::kAlignLeft);
}

UiSize64 HFlowLayout::ArrangeChildInternal(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly, bool bEstimateLayoutSize) const
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
    std::unordered_map<Control*, UiEstSize> itemsMap;

    // 预处理：筛选所有可见、非浮动的控件
    std::vector<Control*> visibleControls;
    for (auto pControl : items) {
        if ((pControl != nullptr) && pControl->IsVisible() && !pControl->IsFloat()) {
            visibleControls.push_back(pControl);
        }
    }

    //计算每个控件的宽度和高度，并记录到Map中
    for (auto pControl : visibleControls) {
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
                sz.cx = (CalcStretchValue(estSize.cx, szAvailable.cx) - rcMargin.left - rcMargin.right);
            }
            sz.cx = std::max(sz.cx, 0);
        }
        sz.cx = std::clamp(sz.cx, pControl->GetMinWidth(), pControl->GetMaxWidth());
        sz.cx = std::max(sz.cx, 0);
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
                if (estSize.cy.GetStretchPercentValue() >= 100) {
                    //高度完全拉伸：无法显示
                    ASSERT(0);
                    sz.cy = 0;
                }
                else {
                    sz.cy = (CalcStretchValue(estSize.cy, szAvailable.cy) - rcMargin.top - rcMargin.bottom);
                }
            }
            sz.cy = std::max(sz.cy, 0);
        }
        sz.cy = std::clamp(sz.cy, pControl->GetMinHeight(), pControl->GetMaxHeight());
        sz.cy = std::max(sz.cy, 0);
        estSize.cy.SetInt32(sz.cy);//cy是已经计算好的确定数值，不再有拉伸和自动类型值

        itemsMap[pControl] = estSize;
    }
    struct TRowControls
    {
        //本行的数据所占的矩形范围
        UiRect m_rowRect;

        //本行的控件列表
        std::vector<Control*> m_pControlList;

        //本行控件的位置和大小
        std::vector<UiRect> m_pControlRects;
    };

    //分行
    int32_t nPosY = rc.top;
    int32_t nPosX = rc.left;
    std::vector<TRowControls> rowControlList; //每一行中的控件和行高

    //总的列数
    size_t nColumnCount = 1;

    const size_t nItemCount = visibleControls.size();
    for (size_t nItem = 0; nItem < nItemCount; ++nItem) {
        Control* pControl = visibleControls[nItem];

        //标记是否为最后一个控件
        bool bLastControl = (nItem == (nItemCount - 1)) ? true : false;

        //对一个控件进行布局
        UiMargin rcMargin = pControl->GetMargin();//子控件的外边距
        nPosX += rcMargin.left;

        UiEstSize estSize = itemsMap[pControl];
        UiRect rcChild;
        rcChild.left = nPosX;
        rcChild.top = nPosY + rcMargin.top;
        rcChild.right = rcChild.left + estSize.cx.GetInt32();
        rcChild.bottom = rcChild.top + estSize.cy.GetInt32();

        nPosX += rcChild.Width();
        nPosX += rcMargin.right;

        //统计数据
        if (nColumnCount != rowControlList.size()) {
            rowControlList.resize(nColumnCount);
        }
        TRowControls& controls = rowControlList[nColumnCount - 1];
        controls.m_pControlList.push_back(pControl);
        controls.m_pControlRects.push_back(rcChild);

        //实际占用空间(含外边距)
        rcChild.top -= rcMargin.top;
        rcChild.bottom += rcMargin.bottom;
        rcChild.left -= rcMargin.left;
        rcChild.right += rcMargin.right;
        controls.m_rowRect.Union(rcChild);

        if (!bLastControl) {
            //还未到达最后一个控件
            nPosX += GetChildMarginX();//增加控件间距

            bool bNeedNewRow = false;
            if (nPosX > rc.right) {
                bNeedNewRow = true;
            }
            else if (nItem < (nItemCount - 1)) {
                //判断下个控件是否能够容纳，如果超过边界，则需要换行
                Control* pNextControl = visibleControls[nItem + 1];//下一个控件            
                if (pNextControl != nullptr) {
                    UiMargin rcNextMargin = pNextControl->GetMargin();//子控件的外边距
                    UiEstSize estNextSize = itemsMap[pNextControl];
                    int32_t nNextWidth = rcNextMargin.left + estNextSize.cx.GetInt32() + rcNextMargin.right;
                    if ((nPosX + nNextWidth) > rc.right) {
                        bNeedNewRow = true;
                    }
                }
            }

            if (bNeedNewRow) {
                //换行
                nColumnCount += 1;
                nPosX = rc.left;
                nPosY += controls.m_rowRect.Height();
                nPosY += GetChildMarginY();
            }
        }
    }

    UiRect childrenRect;
    for (const TRowControls& control : rowControlList) {
        childrenRect.Union(control.m_rowRect);
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
    if ((GetChildHAlignType() == HorAlignType::kAlignCenter) || (GetChildHAlignType() == HorAlignType::kAlignRight)) {
        const HorAlignType hAlignType = GetChildHAlignType();
        //水平方向的对齐方式: 按行设置对齐
        for (TRowControls& control : rowControlList) {
            UiRect& rowRect = control.m_rowRect;
            if (rowRect.Width() < rcBox.Width()) {
                int32_t nOffsetX = 0;
                if (hAlignType == HorAlignType::kAlignCenter) {
                    //水平居中对齐
                    nOffsetX = (rcBox.Width() - rowRect.Width()) / 2;
                }
                else if (hAlignType == HorAlignType::kAlignRight) {
                    //靠右对齐
                    nOffsetX = rcBox.Width() - rowRect.Width();
                }
                if (nOffsetX != 0) {
                    rowRect.Offset(nOffsetX, 0);
                    for (UiRect& rcChild : control.m_pControlRects) {
                        rcChild.Offset(nOffsetX, 0);
                    }

                    //返回的区域大小，需要包含向右调整的偏移量
                    childrenRect.Union(control.m_rowRect);
                }
            }
        }        
    }

    if (cyNeeded < rcBox.Height()) {
        //在ScrollBox中，仅当不出现纵向滚动条时，垂直对齐方式才生效
        UiRect rcRealBox;
        bool bEnableVScrollBar = false;
        ScrollBox* pScrollBox = dynamic_cast<ScrollBox*>(GetOwner());
        if (pScrollBox != nullptr) {
            bEnableVScrollBar = pScrollBox->GetVScrollBar() != nullptr;
            rcRealBox = pScrollBox->GetPos();
        }
        if (!bEnableVScrollBar || (cyNeeded < rcRealBox.Height())) {
            //垂直方向的对齐方式: 把所有项目当作一个整体排列
            int32_t nOffsetY = 0;
            const VerAlignType vAlignType = GetChildVAlignType();
            if (vAlignType == VerAlignType::kAlignCenter) {
                //垂直居中对齐
                nOffsetY = (rcBox.Height() - cyNeeded) / 2;
            }
            else if (vAlignType == VerAlignType::kAlignBottom) {
                //靠下对齐
                nOffsetY = rcBox.Height() - cyNeeded;
            }
            if (nOffsetY != 0) {
                for (TRowControls& control : rowControlList) {
                    control.m_rowRect.Offset(0, nOffsetY);
                    for (UiRect& rcChild : control.m_pControlRects) {
                        rcChild.Offset(0, nOffsetY);
                    }

                    //返回的区域大小，需要包含向下调整的偏移量
                    childrenRect.Union(control.m_rowRect);
                }
            }
        }
    }

    //调整子控件的布局
    if (!bEstimateOnly) {
        //调整子控件的布局（非浮动控件）
        for (TRowControls& control : rowControlList) {
            ASSERT(control.m_pControlRects.size() == control.m_pControlList.size());
            if (control.m_pControlRects.size() != control.m_pControlList.size()) {
                //错误
                return szChildren;
            }
            const UiRect& rowRect = control.m_rowRect;
            const size_t nCount = control.m_pControlList.size();
            for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
                Control* pControl = control.m_pControlList[nIndex];
                UiRect& rcChild = control.m_pControlRects[nIndex];

                ////处理控件的对齐方式（控件本身设置的对齐方式）
                //只需要处理控件的垂直方向对齐方式
                const VerAlignType vAlignType = pControl->GetVerAlignType();
                if ((vAlignType == VerAlignType::kAlignCenter) || (vAlignType == VerAlignType::kAlignBottom)) {
                    UiRect rcFullRect = rowRect;
                    UiMargin rcMargin = pControl->GetMargin();//子控件的外边距
                    rcFullRect.top += rcMargin.top;
                    rcFullRect.bottom -= rcMargin.bottom;
                    if (rcChild.Height() < rcFullRect.Height()) {
                        int32_t nOffset = rcFullRect.Height() - rcChild.Height();
                        if (vAlignType == VerAlignType::kAlignCenter) {
                            rcChild.Offset(0, nOffset / 2);
                        }
                        else if (vAlignType == VerAlignType::kAlignBottom) {
                            rcChild.Offset(0, nOffset);
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

UiSize64 HFlowLayout::ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly)
{
    return ArrangeChildInternal(items, rc, bEstimateOnly, false);
}

UiSize64 HFlowLayout::EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable)
{
    UiRect rc(0, 0, szAvailable.cx, szAvailable.cy);
    return ArrangeChildInternal(items, rc, true, true);
}

} // namespace ui
