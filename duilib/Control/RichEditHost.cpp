#include "RichEditHost.h"
#include "RichEdit.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"

namespace ui
{

/** 文本最大字符个数
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
	m_re(nullptr),
	cRefs(0),
	pserv(nullptr),
	dwStyle(0),
	fEnableAutoWordSel(0),
	fWordWrap(0),
	fAllowBeep(0),
	fRich(0),
	fSaveSelection(0),
	fInplaceActive(0),
	fTransparent(0),
	fTimer(0),
	fCaptured(0),
	lSelBarWidth(0),
	cchTextMost(cInitTextMax),
	dwEventMask(0),
	icf(0),
	ipf(0),
	rcClient(),
	sizelExtent({ 0 }),
	cf({ 0 }),
	pf({ 0 }),
	laccelpos(-1),
	chPasswordChar(0)
{
}

RichEditHost::~RichEditHost()
{
	if (pserv) {
		pserv->OnTxInPlaceDeactivate();
		pserv->Release();
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
	if (NULL == pserv)
		return NULL;
	pserv->AddRef();
	return pserv;
}

BOOL RichEditHost::SetOleCallback(IRichEditOleCallback* pCallback)
{
	if (NULL == pserv)
		return FALSE;
	LRESULT lRes = 0;
	pserv->TxSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pCallback, &lRes);
	return (BOOL)lRes;
}

BOOL RichEditHost::CanPaste(UINT nFormat)
{
	if (NULL == pserv)
		return FALSE;
	LRESULT lRes = 0;
	pserv->TxSendMessage(EM_CANPASTE, nFormat, 0L, &lRes);
	return (BOOL)lRes;
}

void RichEditHost::PasteSpecial(UINT uClipFormat, DWORD dwAspect, HMETAFILE hMF)
{
	if (NULL == pserv)
		return;
	REPASTESPECIAL reps = { dwAspect, (DWORD_PTR)hMF };
	pserv->TxSendMessage(EM_PASTESPECIAL, uClipFormat, (LPARAM)&reps, NULL);
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
	m_re = re;
	// Initialize Reference count
	cRefs = 1;

	// Create and cache CHARFORMAT for this control
	if (FAILED(InitDefaultCharFormat(re, &cf)))
	{
		return FALSE;
	}

	// Create and cache PARAFORMAT for this control
	if (FAILED(InitDefaultParaFormat(re, &pf)))
	{
		return FALSE;
	}

	// edit controls created without a window are multiline by default
	// so that paragraph formats can be
	dwStyle = ES_MULTILINE;

	// edit controls are rich by default
	fRich = re->IsRich();

	cchTextMost = re->GetLimitText();

	if (pcs)
	{
		dwStyle = pcs->style;

		if (!(dwStyle & (ES_AUTOHSCROLL | WS_HSCROLL)))
		{
			fWordWrap = TRUE;
		}
	}

	if (!(dwStyle & ES_LEFT))
	{
		if (dwStyle & ES_CENTER)
			pf.wAlignment = PFA_CENTER;
		else if (dwStyle & ES_RIGHT)
			pf.wAlignment = PFA_RIGHT;
	}

	fInplaceActive = TRUE;

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
		hr = pUnk->QueryInterface(IID_ITextServices, (void**)&pserv);

		// Whether the previous call succeeded or failed we are done
		// with the private interface.
		pUnk->Release();

		if (FAILED(hr))
		{
			return FALSE;
		}
	}


	// Set window text
	if (pserv && pcs && pcs->lpszName)
	{
		if (SUCCEEDED(pserv->TxSetText((TCHAR*)pcs->lpszName)))
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
	return ++cRefs;
}

ULONG RichEditHost::Release(void)
{
	ULONG c_Refs = --cRefs;

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
	return m_re->GetWindowDC();
}

int RichEditHost::TxReleaseDC(HDC /*hdc*/)
{
	return 1;
}

