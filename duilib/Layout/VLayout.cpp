#include "VLayout.h"
#include "duilib/Core/Box.h"
#include <map>

namespace ui 
{

VLayout::VLayout()
{
    //默认靠上对齐
    SetChildVAlignType(VerAlignType::kAlignTop);
}

/** 垂直布局算法描述
一、布局方案：
    按控件高度依次排列，顺序为：[控件上边距] + [控件高度] + [控件下边距] + GetChildMarginY () + ...（最后一个控件后无间距）。
二、有拉伸控件时，拉伸控件的高度按如下算法计算：
    1. 总可分配空间 = 总高度 (rc.Height ()) - 固定控件的总高度（自身高度，不含边距） - 所有控件的（上边距 + 下边距） - (总控件数 - 1) * GetChildMarginY ()。
    2. 控件的拉伸比例直接作用于总可分配空间（比例计算基数为 “总可分配空间”）。
    3. 高度受 min/max 限制：最终高度必须在 [minHeight, maxHeight] 范围内。
    4. 空间不足时（总可分配空间 < 所有拉伸控件的最小高度总和），按顺序优先满足上方控件的最小高度需求。
    5. 拉伸后的总高度不会超出容器范围（仅当固定控件总高度 <总高度 rc.Height () 时生效）。
    6. 如果所有拉伸控件的拉伸比例之和 ≥ 100%，且固定控件总高度 <总高度 rc.Height ()，
       则需保证布局后的总高度与 rc.Height () 相同（通过分配剩余空间或缩减超出空间实现）。
    7. 如果所有拉伸控件的拉伸比例之和 <100%，且固定控件总高度 < 总高度 rc.Height ()，
       则不需要保证布局后的总高度与 rc.Height () 相同（允许总高度 < rc.Height ()，剩余空间不强制分配）。
    8. 无论拉伸比例总和是否 ≥ 100%，若拉伸控件的目标高度总和超出总可分配空间（因 min/max 限制导致），
       均需缩减超出部分，确保总高度不超过容器范围（补充规则，与代码中 “空间超出缩减” 逻辑对应）。
    9. 如果控件的高度为0，则忽略布局，控件的外边距（上边距 + 下边距）不计入布局。
*/
UiSize64 VLayout::ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly)
{
    if (items.empty()) {
        return UiSize64();
    }
    DeflatePadding(rc);
    const UiSize szAvailable(rc.Width(), rc.Height());

    // 存储需要布局的控件尺寸（key：控件，value：宽高信息）
    std::unordered_map<Control*, UiEstSize> itemsMap;       // 非拉伸子控件
    std::vector<Control*> stretchControls;                  // 按顺序存储拉伸控件
    std::unordered_map<Control*, UiEstSize> stretchItemsMap;// 拉伸控件尺寸临时存储

    int32_t cyStretchPercentageTotal = 0;  // 拉伸控件总百分比（垂直方向）
    int32_t cyFixedSelfTotal = 0;          // 固定控件自身总高度（不含边距、不含间距）
    int32_t cyFixedTotal = 0;              // 固定控件总高度（含边距，不含间距）
    int32_t totalAllControlsCount = 0;     // 参与布局的总控件数（固定+拉伸，非浮动、可见）
    int32_t totalAllMargin = 0;            // 所有参与布局控件的边距总和（上+下）

    // 计算每个控件的基础尺寸，分类存储 + 统计关键参数
    for (auto pControl : items) {
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }

        totalAllControlsCount++;  // 统计参与布局的总控件数
        UiMargin rcMargin = pControl->GetMargin();
        totalAllMargin += rcMargin.top + rcMargin.bottom;  // 统计所有控件边距总和（上+下）

        UiEstSize estSize = pControl->EstimateSize(szAvailable);
        UiSize sz = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());

        // 处理宽度（拉伸类型）
        if (estSize.cx.IsStretch()) {
            sz.cx = (CalcStretchValue(estSize.cx, szAvailable.cx) - rcMargin.left - rcMargin.right);
        }
        // 限制宽度在最小/最大值范围内
        sz.cx = std::clamp(sz.cx, pControl->GetMinWidth(), pControl->GetMaxWidth());
        sz.cx = std::max(sz.cx, 0);
        estSize.cx.SetInt32(sz.cx);

        // 处理高度（固定/拉伸分类）
        if (estSize.cy.IsStretch()) {
            cyStretchPercentageTotal += estSize.cy.GetStretchPercentValue();
            stretchControls.push_back(pControl);
            stretchItemsMap[pControl] = estSize;
        }
        else {
            // 限制高度在最小/最大值范围内（固定控件自身高度）
            sz.cy = std::clamp(sz.cy, pControl->GetMinHeight(), pControl->GetMaxHeight());
            sz.cy = std::max(sz.cy, 0);
            estSize.cy.SetInt32(sz.cy);

            cyFixedSelfTotal += sz.cy;  // 累加固定控件自身总高度（不含边距）
            // 累加固定控件总高度（含边距，不含间距）
            if (sz.cy > 0) {
                cyFixedTotal += (sz.cy + rcMargin.top + rcMargin.bottom);
            }
            itemsMap[pControl] = estSize;
        }
    }

    // 确保拉伸百分比基数不小于100（仅用于计算占比，不影响规则判断）
    int32_t cyStretchPercentBase = std::max(cyStretchPercentageTotal, 100);

    // 处理拉伸控件的高度分配
    if ((cyFixedTotal < rc.Height()) && !stretchControls.empty()) {
        const int32_t totalSpacing = (totalAllControlsCount - 1) * GetChildMarginY();  // 总间距（所有参与布局控件，垂直方向用Y轴间距）

        // 一、计算总可分配空间（严格按公式，垂直方向用高度）
        // 总可分配空间 = 总高度 - 固定控件自身总高度 - 所有控件边距总和（上+下） - 总间距
        int32_t totalUsableSpace = rc.Height() - cyFixedSelfTotal - totalAllMargin - totalSpacing;
        totalUsableSpace = std::max(totalUsableSpace, 0);

        // 收集拉伸控件的最小/最大高度需求
        struct StretchInfo {
            Control* pCtrl;
            UiEstSize estSize;
            UiMargin margin;
            int32_t minHeight;   // 控件自身最小高
            int32_t maxHeight;   // 控件自身最大高
            int32_t finalHeight; // 最终高度（自身）
        };
        std::vector<StretchInfo> stretchInfos;
        int32_t totalStretchMin = 0;  // 拉伸控件最小高度总和（自身）

        for (auto pControl : stretchControls) {
            auto& estSize = stretchItemsMap[pControl];
            UiMargin margin = pControl->GetMargin();
            int32_t minH = pControl->GetMinHeight();
            int32_t maxH = pControl->GetMaxHeight();

            stretchInfos.push_back({
                pControl, estSize, margin,
                minH, maxH, 0
                });
            totalStretchMin += minH;
        }

        // 二、分场景分配高度（遵循所有规则）
        if (totalUsableSpace == 0) {
            // 场景0：总可分配空间为0，所有拉伸控件高度设为0（避免布局溢出）
            for (auto& info : stretchInfos) {
                info.finalHeight = 0;
            }
        }
        else if (totalStretchMin <= totalUsableSpace) {
            // 场景1：可用空间满足所有拉伸控件最小高度需求
            // 1. 按总可分配空间×比例计算目标高度（规则2）
            for (auto& info : stretchInfos) {
                int32_t targetHeight = static_cast<int32_t>((float)info.estSize.cy.GetStretchPercentValue() * totalUsableSpace / cyStretchPercentBase + 0.5f);
                // 2. 受min/max限制（规则3）
                info.finalHeight = std::clamp(targetHeight, info.minHeight, info.maxHeight);
            }

            // 3. 计算当前拉伸控件总高度，处理剩余/超出空间（规则6、7、8）
            int32_t currentStretchTotal = 0;
            for (auto& info : stretchInfos) {
                currentStretchTotal += info.finalHeight;
            }

            int32_t diff = totalUsableSpace - currentStretchTotal;
            if (diff > 0) {
                // 空间有剩余：仅比例≥100时强制分配（规则6）
                if (cyStretchPercentageTotal >= 100) {
                    // 计算总可分配的额外空间（所有控件的maxHeight - finalHeight之和）
                    int64_t totalAdjustableSpace = 0;
                    for (auto& info : stretchInfos) {
                        totalAdjustableSpace += (info.maxHeight - info.finalHeight);
                    }
                    if (totalAdjustableSpace <= 0) {
                        // 无可用调整空间，直接退出
                    }
                    else {
                        int32_t remainingDiff = diff;
                        // 批量分配：按比例分配大部分diff
                        for (auto& info : stretchInfos) {
                            if (remainingDiff <= 0) break;
                            int64_t ctrlAdjustable = info.maxHeight - info.finalHeight;
                            if (ctrlAdjustable <= 0) continue;

                            // 按比例分配当前控件可调整的空间（避免浮点精度问题，用整数运算优化）
                            int64_t batchAdd = (ctrlAdjustable * remainingDiff) / totalAdjustableSpace;
                            // 修正计算偏差（确保不超出剩余diff和控件可调整空间）
                            batchAdd = std::min({ batchAdd, (int64_t)remainingDiff, ctrlAdjustable });
                            if (batchAdd > 0) {
                                // 校验batchAdd是否在int32_t范围内
                                ASSERT(batchAdd <= static_cast<int64_t>(INT32_MAX));
                                int32_t batchAdd32 = ui::TruncateToInt32(batchAdd);
                                info.finalHeight += batchAdd32;
                                remainingDiff -= batchAdd32;
                                totalAdjustableSpace -= batchAdd;
                            }
                        }

                        // 剩余少量diff（通常0-总控件数），逐像素分配
                        while (remainingDiff > 0) {
                            bool allocated = false;
                            for (auto& info : stretchInfos) {
                                if (info.finalHeight < info.maxHeight) {
                                    info.finalHeight += 1;
                                    remainingDiff -= 1;
                                    allocated = true;
                                    if (remainingDiff <= 0) break;
                                }
                            }
                            if (!allocated) break;
                        }
                    }
                }
            }
            else if (diff < 0) {
                // 空间超出：无论比例是否≥100，均需缩减（规则8）
                int32_t reduceDiff = -diff; // 避免负数值操作
                // 计算总可缩减的空间（所有控件的finalHeight - minHeight之和）
                int64_t totalAdjustableSpace = 0;
                for (auto& info : stretchInfos) {
                    totalAdjustableSpace += (info.finalHeight - info.minHeight);
                }
                if (totalAdjustableSpace <= 0) {
                    // 无可用调整空间，直接退出
                }
                else {
                    int32_t remainingDiff = reduceDiff;
                    // 批量缩减：按比例缩减大部分diff
                    for (auto& info : stretchInfos) {
                        if (remainingDiff <= 0) break;
                        int64_t ctrlAdjustable = info.finalHeight - info.minHeight;
                        if (ctrlAdjustable <= 0) continue;

                        // 按比例缩减当前控件可调整的空间（整数运算避免精度问题）
                        int64_t batchSub = (ctrlAdjustable * remainingDiff) / totalAdjustableSpace;
                        // 修正计算偏差（确保不超出剩余diff和控件可调整空间）
                        batchSub = std::min({ batchSub, (int64_t)remainingDiff, ctrlAdjustable });
                        if (batchSub > 0) {
                            // 校验batchSub是否在int32_t范围内
                            ASSERT(batchSub <= static_cast<int64_t>(INT32_MAX));
                            int32_t batchSub32 = ui::TruncateToInt32(batchSub);
                            info.finalHeight -= batchSub32;
                            remainingDiff -= batchSub32;
                            totalAdjustableSpace -= batchSub;
                        }
                    }

                    // 剩余少量diff（通常0-总控件数），逐像素缩减
                    while (remainingDiff > 0) {
                        bool reduced = false;
                        for (auto& info : stretchInfos) {
                            if (info.finalHeight > info.minHeight) {
                                info.finalHeight -= 1;
                                remainingDiff -= 1;
                                reduced = true;
                                if (remainingDiff <= 0) break;
                            }
                        }
                        if (!reduced) break; // 无可用调整空间
                    }
                }
            }
        }
        else {
            // 场景2：可用空间不足，优先显示上面控件（垂直方向顺序）
            int32_t remainingSpace = totalUsableSpace;
            for (auto& info : stretchInfos) {
                if (remainingSpace <= 0) break;

                // 优先分配最小高度
                int32_t allocHeight = std::min(info.minHeight, remainingSpace);
                info.finalHeight = allocHeight;
                remainingSpace -= allocHeight;
            }

            // 若仍有剩余空间，分配给上面的控件（按比例）
            if (remainingSpace > 0) {
                for (auto& info : stretchInfos) {
                    if (remainingSpace <= 0) break;
                    if (info.finalHeight >= info.maxHeight) continue;

                    int32_t maxAdd = info.maxHeight - info.finalHeight;
                    int32_t addHeight = static_cast<int32_t>(
                        1.0f * info.estSize.cy.GetStretchPercentValue() * remainingSpace / cyStretchPercentBase + 0.5f
                        );
                    addHeight = std::min({ addHeight, maxAdd, remainingSpace });

                    info.finalHeight += addHeight;
                    remainingSpace -= addHeight;
                }
            }
        }

        // 更新拉伸控件尺寸到itemsMap
        for (auto& info : stretchInfos) {
            info.estSize.cy.SetInt32(info.finalHeight);
            itemsMap[info.pCtrl] = info.estSize;
        }
    }

    // 处理未分配到空间的拉伸控件（设为0高度）
    for (auto pControl : stretchControls) {
        if (itemsMap.find(pControl) == itemsMap.end()) {
            UiEstSize estSize;
            estSize.cy.SetInt32(0);
            itemsMap[pControl] = estSize;
        }
    }
    stretchItemsMap.clear();
    stretchControls.clear();

    // 计算子控件的最终位置（按：上边距 + 控件高度 + 下边距 + 间距 的顺序）
    std::vector<Control*> childrenControlList;  // 控件列表
    std::vector<UiRect> childrenControlRects;   // 控件的位置和大小
    UiRect childrenRect;                        // 子控件原始范围（未对齐前）
    int32_t nPosX = rc.left;
    int32_t nPosY = rc.top;
    bool isFirstControl = true; // 标记是否为第一个控件，第一个控件前不需要添加间距

    for (auto pControl : items) {
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }

        UiRect rcChild;
        auto estSizeIter = itemsMap.find(pControl);
        if ((estSizeIter == itemsMap.end()) || (estSizeIter->second.cy.GetInt32() <= 0)) {
            // 高度为0，忽略布局
            rcChild = { nPosX, nPosY, nPosX, nPosY };
        }
        else {
            // 对一个控件进行布局（严格按布局方案）
            const UiEstSize& estSize = estSizeIter->second;
            UiMargin rcMargin = pControl->GetMargin();

            // 统一处理间距（非第一个控件前添加）
            if (!isFirstControl) {
                nPosY += GetChildMarginY();  // 垂直方向用Y轴间距
            }
            isFirstControl = false;

            // 垂直位置计算（含边距）
            nPosY += rcMargin.top;           // 上边距
            rcChild.top = nPosY;
            rcChild.bottom = rcChild.top + estSize.cy.GetInt32(); // 控件自身高度
            nPosY = rcChild.bottom + rcMargin.bottom; // 下边距

            // 水平位置计算（处理对齐）
            rcChild.left = nPosX + rcMargin.left;
            rcChild.right = rcChild.left + estSize.cx.GetInt32();

            // 处理水平对齐（基于扣除内边距的容器范围）
            HorAlignType hAlign = pControl->GetHorAlignType();
            if (hAlign == HorAlignType::kAlignCenter || hAlign == HorAlignType::kAlignRight) {
                UiRect rcValidArea = rc; // 容器有效范围（已扣内边距）

                // 剪去控件的外边距
                rcValidArea.left += rcMargin.left;
                rcValidArea.right -= rcMargin.right;

                int32_t nOffset = rcValidArea.Width() - rcChild.Width();
                if (nOffset > 0) {
                    if (hAlign == HorAlignType::kAlignCenter) {
                        rcChild.Offset(nOffset / 2, 0);
                    }
                    else if (hAlign == HorAlignType::kAlignRight) {
                        rcChild.Offset(nOffset, 0);
                    }
                }
            }

            // 更新子控件原始范围（含边距）
            UiRect rcChildWithMargin = rcChild;
            rcChildWithMargin.top -= rcMargin.top;
            rcChildWithMargin.bottom += rcMargin.bottom;
            rcChildWithMargin.left -= rcMargin.left;
            rcChildWithMargin.right += rcMargin.right;
            childrenRect.Union(rcChildWithMargin);
        }

        childrenControlList.push_back(pControl);
        childrenControlRects.push_back(rcChild);
    }

    // 处理子控件总体垂直对齐（居中/靠下）
    UiRect fullChildrenRect = childrenRect;     // 子控件总体范围（含对齐偏移）
    UiRect alignedChildrenRect = childrenRect;  // 子控件对齐后实际范围
    const VerAlignType vAlign = GetChildVAlignType();  // 垂直对齐类型
    int32_t containerHeight = rc.Height();

    if (((vAlign == VerAlignType::kAlignCenter) || (vAlign == VerAlignType::kAlignBottom)) &&
        childrenRect.Height() > 0 &&
        childrenRect.Height() < containerHeight) {

        int32_t nOffsetY = 0;
        if (vAlign == VerAlignType::kAlignCenter) {
            nOffsetY = (containerHeight - childrenRect.Height()) / 2;
        }
        else if (vAlign == VerAlignType::kAlignBottom) {
            nOffsetY = containerHeight - childrenRect.Height();
        }

        // 应用垂直偏移
        if (nOffsetY != 0) {
            alignedChildrenRect.Offset(0, nOffsetY);
            for (UiRect& rcChild : childrenControlRects) {
                rcChild.Offset(0, nOffsetY);
            }
            fullChildrenRect.Union(alignedChildrenRect);
        }
    }

    // 应用最终布局（非估算模式）
    if (!bEstimateOnly) {
        // 处理非浮动控件
        ASSERT(childrenControlList.size() == childrenControlRects.size());
        if (childrenControlList.size() == childrenControlRects.size()) {
            const size_t nCount = childrenControlList.size();
            for (size_t i = 0; i < nCount; ++i) {
                Control* pControl = childrenControlList[i];
                pControl->SetPos(childrenControlRects[i]);
            }
        }

        // 处理浮动控件
        for (auto pControl : items) {
            if ((pControl != nullptr) && pControl->IsVisible() && pControl->IsFloat()) {
                SetFloatPos(pControl, rc);  // 浮动控件位置计算逻辑与水平布局一致（通常基于父容器）
            }
        }
    }

    // 计算最终返回的尺寸（含父容器内边距）
    UiPadding rcPadding = (GetOwner() != nullptr) ? GetOwner()->GetPadding() : UiPadding();
    UiSize64 resultSize(
        (int64_t)std::max(fullChildrenRect.Width() + rcPadding.left + rcPadding.right, 0),
        (int64_t)std::max(fullChildrenRect.Height() + rcPadding.top + rcPadding.bottom, 0)
    );
    return resultSize;
}

