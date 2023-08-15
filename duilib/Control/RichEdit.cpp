#include "RichEdit.h"
#include "RichEditHost.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Utils/OnScreenKeyboardManager.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "base/win32/path_util.h"
#include "base/thread/thread_manager.h"

namespace ui {

RichEdit::RichEdit() : 
	ScrollBox(new Layout),
	m_pTwh(nullptr), 
	m_bVScrollBarFixing(false), 
	m_bWantTab(true),
	m_bNeedReturnMsg(false),
	m_bReturnMsgWantCtrl(false),
	m_bRich(false),
	m_bReadOnly(false),
	m_bPassword(false),
	m_bWordWrap(false),
	m_bNumberOnly(false),
	m_bInited(false),
	m_bAllowPrompt(false),
	m_bSelAllEver(false),		 
	m_bNoSelOnKillFocus(true), 
	m_bSelAllOnFocus(false),	
	m_bNoCaretReadonly(false),
	m_bIsCaretVisiable(false),
	m_bIsComposition(false),
	m_iCaretPosX(0),
	m_iCaretPosY(0),
	m_iCaretWidth(0),
	m_iCaretHeight(0),
	m_sFontId(),
	m_iLimitText(0),
	m_lTwhStyle(ES_MULTILINE),
	m_textVerAlignType(kVerAlignTop),
	m_sCurrentColor(),
	m_sTextColor(),
	m_sDisabledTextColor(),
	m_sPromptColor(),
	m_sText(),
	m_sPromptText(),
	m_drawCaretFlag(),
	m_timeFlagMap(),
	m_linkInfo(),
	m_pFocusedImage(nullptr),
	m_bAutoDetect(false)
{
	m_iLimitText = RichEditHost::GetDefaultMaxText();
	m_sCurrentColor = GlobalManager::Instance().Color().GetDefaultTextColor();
	m_sTextColor = m_sCurrentColor;
	m_sDisabledTextColor = m_sCurrentColor;

	//这个标记必须为false，否则绘制有问题
	SetUseCache(false);
}

RichEdit::~RichEdit()
{
    if( m_pTwh ) {
        m_pTwh->Release();
    }
	if (m_pFocusedImage != nullptr) {
		delete m_pFocusedImage;
		m_pFocusedImage = nullptr;
	}
}

bool RichEdit::IsWantTab() const
{
    return m_bWantTab;
}

void RichEdit::SetWantTab(bool bWantTab)
{
    m_bWantTab = bWantTab;
}

bool RichEdit::CanPlaceCaptionBar() const
{
	return true;
}

bool RichEdit::IsNeedReturnMsg()
{
    return m_bNeedReturnMsg;
}

void RichEdit::SetNeedReturnMsg(bool bNeedReturnMsg)
{
    m_bNeedReturnMsg = bNeedReturnMsg;
}

bool RichEdit::IsReturnMsgWantCtrl()
{
    return m_bReturnMsgWantCtrl;
}

void RichEdit::SetReturnMsgWantCtrl(bool bReturnMsgWantCtrl)
{
    m_bReturnMsgWantCtrl = bReturnMsgWantCtrl;
}

bool RichEdit::IsRich()
{
    return m_bRich;
}

void RichEdit::SetRich(bool bRich)
{
    m_bRich = bRich;
    if( m_pTwh ) m_pTwh->SetRichTextFlag(bRich);
}

bool RichEdit::IsReadOnly()
{
    return m_bReadOnly;
}

void RichEdit::SetReadOnly(bool bReadOnly)
{
    m_bReadOnly = bReadOnly;
    if( m_pTwh ) m_pTwh->SetReadOnly(bReadOnly);
}

bool RichEdit::IsPassword()
{
	return m_bPassword;
}

void RichEdit::SetPassword( bool bPassword )
{
	m_bPassword = bPassword;
	if( m_pTwh ) m_pTwh->SetPassword(bPassword);
}

bool RichEdit::GetWordWrap()
{
    return m_bWordWrap;
}

void RichEdit::SetWordWrap(bool bWordWrap)
{
    m_bWordWrap = bWordWrap;
    if( m_pTwh ) m_pTwh->SetWordWrap(bWordWrap);
}

std::wstring RichEdit::GetFontId() const
{
    return m_sFontId.c_str();
}

void RichEdit::SetFontId(const std::wstring& strFontId)
{
    m_sFontId = strFontId;
    if( m_pTwh ) {
		m_pTwh->SetFontId(strFontId);
    }
}

LONG RichEdit::GetWinStyle()
{
    return m_lTwhStyle;
}

void RichEdit::SetWinStyle(LONG lStyle)
{
    m_lTwhStyle = lStyle;
}

ui::VerAlignType RichEdit::GetTextVerAlignType()
{
	return m_textVerAlignType;
}

void RichEdit::SetTextColor(const std::wstring& dwTextColor)
{
	if(m_sCurrentColor == dwTextColor)
		return;
	m_sCurrentColor = dwTextColor;

	UiColor dwTextColor2 = this->GetUiColor(dwTextColor);
    if( m_pTwh ) {
        m_pTwh->SetColor(dwTextColor2.GetARGB());
    }
}

void RichEdit::SetTextColor(UiColor color)
{
	if (m_pTwh) {
		m_pTwh->SetColor(color.GetARGB());
	}
}

std::wstring RichEdit::GetTextColor()
{
	return m_sCurrentColor.c_str();
}

UiColor RichEdit::GetTextColorValue()
{
	return this->GetUiColor(m_sCurrentColor.c_str());
}

int RichEdit::GetLimitText()
{
    return m_iLimitText;
}

void RichEdit::SetLimitText(int iChars)
{
    m_iLimitText = iChars;
    if( m_pTwh ) {
        m_pTwh->LimitText(m_iLimitText);
    }
}

long RichEdit::GetTextLength(DWORD dwFlags) const
{
    GETTEXTLENGTHEX textLenEx;
    textLenEx.flags = dwFlags;
#ifdef _UNICODE
    textLenEx.codepage = 1200;
#else
    textLenEx.codepage = CP_ACP;
#endif
    LRESULT lResult;
    TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)&textLenEx, 0, &lResult);
    return (long)lResult;
}

std::wstring RichEdit::GetText() const
{
    long lLen = GetTextLength(GTL_DEFAULT);
    LPTSTR lpText = NULL;
    GETTEXTEX gt;
    gt.flags = GT_DEFAULT;
#ifdef _UNICODE
    gt.cb = sizeof(TCHAR) * (lLen + 1) ;
    gt.codepage = 1200;
    lpText = new TCHAR[lLen + 1];
    ::ZeroMemory(lpText, (lLen + 1) * sizeof(TCHAR));
#else
    gt.cb = sizeof(TCHAR) * lLen * 2 + 1;
    gt.codepage = CP_ACP;
    lpText = new TCHAR[lLen * 2 + 1];
    ::ZeroMemory(lpText, (lLen * 2 + 1) * sizeof(TCHAR));
#endif
    gt.lpDefaultChar = NULL;
    gt.lpUsedDefChar = NULL;
    TxSendMessage(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)lpText, 0);
    std::wstring sText(lpText);
    delete[] lpText;
    return sText;
}

std::string RichEdit::GetUTF8Text() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(GetText(), strOut, CP_UTF8);
	return strOut;
}

void RichEdit::SetText(const std::wstring& strText)
{
	m_sText = strText;
	if( !m_bInited )
		return;

    SetSel(0, -1);

    ReplaceSel(strText, FALSE);

	m_linkInfo.clear();
}

void RichEdit::SetTextId(const std::wstring& strTextId)
{
	std::wstring strText = GlobalManager::Instance().Lang().GetStringViaID(strTextId);
	SetText(strText);
}

void RichEdit::SetUTF8Text( const std::string& strText )
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strText, strOut, CP_UTF8);
	SetText(strOut);
}

bool RichEdit::GetModify() const
{ 
    if( !m_pTwh ) return false;
    LRESULT lResult;
    TxSendMessage(EM_GETMODIFY, 0, 0, &lResult);
    return (BOOL)lResult == TRUE;
}

void RichEdit::SetModify(bool bModified) const
{ 
    TxSendMessage(EM_SETMODIFY, bModified, 0, 0);
}

void RichEdit::GetSel(CHARRANGE &cr) const
{ 
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0); 
}

void RichEdit::GetSel(long& nStartChar, long& nEndChar) const
{
    CHARRANGE cr;
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0); 
    nStartChar = cr.cpMin;
    nEndChar = cr.cpMax;
}

int RichEdit::SetSel(CHARRANGE &cr)
{ 
    LRESULT lResult;
    TxSendMessage(EM_EXSETSEL, 0, (LPARAM)&cr, &lResult); 
    return (int)lResult;
}

