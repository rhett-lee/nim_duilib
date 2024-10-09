#include "SystemUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Core/Window.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include <shellapi.h>

namespace ui
{

bool SystemUtil::OpenUrl(const DString& url)
{
    if (url.empty()) {
        return false;
    }
    return ::ShellExecuteW(NULL, L"open", StringConvert::TToUTF16(url).c_str(), NULL, NULL, SW_SHOWDEFAULT) > (HINSTANCE)32;
}

bool SystemUtil::ShowMessageBox(const Window* pWindow, const DString& content, const DString& title)
{
    HWND hWnd = (pWindow != nullptr) ? pWindow->NativeWnd()->GetHWND() : nullptr;
    ::MessageBoxW(hWnd, StringConvert::TToUTF16(content).c_str(), StringConvert::TToUTF16(title).c_str(), MB_OK);
    return true;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
