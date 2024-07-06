#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

#include "resource.h"

// duilib
#include "duilib/duilib.h"

#include "RichEditFindReplace.h"

class FindForm;
class ReplaceForm;
class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
    virtual ~MainForm() override;

    /**
     * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
     * GetSkinFolder        接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile            接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

    /** 键盘按下(WM_KEYDOWN 或者 WM_SYSKEYDOWN)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下(WM_KEYUP 或者 WM_SYSKEYUP)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyUpMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    //查找/替换接口
    void FindRichText(const DString& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog);
    void ReplaceRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog);
    void ReplaceAllRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog);

    //获取RichEdit接口
    ui::RichEdit* GetRichEdit() const;

private:
    //加载默认的文本内容
    void LoadRichEditData();

    //打开文件、保存文件、另存为文件
    void OnOpenFile();
    void OnSaveFile();
    void OnSaveAsFile();

    bool LoadFile(const DString& filePath);
    bool SaveFile(const DString& filePath);

    //判断一个文件扩展名是否为RTF文件
    bool IsRtfFile(const DString& filePath) const;

    static DWORD CALLBACK StreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb);
    static DWORD CALLBACK StreamWriteCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb);

    //更新保存状态
    void UpdateSaveStatus();

private:
    //查找/替换
    void OnFindText();
    void OnFindNext();
    void OnReplaceText();

private://设置字体相关
    struct FontInfo
    {
        LOGFONTW lf;
        DWORD fontType;
    };

    //获取字体结构
    bool GetRichEditLogFont(LOGFONTW& lf) const;

    //初始化字体信息
    void InitCharFormat(const LOGFONTW& lf, CHARFORMAT2W& charFormat) const;

    //调用系统默认对话框，设置字体
    void OnSetFont();

    //获取系统字体列表
    void GetSystemFontList(std::vector<FontInfo>& fontList) const;

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

    //枚举字体回调函数
    static int CALLBACK EnumFontFamExProc(const LOGFONTW* lpelfe, const TEXTMETRICW* lpntme, DWORD fontType, LPARAM lParam);

    //将字体大小转换成Rich Edit控件的字体高度
    int32_t ConvertToFontHeight(int32_t fontSize) const;

    /** 获取RichEdit控件的字符格式
    */
    void GetCharFormat(CHARFORMAT2W& charFormat) const;

    /** 设置RichEdit控件的字符格式
    */
    void SetCharFormat(CHARFORMAT2W& charFormat);

private:
    //更新缩放比例
    void UpdateZoomValue();

    //初始化设置颜色的Combo按钮
    void InitColorCombo();

    //显示拾色器窗口
    void ShowColorPicker();

private:

    //RichEdit控件接口
    ui::RichEdit* m_pRichEdit;

    //当前打开的文件
    ui::FilePath m_filePath;

    //保存按钮的文字
    DString m_saveBtnText;

    //查找
    FindForm* m_pFindForm;

    //替换
    ReplaceForm* m_pReplaceForm;

    //查找替换实现
    RichEditFindReplace m_findReplace;

private:
    //字体名称列表
    std::vector<FontInfo> m_fontList;

    //字体大小列表
    std::vector<ui::FontSizeInfo> m_fontSizeList;
};
#endif //EXAMPLES_MAIN_FORM_H_
