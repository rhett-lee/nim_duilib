#ifndef UI_CONTROL_RICHEDIT_HOST_WINDOWS_H_
#define UI_CONTROL_RICHEDIT_HOST_WINDOWS_H_

#include "duilib/Core/UiTypes.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

//使用Windows的ITextHost实现
#include <Richedit.h>
#include <TextServ.h>
#include <RichOle.h>

namespace ui
{

class RichEdit;
class DpiManager;

/** RichEditHost 是 Windows 平台 RichEdit 控件的 ITextHost 接口实现，功能要点如下：
*  1. 为文本服务对象（ITextServices）提供宿主窗口相关的回调接口
*  2. 管理文本服务的创建、激活、滚动条、光标与输入法等交互
*  3. 通过引用计数管理对象的生命周期
*/
class DUILIB_API RichEditHost : public ITextHost
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
    void ShutdownTextServices();

    /** 设置控件的客户区矩形（像素单位），内部会同步更新文本服务的尺寸信息
    * @param [in] rc 客户区矩形
    */
    void SetClientRect(const UiRect& rc);

    /** 获取控件的矩形（输出参数形式）
    * @param [out] prc 用于返回控件矩形，传入 nullptr 时函数不做任何处理
    */
    void GetControlRect(UiRect* prc);

    /** 获取控件的矩形
    * @return 返回控件矩形
    */
    UiRect GetControlRect();

    /** 设置是否自动换行（仅在单行文本模式下有效，多行文本模式时无效）
    * @param [in] bWordWrap true 表示自动换行
    */
    void SetWordWrap(bool bWordWrap);

    /** 是否自动换行
    * @return true 表示自动换行
    */
    bool IsWordWrap() const;

    /** 设置是否自动选择单词（Word 风格的按词选择）
    * @param [in] bEnableAutoWordSel true 表示启用
    */
    void SetEnableAutoWordSel(bool bEnableAutoWordSel);

    /** 设置是否只读
    * @param [in] fReadOnly true 表示只读
    */
    void SetReadOnly(bool fReadOnly);

    /** 是否只读
    * @return true 表示只读
    */
    bool IsReadOnly() const;

    /** 设置密码字符，必须为有效的字符，不可以是'\0'
    * @param [in] chPasswordChar 密码字符
    */
    void SetPasswordChar(WCHAR chPasswordChar);

    /** 设置是否为密码模式
    * @param [in] bPassword true 表示密码模式
    */
    void SetPassword(bool bPassword);

    /** 是否为密码模式
    * @return true 表示密码模式
    */
    bool IsPassword() const;

    /** 设置是否以明文显示密码
    * @param [in] bShow true 表示显示密码明文，false 表示隐藏
    */
    void SetShowPassword(bool bShow);

    /** 是否以明文显示密码
    * @return true 表示显示密码明文，false 表示隐藏
    */
    bool IsShowPassword() const;

    /** 设置是否短暂显示密码字符后再隐藏
    * @param [in] bFlash true 表示短暂显示
    */
    void SetFlashPasswordChar(bool bFlash);

    /** 是否短暂显示密码字符
    * @return true 表示短暂显示
    */
    bool IsFlashPasswordChar() const;

    /** 获取密码文本（明文形式）
    * @return 返回密码文本
    */
    DString GetPasswordText() const;

    /** 是否只允许输入数字字符
    * @return true 表示只允许输入数字
    */
    bool IsNumberOnly() const;

    /** 设置是否只允许输入数字字符
    * @param [in] bNumberOnly true 表示只允许输入数字
    */
    void SetNumberOnly(bool bNumberOnly);

    /** 是否支持多行文本
    * @return true 表示支持多行
    */
    bool IsMultiLine() const;

    /** 设置是否支持多行文本
    * @param [in] bMultiLine true 表示支持多行
    */
    void SetMultiLine(bool bMultiLine);

    /** 设置文本的横向对齐方式
    * @param [in] alignType 横向对齐方式
    */
    void SetTextHAlignType(HorAlignType alignType);

    /** 设置文本的纵向对齐方式
    * @param [in] alignType 纵向对齐方式
    */
    void SetTextVAlignType(VerAlignType alignType);

    /** 获取文本的横向对齐方式（用于绘制提示文字）
    * @return 返回横向对齐方式
    */
    HorAlignType GetTextHAlignType() const;

    /** 设置是否启用纵向滚动条
    * @param [in] bEnable true 表示启用
    */
    void SetVScrollBar(bool bEnable);

    /** 设置是否启用横向滚动条
    * @param [in] bEnable true 表示启用
    */
    void SetHScrollBar(bool bEnable);    

    /** 当用户在最后一行按 ENTER 时，自动将文本向上滚动一页
    * @param [in] bEnable true 表示启用
    */
    void SetAutoVScroll(bool bEnable);

