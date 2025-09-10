#ifndef UI_CONTROL_RICHEDIT_WINDOWS_H_
#define UI_CONTROL_RICHEDIT_WINDOWS_H_

#include "duilib/Box/ScrollBox.h"
#include "duilib/Image/Image.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Control/RichEditCtrl_Windows.h"

/** 定义宏开关，决定是否支持RichText功能(默认开启)
*/
#define DUILIB_RICHEDIT_SUPPORT_RICHTEXT 1

namespace ui 
{

/** 字符的索引号范围
*/
struct TextCharRange
{
    int32_t cpMin = -1; //字符的起始索引值
    int32_t cpMax = -1; //字符的结束索引值
};

/** 字符查找的参数
*/
struct FindTextParam
{
    bool bMatchCase = true;      //查找时是否区分大小写
    bool bMatchWholeWord = true; //查找时，是否按词匹配
    bool bFindDown = true;       //是否向后查找，为true表示向后查找，false表示反向查找
    TextCharRange chrg;          //字符的查找范围
    DString findText;            //查找的文本
};

class RichEditHost;
class ControlDropTarget_Windows;
class VBox;
class UILIB_API RichEdit : public ScrollBox
{
    typedef ScrollBox BaseClass;
public:
    explicit RichEdit(Window* pWindow);
    RichEdit(const RichEdit& r) = delete;
    RichEdit& operator=(const RichEdit& r) = delete;
    virtual ~RichEdit() override;
public:
    //基类的虚函数重写
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& pstrName, const DString& pstrValue) override;
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;
    virtual void SetPos(UiRect rc) override;
    virtual void SetScrollPos(UiSize64 szPos) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void ClearImageCache() override;
    virtual void HandleEvent(const EventArgs& msg) override;
    virtual UiEstSize EstimateSize(UiSize szAvailable) override;
    virtual UiSize EstimateText(UiSize szAvailable) override;

public:
    /** 设置控件的文本, 会触发文本变化事件
     * @param [in] strText 要设置的文本内容
     */
    void SetText(const DStringW& strText);
    void SetText(const DStringA& strText);

    /** 设置控件的文本，不触发文本变化事件
     * @param [in] strText 要设置的文本内容
     */
    void SetTextNoEvent(const DString& strText);

    /** 设置控件的文本对应 ID
     * @param[in] strTextId 要设置的 ID，该 ID 必须在加载的语言文件中存在
     */
    void SetTextId(const DString& strTextId);

    /** 获取控件中的文本
     * @return 返回控件中的文本内容
     */
    DString GetText() const;

    /** 获取内容的长度(Unicode编码，字符个数)
     * @return 返回内容长度
     */
    int32_t GetTextLength() const;

    /** 获取当前设置的字体索引
     * @return 返回字体索引（对应 global.xml 中字体的顺序）
     */
    DString GetFontId() const;

    /** 设置字体索引
     * @param[in] index 要设置的字体索引（对应 global.xml 中字体的顺序）
     */
    void SetFontId(const DString& strFontId);

    /** 获取字体信息(字体大小是进行过DPI缩放处理的)
    * @return RichText模式时，返回的是当前所选择的文本的字体信息，否则返回默认的字体信息
    */
    UiFont GetFontInfo() const;

    /** 设置字体信息（优先级高，会覆盖通过SetFontId设置的字体）
    * @param [in] fontInfo 字体信息，字体大小是进行过DPI缩放处理的
    */
    bool SetFontInfo(const UiFont& fontInfo);

    /** 获取当前使用的字体ID
    * @return 返回值与GetFontId()函数相同
    */
    DString GetCurrentFontId() const;

    /** 设置正常文本颜色
     * @param[in] dwTextColor 要设置的文本颜色
     */
    void SetTextColor(const DString& dwTextColor);

    /** 获取正常文本颜色
     */
    DString GetTextColor() const;

    /** 获取所选文本颜色（仅富文本模式有效）
    */
    DString GetSelectionTextColor() const;

    /** 设置所选文本的颜色（仅富文本模式有效）
     * @param[in] textColor 要设置的文本颜色
     */
    void SetSelectionTextColor(const DString& textColor);

    /** 设置Disabled状态的文本颜色
     * @param[in] dwTextColor 要设置的文本颜色
     */
    void SetDisabledTextColor(const DString& dwTextColor);

    /** 获取Disabled状态的文本颜色
     */
    DString GetDisabledTextColor() const;

public:
    /** 设置是否显示提示文字
 * @param[in] bPrompt 设置为 true 为显示，false 为不显示
 */
    void SetPromptMode(bool bPrompt);

    /** 获取提示文字
     */
    DString GetPromptText() const;

    /** 获取提示文字
     * @return 返回 UTF8 格式的提示文字
     */
    std::string GetUTF8PromptText() const;

    /** 设置提示文字
     * @param[in] strText 要设置的提示文字
     */
    void SetPromptText(const DString& strText);

    /** 设置提示文字
     * @param[in] strText 要设置的 UTF8 格式提示文字
     */
    void SetUTF8PromptText(const std::string& strText);

    /** 设置提示文字 ID
     * @param[in] strText 要设置的提示文字 ID，该 ID 必须在加载的语言文件中存在
     */
    void SetPromptTextId(const DString& strTextId);

