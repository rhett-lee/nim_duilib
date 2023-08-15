#ifndef UI_CONTROL_RICHEDIT_HOST_H_
#define UI_CONTROL_RICHEDIT_HOST_H_

#pragma once

#include "duilib/Core/UiTypes.h"
//使用Windows的ITextHost实现
#include <Richedit.h>
#include <TextServ.h>
#include <RichOle.h>

namespace ui
{

class RichEdit;
class UILIB_API RichEditHost : public ITextHost
{
public:
	RichEditHost();
	RichEditHost(const RichEditHost& r) = delete;
	RichEditHost& operator=(const RichEditHost& r) = delete;
	virtual ~RichEditHost();

public:

    BOOL Init(RichEdit* re, const CREATESTRUCT* pcs);
    ITextServices* GetTextServices(void) { return m_pTextServices; }
    void SetClientRect(UiRect* prc);
    UiRect* GetClientRect() { return &m_rcClient; }
    BOOL GetWordWrap(void) { return m_fWordWrap; }
    void SetWordWrap(BOOL bWordWrap);
    BOOL GetReadOnly();
    void SetReadOnly(BOOL fReadOnly);
    BOOL IsPassword();
    void SetPassword(BOOL bPassword);
    void SetFontId(const std::wstring& fontId);
    void SetColor(DWORD dwColor);
    SIZEL* GetExtent();
    void SetExtent(SIZEL* psizelExtent);
    void LimitText(LONG nChars);
    BOOL IsCaptured();

    BOOL GetAllowBeep();
    void SetAllowBeep(BOOL bAllowBeep);
    WORD GetDefaultAlign();
    void SetDefaultAlign(WORD wNewAlign);
    BOOL GetRichTextFlag();
    void SetRichTextFlag(BOOL fNew);
    LONG GetDefaultLeftIndent();
    void SetDefaultLeftIndent(LONG lNewIndent);
    BOOL SetSaveSelection(BOOL fSaveSelection);
    HRESULT OnTxInPlaceDeactivate();
    HRESULT OnTxInPlaceActivate(LPCRECT prcClient);
    BOOL GetActiveState(void) { return m_fInplaceActive; }
    BOOL DoSetCursor(const UiRect* prc, const UiPoint* pt);
    void SetTransparent(BOOL fTransparent);
    void GetControlRect(UiRect* prc);
    LONG SetAccelPos(LONG lAcceleratorPos);
    WCHAR SetPasswordChar(WCHAR chPasswordChar);
    void SetDisabled(BOOL fOn);
    LONG SetSelBarWidth(LONG lSelBarWidth);
    BOOL GetTimerState();

    void SetCharFormat(const CHARFORMAT2W& c);
    void SetParaFormat(const PARAFORMAT2& p);

	ITextHost* GetTextHost();
    ITextServices* GetTextServices2();
    BOOL SetOleCallback(IRichEditOleCallback* pCallback);
    BOOL CanPaste(UINT nFormat = 0);
    void PasteSpecial(UINT uClipFormat, DWORD dwAspect = 0, HMETAFILE hMF = 0);
    UiRect MakeUiRect(const RECT& rc);

public:

    static HRESULT CreateHost(RichEdit* re, const CREATESTRUCT* pcs, RichEditHost** pptec);
    static HRESULT InitDefaultCharFormat(RichEdit* re, CHARFORMAT2W* pcf);
    static HRESULT InitDefaultParaFormat(RichEdit* re, PARAFORMAT2* ppf);
    static void GetLogFont(const std::wstring& fontId, LOGFONT& lf);
    // Convert Pixels on the X axis to Himetric
    static LONG DXtoHimetricX(LONG dx, LONG xPerInch);
    // Convert Pixels on the Y axis to Himetric
    static LONG DYtoHimetricY(LONG dy, LONG yPerInch);

    /** 获取默认的最大字符个数
    */
    static int32_t GetDefaultMaxText();

public:
    // -----------------------------
    //	IUnknown interface
    // -----------------------------
    virtual HRESULT _stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    virtual ULONG _stdcall AddRef(void) override;
    virtual ULONG _stdcall Release(void) override;

