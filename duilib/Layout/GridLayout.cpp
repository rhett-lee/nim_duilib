#include "GridLayout.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Core/DpiManager.h"

namespace ui 
{

GridLayout::GridLayout():
    m_nRows(0),
    m_nColumns(0),
    m_nGridWidth(0),
    m_nGridHeight(0),
    m_bScaleDown(false)
{
    //默认水平居中
    SetChildHAlignType(HorAlignType::kAlignCenter);

    //默认垂直靠上
    SetChildVAlignType(VerAlignType::kAlignTop);
}

bool GridLayout::SetAttribute(const DString& strName, const DString& strValue, const DpiManager& dpiManager)
{
    bool hasAttribute = true;
    if (strName == _T("rows")) {
        if (strValue == _T("auto")) {
            //自动计算
            SetRows(0);
        }
        else {
            SetRows(StringUtil::StringToInt32(strValue));
        }
    }
    else if (strName == _T("columns")) {
        if (strValue == _T("auto")) {
            //自动计算
            SetColumns(0);
        }
        else {
            SetColumns(StringUtil::StringToInt32(strValue));
        }
    }
    else if (strName == _T("grid_width")) {
        if (strValue == _T("auto")) {
            //自动计算
            SetGridWidth(0, false);
        }
        else {
            int32_t nGridWidth = StringUtil::StringToInt32(strValue);
            dpiManager.ScaleInt(nGridWidth);
            SetGridWidth(nGridWidth, false);
        }
    }
    else if (strName == _T("grid_height")) {
        if (strValue == _T("auto")) {
            //自动计算
            SetGridHeight(0, false);
        }
        else {
            int32_t nGridHeight = StringUtil::StringToInt32(strValue);
            dpiManager.ScaleInt(nGridHeight);
            SetGridHeight(nGridHeight, false);
        }
    }
    else if (strName == _T("scale_down")) {
        SetScaleDown(strValue == _T("true"));
    }
    else {
        hasAttribute = BaseClass::SetAttribute(strName, strValue, dpiManager);
    }
    return hasAttribute;
}

void GridLayout::ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale)
{
    int32_t nGridWidth = GetGridWidth();
    nGridWidth = dpiManager.GetScaleInt(nGridWidth, nOldDpiScale);
    SetGridWidth(nGridWidth, false);

    int32_t nGridHeight = GetGridHeight();
    nGridHeight = dpiManager.GetScaleInt(nGridHeight, nOldDpiScale);
    SetGridHeight(nGridHeight, false);
}

int32_t GridLayout::GetRows() const
{
    return m_nRows;
}

void GridLayout::SetRows(int32_t nRows)
{
    ASSERT(nRows >= 0);
    nRows = std::max(nRows, 0);
    if (m_nRows != nRows) {
        m_nRows = nRows;
        if (GetOwner() != nullptr) {
            GetOwner()->Arrange();
        }
    }
}

int32_t GridLayout::GetColumns() const
{
    return m_nColumns;
}

void GridLayout::SetColumns(int32_t nCols)
{
    ASSERT(nCols >= 0);
    nCols = std::max(nCols, 0);
    if (m_nColumns != nCols) {
        m_nColumns = nCols;
        if (GetOwner() != nullptr) {
            GetOwner()->Arrange();
        }
    }
}

void GridLayout::SetGridWidth(int32_t nGridWidth, bool bNeedDpiScale)
{
    ASSERT(nGridWidth >= 0);
    if (nGridWidth >= 0) {
        if (bNeedDpiScale) {
            ASSERT(GetOwner() != nullptr);
            if (GetOwner() != nullptr) {
                GetOwner()->Dpi().ScaleInt(nGridWidth);
            }
        }
        if (m_nGridWidth != nGridWidth) {
            m_nGridWidth = nGridWidth;
            if (GetOwner() != nullptr) {
                GetOwner()->Arrange();
            }
        }
    }
}

int32_t GridLayout::GetGridWidth() const
{
    return m_nGridWidth;
}

