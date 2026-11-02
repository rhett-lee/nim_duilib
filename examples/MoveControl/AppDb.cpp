#include "AppDb.h"

AppDb::AppDb()
{
    //这里可以打开db的一些操作
}

AppDb::~AppDb()
{
    //这里可以关闭db的一些操作
}

void AppDb::InitDefaultData()
{
    //演示数据：真实项目中这些数据来自数据库或服务器下发，这里直接写死
    //m_icon 是相对于皮肤目录(move_control)的图标文件名，使用 SVG 矢量图，缩放不失真
    m_appList = {
        { "1",  _T("STRID_MOVECONTROL_TEST_01"), _T("app_city.svg"),       false },
        { "2",  _T("STRID_MOVECONTROL_TEST_02"), _T("app_weather.svg"),    false },
        { "3",  _T("STRID_MOVECONTROL_TEST_03"), _T("app_attendance.svg"), true  },
        { "4",  _T("STRID_MOVECONTROL_TEST_04"), _T("app_salary.svg"),     true  },
        { "5",  _T("STRID_MOVECONTROL_TEST_05"), _T("app_mail.svg"),       false },
        { "6",  _T("STRID_MOVECONTROL_TEST_06"), _T("app_calendar.svg"),   false },
        { "7",  _T("STRID_MOVECONTROL_TEST_07"), _T("app_cloud.svg"),      false },
        { "8",  _T("STRID_MOVECONTROL_TEST_08"), _T("app_contacts.svg"),   false },
        { "9",  _T("STRID_MOVECONTROL_TEST_09"), _T("app_approval.svg"),   false },
        { "10", _T("STRID_MOVECONTROL_TEST_10"), _T("app_report.svg"),     false },
    };
    m_bInited = true;
}

bool AppDb::LoadFromDb(std::vector<AppItem>& app_list)
{
    if (!m_bInited) {
        InitDefaultData();
    }
    app_list = m_appList;
    return true;
}

bool AppDb::SaveToDb(const AppItem& item)
{
    //内存模拟数据库：更新已有项的"常用"状态，新应用则追加
    for (AppItem& appItem : m_appList) {
        if (appItem.m_id == item.m_id) {
            appItem.m_isFrequent = item.m_isFrequent;
            return true;
        }
    }
    m_appList.push_back(item);
    return true;
}
