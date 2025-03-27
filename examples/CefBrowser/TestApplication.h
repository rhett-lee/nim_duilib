#ifndef EXAMPLES_MAIN_TEST_APPLICATION_H_
#define EXAMPLES_MAIN_TEST_APPLICATION_H_

/** 主程序入口
*/
class TestApplication
{
public:
    TestApplication();
    ~TestApplication();

public:
    /** 运行程序功能
    */
    int Run(int argc, char** argv);
};

#endif // EXAMPLES_MAIN_TEST_APPLICATION_H_
