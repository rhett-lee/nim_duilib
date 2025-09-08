#ifndef UI_CONTROL_RICHEDIT_SDL_H_
#define UI_CONTROL_RICHEDIT_SDL_H_

#include "duilib/Box/ScrollBox.h"
#include "duilib/Image/Image.h"
#include "duilib/Control/RichEditData.h"

#ifdef DUILIB_BUILD_FOR_SDL

namespace ui 
{

class VBox;
class DrawRichTextCache;

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

class UILIB_API RichEdit : public ScrollBox, protected IRichTextData
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
    virtual void HandleEvent(const EventArgs& msg) override; 
    virtual void SetWindow(Window* pWindow) override;
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void ClearImageCache() override;
    virtual UiSize EstimateText(UiSize szAvailable) override;
    virtual UiSize64 CalcRequiredSize(const UiRect& rc) override;
    virtual void OnScrollOffsetChanged(const UiSize& oldScrollOffset, const UiSize& newScrollOffset) override;

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
    DStringA GetTextA() const;
    DStringW GetTextW() const;

    /** 获取内容的长度(按UTF16编码的字符个数，TODO：有歧义)
     * @return 返回内容长度
     */
    int32_t GetTextLength() const;

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

    /** 是否为空
    */
    bool IsEmpty() const;

    /** 设置字体Id
     * @param[in] index 要设置的字体Id（对应 global.xml 中字体的ID）
     */
    void SetFontId(const DString& strFontId);

    /** 获取当前设置的字体Id（通过SetFontId设置的字体ID）
     * @return 返回字体Id（对应 global.xml 中字体的ID）
     */
    DString GetFontId() const;

    /** 获取字体信息(字体大小是进行过DPI缩放处理的)
    */
    UiFont GetFontInfo() const;

    /** 设置字体信息（优先级高，会覆盖通过SetFontId设置的字体）
    * @param [in] fontInfo 字体信息，字体大小是进行过DPI缩放处理的
    */
    bool SetFontInfo(const UiFont& fontInfo);

    /** 获取当前使用的字体ID
    * @return 如果调用SetFontInfo函数设置过字体，返回内部使用的字体ID；如果未调用过SetFontInfo函数设置字体，则返回通过SetFontId设置的字体ID
    */
    DString GetCurrentFontId() const;

    /** 设置正常文本颜色
     * @param[in] dwTextColor 要设置的文本颜色
     */
    void SetTextColor(const DString& dwTextColor);

    /** 获取正常文本颜色
     */
    DString GetTextColor() const;

    /** 获取所选文本颜色(不支持)
    */
    DString GetSelectionTextColor() const;

    /** 设置所选文本的颜色(不支持)
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

    /** 设置选择文本的背景色（焦点状态）
    */
    void SetSelectionBkColor(const DString& selectionBkColor);

    /** 获取选择文本的背景颜色（焦点状态）
    */
    DString GetSelectionBkColor() const;

    /** 设置选择文本的背景色（非焦点状态）
    */
    void SetInactiveSelectionBkColor(const DString& selectionBkColor);

    /** 获取选择文本的背景颜色（非焦点状态）
    */
    DString GetInactiveSelectionBkColor() const;

    /** 设置当前行的背景颜色（光标所在行，焦点状态）
    */
    void SetCurrentRowBkColor(const DString& currentRowBkColor);

    /** 获取当前行的背景颜色（焦点状态）
    */
    DString GetCurrentRowBkColor() const;

    /** 设置当前行的背景颜色（光标所在行，非焦点状态）
    */
    void SetInactiveCurrentRowBkColor(const DString& currentRowBkColor);

    /** 获取当前行的背景颜色（非焦点状态）
    */
    DString GetInactiveCurrentRowBkColor() const;

public:
    /** 设置是否允许显示提示文字
     * @param[in] bPrompt 设置为 true 为显示，false 为不显示
     */
    void SetPromptMode(bool bPrompt);

    /** 获取是否允许显示提示文字
    */
    bool AllowPromptMode() const;

    /** 获取提示文字
     */
    DString GetPromptText() const;

    /** 设置提示文字
     * @param[in] strText 要设置的提示文字
     */
    void SetPromptText(const DString& strText);

    /** 设置提示文字 ID
     * @param[in] strText 要设置的提示文字 ID，该 ID 必须在加载的语言文件中存在
     */
    void SetPromptTextId(const DString& strTextId);

    /** 设置提示文字的颜色
    */
    void SetPromptTextColor(const DString& promptColor);

