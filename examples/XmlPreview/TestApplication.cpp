#include "TestApplication.h"
#include "MainThread.h"

TestApplication::TestApplication()
{
}

TestApplication::~TestApplication()
{
}

void TestApplication::Run()
{
    // 创建主线程
    MainThread thread;

    // 执行主线程循环
    thread.RunMessageLoop();
}
