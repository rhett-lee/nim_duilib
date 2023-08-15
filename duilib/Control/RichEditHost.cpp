#include "RichEditHost.h"
#include "RichEdit.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"

namespace ui
{

/** 文本最大字符个??
*/
// These constants are for backward compatibility. They are the 
// sizes used for initialization and reset in RichEdit 1.0
const LONG cInitTextMax = (32 * 1024) - 1;

#ifndef LY_PER_INCH
	#define LY_PER_INCH 1440
#endif

#ifndef HIMETRIC_PER_INCH
	#define HIMETRIC_PER_INCH 2540
#endif

EXTERN_C const IID IID_ITextServices = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
	0x8d33f740,
	0xcf58,
	0x11ce,
	{0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
};

EXTERN_C const IID IID_ITextHost = { /* c5bdd8d0-d26e-11ce-a89e-00aa006cadc5 */
	0xc5bdd8d0,
	0xd26e,
	0x11ce,
	{0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
};

RichEditHost::RichEditHost() :
	m_pRichEdit(nullptr),
	m_cRefs(0),
	m_pTextServices(nullptr),
	m_dwStyle(0),
	m_fEnableAutoWordSel(0),
	m_fWordWrap(0),
	m_fAllowBeep(0),
	m_fRich(0),
	m_fSaveSelection(0),
	m_fInplaceActive(0),
	m_fTransparent(0),
	m_fTimer(0),
	m_fCaptured(0),
	m_lSelBarWidth(0),
	m_cchTextMost(cInitTextMax),
	m_dwEventMask(0),
	m_icf(0),
	m_ipf(0),
	m_rcClient(),
	m_sizelExtent({ 0 }),
	m_cf({ 0 }),
	m_pf({ 0 }),
	m_lAcceleratorPos(-1),
	m_chPasswordChar(0)
{
}

RichEditHost::~RichEditHost()
{
	if (m_pTextServices) {
		m_pTextServices->OnTxInPlaceDeactivate();
		m_pTextServices->Release();
	}
}

HRESULT RichEditHost::CreateHost(RichEdit* re, const CREATESTRUCT* pcs, RichEditHost** pptec)
{
	HRESULT hr = E_FAIL;
	//GdiSetBatchLimit(1);

	RichEditHost* phost = new RichEditHost();
	if (phost)
	{
		if (phost->Init(re, pcs))
		{
			if (pptec) {
				*pptec = phost;
				hr = S_OK;
			}
		}
	}

	if (FAILED(hr))
	{
		delete phost;
	}

	return TRUE;
}

ITextHost* RichEditHost::GetTextHost()
{
	AddRef();
	return this;
}

ITextServices* RichEditHost::GetTextServices2()
{
	if (NULL == m_pTextServices)
		return NULL;
	m_pTextServices->AddRef();
	return m_pTextServices;
}

BOOL RichEditHost::SetOleCallback(IRichEditOleCallback* pCallback)
{
	if (NULL == m_pTextServices)
		return FALSE;
	LRESULT lRes = 0;
	m_pTextServices->TxSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pCallback, &lRes);
	return (BOOL)lRes;
}

BOOL RichEditHost::CanPaste(UINT nFormat)
{
	if (NULL == m_pTextServices)
		return FALSE;
	LRESULT lRes = 0;
	m_pTextServices->TxSendMessage(EM_CANPASTE, nFormat, 0L, &lRes);
	return (BOOL)lRes;
}

void RichEditHost::PasteSpecial(UINT uClipFormat, DWORD dwAspect, HMETAFILE hMF)
{
	if (NULL == m_pTextServices)
		return;
	REPASTESPECIAL reps = { dwAspect, (DWORD_PTR)hMF };
	m_pTextServices->TxSendMessage(EM_PASTESPECIAL, uClipFormat, (LPARAM)&reps, NULL);
}

UiRect RichEditHost::MakeUiRect(const RECT& rc)
{
	return UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

////////////////////// Create/Init/Destruct Commands ///////////////////////
BOOL RichEditHost::Init(RichEdit* re, const CREATESTRUCT* pcs)
{
	if (re == nullptr)
	{
		return FALSE;
	}
	IUnknown* pUnk = nullptr;
	HRESULT hr = E_FAIL;
	std::wstring edit_dll(L"msftedit.dll");
	m_pRichEdit = re;
	// Initialize Reference count
	m_cRefs = 1;

	// Create and cache CHARFORMAT for this control
	if (FAILED(InitDefaultCharFormat(re, &m_cf)))
	{
		return FALSE;
	}

	// Create and cache PARAFORMAT for this control
	if (FAILED(InitDefaultParaFormat(re, &m_pf)))
	{
		return FALSE;
	}

	// edit controls created without a window are multiline by default
	// so that paragraph formats can be
	m_dwStyle = ES_MULTILINE;

	// edit controls are rich by default
	m_fRich = re->IsRich();

	m_cchTextMost = re->GetLimitText();

	if (pcs)
	{
		m_dwStyle = pcs->style;

		if (!(m_dwStyle & (ES_AUTOHSCROLL | WS_HSCROLL)))
		{
			m_fWordWrap = TRUE;
		}
	}

	if (!(m_dwStyle & ES_LEFT))
	{
		if (m_dwStyle & ES_CENTER)
			m_pf.wAlignment = PFA_CENTER;
		else if (m_dwStyle & ES_RIGHT)
			m_pf.wAlignment = PFA_RIGHT;
	}

	m_fInplaceActive = TRUE;

	// Create Text Services component
	//if(FAILED(CreateTextServices(NULL, this, &pUnk)))
	//    goto err;

	PCreateTextServices TextServicesProc = nullptr;
	//解决32位系统下跨窗口间拖拽文字在win7及win7以下系统上会出现重复的问题（64位暂不修复） lty 20170714
#if defined(_M_X64) || defined(__x86_64__)
	edit_dll = L"msftedit.dll";
#else
	edit_dll = nbase::win32::GetCurrentModuleDirectory() + L"msftedit50.dll";
	if (!(::GetFileAttributesW(edit_dll.c_str()) != INVALID_FILE_ATTRIBUTES))
		edit_dll = L"msftedit.dll";
#endif
	HMODULE hmod = LoadLibraryW(edit_dll.c_str()); //msftedit.dll
	if (hmod)
	{
		TextServicesProc = (PCreateTextServices)GetProcAddress(hmod, "CreateTextServices");
	}

	if (TextServicesProc)
	{
		TextServicesProc(NULL, this, &pUnk);
	}

	if (pUnk != nullptr)
	{
		hr = pUnk->QueryInterface(IID_ITextServices, (void**)&m_pTextServices);

		// Whether the previous call succeeded or failed we are done
		// with the private interface.
		pUnk->Release();

		if (FAILED(hr))
		{
			return FALSE;
		}
	}


	// Set window text
	if (m_pTextServices && pcs && pcs->lpszName)
	{
		if (SUCCEEDED(m_pTextServices->TxSetText((TCHAR*)pcs->lpszName)))
		{
			return TRUE;
		}
	}
	return FALSE;
}

HRESULT RichEditHost::InitDefaultParaFormat(RichEdit* /*re*/, PARAFORMAT2* ppf)
{
	memset(ppf, 0, sizeof(PARAFORMAT2));
	ppf->cbSize = sizeof(PARAFORMAT2);
	ppf->dwMask = PFM_ALL;
	ppf->wAlignment = PFA_LEFT;
	ppf->cTabCount = 1;
	ppf->rgxTabs[0] = lDefaultTab;

	return S_OK;
}

HRESULT RichEditHost::InitDefaultCharFormat(RichEdit* re, CHARFORMAT2W* pcf)
{
	ASSERT(re != nullptr);
	ASSERT(pcf != nullptr);
	if ((re == nullptr) || (pcf == nullptr)) {
		return S_OK;
	}
	memset(pcf, 0, sizeof(CHARFORMAT2W));
	LOGFONT lf = { 0 };
	GetLogFont(re->GetFontId(), lf);
	UiColor dwColor = re->GetTextColorValue();
	pcf->cbSize = sizeof(CHARFORMAT2W);
	pcf->crTextColor = dwColor.ToCOLORREF();
	LONG yPixPerInch = GetDeviceCaps(re->GetWindowDC(), LOGPIXELSY);
	if (yPixPerInch == 0) {
		yPixPerInch = 96;
	}
	pcf->yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
	pcf->yOffset = 0;
	pcf->dwEffects = 0;
	pcf->dwMask = CFM_SIZE | CFM_OFFSET | CFM_FACE | CFM_CHARSET | CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
	if (lf.lfWeight >= FW_BOLD) {
		pcf->dwEffects |= CFE_BOLD;
	}
	if (lf.lfItalic) {
		pcf->dwEffects |= CFE_ITALIC;
	}
	if (lf.lfUnderline) {
		pcf->dwEffects |= CFE_UNDERLINE;
	}
	if (lf.lfStrikeOut) {
		pcf->dwEffects |= CFE_STRIKEOUT;
	}
	pcf->bCharSet = lf.lfCharSet;
	pcf->bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
	wcscpy_s(pcf->szFaceName, lf.lfFaceName);
#else
	//need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
	MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, pcf->szFaceName, LF_FACESIZE);
#endif

	return S_OK;
}

void RichEditHost::GetLogFont(const std::wstring& fontId, LOGFONT& lf)
{
	//优先获取默认字体
	lf = { 0 };
	::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	IFont* pFont = GlobalManager::Instance().Font().GetIFont(fontId);
	ASSERT(pFont != nullptr);
	if (pFont == nullptr) {
		return;
	}
	wcscpy_s(lf.lfFaceName, pFont->FontName());
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = -pFont->FontSize();
	if (pFont->IsUnderline()) {
		lf.lfUnderline = TRUE;
	}
	if (pFont->IsStrikeOut()) {
		lf.lfStrikeOut = TRUE;
	}
	if (pFont->IsItalic()) {
		lf.lfItalic = TRUE;
	}
	if (pFont->IsBold()) {
		lf.lfWeight = FW_BOLD;
	}
}

/////////////////////////////////  IUnknown ////////////////////////////////
HRESULT RichEditHost::QueryInterface(REFIID riid, void** ppvObject)
{
	if (ppvObject == nullptr) {
		return E_INVALIDARG;
	}
	HRESULT hr = E_NOINTERFACE;
	*ppvObject = NULL;

	if (IsEqualIID(riid, IID_IUnknown)
		|| IsEqualIID(riid, IID_ITextHost))
	{
		AddRef();
		*ppvObject = (ITextHost*)this;
		hr = S_OK;
	}

	return hr;
}

ULONG RichEditHost::AddRef(void)
{
	return ++m_cRefs;
}

ULONG RichEditHost::Release(void)
{
	ULONG c_Refs = --m_cRefs;

	if (c_Refs == 0)
	{
		delete this;
	}

	return c_Refs;
}

/////////////////////////////////  Far East Support  //////////////////////////////////////

HIMC RichEditHost::TxImmGetContext(void)
{
	return NULL;
}

void RichEditHost::TxImmReleaseContext(HIMC /*himc*/)
{
	//::ImmReleaseContext( hwnd, himc );
}

//////////////////////////// ITextHost Interface  ////////////////////////////

HDC RichEditHost::TxGetDC()
{
	return m_pRichEdit->GetWindowDC();
}

int RichEditHost::TxReleaseDC(HDC /*hdc*/)
{
	return 1;
}

BOOL RichEditHost::TxShowScrollBar(INT /*fnBar*/, BOOL /*fShow*/)
{
	ASSERT(FALSE); //暂时注释掉，不知道这代码有啥用   by panqinke 2014.5.6
	//ScrollBar* pVerticalScrollBar = m_pRichEdit->GetVScrollBar();
	//ScrollBar* pHorizontalScrollBar = m_pRichEdit->GetHScrollBar();
	//if( fnBar == SB_VERT && pVerticalScrollBar ) {
	//    pVerticalScrollBar->SetFadeVisible(fShow == TRUE);
	//}
	//else if( fnBar == SB_HORZ && pHorizontalScrollBar ) {
	//    pHorizontalScrollBar->SetFadeVisible(fShow == TRUE);
	//}
	//else if( fnBar == SB_BOTH ) {
	//    if( pVerticalScrollBar ) pVerticalScrollBar->SetFadeVisible(fShow == TRUE);
	//    if( pHorizontalScrollBar ) pHorizontalScrollBar->SetFadeVisible(fShow == TRUE);
	//}
	return TRUE;
}

BOOL RichEditHost::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
	if (fuSBFlags == SB_VERT) {
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			m_pRichEdit->GetVScrollBar()->SetScrollRange(0);
		}
	}
	else if (fuSBFlags == SB_HORZ) {
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			m_pRichEdit->GetHScrollBar()->SetScrollRange(0);
		}
	}
	else if (fuSBFlags == SB_BOTH) {
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			m_pRichEdit->GetVScrollBar()->SetScrollRange(0);
		}
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			m_pRichEdit->GetHScrollBar()->SetScrollRange(0);
		}
	}

	m_pRichEdit->SetPos(m_pRichEdit->GetPos());

	return TRUE;
}

