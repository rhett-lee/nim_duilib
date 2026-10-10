#ifndef UI_CONTROL_SEARCHBOX_H_
#define UI_CONTROL_SEARCHBOX_H_

#include "duilib/Box/HBox.h"

namespace ui
{
class RichEdit;

/** 搜索框控件（SearchBox）：左侧搜索图标 + 中间编辑框 + 右侧清除按钮的组合控件
*
*   内部结构：
*   - 搜索图标：普通 Control，不响应鼠标、不获取焦点
*   - 编辑框：RichEdit，默认开启占位提示（prompt），按回车触发 kEventReturn 事件
*   - 清除按钮：复用 RichEdit 内置的 clear_btn_class 机制，文本非空时自动显示
*
*   XML 属性（除 HBox 通用属性外）：
*     prompt_text     占位提示文字
*     prompt_text_id  占位提示文字的多语言 ID
*     prompt_color    占位提示文字颜色
*     text            初始搜索文本
*     text_id         初始搜索文本的多语言 ID（与 text 同时设置时，后设置的属性生效）
*
*   事件：
*   - kEventTextChanged：搜索文本变化时触发（内部编辑框事件转发）
*   - kEventReturn：按下回车键时触发，用于执行搜索
*
*   皮肤类（global.xml 中定义）：search_box / search_box_icon /
*   search_box_edit / search_box_clear_btn
*/
class DUILIB_API SearchBox : public HBox
{
    typedef HBox BaseClass;
public:
    explicit SearchBox(Window* pWindow);
    SearchBox(const SearchBox& r) = delete;
    SearchBox& operator=(const SearchBox& r) = delete;
    virtual ~SearchBox() override = default;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 让控件获取焦点（实际焦点设置到内部编辑框）
    */
    virtual void SetFocus() override;

    /** 获取内部编辑框控件
    */
    RichEdit* GetEditControl() const;

    /** 设置搜索文本
    */
    void SetSearchText(const DString& text);

    /** 获取搜索文本
    */
    DString GetSearchText() const;

public:
    /** 监听回车按键按下事件
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * @param [in] callback 回车被按下的自定义回调函数
     */
    void AttachReturn(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventReturn, callback, callbackID); }

protected:
    /** 初始化接口：创建搜索图标、编辑框等子控件
    */
    virtual void OnInit() override;

private:
    /** 内部编辑框
    */
    RichEdit* m_pEdit;

    /** 初始搜索文本（OnInit 前缓存 XML 中的 text 属性）
    */
    UiString m_initText;

    /** 初始搜索文本的多语言 ID（OnInit 前缓存 XML 中的 text_id 属性）
    */
    UiString m_initTextId;

    /** 占位提示文字（OnInit 前缓存）
    */
    UiString m_promptText;

    /** 占位提示文字的多语言 ID（OnInit 前缓存）
    */
    UiString m_promptTextId;

    /** 占位提示文字颜色（OnInit 前缓存）
    */
    UiString m_promptColor;
};

} // namespace ui

#endif // UI_CONTROL_SEARCHBOX_H_