    /** 设置提示文字 ID
     * @param[in] strText 要设置的 UTF8 格式提示文字 ID，该 ID 必须在加载的语言文件中存在
     */
    void SetUTF8PromptTextId(const std::string& strTextId);

public:
    /** 设置文字内边距信息
     * @param[in] padding 内边距信息
     * @param[in] bNeedDpiScale 是否支持DPI缩放
     */
    void SetTextPadding(UiPadding padding, bool bNeedDpiScale);

    /** 获取文字内边距
     */
    UiPadding GetTextPadding() const;

    /** 是否为多行文本
    */
    bool IsMultiLine() const;

    /** 设置是否为多行文本
    */
    void SetMultiLine(bool bMultiLine);

    /** 获取超出矩形区域的文本显示方式
     * @return 返回 true 时并且在多行模式下内容被换行显示，false 则表示截断显示
     */
    bool IsWordWrap() const;

    /** 设置超出矩形区域的文本显示方式
     * @param[in] bWordWrap 为 true 时并且在多行模式下内容被换行显示，false 则表示截断显示
     */
    void SetWordWrap(bool bWordWrap);

    /** 判断是否接受 TAB 按键消息
     * @return 返回 true 表示接受，TAB键会作为输入字符转换为文本；false 表示不接受，TAB键会作为控件的快捷键，而不作为输入文本字符
     */
    virtual bool IsWantTab() const override;

    /** 判断是否接受 TAB 按键消息
     * @param[in] bWantTab 为 true 表示接受，TAB键会作为输入字符转换为文本；false 表示不接受，TAB键会作为控件的快捷键，而不作为输入文本字符
     */
    void SetWantTab(bool bWantTab);

    /** 判断是否接受 回车键 按键消息
     * @return 返回 true 表示接受，回车键会作为输入字符转换为文本；false 表示不接受，回车键会作为控件的快捷键，而不作为输入文本字符
     */
    bool IsWantReturn() const;

    /** 设置是否接受 回车键 按键消息
     * @param[in] bWantReturn 为 true 表示接受，回车键会作为输入字符转换为文本；false 表示不接受，回车键会作为控件的快捷键，而不作为输入文本字符
     */
    void SetWantReturn(bool bWantReturn);

    /** 判断是否接受CTRL+RETURN 组合键消息
     * @return 返回 true 表示接受，false 表示不接受
     */
    bool IsWantCtrlReturn() const;

    /** 设置是否接受CTRL+RETUREN 组合键消息
     * @param[in] bWantCtrlReturn 为 true 则接受该消息，false 为不接受
     */
    void SetWantCtrlReturn(bool bWantCtrlReturn);

    /** 是否是只读状态
     * @return 返回 true 为只读状态，否则为 false
     */
    bool IsReadOnly() const;

    /** 设置控件为只读状态
     * @param[in] bReadOnly 设置 true 让控件变为只读状态，false 为可写入状态
     */
    void SetReadOnly(bool bReadOnly);

    /** 是否是密码状态控件
     * @return 返回 true 表示是密码控件，否则为 false
     */
    bool IsPasswordMode() const;

    /** 设置控件为密码控件（显示 ***）
     * @param[in] bPasswordMode 设置为 true 让控件显示内容为 ***，false 为显示正常内容
     */
    void SetPasswordMode(bool bPasswordMode);

    /** 设置是否显示密码
    */
    void SetShowPassword(bool bShow);

    /** 是否显示密码
    */
    bool IsShowPassword() const;

    /** 设置密码字符
    */
    void SetPasswordChar(DStringW::value_type ch);

    /** 设置是否对输入的字符短暂显示再隐藏（仅当IsShowPassword()为true，即密码模式的时候有效）
    */
    void SetFlashPasswordChar(bool bFlash);

    /** 获取是否对输入的字符短暂显示再隐藏
    */
    bool IsFlashPasswordChar() const;

    /** 是否只允许输入数字
    */
    bool IsNumberOnly() const;

    /** 设置是否只允许输入数字
    */
    void SetNumberOnly(bool bNumberOnly);

    /** 设置允许的最大数字(仅当IsNumberOnly()为true的时候有效)
    */
    void SetMaxNumber(int32_t maxNumber);

    /** 获取允许的最大数字
    */
    int32_t GetMaxNumber() const;

    /** 设置允许的最小数字(仅当IsNumberOnly()为true的时候有效)
    */
    void SetMinNumber(int32_t minNumber);

    /** 获取允许的最小数字
    */
    int32_t GetMinNumber() const;

    /** 获取数字的格式（64位有符号整型的格式, 比如"I64d"等）
    */
    void SetNumberFormat64(const DString& numberFormat);

    /** 获取数字的格式（64位有符号整型的格式）
    */
    DString GetNumberFormat64() const;

    /** 获取限制字符数量
     * @return 返回限制字符数量
     */
    int32_t GetLimitText() const;

    /** 设置限制字符数量
     * @param [in] iChars 要限制的字符数量
     */
    void SetLimitText(int32_t iChars);

    /** 获取允许输入哪些字符
    */
    DString GetLimitChars() const;

