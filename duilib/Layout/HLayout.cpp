#include "HLayout.h"
#include "duilib/Core/Box.h"
#include <unordered_map>

namespace ui 
{

HLayout::HLayout()
{
    //默认靠左对齐
    SetChildHAlignType(HorAlignType::kAlignLeft);
}

/** 布局算法描述
一、布局方案：
    按控件宽度依次排列，顺序为：[控件左边距] + [控件宽度] + [控件右边距] + GetChildMarginX() + ...（最后一个控件后无间距）。
二、有拉伸控件时，拉伸控件的宽度按如下算法计算：
    1. 总可分配空间 = 总宽度 (rc.Width ()) - 固定控件的总宽度（自身宽度，不含边距） - 所有控件的（左边距 + 右边距） - (总控件数 - 1) * GetChildMarginX ()。
    2. 控件的拉伸比例直接作用于总可分配空间（比例计算基数为 “总可分配空间”）。
    3. 宽度受 min/max 限制：最终宽度必须在 [minWidth, maxWidth] 范围内。
    4. 空间不足时（总可分配空间 < 所有拉伸控件的最小宽度总和），按顺序优先满足前面控件的最小宽度需求。
    5. 拉伸后的总宽度不会超出容器范围（仅当固定控件总宽度 <总宽度 rc.Width () 时生效）。
    6. 如果所有拉伸控件的拉伸比例之和 ≥ 100%，且固定控件总宽度 <总宽度 rc.Width ()，
       则需保证布局后的总宽度与 rc.Width () 相同（通过分配剩余空间或缩减超出空间实现）。
    7. 如果所有拉伸控件的拉伸比例之和 <100%，且固定控件总宽度 < 总宽度 rc.Width ()，
       则不需要保证布局后的总宽度与 rc.Width () 相同（允许总宽度 < rc.Width ()，剩余空间不强制分配）。
    8. 无论拉伸比例总和是否 ≥ 100%，若拉伸控件的目标宽度总和超出总可分配空间（因 min/max 限制导致），
       均需缩减超出部分，确保总宽度不超过容器范围（补充规则，与代码中 “空间超出缩减” 逻辑对应）。
    9. 如果控件的宽度为0，则忽略布局，控件的外边距（左边距 + 右边距）不计入布局。
*/
UiSize64 HLayout::ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly)
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

    int32_t cxStretchPercentageTotal = 0;  // 拉伸控件总百分比
    int32_t cxFixedSelfTotal = 0;          // 固定控件自身总宽度（不含边距、不含间距）
    int32_t cxFixedTotal = 0;              // 固定控件总宽度（含边距，不含间距）
    int32_t totalAllControlsCount = 0;     // 参与布局的总控件数（固定+拉伸，非浮动、可见）
    int32_t totalAllMargin = 0;            // 所有参与布局控件的边距总和（左+右）

    // 计算每个控件的基础尺寸，分类存储 + 统计关键参数
    for (auto pControl : items) {
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }

        totalAllControlsCount++;  // 统计参与布局的总控件数
        UiMargin rcMargin = pControl->GetMargin();
        totalAllMargin += rcMargin.left + rcMargin.right;  // 统计所有控件边距总和

        UiEstSize estSize = pControl->EstimateSize(szAvailable);
        UiSize sz = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());

        // 处理高度（拉伸类型）
        if (estSize.cy.IsStretch()) {
            sz.cy = (CalcStretchValue(estSize.cy, szAvailable.cy) - rcMargin.top - rcMargin.bottom);
        }
        // 限制高度在最小/最大值范围内
        sz.cy = std::clamp(sz.cy, pControl->GetMinHeight(), pControl->GetMaxHeight());
        sz.cy = std::max(sz.cy, 0);
        estSize.cy.SetInt32(sz.cy);

        // 处理宽度（固定/拉伸分类）
        if (estSize.cx.IsStretch()) {
            cxStretchPercentageTotal += estSize.cx.GetStretchPercentValue();
            stretchControls.push_back(pControl);
            stretchItemsMap[pControl] = estSize;
        }
        else {
            // 限制宽度在最小/最大值范围内（固定控件自身宽度）
            sz.cx = std::clamp(sz.cx, pControl->GetMinWidth(), pControl->GetMaxWidth());
            sz.cx = std::max(sz.cx, 0);
            estSize.cx.SetInt32(sz.cx);

            cxFixedSelfTotal += sz.cx;  // 累加固定控件自身总宽度（不含边距）
            // 累加固定控件总宽度（含边距，不含间距）
            if (sz.cx > 0) {
                cxFixedTotal += (sz.cx + rcMargin.left + rcMargin.right);
            }
            itemsMap[pControl] = estSize;
        }
    }

    // 确保拉伸百分比基数不小于100（仅用于计算占比，不影响规则判断）
    int32_t cxStretchPercentBase = std::max(cxStretchPercentageTotal, 100);

    // 处理拉伸控件的宽度分配
    if ((cxFixedTotal < rc.Width()) && !stretchControls.empty()) {
        const int32_t totalSpacing = (totalAllControlsCount - 1) * GetChildMarginX();  // 总间距（所有参与布局控件）

        // 一、计算总可分配空间（严格按公式）
        // 总可分配空间 = 总宽度 - 固定控件自身总宽度 - 所有控件边距总和 - 总间距
        int32_t totalUsableSpace = rc.Width() - cxFixedSelfTotal - totalAllMargin - totalSpacing;
        totalUsableSpace = std::max(totalUsableSpace, 0);

        // 收集拉伸控件的最小/最大宽度需求
        struct StretchInfo {
            Control* pCtrl;
            UiEstSize estSize;
            UiMargin margin;
            int32_t minWidth;    // 控件自身最小宽
            int32_t maxWidth;    // 控件自身最大宽
            int32_t finalWidth;  // 最终宽度（自身）
        };
        std::vector<StretchInfo> stretchInfos;
        int32_t totalStretchMin = 0;  // 拉伸控件最小宽度总和（自身）
        int32_t totalStretchMax = 0;  // 拉伸控件最大宽度总和（自身）

        for (auto pControl : stretchControls) {
            auto& estSize = stretchItemsMap[pControl];
            UiMargin margin = pControl->GetMargin();
            int32_t minW = pControl->GetMinWidth();
            int32_t maxW = pControl->GetMaxWidth();

            stretchInfos.push_back({
                pControl, estSize, margin,
                minW, maxW, 0
                });
            totalStretchMin += minW;
            totalStretchMax += maxW;
        }

        // 二、分场景分配宽度（遵循所有规则）
        if (totalUsableSpace == 0) {
            // 场景0：总可分配空间为0，所有拉伸控件宽度设为0（避免布局溢出）
            for (auto& info : stretchInfos) {
                info.finalWidth = 0;
            }
        }
        else if (totalStretchMin <= totalUsableSpace) {
            // 场景1：可用空间满足所有拉伸控件最小宽度需求
            // 1. 按总可分配空间×比例计算目标宽度（规则2）
            for (auto& info : stretchInfos) {
                int32_t targetWidth = static_cast<int32_t>((float)info.estSize.cx.GetStretchPercentValue() * totalUsableSpace / cxStretchPercentBase + 0.5f);
                // 2. 受min/max限制（规则3）
                info.finalWidth = std::clamp(targetWidth, info.minWidth, info.maxWidth);
            }

            // 3. 计算当前拉伸控件总宽度，处理剩余/超出空间（规则6、7、8）
            int32_t currentStretchTotal = 0;
            for (auto& info : stretchInfos) {
                currentStretchTotal += info.finalWidth;
            }

            int32_t diff = totalUsableSpace - currentStretchTotal;
            if (diff > 0) {
                // 空间有剩余：仅比例≥100时强制分配（规则6）
                if (cxStretchPercentageTotal >= 100) {
                    // 计算总可分配的额外空间（所有控件的maxWidth - finalWidth之和）
                    int64_t totalAdjustableSpace = 0;
                    for (auto& info : stretchInfos) {
                        totalAdjustableSpace += (info.maxWidth - info.finalWidth);
                    }
                    if (totalAdjustableSpace <= 0) {
                        // 无可用调整空间，直接退出
                    }
                    else {
                        int32_t remainingDiff = diff;
                        // 批量分配：按比例分配大部分diff
                        for (auto& info : stretchInfos) {
                            if (remainingDiff <= 0) break;
                            int64_t ctrlAdjustable = info.maxWidth - info.finalWidth;
                            if (ctrlAdjustable <= 0) continue;

                            // 按比例分配当前控件可调整的空间（避免浮点精度问题，用整数运算优化）
                            int64_t batchAdd = (ctrlAdjustable * remainingDiff) / totalAdjustableSpace;
                            // 修正计算偏差（确保不超出剩余diff和控件可调整空间）
                            batchAdd = std::min({ batchAdd, (int64_t)remainingDiff, ctrlAdjustable });
                            if (batchAdd > 0) {
                                // 校验batchAdd是否在int32_t范围内
                                ASSERT(batchAdd <= static_cast<int64_t>(INT32_MAX));
                                int32_t batchAdd32 = ui::TruncateToInt32(batchAdd);
                                info.finalWidth += batchAdd32;
                                remainingDiff -= batchAdd32;
                                totalAdjustableSpace -= batchAdd;
                            }
                        }

                        // 剩余少量diff（通常0-总控件数），逐像素分配
                        while (remainingDiff > 0) {
                            bool allocated = false;
                            for (auto& info : stretchInfos) {
                                if (info.finalWidth < info.maxWidth) {
                                    info.finalWidth += 1;
                                    remainingDiff -= 1;
                                    allocated = true;
                                    if (remainingDiff <= 0) break;
                                }
                            }
                            if (!allocated) break; // 无可用调整空间
                        }
                    }
                }
            }
            else if (diff < 0) {
                // 空间超出：无论比例是否≥100，均需缩减（规则8）
                int32_t reduceDiff = -diff; // 避免负数值操作
                // 计算总可缩减的空间（所有控件的finalWidth - minWidth之和）
                int64_t totalAdjustableSpace = 0;
                for (auto& info : stretchInfos) {
                    totalAdjustableSpace += (info.finalWidth - info.minWidth);
                }
                if (totalAdjustableSpace <= 0) {
                    // 无可用调整空间，直接退出
                }
                else {
                    int32_t remainingDiff = reduceDiff;
                    // 批量缩减：按比例缩减大部分diff
                    for (auto& info : stretchInfos) {
                        if (remainingDiff <= 0) break;
                        int64_t ctrlAdjustable = info.finalWidth - info.minWidth;
                        if (ctrlAdjustable <= 0) continue;

                        // 按比例缩减当前控件可调整的空间（整数运算避免精度问题）
                        int64_t batchSub = (ctrlAdjustable * remainingDiff) / totalAdjustableSpace;
                        // 修正计算偏差（确保不超出剩余diff和控件可调整空间）
                        batchSub = std::min({ batchSub, (int64_t)remainingDiff, ctrlAdjustable });
                        if (batchSub > 0) {
                            // 校验batchSub是否在int32_t范围内
                            ASSERT(batchSub <= static_cast<int64_t>(INT32_MAX));
                            int32_t batchSub32 = ui::TruncateToInt32(batchSub);
                            info.finalWidth -= batchSub32;
                            remainingDiff -= batchSub32;
                            totalAdjustableSpace -= batchSub;
                        }
                    }

                    // 剩余少量diff（通常0-总控件数），逐像素缩减
                    while (remainingDiff > 0) {
                        bool reduced = false;
                        for (auto& info : stretchInfos) {
                            if (info.finalWidth > info.minWidth) {
                                info.finalWidth -= 1;
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
            // 场景2：可用空间不足，优先显示前面控件（规则4）
            int32_t remainingSpace = totalUsableSpace;
            for (auto& info : stretchInfos) {
                if (remainingSpace <= 0) break;

                // 优先分配最小宽度
                int32_t allocWidth = std::min(info.minWidth, remainingSpace);
                info.finalWidth = allocWidth;
                remainingSpace -= allocWidth;
            }

            // 若仍有剩余空间，分配给前面的控件（按比例，不特殊处理<100%场景）
            if (remainingSpace > 0) {
                for (auto& info : stretchInfos) {
                    if (remainingSpace <= 0) break;
                    if (info.finalWidth >= info.maxWidth) continue;

                    int32_t maxAdd = info.maxWidth - info.finalWidth;
                    int32_t addWidth = static_cast<int32_t>(
                        1.0f * info.estSize.cx.GetStretchPercentValue() * remainingSpace / cxStretchPercentBase + 0.5f
                        );
                    addWidth = std::min({ addWidth, maxAdd, remainingSpace });

                    info.finalWidth += addWidth;
                    remainingSpace -= addWidth;
                }
            }
        }

        // 更新拉伸控件尺寸到itemsMap
        for (auto& info : stretchInfos) {
            info.estSize.cx.SetInt32(info.finalWidth);
            itemsMap[info.pCtrl] = info.estSize;
        }
    }

    // 处理未分配到空间的拉伸控件（设为0宽度）
    for (auto pControl : stretchControls) {
        if (itemsMap.find(pControl) == itemsMap.end()) {
            UiEstSize estSize;
            estSize.cx.SetInt32(0);
            itemsMap[pControl] = estSize;
        }
    }
    stretchItemsMap.clear();
    stretchControls.clear();

    // 计算子控件的最终位置（按：左边距 + 控件宽度 + 右边距 + 间距 的顺序）
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
        if ((estSizeIter == itemsMap.end()) || (estSizeIter->second.cx.GetInt32() <= 0)) {
            // 宽度为0，忽略布局
            rcChild = { nPosX, nPosY, nPosX, nPosY };
        }
        else {
            // 对一个控件进行布局（严格按布局方案）
            const UiEstSize& estSize = estSizeIter->second;
            UiMargin rcMargin = pControl->GetMargin();

            // 统一处理间距（非第一个控件前添加）
            if (!isFirstControl) {
                nPosX += GetChildMarginX();
            }
            isFirstControl = false;

            // 水平位置计算（含边距）
            nPosX += rcMargin.left;          // 左边距
            rcChild.left = nPosX;
            rcChild.right = rcChild.left + estSize.cx.GetInt32(); // 控件自身宽度
            nPosX = rcChild.right + rcMargin.right; // 右边距

            // 垂直位置计算（处理对齐）
            rcChild.top = nPosY + rcMargin.top;
            rcChild.bottom = rcChild.top + estSize.cy.GetInt32();

            // 处理垂直对齐（基于扣除内边距的容器范围）
            VerAlignType vAlign = pControl->GetVerAlignType();
            if (vAlign == VerAlignType::kAlignCenter || vAlign == VerAlignType::kAlignBottom) {
                UiRect rcValidArea = rc; // 容器有效范围（已扣内边距）

                // 剪去控件的外边距
                rcValidArea.top += rcMargin.top;
                rcValidArea.bottom -= rcMargin.bottom;

                int32_t nOffset = rcValidArea.Height() - rcChild.Height();
                if (nOffset > 0) {
                    if (vAlign == VerAlignType::kAlignCenter) {
                        rcChild.Offset(0, nOffset / 2);
                    }
                    else if (vAlign == VerAlignType::kAlignBottom) {
                        rcChild.Offset(0, nOffset);
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

    // 处理子控件总体水平对齐（居中/靠右）
    UiRect fullChildrenRect = childrenRect;     // 子控件总体范围（含对齐偏移）
    UiRect alignedChildrenRect = childrenRect;  // 子控件对齐后实际范围
    const HorAlignType hAlign = GetChildHAlignType();
    int32_t containerWidth = rc.Width();

    if (((hAlign == HorAlignType::kAlignCenter) || (hAlign == HorAlignType::kAlignRight)) &&
        childrenRect.Width() > 0 &&
        childrenRect.Width() < containerWidth) {

        int32_t nOffsetX = 0;
        if (hAlign == HorAlignType::kAlignCenter) {
            nOffsetX = (containerWidth - childrenRect.Width()) / 2;
        }
        else if (hAlign == HorAlignType::kAlignRight) {
            nOffsetX = containerWidth - childrenRect.Width();
        }

        // 应用水平偏移
        if (nOffsetX != 0) {
            alignedChildrenRect.Offset(nOffsetX, 0);
            for (UiRect& rcChild : childrenControlRects) {
                rcChild.Offset(nOffsetX, 0);
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
                SetFloatPos(pControl, rc);
            }
        }
    }

    // 计算最终返回的尺寸（含父容器内边距）
    UiPadding rcPadding = (GetOwner() != nullptr) ? GetOwner()->GetPadding() : UiPadding();
    UiSize64 resultSize(
        std::max((int64_t)fullChildrenRect.Width() + rcPadding.left + rcPadding.right, 0LL),
        std::max((int64_t)fullChildrenRect.Height() + rcPadding.top + rcPadding.bottom, 0LL)
    );
    return resultSize;
}

UiSize64 HLayout::EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable)
{
    // 宽度：所有有效子控件宽度之和（含边距）+ 子控件间隔 + 容器内边距
    // 高度：所有有效子控件高度的最大值（含边距）+ 容器内边距
    // 有效子控件：可见、非浮动，拉伸控件取最小尺寸参与计算
    UiSize64 totalSize(0, 0);  // 64位计算避免溢出
    int32_t validCount = 0;    // 统计参与间隔计算的有效控件（有宽度或边距）

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

        // 累加宽度（含控件宽度和左右边距，无条件计入）
        if (itemWidth > 0) {
            int64_t widthWithMargin = (int64_t)itemWidth + rcMargin.left + rcMargin.right;
            totalSize.cx += widthWithMargin;

            // 统计有效控件（有宽度或边距时，参与间隔计算）
            validCount++;
        }

        // 计算最大高度（含控件高度和上下边距，无条件参与比较）
        if (itemHeight > 0) {
            int64_t heightWithMargin = (int64_t)itemHeight + rcMargin.top + rcMargin.bottom;
            if (heightWithMargin > totalSize.cy) {
                totalSize.cy = heightWithMargin;
            }
        }
    }

    // 累加子控件间隔（有效控件数>1时）
    if (validCount > 1) {
        totalSize.cx += (int64_t)(validCount - 1) * GetChildMarginX();
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
