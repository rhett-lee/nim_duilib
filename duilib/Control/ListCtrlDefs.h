#ifndef UI_CONTROL_LIST_CTRL_DEFS_H_
#define UI_CONTROL_LIST_CTRL_DEFS_H_

#pragma once

#include "duilib/Control/CheckBox.h"

namespace ui
{
/** 列的基本信息（用于添加列）
*/
struct ListCtrlColumn
{
    std::wstring text;              //表头的文本
    int32_t nColumnWidth = 100;     //列宽，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nColumnWidthMin = 0;    //列宽最小值，0表示用默认设置，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nColumnWidthMax = 0;    //列宽最大值，0表示用默认设置，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nTextFormat = -1;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    bool bSortable = true;          //是否支持排序
    bool bResizeable = true;        //是否支持通过拖动调整列宽
    bool bShowCheckBox = true;      //是否显示CheckBox（支持在表头和数据列显示CheckBox）
    bool bNeedDpiScale = true;      //是否对数值做DPI自适应
};

/** 列表数据项的基本信息（列数据，用于添加数据）
*/
struct ListCtrlDataItem
{
    size_t nColumnIndex = 0;        //【必填】第几列，有效范围：[0, GetColumnCount())
    std::wstring text;              //文本内容
    int32_t nTextFormat = -1;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    int32_t nImageIndex = -1;       //图标资源索引号，在图片列表里面的下标值，如果为-1表示不显示图标
    UiColor textColor;              //文本颜色
    UiColor bkColor;                //背景颜色
    bool bShowCheckBox = true;      //是否显示CheckBox
    bool bChecked = false;          //是否处于勾选状态（CheckBox勾选状态）
};

/** 用于内部存储的数据结构(列数据，每<行,列>1条数据)
*/
struct ListCtrlData
{
    UiString text;                  //文本内容
    uint16_t nTextFormat = 0;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    int32_t nImageIndex = -1;       //图标资源索引号，在图片列表里面的下标值，如果为-1表示不显示图标
    UiColor textColor;              //文本颜色
    UiColor bkColor;                //背景颜色
    bool bShowCheckBox = true;      //是否显示CheckBox  
    bool bChecked = false;          //是否处于勾选状态（CheckBox勾选状态）

    //TODO: 待实现功能列表
    //int32_t nBkImageIndex = -1;     //背景图片资源索引号
    //Item的文本可以编辑
            //选择：单选，多选  提供接口
    //事件响应：点击，右键等
    //多视图的支持：Report，Icon等，类似与Windows资源管理器
    //数据类型的支持：比如整型，日期型，下拉表，字符串类型等
    //关联图片列表，图片列表需要单独实现
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
typedef std::function<bool(const ListCtrlData& a, const ListCtrlData& b, const ListCtrlCompareParam& param)>
ListCtrlDataCompareFunc;

/** 行的属性数据结构(行数据，每行1条数据)
*/
struct ListCtrlRowData
{
    bool bVisible = true;           //是否可见
    bool bSelected = false;         //是否处于选择状态（是指本行是否被选中）
    bool bChecked = false;          //是否处于勾选状态（是指本行前面的CheckBox是否被打勾）
    int8_t nAlwaysAtTop = -1;       //是否置顶显示, -1表示不置顶, 0 或者 正数表示置顶，数值越大优先级越高，优先显示在最上面
    int16_t nItemHeight = -1;       //行的高度, -1表示使用ListCtrl设置的默认行高，为DPI自适应处理后的值
    size_t nItemData = 0;           //用户自定义数据
};


/** 列表中使用的CheckBox
*/
class ListCtrlCheckBox: public CheckBox
{
public:
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
        UiEstSize sz = EstimateSize(UiSize(0, 0));
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

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_DEFS_H_