BOOL RichEditHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL /*fRedraw*/)
{
	ScrollBar* pVerticalScrollBar = m_pRichEdit->GetVScrollBar();
	ScrollBar* pHorizontalScrollBar = m_pRichEdit->GetHScrollBar();
	bool bArrange = false;
	if (fnBar == SB_VERT && pVerticalScrollBar) {
		if (nMaxPos - nMinPos - m_rcClient.bottom + m_rcClient.top <= 0) {
			pVerticalScrollBar->SetScrollRange(0);
		}
		else {
			if (!pVerticalScrollBar->IsValid()) {
				bArrange = true;
			}
			pVerticalScrollBar->SetScrollRange(nMaxPos - nMinPos - m_rcClient.bottom + m_rcClient.top);
		}
	}
	else if (fnBar == SB_HORZ && pHorizontalScrollBar) {
		if (nMaxPos - nMinPos - m_rcClient.right + m_rcClient.left <= 0) {
			pHorizontalScrollBar->SetScrollRange(0);
		}
		else {
			if (!pHorizontalScrollBar->IsValid()) {
				bArrange = true;
			}
			pHorizontalScrollBar->SetScrollRange(nMaxPos - nMinPos - m_rcClient.right + m_rcClient.left);
		}
	}

	if (bArrange) {
		m_pRichEdit->SetPos(m_pRichEdit->GetPos());
	}

	return TRUE;
}