int RichEdit::SetSel(long nStartChar, long nEndChar)
{
    CHARRANGE cr;
    cr.cpMin = nStartChar;
    cr.cpMax = nEndChar;
    LRESULT lResult;
    TxSendMessage(EM_EXSETSEL, 0, (LPARAM)&cr, &lResult); 
    return (int)lResult;
}

void RichEdit::ReplaceSel(const std::wstring& lpszNewText, bool bCanUndo)
{
    TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpszNewText.c_str(), 0); 
}

std::wstring RichEdit::GetSelText() const
{
	if (m_pTwh == nullptr) {
		return std::wstring();
	}
	CHARRANGE cr = { 0, };
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0);
	const int32_t nLen = cr.cpMax - cr.cpMin + 1;
	if (nLen < 1) {
		return std::wstring();
	}
    LPWSTR lpText = new WCHAR[nLen];
    ::ZeroMemory(lpText, nLen * sizeof(WCHAR));
    TxSendMessage(EM_GETSELTEXT, 0, (LPARAM)lpText, 0);
    std::wstring sText = lpText;
    delete[] lpText;
    return sText;
}

int RichEdit::SetSelAll()
{
    return SetSel(0, -1);
}

int RichEdit::SetSelNone()
{
    return SetSel(-1, 0);
}

bool RichEdit::GetZoom(int& nNum, int& nDen) const
{
    LRESULT lResult;
    TxSendMessage(EM_GETZOOM, (WPARAM)&nNum, (LPARAM)&nDen, &lResult);
    return (BOOL)lResult == TRUE;
}

bool RichEdit::SetZoom(int nNum, int nDen)
{
    if (nNum < 0 || nNum > 64) return false;
    if (nDen < 0 || nDen > 64) return false;
    LRESULT lResult;
    TxSendMessage(EM_SETZOOM, nNum, nDen, &lResult);
    return (BOOL)lResult == TRUE;
}

bool RichEdit::SetZoomOff()
{
    LRESULT lResult;
    TxSendMessage(EM_SETZOOM, 0, 0, &lResult);
    return (BOOL)lResult == TRUE;
}

WORD RichEdit::GetSelectionType() const
{
    LRESULT lResult;
    TxSendMessage(EM_SELECTIONTYPE, 0, 0, &lResult);
    return (WORD)lResult;
}

bool RichEdit::GetAutoURLDetect() const
{
    LRESULT lResult;
    TxSendMessage(EM_GETAUTOURLDETECT, 0, 0, &lResult);
    return (BOOL)lResult == TRUE;
}

bool RichEdit::SetAutoURLDetect(bool bAutoDetect)
{
	m_bAutoDetect = bAutoDetect;
	if (m_pTwh == nullptr) {
		return true;
	}
	if (bAutoDetect) {
		ASSERT(GetEventMask() & ENM_LINK);
	}
    LRESULT lResult;
    TxSendMessage(EM_AUTOURLDETECT, AURL_ENABLEURL, 0, &lResult);
    return (BOOL)lResult == FALSE;
}

DWORD RichEdit::GetEventMask() const
{
    LRESULT lResult;
    TxSendMessage(EM_GETEVENTMASK, 0, 0, &lResult);
    return (DWORD)lResult;
}

DWORD RichEdit::SetEventMask(DWORD dwEventMask)
{
    LRESULT lResult;
    TxSendMessage(EM_SETEVENTMASK, 0, dwEventMask, &lResult);
    return (DWORD)lResult;
}

std::wstring RichEdit::GetTextRange(long nStartChar, long nEndChar) const
{
    TEXTRANGEW tr = { 0 };
    tr.chrg.cpMin = nStartChar;
    tr.chrg.cpMax = nEndChar;
    LPWSTR lpText = NULL;
    lpText = new WCHAR[nEndChar - nStartChar + 1];
    ::ZeroMemory(lpText, (nEndChar - nStartChar + 1) * sizeof(WCHAR));
    tr.lpstrText = lpText;
    TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&tr, 0);
    std::wstring sText;
    sText = (LPCWSTR)lpText;
    delete[] lpText;
    return sText;
}

void RichEdit::HideSelection(bool bHide, bool bChangeStyle)
{
    TxSendMessage(EM_HIDESELECTION, bHide, bChangeStyle, 0);
}

void RichEdit::ScrollCaret()
{
    TxSendMessage(EM_SCROLLCARET, 0, 0, 0);
}

int RichEdit::InsertText(long nInsertAfterChar, LPCTSTR lpstrText, bool bCanUndo)
{
    int nRet = SetSel(nInsertAfterChar, nInsertAfterChar);

    ReplaceSel(lpstrText, bCanUndo);
   
	return nRet;
}

int RichEdit::AppendText(const std::wstring& strText, bool bCanUndo)
{
    int nRet = SetSel(-1, -1);
   
	ReplaceSel(strText, bCanUndo);

	return nRet;
}

DWORD RichEdit::GetDefaultCharFormat(CHARFORMAT2 &cf) const
{
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETCHARFORMAT, 0, (LPARAM)&cf, &lResult);
    return (DWORD)lResult;
}

bool RichEdit::SetDefaultCharFormat(CHARFORMAT2 &cf)
{
    if( !m_pTwh ) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, 0, (LPARAM)&cf, &lResult);
    if( (BOOL)lResult == TRUE ) {
        CHARFORMAT2W cfw;
        cfw.cbSize = sizeof(CHARFORMAT2W);
        TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM)&cfw, 0);
        m_pTwh->SetCharFormat(cfw);
        return true;
    }
    return false;
}

DWORD RichEdit::GetSelectionCharFormat(CHARFORMAT2 &cf) const
{
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM)&cf, &lResult);
    return (DWORD)lResult;
}

bool RichEdit::SetSelectionCharFormat(CHARFORMAT2 &cf)
{
    if( !m_pTwh ) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, &lResult);
    return (BOOL)lResult == TRUE;
}

bool RichEdit::SetWordCharFormat(CHARFORMAT2 &cf)
{
    if( !m_pTwh ) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION|SCF_WORD, (LPARAM)&cf, &lResult); 
    return (BOOL)lResult == TRUE;
}

DWORD RichEdit::GetParaFormat(PARAFORMAT2 &pf) const
{
    pf.cbSize = sizeof(PARAFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETPARAFORMAT, 0, (LPARAM)&pf, &lResult);
    return (DWORD)lResult;
}

bool RichEdit::SetParaFormat(PARAFORMAT2 &pf)
{
    if( !m_pTwh ) return false;
    pf.cbSize = sizeof(PARAFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf, &lResult);
    if( (BOOL)lResult == TRUE ) {
        m_pTwh->SetParaFormat(pf);
        return true;
    }
    return false;
}

bool RichEdit::Redo()
{ 
    if( !m_pTwh ) return false;
    LRESULT lResult;
    TxSendMessage(EM_REDO, 0, 0, &lResult);
    return (BOOL)lResult == TRUE; 
}

bool RichEdit::Undo()
{ 
    if( !m_pTwh ) return false;
    LRESULT lResult;
    TxSendMessage(EM_UNDO, 0, 0, &lResult);
    return (BOOL)lResult == TRUE; 
}

void RichEdit::Clear()
{ 
    TxSendMessage(WM_CLEAR, 0, 0, 0); 
}

void RichEdit::Copy()
{ 
    TxSendMessage(WM_COPY, 0, 0, 0); 
}

void RichEdit::Cut()
{ 
    TxSendMessage(WM_CUT, 0, 0, 0); 
}

void RichEdit::Paste()
{ 
    TxSendMessage(WM_PASTE, 0, 0, 0); 
}

BOOL RichEdit::CanPaste(UINT nFormat/* = 0*/)
{
	if (NULL == m_pTwh)
		return FALSE;
	return m_pTwh->CanPaste(nFormat);
}

void RichEdit::PasteSpecial(UINT uClipFormat, DWORD dwAspect/* = 0*/, HMETAFILE hMF/* = 0*/)
{
	if (NULL == m_pTwh)
		return;
	m_pTwh->PasteSpecial(uClipFormat, dwAspect, hMF);
}

int RichEdit::GetLineCount() const
{ 
    if( !m_pTwh ) return 0;
    LRESULT lResult;
    TxSendMessage(EM_GETLINECOUNT, 0, 0, &lResult);
    return (int)lResult; 
}

std::wstring RichEdit::GetLine(int nIndex, int nMaxLength) const
{
    LPWSTR lpText = NULL;
    lpText = new WCHAR[nMaxLength + 1];
    ::ZeroMemory(lpText, (nMaxLength + 1) * sizeof(WCHAR));
    *(LPWORD)lpText = (WORD)nMaxLength;
    TxSendMessage(EM_GETLINE, nIndex, (LPARAM)lpText, 0);
    std::wstring sText;
    sText = (LPCWSTR)lpText;
    delete[] lpText;
    return sText;
}

