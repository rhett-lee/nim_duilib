// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

// duilib
#include "duilib/duilib.h"
#include "duilib/duilib_cef.h"
#include "duilib/CEFControl/internal/CefClientApp.h"

#include "include/cef_app.h"
#include "include/wrapper/cef_library_loader.h"

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to disable
// use of the sandbox.
#if defined(CEF_USE_SANDBOX)
#include "include/cef_sandbox_mac.h"
#endif

namespace ui
{
/** CEF模块子进程的入口函数（macOS平台）
*/
int RunMain(int argc, char* argv[])
{
#if defined(CEF_USE_SANDBOX)
    // Initialize the macOS sandbox for this helper process.
    CefScopedSandboxContext sandbox_context;
    if (!sandbox_context.Initialize(argc, argv)) {
        return 1;
    }
#endif

    // Load the CEF framework library at runtime instead of linking directly
    // as required by the macOS sandbox implementation.
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInHelper()) {
        return 1;
    }

    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefClientApp> app(new CefClientApp);    

    // Execute the secondary process.
    return CefExecuteProcess(main_args, app.get(), nullptr);
}

}  // namespace ui

// Entry point function for sub-processes.
int main(int argc, char* argv[])
{
    return ui::RunMain(argc, argv);
}