    /** 获取提示文字的颜色
    */
    DString GetPromptTextColor() const;

public:
    /** 设置文字内边距信息
     * @param[in] padding 内边距信息
     * @param[in] bNeedDpiScale 是否支持DPI缩放
     */
    void SetTextPadding(UiPadding padding, bool bNeedDpiScale);

    /** 获取文字内边距
     */
    UiPadding GetTextPadding() const;

    /** 是否为多行文本（默认为支持多行，非单行文本模式）
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

    /** 获取密码字符
    */
    DStringW::value_type GetPasswordChar() const;

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

    /** 获取数字的格式（64位有符号整型的格式, 比如"%I64d"等）
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

    /** 设置缩放百分比
    * @param [in] fZoomRatio 缩放比例，比如"100"表示100%, 无缩放; "200"表示缩放比例为200%
    */
    void SetZoomPercent(uint32_t nZoomPercent);

    /** 获取缩放百分比
    */
    uint32_t GetZoomPercent() const;

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

    /** 设置文本水平对齐方式
    */
    void SetHAlignType(HorAlignType alignType);

    /** 获取文本水平对齐方式
    */
    HorAlignType GetHAlignType() const;

    /** 设置文本垂直对齐方式
    */
    void SetVAlignType(VerAlignType alignType);

    /** 获取文本垂直对齐方式
    */
    VerAlignType GetVAlignType() const;

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
     * @param[in] nStartChar 返回选择文本的起始字符位置，字符串中从0开始的下标值，如果没有选择文本，返回-1
     * @param[in] nEndChar 返回选择文本的最后一个字符的下一个字符下标值，如果没有选择文本，返回0
     */
    void GetSel(int32_t& nStartChar, int32_t& nEndChar) const;

    /** 选择一部分内容
     * @param[in] nStartChar 要选择的起始位置
     * @param[in] nEndChar 要选择的结束位置
     * @return 返回选择的文字数量
     */
    int32_t SetSel(int32_t nStartChar, int32_t nEndChar);

    /** 替换所选内容
     * @param [in] newText 要替换的目标文字
     * @param [in] bCanUndo 是否可以撤销，true 为可以，否则为 false
     */
    bool ReplaceSel(const DString& newText, bool bCanUndo);

    /** 获取所选文字内容
     * @return 返回所选文字内容
     */
    DString GetSelText() const;

    /** 当前是否有选择的文本
    */
    bool HasSelText() const;

    /** 获取指定范围的内容
     * @param[in] nStartChar 起始位置
     * @param[in] nEndChar 结束为止
     * @return 返回设置的指定位置的内容
     */
    DString GetTextRange(int32_t nStartChar, int32_t nEndChar) const;

    /** 设置隐藏或显示选择的文本（当控件处于非激活状态时，是否隐藏选择内容）
     * @param [in] bHideSelection 是否显示，true 为隐藏，false 为显示
     */
    void SetHideSelection(bool bHideSelection);
    void HideSelection(bool bHideSelection);

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
    int32_t CharFromPos(UiPoint pt);

    /** 清空撤销列表
     */
    void EmptyUndoBuffer();

    /** 设置撤销列表容纳的内容数量
     * @param [in] nLimit
     */
    void SetUndoLimit(uint32_t nLimit);

    /** 确保字符在可见范围内
    * @param [in] nCharIndex 字符的位置
    */
    void EnsureCharVisible(int32_t nCharIndex);

    /** 查找文本
    * @param [in] findParam 查找参数
    * @param [out] chrgText 匹配的文本，字符的索引号范围
    */
    bool FindRichText(const FindTextParam& findParam, TextCharRange& chrgText) const;

    /** 是否是富文本模式
     * @return 始终返回 false，为纯文本模式，不支持富文本模式
     */
    bool IsRichText() const;

    /** 获取文本内容，并转换为数字
    */
    int64_t GetTextNumber() const;

    /** 将数字转换为文本，设置文本内容
    */
    void SetTextNumber(int64_t nValue);

    /** 调整文本数字值
    */
    void AdjustTextNumber(int32_t nDelta);

public:
    /** 向上一行
     * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
     * @param[in] withAnimation 是否附带动画效果，默认为 true
     */
    virtual void LineUp(int32_t deltaValue = DUI_NOSET_VALUE, bool withAnimation = true) override;

    /** 向下一行
     * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
     * @param[in] withAnimation 是否附带动画效果，默认为 true
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

    /** 滚动到最底部
     * @param[in] arrange 是否重置滚动条位置，默认为 true
     * @param[in] withAnimation 是否包含动画特效，默认为 true
     */
    virtual void EndDown(bool arrange = true, bool withAnimation = true) override;

    /** 水平向左滚动
     * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
     */
    virtual void LineLeft(int32_t deltaValue = DUI_NOSET_VALUE) override;

