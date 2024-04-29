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
	/** 构造函数，构造后引用计数为1，外部可以通过AddRef和Release控制对象的生命周期
	*/
	explicit RichEditHost(RichEdit* pRichEdit);
	RichEditHost(const RichEditHost& r) = delete;
	RichEditHost& operator=(const RichEditHost& r) = delete;

private:
	virtual ~RichEditHost();

public:
	/** 获取ITextServices接口
	*/
	ITextServices* GetTextServices(void) const;

	/** 释放资源
	*/
	void Clear();

    void SetClientRect(const UiRect& rc);
	void GetControlRect(UiRect* prc);

	//是否自动换行（仅在单行文本模式下有效，多行文本模式时无效）
    void SetWordWrap(bool bWordWrap);
	bool IsWordWrap() const;

	//是否自动选择单词
	void SetEnableAutoWordSel(bool bEnableAutoWordSel);

	//是否只读
    void SetReadOnly(bool fReadOnly);
	bool IsReadOnly() const;

	//设置密码字符，必须为有效的字符，不可以是'\0'
	void SetPasswordChar(WCHAR chPasswordChar);
    void SetPassword(bool bPassword);
	bool IsPassword() const;
	void SetShowPassword(bool bShow); //设置是否显示密码
	bool IsShowPassword() const;//是否显示密码
	void SetFlashPasswordChar(bool bFlash);
	bool IsFlashPasswordChar() const;

	//是否只允许输入数字字符
	bool IsNumberOnly() const;
	void SetNumberOnly(bool bNumberOnly);

	//是否支持多行
	bool IsMultiLine() const;
	void SetMultiLine(bool bMultiLine);

	//文本横向和纵向对齐方式
	void SetHAlignType(HorAlignType alignType);
	void SetVAlignType(VerAlignType alignType);

	//横向和纵向滚动条设置
	void SetVScrollBar(bool bEnable);
	void SetHScrollBar(bool bEnable);	

	//当用户在最后一行按 ENTER 时，自动将文本向上滚动一页。
	void SetAutoVScroll(bool bEnable);

	//当用户在行尾键入一个字符时，自动将文本向右滚动 10 个字符。
	//当用户按 Enter 时，控件会将所有文本滚动回零位置。
	void SetAutoHScroll(bool bEnable);

	// 设置字体
    void SetFontId(const std::wstring& fontId);
	//设置文本颜色
    void SetTextColor(COLORREF dwColor);
	//获取文本颜色
	COLORREF GetTextColor() const;

    void SetExtent(SIZEL sizelExtent);
    void SetAllowBeep(bool bAllowBeep);
	bool IsAllowBeep() const;
	bool IsRichText() const;
    void SetRichText(bool fNew);
    bool SetSaveSelection(bool fSaveSelection);
	void SetHideSelection(bool fHideSelection);
    bool SetCursor(const UiRect* prc, const UiPoint* pt);
    void SetTransparent(bool fTransparent);    
    void SetDisabled(bool fOn);
    void SetSelBarWidth(LONG lSelBarWidth);

    void SetCharFormat(const CHARFORMAT2W& c);
    void SetParaFormat(const PARAFORMAT2& p);

	HRESULT OnTxInPlaceDeactivate();
	HRESULT OnTxInPlaceActivate(LPCRECT prcClient);

