#include "RichEditHost_Windows.h"
#include "RichEdit_Windows.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringConvert.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include <TextServ.h>

#define UI_ES_LEFT              0x0001L
#define UI_ES_CENTER            0x0002L
#define UI_ES_RIGHT             0x0004L
#define UI_ES_TOP               0x0008L
#define UI_ES_VCENTER           0x0010L
#define UI_ES_BOTTOM            0x0020L
#define UI_ES_MULTILINE         0x0040L
#define UI_ES_NUMBER            0x0080L
#define UI_ES_PASSWORD          0x0100L
#define UI_ES_READONLY          0x0200L
#define UI_ES_NOHIDESEL         0x0400L
#define UI_ES_AUTOHSCROLL       0x0800L
#define UI_ES_AUTOVSCROLL       0x1000L
#define UI_ES_DISABLENOSCROLL   0x2000L
#define UI_WS_HSCROLL           0x4000L
#define UI_WS_VSCROLL           0x8000L

#if defined (DUILIB_COMPILER_MINGW)
    typedef HRESULT(STDAPICALLTYPE* PShutdownTextServices)(IUnknown* pTextServices);
    #define TXTBIT_SHOWPASSWORD		        0x800000	// Show password string
    #define TXTBIT_FLASHLASTPASSWORDCHAR    0x10000000	// Show last password char momentarily
#endif

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

/** RichEdit控件的DLL模块管理
*/
class RichEditModule
{
private:
    RichEditModule():
        m_hRichEditModule(nullptr)
    {
    }

    ~RichEditModule()
    {
        ASSERT(m_hRichEditModule == nullptr);
    }

    RichEditModule(const RichEditModule&) = delete;
    RichEditModule& operator = (const RichEditModule&) = delete;

public:
    /** 获取单例对象
    */
    static RichEditModule& Instance()
    {
        static RichEditModule self;
        return self;
    }

    /** RichEdit依赖的DLL, 加载并返回句柄
    */
    HMODULE GetRichEditModule()
    {
        if (m_hRichEditModule == nullptr) {         
            m_hRichEditModule = ::LoadLibrary(RichEditCtrl::GetLibraryName());
            ASSERT(m_hRichEditModule != nullptr);

            //退出时，清理资源
            auto atExitCallback = []() {
                RichEditModule::Instance().Clear();
                return true;
                };
            GlobalManager::Instance().AddAtExitFunction(atExitCallback);
        }
        return m_hRichEditModule;
    }

    /** 清理资源
    */
    void Clear()
    {
        if (m_hRichEditModule != nullptr) {
            ::FreeLibrary(m_hRichEditModule);
            m_hRichEditModule = nullptr;
        }
    }

private:
    /** RichEdit依赖的DLL, 加载并返回句柄
    */
    HMODULE m_hRichEditModule;
};

RichEditHost::RichEditHost(RichEdit* pRichEdit) :
    m_pRichEdit(pRichEdit),
    m_cRefs(1),
    m_pTextServices(nullptr),
    m_dwStyle(0),
    m_fEnableAutoWordSel(false),
    m_fWordWrap(false),
    m_fAllowBeep(false),
    m_fSaveSelection(false),
    m_fInplaceActive(false),
    m_fTransparent(false),
    m_lSelBarWidth(0),
    m_rcClient(),
    m_sizelExtent({ 0 }),
    m_chPasswordChar(0),
    m_bShowPassword(false),
    m_bFlashPasswordChar(false)
{
    Init();
}

RichEditHost::~RichEditHost()
{
    ASSERT(m_pTextServices == nullptr);
}

