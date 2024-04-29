#include "RichEditHost.h"
#include "RichEdit.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"

#define UI_ES_LEFT				0x0001L
#define UI_ES_CENTER			0x0002L
#define UI_ES_RIGHT				0x0004L
#define UI_ES_TOP				0x0008L
#define UI_ES_VCENTER			0x0010L
#define UI_ES_BOTTOM			0x0020L
#define UI_ES_MULTILINE			0x0040L
#define UI_ES_NUMBER			0x0080L
#define UI_ES_PASSWORD			0x0100L
#define UI_ES_READONLY			0x0200L
#define UI_ES_NOHIDESEL			0x0400L
#define UI_ES_AUTOHSCROLL		0x0800L
#define UI_ES_AUTOVSCROLL		0x1000L
#define UI_ES_DISABLENOSCROLL	0x2000L
#define UI_WS_HSCROLL			0x4000L
#define UI_WS_VSCROLL			0x8000L

namespace ui
{
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

RichEditHost::RichEditHost(RichEdit* pRichEdit) :
	m_pRichEdit(pRichEdit),
	m_cRefs(1),
	m_pTextServices(nullptr),
	m_dwStyle(0),
	m_fEnableAutoWordSel(false),
	m_fWordWrap(false),
	m_fAllowBeep(false),
	m_fRichText(false),
	m_fSaveSelection(false),
	m_fInplaceActive(false),
	m_fTransparent(false),
	m_lSelBarWidth(0),
	m_rcClient(),
	m_sizelExtent({ 0 }),
	m_charFormat({ 0, }),
	m_paraFormat({ 0, }),
	m_chPasswordChar(0),
	m_bShowPassword(false),
	m_bFlashPasswordChar(false)

{
	Init();
}

RichEditHost::~RichEditHost()
{
	if (m_pTextServices) {
		m_pTextServices->OnTxInPlaceDeactivate();
		m_pTextServices->Release();
	}
}

void RichEditHost::Init()
{
	RichEdit* pRichEdit = m_pRichEdit;

	//初始化默认字体
	LOGFONT lf = { 0, };
	::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	InitCharFormat(lf);

	//设置字体颜色
	if (pRichEdit != nullptr) {
		std::wstring textColor;
		if (pRichEdit->IsEnabled()) {
			textColor = pRichEdit->GetTextColor();
		}
		else {
			textColor = pRichEdit->GetDisabledTextColor();
		}
		if (!textColor.empty()) {
			UiColor dwColor = pRichEdit->GetUiColor(textColor);
			m_charFormat.dwMask |= CFM_COLOR;
			m_charFormat.crTextColor = dwColor.ToCOLORREF();
		}
	}

	//初始化默认段落格式
	memset(&m_paraFormat, 0, sizeof(PARAFORMAT2));
	m_paraFormat.cbSize = sizeof(PARAFORMAT2);
	m_paraFormat.dwMask = PFM_ALL;
	m_paraFormat.wAlignment = PFA_LEFT;
	m_paraFormat.cTabCount = 1;
	m_paraFormat.rgxTabs[0] = lDefaultTab;

	IUnknown* pUnk = nullptr;
	HRESULT hr = E_FAIL;

	//默认为单行文本
	m_dwStyle = 0;

	//默认开启：当用户在最后一行按 ENTER 时，自动将文本向上滚动一页。
	m_dwStyle |= UI_ES_AUTOVSCROLL;

	//默认开启：当用户在行尾键入一个字符时，自动将文本向右滚动 10 个字符。
	m_dwStyle |= UI_ES_AUTOHSCROLL;

	//自动选择单词
	m_fEnableAutoWordSel = true;
	//自动换行
	m_fWordWrap = false;
	//默认为纯文本模式
	m_fRichText = false;

	m_fInplaceActive = true;

	PCreateTextServices pfnTextServicesProc = nullptr;
	HMODULE hRichEditModule = GlobalManager::Instance().GetRichEditModule();
	if (hRichEditModule != nullptr) {
		pfnTextServicesProc = (PCreateTextServices)::GetProcAddress(hRichEditModule, "CreateTextServices");
	}

	if (pfnTextServicesProc) {
		pfnTextServicesProc(NULL, this, &pUnk);
	}

	ASSERT(m_pTextServices == nullptr);
	if (pUnk != nullptr) {
		hr = pUnk->QueryInterface(IID_ITextServices, (void**)&m_pTextServices);
		pUnk->Release();
	}

	ASSERT(m_pTextServices != nullptr);
	if (m_pTextServices != nullptr) {
		//设置默认属性
		SetTransparent(TRUE);
		LRESULT lResult = 0;
		m_pTextServices->TxSendMessage(EM_SETLANGOPTIONS, 0, 0, &lResult);
		m_pTextServices->TxSendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_LINK, &lResult);
		m_pTextServices->OnTxInPlaceActivate(nullptr);
	}
}