public:

    static void GetLogFont(const std::wstring& fontId, LOGFONT& lf);

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
	//@cmember Get the DC for the host
	virtual HDC TxGetDC() override;

	//@cmember Release the DC gotten from the host
	virtual INT TxReleaseDC(HDC hdc) override;

	//@cmember Show the scroll bar
	virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow) override;

	//@cmember Enable the scroll bar
	virtual BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags) override;

	//@cmember Set the scroll range
	virtual BOOL TxSetScrollRange( INT fnBar,
								   LONG nMinPos,
								   INT nMaxPos,
								   BOOL fRedraw) override;

	//@cmember Set the scroll position
	virtual BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw) override;

	//@cmember InvalidateRect
	virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode) override;

	//@cmember Send a WM_PAINT to the window
	virtual void TxViewChange(BOOL fUpdate) override;

	//@cmember Create the caret
	virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) override;

	//@cmember Show the caret
	virtual BOOL TxShowCaret(BOOL fShow) override;

	//@cmember Set the caret position
	virtual BOOL TxSetCaretPos(INT x, INT y) override;

	//@cmember Create a timer with the specified timeout
	virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout) override;

	//@cmember Destroy a timer
	virtual void TxKillTimer(UINT idTimer) override;

	//@cmember Scroll the content of the specified window's client area
	virtual void TxScrollWindowEx( INT dx,
								   INT dy,
								   LPCRECT lprcScroll,
								   LPCRECT lprcClip,
								   HRGN hrgnUpdate,
								   LPRECT lprcUpdate,
								   UINT fuScroll) override;

	//@cmember Get mouse capture
	virtual void TxSetCapture(BOOL fCapture) override;

	//@cmember Set the focus to the text window
	virtual void TxSetFocus() override;

	//@cmember Establish a new cursor shape
	virtual void TxSetCursor(HCURSOR hcur, BOOL fText) override;

	//@cmember Converts screen coordinates of a specified point to the client coordinates 
	virtual BOOL TxScreenToClient(LPPOINT lppt) override;

	//@cmember Converts the client coordinates of a specified point to screen coordinates
	virtual BOOL TxClientToScreen(LPPOINT lppt) override;

	//@cmember Request host to activate text services
	virtual HRESULT TxActivate(LONG* plOldState) override;

	//@cmember Request host to deactivate text services
	virtual HRESULT TxDeactivate(LONG lNewState) override;

	//@cmember Retrieves the coordinates of a window's client area
	virtual HRESULT TxGetClientRect(LPRECT prc) override;

	//@cmember Get the view rectangle relative to the inset
	virtual HRESULT TxGetViewInset(LPRECT prc) override;

	//@cmember Get the default character format for the text
	virtual HRESULT TxGetCharFormat(const CHARFORMATW** ppCF) override;

	//@cmember Get the default paragraph format for the text
	virtual HRESULT TxGetParaFormat(const PARAFORMAT** ppPF) override;

	//@cmember Get the background color for the window
	virtual COLORREF TxGetSysColor(int nIndex) override;

	//@cmember Get the background (either opaque or transparent)
	virtual HRESULT TxGetBackStyle(TXTBACKSTYLE* pstyle) override;

	//@cmember Get the maximum length for the text
	virtual HRESULT TxGetMaxLength(DWORD* plength) override;

	//@cmember Get the bits representing requested scroll bars for the window
	virtual HRESULT TxGetScrollBars(DWORD* pdwScrollBar) override;

	//@cmember Get the character to display for password input
	virtual HRESULT TxGetPasswordChar(_Out_ TCHAR* pch) override;

	//@cmember Get the accelerator character
	virtual HRESULT TxGetAcceleratorPos(LONG* pcp) override;

	//@cmember Get the native size
	virtual HRESULT TxGetExtent(LPSIZEL lpExtent) override;

	//@cmember Notify host that default character format has changed
	virtual HRESULT OnTxCharFormatChange(const CHARFORMATW* pCF) override;

	//@cmember Notify host that default paragraph format has changed
	virtual HRESULT OnTxParaFormatChange(const PARAFORMAT* pPF) override;

	//@cmember Bulk access to bit properties
	virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits) override;

	//@cmember Notify host of events
	virtual HRESULT TxNotify(DWORD iNotify, void* pv) override;

	// East Asia Methods for getting the Input Context
	virtual HIMC TxImmGetContext() override;
	virtual void TxImmReleaseContext(HIMC himc) override;

	//@cmember Returns HIMETRIC size of the control bar.
	virtual HRESULT TxGetSelectionBarWidth(LONG* lSelBarWidth) override;

private:
	/** 通知OnTxPropertyBitsChange接口
	*/
	void OnTxPropertyBitsChange(DWORD dwMask, DWORD dwBits);

	//构造后的初始化
	void Init();

	//初始化字体信息
	void InitCharFormat(const LOGFONT& lf);

	//转换矩形格式
	UiRect MakeUiRect(const RECT& rc);

	// Convert Pixels on the X axis to Himetric
	LONG DXtoHimetricX(LONG dx, LONG xPerInch);

	// Convert Pixels on the Y axis to Himetric
	LONG DYtoHimetricY(LONG dy, LONG yPerInch);

private:
	//引用计数
	ULONG m_cRefs;

	//RichEdit控件的UI层接口
    RichEdit* m_pRichEdit;

	// pointer to Text Services object
	ITextServices* m_pTextServices;

    uint16_t m_dwStyle;			// style bits
    bool m_fEnableAutoWordSel;	// enable Word style auto word selection?
    bool m_fWordWrap;			// Whether control should word wrap
	bool m_fAllowBeep;			// Whether beep is allowed
    bool m_fRichText;			// Whether control is rich text
    bool m_fSaveSelection;		// Whether to save the selection when inactive
    bool m_fInplaceActive;		// Whether control is inplace active
    bool m_fTransparent;		// Whether control is transparent

    LONG m_lSelBarWidth;		// Width of the selection bar
    UiRect m_rcClient;			// Client Rect for this control
    SIZEL m_sizelExtent;		// Extent array
	
    CHARFORMAT2 m_charFormat;  //文字的字体
    PARAFORMAT2	m_paraFormat;	//段落格式
    WCHAR m_chPasswordChar;		// Password character
	bool m_bShowPassword;       //是否显示密码
	bool m_bFlashPasswordChar;  //短暂的显示密码字符，然后再隐藏
};


}//name space ui

#endif // UI_CONTROL_RICHEDIT_HOST_H_
