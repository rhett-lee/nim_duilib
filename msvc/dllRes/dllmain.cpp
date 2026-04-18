// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.h"

#ifdef DUILIB_DLL

#include "duilib/Core/GlobalManager.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ui::GlobalManager::Instance().SetPlatformData((void*)hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        ui::GlobalManager::Instance().SetPlatformData(nullptr);
        break;
    }
    return TRUE;
}

#endif //DUILIB_DLL