int RichEdit::LineIndex(int nLine) const
{
    LRESULT lResult;
    TxSendMessage(EM_LINEINDEX, nLine, 0, &lResult);
    return (int)lResult;
}

int RichEdit::LineLength(int nLine) const
{
    LRESULT lResult;
    TxSendMessage(EM_LINELENGTH, nLine, 0, &lResult);
    return (int)lResult;
}

bool RichEdit::LineScroll(int nLines, int nChars)
{
    LRESULT lResult;
    TxSendMessage(EM_LINESCROLL, nChars, nLines, &lResult);
    return (BOOL)lResult == TRUE;
}

UiPoint RichEdit::GetCharPos(long lChar) const
{ 
    UiPoint pt; 
    TxSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)lChar, 0); 
    return pt;
}

long RichEdit::LineFromChar(long nIndex) const
{ 
    if( !m_pTwh ) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_EXLINEFROMCHAR, 0, nIndex, &lResult);
    return (long)lResult;
}

UiPoint RichEdit::PosFromChar(UINT nChar) const
{ 
	POINTL pt = { 0 };
    TxSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, nChar, 0); 
    return UiPoint(pt.x, pt.y); 
}

int RichEdit::CharFromPos(UiPoint pt) const
{ 
    POINTL ptl = {pt.x, pt.y}; 
    if( !m_pTwh ) return 0;
    LRESULT lResult;
    TxSendMessage(EM_CHARFROMPOS, 0, (LPARAM)&ptl, &lResult);
    return (int)lResult; 
}

void RichEdit::EmptyUndoBuffer()
{ 
    TxSendMessage(EM_EMPTYUNDOBUFFER, 0, 0, 0); 
}

UINT RichEdit::SetUndoLimit(UINT nLimit)
{ 
    if( !m_pTwh ) return 0;
    LRESULT lResult;
    TxSendMessage(EM_SETUNDOLIMIT, (WPARAM) nLimit, 0, &lResult);
    return (UINT)lResult; 
}

long RichEdit::StreamIn(int nFormat, EDITSTREAM &es)
{ 
    if( !m_pTwh ) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_STREAMIN, nFormat, (LPARAM)&es, &lResult);
    return (long)lResult;
}

long RichEdit::StreamOut(int nFormat, EDITSTREAM &es)
{ 
    if( !m_pTwh ) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_STREAMOUT, nFormat, (LPARAM)&es, &lResult);
    return (long)lResult; 
}

HRESULT RichEdit::TxSendMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plresult) const
{
    if( m_pTwh ) {
		HRESULT lr =  m_pTwh->GetTextServices()->TxSendMessage(msg, wParam, lParam, plresult);
		return lr;
    }
    return S_FALSE;
}

IDropTarget* RichEdit::GetTxDropTarget()
{
    IDropTarget *pdt = NULL;
    if( m_pTwh->GetTextServices()->TxGetDropTarget(&pdt) == NOERROR ) return pdt;
    return NULL;
}

bool RichEdit::SetDropAcceptFile(bool /*bAccept*/)
{
	LRESULT lResult = 0;
	TxSendMessage(EM_SETEVENTMASK, 0, ENM_DROPFILES | ENM_LINK, // ENM_CHANGE| ENM_CORRECTTEXT | ENM_DRAGDROPDONE | ENM_DROPFILES | ENM_IMECHANGE | ENM_LINK | ENM_OBJECTPOSITIONS | ENM_PROTECTED | ENM_REQUESTRESIZE | ENM_SCROLL | ENM_SELCHANGE | ENM_UPDATE,
		&lResult);
	return (BOOL)lResult == FALSE;
}

void RichEdit::OnTxNotify(DWORD iNotify, void *pv)
{
	switch(iNotify)
	{ 
	case EN_LINK:   
		{
			NMHDR* hdr = (NMHDR*) pv;
			ENLINK* link = (ENLINK*)hdr;

			if((link != nullptr) && (link->msg == WM_LBUTTONUP)) {
				CHARRANGE oldSel = {0, 0};
				GetSel(oldSel);
				SetSel(link->chrg);
				std::wstring url = GetSelText();
				SetSel(oldSel);
				if (!url.empty()) {
					this->SendEvent(kEventCustomLinkClick, (WPARAM)url.c_str());
				}
			}
		}
		break;
	case EN_CHANGE:
		OnTxTextChanged();
		break;
	case EN_DROPFILES:   
	case EN_MSGFILTER:   
	case EN_OLEOPFAILED:    
	case EN_PROTECTED:
	case EN_SAVECLIPBOARD:   
	case EN_SELCHANGE:   
	case EN_STOPNOUNDO:   
	case EN_OBJECTPOSITIONS:   
	case EN_DRAGDROPDONE:   
		{
			if (pv) {   // Fill out NMHDR portion of pv   
				LONG nId =  GetWindowLong(this->GetWindowHandle(), GWL_ID);   
				NMHDR  *phdr = (NMHDR *)pv;   
				phdr->hwndFrom = this->GetWindowHandle();   
				phdr->idFrom = nId;   
				phdr->code = iNotify;  

				SendMessage(this->GetWindowHandle(), WM_NOTIFY, (WPARAM)nId, (LPARAM)pv);
			}    
		}
		break;
	}
}

bool RichEdit::OnTxTextChanged()
{
	SendEvent(kEventTextChange);
	return true;
}
//
//ITextHost* RichEdit::GetTextHost()
//{
//	if (NULL == m_pTwh)
//		return NULL;
//	return m_pTwh->GetTextHost();
//}
//
//ITextServices* RichEdit::GetTextServices()
//{
//	if (NULL == m_pTwh)
//		return NULL;
//	return m_pTwh->GetTextServices2();
//}

HWND RichEdit::GetWindowHandle()
{
	auto window = this->GetWindow();
	return window ? window->GetHWND() : NULL;
}

HDC RichEdit::GetWindowDC()
{
	auto window = this->GetWindow();
	return window ? window->GetPaintDC() : NULL;
}
//
//BOOL RichEdit::SetOleCallback(IRichEditOleCallback* pCallback)
//{
//	if (NULL == m_pTwh)
//		return FALSE;
//	return m_pTwh->SetOleCallback(pCallback);
//}

UiSize RichEdit::GetNaturalSize(LONG width, LONG height)
{
	if (width < 0) {
		width = 0;
	}
	if (height < 0) {
		height = 0;
	}	
	UiSize sz(0,0);
	if (m_cbGetNaturalSize != nullptr && m_cbGetNaturalSize(width, height, sz))
		return sz;
	LONG lWidth = width;
	LONG lHeight = height;
	SIZEL szExtent = { -1, -1 };

	if (m_pTwh) {
		m_pTwh->GetTextServices()->TxGetNaturalSize(
			DVASPECT_CONTENT,
			GetWindowDC(),
			NULL,
			NULL,
			TXTNS_FITTOCONTENT,
			&szExtent,
			&lWidth,
			&lHeight);
	}
	sz.cx = (int)lWidth;
	sz.cy = (int)lHeight;
	return sz;
}

void RichEdit::SetImmStatus(BOOL bOpen)
{
	HWND hwnd = GetWindowHandle();
	if (hwnd != NULL)
	{
		// 失去焦点时关闭输入法
		HIMC hImc = ::ImmGetContext(hwnd);
    // 失去焦点是会把关联的输入法去掉，导致无法无法输入中文
		//::ImmAssociateContext(hwnd, bOpen ? hImc : NULL);
		if (hImc != NULL) {
			if (ImmGetOpenStatus(hImc)) {
				if (!bOpen)
					ImmSetOpenStatus(hImc, FALSE);
			}
			else {
				if (bOpen)
					ImmSetOpenStatus(hImc, TRUE);
			}
			ImmReleaseContext(hwnd, hImc);
		}
	}
}

void RichEdit::SetTimer(UINT idTimer, UINT uTimeout)
{
	auto timeFlag = m_timeFlagMap.find(idTimer);
	if (timeFlag != m_timeFlagMap.end()) {
		timeFlag->second.Cancel();
	}

	auto callback = [this, idTimer]() {
		this->TxSendMessage(WM_TIMER, idTimer, 0, 0);
	};
	GlobalManager::Instance().Timer().AddCancelableTimer(m_timeFlagMap[idTimer].GetWeakFlag(), callback, uTimeout, TimerManager::REPEAT_FOREVER);
}

void RichEdit::KillTimer(UINT idTimer)
{
	auto timeFlag = m_timeFlagMap.find(idTimer);
	if (timeFlag != m_timeFlagMap.end()) {
		timeFlag->second.Cancel();
		m_timeFlagMap.erase(timeFlag);
	}
}