ITextServices* RichEditHost::GetTextServices() const
{ 
	return m_pTextServices; 
}

void RichEditHost::Clear()
{
	if (m_pTextServices != nullptr) {
		m_pTextServices->Release();
		m_pTextServices = nullptr;
	}
}

void RichEditHost::OnTxPropertyBitsChange(DWORD dwMask, DWORD dwBits)
{
	ASSERT(m_pTextServices != nullptr);
	if (m_pTextServices != nullptr) {
		m_pTextServices->OnTxPropertyBitsChange(dwMask, dwBits);
	}
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
	*ppvObject = nullptr;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITextHost)) {
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
	if (c_Refs == 0) {
		delete this;
	}
	return c_Refs;
}

//////////////////////////// ITextHost Interface  ////////////////////////////

HDC RichEditHost::TxGetDC()
{
	if (m_pRichEdit != nullptr) {
		return m_pRichEdit->GetWindowDC();
	}
	return nullptr;
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
	if (m_pRichEdit == nullptr) {
		return TRUE;
	}
	if (fuSBFlags == SB_VERT) {
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			if (m_pRichEdit->GetVScrollBar() != nullptr) {
				m_pRichEdit->GetVScrollBar()->SetScrollRange(0);
			}
		}
	}
	else if (fuSBFlags == SB_HORZ) {
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			if (m_pRichEdit->GetHScrollBar() != nullptr) {
				m_pRichEdit->GetHScrollBar()->SetScrollRange(0);
			}
		}
	}
	else if (fuSBFlags == SB_BOTH) {
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			if (m_pRichEdit->GetVScrollBar() != nullptr) {
				m_pRichEdit->GetVScrollBar()->SetScrollRange(0);
			}
		}
		if (fuArrowflags == ESB_DISABLE_BOTH) {
			if (m_pRichEdit->GetHScrollBar() != nullptr) {
				m_pRichEdit->GetHScrollBar()->SetScrollRange(0);
			}
		}
	}
	m_pRichEdit->SetPos(m_pRichEdit->GetPos());
	return TRUE;
}

BOOL RichEditHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL /*fRedraw*/)
{
	if (m_pRichEdit == nullptr) {
		return TRUE;
	}
	ScrollBar* pVScrollBar = m_pRichEdit->GetVScrollBar();
	ScrollBar* pHScrollBar = m_pRichEdit->GetHScrollBar();
	bool bArrange = false;
	if (fnBar == SB_VERT && (pVScrollBar != nullptr)) {
		if (nMaxPos - nMinPos - m_rcClient.Height() <= 0) {
			pVScrollBar->SetScrollRange(0);
		}
		else {
			if (!pVScrollBar->IsValid()) {
				bArrange = true;
			}
			pVScrollBar->SetScrollRange(nMaxPos - nMinPos - m_rcClient.Height());
		}
	}
	else if (fnBar == SB_HORZ && (pHScrollBar != nullptr)) {
		if (nMaxPos - nMinPos - m_rcClient.Width() <= 0) {
			pHScrollBar->SetScrollRange(0);
		}
		else {
			if (!pHScrollBar->IsValid()) {
				bArrange = true;
			}
			pHScrollBar->SetScrollRange(nMaxPos - nMinPos - m_rcClient.Width());
		}
	}

	if (bArrange) {
		m_pRichEdit->SetPos(m_pRichEdit->GetPos());
	}
	return TRUE;
}

BOOL RichEditHost::TxSetScrollPos(INT fnBar, INT nPos, BOOL /*fRedraw*/)
{
	if (m_pRichEdit == nullptr) {
		return TRUE;
	}
	ScrollBar* pVScrollBar = m_pRichEdit->GetVScrollBar();
	ScrollBar* pHScrollBar = m_pRichEdit->GetHScrollBar();
	if (fnBar == SB_VERT && (pVScrollBar != nullptr)) {
		pVScrollBar->SetScrollPos(nPos);
	}
	else if (fnBar == SB_HORZ && (pHScrollBar != nullptr)) {
		pHScrollBar->SetScrollPos(nPos);
	}
	return TRUE;
}

