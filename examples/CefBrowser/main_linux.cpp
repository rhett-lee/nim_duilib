#if defined(linux) || defined(__linux) || defined(__linux__)

#include "TestApplication.h"

//定义应用程序的入口点
int main(int argc, char** argv)
{
    TestApplication::Instance().Run((argc, argv);
    return 0;
}

#endif