    /** 当用户在行尾键入一个字符时，自动将文本向右滚动 10 个字符；
    *   当用户按 Enter 时，控件会将所有文本滚动回零位置
    * @param [in] bEnable true 表示启用
    */
    void SetAutoHScroll(bool bEnable);

    /** 设置文本服务使用的尺寸（HIMETRIC 单位），由 SetClientRect 内部调用
    * @param [in] sizelExtent 文本尺寸
    */
    void SetExtent(SIZEL sizelExtent);

    /** 设置是否允许发出提示音
    * @param [in] bAllowBeep true 表示允许
    */
    void SetAllowBeep(bool bAllowBeep);

    /** 是否允许发出提示音
    * @return true 表示允许
    */
    bool IsAllowBeep() const;

    /** 设置失焦时是否保存选择状态
    * @param [in] fSaveSelection true 表示保存选择状态
    * @return 返回 fSaveSelection 参数值
    */
    bool SetSaveSelection(bool fSaveSelection);

    /** 设置失焦时是否隐藏选择状态
    * @param [in] fHideSelection true 表示隐藏选择状态
    */
    void SetHideSelection(bool fHideSelection);

    /** 失焦时是否隐藏选择状态
    * @return true 表示隐藏选择状态
    */
    bool IsHideSelection() const;

    /** 根据鼠标位置更新文本光标形状
    * @param [in] prc 判断鼠标是否位于其中的矩形，为 nullptr 时使用控件的客户区矩形
    * @param [in] pt 鼠标位置（窗口客户区坐标），为 nullptr 时返回 false
    * @return true 表示鼠标位于指定矩形内并已处理光标设置，false 表示未处理
    */
    bool SetCursor(const UiRect* prc, const UiPoint* pt);

    /** 设置文本背景是否透明
    * @param [in] fTransparent true 表示透明
    */
    void SetTransparent(bool fTransparent);    

    /** 设置选择条宽度（HIMETRIC 单位），负值按 0 处理
    * @param [in] lSelBarWidth 选择条宽度
    */
    void SetSelBarWidth(LONG lSelBarWidth);

    /** 通知文本服务对象退出就位激活状态
    * @return 返回 HRESULT，S_OK 表示成功
    */
    HRESULT OnTxInPlaceDeactivate();

    /** 通知文本服务对象进入就位激活状态
    * @param [in] prcClient 客户区矩形，可为 nullptr
    * @return 返回 HRESULT，S_OK 表示成功
    */
    HRESULT OnTxInPlaceActivate(LPCRECT prcClient);

public:
    // -----------------------------
    //    IUnknown 接口
    // -----------------------------
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual HRESULT _stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    virtual ULONG _stdcall AddRef(void) override;
    virtual ULONG _stdcall Release(void) override;

    // -----------------------------
    //    ITextHost 接口
    // -----------------------------
    /// 重写父类方法，提供个性化功能，请参考父类声明
    //获取宿主窗口的设备上下文(DC)
    virtual HDC TxGetDC() override;

    //释放从宿主获取的设备上下文(DC)
    virtual INT TxReleaseDC(HDC hdc) override;

