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

	for (auto i = 0; i < 1000; i++)
	{
		Item* item = new Item;
		item->SetWindow(this);
		ui::GlobalManager::Instance().FillBoxWithCache(item, L"list_box/item.xml");

		std::wstring img = L"icon.png";
		std::wstring title = nbase::StringPrintf(L"�������� [%02d]", i + 1);

		item->InitSubControls(img, title);
		list_->AddItem(item);
	}

	// �����б��е��ѡ��������¼�
	list_->AttachSelect(nbase::Bind(&RichlistForm::OnSelected, this, std::placeholders::_1));
}


bool RichlistForm::OnSelected(const ui::EventArgs& args)
{
	int current = static_cast<int>(args.wParam);
	int old = static_cast<int>(args.lParam);


	//auto message = nbase::StringPrintf(L"��ѡ��������Ϊ %d �������һ��ѡ����������Ϊ %d", current, old);
	//nim_comp::ShowMsgBox(GetHWND(), nim_comp::MsgboxCallback(), message, false, L"��ʾ", false);

	return true;
}

LRESULT RichlistForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}