BOOL RichEditHost::TxSetScrollPos(INT fnBar, INT nPos, BOOL /*fRedraw*/)
{
	ScrollBar* pVerticalScrollBar = m_pRichEdit->GetVScrollBar();
	ScrollBar* pHorizontalScrollBar = m_pRichEdit->GetHScrollBar();
	if (fnBar == SB_VERT && pVerticalScrollBar) {
		pVerticalScrollBar->SetScrollPos(nPos);
	}
	else if (fnBar == SB_HORZ && pHorizontalScrollBar) {
		pHorizontalScrollBar->SetScrollPos(nPos);
	}
	return TRUE;
}

void RichEditHost::TxInvalidateRect(LPCRECT prc, BOOL /*fMode*/)
{
	if (!m_pRichEdit->GetWindow())
		return;

	UiPoint scrollOffset = m_pRichEdit->GetScrollOffsetInScrollBox();
	if (prc == NULL) {
		UiRect newRcClient = m_rcClient;
		newRcClient.Offset(-scrollOffset.x, -scrollOffset.y);
		m_pRichEdit->GetWindow()->Invalidate(newRcClient);
		return;
	}
	UiRect rc = MakeUiRect(*prc);
	rc.Offset(-scrollOffset.x, -scrollOffset.y);
	m_pRichEdit->GetWindow()->Invalidate(rc);
}

