#include "CefClientApp.h" 

#include "duilib/CEFControl/util/CefUtil.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_cookie.h"
#include "include/cef_process_message.h"
#include "include/cef_task.h"
#include "include/cef_v8.h"
#pragma warning (pop)

namespace ui
{
//////////////////////////////////////////////////////////////////////////////////////////
// CefBrowserProcessHandler methods.
void CefClientApp::OnContextInitialized() 
{
    
}

}
