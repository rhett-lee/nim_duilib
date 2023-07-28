#include "stdafx.h"
#include "richlist_form.h"
#include "item.h"

const std::wstring RichlistForm::kClassName = L"Basic";

RichlistForm::RichlistForm()
{
}


RichlistForm::~RichlistForm()
{
}

std::wstring RichlistForm::GetSkinFolder()
{
	return L"list_box";
}

std::wstring RichlistForm::GetSkinFile()
{
	return L"list_box.xml";
}

std::wstring RichlistForm::GetWindowClassName() const
{
	return kClassName;
}

void RichlistForm::OnInitWindow()
{
	list_ = dynamic_cast<ui::ListBox*>(FindControl(L"list"));

	for (auto i = 0; i < 100; i++)
	{
		Item* item = new Item;
		ui::GlobalManager::Instance().FillBoxWithCache(item, L"list_box/item.xml");

		std::wstring img = L"icon.png";
		std::wstring title = nbase::StringPrintf(L"下载任务 [%02d]", i + 1);

		item->InitSubControls(img, title);
		list_->AddItem(item);
	}

	// 监听列表中点击选择子项的事件
	list_->AttachSelect(nbase::Bind(&RichlistForm::OnSelected, this, std::placeholders::_1));
}


bool RichlistForm::OnSelected(const ui::EventArgs& args)
{
	int current = static_cast<int>(args.wParam);
	int old = static_cast<int>(args.lParam);


	auto message = nbase::StringPrintf(L"您选择了索引为 %d 的子项，上一次选择子项索引为 %d", current, old);
	nim_comp::ShowMsgBox(GetHWND(), nim_comp::MsgboxCallback(), message, false, L"提示", false);

	return true;
}

LRESULT RichlistForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}
