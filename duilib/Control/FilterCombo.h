#ifndef UI_CONTROL_FILTERCOMBO_H_
#define UI_CONTROL_FILTERCOMBO_H_

#include "duilib/Control/Combo.h"

namespace ui 
{

/** 带有过滤功能的组合框
*/
class UILIB_API FilterCombo : public Combo
{
    typedef Combo BaseClass;
public:
    explicit FilterCombo(Window* pWindow);
    FilterCombo(const FilterCombo& r) = delete;
    Combo& operator=(const FilterCombo& r) = delete;
    virtual ~FilterCombo() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

protected:
    virtual void OnInit() override;
    
protected:
    /** 鼠标在Edit上面按下按钮
     * @param[in] args 参数列表
     * @return 始终返回 true
     */
    virtual bool OnEditButtonDown(const EventArgs& args) override;

    /** 鼠标在Edit上面弹起按钮
     * @param[in] args 参数列表
     * @return 始终返回 true
     */
    virtual bool OnEditButtonUp(const EventArgs& args) override;

    /** Edit的文本内容发生变化
     * @param[in] args 参数列表
     * @return 始终返回 true
     */
    virtual bool OnEditTextChanged(const ui::EventArgs& args) override;

private:

    /** 对下拉框列表里面的内容进行过滤
    */
    void FilterComboList(const DString& filterText);

    /** 判断文本是否满足过滤条件
    */
    bool IsFilterText(const DString& filterText, const DString& itemText) const;
};

} // namespace ui

#endif // UI_CONTROL_FILTERCOMBO_H_