// 多行非rich格式的richedit有一个滚动条bug，在最后一行是空行时，LineDown和SetScrollPos无法滚动到最后
// 引入iPos就是为了修正这个bug
void RichEdit::SetScrollPos(UiSize64 szPos)
{
	int64_t cx = 0;
	int64_t cy = 0;
    if( m_pVScrollBar && m_pVScrollBar->IsValid() ) {
        int64_t iLastScrollPos = m_pVScrollBar->GetScrollPos();
        m_pVScrollBar->SetScrollPos(szPos.cy);
        cy = m_pVScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if( m_pHScrollBar && m_pHScrollBar->IsValid() ) {
		int64_t iLastScrollPos = m_pHScrollBar->GetScrollPos();
        m_pHScrollBar->SetScrollPos(szPos.cx);
        cx = m_pHScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if( cy != 0 ) {
		int64_t iPos = 0;
        if( m_pTwh && !m_bRich && m_pVScrollBar && m_pVScrollBar->IsValid() ) 
            iPos = m_pVScrollBar->GetScrollPos();
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, m_pVScrollBar->GetScrollPos());
        TxSendMessage(WM_VSCROLL, wParam, 0L, 0);
        if( m_pTwh && !m_bRich && m_pVScrollBar && m_pVScrollBar->IsValid() ) {
            if( cy > 0 && m_pVScrollBar->GetScrollPos() <= iPos )
                m_pVScrollBar->SetScrollPos(iPos);
        }
    }
    if( cx != 0 ) {
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, m_pHScrollBar->GetScrollPos());
        TxSendMessage(WM_HSCROLL, wParam, 0L, 0);
    }
}

void RichEdit::LineUp()
{
    TxSendMessage(WM_VSCROLL, SB_LINEUP, 0L, 0);
}

void RichEdit::LineDown()
{
	int64_t iPos = 0;
    if( m_pTwh && !m_bRich && m_pVScrollBar && m_pVScrollBar->IsValid() ) 
        iPos = m_pVScrollBar->GetScrollPos();
    TxSendMessage(WM_VSCROLL, SB_LINEDOWN, 0L, 0);
    if( m_pTwh && !m_bRich && m_pVScrollBar && m_pVScrollBar->IsValid() ) {
        if( m_pVScrollBar->GetScrollPos() <= iPos )
            m_pVScrollBar->SetScrollPos(m_pVScrollBar->GetScrollRange());
    }
}

void RichEdit::PageUp()
{
    TxSendMessage(WM_VSCROLL, SB_PAGEUP, 0L, 0);
}

void RichEdit::PageDown()
{
    TxSendMessage(WM_VSCROLL, SB_PAGEDOWN, 0L, 0);
}

void RichEdit::HomeUp()
{
    TxSendMessage(WM_VSCROLL, SB_TOP, 0L, 0);
}

void RichEdit::EndDown()
{
    TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L, 0);
}

void RichEdit::LineLeft()
{
    TxSendMessage(WM_HSCROLL, SB_LINELEFT, 0L, 0);
}

void RichEdit::LineRight()
{
    TxSendMessage(WM_HSCROLL, SB_LINERIGHT, 0L, 0);
}

void RichEdit::PageLeft()
{
    TxSendMessage(WM_HSCROLL, SB_PAGELEFT, 0L, 0);
}

void RichEdit::PageRight()
{
    TxSendMessage(WM_HSCROLL, SB_PAGERIGHT, 0L, 0);
}

void RichEdit::HomeLeft()
{
    TxSendMessage(WM_HSCROLL, SB_LEFT, 0L, 0);
}

void RichEdit::EndRight()
{
    TxSendMessage(WM_HSCROLL, SB_RIGHT, 0L, 0);
}

std::wstring RichEdit::GetType() const { return DUI_CTR_RICHEDIT; }

void RichEdit::DoInit()
{
	if (m_bInited)
		return;

	CREATESTRUCT cs = {0, };
	cs.style = m_lTwhStyle;
	cs.x = 0;
	cs.y = 0;
	cs.cy = 0;
	cs.cx = 0;
	cs.lpszName = m_sText.c_str();
	RichEditHost::CreateHost(this, &cs, &m_pTwh);
	ASSERT(m_pTwh != nullptr);
	if (m_pTwh != nullptr) {
		m_pTwh->SetTransparent(TRUE);
		LRESULT lResult;
		m_pTwh->GetTextServices()->TxSendMessage(EM_SETLANGOPTIONS, 0, 0, &lResult);
		m_pTwh->GetTextServices()->TxSendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_LINK, &lResult);
		m_pTwh->OnTxInPlaceActivate(NULL);
		if (m_pHScrollBar) {
			m_pHScrollBar->SetScrollRange(0);
		}
		if (m_pVScrollBar) {
			m_pVScrollBar->SetScrollRange(0);
		}

		//设置自动检测URL
		if (m_bAutoDetect) {
			SetAutoURLDetect(true);
		}
	}
	m_bInited = true;
}

void RichEdit::SetEnabled(bool bEnable /*= true*/)
{
	if (IsEnabled() == bEnable)
		return;
	__super::SetEnabled(bEnable);

	if (bEnable) {
		SetState(kControlStateNormal);
		SetTextColor(m_sTextColor.c_str());
	}
	else {
		SetState(kControlStateDisabled);
		SetTextColor(m_sDisabledTextColor.c_str());
	}
}

UiEstSize RichEdit::EstimateSize(UiSize /*szAvailable*/)
{
	UiFixedSize fixexSize = GetFixedSize();
	UiSize size(fixexSize.cx.GetInt32(), fixexSize.cy.GetInt32());
	if (fixexSize.cx.IsAuto() || fixexSize.cy.IsAuto()) {
		LONG iWidth = size.cx;
		LONG iHeight = size.cy;
		if (fixexSize.cy.IsAuto()) {
			ASSERT(!fixexSize.cx.IsAuto());
			iHeight = 0;
		}
		else if (fixexSize.cx.IsAuto()) {
			ASSERT(!fixexSize.cy.IsAuto());
			iWidth = 0;
		}
		SIZEL szExtent = {-1, -1};
		m_pTwh->GetTextServices()->TxGetNaturalSize(DVASPECT_CONTENT, 
													GetWindowDC(),
													NULL,
													NULL,
													TXTNS_FITTOCONTENT,
													&szExtent,
													&iWidth,
													&iHeight) ;
		
		//返回大小需要包含内边距
		UiPadding rcPadding = GetControlPadding();
		UiPadding rcTextPadding = GetTextPadding();
		if (fixexSize.cy.IsAuto()) {
			size.cy = iHeight + (rcPadding.top + rcPadding.bottom) + (rcTextPadding.top + rcTextPadding.bottom);
			fixexSize.cy.SetInt32(size.cy);
		}
		else if (fixexSize.cx.IsAuto()) {
			size.cx = iWidth + (rcPadding.left + rcPadding.right) + (rcTextPadding.left + rcTextPadding.right);
			fixexSize.cx.SetInt32(size.cx);
		}
	}
    return MakeEstSize(fixexSize);
}

UiSize RichEdit::EstimateText(UiSize szAvailable)
{
	UiPadding rcPadding = GetControlPadding();
	UiPadding rcTextPadding = GetTextPadding();
	szAvailable.cx -= (rcPadding.left + rcPadding.right);
	szAvailable.cy -= (rcPadding.top + rcPadding.bottom);
	szAvailable.cx -= (rcTextPadding.left + rcTextPadding.right);
	szAvailable.cy -= (rcTextPadding.top + rcTextPadding.bottom);

    szAvailable.Validate();
    LONG iWidth = szAvailable.cx;
	if (iWidth < 0) {
		iWidth = 0;
	}
    LONG iHeight = 0;
    SIZEL szExtent = { -1, -1 };
    m_pTwh->GetTextServices()->TxGetNaturalSize(DVASPECT_CONTENT,
												GetWindow()->GetPaintDC(),
												NULL,
												NULL,
												TXTNS_FITTOCONTENT,
												&szExtent,
												&iWidth,
												&iHeight);

	iWidth = std::max((int32_t)iWidth, 0);
	iHeight = std::max((int32_t)iHeight, 0);
	iWidth += (rcPadding.left + rcPadding.right);
	iHeight += (rcPadding.top + rcPadding.bottom);

	iWidth += (rcTextPadding.left + rcTextPadding.right);
	iHeight += (rcTextPadding.top + rcTextPadding.bottom);

	szAvailable.cx = std::max((int32_t)iWidth, 0);
    szAvailable.cy = std::max((int32_t)iHeight, 0);
    return szAvailable;
}

