#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

#include "RichEditFindReplace.h"

#include <functional>

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

    /** 窗口的DPI缩放比发生变化时，重建字体大小列表
    */
    virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) override;

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

    //获取上次查找/替换使用的选项
    const RichEditFindOptions& GetFindOptions() const;

    //设置上次查找/替换使用的选项
    void SetFindOptions(const RichEditFindOptions& options);

    /** 获取当前查找条件的匹配情况
    @param [out] nIndex 当前选区所在的匹配序号（从1开始；当前选区不是匹配项时为0）
    @param [out] nTotal 全文匹配总数
    */
    void GetFindMatchInfo(int32_t& nIndex, int32_t& nTotal) const;

    //在状态栏显示一条提示信息
    void ShowStatusMessage(const DString& message);

    //获取RichEdit接口
    ui::RichEdit* GetRichEdit() const;

private:
    //界面初始化
    void SetupTestControls();
    void SetupFileTools();
    void SetupEditTools();
    void SetupFindTools();
    void SetupFontTools();
    void SetupViewTools();
    void SetupTextEvents();
    void SetupDropFile();

    //绑定按钮的点击事件
    void BindClick(const DString& controlName, const std::function<void()>& onClick);

    //绑定复选框的状态切换事件（CheckBox不会触发kEventClick，需要监听select/unselect事件）
    void BindCheck(const DString& controlName, const std::function<void(bool)>& onChanged);

    //加载默认的示例内容
    void LoadRichEditData();

    //新建文档、打开文件、保存文件、另存为文件
    bool OnNewFile();
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

    //更新编辑按钮的可用状态
    void UpdateEditButtonStatus();

    //更新行距选项的可用状态
    void UpdateRowSpacingEnabled();

    //更新状态栏
    void UpdateStatusBar();

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

    //初始化字体大小的下拉列表（DPI变化时重建）
    void InitFontSizeCombo();

    //是否勾选了"应用到全文"
    bool IsApplyToAll();

    //设置字体信息（勾选"应用到全文"时作用于整个文档，否则作用于当前选区）
    bool SetRichEditFontInfo(const ui::UiFont& fontInfo);

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

private://缩放与视图
    //更新缩放比例
    void UpdateZoomValue();

    //初始化缩放比例下拉列表
    void InitZoomCombo();

    //同步缩放比例下拉列表与标签
    void SyncZoomCombo();

    //设置缩放百分比
    void SetZoomPercent(uint32_t nZoomPercent);

    //解析缩放比例下拉框中输入的文本并应用（支持"150"与"150%"两种写法）
    void ApplyZoomText();

    //初始化设置颜色的Combo按钮
    void InitColorCombo();

    //显示拾色器窗口
    void ShowColorPicker();

private://用户设置
    //获取设置文件的路径
    ui::FilePath GetSettingsFilePath() const;

    //加载用户设置
    void LoadSettings();

    //保存用户设置
    void SaveSettings() const;

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

    //是否正在响应缩放比例下拉框的选择（此时不再回写下拉框，避免打断下拉列表的选择流程）
    bool m_bZoomComboSelecting;

    //保存按钮的文字
    DString m_saveBtnText;

    //状态栏提示信息
    DString m_statusMessage;

    //当前文档编码说明
    DString m_encodingText;

    //最近一次打开/保存的目录
    ui::FilePath m_lastFileDir;

    //上次使用的字体名称
    DString m_lastFontName;

    //上次查找/替换使用的选项
    RichEditFindOptions m_findOptions;

    //仅富文本模式下可用的控件（行距选项、应用到全文）
    std::vector<ui::Control*> m_rowSpacingControls;

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