    /** 设置允许输入哪些字符，比如颜色值可以设置：limit_chars="#0123456789ABCDEFabcdef"
    * @param [in] limitChars 允许输入的字符列表
    */
    void SetLimitChars(const DString& limitChars);

    /** 获取焦点状态下的图片
    * @return 返回焦点状态下的图片
    */
    DString GetFocusedImage();

    /** 设置焦点状态下的图片
     * @param[in] strImage 要设置的图片位置
     */
    void SetFocusedImage(const DString& strImage);

    /** 设置是否允许通过Ctrl + 滚轮来调整缩放比例
    */
    void SetEnableWheelZoom(bool bEnable);

    /** 获取是否允许通过Ctrl + 滚轮来调整缩放比例
    */
    bool IsEnableWheelZoom(void) const;

    /** 是否允许使用默认的右键菜单
    */
    void SetEnableDefaultContextMenu(bool bEnable);

    /** 是否允许使用默认的右键菜单
    */
    bool IsEnableDefaultContextMenu() const;

    /** 设置是否支持Spin控件
    * @param [in] bEnable true表示支持Spin控件，false表示不支持Spin控件
    * @param [in] spinClass Spin控件的Class属性，字符串需要包含3个值，具体设置参见：global.xml里面的rich_edit_spin设置
    *             取值举例：rich_edit_spin_box,rich_edit_spin_btn_up,rich_edit_spin_btn_down
    * @param [in] nMin 表示设置数字的最小值
    * @param [in] nMax 表示设置数字的最大值，如果 nMin和nMax同时为0, 表示不设置数字的最小值和最大值
    */
    bool SetEnableSpin(bool bEnable, const DString& spinClass, int32_t nMin = 0, int32_t nMax = 0);

public:
    /** 创建光标
     * @param [in] xWidth 光标宽度
     * @param [in] yHeight 光标高度
     */
    void CreateCaret(int32_t xWidth, int32_t yHeight);

    /** 获取光标的宽度和高度
    */
    void GetCaretSize(int32_t& xWidth, int32_t& yHeight) const;

    /** 设置是否显示光标
     * @param [in] fShow 设置 true 为显示，false 为不显示
     */
    void ShowCaret(bool fShow);

    /** 设置光标颜色
     * @param[in] dwColor 要设置的颜色值，该值必须在 global.xml 中存在
     */
    void SetCaretColor(const DString& dwColor);

    /** 获取光标颜色
     * @return 返回光标颜色
     */
    DString GetCaretColor() const;

    /** 获取光标矩形位置
     * @return 返回光标矩形位置
     */
    UiRect GetCaretRect() const;

    /** 设置光标位置
     * @param [in] xPos X 轴坐标
     * @param [in] yPos Y 轴坐标
     */
    void SetCaretPos(int32_t xPos, int32_t yPos);

    /** 获取光标位置
     * @param [out] xPos X 轴坐标
     * @param [out] yPos Y 轴坐标
    */
    void GetCaretPos(int32_t& xPos, int32_t& yPos) const;

    /** 设置只读模式不显示光标
    */
    void SetNoCaretReadonly();

    /** 设置是否使用Control的光标
    */
    void SetUseControlCursor(bool bUseControlCursor);

public:
    /** 监听回车按键按下事件
     * @param[in] callback 回车被按下的自定义回调函数
     */
    void AttachReturn(const EventCallback& callback) { AttachEvent(kEventReturn, callback); }

    /** 监听ESC按键按下事件
     * @param[in] callback 回车被按下的自定义回调函数
     */
    void AttachEsc(const EventCallback& callback) { AttachEvent(kEventEsc, callback); }

    /** 监听 TAB 按键按下事件
     * @param[in] callback TAB 被按下的自定义回调函数
     */
    void AttachTab(const EventCallback& callback) { AttachEvent(kEventTab, callback); }

    /* 监听缩放比例变化事件
     * @param[in] callback 文本被修改后的自定义回调函数
     */
    void AttachZoom(const EventCallback& callback) { AttachEvent(kEventZoom, callback); }

    /* 监听文本被修改事件
     * @param[in] callback 文本被修改后的自定义回调函数
     */
    void AttachTextChange(const EventCallback& callback) { AttachEvent(kEventTextChange, callback); }

    /* 监听文本选择变化事件
     * @param[in] callback 文本选择变化后的自定义回调函数
     */
    void AttachSelChange(const EventCallback& callback);

    /** 监听超级链接被点击事件
     * @param[in] callback 超级链接被点击后的回调函数
     */
    void AttachLinkClick(const EventCallback& callback) { AttachEvent(kEventLinkClick, callback); }

public:
    /** 获取修改标志
     * @return 返回 true 为设置了修改标志，否则为 false
     */
    bool GetModify() const;

    /** 设置修改标志
     * @param[in] bModified 设置为 true 表示文本已经被修改，false 为未修改，默认为 true
     */
    void SetModify(bool bModified = true);

    /** 全选
     * @return 返回选择的内容数量
     */
    int32_t SetSelAll();

    /** 不选择任何内容
     */
    void SetSelNone();

    /** 设置失去焦点后是否取消选择项
     * @param[in] bOnSel 设置为 true 表示取消选择项，false 为不取消
     */
    void SetNoSelOnKillFocus(bool bOnSel);