BOOL RichEditHost::TxShowScrollBar(INT /*fnBar*/, BOOL /*fShow*/)
{
	ASSERT(FALSE); //暂时注释掉，不知道这代码有啥用   by panqinke 2014.5.6
	//ScrollBar* pVerticalScrollBar = m_re->GetVScrollBar();
	//ScrollBar* pHorizontalScrollBar = m_re->GetHScrollBar();
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
			m_re->GetVScrollBar()->SetScrollRange(0);
		}
	}
	else if (fuSBFlags == SB_HORZ) {
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			m_re->GetHScrollBar()->SetScrollRange(0);
		}
	}
	else if (fuSBFlags == SB_BOTH) {
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			m_re->GetVScrollBar()->SetScrollRange(0);
		}
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			m_re->GetHScrollBar()->SetScrollRange(0);
		}
	}

	m_re->SetPos(m_re->GetPos());

	return TRUE;
}

BOOL RichEditHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL /*fRedraw*/)
{
	ScrollBar* pVerticalScrollBar = m_re->GetVScrollBar();
	ScrollBar* pHorizontalScrollBar = m_re->GetHScrollBar();
	bool bArrange = false;
	if (fnBar == SB_VERT && pVerticalScrollBar) {
		if (nMaxPos - nMinPos - rcClient.bottom + rcClient.top <= 0) {
			pVerticalScrollBar->SetScrollRange(0);
		}
		else {
			if (!pVerticalScrollBar->IsValid()) {
				bArrange = true;
			}
			pVerticalScrollBar->SetScrollRange(nMaxPos - nMinPos - rcClient.bottom + rcClient.top);
		}
	}
	else if (fnBar == SB_HORZ && pHorizontalScrollBar) {
		if (nMaxPos - nMinPos - rcClient.right + rcClient.left <= 0) {
			pHorizontalScrollBar->SetScrollRange(0);
		}
		else {
			if (!pHorizontalScrollBar->IsValid()) {
				bArrange = true;
			}
			pHorizontalScrollBar->SetScrollRange(nMaxPos - nMinPos - rcClient.right + rcClient.left);
		}
	}

	if (bArrange) {
		m_re->SetPos(m_re->GetPos());
	}

	return TRUE;
}

BOOL RichEditHost::TxSetScrollPos(INT fnBar, INT nPos, BOOL /*fRedraw*/)
{
	ScrollBar* pVerticalScrollBar = m_re->GetVScrollBar();
	ScrollBar* pHorizontalScrollBar = m_re->GetHScrollBar();
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
	if (!m_re->GetWindow())
		return;

	UiPoint scrollOffset = m_re->GetScrollOffsetInScrollBox();
	if (prc == NULL) {
		UiRect newRcClient = rcClient;
		newRcClient.Offset(-scrollOffset.x, -scrollOffset.y);
		m_re->GetWindow()->Invalidate(newRcClient);
		return;
	}
	UiRect rc = MakeUiRect(*prc);
	rc.Offset(-scrollOffset.x, -scrollOffset.y);
	m_re->GetWindow()->Invalidate(rc);
}

void RichEditHost::TxViewChange(BOOL /*fUpdate*/)
{

}

BOOL RichEditHost::TxCreateCaret(HBITMAP /*hbmp*/, INT xWidth, INT yHeight)
{
	return m_re->CreateCaret(xWidth, yHeight);
}

BOOL RichEditHost::TxShowCaret(BOOL /*fShow*/)
{
	return true; // m_re->ShowCaret(fShow);
}

BOOL RichEditHost::TxSetCaretPos(INT x, INT y)
{
	return m_re->SetCaretPos(x, y);
}

BOOL RichEditHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
	fTimer = TRUE;
	m_re->SetTimer(idTimer, uTimeout);
	return TRUE;
}

void RichEditHost::TxKillTimer(UINT idTimer)
{
	m_re->KillTimer(idTimer);
	fTimer = FALSE;
}

void RichEditHost::TxScrollWindowEx(INT /*dx*/, INT /*dy*/, LPCRECT /*lprcScroll*/, LPCRECT /*lprcClip*/, HRGN /*hrgnUpdate*/, LPRECT /*lprcUpdate*/, UINT /*fuScroll*/)
{
	return;
}

void RichEditHost::TxSetCapture(BOOL fCapture)
{
	if (!m_re->GetWindow())
		return;

	if (fCapture) m_re->GetWindow()->SetCapture();
	else m_re->GetWindow()->ReleaseCapture();
	fCaptured = fCapture;
}