void GridLayout::SetGridHeight(int32_t nGridHeight, bool bNeedDpiScale)
{
    ASSERT(nGridHeight >= 0);
    if (nGridHeight >= 0) {
        if (bNeedDpiScale) {
            ASSERT(GetOwner() != nullptr);
            if (GetOwner() != nullptr) {
                GetOwner()->Dpi().ScaleInt(nGridHeight);
            }
        }
        if (m_nGridHeight != nGridHeight) {
            m_nGridHeight = nGridHeight;
            if (GetOwner() != nullptr) {
                GetOwner()->Arrange();
            }
        }
    }
}

int32_t GridLayout::GetGridHeight() const
{
    return m_nGridHeight;
}

void GridLayout::SetScaleDown(bool bScaleDown)
{
    if (m_bScaleDown != bScaleDown) {
        m_bScaleDown = bScaleDown;
        if (GetOwner() != nullptr) {
            GetOwner()->Arrange();
        }
    }
}

bool GridLayout::IsScaleDown() const
{
    return m_bScaleDown;
}

UiSize64 GridLayout::ArrangeChildInternal(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly) const
{
    if (items.empty()) {
        return UiSize64();
    }
    // 预处理：筛选所有可见、非浮动的控件
    std::vector<Control*> visibleControls;
    for (auto pControl : items) {
        if ((pControl != nullptr) && pControl->IsVisible() && !pControl->IsFloat()) {
            visibleControls.push_back(pControl);
        }
    }
    if (visibleControls.empty()) {
        //可见控件为空, 调整子控件的布局（浮动控件），然后直接返回
        if (!bEstimateOnly) {
            for (auto pControl : items) {
                if ((pControl != nullptr) && pControl->IsVisible() && pControl->IsFloat()) {
                    //浮动控件（容器本身的对齐方式不生效）
                    SetFloatPos(pControl, rc);
                }
            }
        }
        return UiSize64();
    }

    DeflatePadding(rc);
    const UiRect contentRect = rc; //容器的矩形范围, 已扣除容器内边距

    // 计算实际行列数（处理自动计算）
    int32_t actualRows = GetRows();
    int32_t actualCols = GetColumns();

    //计算格子数量
    int32_t gridCount = 0;
    for (auto pControl : visibleControls) {
        int32_t rowSpan = pControl->GetRowSpan();
        int32_t colSpan = pControl->GetColumnSpan();
        ASSERT(rowSpan > 0);
        ASSERT(colSpan > 0);
        gridCount += rowSpan * colSpan;
    }
    gridCount = std::max(gridCount, (int32_t)visibleControls.size());

    CalcActualGridSize(gridCount, actualRows, actualCols);

    // 计算单元格基础尺寸（未考虑合并）
    std::vector<int32_t> colWidths = CalcColumnWidths(contentRect, actualCols);
    std::vector<int32_t> rowHeights = CalcRowHeights(contentRect, actualRows);

    // 标记已占用的单元格（处理合并）
    std::vector<std::vector<bool>> cellOccupied(actualRows, std::vector<bool>(actualCols, false));
    std::unordered_map<Control*, UiRect> ctrlCellRects; // 控件对应的合并后单元格区域
    if (!CalcMergedCellRects(visibleControls, actualRows, actualCols, colWidths, rowHeights, cellOccupied, ctrlCellRects)) {
        ASSERT(!"CalcMergedCellRects failed!");
        return UiSize64(); // 单元格合并冲突，返回无效尺寸
    }

    //计算网格整体位置（基于布局的对齐属性）
    UiSize gridTotalSize = CalcGridTotalSize(colWidths, rowHeights);
    if (!bEstimateOnly) {
        UiRect gridRect = CalcGridPosition(contentRect, gridTotalSize);

        //调整合并单元格位置（基于网格整体偏移）
        AdjustMergedCellPositions(ctrlCellRects, gridRect);

        //排列控件到合并单元格（使用控件自身对齐属性）
        ArrangeControlsInMergedCells(visibleControls, ctrlCellRects);

        // 处理浮动控件
        for (auto pControl : items) {
            if ((pControl != nullptr) && pControl->IsVisible() && pControl->IsFloat()) {
                SetFloatPos(pControl, rc);
            }
        }
    }
    //计算最终所需尺寸（含内边距）
    UiPadding rcPadding = GetOwner() != nullptr ? GetOwner()->GetPadding() : UiPadding();
    return UiSize64(
            gridTotalSize.cx + rcPadding.left + rcPadding.right,
            gridTotalSize.cy + rcPadding.top + rcPadding.bottom
        );
}