void RichEdit::SetPos(UiRect rc)
{
    Control::SetPos(rc);
    rc = GetRectWithoutPadding();
    bool bVScrollBarVisiable = false;
    if (m_pVScrollBar && m_pVScrollBar->IsValid()) {
        bVScrollBarVisiable = true;
        rc.right -= m_pVScrollBar->GetFixedWidth().GetInt32();
    }
    if (m_pHScrollBar && m_pHScrollBar->IsValid()) {
        rc.bottom -= m_pHScrollBar->GetFixedHeight().GetInt32();
    }

    if (m_pTwh != nullptr) {
		//调整编辑框的位置, 剪去文本内边距
		UiRect textRect = rc;
		UiPadding rcTextPadding = GetTextPadding();
		textRect.Deflate(rcTextPadding);
        m_pTwh->SetClientRect(&textRect);
        if (bVScrollBarVisiable && (!m_pVScrollBar->IsValid() || m_bVScrollBarFixing)) {
            LONG lWidth = rc.Width() + m_pVScrollBar->GetFixedWidth().GetInt32();
            LONG lHeight = 0;
            SIZEL szExtent = { -1, -1 };
            m_pTwh->GetTextServices()->TxGetNaturalSize(DVASPECT_CONTENT,
														GetWindowDC(),
														NULL,
														NULL,
														TXTNS_FITTOCONTENT,
														&szExtent,
														&lWidth,
														&lHeight);
            if (lHeight > rc.Height()) {
                //m_pVScrollBar->SetFadeVisible(true);
                m_pVScrollBar->SetScrollPos(0);
                m_bVScrollBarFixing = true;
            }
            else {
                if (m_bVScrollBarFixing) {
                    //m_pVScrollBar->SetFadeVisible(false);
                    m_pVScrollBar->SetScrollRange(0);
                    m_bVScrollBarFixing = false;
                }
            }
        }
    }

    if (m_pVScrollBar != NULL && m_pVScrollBar->IsValid()) {
        UiRect rcScrollBarPos(rc.right, rc.top, rc.right + m_pVScrollBar->GetFixedWidth().GetInt32(), rc.bottom);
        m_pVScrollBar->SetPos(rcScrollBarPos);
    }
    if (m_pHScrollBar != NULL && m_pHScrollBar->IsValid()) {
        UiRect rcScrollBarPos(rc.left, rc.bottom, rc.right, rc.bottom + m_pHScrollBar->GetFixedHeight().GetInt32());
        m_pHScrollBar->SetPos(rcScrollBarPos);
    }

    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        auto pControl = *it;
        if ((pControl == nullptr) || !pControl->IsVisible()){
            continue;
        }
        if (pControl->IsFloat()) {
            Layout::SetFloatPos(pControl, GetPos());
        }
        else {
            pControl->SetPos(rc); // 所有非float子控件放大到整个客户区
        }
    }
}