void RichEditHost::TxInvalidateRect(LPCRECT prc, BOOL /*fMode*/)
{
	if (m_pRichEdit == nullptr) {
		return;
	}
	Window* pWindow = m_pRichEdit->GetWindow();
	if (pWindow == nullptr) {
		return;
	}

	UiPoint scrollOffset = m_pRichEdit->GetScrollOffsetInScrollBox();
	UiRect rc = (prc == nullptr) ? m_rcClient : MakeUiRect(*prc);
	rc.Offset(-scrollOffset.x, -scrollOffset.y);
	pWindow->Invalidate(rc);
}

void RichEditHost::TxViewChange(BOOL /*fUpdate*/)
{
}

BOOL RichEditHost::TxCreateCaret(HBITMAP /*hbmp*/, INT xWidth, INT yHeight)
{
	if (m_pRichEdit == nullptr) {
		return TRUE;
	}
	return m_pRichEdit->CreateCaret(xWidth, yHeight);
}

BOOL RichEditHost::TxShowCaret(BOOL /*fShow*/)
{
	return true;
}

BOOL RichEditHost::TxSetCaretPos(INT x, INT y)
{
	if (m_pRichEdit == nullptr) {
		return TRUE;
	}
	return m_pRichEdit->SetCaretPos(x, y);
}

BOOL RichEditHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
	if (m_pRichEdit == nullptr) {
		return TRUE;
	}
	m_pRichEdit->SetTimer(idTimer, uTimeout);
	return TRUE;
}

void RichEditHost::TxKillTimer(UINT idTimer)
{
	if (m_pRichEdit == nullptr) {
		return;
	}
	m_pRichEdit->KillTimer(idTimer);
}

void RichEditHost::TxScrollWindowEx(INT /*dx*/, INT /*dy*/, LPCRECT /*lprcScroll*/, LPCRECT /*lprcClip*/, HRGN /*hrgnUpdate*/, LPRECT /*lprcUpdate*/, UINT /*fuScroll*/)
{
	return;
}

void RichEditHost::TxSetCapture(BOOL fCapture)
{
	Window* pWindow = nullptr;
	if (m_pRichEdit != nullptr) {
		pWindow = m_pRichEdit->GetWindow();
	}
	if (pWindow == nullptr) {
		return;
	}
	if (fCapture) {
		pWindow->SetCapture();
	}
	else {
		pWindow->ReleaseCapture();
	}
}

void RichEditHost::TxSetFocus()
{
	if (m_pRichEdit != nullptr) {
		m_pRichEdit->SetFocus();
	}
}

void RichEditHost::TxSetCursor(HCURSOR hcur, BOOL /*fText*/)
{
	::SetCursor(hcur);
}

BOOL RichEditHost::TxScreenToClient(LPPOINT lppt)
{
	if (m_pRichEdit == nullptr) {
		return FALSE;
	}
	return ::ScreenToClient(m_pRichEdit->GetWindowHandle(), lppt);
}

