// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_)
#define AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_

#pragma once

#ifndef __FILET__
#define __DUILIB_STR2WSTR(str)	L##str
#define _DUILIB_STR2WSTR(str)	__DUILIB_STR2WSTR(str)
#ifdef _UNICODE
#define __FILET__	_DUILIB_STR2WSTR(__FILE__)
#define __FUNCTIONT__	_DUILIB_STR2WSTR(__FUNCTION__)
#else
#define __FILET__	__FILE__
#define __FUNCTIONT__	__FUNCTION__
#endif
#endif

#define _CRT_SECURE_NO_DEPRECATE

#ifdef __BORLANDC__
#pragma option -w-8027		   // function not expanded inline
#endif

// Must define GDIPVER before include UIlib.h
#ifdef GDIPVER
#undef GDIPVER
#endif
#define GDIPVER 0x0110

#include "UIlib.h"
#include <olectl.h>
#include <GdiPlus.h>

#endif // !defined(AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_)