void GridLayout::CalcActualGridSize(int32_t gridCount, int32_t& rows, int32_t& cols) const
{
    rows = std::max(rows, 0);
    cols = std::max(cols, 0);
    if (gridCount > 0) {
        if (rows == 0 && cols == 0) {
            cols = static_cast<int32_t>(std::sqrt(gridCount)) + 1;
            rows = (gridCount + cols - 1) / cols;
        }
        else if (rows == 0) {
            rows = (gridCount + cols - 1) / cols;
        }
        else if (cols == 0) {
            cols = (gridCount + rows - 1) / rows;
        }
    }
    rows = std::max(rows, 1);
    cols = std::max(cols, 1);
}

std::vector<int32_t> GridLayout::CalcColumnWidths(const UiRect& contentRect, int32_t cols) const
{
    ASSERT(cols > 0);
    if (cols <= 0) {
        return std::vector<int32_t>();
    }

    int32_t nGridWidth = GetGridWidth();
    if (nGridWidth > 0) {
        // 已经设置了网格的固定宽度，直接使用
        std::vector<int32_t> widths(cols, nGridWidth);
        return widths;
    }
    else {
        // 根据内容显示区域的总宽度，计算每列的宽度
        int32_t availableWidth = contentRect.Width() - (cols - 1) * GetChildMarginX();
        int32_t baseWidth = std::max(0, availableWidth / cols);
        std::vector<int32_t> widths(cols, baseWidth);

        // 分配剩余宽度（解决整除误差）
        int32_t remaining = availableWidth - baseWidth * cols;
        for (int32_t i = 0; i < remaining; ++i) {
            widths[i]++;
        }
        return widths;
    }
}

std::vector<int32_t> GridLayout::CalcRowHeights(const UiRect& contentRect, int32_t rows) const
{
    ASSERT(rows > 0);
    if (rows <= 0) {
        return std::vector<int32_t>();
    }

    int32_t nGridHeight = GetGridHeight();
    if (nGridHeight > 0) {
        // 已经设置了网格的固定宽度，直接使用
        std::vector<int32_t> heights(rows, nGridHeight);
        return heights;
    }
    else {
        // 根据内容显示区域的总高度，计算每列的高度
        int32_t availableHeight = contentRect.Height() - (rows - 1) * GetChildMarginY();
        int32_t baseHeight = std::max(0, availableHeight / rows);
        std::vector<int32_t> heights(rows, baseHeight);

        // 分配剩余高度
        int32_t remaining = availableHeight - baseHeight * rows;
        for (int32_t i = 0; i < remaining; ++i) {
            heights[i]++;
        }
        return heights;
    }
}

bool GridLayout::CalcMergedCellRects(const std::vector<Control*>& controls,
                                     int32_t rows, int32_t cols,
                                     const std::vector<int32_t>& colWidths,
                                     const std::vector<int32_t>& rowHeights,
                                     std::vector<std::vector<bool>>& cellOccupied,
                                     std::unordered_map<Control*, UiRect>& ctrlCellRects) const
{
    ASSERT((rows > 0) && (cols > 0) && !controls.empty() && ((int32_t)colWidths.size() == cols) && ((int32_t)rowHeights.size() == rows));
    if ((rows <= 0) || (cols <= 0) || controls.empty() ||
        ((int32_t)colWidths.size() != cols) ||
        ((int32_t)rowHeights.size() != rows)) {
        return false;
    }
    int32_t ctrlIndex = 0;
    for (int32_t row = 0; row < rows && ctrlIndex < (int32_t)controls.size(); ++row) {
        for (int32_t col = 0; col < cols && ctrlIndex < (int32_t)controls.size(); ++col) {
            if (cellOccupied[row][col]) {
                continue; // 跳过已占用单元格
            }

            Control* ctrl = controls[ctrlIndex++];
            int32_t rowSpan = std::min(ctrl->GetRowSpan(), rows - row); // 限制最大行合并
            int32_t colSpan = std::min(ctrl->GetColumnSpan(), cols - col); // 限制最大列合并

            // 标记合并范围内的单元格为已占用
            for (int32_t r = row; r < row + rowSpan; ++r) {
                for (int32_t c = col; c < col + colSpan; ++c) {
                    if (cellOccupied[r][c]) {
                        return false; // 合并冲突（重叠），返回失败
                    }
                    cellOccupied[r][c] = true;
                }
            }

            // 计算合并后单元格的原始尺寸（未考虑网格偏移）
            UiRect cellRect;
            cellRect.left = 0;
            for (size_t c = 0; c < col; ++c) {
                cellRect.left += colWidths[c] + GetChildMarginX();
            }
            cellRect.right = cellRect.left;
            for (size_t c = col; c < col + colSpan; ++c) {
                cellRect.right += colWidths[c] + (c == (col + colSpan - 1) ? 0 : GetChildMarginX());
            }

            cellRect.top = 0;
            for (size_t r = 0; r < row; ++r) {
                cellRect.top += rowHeights[r] + GetChildMarginY();
            }
            cellRect.bottom = cellRect.top;
            for (size_t r = row; r < row + rowSpan; ++r) {
                cellRect.bottom += rowHeights[r] + (r == (row + rowSpan - 1) ? 0 : GetChildMarginY());
            }

            ctrlCellRects[ctrl] = cellRect;
        }
    }
    return true;
}