void RichEditHost::TxViewChange(BOOL /*fUpdate*/)
{

}

BOOL RichEditHost::TxCreateCaret(HBITMAP /*hbmp*/, INT xWidth, INT yHeight)
{
	return m_pRichEdit->CreateCaret(xWidth, yHeight);
}

BOOL RichEditHost::TxShowCaret(BOOL /*fShow*/)
{
	return true; // m_pRichEdit->ShowCaret(fShow);
}

BOOL RichEditHost::TxSetCaretPos(INT x, INT y)
{
	return m_pRichEdit->SetCaretPos(x, y);
}

BOOL RichEditHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
	m_fTimer = TRUE;
	m_pRichEdit->SetTimer(idTimer, uTimeout);
	return TRUE;
}

void RichEditHost::TxKillTimer(UINT idTimer)
{
	m_pRichEdit->KillTimer(idTimer);
	m_fTimer = FALSE;
}

void RichEditHost::TxScrollWindowEx(INT /*dx*/, INT /*dy*/, LPCRECT /*lprcScroll*/, LPCRECT /*lprcClip*/, HRGN /*hrgnUpdate*/, LPRECT /*lprcUpdate*/, UINT /*fuScroll*/)
{
	return;
}

void RichEditHost::TxSetCapture(BOOL fCapture)
{
	if (!m_pRichEdit->GetWindow())
		return;

	if (fCapture) m_pRichEdit->GetWindow()->SetCapture();
	else m_pRichEdit->GetWindow()->ReleaseCapture();
	m_fCaptured = fCapture;
}