    /** 设置获取焦点后是否选择所有内容
     * @param[in] bSelAll 设置 true 表示在获取焦点时选择所有内容，false 为不选择
     */
    void SetSelAllOnFocus(bool bSelAll);

    /** 获取所选文本的起始位置和结束位置
     * @param[in] nStartChar 返回起始位置
     * @param[in] nEndChar 返回结束位置
     */
    void GetSel(int32_t& nStartChar, int32_t& nEndChar) const;

    /** 选择一部分内容
     * @param[in] nStartChar 要选择的起始位置
     * @param[in] nEndChar 要选择的结束位置
     * @return 返回选择的文字数量
     */
    int32_t SetSel(int32_t nStartChar, int32_t nEndChar);

    /** 替换所选内容
     * @param[in] lpszNewText 要替换的文字
     * @param[in] bCanUndo 是否可以撤销，true 为可以，否则为 false
     */
    void ReplaceSel(const DString& lpszNewText, bool bCanUndo);

    /** 获取所选文字内容
     * @return 返回所选文字内容
     */
    DString GetSelText() const;

    /** 获取指定范围的内容
     * @param[in] nStartChar 起始位置
     * @param[in] nEndChar 结束为止
     * @return 返回设置的指定位置的内容
     */
    DString GetTextRange(int32_t nStartChar, int32_t nEndChar) const;

    /** 设置隐藏或显示选择的文本
     * @param [in] bHide 是否显示，true 为隐藏，false 为显示
     * @param [in] bChangeStyle 是否修改样式，true 为修改，false 为不修改
     */
    void HideSelection(bool bHide = true, bool bChangeStyle = false);

    /** 是否可以Redo
    */
    bool CanRedo() const;

    /** 重做操作
     * @return 成功返回 true，失败返回 false
     */
    bool Redo();

    /** 是否可撤销
    */
    bool CanUndo() const;

    /** 撤销操作
     * @return 成功返回 true，失败返回 false
     */
    bool Undo();

    /** 删除当前所选内容
     */
    void Clear();

    /** 复制所选内容
     */
    void Copy();

    /** 剪切所选内容
     */
    void Cut();

    /** 粘贴
     */
    void Paste();

    /** 检测是否可以粘贴
    */
    bool CanPaste() const;

    /** 获取总行数
     * @return 返回总行数
     */
    int32_t GetLineCount() const;

    /** 获取一行数据
     * @param[in] nIndex 要获取的行数
     * @param[in] nMaxLength 要获取当前行最大的数据长度
     * @return 返回获取的一行数据
     */
    DString GetLine(int32_t nIndex, int32_t nMaxLength) const;

    /** 获取指定行的第一个字符索引
     * @param[in] nLine 要获取第几行数据，默认为 -1，表示当前插入点的行
     * @return 返回指定行的第一个字符索引
     */
    int32_t LineIndex(int32_t nLine = -1) const;

    /** 获取指定行的数据长度
     * @param[in] nLine 要获取第几行数据，默认为 -1，表示当前插入点的行
     * @return 返回指定行的数据长度
     */
    int32_t LineLength(int32_t nLine = -1) const;

    /** 滚动文本
     * @param[in] nLines 指定垂直滚动方向
     * @return 成功返回 true，失败返回 false
     */
    bool LineScroll(int32_t nLines);

    /** 获取指定字符所在行号
     * @param[in] nIndex 字符的索引位置
     * @return 返回当前字符所在的行号
     */
    int32_t LineFromChar(int32_t nIndex) const;

    /** 检索编辑控件中指定字符的工作区坐标。
     * @param[in] nChar 字符索引位置
     * @return 返回值包含字符的客户端区域坐标。
     */
    UiPoint PosFromChar(int32_t nChar) const;

    /** 获取有关距离编辑控件客户区中指定点最近的字符的信息
     * @param[in] pt 坐标信息
     * @return 返回值指定了距指定点最近字符的从零开始的字符索引。 如果指定点超出控件中的最后一个字符，则返回值会指示编辑控件中的最后一个字符。
     */
    int32_t CharFromPos(UiPoint pt) const;

    /** 清空撤销列表
     */
    void EmptyUndoBuffer();

    /** 设置撤销列表容纳的内容数量
     * @param [in] nLimit
     * @return 返回设置后的撤销列表可容纳内容数量
     */
    uint32_t SetUndoLimit(uint32_t nLimit);

public:
    /** 向上滚动滚动条
     * @param[in] deltaValue 未使用
     * @param[in] withAnimation 未使用
     */
    virtual void LineUp(int32_t deltaValue = DUI_NOSET_VALUE, bool withAnimation = true) override;

    /** 向下滚动滚动条
     * @param[in] deltaValue 未使用
     * @param[in] withAnimation 未使用
     */
    virtual void LineDown(int32_t deltaValue = DUI_NOSET_VALUE, bool withAnimation = true) override;

    /** 向上翻页
     */
    virtual void PageUp() override;

    /** 向下翻页
     */
    virtual void PageDown() override;

    /** 返回到顶端
     */
    virtual void HomeUp() override;