    /** 水平向右滚动
     * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
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

protected:

    //一些基类的虚函数
    virtual bool CanPlaceCaptionBar() const override;
    virtual void OnInit() override;
    virtual uint32_t GetControlFlags() const override;

    //消息处理函数
    virtual bool OnSetCursor(const EventArgs& msg) override;
    virtual bool OnSetFocus(const EventArgs& msg) override;
    virtual bool OnKillFocus(const EventArgs& msg) override;
    virtual bool OnImeStartComposition(const EventArgs& msg) override;
    virtual bool OnImeEndComposition(const EventArgs& msg) override;
    
    virtual bool OnKeyDown(const EventArgs& msg) override;
    virtual bool OnKeyUp(const EventArgs& msg) override;
    virtual bool OnChar(const EventArgs& msg) override;

    //鼠标消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool ButtonDoubleClick(const EventArgs& msg) override;
    virtual bool RButtonDown(const EventArgs& msg) override;
    virtual bool RButtonUp(const EventArgs& msg) override;
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool MouseWheel(const EventArgs& msg) override;
    virtual bool OnWindowKillFocus(const EventArgs& msg) override;//控件所属的窗口失去焦点

    //绘制相关函数
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;
    virtual void PaintBorder(IRender* pRender) override;

    /** 将文本生成可绘制的格式
    * @param [in] textView 按行组织切分后的文本视图，每行一条数据（以'\n'切分的行）
    * @param [out] richTextDataList 返回格式化的文本，返回的容器个数应与传入的textView相同，如果此行不更新，可用填充空数据: RichTextData()
    * @param [in] nStartLine 重新计算的起始行号（增量计算时使用）
    * @param [in] modifiedLines 有修改的行号（增量计算时使用）
    */
    virtual bool GetRichTextForDraw(const std::vector<std::wstring_view>& textView,
                                    std::vector<RichTextData>& richTextDataList,
                                    size_t nStartLine = (size_t)-1,
                                    const std::vector<size_t>& modifiedLines = std::vector<size_t>()) const override;

    /** 获取文本绘制矩形范围（需要时，随时调用该接口获取绘制文本的矩形范围）
    * @return 返回当前文本绘制的矩形范围，该范围需要去除内边距，滚动条所占空间
    */
    virtual UiRect GetRichTextDrawRect() const override;

    /** 获取绘制的透明度
    */
    virtual uint8_t GetDrawAlpha() const override;

    /** 文字区域已经重新计算过的事件
    */
    virtual void OnTextRectsChanged() override;

    /** 获取行高值
    */
    virtual int32_t GetTextRowHeight() const override;

    /** 获取光标的宽度
    */
    virtual int32_t GetTextCaretWidth() const override;

    /** 当前是否为密码模式
    */
    virtual bool IsTextPasswordMode() const override;

    /** 处理密码模式下的显示字符
    */
    virtual void ReplacePasswordChar(DStringW& text) const override;

    /** 获取文本限制长度
    */
    virtual int32_t GetTextLimitLength() const override;

    /** 设置可用状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetEnabled(bool bChanged) override;

private:
    void OnLButtonDown(const UiPoint& ptMouse, Control* pSender, bool bShiftDown);
    void OnLButtonUp(const UiPoint& ptMouse, Control* pSender);
    void OnLButtonDoubleClick(const UiPoint& ptMouse, Control* pSender);
    void OnRButtonDown(const UiPoint& ptMouse, Control* pSender);
    void OnRButtonUp(const UiPoint& ptMouse, Control* pSender);
    void OnMouseMove(const UiPoint& ptMouse, Control* pSender);
    void OnMouseWheel(int32_t wheelDelta, bool bCtrlDown);
    void OnWindowKillFocus();

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

    /** 获取字体接口
    */
    IFont* GetIFontInternal(const DString& fontId) const;

    /** 获取本控件内部的字体ID
    */
    DString GetInternalFontId() const;

    /** 绘制光标
     * @param[in] pRender 绘制引擎
     * @param[in] rcPaint 绘制位置
     */
    void PaintCaret(IRender* pRender, const UiRect& rcPaint);

    /** 绘制当前编辑行的背景色
    */
    void PaintCurrentRowBkColor(IRender* pRender, const UiRect& rcPaint);

    /** 绘制选择背景色
    */
    void PaintSelectionColor(IRender* pRender, const UiRect& rcPaint);

    /** 切换光标是否显示
    */
    void ChangeCaretVisiable();

    /** 绘制提示文字
     * @param[in] pRender 绘制引擎
     */
    void PaintPromptText(IRender* pRender);

