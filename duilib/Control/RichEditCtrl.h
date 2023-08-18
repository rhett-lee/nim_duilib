#ifndef UI_CONTROL_RICHEDIT_CTRL_H_
#define UI_CONTROL_RICHEDIT_CTRL_H_

#pragma once

#include "duilib/Core/UiTypes.h"
#include <Richedit.h>
#include <TextServ.h>
#include <RichOle.h>
#include <string>

namespace ui
{

#if !defined(_UNICODE) && (_RICHEDIT_VER >= 0x0500)
  #undef MSFTEDIT_CLASS
  #define MSFTEDIT_CLASS	"RICHEDIT50W"
#endif

/** RichEdit控件主要功能封装（来自WTL源码）
*/
class RichEditCtrl
{
public:
// Constructors
	RichEditCtrl(): m_pTextServices(nullptr)
	{ }


// Attributes
	static LPCTSTR GetWndClassName()
	{
#if (_RICHEDIT_VER >= 0x0500)
		return MSFTEDIT_CLASS;
#else
		return RICHEDIT_CLASS;
#endif
	}

	static LPCTSTR GetLibraryName()
	{
#if (_RICHEDIT_VER >= 0x0500)
		return L"MSFTEDIT.DLL";
#else
		return L"RICHED20.DLL";
#endif
	}

	void SetTextServices(ITextServices* pTextServices)
	{
		m_pTextServices = pTextServices;
	}

	LRESULT TxSendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		LRESULT lResult = 0;
		ASSERT(m_pTextServices != nullptr);
		if (m_pTextServices != nullptr) {
			m_pTextServices->TxSendMessage(uMsg, wParam, lParam, &lResult);
		}
		return lResult;
	}