UiSize GridLayout::CalcGridTotalSize(const std::vector<int32_t>& colWidths, const std::vector<int32_t>& rowHeights) const
{
    int32_t totalWidth = 0;
    for (size_t i = 0; i < colWidths.size(); ++i) {
        totalWidth += colWidths[i] + (i == (colWidths.size() - 1) ? 0 : GetChildMarginX());
    }
    int32_t totalHeight = 0;
    for (size_t i = 0; i < rowHeights.size(); ++i) {
        totalHeight += rowHeights[i] + (i == (rowHeights.size() - 1) ? 0 : GetChildMarginY());
    }
    return UiSize(totalWidth, totalHeight);
}

UiRect GridLayout::CalcGridPosition(const UiRect& contentRect, const UiSize& gridSize) const
{
    UiRect gridRect;
    gridRect.left = contentRect.left;
    gridRect.top = contentRect.top;
    gridRect.right = gridRect.left + gridSize.cx;
    gridRect.bottom = gridRect.top + gridSize.cy;

    // 水平对齐（布局自身的GetChildHAlignType）
    switch (GetChildHAlignType()) {
    case HorAlignType::kAlignCenter:
        gridRect.Offset((contentRect.Width() - gridSize.cx) / 2, 0);
        break;
    case HorAlignType::kAlignRight:
        gridRect.Offset(contentRect.Width() - gridSize.cx, 0);
        break;
    default:
        break;
    }

    // 垂直对齐（布局自身的GetChildVAlignType）
    switch (GetChildVAlignType()) {
    case VerAlignType::kAlignCenter:
        gridRect.Offset(0, (contentRect.Height() - gridSize.cy) / 2);
        break;
    case VerAlignType::kAlignBottom:
        gridRect.Offset(0, contentRect.Height() - gridSize.cy);
        break;
    default:
        break;
    }
    return gridRect;
}

void GridLayout::AdjustMergedCellPositions(std::unordered_map<Control*, UiRect>& ctrlCellRects,
                                           const UiRect& gridRect) const
{
    for (auto& [ctrl, rect] : ctrlCellRects) {
        rect.Offset(gridRect.left, gridRect.top);
    }
}

