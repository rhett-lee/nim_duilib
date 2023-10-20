#ifndef UI_CONTROL_LIST_CTRL_H_
#define UI_CONTROL_LIST_CTRL_H_

#pragma once

#include "duilib/Box/VBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VirtualListBox.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/Split.h"

namespace ui
{

/** 列表的列属性
*/
class ListCtrlColumn
{
public:
    /** 构造函数
    */
    ListCtrlColumn():
        m_width(0),
        m_widthMin(0),
        m_widthMax(0),
        m_bSortable(false),
        m_bResizeable(false),
        m_bAutoWidth(0),
        m_nSortIconSpace(0)
    {

    }

    /** 构造函数
    */
    ListCtrlColumn(const std::wstring& text, int32_t width):
        ListCtrlColumn()
    {
        m_text = text;
        m_width = width;
    }
    /** 文字
    */
    UiString m_text;

    /** 宽度
    */
    int32_t m_width;

    /** 最小宽度
    */
    int32_t m_widthMin;

    /** 最大宽度
    */
    int32_t m_widthMax;

    /** 是否允许排序
    */
    bool m_bSortable;

    /** 是否允许改变列宽
    */
    bool m_bResizeable;

    /** 是否列宽自适应
    */
    bool m_bAutoWidth;

    /** 绘制文字内边距
    */
    UiPadding m_textPadding;

    /** 文字对齐方式
    */
    UiString m_textAligh;

    /** 排序图标与文字间距
    */
    int32_t m_nSortIconSpace;
};

/** ListCtrl子项控件
*/
class ListCtrlItem: public ListBoxItem
{
public:
	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override { return L"ListCtrlItem"; }

};

//列表数据管理类
class ListCtrlData;
class ListCtrlItemProvider;

/** ListCtrl控件
*/
class ListCtrl: public VBox
{
    friend class ListCtrlHeader;
public:
	ListCtrl();
	virtual ~ListCtrl();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

protected:
	/** 控件初始化
	*/
	virtual void DoInit() override;

private:
    /** 调整列的宽度
    */
    void OnColumnWidthChanged(size_t nColumn1, size_t nColumn2);

private:
	/** 初始化标志
	*/
	bool m_bInited;

	/** 表头控件
	*/
	ListCtrlHeader* m_pListCtrlHeader;

	/** 列表数据展示
	*/
	ListCtrlData* m_pListCtrlData;

	/** 列表数据管理
	*/
	std::unique_ptr<ListCtrlItemProvider> m_spItemProvider;
};

/** ListCtrl的表头控件
*/
class ListCtrlHeader : public ListBoxItem
{
public:
    ListCtrlHeader() :
        m_pListCtrl(nullptr)
    {
        m_nSplitWidth = ui::GlobalManager::Instance().Dpi().GetScaleInt(3);
    }

    /** 获取控件类型
    */
    virtual std::wstring GetType() const override { return L"ListCtrlHeader"; }

public:
    /** 添加1列
    */
    void AddColumn(const ListCtrlColumn& column)
    {
        m_listColumns.push_back(column);
    }

    /** 获取列的个数
    */
    size_t GetColumnCount() const
    {
        return m_listColumns.size();
    }

    /** 获取列数据
    */
    bool GetColumn(size_t index, ListCtrlColumn& column) const
    {
        if (index < m_listColumns.size()) {
            column = m_listColumns[index];
            return true;
        }
        return false;
    }

public:

    /** 设置关联的ListCtrl接口
    */
    void SetListCtrl(ListCtrl* pListCtrl)
    {
        m_pListCtrl = pListCtrl;
    }