	int GetLineCount() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_GETLINECOUNT, 0, 0L);
	}

	BOOL GetModify() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_GETMODIFY, 0, 0L);
	}

	void SetModify(BOOL bModified = TRUE)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SETMODIFY, bModified, 0L);
	}

	void GetRect(LPRECT lpRect) const
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_GETRECT, 0, (LPARAM)lpRect);
	}

	DWORD GetOptions() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_GETOPTIONS, 0, 0L);
	}

	DWORD SetOptions(WORD wOperation, DWORD dwOptions)
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_SETOPTIONS, wOperation, dwOptions);
	}

	// NOTE: first word in lpszBuffer must contain the size of the buffer!
	int GetLine(int nIndex, LPTSTR lpszBuffer) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_GETLINE, nIndex, (LPARAM)lpszBuffer);
	}

	int GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const
	{
		ASSERT(m_pTextServices != nullptr);
		ASSERT(lpszBuffer != nullptr);
		if (lpszBuffer == nullptr) {
			return 0;
		}
		*(LPWORD)lpszBuffer = (WORD)nMaxLength;
		return (int)TxSendMessage(EM_GETLINE, nIndex, (LPARAM)lpszBuffer);
	}

	BOOL CanUndo() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_CANUNDO, 0, 0L);
	}

	BOOL CanPaste(UINT nFormat = 0) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_CANPASTE, nFormat, 0L);
	}

	void GetSel(LONG& nStartChar, LONG& nEndChar) const
	{
		ASSERT(m_pTextServices != nullptr);
		CHARRANGE cr = {};
		TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr);
		nStartChar = cr.cpMin;
		nEndChar = cr.cpMax;
	}

	void GetSel(CHARRANGE& cr) const
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr);
	}

	int SetSel(LONG nStartChar, LONG nEndChar)
	{
		ASSERT(m_pTextServices != nullptr);
		CHARRANGE cr = { nStartChar, nEndChar };
		return (int)TxSendMessage(EM_EXSETSEL, 0, (LPARAM)&cr);
	}

	int SetSel(CHARRANGE& cr)
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_EXSETSEL, 0, (LPARAM)&cr);
	}

	int SetSelAll()
	{
		return SetSel(0, -1);
	}

	int SetSelNone()
	{
		return SetSel(-1, 0);
	}

	DWORD GetDefaultCharFormat(CHARFORMAT& cf) const
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT);
		return (DWORD)TxSendMessage(EM_GETCHARFORMAT, 0, (LPARAM)&cf);
	}

	DWORD GetSelectionCharFormat(CHARFORMAT& cf) const
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT);
		return (DWORD)TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM)&cf);
	}

	DWORD GetEventMask() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_GETEVENTMASK, 0, 0L);
	}

	void SetLimitText(LONG nChars) const
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SETLIMITTEXT, (WPARAM)nChars, 0L);
	}

	LONG GetLimitText() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (LONG)TxSendMessage(EM_GETLIMITTEXT, 0, 0L);
	}

	DWORD GetParaFormat(PARAFORMAT& pf) const
	{
		ASSERT(m_pTextServices != nullptr);
		pf.cbSize = sizeof(PARAFORMAT);
		return (DWORD)TxSendMessage(EM_GETPARAFORMAT, 0, (LPARAM)&pf);
	}

	LONG GetSelText(LPTSTR lpstrBuff) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (LONG)TxSendMessage(EM_GETSELTEXT, 0, (LPARAM)lpstrBuff);
	}

	BOOL GetSelText(std::wstring& text) const
	{
		ASSERT(m_pTextServices != nullptr);

		CHARRANGE cr = {};
		TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr);

		text.clear();
		int32_t textLen = cr.cpMax - cr.cpMin + 1;
		if (textLen <= 0) {
			return TRUE;
		}
		wchar_t* pText = new wchar_t[textLen];
		if (pText == nullptr) {
			return FALSE;
		}
		memset(pText, 0, sizeof(wchar_t) * textLen);
		if (TxSendMessage(EM_GETSELTEXT, 0, (LPARAM)pText) == 0) {
			delete[] pText;
			return FALSE;
		}
		text = pText;
		delete[] pText;
		return TRUE;
	}

	WORD GetSelectionType() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (WORD)TxSendMessage(EM_SELECTIONTYPE, 0, 0L);
	}

	COLORREF SetBackgroundColor(COLORREF cr)
	{
		ASSERT(m_pTextServices != nullptr);
		return (COLORREF)TxSendMessage(EM_SETBKGNDCOLOR, 0, cr);
	}

	COLORREF SetBackgroundColor()   // sets to system background
	{
		ASSERT(m_pTextServices != nullptr);
		return (COLORREF)TxSendMessage(EM_SETBKGNDCOLOR, 1, 0);
	}

	BOOL SetCharFormat(CHARFORMAT& cf, WORD wFlags)
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT);
		return (BOOL)TxSendMessage(EM_SETCHARFORMAT, (WPARAM)wFlags, (LPARAM)&cf);
	}

	BOOL SetDefaultCharFormat(CHARFORMAT& cf)
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT);
		return (BOOL)TxSendMessage(EM_SETCHARFORMAT, 0, (LPARAM)&cf);
	}

	BOOL SetSelectionCharFormat(CHARFORMAT& cf)
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT);
		return (BOOL)TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}

	BOOL SetWordCharFormat(CHARFORMAT& cf)
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT);
		return (BOOL)TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf);
	}

	DWORD SetEventMask(DWORD dwEventMask)
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_SETEVENTMASK, 0, dwEventMask);
	}

	BOOL SetParaFormat(PARAFORMAT& pf)
	{
		ASSERT(m_pTextServices != nullptr);
		pf.cbSize = sizeof(PARAFORMAT);
		return (BOOL)TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);
	}

	BOOL SetTargetDevice(HDC hDC, int cxLineWidth)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETTARGETDEVICE, (WPARAM)hDC, cxLineWidth);
	}

	int GetTextLength() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(WM_GETTEXTLENGTH, 0, 0L);
	}

	BOOL SetReadOnly(BOOL bReadOnly = TRUE)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETREADONLY, bReadOnly, 0L);
	}

	int GetFirstVisibleLine() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_GETFIRSTVISIBLELINE, 0, 0L);
	}

	int GetTextRange(TEXTRANGE* pTextRange) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)pTextRange);
	}

	int GetTextRange(LONG nStartChar, LONG nEndChar, LPTSTR lpstrText) const
	{
		ASSERT(m_pTextServices != nullptr);
		TEXTRANGE tr = {};
		tr.chrg.cpMin = nStartChar;
		tr.chrg.cpMax = nEndChar;
		tr.lpstrText = lpstrText;
		return (int)TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	}

	DWORD GetDefaultCharFormat(CHARFORMAT2& cf) const
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT2);
		return (DWORD)TxSendMessage(EM_GETCHARFORMAT, 0, (LPARAM)&cf);
	}

	BOOL SetCharFormat(CHARFORMAT2& cf, WORD wFlags)
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT2);
		return (BOOL)TxSendMessage(EM_SETCHARFORMAT, (WPARAM)wFlags, (LPARAM)&cf);
	}

	BOOL SetDefaultCharFormat(CHARFORMAT2& cf)
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT2);
		return (BOOL)TxSendMessage(EM_SETCHARFORMAT, 0, (LPARAM)&cf);
	}

	DWORD GetSelectionCharFormat(CHARFORMAT2& cf) const
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT2);
		return (DWORD)TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM)&cf);
	}

	BOOL SetSelectionCharFormat(CHARFORMAT2& cf)
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT2);
		return (BOOL)TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}

	BOOL SetWordCharFormat(CHARFORMAT2& cf)
	{
		ASSERT(m_pTextServices != nullptr);
		cf.cbSize = sizeof(CHARFORMAT2);
		return (BOOL)TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf);
	}

	DWORD GetParaFormat(PARAFORMAT2& pf) const
	{
		ASSERT(m_pTextServices != nullptr);
		pf.cbSize = sizeof(PARAFORMAT2);
		return (DWORD)TxSendMessage(EM_GETPARAFORMAT, 0, (LPARAM)&pf);
	}

	BOOL SetParaFormat(PARAFORMAT2& pf)
	{
		ASSERT(m_pTextServices != nullptr);
		pf.cbSize = sizeof(PARAFORMAT2);
		return (BOOL)TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);
	}

	TEXTMODE GetTextMode() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (TEXTMODE)TxSendMessage(EM_GETTEXTMODE, 0, 0L);
	}

	BOOL SetTextMode(TEXTMODE enumTextMode)
	{
		ASSERT(m_pTextServices != nullptr);
		return !(BOOL)TxSendMessage(EM_SETTEXTMODE, enumTextMode, 0L);
	}

	UNDONAMEID GetUndoName() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (UNDONAMEID)TxSendMessage(EM_GETUNDONAME, 0, 0L);
	}

	UNDONAMEID GetRedoName() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (UNDONAMEID)TxSendMessage(EM_GETREDONAME, 0, 0L);
	}

	BOOL CanRedo() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_CANREDO, 0, 0L);
	}

	BOOL GetAutoURLDetect() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_GETAUTOURLDETECT, 0, 0L);
	}

	BOOL SetAutoURLDetect(BOOL bAutoDetect = TRUE)
	{
		ASSERT(m_pTextServices != nullptr);
		return !(BOOL)TxSendMessage(EM_AUTOURLDETECT, bAutoDetect, 0L);
	}

	UINT SetUndoLimit(UINT uUndoLimit)
	{
		ASSERT(m_pTextServices != nullptr);
		return (UINT)TxSendMessage(EM_SETUNDOLIMIT, uUndoLimit, 0L);
	}

	void SetPalette(HPALETTE hPalette)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SETPALETTE, (WPARAM)hPalette, 0L);
	}

	int GetTextEx(GETTEXTEX* pGetTextEx, LPTSTR lpstrText) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_GETTEXTEX, (WPARAM)pGetTextEx, (LPARAM)lpstrText);
	}

	int GetTextEx(LPTSTR lpstrText, int nTextLen, DWORD dwFlags = GT_DEFAULT, UINT uCodePage = CP_ACP, LPCSTR lpDefaultChar = NULL, LPBOOL lpUsedDefChar = NULL) const
	{
		ASSERT(m_pTextServices != nullptr);
		GETTEXTEX gte = {};
		gte.cb = nTextLen * sizeof(TCHAR);
		gte.codepage = uCodePage;
		gte.flags = dwFlags;
		gte.lpDefaultChar = lpDefaultChar;
		gte.lpUsedDefChar = lpUsedDefChar;
		return (int)TxSendMessage(EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)lpstrText);
	}

	int GetTextLengthEx(GETTEXTLENGTHEX* pGetTextLengthEx) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)pGetTextLengthEx, 0L);
	}

	int GetTextLengthEx(DWORD dwFlags = GTL_DEFAULT, UINT uCodePage = CP_ACP) const
	{
		ASSERT(m_pTextServices != nullptr);
		GETTEXTLENGTHEX gtle = {};
		gtle.codepage = uCodePage;
		gtle.flags = dwFlags;
		return (int)TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)&gtle, 0L);
	}

	EDITWORDBREAKPROC GetWordBreakProc() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (EDITWORDBREAKPROC)TxSendMessage(EM_GETWORDBREAKPROC, 0, 0L);
	}

	void SetWordBreakProc(EDITWORDBREAKPROC ewbprc)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SETWORDBREAKPROC, 0, (LPARAM)ewbprc);
	}

	int SetTextEx(SETTEXTEX* pSetTextEx, LPCTSTR lpstrText)
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_SETTEXTEX, (WPARAM)pSetTextEx, (LPARAM)lpstrText);
	}

	int SetTextEx(LPCTSTR lpstrText, DWORD dwFlags = ST_DEFAULT, UINT uCodePage = CP_ACP)
	{
		ASSERT(m_pTextServices != nullptr);
		SETTEXTEX ste = {};
		ste.flags = dwFlags;
		ste.codepage = uCodePage;
		return (int)TxSendMessage(EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpstrText);
	}

	int GetEditStyle() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_GETEDITSTYLE, 0, 0L);
	}

	int SetEditStyle(int nStyle, int nMask = -1)
	{
		ASSERT(m_pTextServices != nullptr);
		if (nMask == -1)
			nMask = nStyle;   // set everything specified
		return (int)TxSendMessage(EM_SETEDITSTYLE, nStyle, nMask);
	}

	BOOL SetFontSize(int nFontSizeDelta)
	{
		ASSERT(m_pTextServices != nullptr);
		ASSERT((nFontSizeDelta >= -1637) && (nFontSizeDelta <= 1638));
		return (BOOL)TxSendMessage(EM_SETFONTSIZE, nFontSizeDelta, 0L);
	}

	void GetScrollPos(LPPOINT lpPoint) const
	{
		ASSERT(m_pTextServices != nullptr);
		ASSERT(lpPoint != NULL);
		TxSendMessage(EM_GETSCROLLPOS, 0, (LPARAM)lpPoint);
	}

	void SetScrollPos(LPPOINT lpPoint)
	{
		ASSERT(m_pTextServices != nullptr);
		ASSERT(lpPoint != NULL);
		TxSendMessage(EM_SETSCROLLPOS, 0, (LPARAM)lpPoint);
	}

	BOOL GetZoom(int& nNum, int& nDen) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_GETZOOM, (WPARAM)&nNum, (LPARAM)&nDen);
	}

	BOOL SetZoom(int nNum, int nDen)
	{
		ASSERT(m_pTextServices != nullptr);
		ASSERT((nNum >= 0) && (nNum <= 64));
		ASSERT((nDen >= 0) && (nDen <= 64));
		return (BOOL)TxSendMessage(EM_SETZOOM, nNum, nDen);
	}

	BOOL SetZoomOff()
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETZOOM, 0, 0L);
	}

	void SetMargins(UINT nLeft, UINT nRight, WORD wFlags = EC_LEFTMARGIN | EC_RIGHTMARGIN)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SETMARGINS, wFlags, MAKELONG(nLeft, nRight));
	}

	WORD GetTypographyOptions() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (WORD)TxSendMessage(EM_GETTYPOGRAPHYOPTIONS, 0, 0L);
	}

	BOOL SetTypographyOptions(WORD wOptions, WORD wMask) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETTYPOGRAPHYOPTIONS, wOptions, wMask);
	}

	// Operations(设置最大字符格式，与SetLimitText相同)
	void LimitText(LONG nChars = 0)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_EXLIMITTEXT, 0, nChars);
	}

	int LineFromChar(LONG nIndex) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_EXLINEFROMCHAR, 0, nIndex);
	}

	POINT PosFromChar(LONG nChar) const
	{
		ASSERT(m_pTextServices != nullptr);
		POINT point = {};
		TxSendMessage(EM_POSFROMCHAR, (WPARAM)&point, nChar);
		return point;
	}

	int CharFromPos(POINT pt) const
	{
		ASSERT(m_pTextServices != nullptr);
		POINTL ptl = { pt.x, pt.y };
		return (int)TxSendMessage(EM_CHARFROMPOS, 0, (LPARAM)&ptl);
	}

	void EmptyUndoBuffer()
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_EMPTYUNDOBUFFER, 0, 0L);
	}

	int LineIndex(int nLine = -1) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_LINEINDEX, nLine, 0L);
	}

	int LineLength(int nLine = -1) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (int)TxSendMessage(EM_LINELENGTH, nLine, 0L);
	}

	BOOL LineScroll(int nLines)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_LINESCROLL, 0, nLines);
	}

	void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_REPLACESEL, (WPARAM)bCanUndo, (LPARAM)lpszNewText);
	}

	void SetRect(LPCRECT lpRect)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SETRECT, 0, (LPARAM)lpRect);
	}

	BOOL DisplayBand(LPRECT pDisplayRect)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_DISPLAYBAND, 0, (LPARAM)pDisplayRect);
	}

	LONG FindText(DWORD dwFlags, FINDTEXT& ft) const
	{
		ASSERT(m_pTextServices != nullptr);
#ifdef _UNICODE
		return (LONG)TxSendMessage(EM_FINDTEXTW, dwFlags, (LPARAM)&ft);
#else
		return (LONG)TxSendMessage(EM_FINDTEXT, dwFlags, (LPARAM)&ft);
#endif
	}

	LONG FindText(DWORD dwFlags, FINDTEXTEX& ft) const
	{
		ASSERT(m_pTextServices != nullptr);
#ifdef _UNICODE
		return (LONG)TxSendMessage(EM_FINDTEXTEXW, dwFlags, (LPARAM)&ft);
#else
		return (LONG)TxSendMessage(EM_FINDTEXTEX, dwFlags, (LPARAM)&ft);
#endif
	}

	LONG FormatRange(FORMATRANGE& fr, BOOL bDisplay = TRUE)
	{
		ASSERT(m_pTextServices != nullptr);
		return (LONG)TxSendMessage(EM_FORMATRANGE, bDisplay, (LPARAM)&fr);
	}

	LONG FormatRange(FORMATRANGE* pFormatRange, BOOL bDisplay = TRUE)
	{
		ASSERT(m_pTextServices != nullptr);
		return (LONG)TxSendMessage(EM_FORMATRANGE, bDisplay, (LPARAM)pFormatRange);
	}

	void HideSelection(BOOL bHide = TRUE, BOOL bChangeStyle = FALSE)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_HIDESELECTION, bHide, bChangeStyle);
	}

	void PasteSpecial(UINT uClipFormat, DWORD dwAspect = 0, HMETAFILE hMF = 0)
	{
		ASSERT(m_pTextServices != nullptr);
		REPASTESPECIAL reps = { dwAspect, (DWORD_PTR)hMF };
		TxSendMessage(EM_PASTESPECIAL, uClipFormat, (LPARAM)&reps);
	}

	void RequestResize()
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_REQUESTRESIZE, 0, 0L);
	}

	LONG StreamIn(UINT uFormat, EDITSTREAM& es)
	{
		ASSERT(m_pTextServices != nullptr);
		return (LONG)TxSendMessage(EM_STREAMIN, uFormat, (LPARAM)&es);
	}

	LONG StreamOut(UINT uFormat, EDITSTREAM& es)
	{
		ASSERT(m_pTextServices != nullptr);
		return (LONG)TxSendMessage(EM_STREAMOUT, uFormat, (LPARAM)&es);
	}

	DWORD FindWordBreak(int nCode, LONG nStartChar)
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_FINDWORDBREAK, nCode, nStartChar);
	}

	// Additional operations
	void ScrollCaret()
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SCROLLCARET, 0, 0L);
	}

	int InsertText(long nInsertAfterChar, LPCTSTR lpstrText, BOOL bCanUndo = FALSE)
	{
		int nRet = SetSel(nInsertAfterChar, nInsertAfterChar);
		ReplaceSel(lpstrText, bCanUndo);
		return nRet;
	}

	int AppendText(LPCTSTR lpstrText, BOOL bCanUndo = FALSE)
	{
		int nRet = SetSel(-1, -1);
		ReplaceSel(lpstrText, bCanUndo);
		return nRet;
		//return InsertText(this->GetWindowTextLength(), lpstrText, bCanUndo);
	}

	// Clipboard operations
	BOOL Undo()
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_UNDO, 0, 0L);
	}

	void Clear()
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(WM_CLEAR, 0, 0L);
	}

	void Copy()
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(WM_COPY, 0, 0L);
	}

	void Cut()
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(WM_CUT, 0, 0L);
	}

	void Paste()
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(WM_PASTE, 0, 0L);
	}

	// OLE support
	IRichEditOle* GetOleInterface() const
	{
		ASSERT(m_pTextServices != nullptr);
		IRichEditOle* pRichEditOle = NULL;
		TxSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&pRichEditOle);
		return pRichEditOle;
	}

	BOOL SetOleCallback(IRichEditOleCallback* pCallback)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pCallback);
	}

	BOOL Redo()
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_REDO, 0, 0L);
	}

	void StopGroupTyping()
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_STOPGROUPTYPING, 0, 0L);
	}

	void ShowScrollBar(int nBarType, BOOL bVisible = TRUE)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SHOWSCROLLBAR, nBarType, bVisible);
	}

	BOOL SetTabStops(int nTabStops, LPINT rgTabStops)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETTABSTOPS, nTabStops, (LPARAM)rgTabStops);
	}

	BOOL SetTabStops()
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETTABSTOPS, 0, 0L);
	}

	BOOL SetTabStops(const int& cxEachStop)    // takes an 'int'
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETTABSTOPS, 1, (LPARAM)(LPINT)&cxEachStop);
	}

