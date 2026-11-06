#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

#include "RichEditFindReplace.h"

class FindForm;
class ReplaceForm;
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MainForm();
    virtual ~MainForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 窗口关闭消息：有未保存的修改时，提示用户保存
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 当窗口已经被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

    /** 窗口关闭前：保存相关状态
    */
    virtual void OnPreCloseWindow() override;

    /** 界面语言切换后：刷新动态文本
    */
    virtual bool OnLanguageChanged() override;

    /** 键盘按下(WM_KEYDOWN 或者 WM_SYSKEYDOWN)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下(WM_KEYUP 或者 WM_SYSKEYUP)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyUpMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    //查找/替换接口
    bool FindRichText(const DString& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog);
    void ReplaceRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog);
    void ReplaceAllRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog);

    //获取RichEdit接口
    ui::RichEdit* GetRichEdit() const;

private:
    //加载默认的文本内容
    void LoadRichEditData();

    //打开文件、保存文件、另存为文件
    bool OnOpenFile();
    bool OnSaveFile();
    bool OnSaveAsFile();

    bool LoadFile(const ui::FilePath& filePath);
    bool SaveFile(const ui::FilePath& filePath);

    //保存当前文档（文件路径为空时自动转为另存为）
    bool SaveDocument();

    //检查未保存的修改，返回 true 表示可以继续操作
    bool ConfirmDiscardChanges();

    //显示错误提示框
    void ShowErrorMessage(const DString& textId, const DString& titleId);

    //更新保存状态
    void UpdateSaveStatus();

    //更新状态栏
    void UpdateStatusBar();

    //在状态栏显示一条提示信息
    void ShowStatusMessage(const DString& message);

private:
    //查找/替换
    void OnFindText();
    void OnFindNext();
    void OnReplaceText();

private://设置字体相关
    //更新字体按钮的状态
    void UpdateFontStatus();

    //更新字体大小的状态
    void UpdateFontSizeStatus();

    //设置字体名称
    void SetFontName(const DString& fontName);

    //设置字体大小
    void SetFontSize(const DString& fontSize);

    //调整字体大小: bIncreaseFontSize 为true表示增加字体大小，为false表示减小字体大小
    void AdjustFontSize(bool bIncreaseFontSize);

    //设置字体样式（粗体）
    void SetFontBold(bool bBold);

    //设置字体样式（斜体）
    void SetFontItalic(bool bItalic);

    //设置字体样式（下划线）
    void SetFontUnderline(bool bUnderline);

    //设置字体样式（删除线）
    void SetFontStrikeOut(bool bStrikeOut);

    /** 设置文本颜色
    */
    void SetTextColor(const DString& newColor);

private:
    //更新缩放比例
    void UpdateZoomValue();

    //初始化设置颜色的Combo按钮
    void InitColorCombo();

    //显示拾色器窗口
    void ShowColorPicker();

    /** 获取下一个缩放百分比值
    * @param [in] nOldZoomPercent 当前的缩放百分比
    * @param [in] bZoomIn true表示放大，false表示缩小
    */
    uint32_t GetNextZoomPercent(uint32_t nOldZoomPercent, bool bZoomIn) const;

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

    //判断一个文件扩展名是否为RTF文件
    bool IsRtfFile(const DString& filePath) const;

    static DWORD CALLBACK StreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb);
    static DWORD CALLBACK StreamWriteCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb);

    //调用系统默认对话框，设置字体
    void OnSetFont();

    //获取字体结构
    bool GetRichEditLogFont(LOGFONTW& lf) const;

    //初始化字体信息
    void InitCharFormat(const LOGFONTW& lf, CHARFORMAT2W& charFormat) const;

    /** 获取RichEdit控件的字符格式
*/
    void GetCharFormat(CHARFORMAT2W& charFormat) const;

    /** 设置RichEdit控件的字符格式
    */
    void SetCharFormat(CHARFORMAT2W& charFormat);

    //将字体大小转换成Rich Edit控件的字体高度
    int32_t ConvertToFontHeight(int32_t fontSize) const;

#endif

private:

    //RichEdit控件接口
    ui::RichEdit* m_pRichEdit;

    //当前打开的文件
    ui::FilePath m_filePath;

    //是否是示例文档（示例内容来自界面资源，不能直接保存到该文件）
    bool m_bSampleDocument;

    //是否正在加载文档（加载期间忽略文本变化事件）
    bool m_bLoadingDocument;

    //是否正在同步富文本模式复选框的状态
    bool m_bUpdatingRichTextCheck;

    //保存按钮的文字
    DString m_saveBtnText;

    //状态栏提示信息
    DString m_statusMessage;

    //当前文档编码说明
    DString m_encodingText;

    //最近一次打开/保存的目录
    ui::FilePath m_lastFileDir;

    //查找
    FindForm* m_pFindForm;

    //替换
    ReplaceForm* m_pReplaceForm;

    //查找替换实现
    RichEditFindReplace m_findReplace;

private:
    //字体名称列表
    std::vector<DString> m_fontList;

    //字体大小列表
    std::vector<ui::FontSizeInfo> m_fontSizeList;
};
#endif //EXAMPLES_MAIN_FORM_H_