void GridLayout::ArrangeControlsInMergedCells(const std::vector<Control*>& controls,
                                              const std::unordered_map<Control*, UiRect>& ctrlCellRects) const
{
    for (Control* pControl : controls) {
        auto it = ctrlCellRects.find(pControl);
        if (it == ctrlCellRects.end()) {
            continue;
        }

        const UiRect& cellRect = it->second;
        UiMargin margin = pControl->GetMargin();

        // 控件在单元格内的位置
        UiRect rcChild;

        // 单元格内可用空间（扣除控件外边距）
        UiRect contentRect = cellRect;
        contentRect.left += margin.left;
        contentRect.top += margin.top;
        contentRect.right -= margin.right;
        contentRect.bottom -= margin.bottom;
        if (contentRect.Width() <= 0 || contentRect.Height() <= 0) {
            // 单元格内空间不足：设置其宽度和高度为0
            rcChild = contentRect;
            rcChild.right = rcChild.left;
            rcChild.bottom = rcChild.top;
        }
        else {
            // 单元格内空间充足: 计算控件尺寸（受限于自身min/max）
            UiSize childSize;
            if (IsScaleDown()) {
                UiSize szAvailable(contentRect.Width(), contentRect.Height());
                UiEstSize estSize = pControl->EstimateSize(szAvailable);
                childSize = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
                if (estSize.cx.IsStretch()) {
                    childSize.cx = CalcStretchValue(estSize.cx, szAvailable.cx);
                }
                if (estSize.cy.IsStretch()) {
                    childSize.cy = CalcStretchValue(estSize.cy, szAvailable.cy);
                }
                childSize.cx = std::clamp(childSize.cx, pControl->GetMinWidth(), pControl->GetMaxWidth());
                childSize.cy = std::clamp(childSize.cy, pControl->GetMinHeight(), pControl->GetMaxHeight());

                // 如果子控件大小超出格子范围，则等比例缩小到格子以内，避免超出边界
                if ((childSize.cx > 0) && (childSize.cy > 0)) {
                    if ((childSize.cx > contentRect.Width()) || (childSize.cy > contentRect.Height())) {
                        //满足缩放条件，进行等比缩放
                        double cx = childSize.cx;
                        double cy = childSize.cy;
                        double cxRatio = cx / contentRect.Width();
                        double cyRatio = cy / contentRect.Height();
                        if (cxRatio > cyRatio) {
                            ASSERT(childSize.cx > contentRect.Width());
                            double ratio = cx / childSize.cy;
                            childSize.cx = contentRect.Width();
                            childSize.cy = static_cast<int32_t>(childSize.cx / ratio + 0.5);
                        }
                        else {
                            ASSERT(childSize.cy > contentRect.Height());
                            double ratio = cy / childSize.cx;
                            childSize.cy = contentRect.Height();
                            childSize.cx = static_cast<int32_t>(childSize.cy / ratio + 0.5);
                        }
                    }
                }
            }
            else {
                childSize = UiSize(contentRect.Width(), contentRect.Height());
            }

            // 计算控件在单元格内的位置（使用控件自身的对齐属性）
            rcChild = CalcControlPosition(contentRect, childSize, pControl);
        }
        pControl->SetPos(rcChild);
    }
}

UiRect GridLayout::CalcControlPosition(const UiRect& contentRect, const UiSize& ctrlSize, Control* ctrl) const
{
    UiRect ctrlRect;
    ctrlRect.left = contentRect.left;
    ctrlRect.top = contentRect.top;
    ctrlRect.right = ctrlRect.left + ctrlSize.cx;
    ctrlRect.bottom = ctrlRect.top + ctrlSize.cy;

    // 水平对齐（控件自身的GetHorAlignType）
    switch (ctrl->GetHorAlignType()) {
    case HorAlignType::kAlignCenter:
        ctrlRect.Offset((contentRect.Width() - ctrlSize.cx) / 2, 0);
        break;
    case HorAlignType::kAlignRight:
        ctrlRect.Offset(contentRect.Width() - ctrlSize.cx, 0);
        break;
    default:
        break;
    }

    // 垂直对齐（控件自身的GetVerAlignType）
    switch (ctrl->GetVerAlignType()) {
    case VerAlignType::kAlignCenter:
        ctrlRect.Offset(0, (contentRect.Height() - ctrlSize.cy) / 2);
        break;
    case VerAlignType::kAlignBottom:
        ctrlRect.Offset(0, contentRect.Height() - ctrlSize.cy);
        break;
    default:
        break;
    }
    return ctrlRect;
}

UiSize64 GridLayout::ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly)
{
    return ArrangeChildInternal(items, rc, bEstimateOnly);
}

UiSize64 GridLayout::EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable)
{
    UiRect rc(0, 0, szAvailable.cx, szAvailable.cy);
    return ArrangeChildInternal(items, rc, true);
}

} // namespace ui