BOOL RichEditHost::TxClientToScreen(LPPOINT lppt)
{
	if (m_pRichEdit == nullptr) {
		return FALSE;
	}
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
	if (prc != nullptr) {
		UiRect rcTemp = m_rcClient;
		GetControlRect(&rcTemp);
		*prc = { rcTemp.left, rcTemp.top, rcTemp.right, rcTemp.bottom };
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetViewInset(LPRECT prc)
{
	ASSERT(prc != nullptr);
	if (prc != nullptr) {
		prc->left = prc->right = prc->top = prc->bottom = 0;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetCharFormat(const CHARFORMATW** ppCF)
{
	ASSERT(ppCF != nullptr);
	if (ppCF != nullptr) {
		*ppCF = &m_charFormat;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetParaFormat(const PARAFORMAT** ppPF)
{
	ASSERT(ppPF != nullptr);
	if (ppPF != nullptr) {
		*ppPF = &m_paraFormat;
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
	if (pstyle != nullptr) {
		*pstyle = !m_fTransparent ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetMaxLength(DWORD* pLength)
{
	ASSERT(pLength != nullptr);
	if (pLength) {
		*pLength = INFINITE;
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetScrollBars(DWORD* pdwScrollBar)
{
	ASSERT(pdwScrollBar != nullptr);
	if (pdwScrollBar != nullptr) {
		if (m_dwStyle & UI_WS_VSCROLL) {
			*pdwScrollBar |= WS_VSCROLL;
		}
		if (m_dwStyle & UI_WS_HSCROLL) {
			*pdwScrollBar |= WS_HSCROLL;
		}
		if (m_dwStyle & UI_ES_AUTOVSCROLL) {
			*pdwScrollBar |= ES_AUTOVSCROLL;
		}
		if (m_dwStyle & UI_ES_AUTOHSCROLL) {
			*pdwScrollBar |= ES_AUTOHSCROLL;
		}
		if (m_dwStyle & UI_ES_DISABLENOSCROLL) {
			*pdwScrollBar |= ES_DISABLENOSCROLL;
		}
	}
	return NOERROR;
}

HRESULT RichEditHost::TxGetPasswordChar(TCHAR* pch)
{
	ASSERT(pch != nullptr);
	if (pch == nullptr) {
		return NOERROR;
	}
	*pch = m_chPasswordChar;
	if (!IsPassword()) {
		//未启用密码
		return S_FALSE;
	}
	else {
		//启用密码
		return S_OK;
	}
}

HRESULT RichEditHost::TxGetAcceleratorPos(LONG* pcp)
{
	ASSERT(pcp != nullptr);
	if (pcp != nullptr) {
		//不支持这个功能
		*pcp = -1;
	}
	return S_OK;
}

HRESULT RichEditHost::TxGetExtent(LPSIZEL lpExtent)
{
	ASSERT(lpExtent != nullptr);
	if (lpExtent != nullptr) {
		*lpExtent = m_sizelExtent;
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

	if (m_fRichText) {
		//RichText
		dwProperties |= TXTBIT_RICHTEXT;
	}

	if (m_dwStyle & UI_ES_MULTILINE) {
		//多行文本
		dwProperties |= TXTBIT_MULTILINE;
	}

	if (m_dwStyle & UI_ES_READONLY) {
		//只读模式
		dwProperties |= TXTBIT_READONLY;
	}

	if (m_dwStyle & UI_ES_PASSWORD) {
		//密码模式
		dwProperties |= TXTBIT_USEPASSWORD;
	}

	if (m_bShowPassword) {
		//显示密码
		dwProperties |= TXTBIT_SHOWPASSWORD;
	}

	if (m_bFlashPasswordChar) {
		//先显示字符，然后再显示密码字符
		dwProperties |= TXTBIT_FLASHLASTPASSWORDCHAR;
	}

	if (!(m_dwStyle & UI_ES_NOHIDESEL)) {
		//隐藏选择
		dwProperties |= TXTBIT_HIDESELECTION;
	}

	if (m_fEnableAutoWordSel) {
		//自动选择单词
		dwProperties |= TXTBIT_AUTOWORDSEL;
	}

	if (m_fWordWrap) {
		//自动换行
		dwProperties |= TXTBIT_WORDWRAP;
	}

	if (m_fAllowBeep) {
		//是否允许发出Beep声音
		dwProperties |= TXTBIT_ALLOWBEEP;
	}

	if (m_fSaveSelection) {
		//保存选择：如果 为 TRUE，则当控件处于非活动状态时，应保存所选内容的边界。
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
		SetClientRect(rc);
	}
	if (m_pRichEdit != nullptr) {
		m_pRichEdit->OnTxNotify(iNotify, pv);
	}	
	return S_OK;
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

HRESULT	RichEditHost::TxGetSelectionBarWidth(LONG* plSelBarWidth)
{
	ASSERT(plSelBarWidth != nullptr);
	if (plSelBarWidth) {
		*plSelBarWidth = m_lSelBarWidth;
	}
	return S_OK;
}

void RichEditHost::SetWordWrap(bool bWordWrap)
{
	if (m_fWordWrap != bWordWrap) {
		m_fWordWrap = bWordWrap;
		OnTxPropertyBitsChange(TXTBIT_WORDWRAP, bWordWrap ? TXTBIT_WORDWRAP : 0);
	}
}

bool RichEditHost::IsWordWrap() const
{
	return m_fWordWrap;
}

void RichEditHost::SetEnableAutoWordSel(bool bEnableAutoWordSel)
{
	if (m_fEnableAutoWordSel != bEnableAutoWordSel) {
		m_fEnableAutoWordSel = bEnableAutoWordSel;
		OnTxPropertyBitsChange(TXTBIT_AUTOWORDSEL, bEnableAutoWordSel ? TXTBIT_AUTOWORDSEL : 0);
	}
}

void RichEditHost::SetReadOnly(bool fReadOnly)
{
	bool bOldValue = m_dwStyle & UI_ES_READONLY;
	if (bOldValue != fReadOnly) {
		if (fReadOnly) {
			m_dwStyle |= UI_ES_READONLY;
		}
		else {
			m_dwStyle &= ~UI_ES_READONLY;
		}
		OnTxPropertyBitsChange(TXTBIT_READONLY, fReadOnly ? TXTBIT_READONLY : 0);
	}	
}

bool RichEditHost::IsReadOnly() const
{
	return m_dwStyle & UI_ES_READONLY;
}

void RichEditHost::SetPasswordChar(WCHAR chPasswordChar)
{
	ASSERT(chPasswordChar != L'\0');
	if (chPasswordChar == L'\0') {
		return;
	}
	//启用密码
	m_dwStyle |= UI_ES_PASSWORD;
	m_chPasswordChar = chPasswordChar;
	OnTxPropertyBitsChange(TXTBIT_USEPASSWORD, (m_chPasswordChar != 0) ? TXTBIT_USEPASSWORD : 0);
}

void RichEditHost::SetPassword(bool bPassword)
{
	bool bOldValue = m_dwStyle & UI_ES_PASSWORD;
	if (bOldValue != bPassword) {
		if (bPassword) {
			m_dwStyle |= UI_ES_PASSWORD;
		}
		else {
			m_dwStyle &= ~UI_ES_PASSWORD;
		}
		OnTxPropertyBitsChange(TXTBIT_USEPASSWORD, bPassword ? TXTBIT_USEPASSWORD : 0);
	}	
}

bool RichEditHost::IsPassword() const
{
	return m_dwStyle & UI_ES_PASSWORD;
}

void RichEditHost::SetShowPassword(bool bShow)
{
	if (m_bShowPassword != bShow) {
		m_bShowPassword = bShow;
		OnTxPropertyBitsChange(TXTBIT_SHOWPASSWORD, bShow ? TXTBIT_SHOWPASSWORD : 0);
	}	
}

bool RichEditHost::IsShowPassword() const
{
	return m_bShowPassword;
}

void RichEditHost::SetFlashPasswordChar(bool bFlash)
{
	if (m_bFlashPasswordChar != bFlash) {
		m_bFlashPasswordChar = bFlash;
		OnTxPropertyBitsChange(TXTBIT_FLASHLASTPASSWORDCHAR, bFlash ? TXTBIT_FLASHLASTPASSWORDCHAR : 0);
	}
}

bool RichEditHost::IsFlashPasswordChar() const
{
	return m_bFlashPasswordChar;
}

bool RichEditHost::IsNumberOnly() const
{
	return m_dwStyle & UI_ES_NUMBER;
}

void RichEditHost::SetNumberOnly(bool bNumberOnly)
{
	if (bNumberOnly) {
		m_dwStyle |= UI_ES_NUMBER;
	}
	else {
		m_dwStyle &= ~UI_ES_NUMBER;
	}
}

bool RichEditHost::IsMultiLine() const
{
	return m_dwStyle & UI_ES_MULTILINE;
}

void RichEditHost::SetMultiLine(bool bMultiLine)
{
	if (bMultiLine) {
		m_dwStyle |= UI_ES_MULTILINE;
	}
	else {
		m_dwStyle &= ~UI_ES_MULTILINE;
	}
	OnTxPropertyBitsChange(TXTBIT_MULTILINE, bMultiLine ? TXTBIT_MULTILINE : 0);
}

void RichEditHost::SetHAlignType(HorAlignType alignType)
{
	if (alignType == kHorAlignLeft) {
		m_dwStyle &= ~(UI_ES_CENTER | UI_ES_RIGHT);
		m_dwStyle |= UI_ES_LEFT;
	}
	else if (alignType == kHorAlignCenter) {
		m_dwStyle &= ~(UI_ES_LEFT | UI_ES_RIGHT);
		m_dwStyle |= UI_ES_CENTER;
	}
	else if (alignType == kHorAlignRight) {
		m_dwStyle &= ~(UI_ES_LEFT | UI_ES_CENTER);
		m_dwStyle |= UI_ES_RIGHT;
	}
	;
	if (m_dwStyle & UI_ES_CENTER) {
		m_paraFormat.wAlignment = PFA_CENTER;
	}		
	else if (m_dwStyle & UI_ES_RIGHT) {
		m_paraFormat.wAlignment = PFA_RIGHT;
	}
	else {
		m_paraFormat.wAlignment = PFA_LEFT;
	}
	m_paraFormat.dwMask |= PFM_ALIGNMENT;
	OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, TXTBIT_PARAFORMATCHANGE);
}

void RichEditHost::SetVAlignType(VerAlignType alignType)
{
	if (alignType == kVerAlignTop) {
		m_dwStyle &= ~(UI_ES_VCENTER | UI_ES_BOTTOM);
		m_dwStyle |= UI_ES_TOP;
	}
	else if (alignType == kVerAlignCenter) {
		m_dwStyle &= ~(UI_ES_TOP | UI_ES_BOTTOM);
		m_dwStyle |= UI_ES_VCENTER;
	}
	else if (alignType == kVerAlignBottom) {
		m_dwStyle &= ~(UI_ES_TOP | UI_ES_VCENTER);
		m_dwStyle |= UI_ES_BOTTOM;
	}
	OnTxPropertyBitsChange(TXTBIT_CLIENTRECTCHANGE, TXTBIT_CLIENTRECTCHANGE);
}

void RichEditHost::SetVScrollBar(bool bEnable)
{
	if (bEnable) {
		m_dwStyle |= UI_ES_DISABLENOSCROLL | UI_WS_VSCROLL;
	}
	else {
		m_dwStyle &= ~UI_WS_VSCROLL;
		if (!(m_dwStyle & UI_WS_HSCROLL)) {
			m_dwStyle &= ~UI_ES_DISABLENOSCROLL;
		}
	}
}

void RichEditHost::SetAutoVScroll(bool bEnable)
{
	//当用户在最后一行按 ENTER 时，自动将文本向上滚动一页。
	if (bEnable) {
		m_dwStyle |= UI_ES_AUTOVSCROLL;
	}
	else {
		m_dwStyle &= ~UI_ES_AUTOVSCROLL;
	}
}

void RichEditHost::SetHScrollBar(bool bEnable)
{
	if (bEnable) {
		m_dwStyle |= UI_ES_DISABLENOSCROLL | UI_WS_HSCROLL;
	}
	else {
		m_dwStyle &= ~UI_WS_HSCROLL;
		if (!(m_dwStyle & UI_WS_VSCROLL)) {
			m_dwStyle &= ~UI_ES_DISABLENOSCROLL;
		}
	}
}

void RichEditHost::SetAutoHScroll(bool bEnable)
{
	//当用户在行尾键入一个字符时，自动将文本向右滚动 10 个字符。 当用户按 Enter 时，控件会将所有文本滚动回零位置。
	if (bEnable) {
		m_dwStyle |= UI_ES_AUTOHSCROLL;
	}
	else {
		m_dwStyle &= ~UI_ES_AUTOHSCROLL;
	}
}

void RichEditHost::SetFontId(const std::wstring& fontId)
{
	//fontId不需要判空，如果fontId为空，则使用默认字体
	LOGFONT lf = { 0, };
	GetLogFont(fontId, lf);
	InitCharFormat(lf);
	OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
}

void RichEditHost::SetTextColor(COLORREF dwColor)
{
	if (dwColor != m_charFormat.crTextColor) {
		m_charFormat.dwMask |= CFM_COLOR;
		m_charFormat.crTextColor = dwColor;
		OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
	}
}

COLORREF RichEditHost::GetTextColor() const
{
	return m_charFormat.crTextColor;
}

void RichEditHost::SetExtent(SIZEL sizelExtent)
{
	if ((m_sizelExtent.cx != sizelExtent.cx) || (m_sizelExtent.cy != sizelExtent.cy)) {
		m_sizelExtent = sizelExtent;
		OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
	}	
}

void RichEditHost::SetAllowBeep(bool bAllowBeep)
{
	if (m_fAllowBeep != bAllowBeep) {
		m_fAllowBeep = bAllowBeep;
		OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP, bAllowBeep ? TXTBIT_ALLOWBEEP : 0);
	}
}

bool RichEditHost::IsAllowBeep() const
{
	return m_fAllowBeep;
}

bool RichEditHost::IsRichText() const
{
	return m_fRichText;
}

void RichEditHost::SetRichText(bool fNew)
{
	if (m_fRichText != fNew) {
		m_fRichText = fNew;
		OnTxPropertyBitsChange(TXTBIT_RICHTEXT, fNew ? TXTBIT_RICHTEXT : 0);
	}
}

void RichEditHost::SetClientRect(const UiRect& rc)
{
	if (m_rcClient == rc) {
		return;
	}
	m_rcClient = rc;

	bool bGetDC = false;
	HDC hDC = nullptr;
	if (m_pRichEdit != nullptr) {
		hDC = m_pRichEdit->GetWindowDC();
	}
	if (hDC == nullptr) {
		hDC = ::GetDC(nullptr);
		bGetDC = true;
	}
	LONG xPerInch = ::GetDeviceCaps(hDC, LOGPIXELSX);
	LONG yPerInch = ::GetDeviceCaps(hDC, LOGPIXELSY);
	if (bGetDC && (hDC != nullptr)) {
		::ReleaseDC(nullptr, hDC);
		hDC = nullptr;
	}

	if (xPerInch == 0) {
		xPerInch = 96;
	}
	if (yPerInch == 0) {
		yPerInch = 96;
	}
	SIZEL sizelExtent;
	sizelExtent.cx = DXtoHimetricX(m_rcClient.Width(), xPerInch);
	sizelExtent.cy = DYtoHimetricY(m_rcClient.Height(), yPerInch);
	SetExtent(sizelExtent);
	OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE, TXTBIT_VIEWINSETCHANGE);
}

void RichEditHost::GetControlRect(UiRect* prc)
{
	if ((prc == nullptr) || (m_pTextServices == nullptr) || (m_pRichEdit == nullptr)) {
		return;
	}
	UiRect rc = m_rcClient;
	if ((m_dwStyle & UI_ES_VCENTER) || (m_dwStyle & UI_ES_BOTTOM)) {
		LONG iWidth = rc.Width();
		LONG iHeight = 0;
		SIZEL szExtent = { -1, -1 };
		m_pTextServices->TxGetNaturalSize(DVASPECT_CONTENT,
										  m_pRichEdit->GetWindowDC(),
										  NULL,
										  NULL,
										  TXTNS_FITTOCONTENT,
										  &szExtent,
										  &iWidth,
										  &iHeight);
		if (m_dwStyle & UI_ES_VCENTER) {
			//纵向居中对齐
			int32_t yOffset = (rc.Height() - iHeight) / 2;
			rc.Offset(0, yOffset);
		}
		else if (m_dwStyle & UI_ES_BOTTOM) {
			//纵向底端对齐
			int32_t yOffset = rc.Height() - iHeight;
			rc.Offset(0, yOffset);
		}
	}
	prc->left = rc.left;
	prc->top = rc.top;
	prc->right = rc.right;
	prc->bottom = rc.bottom;
}

bool RichEditHost::SetSaveSelection(bool fSaveSelection)
{
	bool fResult = fSaveSelection;
	if (m_fSaveSelection != fSaveSelection) {
		m_fSaveSelection = fSaveSelection;
		OnTxPropertyBitsChange(TXTBIT_SAVESELECTION, m_fSaveSelection ? TXTBIT_SAVESELECTION : 0);
	}
	return fResult;
}

void RichEditHost::SetHideSelection(bool fHideSelection)
{
	if (!fHideSelection) {
		m_dwStyle |= UI_ES_NOHIDESEL;
	}
	else {
		m_dwStyle &= ~UI_ES_NOHIDESEL;
	}
	OnTxPropertyBitsChange(TXTBIT_HIDESELECTION, fHideSelection ? TXTBIT_HIDESELECTION : 0);
}

HRESULT	RichEditHost::OnTxInPlaceDeactivate()
{
	HRESULT hr = S_OK;
	if (m_pTextServices != nullptr) {
		hr = m_pTextServices->OnTxInPlaceDeactivate();
	}
	if (SUCCEEDED(hr)) {
		m_fInplaceActive = false;
	}
	return hr;
}

HRESULT	RichEditHost::OnTxInPlaceActivate(LPCRECT prcClient)
{
	m_fInplaceActive = true;
	HRESULT hr = E_FAIL;
	if (m_pTextServices != nullptr) {
		hr = m_pTextServices->OnTxInPlaceActivate(prcClient);
	}
	if (FAILED(hr))	{
		m_fInplaceActive = false;
	}

	return hr;
}

bool RichEditHost::SetCursor(const UiRect* prc, const UiPoint* pt)
{
	if ((pt == nullptr) || (m_pRichEdit == nullptr)) {
		return false;
	}

	UiRect rc = (prc != nullptr) ? *prc : m_rcClient;

	// Is this in our rectangle?
	UiPoint newPt(pt->x, pt->y);
	newPt.Offset(m_pRichEdit->GetScrollOffsetInScrollBox());
	if (rc.ContainsPt(newPt)) {
		RECT* pRect = nullptr;
		const UiRect* prcClient = (!m_fInplaceActive || prc) ? &rc : nullptr;
		RECT rect = { 0, };
		if (prcClient != nullptr) {
			rect = { prcClient->left, prcClient->top, prcClient->right, prcClient->bottom };
			pRect = &rect;
		}
		if (m_pTextServices != nullptr) {
			m_pTextServices->OnTxSetCursor(DVASPECT_CONTENT, -1, NULL, NULL, m_pRichEdit->GetWindowDC(),
										   NULL, pRect, newPt.x, newPt.y);
			return true;
		}
	}
	return false;
}

void RichEditHost::SetTransparent(bool fTransparent)
{
	if (m_fTransparent != fTransparent) {
		m_fTransparent = fTransparent;
		OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 0);
	}	
}

void RichEditHost::SetDisabled(bool fOn)
{
	m_charFormat.dwMask |= CFM_COLOR | CFM_DISABLED;
	m_charFormat.dwEffects |= CFE_AUTOCOLOR | CFE_DISABLED;
	if (!fOn) {
		m_charFormat.dwEffects &= ~CFE_DISABLED;
	}
	OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
}

void RichEditHost::SetSelBarWidth(LONG lSelBarWidth)
{
	if (lSelBarWidth < 0) {
		lSelBarWidth = 0;
	}
	if (m_lSelBarWidth != lSelBarWidth) {
		m_lSelBarWidth = lSelBarWidth;
		OnTxPropertyBitsChange(TXTBIT_SELBARCHANGE, TXTBIT_SELBARCHANGE);
	}
}

void RichEditHost::SetCharFormat(const CHARFORMAT2W& c)
{
	//只保存，不通知
	m_charFormat = c;
}

void RichEditHost::SetParaFormat(const PARAFORMAT2& p)
{
	//只保存，不通知
	m_paraFormat = p;
}

void RichEditHost::InitCharFormat(const LOGFONT& lf)
{
	//字体字号需要转换, 否则字体大小显示异常
	bool bGetDC = false;
	HDC hDC = nullptr;
	if (m_pRichEdit != nullptr) {
		hDC = m_pRichEdit->GetWindowDC();
	}
	if (hDC == nullptr) {
		hDC = ::GetDC(nullptr);
		bGetDC = true;
	}
	LONG yPixPerInch = ::GetDeviceCaps(hDC, LOGPIXELSY);
	if (bGetDC && (hDC != nullptr)) {
		::ReleaseDC(nullptr, hDC);
		hDC = nullptr;
	}
	if (yPixPerInch == 0) {
		yPixPerInch = 96;
	}
	LONG lfHeight = lf.lfHeight * LY_PER_INCH / yPixPerInch;

	m_charFormat = {0, };
	m_charFormat.cbSize = sizeof(CHARFORMAT2W);
	m_charFormat.dwMask = CFM_SIZE | CFM_OFFSET | CFM_FACE | CFM_CHARSET | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
	m_charFormat.yHeight = -lfHeight;
	if (lf.lfWeight >= FW_BOLD) {
		m_charFormat.dwEffects |= CFE_BOLD;
	}
	else {
		m_charFormat.dwEffects &= ~CFE_BOLD;
	}
	if (lf.lfItalic) {
		m_charFormat.dwEffects |= CFE_ITALIC;
	}
	else {
		m_charFormat.dwEffects &= ~CFE_ITALIC;
	}
	if (lf.lfUnderline) {
		m_charFormat.dwEffects |= CFE_UNDERLINE;
	}
	else {
		m_charFormat.dwEffects &= ~CFE_UNDERLINE;
	}
	if (lf.lfStrikeOut) {
		m_charFormat.dwEffects |= CFE_STRIKEOUT;
	}
	else {
		m_charFormat.dwEffects &= ~CFE_STRIKEOUT;
	}
	m_charFormat.bCharSet = lf.lfCharSet;
	m_charFormat.bPitchAndFamily = lf.lfPitchAndFamily;
	wcscpy_s(m_charFormat.szFaceName, lf.lfFaceName);
}

UiRect RichEditHost::MakeUiRect(const RECT& rc)
{
	return UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

LONG RichEditHost::DXtoHimetricX(LONG dx, LONG xPerInch)
{
	return (LONG)MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
}

LONG RichEditHost::DYtoHimetricY(LONG dy, LONG yPerInch)
{
	return (LONG)MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
}

}//name space ui
