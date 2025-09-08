#ifndef UI_CONTROL_ADDRESS_BAR_H_
#define UI_CONTROL_ADDRESS_BAR_H_

#include "duilib/Box/HBox.h"

namespace ui
{
class RichEdit;

/** 地址栏控件，用于显示本地文件系统的路径
*/
class AddressBar : public HBox
{
    typedef HBox BaseClass;
public:
    explicit AddressBar(Window* pWindow);

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void SetPos(UiRect rc) override;

    /** 设置路径
    */
    void SetAddressPath(const DString& addressPath);

    /** 获取路径
    */
    DString GetAddressPath() const;

    /** 显示或者隐藏地址栏编辑框
    * @param [in] bShow true表示显示编辑框，false表示隐藏编辑框
    */
    void ShowAddressEdit(bool bShow);

    /** 最近一次用户在地址栏子路径按钮上点击的子路径
    */
    DString GetClickedAddressPath() const;

    /** 获取上次控件显示的路径
    */
    DString GetPreviousAddressPath() const;

public:
    /** 设置是否显示路径的tooltip
    */
    void SetEnablePathTooltip(bool bEnable);

    /** 获取是否显示路径的tooltip
    */
    bool IsEnablePathTooltip() const;

    /** 设置按回车时自动更新显示控件
    */
    void SetReturnUpdateUI(bool bUpdateUI);

    /** 获取按回车时自动更新显示控件
    */
    bool IsReturnUpdateUI() const;

    /** 设置按ESC时自动更新显示控件
    */
    void SetEscUpdateUI(bool bUpdateUI);

    /** 获取按ESC时自动更新显示控件
    */
    bool IsEscUpdateUI() const;

    /** 设置失去焦点时自动更新显示控件
    */
    void SetKillFocusUpdateUI(bool bUpdateUI);

    /** 获取失去焦点时自动更新显示控件
    */
    bool IsKillFocusUpdateUI() const;

    /** 设置编辑框的Class
    */
    void SetRichEditClass(const DString& editClass);

    /** 获取编辑框的Class
    */
    DString GetRichEditClass() const;

    /** 设置编辑框的清除按钮Class
    */
    void SetRichEditClearBtnClass(const DString& clearBtnClass);

    /** 获取编辑框的清除按钮Class
    */
    DString GetRichEditClearBtnClass() const;

    /** 设置地址栏路径的容器（HBox）Class，每个子路径一个HBox容器
    */
    void SetSubPathHBoxClass(const DString& hboxClass);

    /** 获取地址栏路径的容器（HBox）Class
    */
    DString GetSubPathHBoxClass() const;

    /** 设置地址栏子路径按钮的Class
    */
    void SetSubPathBtnClass(const DString& subPathBtnClass);
         
    /** 获取地址栏子路径按钮的Class
    */
    DString GetSubPathBtnClass() const;

    /** 设置地址栏根路径的Class（"/"路径）
    */
    void SetSubPathRootClass(const DString& subPathRootClass);

    /** 获取地址栏根路径的Class（"/"路径）
    */
    DString GetSubPathRootClass() const;

    /** 设置地址栏路径分隔符的Class
    */
    void SetPathSeparatorClass(const DString& pathSeparatorClass);

    /** 获取地址栏路径分隔符的Class
    */
    DString GetPathSeparatorClass() const;

public:
    /** 监听地址栏上的路径变化事件（当回车或者失去焦点时，地址栏编辑框数据会应用）
    * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
    */
    void AttachPathChanged(const EventCallback& callback) { AttachEvent(kEventPathChanged, callback); }

    /** 监听地址栏上用户在子路径按钮上的点击事件
    * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
    */
    void AttachPathClick(const EventCallback& callback) { AttachEvent(kEventPathClick, callback); }

    /** 监听地址栏上的回车事件
    * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
    */
    void AttachReturn(const EventCallback& callback) { AttachEvent(kEventReturn, callback); }

    /** 监听地址栏上的ESC事件
    * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
    */
    void AttachEsc(const EventCallback& callback) { AttachEvent(kEventEsc, callback); }

protected:
    /** 初始化
    */
    virtual void OnInit() override;

    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

private:
    /** 添加一个路径
    * @param [in] displayName 显示名称
    * @param [in] filePath 文件的本地路径
    */
    bool AddSubPath(const DString& displayName, const DString& filePath);

    /** 地址栏编辑框上输入回车
    */
    void OnAddressBarReturn();

    /** 地址栏编辑框上输入ESC
    */
    void OnAddressBarEsc();

    /** 获取焦点事件
    */
    void OnAddressBarSetFocus(Control* pNewFocus);

    /** 失去焦点事件
    */
    void OnAddressBarKillFocus(Control* pNewFocus);

    /** 用户点击了某个子路径
    */
    void OnClickedSubPath(const DString& filePath);

    /** 设置显示地址控件
    */
    bool UpdateAddressBarControls(const DString& addressPath);

    /** 更新显示地址控件的状态
    */
    void UpdateAddressBarControlsStatus();

    /** 动态计算每个控件的显示宽度
    */
    std::vector<int32_t> AdjustControlsWidth(const std::vector<int32_t>& originalWidths, int32_t totalWidth);

private:
    /** 地址栏编辑框控件
    */
    RichEdit* m_pRichEdit;

    /** 地址栏选择界面控件
    */
    HBox* m_pBarBox;

    /** 当前设置的路径
    */
    UiString m_addressPath;

    /** 当前控件显示的路径
    */
    UiString m_showAddressPath;

    /** 前次控件显示的路径
    */
    UiString m_prevShowAddressPath;

    /** 当前点击的子路径
    */
    UiString m_clickedAddressPath;

    /** 编辑框的Class
    */
    UiString m_editClass;

    /** 编辑框的清除按钮Class
    */
    UiString m_editClearBtnClass;

    /** 地址栏子路径的容器（HBox）Class
    */
    UiString m_subPathHBoxClass;

    /** 地址栏子路径按钮的Class
    */
    UiString m_subPathBtnClass;

    /** 地址栏根路径的Class（"/"路径）
    */
    UiString m_subPathRootClass;

    /** 地址栏路径分隔符的Class
    */
    UiString m_pathSeparatorClass;

    /** 是否显示tooltip
    */
    bool m_bEnableTooltip;

    /** 按回车时自动更新显示控件
    */
    bool m_bReturnUpdateUI;

    /** 按ESC时自动更新显示控件
    */
    bool m_bEscUpdateUI;

    /** 失去焦点时自动更新显示控件
    */
    bool m_bKillFocusUpdateUI;

    /** 当前是否正在执行更新操作
    */
    bool m_bUpdatingUI;
};

}//namespace ui

#endif //UI_CONTROL_ADDRESS_BAR_H_
