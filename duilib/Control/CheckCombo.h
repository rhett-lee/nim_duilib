#ifndef UI_CONTROL_CHECKCOMBO_H_
#define UI_CONTROL_CHECKCOMBO_H_

#include "duilib/Box/ScrollBox.h"
#include "duilib/Core/Window.h"

namespace ui
{

class CCheckComboWnd;
class UILIB_API CheckCombo : public Box
{
    typedef Box BaseClass;
    friend class CCheckComboWnd;
public:
    explicit CheckCombo(Window* pWindow);
    CheckCombo(const CheckCombo& r) = delete;
    CheckCombo& operator=(const CheckCombo& r) = delete;
    virtual ~CheckCombo() override;

public:
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void Activate(const EventArgs* pMsg) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual bool AddItem(Control* pControl) override;
    virtual bool AddItemAt(Control* pControl, size_t iIndex) override;
    virtual bool RemoveItem(Control* pControl) override;
    virtual bool RemoveItemAt(size_t iIndex) override;
    virtual void RemoveAllItems() override;
    virtual Control* GetItemAt(size_t iIndex) const override;
    virtual size_t GetItemIndex(Control* pControl) const override;
    virtual bool SetItemIndex(Control* pControl, size_t iIndex) override;
    virtual size_t GetItemCount() const override;

    /** 添加一个下拉框的列表项
    */
    bool AddTextItem(const DString& itemText);

    /** 获取选择的文本列表
    */
    void GetSelectedText(std::vector<DString>& selectedText) const;

    /** 清除所有列表项和选择项
    */
    void ClearAll();

public:
    /** 获取当前所属的 List 对象
    */
    ScrollBox* GetListBox() { return m_pDropList.get(); }

    /** 设置下拉框的属性信息
    * @param [in] pstrList 转义后的 XML 格式属性列表
    */
    void SetDropBoxAttributeList(const DString& pstrList);

    /** 获取下拉框容器大小
    */
    const UiSize& GetDropBoxSize() const;

    /** 设置下拉框列表大小(宽度和高度)
     * @param [in] szDropBox 要设置的大小信息
     * @param [in] bNeedScaleDpi 是否需要做DPI自适应
     */
    void SetDropBoxSize(UiSize szDropBox, bool bNeedScaleDpi);

    /** 设置 Combobox 是否向上弹出
    * @param [in] top 为 true 则向上弹出，false 为向下弹出
    */
    void SetPopupTop(bool top) { m_bPopupTop = top; };

    /** 判断 Combobox 弹出模式是否是向上弹出
    */
    bool IsPopupTop() const { return m_bPopupTop; };

    /** 设置下拉列表中每一个列表项的属性
    */
    void SetDropboxItemClass(const DString& classValue);

    /** 设置选择项中每一个子项的属性
    */
    void SetSelectedItemClass(const DString& classValue);

    /** 更新下拉列表窗口的位置
    */
    void UpdateComboWndPos();

    /** 下拉框的窗口接口(只有在显示时能获取到，隐藏时即失效)
    */
    Window* GetCheckComboWnd() const;

    /** 设置下拉窗口的阴影类型
    */
    void SetComboWndShadowType(Shadow::ShadowType nShadowType);

    /** 获取下拉窗口的阴影类型
    */
    Shadow::ShadowType GetComboWndShadowType() const;

public:
    /** 监听下拉窗创建事件
     * @param[in] callback 下拉窗关闭后触发的回调函数
     */
    void AttachWindowCreate(const EventCallback& callback) { AttachEvent(kEventWindowCreate, callback); }

    /** 监听下拉窗关闭事件
    * @param[in] callback 下拉窗关闭后触发的回调函数
    */
    void AttachWindowClose(const ui::EventCallback& callback) { AttachEvent(ui::kEventWindowClose, callback); }

private:
    /** 默认的子项被选择处理函数
    * @param[in] args 参数列表
    * @return 始终返回 true
    */
    bool OnSelectItem(const ui::EventArgs& args);
    bool OnUnSelectItem(const ui::EventArgs& args);
    bool OnListButtonDown(const ui::EventArgs& args);

private:
    /** 解析属性列表
    */
    void ParseAttributeList(const DString& strList,
                            std::vector<std::pair<DString, DString>>& attributeList) const;

    /** 设置控件的属性列表
    */
    void SetAttributeList(Control* pControl, const DString& classValue);

    /** 更新选择列表的高度
    */
    void UpdateSelectedListHeight();

private:
    /** 下拉框的窗口接口
    */
    CCheckComboWnd* m_pCheckComboWnd;

    /** 阴影类型
    */
    Shadow::ShadowType m_nShadowType;

    /** 下拉列表容器
    */
    std::unique_ptr<ui::ScrollBox> m_pDropList;

    /** 选择的列表项容器
    */
    std::unique_ptr<ui::ScrollBox> m_pList;
    
    /** 下拉框的宽度和高度
    */
    ui::UiSize m_szDropBox;

    /** 下拉框是否向上弹出
    */
    bool m_bPopupTop;

    /** 容器的高度
    */
    int32_t m_iOrgHeight;

    /** 选择项的文本
    */
    std::vector<std::string> m_vecDate;

    /** 下拉列表中每一个列表项的属性
    */
    UiString m_dropboxItemClass;

    /** 选择项中每一个子项的属性
    */
    UiString m_selectedItemClass;
};

} //namespace ui

#endif //UI_CONTROL_CHECKCOMBO_H_
