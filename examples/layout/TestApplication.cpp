#include "TestApplication.h"
#include "MainThread.h"
#include "duilib/Utils/PerformanceUtil.h"

TestApplication::TestApplication()
{
}

TestApplication::~TestApplication()
{
}

void TestApplication::Run()
{
    //性能统计
    ui::PerformanceStat stat(_T("TestApplication::Run"));

    // 创建主线程
    MainThread thread;

    // 执行主线程循环
    thread.RunMessageLoop();
}
