#pragma once

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_task.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#pragma warning (pop)

#include "duilib/duilib_defs.h"

#define REQUIRE_UI_THREAD()   ASSERT(CefCurrentlyOn(TID_UI));
#define REQUIRE_IO_THREAD()   ASSERT(CefCurrentlyOn(TID_IO));
#define REQUIRE_FILE_THREAD() ASSERT(CefCurrentlyOn(TID_FILE));