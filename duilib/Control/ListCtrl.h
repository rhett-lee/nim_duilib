#ifndef UI_CONTROL_LIST_CTRL_H_
#define UI_CONTROL_LIST_CTRL_H_

#pragma once

#include "duilib/Box/VBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VirtualListBox.h"
namespace ui
{

/** ListCtrl的表头控件
*/
class ListCtrlHeader: public HBox
{
public:
	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override { return L"ListCtrlHeader"; }

};

/** ListCtrl的表头子项控件
*/
class ListCtrlHeaderItem: public HBox
{
public:
	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override { return L"ListCtrlHeaderItem"; }

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

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_H_
