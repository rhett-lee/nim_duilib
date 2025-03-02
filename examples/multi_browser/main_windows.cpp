#if defined (_WIN32) || defined (_WIN64)

#include "duilib/duilib_config_windows.h"
#include "TestApplication.h"

//定义应用程序的入口点
int APIENTRY wWinMain(_In_ HINSTANCE /*hInstance*/,
                      _In_opt_ HINSTANCE /*hPrevInstance*/,
                      _In_ LPWSTR    /*lpCmdLine*/,
                      _In_ int       /*nCmdShow*/)
{
    TestApplication app;
    app.Run(0, nullptr);
    return 0;
}

#endif