    // -----------------------------
    //	ITextHost interface
    // -----------------------------
    virtual HDC TxGetDC() override;
    virtual INT TxReleaseDC(HDC hdc) override;
    virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow) override;
    virtual BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags) override;
    virtual BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw) override;
    virtual BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw) override;
    virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode) override;
    virtual void TxViewChange(BOOL fUpdate) override;
    virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) override;
    virtual BOOL TxShowCaret(BOOL fShow) override;
    virtual BOOL TxSetCaretPos(INT x, INT y) override;
    virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout) override;
    virtual void TxKillTimer(UINT idTimer) override;
    virtual void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll) override;
    virtual void TxSetCapture(BOOL fCapture) override;
    virtual void TxSetFocus() override;
    virtual void TxSetCursor(HCURSOR hcur, BOOL fText) override;
    virtual BOOL TxScreenToClient(LPPOINT lppt) override;
    virtual BOOL TxClientToScreen(LPPOINT lppt) override;
    virtual HRESULT TxActivate(LONG* plOldState) override;
    virtual HRESULT TxDeactivate(LONG lNewState) override;
    virtual HRESULT TxGetClientRect(LPRECT prc) override;
    virtual HRESULT TxGetViewInset(LPRECT prc) override;
    virtual HRESULT TxGetCharFormat(const CHARFORMATW** ppCF) override;
    virtual HRESULT TxGetParaFormat(const PARAFORMAT** ppPF) override;
    virtual COLORREF TxGetSysColor(int nIndex) override;
    virtual HRESULT TxGetBackStyle(TXTBACKSTYLE* pstyle) override;
    virtual HRESULT TxGetMaxLength(DWORD* plength) override;
    virtual HRESULT TxGetScrollBars(DWORD* pdwScrollBar) override;
    virtual HRESULT TxGetPasswordChar(TCHAR* pch) override;
    virtual HRESULT TxGetAcceleratorPos(LONG* pcp) override;
    virtual HRESULT TxGetExtent(LPSIZEL lpExtent) override;
    virtual HRESULT OnTxCharFormatChange(const CHARFORMATW* pcf) override;
    virtual HRESULT OnTxParaFormatChange(const PARAFORMAT* ppf) override;
    virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits) override;
    virtual HRESULT TxNotify(DWORD iNotify, void* pv) override;
    virtual HIMC TxImmGetContext(void) override;
    virtual void TxImmReleaseContext(HIMC himc) override;
    virtual HRESULT TxGetSelectionBarWidth(LONG* plSelBarWidth) override;

private:
    RichEdit* m_pRichEdit;
    ULONG m_cRefs;					// Reference Count
    ITextServices* m_pTextServices;		    // pointer to Text Services object

    DWORD m_dwStyle;				// style bits

    unsigned	m_fEnableAutoWordSel : 1;	// enable Word style auto word selection?
    unsigned	m_fWordWrap : 1;	// Whether control should word wrap
    unsigned	m_fAllowBeep : 1;	// Whether beep is allowed
    unsigned	m_fRich : 1;	// Whether control is rich text
    unsigned	m_fSaveSelection : 1;	// Whether to save the selection when inactive
    unsigned	m_fInplaceActive : 1; // Whether control is inplace active
    unsigned	m_fTransparent : 1; // Whether control is transparent
    unsigned	m_fTimer : 1;	// A timer is set
    unsigned    m_fCaptured : 1;

    LONG		m_lSelBarWidth;			// Width of the selection bar
    LONG  		m_cchTextMost;			// maximum text size
    DWORD		m_dwEventMask;			// HandleMessage mask to pass on to parent window
    LONG		m_icf;
    LONG		m_ipf;
    UiRect		m_rcClient;				// Client Rect for this control
    SIZEL		m_sizelExtent;			// Extent array
    CHARFORMAT2W m_cf;					// Default character format
    PARAFORMAT2	m_pf;					    // Default paragraph format
    LONG		m_lAcceleratorPos;				// Accelerator position
    WCHAR		m_chPasswordChar;		    // Password character
};


}//name space ui

#endif // UI_CONTROL_RICHEDIT_HOST_H_