    /** 停止密码字符闪现
    */
    void StopFlashPasswordChar();

    /** 获取下一个缩放百分比值
    * @param [in] nOldZoomPercent 当前的缩放百分比
    * @param [in] bZoomIn true表示放大，false表示缩小
    */
    uint32_t GetNextZoomPercent(uint32_t nOldZoomPercent, bool bZoomIn) const;

    /** 缩放百分比变化
    */
    void OnZoomPercentChanged(uint32_t nOldZoomPercent, uint32_t nNewZoomPercent);

    /** 字体发生变化
    */
    void OnFontChanged(const DString& fontId);

private:

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    /** 获取关联的窗口局部
    */
    HWND GetWindowHWND() const;
#endif

private:
    /** 调整光标的位置（按点的坐标）
    * @param [in] pt 需要设置调整的位置（客户区坐标）
    */
    void SetCaretPos(const UiPoint& pt);

    /** 调整光标的位置（按字符位置）
    */
    void SetCaretPos(int32_t nCharPosIndex);

    /** 设置光标位置
     * @param [in] xPos X 轴坐标
     * @param [in] yPos Y 轴坐标
     */
    void SetCaretPosInternal(int32_t xPos, int32_t yPos);

    /** 获取当前绘制文字的属性
    */
    uint16_t GetTextStyle() const;

    /** 将文本生成可绘制的格式
    */
    bool GetRichTextForDraw(std::vector<RichTextData>& richTextDataList) const;

    /** 获取文本绘制区域
    * @param [in] rc 当前控件的矩形区域
    * @return 返回文本绘制区域（减去内边距，减去滚动条所占的宽度和高度）
    */
    UiRect GetTextDrawRect(const UiRect& rc) const;

    /** 重绘
    */
    void Redraw();

    /** 检查是否需要滚动视图
    */
    void OnCheckScrollView();

    /** 执行了鼠标框选操作(坐标包含了scrollPos值)
    * @param [in] ptMouseDown64 鼠标按下的起点
    * @param [in] ptMouseMove64 鼠标移动的终点
    */
    void OnFrameSelection(UiSize64 ptMouseDown64, UiSize64 ptMouseMove64);

    /** 在当前光标位置，插入一个字符（文本输入模式）
    */
    void OnInputChar(const EventArgs& msg);

    /** 检查Shift和Ctrl按键的标志
    */
    void CheckKeyDownStartIndex(const EventArgs& msg);

    /** 处理视图滚动的方向键的快捷键（Ctrl + 方向键）
    */
    bool OnCtrlArrowKeyDownScrollView(const EventArgs& msg);

    /** 获取纵向滚动一行的距离，按行对齐
    */
    int32_t GetLineScrollDeltaValue(bool bLineDown) const;

    /** 获取纵向滚动一页的距离，按页对齐
    */
    int32_t GetPageScrollDeltaValue(bool bPageDown) const;

    /** 方向键的快捷键处理
    */
    bool OnArrowKeyDown(const EventArgs& msg);

    /** 选择一部分内容(内部函数)
     * @param[in] nStartChar 要选择的起始位置
     * @param[in] nEndChar 要选择的结束位置
     * @return 返回选择的文字起始下标值
     */
    int32_t InternalSetSel(int32_t nStartChar, int32_t nEndChar);

    /** 移除不支持的密码字符
    */
    bool RemoveInvalidPasswordChar(DStringA& text);
    bool RemoveInvalidPasswordChar(DStringW& text);

    /** 更新滚动条的范围
    */
    void UpdateScrollRange();

    /** 获得焦点时，全选
    */
    void CheckSelAllOnFocus();

private:
    bool m_bWantTab;            //是否接收TAB键，如果为true的时候，TAB键会当作文本输入，否则过滤掉TAB键
    bool m_bWantReturn;         //是否接收回车键，如果为true的时候，回车键会当作文本输入，否则过滤掉回车键
    bool m_bWantCtrlReturn;     //是否接收Ctrl + 回车键，如果为true的时候，回车键会当作文本输入，否则过滤掉回车键
     
    bool m_bSelAllEver;         //只在获取焦点后的第一次鼠标弹起全选

    bool m_bNoSelOnKillFocus;   //失去焦点的时候，取消文本选择（针对 m_bEnabled && IsReadOnly()）
    bool m_bSelAllOnFocus;      //获取焦点的时候，全选文本（针对 m_bEnabled && !IsReadOnly()）

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    bool m_bIsComposition;      //输入法合成窗口是否可见
#endif