    /** 同步列数据对应的控件
    */
    void OnColumnsChanged()
    {
        //基本结构：
        //  <HBox>
        //      <Label/>
        //      <Split/>
        //  </HBox>
        HBox* pHeaderHBox = nullptr;
        if (GetItemCount() > 0) {
            pHeaderHBox = dynamic_cast<HBox*>(GetItemAt(0));
            ASSERT(pHeaderHBox != nullptr);
            if (pHeaderHBox == nullptr) {
                return;
            }
        }
        if (pHeaderHBox == nullptr) {
            pHeaderHBox = new HBox;
            AddItem(pHeaderHBox);
        }
        for (size_t index = 0; index < m_listColumns.size(); ++index) {
            const ListCtrlColumn& column = m_listColumns[index];
            Label* pLabel = nullptr;
            SplitBox* pSplit = nullptr;
            if (index < (pHeaderHBox->GetItemCount() / 2)) {
                pLabel = dynamic_cast<Label*>(pHeaderHBox->GetItemAt(index * 2));
                pSplit = dynamic_cast<SplitBox*>(pHeaderHBox->GetItemAt(index * 2 + 1));
                ASSERT((pLabel != nullptr) && (pSplit != nullptr));
                if ((pLabel == nullptr) || (pSplit == nullptr)) {
                    return;
                }
            }
            else {
                pLabel = new Label;
                pSplit = new SplitBox;
                pHeaderHBox->AddItem(pLabel);
                pHeaderHBox->AddItem(pSplit);

                Control* pSplitCtrl = new Control;
                pSplitCtrl->SetAttribute(L"width", L"1");
                pSplitCtrl->SetAttribute(L"height", L"100%");
                pSplitCtrl->SetAttribute(L"margin", L"2,4,0,2");
                pSplitCtrl->SetBkColor(L"splitline_level1");
                pSplitCtrl->SetMouseEnabled(false);
                pSplitCtrl->SetMouseFocused(false);
                pSplitCtrl->SetNoFocus();
                pSplit->AddItem(pSplitCtrl);
            }

            const int32_t nSplitWidth = m_nSplitWidth;
            pSplit->SetFixedWidth(UiFixedInt(nSplitWidth), true, false);
            pSplit->SetAttribute(L"height", L"32");
            pSplit->SetBkColor(L"Yellow");

            int32_t width = column.m_width - nSplitWidth;
            if (width < 0) {
                width = 0;
            }
            pLabel->SetText(column.m_text.c_str());
            pLabel->SetBkColor(L"Yellow");
            pLabel->SetFixedWidth(UiFixedInt(width), true, false);
            pLabel->SetAttribute(L"height", L"32");
            pLabel->SetAttribute(L"text_align", L"vcenter,hcenter");

            /*pLabel->SetAttribute(L"padding", L"1,1,1,1");
            pLabel->SetAttribute(L"border_size", L"1");
            pLabel->SetAttribute(L"border_color", L"blue");*/

            //保存列序号
            pLabel->SetUserDataID(index);

            //挂载拖动响应事件
            pSplit->AttachSplitDraged([this](const EventArgs& args) {
                OnSplitDraged((Control*)args.wParam, (Control*)args.lParam);
                return true;
                });
        }
    }

    /** 拖动响应事件
    */
    void OnSplitDraged(const Control* pLeftControl, const Control* pRightControl)
    {
        size_t nColumn1 = Box::InvalidIndex;
        size_t nColumn2 = Box::InvalidIndex;
        if (pLeftControl != nullptr) {
            nColumn1 = pLeftControl->GetUserDataID();
            if (nColumn1 < m_listColumns.size()) {
                ListCtrlColumn& column = m_listColumns[nColumn1];
                const int32_t nSplitWidth = m_nSplitWidth;
                column.m_width = pLeftControl->GetFixedWidth().GetInt32();
                column.m_width += nSplitWidth;
            }
        }
        if (pRightControl != nullptr) {
            nColumn2 = pRightControl->GetUserDataID();
            if (nColumn2 < m_listColumns.size()) {
                const int32_t nSplitWidth = m_nSplitWidth;
                ListCtrlColumn& column = m_listColumns[nColumn2];
                column.m_width = pRightControl->GetFixedWidth().GetInt32();
                column.m_width += nSplitWidth;
            }
        }

        if ((nColumn1 != Box::InvalidIndex) || (nColumn2 != Box::InvalidIndex)){
            if (m_pListCtrl != nullptr) {
                m_pListCtrl->OnColumnWidthChanged(nColumn1, nColumn2);
            }
        }
    }

protected:
    /** 列数据
    */
    std::vector<ListCtrlColumn> m_listColumns;

    /** 关联的ListCtrl接口
    */
    ListCtrl* m_pListCtrl;

    /** 分割条的宽度
    */
    int32_t m_nSplitWidth;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_H_