    /** 滚动到最下方位置
     * @param[in] arrange 未使用
     * @param[in] withAnimation 未使用
     */
    virtual void EndDown(bool arrange = true, bool withAnimation = true) override;

    /** 向左滚动滚动条
     * @param[in] deltaValue 未使用
     */
    virtual void LineLeft(int32_t deltaValue = DUI_NOSET_VALUE) override;

    /** 向右滚动滚动条
     * @param[in] deltaValue 未使用
     */
    virtual void LineRight(int32_t deltaValue = DUI_NOSET_VALUE) override;

    /** 水平向左翻页
     */
    virtual void PageLeft() override;

    /** 水平向右翻页
     */
    virtual void PageRight() override;

    /** 返回到最左侧
     */
    virtual void HomeLeft() override;

    /** 返回到最后侧
     */
    virtual void EndRight() override;

public:
    /** 屏幕坐标转换为客户区坐标
    */
    virtual bool ScreenToClient(UiPoint& pt) override;

    /** 客户区坐标转换为屏幕坐标
    */
    virtual bool ClientToScreen(UiPoint& pt) override;

    /** 将字体大小转换成Rich Edit控件的字体高度
    */
    int32_t ConvertToFontHeight(int32_t fontSize) const;

    /** 查找文本
    * @param [in] findParam 查找参数
    * @param [out] chrgText 匹配的文本，字符的索引号范围
    */
    bool FindRichText(const FindTextParam& findParam, TextCharRange& chrgText) const;

    /** 获取文本内容，并转换为数字
    */
    int64_t GetTextNumber() const;

    /** 将数字转换为文本，设置文本内容
    */
    void SetTextNumber(int64_t nValue);

    /** 调整文本数字值
    */
    void AdjustTextNumber(int32_t nDelta);

    /** 设置是否隐藏选择项
     */
    void SetHideSelection(bool fHideSelection);

    /** 是否隐藏选择的文本（当控件处于非激活状态时，是否隐藏选择内容）
    */
    bool IsHideSelection() const;

    /** 设置焦点状态时，底部边框的大小
    * @param [in] nBottomBorderSize 底部边框的大小(未经DPI缩放)
    * @param [in] bNeedDpiScale 是否支持DPI缩放
    */
    void SetFocusBottomBorderSize(int32_t nBottomBorderSize);

    /** 获取焦点状态时，底部边框的大小(未经DPI缩放)
    */
    int32_t GetFocusBottomBorderSize() const;

    /** 设置焦点状态时，底部边框的颜色
    */
    void SetFocusBottomBorderColor(const DString& bottomBorderColor);

    /** 获取焦点状态时，底部边框的颜色
    */
    DString GetFocusBottomBorderColor() const;

    /** 设置是否允许拖放功能
    */
    virtual void SetEnableDragDrop(bool bEnable) override;

    /** 判断是否已经允许拖放功能
    */
    virtual bool IsEnableDragDrop() const override;

    /** 获取拖放接口
    * @return 返回拖放目标接口，如果返回nullptr表示不支持拖放操作
    */
    virtual ControlDropTarget_Windows* GetControlDropTarget() override;

    /** 获取拖放接口（SDL）
    * @return 返回拖放目标接口，如果返回nullptr表示不支持拖放操作
    */
    virtual ControlDropTarget_SDL* GetControlDropTarget_SDL() override;

#ifdef DUILIB_RICHEDIT_SUPPORT_RICHTEXT
public:
    /** 是否是富文本模式
     * @return 返回 true 为富文本模式：支持丰富的文本格式，支持RTF格式
               返回 false 为纯文本模式：纯文本控件中的文本只能有一种格式
     */
    bool IsRichText() const;

    /** 设置控件为富文本模式
     * @param[in] bRichText 设置 true 为富文本模式，false 为纯文本模式
     */
    void SetRichText(bool bRichText);

    /** 设置是否保存所选内容的边界
    */
    void SetSaveSelection(bool fSaveSelection);

    /** 获取控件的选择类型
     * @return 返回控件的选择类型，参考：https://docs.microsoft.com/en-us/windows/desktop/controls/em-selectiontype
        SEL_TEXT: Text.
        SEL_OBJECT: At least one COM object.
        SEL_MULTICHAR: More than one character of text.
        SEL_MULTIOBJECT: More than one COM object.
     */
    WORD GetSelectionType() const;

    /** 查找文本
    */
    int32_t FindRichText(DWORD dwFlags, FINDTEXTW& ft) const;

    /** 查找文本
    */
    int32_t FindRichText(DWORD dwFlags, FINDTEXTEXW& ft) const;

    /** 设置缩放百分比
    * @param [in] fZoomRatio 缩放比例，比如"100"表示100%, 无缩放; "200"表示缩放比例为200%
    */
    void SetZoomPercent(uint32_t nZoomPercent);

    /** 获取缩放百分比
    */
    uint32_t GetZoomPercent() const;

    /** 获取是否开启了自动 URL 检测(从RichEditHost读取)
     * @return 返回 true 表示开启了自动检测，否则为 false
     */
    bool GetAutoURLDetect() const;