void RichEditHost::TxSetFocus()
{
	m_pRichEdit->SetFocus();
}

void RichEditHost::TxSetCursor(HCURSOR hcur, BOOL /*fText*/)
{
	::SetCursor(hcur);
}

BOOL RichEditHost::TxScreenToClient(LPPOINT lppt)
{
	return ::ScreenToClient(m_pRichEdit->GetWindowHandle(), lppt);
}

BOOL RichEditHost::TxClientToScreen(LPPOINT lppt)
{
	return ::ClientToScreen(m_pRichEdit->GetWindowHandle(), lppt);
}

HRESULT RichEditHost::TxActivate(LONG*/*plOldState*/)
{
	return S_OK;
}

HRESULT RichEditHost::TxDeactivate(LONG /*lNewState*/)
{
	return S_OK;
}

HRESULT RichEditHost::TxGetClientRect(LPRECT prc)
{
	ASSERT(prc != nullptr);
	if (prc) {
		UiRect rcTemp = m_rcClient;
		GetControlRect(&rcTemp);
		*prc = { rcTemp.left, rcTemp.top, rcTemp.right, rcTemp.bottom };
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetViewInset(LPRECT prc)
{
	ASSERT(prc != nullptr);
	if (prc) {
		prc->left = prc->right = prc->top = prc->bottom = 0;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetCharFormat(const CHARFORMATW** ppCF)
{
	ASSERT(ppCF != nullptr);
	if (ppCF) {
		*ppCF = &m_cf;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetParaFormat(const PARAFORMAT** ppPF)
{
	ASSERT(ppPF != nullptr);
	if (ppPF) {
		*ppPF = &m_pf;
	}
	return NOERROR;
}

COLORREF RichEditHost::TxGetSysColor(int nIndex)
{
	return ::GetSysColor(nIndex);
}

HRESULT RichEditHost::TxGetBackStyle(TXTBACKSTYLE* pstyle)
{
	ASSERT(pstyle != nullptr);
	if (pstyle) {
		*pstyle = !m_fTransparent ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetMaxLength(DWORD* pLength)
{
	ASSERT(pLength != nullptr);
	if (pLength) {
		*pLength = m_cchTextMost;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetScrollBars(DWORD* pdwScrollBar)
{
	ASSERT(pdwScrollBar != nullptr);
	if (pdwScrollBar) {
		*pdwScrollBar = m_dwStyle & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL |
			ES_AUTOHSCROLL | ES_DISABLENOSCROLL);
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetPasswordChar(TCHAR* pch)
{
	ASSERT(pch != nullptr);
	if (pch == nullptr) {
		return NOERROR;
	}
#ifdef _UNICODE
	* pch = m_chPasswordChar;
#else
	::WideCharToMultiByte(CP_ACP, 0, &m_chPasswordChar, 1, pch, 1, NULL, NULL);
#endif
	return NOERROR;
}

HRESULT RichEditHost::TxGetAcceleratorPos(LONG* pcp)
{
	ASSERT(pcp != nullptr);
	if (pcp != nullptr) {
		*pcp = m_lAcceleratorPos;
	}
	return S_OK;
}

HRESULT RichEditHost::OnTxCharFormatChange(const CHARFORMATW*/*pcf*/)
{
	return S_OK;
}

HRESULT RichEditHost::OnTxParaFormatChange(const PARAFORMAT*/*ppf*/)
{
	return S_OK;
}

HRESULT RichEditHost::TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits)
{
	ASSERT(pdwBits != nullptr);
	if (pdwBits == nullptr) {
		return S_OK;
	}
	DWORD dwProperties = 0;

	if (m_fRich)
	{
		dwProperties = TXTBIT_RICHTEXT;
	}

	if (m_dwStyle & ES_MULTILINE)
	{
		dwProperties |= TXTBIT_MULTILINE;
	}

	if (m_dwStyle & ES_READONLY)
	{
		dwProperties |= TXTBIT_READONLY;
	}

	if (m_dwStyle & ES_PASSWORD)
	{
		dwProperties |= TXTBIT_USEPASSWORD;
	}

	if (!(m_dwStyle & ES_NOHIDESEL))
	{
		dwProperties |= TXTBIT_HIDESELECTION;
	}

	if (m_fEnableAutoWordSel)
	{
		dwProperties |= TXTBIT_AUTOWORDSEL;
	}

	if (m_fWordWrap)
	{
		dwProperties |= TXTBIT_WORDWRAP;
	}

	if (m_fAllowBeep)
	{
		dwProperties |= TXTBIT_ALLOWBEEP;
	}

	if (m_fSaveSelection)
	{
		dwProperties |= TXTBIT_SAVESELECTION;
	}

	*pdwBits = dwProperties & dwMask;
	return NOERROR;
}


HRESULT RichEditHost::TxNotify(DWORD iNotify, void* pv)
{
	if (iNotify == EN_REQUESTRESIZE) {
		UiRect rc;
		REQRESIZE* preqsz = (REQRESIZE*)pv;
		GetControlRect(&rc);
		rc.bottom = rc.top + preqsz->rc.bottom;
		rc.right = rc.left + preqsz->rc.right;
		SetClientRect(&rc);
	}
	m_pRichEdit->OnTxNotify(iNotify, pv);
	return S_OK;
}

HRESULT RichEditHost::TxGetExtent(LPSIZEL lpExtent)
{
	ASSERT(lpExtent != nullptr);
	if (lpExtent) {
		*lpExtent = m_sizelExtent;
	}
	return S_OK;
}

HRESULT	RichEditHost::TxGetSelectionBarWidth(LONG* plSelBarWidth)
{
	ASSERT(plSelBarWidth != nullptr);
	if (plSelBarWidth) {
		*plSelBarWidth = m_lSelBarWidth;
	}
	return S_OK;
}

void RichEditHost::SetWordWrap(BOOL bWordWrap)
{
	m_fWordWrap = bWordWrap;
	if (m_pTextServices) {
		m_pTextServices->OnTxPropertyBitsChange(TXTBIT_WORDWRAP, bWordWrap ? TXTBIT_WORDWRAP : 0);
	}
}

BOOL RichEditHost::GetReadOnly()
{
	return (m_dwStyle & ES_READONLY) != 0;
}

void RichEditHost::SetReadOnly(BOOL fReadOnly)
{
	if (fReadOnly)
	{
		m_dwStyle |= ES_READONLY;
	}
	else
	{
		m_dwStyle &= ~ES_READONLY;
	}

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_READONLY,
		fReadOnly ? TXTBIT_READONLY : 0);
}

BOOL RichEditHost::IsPassword()
{
	return (m_dwStyle & ES_PASSWORD) != 0;
}

void RichEditHost::SetPassword(BOOL bPassword)
{
	if (bPassword)
	{
		m_dwStyle |= ES_PASSWORD;
	}
	else
	{
		m_dwStyle &= ~ES_PASSWORD;
	}

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD,
		bPassword ? TXTBIT_USEPASSWORD : 0);
}

void RichEditHost::SetFontId(const std::wstring& fontId)
{
	ASSERT(!fontId.empty());
	if (fontId.empty()) {
		return;
	}
	LOGFONT lf = { 0, };
	GetLogFont(fontId, lf);
	LONG yPixPerInch = ::GetDeviceCaps(m_pRichEdit->GetWindowDC(), LOGPIXELSY);
	if (yPixPerInch == 0)
		yPixPerInch = 96;
	m_cf.yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
	if (lf.lfWeight >= FW_BOLD)
		m_cf.dwEffects |= CFE_BOLD;
	if (lf.lfItalic)
		m_cf.dwEffects |= CFE_ITALIC;
	if (lf.lfUnderline)
		m_cf.dwEffects |= CFE_UNDERLINE;
	if (lf.lfStrikeOut)
		m_cf.dwEffects |= CFE_STRIKEOUT;
	m_cf.bCharSet = lf.lfCharSet;
	m_cf.bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
	wcscpy_s(m_cf.szFaceName, lf.lfFaceName);
#else
	//need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
	MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, m_cf.szFaceName, LF_FACESIZE);
#endif

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE,
		TXTBIT_CHARFORMATCHANGE);
}

void RichEditHost::SetColor(DWORD dwColor)
{
	m_cf.crTextColor = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE,
		TXTBIT_CHARFORMATCHANGE);
}

SIZEL* RichEditHost::GetExtent()
{
	return &m_sizelExtent;
}

void RichEditHost::SetExtent(SIZEL* psizelExtent)
{
	if (psizelExtent) {
		m_sizelExtent = *psizelExtent;
		if (m_pTextServices) {
			m_pTextServices->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
		}
	}
}

void RichEditHost::LimitText(LONG nChars)
{
	m_cchTextMost = nChars;
	if (m_cchTextMost <= 0) m_cchTextMost = cInitTextMax;
	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_MAXLENGTHCHANGE, TXTBIT_MAXLENGTHCHANGE);
}

BOOL RichEditHost::IsCaptured()
{
	return m_fCaptured;
}

BOOL RichEditHost::GetAllowBeep()
{
	return m_fAllowBeep;
}

void RichEditHost::SetAllowBeep(BOOL bAllowBeep)
{
	m_fAllowBeep = bAllowBeep;

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP,
		bAllowBeep ? TXTBIT_ALLOWBEEP : 0);
}

WORD RichEditHost::GetDefaultAlign()
{
	return m_pf.wAlignment;
}

void RichEditHost::SetDefaultAlign(WORD wNewAlign)
{
	m_pf.wAlignment = wNewAlign;

	// Notify control of property change
	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

BOOL RichEditHost::GetRichTextFlag()
{
	return m_fRich;
}

void RichEditHost::SetRichTextFlag(BOOL fNew)
{
	m_fRich = fNew;

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_RICHTEXT,
		fNew ? TXTBIT_RICHTEXT : 0);
}

LONG RichEditHost::GetDefaultLeftIndent()
{
	return m_pf.dxOffset;
}

void RichEditHost::SetDefaultLeftIndent(LONG lNewIndent)
{
	m_pf.dxOffset = lNewIndent;

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

void RichEditHost::SetClientRect(UiRect* prc)
{
	m_rcClient = *prc;

	LONG xPerInch = ::GetDeviceCaps(m_pRichEdit->GetWindowDC(), LOGPIXELSX);
	LONG yPerInch = ::GetDeviceCaps(m_pRichEdit->GetWindowDC(), LOGPIXELSY);
	if (xPerInch == 0)
		xPerInch = 96;
	if (yPerInch == 0)
		yPerInch = 96;
	m_sizelExtent.cx = DXtoHimetricX(m_rcClient.right - m_rcClient.left, xPerInch);
	m_sizelExtent.cy = DYtoHimetricY(m_rcClient.bottom - m_rcClient.top, yPerInch);

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE, TXTBIT_VIEWINSETCHANGE);
}

BOOL RichEditHost::SetSaveSelection(BOOL fSaveSelection)
{
	BOOL fResult = fSaveSelection;

	m_fSaveSelection = fSaveSelection;

	// notify text services of property change
	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_SAVESELECTION,
		m_fSaveSelection ? TXTBIT_SAVESELECTION : 0);

	return fResult;
}

HRESULT	RichEditHost::OnTxInPlaceDeactivate()
{
	HRESULT hr = m_pTextServices->OnTxInPlaceDeactivate();

	if (SUCCEEDED(hr))
	{
		m_fInplaceActive = FALSE;
	}

	return hr;
}

HRESULT	RichEditHost::OnTxInPlaceActivate(LPCRECT prcClient)
{
	m_fInplaceActive = TRUE;

	HRESULT hr = m_pTextServices->OnTxInPlaceActivate(prcClient);

	if (FAILED(hr))
	{
		m_fInplaceActive = FALSE;
	}

	return hr;
}

BOOL RichEditHost::DoSetCursor(const UiRect* prc, const UiPoint* pt)
{
	ASSERT(pt != nullptr);
	if (pt == nullptr) {
		return FALSE;
	}

	UiRect rc = (prc != NULL) ? *prc : m_rcClient;

	// Is this in our rectangle?
	UiPoint newPt(pt->x, pt->y);
	newPt.Offset(m_pRichEdit->GetScrollOffsetInScrollBox());
	if (rc.ContainsPt(newPt))
	{
		RECT* pRect = nullptr;
		const UiRect* prcClient = (!m_fInplaceActive || prc) ? &rc : nullptr;
		RECT rect = { 0, };
		if (prcClient != nullptr) {
			rect = { prcClient->left, prcClient->top, prcClient->right, prcClient->bottom };
			pRect = &rect;
		}
		m_pTextServices->OnTxSetCursor(DVASPECT_CONTENT, -1, NULL, NULL, m_pRichEdit->GetWindowDC(),
			NULL, pRect, newPt.x, newPt.y);

		return TRUE;
	}

	return FALSE;
}

void RichEditHost::GetControlRect(UiRect* prc)
{
	ASSERT(prc != nullptr);
	if (prc == nullptr) {
		return;
	}
	UiRect rc = m_rcClient;
	VerAlignType alignType = m_pRichEdit->GetTextVerAlignType();
	if (alignType != kVerAlignTop) {
		LONG iWidth = rc.right - rc.left;
		LONG iHeight = 0;
		SIZEL szExtent = { -1, -1 };
		GetTextServices()->TxGetNaturalSize(
			DVASPECT_CONTENT,
			m_pRichEdit->GetWindowDC(),
			NULL,
			NULL,
			TXTNS_FITTOCONTENT,
			&szExtent,
			&iWidth,
			&iHeight);
		if (alignType == kVerAlignCenter) {
			rc.Offset(0, (rc.bottom - rc.top - iHeight) / 2);
		}
		else if (alignType == kVerAlignBottom) {
			rc.Offset(0, prc->bottom - prc->top - iHeight);
		}
	}

	prc->left = rc.left;
	prc->top = rc.top;
	prc->right = rc.right;
	prc->bottom = rc.bottom;
}

void RichEditHost::SetTransparent(BOOL fTransparent)
{
	m_fTransparent = fTransparent;

	// notify text services of property change
	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 0);
}

LONG RichEditHost::SetAccelPos(LONG lAcceleratorPos)
{
	LONG laccelposOld = lAcceleratorPos;

	m_lAcceleratorPos = lAcceleratorPos;

	// notify text services of property change
	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_SHOWACCELERATOR, 0);

	return laccelposOld;
}

