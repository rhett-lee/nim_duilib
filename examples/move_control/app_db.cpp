#include "stdafx.h"
#include "app_db.h"

CAppDb::CAppDb()
{
	//这里可以打开db的一些操作 
}


CAppDb::~CAppDb()
{
	//这里可以关闭db的一些操作
}

bool CAppDb::LoadFromDb(std::vector<AppItem>& app_list)
{
	app_list.clear();
	//select from....
	//目前写死
	{
		AppItem item;
		item.m_id = "1";
		item.m_icon = L"1.png";
		item.m_isFrequent = false;
		item.m_name = L"城市服务";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "2";
		item.m_icon = L"2.png";
		item.m_isFrequent = false;
		item.m_name = L"天气预报";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "3";
		item.m_icon = L"3.png";
		item.m_isFrequent = true;
		item.m_name = L"考勤";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "4";
		item.m_icon = L"4.png";
		item.m_isFrequent = true;
		item.m_name = L"工资单";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "5";
		item.m_icon = L"5.png";
		item.m_isFrequent = false;
		item.m_name = L"测试5";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "6";
		item.m_icon = L"5.png";
		item.m_isFrequent = false;
		item.m_name = L"测试6";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "7";
		item.m_icon = L"5.png";
		item.m_isFrequent = false;
		item.m_name = L"测试7";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "8";
		item.m_icon = L"5.png";
		item.m_isFrequent = false;
		item.m_name = L"测试8";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "9";
		item.m_icon = L"5.png";
		item.m_isFrequent = false;
		item.m_name = L"测试9";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "10";
		item.m_icon = L"5.png";
		item.m_isFrequent = false;
		item.m_name = L"测试10";
		app_list.emplace_back(item);
	}
	{
		AppItem item;
		item.m_id = "11";
		item.m_icon = L"5.png";
		item.m_isFrequent = false;
		item.m_name = L"测试11";
		app_list.emplace_back(item);
	}
	
	return true;
}

bool CAppDb::SaveToDb(const AppItem& item)
{
	// insert or replace into ....
	return true;
}