    /** 设置是否开启 URL 自动检测
     * @param[in] bAutoDetect 设置为 true 表示检测 URL，false 为不检测，默认为 true
     * @return 返回 true 为设置成功，false 为失败
     */
    bool SetAutoURLDetect(bool bAutoDetect = true);

    /** 获取控件的事件掩码
     * @return 返回事件掩码
     */
    DWORD GetEventMask() const;

    /** 设置控件的事件掩码
     * @param[in] dwEventMask 要设置的事件掩码值
     * @return 返回设置之前的事件掩码值
     */
    DWORD SetEventMask(DWORD dwEventMask);

    /** 选择一部分内容
     * @param[in] cr 要选择的文字起始位置和结束位置
     * @return 返回选择的文字数量
     */
    int32_t SetSel(CHARRANGE& cr);

    /** 是否允许发出Beep声音
    */
    bool GetAllowBeep() const;

    /** 设置是否允许发出Beep声音
    */
    void SetAllowBeep(bool bAllowBeep);

    /** 设置光标到可见位置
     */
    void ScrollCaret();

    /** 插入文字
     * @param[in] nInsertAfterChar 要插入的位置
     * @param[in] text 要插入的文本
     * @param[in] bCanUndo 是否可以撤销，true 为可以，否则为 false，默认为 false
     * @return 返回插入后的文本位置
     */
    int32_t InsertText(int32_t nInsertAfterChar, const DString& text, bool bCanUndo = false);

    /** 追加文字
     * @param [in] text 要追加的文字
     * @param [in] bCanUndo 是否可以撤销，true 为可以，否则为 false，默认为 false
     * @param [in] bScrollBottom 是否将视图滚动到底部
     * @return 返回追加后的文字位置
     */
    int32_t AppendText(const DString& text, bool bCanUndo = false, bool bScrollBottom = true);

    /** 获取字符格式
     * @param[out] cf 返回获取的字符格式
     * @return 返回参数 cf 中 dwMask 的值
     */
    DWORD GetDefaultCharFormat(CHARFORMAT2W& cf) const;

    /** 设置默认的字符格式
     * @param[in] cf 要设置字符格式
     * @return 返回 true 表示成功，false 为失败
     */
    bool SetDefaultCharFormat(CHARFORMAT2W& cf);

    /** 获取被选择的字符格式
     * @param[out] cf 返回获取的字符格式
     * @return 返回参数 cf 中 dwMask 的值
     */
    DWORD GetSelectionCharFormat(CHARFORMAT2W& cf) const;

    /** 设置被选择的字符格式
     * @param[in] cf 要设置的字符格式
     * @return 返回 true 表示成功，false 为失败
     */
    bool SetSelectionCharFormat(CHARFORMAT2W& cf);

    /** 设置当前插入点的单词格式
     * @param[in] cf 要设置的单词格式
     * @return 成功返回 true，失败返回 false
     */
    bool SetWordCharFormat(CHARFORMAT2W& cf);

    /** 获取当前段落格式
     * @param[out] pf 返回当前段落格式
     * @return 返回 pf 参数的 dwMask 成员
     */
    DWORD GetParaFormat(PARAFORMAT2& pf) const;

    /** 设置当前段落格式
     * @param[in] pf 要设置的段落格式样式
     * @return 成功返回 true，否则返回 false
     */
    bool SetParaFormat(PARAFORMAT2& pf);

    /** 获取所选文本的起始位置和结束位置
     * @param[out] cr 返回起始位置和结束位置
     */
    void GetSel(CHARRANGE& cr) const;

    /** 检测是否可以粘贴指定剪切板格式
     * @param[in] nFormat 要检测的格式
     * @return 可以返回 true，否则返回 false
     */
    BOOL CanPaste(UINT nFormat);

    /** 在控件中粘贴特定的剪贴板格式
     * @param[in] uClipFormat 指定剪切板格式
     * @param[in] dwAspect 指定展示形式
     * @param[in] hMF 如果 dwAspect 为 DVASPECT_ICON，该函数应该包含图标句柄
     */
    void PasteSpecial(UINT uClipFormat, DWORD dwAspect = 0, HMETAFILE hMF = 0);

    /** 增加一个回调用于控制输入内容
     * @param[in] nFormat 指定数据格式的替换选项，见：https://docs.microsoft.com/en-us/windows/desktop/controls/em-streamin
     * @param[in] es 包含自定义回调的结构体
     * @return 返回读入数据流的数据大小
     */
    long StreamIn(UINT nFormat, EDITSTREAM& es);

    /** 指定一个回调用于控制输出内容
     * @param[in] nFormat 指定数据格式的替换选项，见：https://docs.microsoft.com/en-us/windows/desktop/controls/em-streamin
     * @param[in] es 包含自定义回调的结构体
     * @return 返回写入数据流的数据大小
     */
    long StreamOut(UINT nFormat, EDITSTREAM& es);

    /** 添加带颜色的文本
     * @param[in] str 文本内容
     * @param[in] color 颜色值，该值必须在 global.xml 中存在
     */
    void AddColorText(const DString& str, const DString& color);

    /** 添加一个指定字体带有文字颜色的超链接
     * @param[in] str 文字内容
     * @param[in] color 文字颜色
     * @param[in] linkInfo 链接地址
     * @param[in] font 字体索引
     */
    void AddLinkColorTextEx(const DString& str, const DString& color, const DString& linkInfo = _T(""), const DString& strFontId = _T(""));

#endif

protected:

    //一些基类的虚函数
    virtual bool CanPlaceCaptionBar() const override;
    virtual void OnInit() override;
    virtual uint32_t GetControlFlags() const override;

    //消息处理函数
    virtual bool OnSetCursor(const EventArgs& msg) override;
    virtual bool OnSetFocus(const EventArgs& msg) override;
    virtual bool OnKillFocus(const EventArgs& msg) override;
    virtual bool OnChar(const EventArgs& msg) override;
    virtual bool OnKeyDown(const EventArgs& msg) override;
    virtual bool OnImeStartComposition(const EventArgs& msg) override;
    virtual bool OnImeEndComposition(const EventArgs& msg) override;
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;
    virtual void PaintBorder(IRender* pRender) override;

    /** 调整内部所有子控件的位置信息
     * @param[in] items 控件列表
     */
    virtual void ArrangeChild(const std::vector<Control*>& items) const;

    /** 设置可用状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetEnabled(bool bChanged) override;

private:
    /** 显示RichEdit上的菜单
    * @param [in] point 客户区的坐标
    */
    void ShowPopupMenu(const ui::UiPoint& point);

    /** 判断一个字符，是否在限制字符列表中
    */
    bool IsInLimitChars(DStringW::value_type charValue) const;

    /** 判断是否可以进行粘贴操作(判断是否有字符限制、数字限制)
    * @return 如果返回true, 表示不可用进行粘贴操作
    */
    bool IsPasteLimited() const;

private:
    /** 触发文本变化事件
    */
    void OnTextChanged();

    /** 设置Spin功能的Class名称
    */
    bool SetSpinClass(const DString& spinClass);

    /** 开始启动调整文本数字值的定时器
    */
    void StartAutoAdjustTextNumberTimer(int32_t nDelta);

    /** 开始自动调整文本数字值
    */
    void StartAutoAdjustTextNumber(int32_t nDelta);

    /** 结束自动调整文本数字值
    */
    void StopAutoAdjustTextNumber();

private:
    /** 设置清除按钮功能的Class名称
    */
    void SetClearBtnClass(const DString& btnClass);

    /** 设置显示密码按钮功能的Class名称
    */
    void SetShowPasswordBtnClass(const DString& btnClass);

    /** 设置字体ID
    */
    void SetFontIdInternal(const DString& fontId);

    /** 设置字体颜色
    */
    void SetTextColorInternal(const UiColor& textColor);

    /** 获取字体对应的格式
    */
    void GetCharFormat(const DString& fontId, CHARFORMAT2W& cf) const;

    //文本横向和纵向对齐方式
    void SetHAlignType(HorAlignType alignType);
    void SetVAlignType(VerAlignType alignType);

    /** 使用私有的DC绘制
    */
    void PaintRichEdit(IRender* pRender, const UiRect& rcPaint);

    /** 绘制光标
     * @param[in] pRender 绘制引擎
     * @param[in] rcPaint 绘制位置
     */
    void PaintCaret(IRender* pRender, const UiRect& rcPaint);

    /** 切换光标是否显示
    */
    void ChangeCaretVisiable();

    /** 绘制提示文字
     * @param[in] pRender 绘制引擎
     */
    void PaintPromptText(IRender* pRender);

    /** 转发消息控件的实现
    */
    void OnMouseMessage(uint32_t uMsg, const EventArgs& msg);

    /** 获取粘贴板字符串
    */
    static void GetClipboardText(DStringW& out);

private:
    //一组供RichEditHost使用的函数
    friend class RichEditHost;

    /** Notify消息的处理
    */
    void OnTxNotify(DWORD iNotify, void* pv);

    /** 获取关联的窗口局部
    */
    HWND GetWindowHWND() const;

    /** 获取绘制的设备上下文
    */
    HDC GetDrawDC() const;

    /** 获取文本区域的矩形范围
    */
    UiSize GetNaturalSize(LONG width, LONG height);

    /** 设置一个定时器（由内部回调使用）
    */
    void SetTimer(UINT idTimer, UINT uTimeout);

    /** 取消一个定时器（由内部回调使用）
    */
    void KillTimer(UINT idTimer);

    /** 获取当前缩放比， 按缩放比例分子/分母显示的缩放：1/64 < (wParam / lParam) < 64
     * @param[out] nNum 缩放比率分子
     * @param[out] nDen 缩放比率分母
     * @return 如果处理了消息则返回 TRUE
     */
    bool GetZoom(int& nNum, int& nDen) const;

    /** 设置缩放比
     * @param[in] nNum 缩放比率分子，取值范围：[0, 64]
     * @param[in] nDen 缩放比率分母，取值范围：[0, 64]
     * @return 成功返回 true，失败返回 false
     */
    bool SetZoom(int nNum, int nDen);

    /** 恢复缩放到初始状态
     * @return 成功返回 true，否则返回 false
     */
    bool SetZoomOff();

    /** 获得焦点时，全选
    */
    void CheckSelAllOnFocus();

private:
    //RichEdit控制辅助工具类
    RichEditCtrl m_richCtrl;

