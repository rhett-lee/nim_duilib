#ifndef UI_CONTROL_TAGINPUT_H_
#define UI_CONTROL_TAGINPUT_H_

#include "duilib/Core/Box.h"

namespace ui
{
class RichEdit;
class TagInputLayout;

/** 标签输入框控件（TagInput）：输入文本后按回车生成标签，支持删除标签的组合控件
*
*   内部结构（水平流式布局，标签过多时自动换行）：
*   - 标签：HBox（class: tag_input_tag），内含文本 Label（tag_input_tag_text）
*           与右侧删除按钮 Button（tag_input_tag_close）
*   - 编辑框：RichEdit（class: tag_input_edit），始终位于最后一个标签之后，
*             宽度自动填充当前行剩余空间；空文本时按 Backspace 删除最后一个标签
*
*   交互：
*   - 输入文本后按回车键：文本生成一个标签，编辑框清空
*   - 配置 delimiter 属性后，输入指定分隔符（如逗号）也会生成标签
*   - 点击标签右侧 X 按钮：删除该标签
*   - 编辑框为空时按 Backspace：删除最后一个标签
*
*   XML 属性（除 Box 通用属性外）：
*     prompt_text     编辑框占位提示文字
*     prompt_text_id  编辑框占位提示文字的多语言 ID
*     prompt_color    编辑框占位提示文字颜色
*     allow_duplicate 是否允许重复标签（默认 false，不允许）
*     max_count       最多标签个数（默认 0，表示不限制）
*     delimiter       输入时直接生成标签的分隔符（默认空，表示仅回车键生成）
*     tags            初始标签文本，多个标签以分号";"分隔
*
*   事件：
*   - kEventTagAdded：新增标签后触发，WPARAM 为新标签序号（从 0 开始）
*   - kEventTagRemoved：标签被删除前触发，WPARAM 为被删除标签序号，
*                       回调中仍可通过 GetTagText(序号) 获取文本
*   - kEventTagClick：点击标签正文区域触发，WPARAM 为标签序号
*   - kEventTextChanged：内部编辑框文本变化时触发
*
*   皮肤类（global.xml 中定义）：tag_input / tag_input_tag /
*   tag_input_tag_text / tag_input_tag_close / tag_input_edit
*/
class DUILIB_API TagInput : public Box
{
    typedef Box BaseClass;
public:
    explicit TagInput(Window* pWindow);
    TagInput(const TagInput& r) = delete;
    TagInput& operator=(const TagInput& r) = delete;
    virtual ~TagInput() override = default;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 让控件获取焦点（实际焦点设置到内部编辑框）
    */
    virtual void SetFocus() override;

    /** 新增一个标签
    * @param [in] text 标签文本（首尾空白会被裁剪；空文本返回 false）
    * @return true 新增成功；false 表示文本为空、标签重复或超过最大个数
    */
    bool AddTag(const DString& text);

    /** 删除指定序号的标签
    * @param [in] index 标签序号（从 0 开始）
    * @return true 删除成功；false 表示序号无效
    */
    bool RemoveTagAt(size_t index);

    /** 删除全部标签（每个标签都会触发 kEventTagRemoved 事件）
    */
    void RemoveAllTags();

    /** 获取标签个数
    */
    size_t GetTagCount() const;

    /** 获取指定序号标签的文本
    */
    DString GetTagText(size_t index) const;

    /** 设置是否允许重复标签
    */
    void SetAllowDuplicate(bool bAllowDuplicate);

    /** 是否允许重复标签
    */
    bool IsAllowDuplicate() const;

    /** 设置最多标签个数（0 表示不限制）
    */
    void SetMaxCount(int32_t nMaxCount);

    /** 获取最多标签个数（0 表示不限制）
    */
    int32_t GetMaxCount() const;

    /** 获取内部编辑框控件
    */
    RichEdit* GetEditControl() const;

public:
    /** 监听标签新增事件（WPARAM 为新标签序号）
    */
    void AttachTagAdded(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventTagAdded, callback, callbackID);
    }

    /** 监听标签删除事件（WPARAM 为被删除标签序号，事件触发时标签尚未删除）
    */
    void AttachTagRemoved(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventTagRemoved, callback, callbackID);
    }

    /** 监听标签正文点击事件（WPARAM 为标签序号）
    */
    void AttachTagClick(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventTagClick, callback, callbackID);
    }

protected:
    /** 初始化接口：创建标签、编辑框等子控件
    */
    virtual void OnInit() override;

private:
    /** 创建一个标签控件
    * @param [in] text 标签文本
    */
    Control* CreateTagItem(const DString& text);

    /** 提交编辑框当前文本为标签
    */
    void CommitInput();

    /** 内部编辑框为空时按 Backspace 的处理：删除最后一个标签
    * @return true 表示已处理（不再向编辑框传递按键）
    */
    bool HandleEditBackspace();

    /** 编辑框文本变化处理：分隔符成标签 + 转发文本变化事件
    */
    void HandleEditTextChanged();

    /** 判断指定文本的标签是否已存在
    */
    bool IsTagExists(const DString& text) const;

    /** 解析 tags 属性（分号分隔的初始标签文本）
    */
    void ParseInitTags(const DString& tags);

private:
    /** 内部编辑框
    */
    RichEdit* m_pEdit;

    /** 标签文本列表（序号与标签子控件顺序一致）
    */
    std::vector<UiString> m_tags;

    /** 是否允许重复标签
    */
    bool m_bAllowDuplicate;

    /** 最多标签个数（0 表示不限制）
    */
    int32_t m_nMaxCount;

    /** 输入成标签的分隔符（空表示不启用）
    */
    UiString m_delimiter;

    /** 初始标签文本（OnInit 前缓存 XML 中的 tags 属性，分号分隔）
    */
    UiString m_initTags;

    /** 占位提示文字（OnInit 前缓存）
    */
    UiString m_promptText;

    /** 占位提示文字的多语言 ID（OnInit 前缓存）
    */
    UiString m_promptTextId;

    /** 占位提示文字颜色（OnInit 前缓存）
    */
    UiString m_promptColor;

    friend class TagInputEdit;
};

} // namespace ui

#endif // UI_CONTROL_TAGINPUT_H_
