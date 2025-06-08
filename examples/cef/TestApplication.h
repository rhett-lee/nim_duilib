#ifndef EXAMPLES_MAIN_TEST_APPLICATION_H_
#define EXAMPLES_MAIN_TEST_APPLICATION_H_

// duilib
#include "duilib/duilib.h"

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

    /** 设置主窗口接口
    */
    void SetMainWindow(ui::Window* pWindow);

    /** 激活主窗口
    */
    void ActiveMainWindow();

    /** 关闭主窗口
    */
    void CloseMainWindow();

private:
    /** 主窗口
    */
    ui::ControlPtrT<ui::Window> m_pMainWindow;
};

#endif // EXAMPLES_MAIN_TEST_APPLICATION_H_
