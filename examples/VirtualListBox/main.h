#ifndef EXAMPLES_MAIN_H_
#define EXAMPLES_MAIN_H_

// duilib
#include "duilib/duilib.h"

#include "resource.h"

class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread();

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;
};

#endif //EXAMPLES_MAIN_H_
