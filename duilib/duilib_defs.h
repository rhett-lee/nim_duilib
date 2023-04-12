// Copyright (c) 2010-2011, duilib develop team(www.duilib.com).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or 
// without modification, are permitted provided that the 
// following conditions are met.
//
// Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above 
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials 
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#if defined(UILIB_DLL)
    #if defined(UILIB_EXPORTS)
        #if defined(_MSC_VER)
            #define UILIB_API __declspec(dllexport)
        #else
            #define UILIB_API 
        #endif
    #else
        #if defined(_MSC_VER)
            #define UILIB_API __declspec(dllimport)
        #else
            #define UILIB_API 
        #endif
    #endif
#else
    #define UILIB_API
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1928)
    //支持UI自动化的编译选项，默认关闭
    //#define ENABLE_UIAUTOMATION
#endif

#ifndef VC_EXTRALEAN
    #define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN			// remove rarely used header files, including 'winsock.h'
    #define WIN32_LEAN_AND_MEAN			// which will conflict with 'winsock2.h'
#endif

#ifndef WINVER
    #define WINVER _WIN32_WINNT_WINXP
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT _WIN32_WINNT_WINXP
#endif

#ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS _WIN32_WINNT_WINXP
#endif

#ifndef _WIN32_IE
    #define _WIN32_IE _WIN32_IE_IE60
#endif

#ifndef NOMINMAX
    #define NOMINMAX 1
#endif

#ifndef ASSERT
    #define ASSERT(expr)  _ASSERTE(expr)
#endif

#include <SDKDDKVer.h>
#include <windows.h>
#include <crtdbg.h>