    //RichEdit Host类
    RichEditHost* m_pRichHost;

    bool m_bVScrollBarFixing;   //滚动条修正标志
    bool m_bWantTab;            //是否接收TAB键，如果为true的时候，TAB键会当作文本输入，否则过滤掉TAB键
    bool m_bWantReturn;         //是否接收回车键，如果为true的时候，回车键会当作文本输入，否则过滤掉回车键
    bool m_bWantCtrlReturn;     //是否接收Ctrl + 回车键，如果为true的时候，回车键会当作文本输入，否则过滤掉回车键
     
    bool m_bSelAllEver;         //只在获取焦点后的第一次鼠标弹起全选

    bool m_bNoSelOnKillFocus;   //失去焦点的时候，取消文本选择（针对 m_bEnabled && IsReadOnly()）
    bool m_bSelAllOnFocus;      //获取焦点的时候，全选文本（针对 m_bEnabled && !IsReadOnly()）
    bool m_bHideSelection;      //是否隐藏选择状态
    bool m_bContextMenuShown;   //是否正在显示右键菜单

    bool m_bIsComposition;      //输入法合成窗口是否可见

private:
    bool m_bNoCaretReadonly;    //只读模式下，不显示光标
    bool m_bIsCaretVisiable;    //光标是否可见
    int32_t m_iCaretPosX;       //光标X坐标
    int32_t m_iCaretPosY;       //光标Y坐标
    int32_t m_iCaretWidth;      //光标宽度
    int32_t m_iCaretHeight;     //光标高度
    UiString m_sCaretColor;     //光标颜色

    WeakCallbackFlag m_drawCaretFlag;   //绘制光标的定时器生命周期
    std::map<UINT, WeakCallbackFlag> m_timerFlagMap; //内部定时器

private:
    UiString m_sFontId;                 //字体ID
    UiString m_sTextColor;              //正常文本颜色
    UiString m_sDisabledTextColor;      //Disabled状态的文本颜色
    UiPadding16 m_rcTextPadding;        //文本内边距

    UiString m_sFocusBottomBorderColor; //焦点状态时，底部边框的颜色
    
    UiString m_sPromptColor;            //提示文字颜色
    UiString m_sPromptText;             //提示文本内容（只有编辑框为空的时候显示）
    UiString m_sPromptTextId;           //提示文字ID
    bool m_bAllowPrompt;                //是否支持提示文字

    uint8_t m_nFocusBottomBorderSize;    //焦点状态时，底部边框的大小

private:
    /** 是否使用Control设置的光标
    */
    bool m_bUseControlCursor;

    /** 是否允许通过Ctrl + 滚轮来调整缩放比例
    */
    bool m_bEnableWheelZoom;

    /** 是否允许使用默认的右键菜单
    */
    bool m_bEnableDefaultContextMenu;

    /** 是否禁止触发文本变化事件
    */
    bool m_bDisableTextChangeEvent;

    /** 设置允许的最大数字(仅当IsNumberOnly()为true的时候有效)
    */
    int32_t m_maxNumber;

    /** 设置允许的最小数字(仅当IsNumberOnly()为true的时候有效)
    */
    int32_t m_minNumber;

    /** 数字的格式
    */
    UiString m_numberFormat;

    /** 允许输入的字符列表
    */
    std::unique_ptr<DStringW::value_type[]> m_pLimitChars;

    /** Spin功能的容器
    */
    VBox* m_pSpinBox;

    /** 自动调整文本数字值的定时器生命周期管理
    */
    WeakCallbackFlag m_flagAdjustTextNumber;

    /** 获取焦点时，显示的图片
    */
    Image* m_pFocusedImage;

    /** 清除功能的按钮(仅当非只读模式有效)
    */
    Control* m_pClearButton;

    /** 显示/隐藏密码按钮(仅当密码模式有效)
    */
    Control* m_pShowPasswordButton;

private:
    /** 绘制所需的数据结构
    */
    struct TxDrawData
    {
    public:
        /** 绘制所需DC
        */
        HDC m_hDrawDC;

        /** DC资源的原位图数据
        */
        HGDIOBJ m_hOldBitmap;

        /** 位图资源
        */
        HBITMAP m_hBitmap;

        /** 位图的大小
        */
        UiSize m_szBitmap;

        /** 位图的数据指针
        */
        LPVOID m_pBitmapBits ;

    public:
        TxDrawData();
        ~TxDrawData();

        /** 清理资源
        */
        void Clear();

        /** 检查并重建位图
        */
        bool CheckCreateBitmap(HDC hWindowDC, int32_t nWidth, int32_t nHeight);
    };

    /** 绘制所需的数据
    */
    TxDrawData m_txDrawData;

private:
#ifndef DUILIB_UNICODE
    /** MBCS时，输入的字符
    */
    std::vector<BYTE> m_pendingChars;

    /** 上次输入的时间
    */
    DWORD m_dwLastCharTime = 0;
#endif

    /** 拖放功能的实现接口, 如果不为空表示功能已经开启
    */
    ControlDropTarget_Windows* m_pControlDropTarget;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_CONTROL_RICHEDIT_WINDOWS_H_