UINT RichEdit::GetControlFlags() const
{
	return IsEnabled() && IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

void RichEdit::HandleEvent(const EventArgs& msg)
{
	if (IsDisabledEvents(msg)) {
		//如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
		Box* pParent = GetParent();
		if (pParent != nullptr) {
			pParent->SendEvent(msg);
		}
		else {
			__super::HandleEvent(msg);
		}
	}
	if (msg.Type == kEventMouseMove) {
		OnMouseMessage(WM_MOUSEMOVE, msg);
		return;
	}
	if (msg.Type == kEventMouseWheel) {
		if (::GetAsyncKeyState(VK_CONTROL) < 0)
			return;
		
		ScrollBox::HandleEvent(msg);
		//OnMouseMessage(WM_MOUSEWHEEL, event);
		return;
	}

	if (msg.Type == kEventMouseButtonDown) {
		if (m_linkInfo.size() > 0)	{
			std::wstring url;
			if (HittestCustomLink(UiPoint(msg.ptMouse), url)) {
				SendEvent(kEventCustomLinkClick, (WPARAM)url.c_str());
				return;
			}
		}
		OnMouseMessage(WM_LBUTTONDOWN, msg);
		return;
	}
	if (msg.Type == kEventMouseButtonUp) {
		if (IsEnabled() && !m_bSelAllEver) {
			m_bSelAllEver = true;

			if (m_bSelAllOnFocus) {
				SetSelAll();
				if (m_lTwhStyle & ES_MULTILINE)
					HomeUp();
				else
					HomeLeft();
			}
		}

		OnMouseMessage(WM_LBUTTONUP, msg);
		return;
	}
	if (msg.Type == kEventMouseDoubleClick) {
		if (m_bReadOnly) {
			SetSelAll();
			return;
		}

		OnMouseMessage(WM_LBUTTONDBLCLK, msg);
		return;
	}
	if (msg.Type == kEventMouseRButtonDown) {
		OnMouseMessage(WM_RBUTTONDOWN, msg);
		return;
	}
	if (msg.Type == kEventMouseRButtonUp) {
		OnMouseMessage(WM_RBUTTONUP, msg);
		return;
	}
	ScrollBox::HandleEvent(msg);
}

bool RichEdit::OnSetCursor(const EventArgs& msg)
{
	std::wstring strLink;
	if (HittestCustomLink(UiPoint(msg.ptMouse), strLink)) {
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
		return true;
	}
	if (m_pTwh && !IsReadOnly() && m_pTwh->DoSetCursor(NULL, &msg.ptMouse)) {
		return true;
	}
	else {
		::SetCursor(::LoadCursor(NULL, IsReadOnly() ? IDC_ARROW : IDC_IBEAM));
		return true;
	}
}

bool RichEdit::OnSetFocus(const EventArgs& /*msg*/)
{
	if (m_pTwh) {
		m_pTwh->OnTxInPlaceActivate(NULL);
		m_pTwh->GetTextServices()->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
		ShowCaret(true);
	}
	SetImmStatus(TRUE);
	Invalidate();
	return true;
}

bool RichEdit::OnKillFocus(const EventArgs& /*msg*/)
{
	if (m_pTwh) {
		m_pTwh->OnTxInPlaceActivate(NULL);
		m_pTwh->GetTextServices()->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
		ShowCaret(false);
	}

	m_bSelAllEver = false;
	if (m_bNoSelOnKillFocus && m_bReadOnly && IsEnabled()) {
		SetSelNone();
	}
	if (m_bSelAllOnFocus && IsEnabled()) {
		SetSelNone();
	}

	OnScreenKeyboardManager::GetInstance()->ShowOSK(false);
	SetImmStatus(FALSE);
	Invalidate();
	return true;
}

bool RichEdit::OnChar(const EventArgs& msg)
{
	//TAB
	if (::GetKeyState(VK_TAB) < 0 && !m_bWantTab) {
		SendEvent(kEventTab);
		return true;
	}
	//Number
	if (m_bNumberOnly) {
		if (msg.wParam < '0' || msg.wParam > '9')
			return true;
	}

	TxSendMessage(WM_CHAR, msg.wParam, msg.lParam, NULL);
	return true;
}

bool RichEdit::OnKeyDown(const EventArgs& msg)
{
	if (msg.wParam == VK_RETURN && ::GetAsyncKeyState(VK_SHIFT) >= 0)	{
		if (m_bNeedReturnMsg && ((m_bReturnMsgWantCtrl && ::GetAsyncKeyState(VK_CONTROL) < 0) ||
			(!m_bReturnMsgWantCtrl && ::GetAsyncKeyState(VK_CONTROL) >= 0))) {
			SendEvent(kEventReturn);
			return true;
		}
	}
	else if (m_bNumberOnly && msg.wParam == 'V' && ::GetKeyState(VK_CONTROL) < 0) {
		std::wstring strClipText;
		GetClipboardText(strClipText);
		if (!strClipText.empty()) {
			std::wstring strNum;
			for (auto it = strClipText.begin(); it != strClipText.end(); it++)
			{
				if ((*it) <= L'9' && (*it) >= L'0') {
					strNum.push_back((*it));
				}
			}
			if (strNum.empty())
				return true;

			SetClipBoardText(strNum); //修改剪切板内容为纯数字
			nbase::ThreadManager::PostTask([strClipText]() { SetClipBoardText(strClipText); }); //粘贴完后又把剪切板内容改回来
		}
	}

	TxSendMessage(WM_KEYDOWN, msg.wParam, msg.lParam, NULL);
	return true;
}

bool RichEdit::OnImeStartComposition(const EventArgs& /*msg*/)
{
	HWND hWnd = GetWindowHandle();
	if (hWnd == NULL)
		return true;

	HIMC hImc = ::ImmGetContext(hWnd);
	if (hImc == NULL)
		return true;

	COMPOSITIONFORM	cfs;
	UiPoint ptScrollOffset = GetScrollOffsetInScrollBox();
	POINT pt;
	pt.x = m_iCaretPosX - ptScrollOffset.x;
	pt.y = m_iCaretPosY - ptScrollOffset.y;

	//pt.y += (m_iCaretHeight + lf.lfHeight) / 4;
	cfs.dwStyle = CFS_POINT;
	if (pt.x < 1) pt.x = 1;
	if (pt.y < 1) pt.y = 1;
	cfs.ptCurrentPos = pt;
	::ImmSetCompositionWindow(hImc, &cfs);
	::ImmReleaseContext(hWnd, hImc);
	m_bIsComposition = true;
	return true;
}

bool RichEdit::OnImeEndComposition(const EventArgs& /*msg*/)
{
	m_bIsComposition = false;
	return true;
}

void RichEdit::OnMouseMessage(UINT uMsg, const EventArgs& msg)
{
	UiPoint pt(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
	pt.Offset(GetScrollOffsetInScrollBox());
	TxSendMessage(uMsg, msg.wParam, MAKELPARAM(pt.x, pt.y), NULL);
}

void RichEdit::Paint(IRender* pRender, const UiRect& rcPaint)
{
	//必须不使用缓存，否则绘制异常
	ASSERT(IsUseCache() == false);
    UiRect rcTemp;
	if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
		return;
	}

    Control::Paint(pRender, rcPaint);

    if( m_pTwh ) {
        UiRect rc;
        m_pTwh->GetControlRect(&rc);
        // Remember wparam is actually the hdc and lparam is the update
        // rect because this message has been preprocessed by the window.
		HDC hdc = pRender->GetDC();
		RECT paintRect = { rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom };
        m_pTwh->GetTextServices()->TxDraw(DVASPECT_CONTENT,  	// Draw Aspect
										  /*-1*/0,				// Lindex
										  NULL,					// Info for drawing optimazation
										  NULL,					// target device information
										  hdc,			        // Draw device HDC
										  NULL, 				// Target device HDC
										  (RECTL*)&rc,			// Bounding client rectangle
										  NULL, 		        // Clipping rectangle for metafiles
										  &paintRect,			// Update rectangle
										  NULL, 	   			// Call back function
										  NULL,					// Call back parameter
										  0);				    // What view of the object

		pRender->ReleaseDC(hdc);
		if( m_bVScrollBarFixing ) {
            LONG lWidth = rc.right - rc.left + m_pVScrollBar->GetFixedWidth().GetInt32();
			//LONG lWidth = rc.right - rc.left;
            LONG lHeight = 0;
            SIZEL szExtent = { -1, -1 };
            m_pTwh->GetTextServices()->TxGetNaturalSize(
                DVASPECT_CONTENT, 
                GetWindowDC(), 
                NULL,
                NULL,
                TXTNS_FITTOCONTENT,
                &szExtent,
                &lWidth,
                &lHeight);

            if( lHeight <= rc.bottom - rc.top ) {
                Arrange();
            }
        }
    }
}

void RichEdit::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        return;
    }

    PaintCaret(pRender, rcPaint);

    if (m_items.size() > 0) {
        UiRect rc = GetRectWithoutPadding();
        if (m_pVScrollBar && m_pVScrollBar->IsValid()) {
            rc.right -= m_pVScrollBar->GetFixedWidth().GetInt32();
        }
        if (m_pHScrollBar && m_pHScrollBar->IsValid()) {
            rc.bottom -= m_pHScrollBar->GetFixedHeight().GetInt32();
        }

        if (!UiRect::Intersect(rcTemp, rcPaint, rc)) {
            for (auto it = m_items.begin(); it != m_items.end(); ++it) {
                auto pControl = *it;
                if ((pControl == nullptr) || !pControl->IsVisible()) {
                    continue;
                }
                UiRect controlPos = pControl->GetPos();
                if (!UiRect::Intersect(rcTemp, rcPaint, controlPos)) {
                    continue;
                }
                if (pControl->IsFloat()) {
                    if (!UiRect::Intersect(rcTemp, GetRect(), controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
            }
        }
        else {
            AutoClip childClip(pRender, rcTemp);
            for (auto it = m_items.begin(); it != m_items.end(); ++it) {
                auto pControl = *it;
                if ((pControl == nullptr) || !pControl->IsVisible()) {
                    continue;
                }
                UiRect controlPos = pControl->GetPos();
                if (!UiRect::Intersect(rcTemp, rcPaint, controlPos)) {
                    continue;
                }
                if (pControl->IsFloat()) {
                    if (!UiRect::Intersect(rcTemp, GetRect(), controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
                else {
                    if (!UiRect::Intersect(rcTemp, rc, controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
            }
        }
    }

    if (m_pVScrollBar != NULL && m_pVScrollBar->IsVisible()) {
        UiRect verBarPos = m_pVScrollBar->GetPos();
        if (UiRect::Intersect(rcTemp, rcPaint, verBarPos)) {
            m_pVScrollBar->AlphaPaint(pRender, rcPaint);
        }
    }

    if (m_pHScrollBar != NULL && m_pHScrollBar->IsVisible()) {
        UiRect horBarPos = m_pVScrollBar->GetPos();
        if (UiRect::Intersect(rcTemp, rcPaint, horBarPos)) {
            m_pHScrollBar->AlphaPaint(pRender, rcPaint);
        }
    }
}

void RichEdit::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"vscrollbar") {
		if (strValue == L"true") {
			m_lTwhStyle |= ES_DISABLENOSCROLL | WS_VSCROLL;
			EnableScrollBar(true, GetHScrollBar() != NULL);
		}
		else {
			m_lTwhStyle &= ~WS_VSCROLL;
			if (!(m_lTwhStyle & WS_HSCROLL)) {
				m_lTwhStyle &= ~ES_DISABLENOSCROLL;
			}
			EnableScrollBar(false, GetHScrollBar() != NULL);
		}
	}
	else if ((strName == L"auto_vscroll") || (strName == L"autovscroll")){
		if (strValue == L"true") {
			m_lTwhStyle |= ES_AUTOVSCROLL;
		}
		else if (strValue == L"false") {
			m_lTwhStyle &= ~ES_AUTOVSCROLL;
		}
	}
	else if (strName == L"hscrollbar") {
		if (strValue == L"true") {
			m_lTwhStyle |= ES_DISABLENOSCROLL | WS_HSCROLL;
			EnableScrollBar(GetVScrollBar() != NULL, true);
		}
		else {
			m_lTwhStyle &= ~WS_HSCROLL;
			if (!(m_lTwhStyle & WS_VSCROLL)) {
				m_lTwhStyle &= ~ES_DISABLENOSCROLL;
			}
			EnableScrollBar(GetVScrollBar() != NULL, false);
		}
	}
	else if ((strName == L"auto_hscroll") || (strName == L"autohscroll")){
		if (strValue == L"true") {
			m_lTwhStyle |= ES_AUTOHSCROLL;
		}
		else if (strValue == L"false") {
			m_lTwhStyle &= ~ES_AUTOHSCROLL;
		}
	}
	else if ((strName == L"single_line") || (strName == L"singleline")) {
		if (strValue == L"true") {
			m_lTwhStyle &= ~ES_MULTILINE;
		}
		else if (strValue == L"false") {
			m_lTwhStyle |= ES_MULTILINE;
		}
	}
	else if ((strName == L"multi_line") || (strName == L"multiline")) {
		if (strValue == L"false") {
			m_lTwhStyle &= ~ES_MULTILINE;
		}
		else if (strValue == L"true") {
			m_lTwhStyle |= ES_MULTILINE;
		}
	}
	else if (strName == L"readonly") {
		if (strValue == L"true") {
			m_lTwhStyle |= ES_READONLY;
			m_bReadOnly = true;
		}
		else if (strValue == L"false") {
			m_lTwhStyle &= ~ES_READONLY;
			m_bReadOnly = false;
		}
	}
	else if (strName == L"password") {
		if (strValue == L"true") {
			m_lTwhStyle |= ES_PASSWORD;
			m_bPassword = true;
		}
		else if (strValue == L"false") {
			m_lTwhStyle &= ~ES_PASSWORD;
			m_bPassword = false;
		}
	}
	else if (strName == L"number") {
		if (strValue == L"true") {
			m_lTwhStyle |= ES_NUMBER;
			m_bNumberOnly = true;
		}
		else if (strValue == L"false"){
			m_lTwhStyle &= ~ES_NUMBER;
			m_bNumberOnly = false;
		}
	}
	else if ((strName == L"text_align") || (strName == L"align")) {
		if (strValue.find(L"left") != std::wstring::npos) {
			m_lTwhStyle &= ~(ES_CENTER | ES_RIGHT);
			m_lTwhStyle |= ES_LEFT;
		}		
		if (strValue.find(L"right") != std::wstring::npos) {
			m_lTwhStyle &= ~(ES_LEFT | ES_CENTER);
			m_lTwhStyle |= ES_RIGHT;
		}
		if (strValue.find(L"hcenter") != std::wstring::npos) {
			m_lTwhStyle &= ~(ES_LEFT | ES_RIGHT);
			m_lTwhStyle |= ES_CENTER;
		}
		if (strValue.find(L"top") != std::wstring::npos) {
			m_textVerAlignType = kVerAlignTop;
		}
		if (strValue.find(L"bottom") != std::wstring::npos) {
			m_textVerAlignType = kVerAlignBottom;
		}
		if (strValue.find(L"vcenter") != std::wstring::npos) {
			m_textVerAlignType = kVerAlignCenter;
		}		
	}
	else if ((strName == L"text_padding") || (strName == L"textpadding")) {
		UiPadding rcTextPadding;
		AttributeUtil::ParsePaddingValue(strValue.c_str(), rcTextPadding);
		SetTextPadding(rcTextPadding);
	}
	else if ((strName == L"normal_text_color") || (strName == L"normaltextcolor")){
		m_sTextColor = strValue;
		if (IsEnabled()) {
			SetTextColor(m_sTextColor.c_str());
		}
	}
	else if ((strName == L"disabled_text_color") || (strName == L"disabledtextcolor")){
		m_sDisabledTextColor = strValue;
		if (!IsEnabled()) {
			SetTextColor(m_sDisabledTextColor.c_str());
		}
	}
	else if ((strName == L"caret_color") || (strName == L"caretcolor")){
		//设置光标的颜色
		SetCaretColor(strValue);
	}
	else if ((strName == L"prompt_mode") || (strName == L"promptmode")){
		//提示模式
		if (strValue == L"true") {
			m_bAllowPrompt = true;
		}
		else {
			m_bAllowPrompt = false;
		}
	}
	else if ((strName == L"prompt_color") || (strName == L"promptcolor")){
		//提示文字的颜色
		m_sPromptColor = strValue;
	}
	else if ((strName == L"prompt_text") || (strName == L"prompttext")) {
		//提示文字
		SetPromptText(strValue);
	}
	else if ((strName == L"prompt_textid") || (strName == L"prompttextid")){
		//提示文字ID
		SetPromptTextId(strValue);
	}
	else if ((strName == L"focused_image") || (strName == L"focusedimage")){
		SetFocusedImage(strValue);
	}
	else if (strName == L"font") {
		SetFontId(strValue);
	}
	else if (strName == L"text") {
		SetText(strValue);
	}
	else if ((strName == L"textid") || (strName == L"textid")){
		SetTextId(strValue);
	}
	else if ((strName == L"want_tab") || (strName == L"wanttab")){
		SetWantTab(strValue == L"true");
	}
	else if ((strName == L"want_return_msg") || (strName == L"wantreturnmsg")){
		SetNeedReturnMsg(strValue == L"true");
	}
	else if ((strName == L"return_msg_want_ctrl") || (strName == L"returnmsgwantctrl")){
		SetReturnMsgWantCtrl(strValue == L"true");
	}
	else if (strName == L"rich") {
		//是否为富文本属性
		SetRich(strValue == L"true");
	}
	else if (strName == L"auto_detect_url") {
		//是否自动检测URL，如果是URL则显示为超链接
		SetAutoURLDetect(strValue == L"true");
	}
	else if ((strName == L"max_char") || (strName == L"maxchar")){
		//限制最多字符数（默认为32KB）
		SetLimitText(_wtoi(strValue.c_str()));
	}
	else {
		Box::SetAttribute(strName, strValue);
	}
}

BOOL RichEdit::CreateCaret(INT xWidth, INT yHeight)
{
	m_iCaretWidth = xWidth;
	m_iCaretHeight = yHeight;
	return true;
}

BOOL RichEdit::ShowCaret(BOOL fShow)
{
	if (fShow) {
		m_bIsCaretVisiable = true;
		m_drawCaretFlag.Cancel();
		std::function<void()> closure = nbase::Bind(&RichEdit::ChangeCaretVisiable, this);
		GlobalManager::Instance().Timer().AddCancelableTimer(m_drawCaretFlag.GetWeakFlag(), closure, 500, TimerManager::REPEAT_FOREVER);
	}
	else {
		m_bIsCaretVisiable = false;
		m_drawCaretFlag.Cancel();
	}

	Invalidate();
	return true;
}

void RichEdit::SetCaretColor(const std::wstring& dwColor)
{
	m_sCaretColor = dwColor;
}

std::wstring RichEdit::GetCaretColor()
{
	return m_sCaretColor.c_str();
}

RECT RichEdit::GetCaretRect()
{
	RECT rc = { m_iCaretPosX, m_iCaretPosY, m_iCaretPosX + m_iCaretWidth, m_iCaretPosY + m_iCaretHeight };
	return rc;
}

BOOL RichEdit::SetCaretPos(INT x, INT y)
{
	m_iCaretPosX = x;
	m_iCaretPosY = y;
	ShowCaret(GetSelText().empty());

	return true;
}

void RichEdit::ChangeCaretVisiable()
{
	m_bIsCaretVisiable = !m_bIsCaretVisiable;
	Invalidate();
}

void RichEdit::PaintCaret(IRender* pRender, const UiRect& /*rcPaint*/)
{
	assert(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (m_bReadOnly && m_bNoCaretReadonly)
		return;

	if (m_bIsCaretVisiable && !m_bIsComposition) {
		UiRect rect(m_iCaretPosX, m_iCaretPosY, m_iCaretPosX, m_iCaretPosY + m_iCaretHeight);
		UiColor dwClrColor(0xff000000);
		if (!m_sCaretColor.empty()) {
			dwClrColor = this->GetUiColor(m_sCaretColor.c_str());
		}
		pRender->DrawLine(UiPoint(rect.left, rect.top), UiPoint(rect.right, rect.bottom), dwClrColor, m_iCaretWidth);
	}
}

void RichEdit::SetPromptMode(bool bPrompt)
{
	if(bPrompt == m_bAllowPrompt)
		return;
	m_bAllowPrompt = bPrompt;
	Invalidate();
}

std::wstring RichEdit::GetPromptText() const
{
	std::wstring strText = m_sPromptText.c_str();
	if (strText.empty() && !m_sPromptTextId.empty()) {
		strText = GlobalManager::Instance().Lang().GetStringViaID(m_sPromptTextId.c_str());
	}

	return strText;
}

std::string RichEdit::GetUTF8PromptText() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(GetPromptText(), strOut, CP_UTF8);
	return strOut;
}

void RichEdit::SetPromptText(const std::wstring& strText)
{
	if (m_sPromptText == strText) return;
	m_sPromptText = strText;

	Invalidate();
}

void RichEdit::SetUTF8PromptText(const std::string& strText)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strText, strOut, CP_UTF8);
	SetPromptText(strOut);
}

void RichEdit::SetPromptTextId(const std::wstring& strTextId)
{
	if (m_sPromptTextId == strTextId) return;
	m_sPromptTextId = strTextId;

	Invalidate();
}

void RichEdit::SetUTF8PromptTextId(const std::string& strTextId)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strTextId, strOut, CP_UTF8);
	SetPromptTextId(strOut);
}

void RichEdit::PaintPromptText(IRender* pRender)
{
	assert(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	long len = GetTextLength(GTL_DEFAULT);
	if (len != 0) {
		return;
	}

	if (!m_pTwh) {
		return;
	}

	std::wstring strPrompt = GetPromptText();
	if (strPrompt.empty() || m_sPromptColor.empty()) {
		return;
	}

	UiRect rc;
	m_pTwh->GetControlRect(&rc);

	UiColor dwClrColor = this->GetUiColor(m_sPromptColor.c_str());
	UINT dwStyle = TEXT_NOCLIP;
	pRender->DrawString(rc, strPrompt, dwClrColor, m_sFontId.c_str(), dwStyle);
}

std::wstring RichEdit::GetFocusedImage()
{
	if (m_pFocusedImage != nullptr) {
		return m_pFocusedImage->GetImageString();
	}
	return std::wstring();
}

void RichEdit::SetFocusedImage( const std::wstring& strImage )
{
	if (m_pFocusedImage == nullptr) {
		m_pFocusedImage = new Image;
	}
	m_pFocusedImage->SetImageString(strImage);
	Invalidate();
}

void RichEdit::PaintStateImages(IRender* pRender)
{
	if( IsReadOnly() )
		return;

	if(IsFocused()) {
		if (m_pFocusedImage != nullptr) {
			PaintImage(pRender, m_pFocusedImage);
		}		
		PaintPromptText(pRender);
		return;
	}

	__super::PaintStateImages(pRender);
	PaintPromptText(pRender);
}

void RichEdit::SetNoSelOnKillFocus(bool bNoSel)
{
	m_bNoSelOnKillFocus = bNoSel;
}

void RichEdit::SetSelAllOnFocus(bool bSelAll)
{
	m_bSelAllOnFocus = bSelAll;
}

void RichEdit::SetNoCaretReadonly()
{
	m_bNoCaretReadonly = true;
}

void RichEdit::AddColorText(const std::wstring &str, const std::wstring &color)
{
	if( !m_bRich || str.empty() || color.empty() ) {
		ASSERT(FALSE);
		return;
	}
	UiColor dwColor = this->GetUiColor(color);

	CHARFORMAT2W cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.cbSize = sizeof(CHARFORMAT2W);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = dwColor.ToCOLORREF();

	this->ReplaceSel(str, FALSE);
	int len = GetTextLength();
	this->SetSel(len - (int)str.size(), len);
	this->SetSelectionCharFormat(cf);

	this->SetSelNone();
	GetDefaultCharFormat(cf);
	SetSelectionCharFormat(cf);
}

void RichEdit::AddLinkColorText(const std::wstring &str, const std::wstring &color, const std::wstring &linkInfo)
{
	if( !m_bRich || str.empty() || color.empty() ) {
		ASSERT(FALSE);
		return;
	}
	UiColor dwColor = this->GetUiColor(color);

	CHARFORMAT2W cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.cbSize = sizeof(CHARFORMAT2W);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = dwColor.ToCOLORREF();

	this->ReplaceSel(str, FALSE);
	int len = GetTextLength();
	this->SetSel(len - (int)str.size(), len);
	this->SetSelectionCharFormat(cf);
	LinkInfo info;
	info.info = linkInfo;
	TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&(info.cr), NULL);
	m_linkInfo.push_back(info);

	this->SetSelNone();
	GetDefaultCharFormat(cf);
	SetSelectionCharFormat(cf);
}
void  RichEdit::AddLinkColorTextEx(const std::wstring& str, const std::wstring &color, const std::wstring &linkInfo, const std::wstring& strFontId)
{
	if (!m_bRich || str.empty() || color.empty()) {
		ASSERT(FALSE);
		return;
	}
	
	std::string link;
	std::string text;
	std::string font_face;
	StringHelper::UnicodeToMBCS(linkInfo, link);
	StringHelper::UnicodeToMBCS(str, text);
	LOGFONT lf = {0,};
	if (strFontId.empty()) {
		RichEditHost::GetLogFont(m_sFontId.c_str(), lf);
	}
	else {
		RichEditHost::GetLogFont(strFontId, lf);
	}
	StringHelper::UnicodeToMBCS(lf.lfFaceName, font_face);
	UiColor dwTextColor = GlobalManager::Instance().Color().GetColor(color);
	static std::string font_format = "{\\fonttbl{\\f0\\fnil\\fcharset%d %s;}}";
	static std::string color_format = "{\\colortbl ;\\red%d\\green%d\\blue%d;}";
	static std::string link_format = "{\\rtf1%s%s\\f0\\fs%d{\\field{\\*\\fldinst{HYPERLINK \"%s\"}}{\\fldrslt{\\cf1 %s}}}}";
	char sfont[255] = { 0 };
	sprintf_s(sfont, font_format.c_str(), lf.lfCharSet, font_face.c_str());
	char scolor[255] = { 0 };
	sprintf_s(scolor, color_format.c_str(), dwTextColor.GetR(), dwTextColor.GetG(), dwTextColor.GetB());
	char slinke[1024] = { 0 };
	sprintf_s(slinke, link_format.c_str(), sfont, scolor, ((int)(-lf.lfHeight *1.5))/2*2, link.c_str(), text.c_str());
	std::wstring temp;
	StringHelper::MBCSToUnicode(slinke, temp);
	SETTEXTEX st;
	st.codepage = ((UINT32)~((UINT32)0));
	st.flags = ST_SELECTION | ST_KEEPUNDO;
	TxSendMessage(EM_SETTEXTEX, (WPARAM)&st, (LPARAM)(LPCTSTR)slinke, NULL);
	return;
}
void RichEdit::AddLinkInfo(const CHARRANGE cr, const std::wstring &linkInfo)
{
	LinkInfo info;
	info.info = linkInfo;
	info.cr = cr;
	m_linkInfo.push_back(info);
}

void RichEdit::AddLinkInfoEx(const CHARRANGE cr, const std::wstring& linkInfo)
{
	CHARFORMAT2 cf2;
	ZeroMemory(&cf2, sizeof(CHARFORMAT2));
	cf2.cbSize = sizeof(CHARFORMAT2);
	cf2.dwMask = CFM_LINK;
	cf2.dwEffects |= CFE_LINK;

	SetSel(cr.cpMin, cr.cpMax);
	SetSelectionCharFormat(cf2);

	AddLinkInfo(cr, linkInfo);
}

//根据point来hittest自定义link的数据，返回true表示在link上，info是link的自定义属性
bool RichEdit::HittestCustomLink(UiPoint pt, std::wstring& info)
{
	bool bLink = false;
	info.clear();
	if (m_linkInfo.size() > 0) {
		pt.Offset(GetScrollOffsetInScrollBox());
		int nCharIndex = CharFromPos(pt);
		for (auto it = m_linkInfo.begin(); it != m_linkInfo.end(); it++)
		{
			if ((*it).cr.cpMin <= nCharIndex && (*it).cr.cpMax > nCharIndex) {
				info = (*it).info.c_str();
				bLink = true;
				break;
			}
		}
	}

	return bLink;
}

void RichEdit::ClearImageCache()
{
	__super::ClearImageCache();
	if (m_pFocusedImage != nullptr) {
		m_pFocusedImage->ClearImageCache();
	}	
}

void RichEdit::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
{
	ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
	if ((padding.left < 0) || (padding.top < 0) ||
		(padding.right < 0) || (padding.bottom < 0)) {
		return;
	}
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScalePadding(padding);
	}
	if (!GetTextPadding().Equals(padding)) {
		m_rcTextPadding.left = TruncateToUInt16(padding.left);
		m_rcTextPadding.top = TruncateToUInt16(padding.top);
		m_rcTextPadding.right = TruncateToUInt16(padding.right);
		m_rcTextPadding.bottom = TruncateToUInt16(padding.bottom);
		RelayoutOrRedraw();
	}
}

UiPadding RichEdit::GetTextPadding() const
{
	return UiPadding(m_rcTextPadding.left, m_rcTextPadding.top, m_rcTextPadding.right, m_rcTextPadding.bottom);
}

//----------------下面函数用作辅助 字节数限制
bool RichEdit::IsAsciiChar(const wchar_t ch)
{
	return (ch <= 0x7e && ch >= 0x20);
}

int RichEdit::GetAsciiCharNumber(const std::wstring &str)
{
	int len = (int)str.size(), sum = 0;
	for( int i = 0; i < len; i++ )
	{
		if( IsAsciiChar(str[i]) )
			sum += 1;
		else
			sum += 2;
	}
	return sum;
}

void RichEdit::LimitAsciiNumber(std::wstring &src, int limit)
{
	int len = (int)src.size(), sum = 0;
	for( int i = 0; i < len; i++ )
	{
		if( IsAsciiChar(src[i]) )
			sum += 1;
		else
			sum += 2;
		if( sum > limit ) {
			src.erase(i);
			break;
		}
	}
}

void RichEdit::GetClipboardText( std::wstring &out )
{
	out.clear();

	BOOL ret = ::OpenClipboard(NULL);
	if(ret) {
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT)) {
			HANDLE h = ::GetClipboardData(CF_UNICODETEXT);
			if(h != INVALID_HANDLE_VALUE) {
				wchar_t* buf = (wchar_t*)::GlobalLock(h);
				if(buf != NULL)	{
					std::wstring str(buf, GlobalSize(h)/sizeof(wchar_t));
					out = str;
					::GlobalUnlock(h);
				}
			}
		}
		else if(::IsClipboardFormatAvailable(CF_TEXT)) {
			HANDLE h = ::GetClipboardData(CF_TEXT);
			if(h != INVALID_HANDLE_VALUE) {
				char* buf = (char*)::GlobalLock(h);
				if(buf != NULL)	{
					std::string str(buf, GlobalSize(h));
					StringHelper::MBCSToUnicode(str, out);

					::GlobalUnlock(h);
				}
			}
		}
		::CloseClipboard();
	}
}

void RichEdit::SetClipBoardText(const std::wstring &str)
{
	if (!::OpenClipboard(NULL))
		return;

	if (!::EmptyClipboard()) {
		::CloseClipboard();
		return;
	}

	size_t len = str.length();
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t)); //分配全局内存  
	if (!hMem) {
		::CloseClipboard();
		return;
	}

	wchar_t* lpStr = (wchar_t*)::GlobalLock(hMem); //锁住内存区 
	if (lpStr) {
		::memcpy(lpStr, str.c_str(), len * sizeof(wchar_t)); //把数据拷贝考全局内存中
		lpStr[len] = wchar_t(0); //字符串末尾设为'\0'
		::GlobalUnlock(hMem); //释放锁 
	}

	::SetClipboardData(CF_UNICODETEXT, hMem); //把内存中的数据放到剪切板上
	::CloseClipboard(); //关闭剪切板	
}

} // namespace ui