UiSize64 VLayout::EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable)
{
    // 宽度：所有有效子控件宽度的最大值（含边距）+ 容器内边距
    // 高度：所有有效子控件高度之和（含边距）+ 子控件间隔 + 容器内边距
    // 有效子控件：可见、非浮动，拉伸控件取最小尺寸参与计算
    UiSize64 totalSize(0, 0);  // 64位计算避免溢出
    int32_t validCount = 0;    // 统计参与间隔计算的有效控件（有高度或边距）

    for (Control* pControl : items) {
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }

        UiMargin rcMargin = pControl->GetMargin();
        UiEstSize estSize = pControl->EstimateSize(szAvailable);
        int32_t minWidth = pControl->GetMinWidth();
        int32_t maxWidth = pControl->GetMaxWidth();
        int32_t minHeight = pControl->GetMinHeight();
        int32_t maxHeight = pControl->GetMaxHeight();

        // 处理宽度（含拉伸逻辑和边界限制）
        int32_t itemWidth = 0;
        if (estSize.cx.IsStretch()) {
            itemWidth = std::max(minWidth, 0);  // 拉伸控件取最小宽度（至少0）
        }
        else {
            // 非拉伸控件：用std::clamp限制在[minWidth, maxWidth]范围内
            itemWidth = std::clamp(estSize.cx.GetInt32(), minWidth, maxWidth);
        }

        // 处理高度（含拉伸逻辑和边界限制）
        int32_t itemHeight = 0;
        if (estSize.cy.IsStretch()) {
            itemHeight = std::max(minHeight, 0);  // 拉伸控件取最小高度（至少0）
        }
        else {
            // 非拉伸控件：用std::clamp限制在[minHeight, maxHeight]范围内
            itemHeight = std::clamp(estSize.cy.GetInt32(), minHeight, maxHeight);
        }

        // 计算最大宽度（含控件宽度和左右边距，无条件参与比较）
        if (itemWidth > 0) {
            int64_t widthWithMargin = (int64_t)itemWidth + rcMargin.left + rcMargin.right;
            if (widthWithMargin > totalSize.cx) {
                totalSize.cx = widthWithMargin;
            }
        }

        // 累加高度（含控件高度和上下边距，无条件计入）
        if (itemHeight > 0) {
            int64_t heightWithMargin = (int64_t)itemHeight + rcMargin.top + rcMargin.bottom;
            totalSize.cy += heightWithMargin;

            // 统计有效控件（有高度或边距时，参与间隔计算）
            validCount++;
        }
    }

    // 累加子控件间隔（有效控件数>1时，垂直方向用ChildMarginY）
    if (validCount > 1) {
        totalSize.cy += (int64_t)(validCount - 1) * GetChildMarginY();
    }

    // 累加容器内边距（无条件计入）
    UiPadding rcPadding;
    if (GetOwner() != nullptr) {
        rcPadding = GetOwner()->GetPadding();
    }
    totalSize.cx += rcPadding.left + rcPadding.right;
    totalSize.cy += rcPadding.top + rcPadding.bottom;

    // 仅当宽高均为0时检查配置（避免误判有效拉伸布局）
    if (totalSize.cx == 0 && totalSize.cy == 0) {
        CheckConfig(items);
    }

    return totalSize;
}

} // namespace ui
