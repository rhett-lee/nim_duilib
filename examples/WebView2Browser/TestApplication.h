#ifndef EXAMPLES_MAIN_TEST_APPLICATION_H_
#define EXAMPLES_MAIN_TEST_APPLICATION_H_

// duilib
#include "duilib/duilib.h"
#include <vector>

/** 主程序入口
*/
class TestApplication
{
private:
    TestApplication();
    ~TestApplication();
    TestApplication(const TestApplication&) = delete;
    TestApplication& operator = (const TestApplication&) = delete;

public:
    /** 获取单例对象
    */
    static TestApplication& Instance();

    /** 运行程序功能
    */
    int Run(int argc, char** argv);

    /** 添加窗口接口
    */
    void AddMainWindow(ui::Window* pWindow);

    /** 移除窗口接口
    */
    void RemoveMainWindow(ui::Window* pWindow);

    /** 设置活动窗口
    */
    void SetActiveMainWindow(ui::Window* pWindow);

    /** 激活窗口
    */
    void ActiveMainWindow();

    /** 关闭所有的窗口
    */
    void CloseMainWindow();

    /** 退出进程
    */
    void PostQuitMessage();

private:
    /** 主窗口列表
    */
    std::vector<ui::ControlPtrT<ui::Window>> m_pMainWindows;

    /** 活动窗口
    */
    ui::ControlPtrT<ui::Window> m_pActiveWindow;
};

#endif // EXAMPLES_MAIN_TEST_APPLICATION_H_
