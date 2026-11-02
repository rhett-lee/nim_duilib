//MainForm.h
#ifndef EXAMPLES_COLORPICKER_MAIN_FORM_H_
#define EXAMPLES_COLORPICKER_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

/** 调色器示例的主窗口：演示 ColorPicker 的两个回调
*   AttachSelectColor  选色过程中实时回调（可用于即时预览）
*   AttachWindowClose  窗口关闭时回调，只有点「确定」才采用最终颜色
*/
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MainForm();
    virtual ~MainForm() override;

    /** 窗口皮肤目录
    */
    virtual DString GetSkinFolder() override;

    /** 窗口皮肤 XML 描述文件
    */
    virtual DString GetSkinFile() override;

    /** 窗口创建完成后的初始化
    */
    virtual void OnInitWindow() override;

    /** 语言切换完成后的处理：日志行是代码拼接的文本，需要重新生成
    */
    virtual bool OnLanguageChanged() override;

private:
    /** 打开调色板窗口
    */
    void OpenColorPicker();

    /** 更新界面显示的颜色
    * @param [in] color 需要显示的颜色
    */
    void SetColor(const ui::UiColor& color);

    /** 输出一行日志
    */
    void PrintLog(const DString& text);

    /** 清空日志
    */
    void ClearLog();

    /** 获取窗口关闭原因的文本（确定/取消/关闭按钮）
    */
    DString GetCloseResultText(uint32_t wParam);

private:
    /** 颜色预览块
    */
    ui::Control* m_pColorBlock;

    /** 颜色值文本（#AARRGGBB）
    */
    ui::Label* m_pColorText;

    /** 颜色分量文本（R/G/B/A）
    */
    ui::Label* m_pColorRgba;

    /** 日志控件
    */
    ui::RichEdit* m_pLogView;

    /** 当前显示的颜色
    */
    ui::UiColor m_color;

    /** 打开调色板之前显示的颜色（用于「取消」时恢复）
    */
    ui::UiColor m_colorBeforePick;

    /** 已打开的调色板窗口（避免重复打开）
    */
    ui::ControlPtrT<ui::ColorPicker> m_pColorPicker;

    /** 日志序号
    */
    int32_t m_nLogCount;
};

#endif //EXAMPLES_COLORPICKER_MAIN_FORM_H_