    bool m_bReadOnly;           //是否为只读模式
    bool m_bPasswordMode;       //是否为密码模式
    bool m_bShowPassword;       //是否显示密码
    DStringW::value_type m_chPasswordChar;   //密码字符
    bool m_bFlashPasswordChar;  //是否短暂的显示密码字符，然后再隐藏
    bool m_bInputPasswordChar;  //当前是否存在输入密码

    bool m_bNumberOnly;         //是否只允许输入数字
    bool m_bWordWrap;           //当显示超出边界时，是否自动换行

    int32_t m_nLimitText;       //最大文本字符数（仅当为正数的时候代表有限制）
    bool m_bModified;           //文本内容是否有修改

private:
    bool m_bNoCaretReadonly;    //只读模式下，不显示光标
    bool m_bIsCaretVisiable;    //光标是否可见
    int32_t m_iCaretPosX;       //光标X坐标
    int32_t m_iCaretPosY;       //光标Y坐标
    int32_t m_iCaretWidth;      //光标宽度
    int32_t m_iCaretHeight;     //光标高度
    UiString m_sCaretColor;     //光标颜色

    int32_t m_nRowHeight;       //行高(逻辑行)，与字体有关

    WeakCallbackFlag m_drawCaretFlag;   //绘制光标的定时器生命周期

private:
    UiString m_sFontId;                 //字体ID
    UiString m_sTextColor;              //正常文本颜色
    UiString m_sDisabledTextColor;      //Disabled状态的文本颜色
    UiPadding16 m_rcTextPadding;        //文本内边距

    UiString m_sFocusBottomBorderColor; //焦点状态时，底部边框的颜色

    UiString m_sCurrentRowBkColor;         //当前行的背景颜色（光标所在行，焦点状态）
    UiString m_sInactiveCurrentRowBkColor; //当前行的背景颜色（光标所在行, 非焦点状态）
    UiString m_sSelectionBkColor;          //选择文本的背景颜色（焦点状态）
    UiString m_sInactiveSelectionBkColor;  //选择文本的背景颜色（非焦点状态）

    UiString m_sPromptColor;            //提示文字颜色
    UiString m_sPromptText;             //提示文本内容（只有编辑框为空的时候显示）
    UiString m_sPromptTextId;           //提示文字ID
    bool m_bAllowPrompt;                //是否支持提示文字

    uint8_t m_nFocusBottomBorderSize;    //焦点状态时，底部边框的大小

private:
    /** 是否使用Control设置的光标
    */
    bool m_bUseControlCursor;

    /** 缩放百分比，100表示100%
    */
    uint16_t m_nZoomPercent;

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

    /** 允许输入的字符列表
    */
    std::unique_ptr<DStringW::value_type[]> m_pLimitChars;

    /** 数字的格式
    */
    UiString m_numberFormat;

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

    /** 当控件处于非激活状态时，是否隐藏选择内容(显示时：选择的文本背景色与正常文本不同)
    */
    bool m_bHideSelection;

    /** 当前控件是否处于激活状态
    */
    bool m_bActive;

    /** 是否正在处于文本输入状态
    */
    bool m_bTextInputMode;

private:
    /** 文本内容管理接口
    */
    RichEditData* m_pTextData;

private:
    /** 选择的起始字符
    */
    int32_t m_nSelStartIndex;

    /** 选择的结束字符
    */
    int32_t m_nSelEndCharIndex;

    /** 按住Shift键时的选择起始字符
    */
    int32_t m_nShiftStartIndex;

    /** 按住Ctrl键时的选择起始字符
    */
    int32_t m_nCtrlStartIndex;

    /** 当前选择的操作方向（向前、向后）
    */
    bool m_bSelForward;

    /** 方向键切换位置时的X坐标值
    */
    int32_t m_nSelXPos;

private:
    /** 是否鼠标在视图中按下左键或者右键
    */
    bool m_bMouseDownInView;

    /** 是否鼠标左键按下
    */
    bool m_bMouseDown;

    /** 是否鼠标右键按下
    */
    bool m_bRMouseDown;

    /** 是否处于鼠标滑动操作中
    */
    bool m_bInMouseMove;

    /** 鼠标按下时的鼠标位置
    */
    UiSize64 m_ptMouseDown;

    /** 鼠标滑动时的鼠标位置
    */
    UiSize64 m_ptMouseMove;

    /** 鼠标按下时的控件接口
    */
    Control* m_pMouseSender;

    /** 定时器滚动视图时的取消机制
    */
    WeakCallbackFlag m_scrollViewFlag;

    /** 密码字符闪现功能的定时器取消机制
    */
    WeakCallbackFlag m_falshPasswordFlag;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL

#endif // UI_CONTROL_RICHEDIT_SDL_H_
