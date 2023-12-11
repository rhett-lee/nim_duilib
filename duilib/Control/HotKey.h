#ifndef UI_CONTROL_HOTKEY_H_
#define UI_CONTROL_HOTKEY_H_

#pragma once

#include "duilib/Box/HBox.h"

namespace ui
{
class HotKeyRichEdit;

/** 热键控件
*/
class HotKey: public HBox
{
public:
	HotKey();
    virtual ~HotKey();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;

    /** 设置属性
    */
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

    /** 让控件获取焦点
     */
    virtual void SetFocus() override;

public:
    /** 设置热键
    * @param [in] wVirtualKeyCode 虚拟键盘码，比如：VK_DOWN等，可参考：https://learn.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes
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

    /** 获取热键显示名称，比如"Ctrl + C"等
    */
    std::wstring GetHotKeyName() const;

    /** 根据显示名称设置热键
    * @param [in] hotKeyName 热键名称，比如"Ctrl + C"等
    */
    bool SetHotKeyName(const std::wstring& hotKeyName);

public:
    /** 获取键的显示名称
    * @param [in] wVirtualKeyCode 虚拟键盘码，比如：VK_DOWN等
    * @param [in] fExtended 如果虚拟键代码是扩展密钥，则为 true；否则为 false。
    */
    static std::wstring GetKeyName(uint8_t wVirtualKeyCode, bool fExtended);

protected:

	//初始化
	virtual void OnInit() override;

private:
	/** 默认显示的文字
    */
	UiString m_defaultText;

    /** 编辑框控件的接口
    */
    HotKeyRichEdit* m_pRichEdit;
};

}//namespace ui

#endif //UI_CONTROL_HOTKEY_H_
