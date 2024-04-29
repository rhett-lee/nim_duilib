#ifndef UI_CONTROL_LIST_CTRL_HEADER_ITEM_H_
#define UI_CONTROL_LIST_CTRL_HEADER_ITEM_H_

#pragma once

#include "duilib/Control/Button.h"
#include "duilib/Control/CheckBox.h"
#include "duilib/Control/Split.h"
#include "duilib/Image/Image.h"
#include "duilib/Core/ControlDragable.h"

namespace ui
{

/** ListCtrl的表头控件的显示项
*/
class ListCtrlHeader;
class ListCtrlHeaderItem:
    public ControlDragableT<ButtonBox>
{
public:
    ListCtrlHeaderItem();
    virtual ~ListCtrlHeaderItem();

    /** 获取控件类型
    */
    virtual std::wstring GetType() const override;

    /** 设置属性
    */
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

    /** 绘制文字
    */
    virtual void PaintText(IRender* pRender) override;

    /** 按钮点击事件
    */
    virtual void Activate() override;

public:
    /** 排序方式
    */
    enum class SortMode
    {
        kNone, //不支持排序
        kUp,   //升序
        kDown  //降序
    };

    /** 设置排序方式
    * @param [in] sortMode 排序方式
    * @param [in] bTriggerEvent true表示触发排序变化事件，对数据重新排序，否则不触发事件
    */
    void SetSortMode(SortMode sortMode, bool bTriggerEvent = false);

    /** 获取排序方式
    */
    SortMode GetSortMode() const;

    /** 设置排序图标：降序
    */
    void SetSortedDownImage(const std::wstring& sImageString);

    /** 设置排序图标：升序
    */
    void SetSortedUpImage(const std::wstring& sImageString);

    /** 获取唯一ID
    */
    size_t GetColomnId() const;

    /** 设置关联的Split控件接口
    */
    void SetSplitBox(SplitBox* pSplitBox);

    /** 获取关联的Split控件接口
    */
    SplitBox* GetSplitBox() const;

    /** 设置是否允许调整列宽
    */
    void SetColumnResizeable(bool bResizeable);

    /** 获取是否允许调整列宽
    */
    bool IsColumnResizeable() const;

    /** 设置列宽
    * @param [in] nWidth 列宽值
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
    */
    void SetColumnWidth(int32_t nWidth, bool bNeedDpiScale);

    /** 获取列宽值
    */
    int32_t GetColumnWidth() const;

    /** 设置文字与图标之间的间隔（像素）
    */
    void SetIconSpacing(int32_t nIconSpacing, bool bNeedDpiScale);

    /** 获取文字与图标之间的间隔（像素）
    */
    int32_t GetIconSpacing() const;

    /** 设置图标显示在文字上方，居中显示
    */
    void SetShowIconAtTop(bool bShowIconAtTop);

    /** 获取图标是否显示在文字上方
    */
    bool IsShowIconAtTop() const;

    /** 设置文本水平对齐方式
    */
    void SetTextHorAlign(HorAlignType alignType);

    /** 获取文本水平对齐方式
    */
    HorAlignType GetTextHorAlign() const;

    /** 设置关联图标Id, 如果为-1表示不显示图标，图标显示在文本前面
    */
    void SetImageId(int32_t imageId);

    /** 获取关联图标Id
    */
    int32_t GetImageId() const;

    /** 该列是否支持列表头拖动改变列的顺序
    */
    virtual bool IsEnableDragOrder() const override;

public:
    /** 设置是否显示CheckBox
    * @param [in] bVisible true表示显示，false表示隐藏
    */
    bool SetCheckBoxVisible(bool bVisible);

    /** 判断当前CheckBox是否处于显示状态
    @return 返回true表示CheckBox存在，并且可见； 如果不含CheckBox，返回false
    */
    bool IsCheckBoxVisible() const;

    /** 设置CheckBox的勾选状态
    * @param [in] bChecked true表示勾选，false表示不勾选
    * @param [in] 如果bChecked和bPartChecked同时为true，表示部分选择
    */
    bool SetCheckBoxCheck(bool bChecked, bool bPartChecked);

    /** 获取CheckBox的勾选状态
    * @param [out] bChecked true表示勾选，false表示不勾选
    * @param [out] 如果bSelected和bPartSelect同时为true，表示部分选择
    */
    bool GetCheckBoxCheck(bool& bChecked, bool& bPartChecked) const;

    /** 显示/隐藏该列
    */
    void SetColumnVisible(bool bColumnVisible);

    /** 判断该列是否显示
    * @return 返回true表示该列显示，否则表示该列隐藏
    */
    bool IsColumnVisible() const;

    /** 设置关联的Header接口
    */
    void SetHeaderCtrl(ListCtrlHeader* pHeaderCtrl);

    /** 获取关联的Header接口
    */
    ListCtrlHeader* GetHeaderCtrl() const;

private:
    /** 同步列宽与UI控件宽度
    */
    void CheckColumnWidth();

    /** 使得目标区域纵向对齐
    */
    void VAlignRect(UiRect& rc, uint32_t textStyle, int32_t nImageHeight);

protected:
    /** @name 拖动调序相关的成员函数
    * @{ */
    /** 根据鼠标位置调整各个控件的位置(拖动操作的一部分)
	* @param [in] pt 当前鼠标的位置
	* @param [in] ptMouseDown 鼠标按下时的位置
	* @param [in] rcItemList 子控件的列表
	*/
	virtual void AdjustItemPos(const UiPoint& pt, const UiPoint& ptMouseDown,
							   const std::vector<ItemStatus>& rcItemList) const override;

	/** 交换两个控件的位置，完成顺序调整
	* @param [in] pt 当前鼠标的位置
	* @param [in] rcItemList 子控件的列表
	* @param [in] nOldItemIndex 原来的子项索引号
	* @param [in] nNewItemIndex 最新的子项索引号
	* @return 如果有顺序调整，返回true；否则返回false
	*/
	virtual bool AdjustItemOrders(const UiPoint& pt,
								  const std::vector<ItemStatus>& rcItemList,
								  size_t& nOldItemIndex,
								  size_t& nNewItemIndex) override;

	/** 控件位置拖动完成事件
	* @param [in] nOldItemIndex 原来的子项索引号
	* @param [in] nNewItemIndex 最新的子项索引号
	*/
	virtual void OnItemOrdersChanged(size_t nOldItemIndex, size_t nNewItemIndex) override;

    /** @} */

protected:
    //禁止外部调用调整可见性的函数，避免数据不同步
    virtual void SetFadeVisible(bool bVisible) override;
    virtual void SetVisible(bool bVisible) override;

private:
    /** 排序图标：降序
    */
    Image* m_pSortedDownImage;

    /** 排序图标：升序
    */
    Image* m_pSortedUpImage;

    /** 关联的Split控件接口
    */
    SplitBox* m_pSplitBox;

    /** 排序方式
    */
    SortMode m_sortMode;

    /** 列宽
    */
    int32_t m_nColumnWidth;

    /** 是否允许改变列宽
    */
    bool m_bColumnResizeable;

    /** 文字与图标之间的间隔(图标显示在文字的右侧或者左侧时)
    */
    int32_t m_nIconSpacing;

    /** 图标显示在文字上方，居中显示
    */
    bool m_bShowIconAtTop;

    /** 显示/隐藏该列
    */
    bool m_bColumnVisible;

    /** 关联图标Id, 如果为-1表示不显示图标，图标显示在文本前面
    */
    int32_t m_imageId;

    /** 关联的Header接口
    */
    ListCtrlHeader* m_pHeaderCtrl;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_HEADER_ITEM_H_
