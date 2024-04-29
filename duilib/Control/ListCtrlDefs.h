#ifndef UI_CONTROL_LIST_CTRL_DEFS_H_
#define UI_CONTROL_LIST_CTRL_DEFS_H_

#pragma once

#include "duilib/Control/ListCtrlView.h"
#include "duilib/Control/CheckBox.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/RichEdit.h"

namespace ui
{
/** 表格类型
*/
enum class ListCtrlType
{
    Report = 0, //Report类型
    Icon   = 1, //Icon类型
    List   = 2  //List类型
};

/** 列的基本信息（用于添加列）
*/
struct ListCtrlColumn
{
    std::wstring text;              //表头的文本
    int32_t nColumnWidth = 100;     //列宽，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nColumnWidthMin = 0;    //列宽最小值，0表示用默认设置，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nColumnWidthMax = 0;    //列宽最大值，0表示用默认设置，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nTextFormat = -1;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    int32_t nImageId = -1;          //图标资源Id，如果为-1表示不显示图标，图标显示在文本前面
    bool bSortable = true;          //是否支持排序
    bool bResizeable = true;        //是否支持通过拖动调整列宽
    bool bShowCheckBox = true;      //是否显示CheckBox（支持在表头和数据列显示CheckBox）
    bool bNeedDpiScale = true;      //是否对数值做DPI自适应
};

/** 行的属性数据结构(行数据，每行1条数据)
*/
struct ListCtrlItemData
{
    bool bVisible = true;           //是否可见
    bool bSelected = false;         //是否处于选择状态（是指本行是否被选中）
    bool bChecked = false;          //是否处于勾选状态（是指本行前面的CheckBox是否被打勾）
    int32_t nImageId = -1;          //图标资源Id，如果为-1表示行首不显示图标
    int8_t nAlwaysAtTop = -1;       //是否置顶显示, -1表示不置顶, 0 或者 正数表示置顶，数值越大优先级越高，优先显示在最上面
    int16_t nItemHeight = -1;       //行的高度, -1表示使用ListCtrl设置的默认行高，为DPI自适应处理后的值
    size_t nUserData = 0;           //用户自定义数据
};

/** 列表数据项的基本信息（列数据，用于添加数据）
*/
struct ListCtrlSubItemData
{
    std::wstring text;              //文本内容
    int32_t nTextFormat = -1;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    int32_t nImageId = -1;          //图标资源Id，如果为-1表示不显示图标
    UiColor textColor;              //文本颜色
    UiColor bkColor;                //背景颜色
    bool bShowCheckBox = true;      //是否显示CheckBox
    bool bChecked = false;          //是否处于勾选状态（CheckBox勾选状态）
    bool bEditable = false;         //是否可编辑
};

/** 列表数据项用于内部存储的数据结构(列数据，每<行,列>1条数据)
*/
struct ListCtrlSubItemData2
{
    UiString text;                  //文本内容
    uint16_t nTextFormat = 0;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    int32_t nImageId = -1;          //图标资源Id，如果为-1表示不显示图标
    UiColor textColor;              //文本颜色
    UiColor bkColor;                //背景颜色
    bool bShowCheckBox = true;      //是否显示CheckBox  
    bool bChecked = false;          //是否处于勾选状态（CheckBox勾选状态）
    bool bEditable = false;         //是否可编辑
};

//列数据的智能指针
typedef std::shared_ptr<ListCtrlSubItemData2> ListCtrlSubItemData2Ptr;

struct ListCtrlSubItemData2Pair
{
    size_t nColumnId; //列的ID
    ListCtrlSubItemData2Ptr pSubItemData; //列的数据
};

/** 比较数据的附加信息
*/
struct ListCtrlCompareParam
{
    size_t nColumnIndex; //数据关联第几列，有效范围：[0, GetColumnCount())
    size_t nColumnId;    //数据关联列的ID
    void* pUserData;     //用户自定义数据，设置比较函数的时候一同传入
};

/** 存储数据的比较函数的原型, 实现升序的比较(a < b)
* @param [in] a 第一个比较数据
* @param [in] b 第二个比较数据
* @param [in] param 数据关联的参数
* @return 如果 (a < b)，返回true，否则返回false
*/
typedef std::function<bool(const ListCtrlSubItemData2& a, 
                           const ListCtrlSubItemData2& b, 
                           const ListCtrlCompareParam& param)> ListCtrlDataCompareFunc;

/** 视图填充数据到UI控件的相关接口
*/
class IListCtrlView
{
public:
    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual Control* CreateDataItem() = 0;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] itemData 数据项（代表行的属性）
    * @param [in] subItemList 数据子项（代表每一列的数据, 第1个是列的ID，第2个是列的数据）
    */
    virtual bool FillDataItem(ui::Control* pControl,
                              size_t nElementIndex,
                              const ListCtrlItemData& itemData,
                              const std::vector<ListCtrlSubItemData2Pair>& subItemList) = 0;