void RichEditHost::TxSetFocus()
{
	m_re->SetFocus();
}

void RichEditHost::TxSetCursor(HCURSOR hcur, BOOL /*fText*/)
{
	::SetCursor(hcur);
}

BOOL RichEditHost::TxScreenToClient(LPPOINT lppt)
{
	return ::ScreenToClient(m_re->GetWindowHandle(), lppt);
}

BOOL RichEditHost::TxClientToScreen(LPPOINT lppt)
{
	return ::ClientToScreen(m_re->GetWindowHandle(), lppt);
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
		UiRect rcTemp = rcClient;
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
		*ppCF = &cf;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetParaFormat(const PARAFORMAT** ppPF)
{
	ASSERT(ppPF != nullptr);
	if (ppPF) {
		*ppPF = &pf;
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
		*pstyle = !fTransparent ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetMaxLength(DWORD* pLength)
{
	ASSERT(pLength != nullptr);
	if (pLength) {
		*pLength = cchTextMost;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetScrollBars(DWORD* pdwScrollBar)
{
	ASSERT(pdwScrollBar != nullptr);
	if (pdwScrollBar) {
		*pdwScrollBar = dwStyle & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL |
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
	* pch = chPasswordChar;
#else
	::WideCharToMultiByte(CP_ACP, 0, &chPasswordChar, 1, pch, 1, NULL, NULL);
#endif
	return NOERROR;
}

HRESULT RichEditHost::TxGetAcceleratorPos(LONG* pcp)
{
	ASSERT(pcp != nullptr);
	if (pcp != nullptr) {
		*pcp = laccelpos;
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

	if (fRich)
	{
		dwProperties = TXTBIT_RICHTEXT;
	}

	if (dwStyle & ES_MULTILINE)
	{
		dwProperties |= TXTBIT_MULTILINE;
	}

	if (dwStyle & ES_READONLY)
	{
		dwProperties |= TXTBIT_READONLY;
	}

	if (dwStyle & ES_PASSWORD)
	{
		dwProperties |= TXTBIT_USEPASSWORD;
	}

	if (!(dwStyle & ES_NOHIDESEL))
	{
		dwProperties |= TXTBIT_HIDESELECTION;
	}

	if (fEnableAutoWordSel)
	{
		dwProperties |= TXTBIT_AUTOWORDSEL;
	}

	if (fWordWrap)
	{
		dwProperties |= TXTBIT_WORDWRAP;
	}

	if (fAllowBeep)
	{
		dwProperties |= TXTBIT_ALLOWBEEP;
	}

	if (fSaveSelection)
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
	m_re->OnTxNotify(iNotify, pv);
	return S_OK;
}

HRESULT RichEditHost::TxGetExtent(LPSIZEL lpExtent)
{
	ASSERT(lpExtent != nullptr);
	if (lpExtent) {
		*lpExtent = sizelExtent;
	}
	return S_OK;
}

HRESULT	RichEditHost::TxGetSelectionBarWidth(LONG* plSelBarWidth)
{
	ASSERT(plSelBarWidth != nullptr);
	if (plSelBarWidth) {
		*plSelBarWidth = lSelBarWidth;
	}
	return S_OK;
}

void RichEditHost::SetWordWrap(BOOL bWordWrap)
{
	fWordWrap = bWordWrap;
	if (pserv) {
		pserv->OnTxPropertyBitsChange(TXTBIT_WORDWRAP, bWordWrap ? TXTBIT_WORDWRAP : 0);
	}
}

BOOL RichEditHost::GetReadOnly()
{
	return (dwStyle & ES_READONLY) != 0;
}

void RichEditHost::SetReadOnly(BOOL fReadOnly)
{
	if (fReadOnly)
	{
		dwStyle |= ES_READONLY;
	}
	else
	{
		dwStyle &= ~ES_READONLY;
	}

	pserv->OnTxPropertyBitsChange(TXTBIT_READONLY,
		fReadOnly ? TXTBIT_READONLY : 0);
}

BOOL RichEditHost::IsPassword()
{
	return (dwStyle & ES_PASSWORD) != 0;
}

void RichEditHost::SetPassword(BOOL bPassword)
{
	if (bPassword)
	{
		dwStyle |= ES_PASSWORD;
	}
	else
	{
		dwStyle &= ~ES_PASSWORD;
	}

	pserv->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD,
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
	LONG yPixPerInch = ::GetDeviceCaps(m_re->GetWindowDC(), LOGPIXELSY);
	if (yPixPerInch == 0)
		yPixPerInch = 96;
	cf.yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
	if (lf.lfWeight >= FW_BOLD)
		cf.dwEffects |= CFE_BOLD;
	if (lf.lfItalic)
		cf.dwEffects |= CFE_ITALIC;
	if (lf.lfUnderline)
		cf.dwEffects |= CFE_UNDERLINE;
	if (lf.lfStrikeOut)
		cf.dwEffects |= CFE_STRIKEOUT;
	cf.bCharSet = lf.lfCharSet;
	cf.bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
	wcscpy_s(cf.szFaceName, lf.lfFaceName);
#else
	//need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
	MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, cf.szFaceName, LF_FACESIZE);
#endif

	pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE,
		TXTBIT_CHARFORMATCHANGE);
}

void RichEditHost::SetColor(DWORD dwColor)
{
	cf.crTextColor = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
	pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE,
		TXTBIT_CHARFORMATCHANGE);
}

SIZEL* RichEditHost::GetExtent()
{
	return &sizelExtent;
}

void RichEditHost::SetExtent(SIZEL* psizelExtent)
{
	if (psizelExtent) {
		sizelExtent = *psizelExtent;
		if (pserv) {
			pserv->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
		}
	}
}

void RichEditHost::LimitText(LONG nChars)
{
	cchTextMost = nChars;
	if (cchTextMost <= 0) cchTextMost = cInitTextMax;
	pserv->OnTxPropertyBitsChange(TXTBIT_MAXLENGTHCHANGE, TXTBIT_MAXLENGTHCHANGE);
}

BOOL RichEditHost::IsCaptured()
{
	return fCaptured;
}

BOOL RichEditHost::GetAllowBeep()
{
	return fAllowBeep;
}

void RichEditHost::SetAllowBeep(BOOL bAllowBeep)
{
	fAllowBeep = bAllowBeep;

	pserv->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP,
		bAllowBeep ? TXTBIT_ALLOWBEEP : 0);
}

WORD RichEditHost::GetDefaultAlign()
{
	return pf.wAlignment;
}

void RichEditHost::SetDefaultAlign(WORD wNewAlign)
{
	pf.wAlignment = wNewAlign;

	// Notify control of property change
	pserv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

BOOL RichEditHost::GetRichTextFlag()
{
	return fRich;
}

void RichEditHost::SetRichTextFlag(BOOL fNew)
{
	fRich = fNew;

	pserv->OnTxPropertyBitsChange(TXTBIT_RICHTEXT,
		fNew ? TXTBIT_RICHTEXT : 0);
}

LONG RichEditHost::GetDefaultLeftIndent()
{
	return pf.dxOffset;
}

void RichEditHost::SetDefaultLeftIndent(LONG lNewIndent)
{
	pf.dxOffset = lNewIndent;

	pserv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

void RichEditHost::SetClientRect(UiRect* prc)
{
	rcClient = *prc;

	LONG xPerInch = ::GetDeviceCaps(m_re->GetWindowDC(), LOGPIXELSX);
	LONG yPerInch = ::GetDeviceCaps(m_re->GetWindowDC(), LOGPIXELSY);
	if (xPerInch == 0)
		xPerInch = 96;
	if (yPerInch == 0)
		yPerInch = 96;
	sizelExtent.cx = DXtoHimetricX(rcClient.right - rcClient.left, xPerInch);
	sizelExtent.cy = DYtoHimetricY(rcClient.bottom - rcClient.top, yPerInch);

	pserv->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE, TXTBIT_VIEWINSETCHANGE);
}

BOOL RichEditHost::SetSaveSelection(BOOL f_SaveSelection)
{
	BOOL fResult = f_SaveSelection;

	fSaveSelection = f_SaveSelection;

	// notify text services of property change
	pserv->OnTxPropertyBitsChange(TXTBIT_SAVESELECTION,
		fSaveSelection ? TXTBIT_SAVESELECTION : 0);

	return fResult;
}

HRESULT	RichEditHost::OnTxInPlaceDeactivate()
{
	HRESULT hr = pserv->OnTxInPlaceDeactivate();

	if (SUCCEEDED(hr))
	{
		fInplaceActive = FALSE;
	}

	return hr;
}

HRESULT	RichEditHost::OnTxInPlaceActivate(LPCRECT prcClient)
{
	fInplaceActive = TRUE;

	HRESULT hr = pserv->OnTxInPlaceActivate(prcClient);

	if (FAILED(hr))
	{
		fInplaceActive = FALSE;
	}

	return hr;
}

BOOL RichEditHost::DoSetCursor(const UiRect* prc, const UiPoint* pt)
{
	ASSERT(pt != nullptr);
	if (pt == nullptr) {
		return FALSE;
	}

	UiRect rc = (prc != NULL) ? *prc : rcClient;

	// Is this in our rectangle?
	UiPoint newPt(pt->x, pt->y);
	newPt.Offset(m_re->GetScrollOffsetInScrollBox());
	if (rc.ContainsPt(newPt))
	{
		RECT* pRect = nullptr;
		const UiRect* prcClient = (!fInplaceActive || prc) ? &rc : nullptr;
		RECT rect = { 0, };
		if (prcClient != nullptr) {
			rect = { prcClient->left, prcClient->top, prcClient->right, prcClient->bottom };
			pRect = &rect;
		}
		pserv->OnTxSetCursor(DVASPECT_CONTENT, -1, NULL, NULL, m_re->GetWindowDC(),
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
	UiRect rc = rcClient;
	VerAlignType alignType = m_re->GetTextVerAlignType();
	if (alignType != kVerAlignTop) {
		LONG iWidth = rc.right - rc.left;
		LONG iHeight = 0;
		SIZEL szExtent = { -1, -1 };
		GetTextServices()->TxGetNaturalSize(
			DVASPECT_CONTENT,
			m_re->GetWindowDC(),
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

void RichEditHost::SetTransparent(BOOL f_Transparent)
{
	fTransparent = f_Transparent;

	// notify text services of property change
	pserv->OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 0);
}

LONG RichEditHost::SetAccelPos(LONG l_accelpos)
{
	LONG laccelposOld = l_accelpos;

	laccelpos = l_accelpos;

	// notify text services of property change
	pserv->OnTxPropertyBitsChange(TXTBIT_SHOWACCELERATOR, 0);

	return laccelposOld;
}

WCHAR RichEditHost::SetPasswordChar(WCHAR ch_PasswordChar)
{
	WCHAR chOldPasswordChar = chPasswordChar;

	chPasswordChar = ch_PasswordChar;

	// notify text services of property change
	pserv->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD,
		(chPasswordChar != 0) ? TXTBIT_USEPASSWORD : 0);

	return chOldPasswordChar;
}

void RichEditHost::SetDisabled(BOOL fOn)
{
	cf.dwMask |= CFM_COLOR | CFM_DISABLED;
	cf.dwEffects |= CFE_AUTOCOLOR | CFE_DISABLED;

	if (!fOn)
	{
		cf.dwEffects &= ~CFE_DISABLED;
	}

	pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE,
		TXTBIT_CHARFORMATCHANGE);
}

LONG RichEditHost::SetSelBarWidth(LONG l_SelBarWidth)
{
	LONG lOldSelBarWidth = lSelBarWidth;

	lSelBarWidth = l_SelBarWidth;

	if (lSelBarWidth)
	{
		dwStyle |= ES_SELECTIONBAR;
	}
	else
	{
		dwStyle &= (~ES_SELECTIONBAR);
	}

	pserv->OnTxPropertyBitsChange(TXTBIT_SELBARCHANGE, TXTBIT_SELBARCHANGE);

	return lOldSelBarWidth;
}

BOOL RichEditHost::GetTimerState()
{
	return fTimer;
}

void RichEditHost::SetCharFormat(const CHARFORMAT2W& c)
{
	cf = c;
}

void RichEditHost::SetParaFormat(const PARAFORMAT2& p)
{
	pf = p;
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