#if (_RICHEDIT_VER >= 0x0800)
	AutoCorrectProc GetAutoCorrectProc() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (AutoCorrectProc)TxSendMessage(EM_GETAUTOCORRECTPROC, 0, 0L);
	}

	BOOL SetAutoCorrectProc(AutoCorrectProc pfn)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETAUTOCORRECTPROC, (WPARAM)pfn, 0L);
	}

	BOOL CallAutoCorrectProc(WCHAR ch)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_CALLAUTOCORRECTPROC, (WPARAM)ch, 0L);
	}

	DWORD GetEditStyleEx() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_GETEDITSTYLEEX, 0, 0L);
	}

	DWORD SetEditStyleEx(DWORD dwStyleEx, DWORD dwMask)
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_SETEDITSTYLEEX, dwStyleEx, dwMask);
	}

	DWORD GetStoryType(int nStoryIndex) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_GETSTORYTYPE, nStoryIndex, 0L);
	}

	DWORD SetStoryType(int nStoryIndex, DWORD dwStoryType)
	{
		ASSERT(m_pTextServices != nullptr);
		return (DWORD)TxSendMessage(EM_SETSTORYTYPE, nStoryIndex, dwStoryType);
	}

	DWORD GetEllipsisMode() const
	{
		ASSERT(m_pTextServices != nullptr);

		DWORD dwMode = 0;
		BOOL bRet = (BOOL)TxSendMessage(EM_GETELLIPSISMODE, 0, (LPARAM)&dwMode);
		(void)bRet;   // avoid level 4 warning
		ASSERT(bRet != FALSE);

		return dwMode;
	}

	BOOL SetEllipsisMode(DWORD dwEllipsisMode)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETELLIPSISMODE, 0, dwEllipsisMode);
	}

	BOOL GetEllipsisState() const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_GETELLIPSISSTATE, 0, 0L);
	}

	BOOL GetTouchOptions(int nTouchOptions) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_GETTOUCHOPTIONS, nTouchOptions, 0L);
	}

	void SetTouchOptions(int nTouchOptions, BOOL bEnable)
	{
		ASSERT(m_pTextServices != nullptr);
		TxSendMessage(EM_SETTOUCHOPTIONS, nTouchOptions, bEnable);
	}

	HRESULT InsertTable(TABLEROWPARMS* pRowParams, TABLECELLPARMS* pCellParams)
	{
		ASSERT(m_pTextServices != nullptr);
		return (HRESULT)TxSendMessage(EM_INSERTTABLE, (WPARAM)pRowParams, (LPARAM)pCellParams);
	}

	HRESULT GetTableParams(TABLEROWPARMS* pRowParams, TABLECELLPARMS* pCellParams) const
	{
		ASSERT(m_pTextServices != nullptr);
		return (HRESULT)TxSendMessage(EM_GETTABLEPARMS, (WPARAM)pRowParams, (LPARAM)pCellParams);
	}

	HRESULT SetTableParams(TABLEROWPARMS* pRowParams, TABLECELLPARMS* pCellParams)
	{
		ASSERT(m_pTextServices != nullptr);
		return (HRESULT)TxSendMessage(EM_SETTABLEPARMS, (WPARAM)pRowParams, (LPARAM)pCellParams);
	}

	HRESULT InsertImage(RICHEDIT_IMAGE_PARAMETERS* pParams)
	{
		ASSERT(m_pTextServices != nullptr);
		return (HRESULT)TxSendMessage(EM_INSERTIMAGE, 0, (LPARAM)pParams);
	}

	BOOL SetUiaName(LPCTSTR lpstrName)
	{
		ASSERT(m_pTextServices != nullptr);
		return (BOOL)TxSendMessage(EM_SETUIANAME, 0, (LPARAM)lpstrName);
	}
#endif // (_RICHEDIT_VER >= 0x0800)

private:
	/** TextServices 接口
	*/
	ITextServices* m_pTextServices;
};

}//namespace ui

#endif //UI_CONTROL_RICHEDIT_CTRL_H_