void RichEditHost::Init()
{
    IUnknown* pUnk = nullptr;

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

    m_fInplaceActive = true;

    PCreateTextServices pfnTextServicesProc = nullptr;
    HMODULE hRichEditModule = RichEditModule::Instance().GetRichEditModule();
    if (hRichEditModule != nullptr) {
        pfnTextServicesProc = (PCreateTextServices)::GetProcAddress(hRichEditModule, "CreateTextServices");
    }

    if (pfnTextServicesProc) {
        pfnTextServicesProc(nullptr, this, &pUnk);
    }

    ASSERT(m_pTextServices == nullptr);
    if (pUnk != nullptr) {
        pUnk->QueryInterface(IID_ITextServices, (void**)&m_pTextServices);
        pUnk->Release();
    }

    ASSERT(m_pTextServices != nullptr);
    if (m_pTextServices != nullptr) {
        //设置默认属性
        SetTransparent(TRUE);
        LRESULT lResult = 0;
        m_pTextServices->TxSendMessage(EM_SETLANGOPTIONS, 0, 0, &lResult);
        m_pTextServices->TxSendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_LINK, &lResult);
        m_pTextServices->OnTxInPlaceActivate(nullptr);
    }
}

ITextServices* RichEditHost::GetTextServices() const
{ 
    return m_pTextServices; 
}

void RichEditHost::ShutdownTextServices()
{
    if (m_pTextServices != nullptr) {
        PShutdownTextServices pfnShutdownTextServicesProc = nullptr;
        HMODULE hRichEditModule = RichEditModule::Instance().GetRichEditModule();
        if (hRichEditModule != nullptr) {
            pfnShutdownTextServicesProc = (PShutdownTextServices)::GetProcAddress(hRichEditModule, "ShutdownTextServices");
        }
        if (pfnShutdownTextServicesProc != nullptr) {
            IUnknown* pUnk = nullptr;
            m_pTextServices->QueryInterface(IID_IUnknown, (void**)&pUnk);
            if (pUnk != nullptr) {
                HRESULT hr = pfnShutdownTextServicesProc(pUnk);
                ASSERT_UNUSED_VARIABLE(hr == S_OK);
            }
        }
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

/////////////////////////////////  IUnknown ////////////////////////////////
HRESULT RichEditHost::QueryInterface(REFIID riid, void** ppvObject)
{
    if (ppvObject == nullptr) {
        return E_INVALIDARG;
    }
    HRESULT hr = E_NOINTERFACE;
    *ppvObject = nullptr;

    if (IsEqualIID(riid, IID_IUnknown)) {
        AddRef();
        *ppvObject = (IUnknown*)this;
        hr = S_OK;
    }
    else if (IsEqualIID(riid, IID_ITextHost)) {
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
        return m_pRichEdit->GetDrawDC();
    }
    return nullptr;
}

int RichEditHost::TxReleaseDC(HDC /*hdc*/)
{
    return 1;
}

BOOL RichEditHost::TxShowScrollBar(INT /*fnBar*/, BOOL /*fShow*/)
{
    //ASSERT(FALSE);
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
            pVScrollBar->SetScrollRange((int64_t)nMaxPos - nMinPos - m_rcClient.Height());
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
            pHScrollBar->SetScrollRange((int64_t)nMaxPos - nMinPos - m_rcClient.Width());
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

    //标记窗口指定区域为脏区域，进行重绘(取控件位置部分，避免引发其他控件的重绘)
    UiRect rcRichEdit = m_pRichEdit->GetRect();
    rcRichEdit.Offset(-scrollOffset.x, -scrollOffset.y);
    rc.Intersect(rcRichEdit);
    pWindow->Invalidate(rc);
}

void RichEditHost::TxViewChange(BOOL /*fUpdate*/)
{
}

BOOL RichEditHost::TxCreateCaret(HBITMAP /*hbmp*/, INT xWidth, INT yHeight)
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->CreateCaret(xWidth, yHeight);
    }    
    return TRUE;
}

BOOL RichEditHost::TxShowCaret(BOOL /*fShow*/)
{
    return TRUE;
}

BOOL RichEditHost::TxSetCaretPos(INT x, INT y)
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetCaretPos(x, y);
    }
    return TRUE;
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
    return ::ScreenToClient(m_pRichEdit->GetWindowHWND(), lppt);
}