    /** 获取某列的宽度最大值
    * @param [in] subItemList 数据子项（代表每一列的数据）
    * @return 返回该列宽度的最大值，返回的是DPI自适应后的值； 如果失败返回-1
    */
    virtual int32_t GetMaxDataItemWidth(const std::vector<ListCtrlSubItemData2Ptr>& subItemList) = 0;
};

/** 列表中使用的Label控件，用于显示文本，并提供文本编辑功能
*/
class ListCtrlLabel: public LabelTemplate<HBox>
{
public:
    /** 消息处理
    */
    virtual void HandleEvent(const EventArgs& msg) override
    {
        if ((msg.Type > kEventMouseBegin) && (msg.Type < kEventMouseEnd)) {
            //当前控件禁止接收鼠标消息时，将鼠标相关消息转发给上层处理
            bool bButtonUpEvent = false;
            if (IsEnabled() && IsMouseEnabled()) {
                if (msg.Type == kEventMouseButtonDown) {
                    m_bMouseDown = false;
                    if (GetWindow() != nullptr) {
                        Control* pFocus = GetWindow()->GetFocus();
                        if ((pFocus != nullptr) && (pFocus == m_pListBoxItem)) {
                            //避免每次点击都进入编辑模式
                            m_bMouseDown = true;
                        }
                    }
                }
                else if (msg.Type == kEventMouseButtonUp) {
                    if (m_bMouseDown) {
                        m_bMouseDown = false;
                        bButtonUpEvent = true;
                    }                    
                }
            }
            Box* pParent = GetParent();
            if (pParent != nullptr) {
                pParent->SendEvent(msg);
            }
            if (bButtonUpEvent) {
                //进入编辑状态
                OnItemEnterEditMode();
            }
            return;
        }
        __super::HandleEvent(msg);
    }

    /** 进入编辑状态
    */
    virtual void OnItemEnterEditMode()
    {
        SendEvent(kEventEnterEdit, (WPARAM)this);
    }

    /** 设置文本所在位置的矩形区域
    */
    void SetTextRect(const UiRect& rect)
    {
        m_textRect = rect;
    }

    /** 获取文本所在位置的矩形区域
    */
    UiRect GetTextRect() const
    {
        UiRect rect = m_textRect;
        if (rect.IsZero()) {
            rect = GetRect();
            rect.Deflate(GetControlPadding());
            rect.Deflate(GetTextPadding());
        }        
        return rect;
    }

    /** 设置关联的列表项
    */
    /** 关联的列表项
    */
    void SetListBoxItem(Control* pListBoxItem)
    {
        m_pListBoxItem = pListBoxItem;
    }

private:
    /** 是否鼠标点击在控件范围内
    */
    bool m_bMouseDown = false;

    /** 关联的列表项
    */
    Control* m_pListBoxItem = nullptr;

    /** 文本所在位置的矩形区域
    */
    UiRect m_textRect;
};

/** 编辑状态的输入参数
*/
struct ListCtrlEditParam
{
    ListCtrlType listCtrlType;
    size_t nItemIndex;          //数据项的索引号, 有效范围：[0, GetDataItemCount())
    size_t nColumnId;           //列的ID
    size_t nColumnIndex;        //列的序号, 有效范围：[0, GetColumnCount())
    IListBoxItem* pItem;        //数据子项接口
    ListCtrlLabel* pSubItem;    //文本控件接口（含修改前的文本内容）