WCHAR RichEditHost::SetPasswordChar(WCHAR chPasswordChar)
{
	WCHAR chOldPasswordChar = m_chPasswordChar;

	m_chPasswordChar = chPasswordChar;

	// notify text services of property change
	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD,
		(m_chPasswordChar != 0) ? TXTBIT_USEPASSWORD : 0);

	return chOldPasswordChar;
}

void RichEditHost::SetDisabled(BOOL fOn)
{
	m_cf.dwMask |= CFM_COLOR | CFM_DISABLED;
	m_cf.dwEffects |= CFE_AUTOCOLOR | CFE_DISABLED;

	if (!fOn)
	{
		m_cf.dwEffects &= ~CFE_DISABLED;
	}

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE,
		TXTBIT_CHARFORMATCHANGE);
}

LONG RichEditHost::SetSelBarWidth(LONG lSelBarWidth)
{
	LONG lOldSelBarWidth = m_lSelBarWidth;

	m_lSelBarWidth = lSelBarWidth;

	if (m_lSelBarWidth)
	{
		m_dwStyle |= ES_SELECTIONBAR;
	}
	else
	{
		m_dwStyle &= (~ES_SELECTIONBAR);
	}

	m_pTextServices->OnTxPropertyBitsChange(TXTBIT_SELBARCHANGE, TXTBIT_SELBARCHANGE);

	return lOldSelBarWidth;
}

BOOL RichEditHost::GetTimerState()
{
	return m_fTimer;
}

void RichEditHost::SetCharFormat(const CHARFORMAT2W& c)
{
	m_cf = c;
}

void RichEditHost::SetParaFormat(const PARAFORMAT2& p)
{
	m_pf = p;
}

// Convert Pixels on the X axis to Himetric
LONG RichEditHost::DXtoHimetricX(LONG dx, LONG xPerInch)
{
	return (LONG)MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
}

// Convert Pixels on the Y axis to Himetric
LONG RichEditHost::DYtoHimetricY(LONG dy, LONG yPerInch)
{
	return (LONG)MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
}

int32_t RichEditHost::GetDefaultMaxText()
{
	return cInitTextMax;
}

}//name space ui
