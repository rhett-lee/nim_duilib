#ifndef UI_CONTROL_HOTKEY_H_
#define UI_CONTROL_HOTKEY_H_

#include "duilib/Box/HBox.h"

namespace ui
{
class HotKeyRichEdit;

/** 热键控件
*  内部使用一个编辑框捕获按键，支持组合键的录入：
*  (1) 按下修饰键（Ctrl/Shift/Alt）时仅预览显示，松开后回退，不产生无效热键；
*  (2) 按下其他键时提交热键，允许不带修饰键的单独按键；
*  (3) Delete/Backspace 清空热键，Esc 取消本次输入；
*  (4) 未设置热键时，default_text/default_text_id 以灰色占位提示显示。
*/
class DUILIB_API HotKey: public HBox
{
    typedef HBox BaseClass;
public:
    explicit HotKey(Window* pWindow);
    virtual ~HotKey() override;

    /** 获取控件类型
    */
    virtual DString GetType() const override;

    /** 设置属性
    */
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 让控件获取焦点
     */
    virtual void SetFocus() override;

    /** 当前控件是否拥有焦点（焦点位于内部的编辑框时，也视为拥有焦点）
     */
    virtual bool IsFocused() const override;

public:
    /** 设置热键
    * @param [in] wVirtualKeyCode 虚拟键盘码，比如：VK_DOWN等，可参考：https://learn.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes
    *             该值为 0 时表示清空热键（wModifiers 同时被清空）
    * @param [in] wModifiers 热键组合键标志位，参见HotKeyModifiers枚举类型的值
    */
    void SetHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers);

    /** 获取热键
    * @param [out] wVirtualKeyCode 虚拟键盘码，比如：VK_DOWN等
    * @param [out] wModifiers 热键组合键标志位，参见HotKeyModifiers枚举类型的值
    */
    void GetHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const;

    /** 设置热键的值[由MAKEWORD(wVirtualKeyCode, wModifiers)生成]
    * @param [in] dwHotKey 高8位为wModifiers，低8位为wVirtualKeyCode
    */
    void SetHotKey(uint16_t dwHotKey);

    /** 获取热键的值[由MAKEWORD(wVirtualKeyCode, wModifiers)生成]
    * @return 热键的值，高8位为wModifiers，低8位为wVirtualKeyCode
    */
    uint16_t GetHotKey() const;

    /** 清空热键（清空后显示占位提示文字）
    */
    void ClearHotKey();

    /** 获取热键显示名称，比如"Ctrl + C"等
    */
    DString GetHotKeyName() const;

    /** 根据显示名称设置热键
    * @param [in] hotKeyName 热键名称，比如"Ctrl + C"等；传入空字符串时表示清空热键
    * @return 返回true表示设置成功，返回false表示解析失败（此时不修改当前热键）
    */
    bool SetHotKeyName(const DString& hotKeyName);

public:
    /** 获取键的显示名称
    * @param [in] wVirtualKeyCode 虚拟键盘码，比如：VK_DOWN等
    * @param [in] fExtended 如果虚拟键代码是扩展密钥，则为 true；否则为 false。
    */
    static DString GetKeyName(uint8_t wVirtualKeyCode, bool fExtended);

protected:
    //初始化
    virtual void OnInit() override;

private:
    /** 应用热键值（内部修饰键标志为HOTKEYF_*格式）
    * @param [in] wVirtualKeyCode 虚拟键盘码，为 0 时表示清空热键
    * @param [in] wModifiers 内部修饰键标志（HOTKEYF_*）
    */
    void ApplyHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers);

    /** 刷新编辑框显示的文本（已提交的热键，或占位提示文字）
    */
    void UpdateDisplayText();

    /** 预览修饰键（仅在按下修饰键时刷新显示，不修改热键值）
    */
    void PreviewHotKey(uint8_t wModifiers);

    /** 结束修饰键的预览，恢复为已提交热键的显示
    */
    void RestoreDisplayText();

    /** 热键值变化时发送kEventTextChanged事件（相同值不重复发送）
    */
    void NotifyHotKeyChanged();

    /** 根据虚拟键码与修饰键生成显示名称（修饰键为HOTKEYF_*标志）
    */
    static DString FormatHotKeyName(uint8_t wVirtualKeyCode, uint8_t wModifiers);

private:
    friend class HotKeyRichEdit;

    /** 编辑框控件的接口
    */
    HotKeyRichEdit* m_pRichEdit;

    /** 虚拟键盘码，比如：VK_DOWN等（0表示未设置热键）
    */
    uint8_t m_wVirtualKeyCode;

    /** 热键组合键标志（HOTKEYF_* 标志位）
    */
    uint8_t m_wModifiers;

    /** 最近一次发送变化事件的热键值，用于避免重复发送事件
    */
    uint16_t m_nLastHotKeyValue;

    /** 当前显示的内容是否为修饰键的预览（预览时热键值尚未提交）
    */
    bool m_bPreviewing;
};

}//namespace ui

#endif //UI_CONTROL_HOTKEY_H_
