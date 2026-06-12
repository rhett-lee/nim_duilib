#if defined(__FreeBSD__)

#include "duilib/duilib_config_freebsd.h"
#include "TestApplication.h"

//定义应用程序的入口点
int main(int argc, char** argv)
{
    TestApplication app;
    app.Run();
    return 0;
}

#endif