BOOL RichEditHost::TxClientToScreen(LPPOINT lppt)
{
    if (m_pRichEdit == nullptr) {
        return FALSE;
    }
    return ::ClientToScreen(m_pRichEdit->GetWindowHWND(), lppt);
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

HRESULT RichEditHost::TxGetCharFormat(const CHARFORMATW** /*ppCF*/)
{
    return E_NOTIMPL;
}

HRESULT RichEditHost::TxGetParaFormat(const PARAFORMAT** /*ppPF*/)
{
    return E_NOTIMPL;
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

HRESULT RichEditHost::TxGetPasswordChar(_Out_ TCHAR* pch)
{
    ASSERT(pch != nullptr);
    if (pch == nullptr) {
        return NOERROR;
    }
#ifndef DUILIB_UNICODE
    ASSERT(m_chPasswordChar <= CHAR_MAX);
#endif // !DUILIB_UNICODE
    *pch = (TCHAR)m_chPasswordChar;
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
    return nullptr;
}

void RichEditHost::TxImmReleaseContext(HIMC /*himc*/)
{
    //::ImmReleaseContext( hwnd, himc );
}

HRESULT RichEditHost::TxGetSelectionBarWidth(LONG* plSelBarWidth)
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
    ASSERT(chPasswordChar != _T('\0'));
    if (chPasswordChar == _T('\0')) {
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

DString RichEditHost::GetPasswordText() const
{
    DString pwdText;
    if (IsPassword() && (m_pTextServices != nullptr)) {        
        ITextServices* pTextServices = m_pTextServices;
        BSTR bstrText = nullptr;
        HRESULT hr = pTextServices->TxGetText(&bstrText);
        if ((hr == S_OK) && (bstrText != nullptr)) {
            std::wstring pwdTextW(bstrText, SysStringLen(bstrText));
            ::SysFreeString(bstrText);
            pwdText = StringConvert::WStringToT(pwdTextW);
        }        
    }
    return pwdText;
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

void RichEditHost::SetClientRect(const UiRect& rc)
{
    if (m_rcClient == rc) {
        return;
    }
    m_rcClient = rc;
    ASSERT(m_pRichEdit != nullptr);
    if (m_pRichEdit == nullptr) {
        return;
    }
    HDC hDC = m_pRichEdit->GetDrawDC();
    LONG xPerInch = ::GetDeviceCaps(hDC, LOGPIXELSX);
    LONG yPerInch = ::GetDeviceCaps(hDC, LOGPIXELSY);
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
    if ((prc == nullptr) || (m_pRichEdit == nullptr)) {
        return;
    }
    UiRect rc = m_rcClient;
    if ((m_dwStyle & UI_ES_VCENTER) || (m_dwStyle & UI_ES_BOTTOM)) {        
        UiSize szNaturalSize = m_pRichEdit->GetNaturalSize(rc.Width(), 0);
        if (m_dwStyle & UI_ES_VCENTER) {
            //纵向居中对齐
            int32_t yOffset = (rc.Height() - szNaturalSize.cy) / 2;
            rc.Offset(0, yOffset);
        }
        else if (m_dwStyle & UI_ES_BOTTOM) {
            //纵向底端对齐
            int32_t yOffset = rc.Height() - szNaturalSize.cy;
            rc.Offset(0, yOffset);
        }
    }
    prc->left = rc.left;
    prc->top = rc.top;
    prc->right = rc.right;
    prc->bottom = rc.bottom;
}

UiRect RichEditHost::GetControlRect()
{
    UiRect rc;
    GetControlRect(&rc);
    return rc;
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
    if (fHideSelection) {
        ASSERT(IsHideSelection());
    }
    else {
        ASSERT(!IsHideSelection());
    }
}

bool RichEditHost::IsHideSelection() const
{
    return !(m_dwStyle & UI_ES_NOHIDESEL);
}

HRESULT RichEditHost::OnTxInPlaceDeactivate()
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

HRESULT RichEditHost::OnTxInPlaceActivate(LPCRECT prcClient)
{
    m_fInplaceActive = true;
    HRESULT hr = E_FAIL;
    if (m_pTextServices != nullptr) {
        hr = m_pTextServices->OnTxInPlaceActivate(prcClient);
    }
    if (FAILED(hr))    {
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
            m_pTextServices->OnTxSetCursor(DVASPECT_CONTENT, -1, nullptr, nullptr, m_pRichEdit->GetDrawDC(),
                                           nullptr, pRect, newPt.x, newPt.y);
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

#endif //DUILIB_BUILD_FOR_WIN