    UiString sNewText;          //修改后的文本内容
    bool bCancelled;            //是否取消操作，如果设置为true，则取消编辑操作
};

/** 列表中使用的CheckBox
*/
class ListCtrlCheckBox: public CheckBox
{
public:
    ListCtrlCheckBox()
    {
        //CheckBox本身不接收键盘消息
        SetKeyboardEnabled(false);
    }

    /** 获取控件类型和设置属性
    */
    virtual std::wstring GetType() const override { return L"ListCtrlCheckBox"; }
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override
    {
        if (strName == L"check_box_width") {
            SetCheckBoxWidth(_wtoi(strValue.c_str()), true);
        }
        else {
            __super::SetAttribute(strName, strValue);
        }
    }

    /** 设置CheckBox所占的宽度值
    */
    void SetCheckBoxWidth(int32_t nWidth, bool bNeedDpiScale)
    {
        if (bNeedDpiScale) {
            GlobalManager::Instance().Dpi().ScaleInt(nWidth);
        }
        if (nWidth < 0) {
            nWidth = 0;
        }
        m_nCheckBoxWidth = nWidth;
    }

    /** 获取CheckBox所占的宽度值
    */
    int32_t GetCheckBoxWidth()
    {
        if (m_nCheckBoxWidth > 0) {
            return m_nCheckBoxWidth;
        }
        //如果没设置，查询图片的大小，并记住
        UiEstSize sz;
        if (GetWindow() != nullptr) {
            sz = EstimateSize(UiSize(0, 0));
        }
        int32_t nWidth = sz.cx.GetInt32();
        if (nWidth > 0) {
            UiMargin rcMargin = GetMargin();
            UiPadding rcPadding = GetPadding();
            nWidth += rcMargin.left + rcMargin.right;
            nWidth += rcPadding.left + rcPadding.right;
            m_nCheckBoxWidth = nWidth;

            m_nCheckBoxWidth += GlobalManager::Instance().Dpi().GetScaleInt(2);
        }
        return m_nCheckBoxWidth;
    }

private:
    /** 显示CheckBox所占的宽度，用于设置父控件的Padding值
    */
    int32_t m_nCheckBoxWidth = 0;
};

/** Icon视图的列表项类型(垂直布局)
*   基本结构：<ListCtrlIconViewItem> <Control/><ListCtrlLabel/> </ListCtrlListViewItem>
*   其中的Control和Label的属性，支持从配置文件读取
*/
class ListCtrl;
class ListCtrlIconViewItem : public ListCtrlItemBaseV
{
public:
    /** 获取控件类型
    */
    virtual std::wstring GetType() const override { return L"ListCtrlIconViewItem"; }

    /** 事件处理函数
    */
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 设置关联的ListCtrl接口
    */
    void SetListCtrl(ListCtrl* pListCtrl) { m_pListCtrl = pListCtrl; }

    /** 获取关联的ListCtrl接口
    */
    ListCtrl* GetListCtrl() const { return m_pListCtrl; }

private:
    /** 关联的ListCtrl接口
    */
    ListCtrl* m_pListCtrl = nullptr;
};

/** List视图的列表项类型(水平布局)
*   基本结构：<ListCtrlListViewItem> <Control/><ListCtrlLabel/> </ListCtrlListViewItem>
*   其中的Control和Label的属性，支持从配置文件读取
*/
class ListCtrlListViewItem : public ListCtrlItemBaseH
{
public:
    /** 获取控件类型
    */
    virtual std::wstring GetType() const override { return L"ListCtrlListViewItem"; }

    /** 事件处理函数
    */
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 设置关联的ListCtrl接口
    */
    void SetListCtrl(ListCtrl* pListCtrl) { m_pListCtrl = pListCtrl; }

    /** 获取关联的ListCtrl接口
    */
    ListCtrl* GetListCtrl() const { return m_pListCtrl; }

private:
    /** 关联的ListCtrl接口
    */
    ListCtrl* m_pListCtrl = nullptr;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_DEFS_H_
