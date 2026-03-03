#ifndef UI_BOX_GRIDBOX_H_
#define UI_BOX_GRIDBOX_H_

#include "duilib/Core/Box.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Layout/GridLayout.h"

namespace ui
{
/** 网格布局的容器(模板类)
*/
template<typename InheritType = Box>
class UILIB_API GridBoxT : public InheritType
{
    typedef GridBoxT<InheritType> BaseClass;
public:
    explicit GridBoxT(Window* pWindow) :
        InheritType(pWindow, new GridLayout())
    {
    }

    /** 获取控件类型
    */
    virtual DString GetType() const override { return DUI_CTR_GRIDBOX; }

public:
    /** 获取行数(0表示自动计算)
     */
    int32_t GetRows() const
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            return pGridLayout->GetRows();
        }
        return 0;
    }

    /** 设置行数(0表示自动计算)
     * @param [in] nRows 行数
     */
    void SetRows(int32_t nRows)
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            pGridLayout->SetRows(nRows);
        }
    }

    /** 获取列数(0表示自动计算)
     */
    int32_t GetColumns() const
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            return pGridLayout->GetColumns();
        }
        return 0;
    }

    /** 设置列数(0表示自动计算)
     * @param [in] nCols 列数
     */
    void SetColumns(int32_t nCols)
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            pGridLayout->SetColumns(nCols);
        }
    }

    /** 设置网格单元格宽度(0表示自动计算)
    */
    void SetGridWidth(int32_t nGridWidth, bool bNeedDpiScale)
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            pGridLayout->SetGridWidth(nGridWidth, bNeedDpiScale);
        }
    }

    /** 获取网格单元格宽度(0表示自动计算)
    * @return 返回网格宽度（已经做过DPI缩放）
    */
    int32_t GetGridWidth() const
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            return pGridLayout->GetGridWidth();
        }
        return 0;
    }

    /** 设置网格单元格高度(0表示自动计算)
    */
    void SetGridHeight(int32_t nGridHeight, bool bNeedDpiScale)
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            pGridLayout->SetGridHeight(nGridHeight, bNeedDpiScale);
        }
    }

    /** 获取网格单元格高度(0表示自动计算)
    * @return 返回网格高度（已经做过DPI缩放）
    */
    int32_t GetGridHeight() const
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            return pGridLayout->GetGridHeight();
        }
        return 0;
    }

    /** 当控件内容超出边界时，是否按比例缩小
     *   true  使用子控件的大小，若超出网格大小，按比例缩小，以使控件内容完全显示在网格内
     *   false 忽略子控件自身的大小，子控件的大小与网格大小一致
     */
    void SetScaleDown(bool bScaleDown)
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            pGridLayout->SetScaleDown(bScaleDown);
        }
    }

    /** 判断超出边界的时候，是否按比例缩小
     *@return 返回值 true  使用子控件的大小，若超出网格大小，按比例缩小，以使控件内容完全显示在网格内
     *              false 忽略子控件自身的大小，子控件的大小与网格大小一致
     */
    bool IsScaleDown() const
    {
        ASSERT(this->GetLayout()->GetLayoutType() == LayoutType::GridLayout);
        GridLayout* pGridLayout = dynamic_cast<GridLayout*>(this->GetLayout());
        if (pGridLayout != nullptr) {
            return pGridLayout->IsScaleDown();
        }
        return false;
    }
};

/** 网格布局的容器
*/
class UILIB_API GridBox : public GridBoxT<Box>
{
    typedef GridBoxT<Box> BaseClass;
public:
    explicit GridBox(Window* pWindow) :
        GridBoxT<Box>(pWindow)
    {
    }

    /** 获取控件类型
    */
    virtual DString GetType() const override { return DUI_CTR_GRIDBOX; }
};

/** 网格布局的容器(支持滚动条)
*/
class UILIB_API GridScrollBox : public GridBoxT<ScrollBox>
{
    typedef GridBoxT<ScrollBox> BaseClass;
public:
    explicit GridScrollBox(Window* pWindow) :
        GridBoxT<ScrollBox>(pWindow)
    {
    }

    /** 获取控件类型
    */
    virtual DString GetType() const override { return DUI_CTR_GRID_SCROLLBOX; }
};

} //namespace ui
#endif // UI_BOX_GRIDBOX_H_