    //显示或隐藏滚动条
    virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow) override;

    //启用或禁用滚动条
    virtual BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags) override;

    //设置滚动条的滚动范围
    virtual BOOL TxSetScrollRange( INT fnBar,
                                   LONG nMinPos,
                                   INT nMaxPos,
                                   BOOL fRedraw) override;

    //设置滚动条的当前位置
    virtual BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw) override;

    //使指定矩形区域无效，触发重绘
    virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode) override;

    //通知宿主视图内容已变化
    virtual void TxViewChange(BOOL fUpdate) override;

    //创建插入符
    virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) override;

    //显示或隐藏插入符
    virtual BOOL TxShowCaret(BOOL fShow) override;

    //设置插入符位置
    virtual BOOL TxSetCaretPos(INT x, INT y) override;

    //创建指定超时时间的定时器
    virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout) override;

    //销毁定时器
    virtual void TxKillTimer(UINT idTimer) override;

    //滚动指定窗口客户区的内容
    virtual void TxScrollWindowEx( INT dx,
                                   INT dy,
                                   LPCRECT lprcScroll,
                                   LPCRECT lprcClip,
                                   HRGN hrgnUpdate,
                                   LPRECT lprcUpdate,
                                   UINT fuScroll) override;

    //设置鼠标捕获
    virtual void TxSetCapture(BOOL fCapture) override;

    //将焦点设置到文本窗口
    virtual void TxSetFocus() override;

    //设置新的鼠标光标形状
    virtual void TxSetCursor(HCURSOR hcur, BOOL fText) override;

    //将指定点的屏幕坐标转换为客户区坐标
    virtual BOOL TxScreenToClient(LPPOINT lppt) override;

    //将指定点的客户区坐标转换为屏幕坐标
    virtual BOOL TxClientToScreen(LPPOINT lppt) override;

    //请求宿主激活文本服务
    virtual HRESULT TxActivate(LONG* plOldState) override;

    //请求宿主取消激活文本服务
    virtual HRESULT TxDeactivate(LONG lNewState) override;

    //获取窗口客户区矩形
    virtual HRESULT TxGetClientRect(LPRECT prc) override;

    //获取相对于内边距的视图矩形
    virtual HRESULT TxGetViewInset(LPRECT prc) override;

    //获取文本的默认字符格式
    virtual HRESULT TxGetCharFormat(const CHARFORMATW** ppCF) override;

    //获取文本的默认段落格式
    virtual HRESULT TxGetParaFormat(const PARAFORMAT** ppPF) override;

    //获取系统颜色
    virtual COLORREF TxGetSysColor(int nIndex) override;

    //获取背景样式（不透明或透明）
    virtual HRESULT TxGetBackStyle(TXTBACKSTYLE* pstyle) override;

    //获取文本的最大长度
    virtual HRESULT TxGetMaxLength(DWORD* plength) override;

    //获取请求的滚动条标志位
    virtual HRESULT TxGetScrollBars(DWORD* pdwScrollBar) override;

    //获取密码输入时显示的字符
    virtual HRESULT TxGetPasswordChar(_Out_ TCHAR* pch) override;

    //获取快捷键字符的位置
    virtual HRESULT TxGetAcceleratorPos(LONG* pcp) override;

    //获取文本的自然尺寸
    virtual HRESULT TxGetExtent(LPSIZEL lpExtent) override;

    //通知宿主默认字符格式已变化
    virtual HRESULT OnTxCharFormatChange(const CHARFORMATW* pCF) override;

    //通知宿主默认段落格式已变化
    virtual HRESULT OnTxParaFormatChange(const PARAFORMAT* pPF) override;

    //批量获取属性位
    virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits) override;

    //向宿主发送事件通知
    virtual HRESULT TxNotify(DWORD iNotify, void* pv) override;

    //东亚语言：获取输入法上下文
    virtual HIMC TxImmGetContext() override;

    //东亚语言：释放输入法上下文
    virtual void TxImmReleaseContext(HIMC himc) override;

    //获取选择条的 HIMETRIC 宽度
    virtual HRESULT TxGetSelectionBarWidth(LONG* lSelBarWidth) override;

private:
    /** 通知OnTxPropertyBitsChange接口
    */
    void OnTxPropertyBitsChange(DWORD dwMask, DWORD dwBits);

    //构造后的初始化
    void Init();

    /** 转换矩形格式（RECT 转 UiRect）
    * @param [in] rc 待转换的 RECT
    * @return 返回转换后的 UiRect
    */
    UiRect MakeUiRect(const RECT& rc);

    /** 将 X 轴的像素值转换为 HIMETRIC 单位
    * @param [in] dx 像素值
    * @param [in] xPerInch 每英寸的像素数
    * @return 返回 HIMETRIC 单位的值
    */
    LONG DXtoHimetricX(LONG dx, LONG xPerInch);

    /** 将 Y 轴的像素值转换为 HIMETRIC 单位
    * @param [in] dy 像素值
    * @param [in] yPerInch 每英寸的像素数
    * @return 返回 HIMETRIC 单位的值
    */
    LONG DYtoHimetricY(LONG dy, LONG yPerInch);

private:
    //引用计数
    ULONG m_cRefs;

    //RichEdit控件的UI层接口
    RichEdit* m_pRichEdit;

    //文本服务对象指针
    ITextServices* m_pTextServices;

    uint16_t m_dwStyle;         // style bits
    bool m_fEnableAutoWordSel;  // enable Word style auto word selection?
    bool m_fWordWrap;           // Whether control should word wrap
    bool m_fAllowBeep;          // Whether beep is allowed
    bool m_fSaveSelection;      // Whether to save the selection when inactive
    bool m_fInplaceActive;      // Whether control is inplace active
    bool m_fTransparent;        // Whether control is transparent

    LONG m_lSelBarWidth;        // Width of the selection bar
    UiRect m_rcClient;          // Client Rect for this control
    SIZEL m_sizelExtent;        // Extent array
    
    WCHAR m_chPasswordChar;     // Password character
    bool m_bShowPassword;       //是否显示密码
    bool m_bFlashPasswordChar;  //短暂的显示密码字符，然后再隐藏
};


}//name space ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_CONTROL_RICHEDIT_HOST_WINDOWS_H_
