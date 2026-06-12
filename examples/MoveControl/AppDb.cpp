#include "AppDb.h"

AppDb::AppDb()
{
    //这里可以打开db的一些操作 
}

AppDb::~AppDb()
{
    //这里可以关闭db的一些操作
}

bool AppDb::LoadFromDb(std::vector<AppItem>& app_list)
{
    app_list.clear();
    //select from....
    //目前写死
    {
        AppItem item;
        item.m_id = "1";
        item.m_icon = _T("1.png");
        item.m_isFrequent = false;
        item.m_name = _T("STRID_MOVECONTROL_TEST_01");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "2";
        item.m_icon = _T("2.png");
        item.m_isFrequent = false;
        item.m_name = _T("STRID_MOVECONTROL_TEST_02");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "3";
        item.m_icon = _T("3.png");
        item.m_isFrequent = true;
        item.m_name = _T("STRID_MOVECONTROL_TEST_03");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "4";
        item.m_icon = _T("4.png");
        item.m_isFrequent = true;
        item.m_name = _T("STRID_MOVECONTROL_TEST_04");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "5";
        item.m_icon = _T("5.png");
        item.m_isFrequent = false;
        item.m_name = _T("STRID_MOVECONTROL_TEST_05");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "6";
        item.m_icon = _T("5.png");
        item.m_isFrequent = false;
        item.m_name = _T("STRID_MOVECONTROL_TEST_06");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "7";
        item.m_icon = _T("5.png");
        item.m_isFrequent = false;
        item.m_name = _T("STRID_MOVECONTROL_TEST_07");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "8";
        item.m_icon = _T("5.png");
        item.m_isFrequent = false;
        item.m_name = _T("STRID_MOVECONTROL_TEST_08");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "9";
        item.m_icon = _T("5.png");
        item.m_isFrequent = false;
        item.m_name = _T("STRID_MOVECONTROL_TEST_09");
        app_list.emplace_back(item);
    }
    {
        AppItem item;
        item.m_id = "10";
        item.m_icon = _T("5.png");
        item.m_isFrequent = false;
        item.m_name = _T("STRID_MOVECONTROL_TEST_10");
        app_list.emplace_back(item);
    }   
    return true;
}

bool AppDb::SaveToDb(const AppItem& item)
{
    // insert or replace into ....
    return true;
}
