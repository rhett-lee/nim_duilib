#if defined(__FreeBSD__)

#include "duilib/duilib_config.h"
#include "TestApplication.h"

#if defined(DUILIB_BUILD_FOR_SDL)
    #include "duilib/Core/MessageLoop_SDL.h"
    #include "duilib/Utils/StringConvert.h"
#endif

//定义应用程序的入口点
int main(int argc, char** argv)
{
#if defined(DUILIB_BUILD_FOR_SDL)
    if ((argv != nullptr) && (argc >=2)) {
        //第1个参数：DriverName
        DStringA videoDriverName = argv[1];
        if (!videoDriverName.empty()) {
            ui::MessageLoop_SDL::CheckInitSDL(ui::StringConvert::UTF8ToT(videoDriverName));
        }
    }
#endif

    TestApplication app;
    app.Run();
    return 0;
}

#endif
