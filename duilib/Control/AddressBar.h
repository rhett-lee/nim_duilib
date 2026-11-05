#ifndef UI_CONTROL_ADDRESS_BAR_H_
#define UI_CONTROL_ADDRESS_BAR_H_

#include "duilib/Box/HBox.h"
#include "duilib/Control/Button.h"
#include <functional>
#include <vector>

namespace ui
{
class RichEdit;
class Menu;
class AutoCompleteFlyout;

/** 地址栏控件，用于显示本地文件系统的路径
*/
class DUILIB_API AddressBar : public HBox
{
    typedef HBox BaseClass;

    //内部编辑框子类需要调用私有的按键预处理接口
    friend class AddressBarEdit;
public:
    explicit AddressBar(Window* pWindow);
    virtual ~AddressBar() override;

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void SetPos(UiRect rc) override;

    /** 按键处理（焦点在面包屑子控件上时处理方向键）
    */
    virtual bool OnKeyDown(const EventArgs& msg) override;

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

    /** 是否处于编辑状态（编辑框可见）
    */
    bool IsEditing() const;

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

    /** 设置编辑框右侧“恢复当前路径”按钮的Class
    */
    void SetResetButtonClass(const DString& resetButtonClass);

    /** 获取编辑框右侧“恢复当前路径”按钮的Class
    */
    DString GetResetButtonClass() const;

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

    /** 设置地址栏盘符根路径的Class（Windows盘符）
    */
    void SetSubPathDriveClass(const DString& subPathDriveClass);

    /** 获取地址栏盘符根路径的Class
    */
    DString GetSubPathDriveClass() const;

    /** 设置地址栏当前路径（最后一段）的强调Class
    */
    void SetSubPathCurrentClass(const DString& subPathCurrentClass);

    /** 获取地址栏当前路径（最后一段）的强调Class
    */
    DString GetSubPathCurrentClass() const;

    /** 设置折叠段（"…"）按钮的Class
    */
    void SetCollapseBtnClass(const DString& collapseBtnClass);

    /** 获取折叠段（"…"）按钮的Class
    */
    DString GetCollapseBtnClass() const;

    /** 设置地址栏路径分隔符的Class
    */
    void SetPathSeparatorClass(const DString& pathSeparatorClass);

    /** 获取地址栏路径分隔符的Class
    */
    DString GetPathSeparatorClass() const;

    /** 设置地址栏右侧的空白宽度（DIP）
    */
    void SetRightPadding(int32_t nPadding);

    /** 获取地址栏右侧的空白宽度（DIP）
    */
    int32_t GetRightPadding() const;

    /** 设置路径为空时的显示文本（如“此电脑”）
    */
    void SetEmptyDisplayText(const DString& text);

    /** 获取路径为空时的显示文本
    */
    DString GetEmptyDisplayText() const;

    /** 设置路径为空时显示文本的多语言ID
    */
    void SetEmptyDisplayTextId(const DString& textId);

    /** 获取路径为空时显示文本的多语言ID
    */
    DString GetEmptyDisplayTextId() const;

    /** 设置是否记录路径历史（用于自动补全）
    */
    void SetHistoryEnabled(bool bEnable);

    /** 获取是否记录路径历史
    */
    bool IsHistoryEnabled() const;

    /** 设置路径历史最多保留条数
    */
    void SetMaxHistory(int32_t nMax);

    /** 获取路径历史最多保留条数
    */
    int32_t GetMaxHistory() const;

    /** 追加一条路径历史（已存在时移动到最前面）
    */
    void AddPathHistory(const DString& path);

    /** 获取路径历史（最近使用的在最前面）
    */
    std::vector<DString> GetPathHistory() const;

    /** 清空路径历史
    */
    void ClearPathHistory();

    /** 设置是否支持拖放目录到地址栏进行导航
    */
    void SetEnableDropNavigation(bool bEnable);

    /** 获取是否支持拖放目录到地址栏进行导航
    */
    bool IsEnableDropNavigation() const;

public:
    /** 监听地址栏上的路径变化事件（当回车或者失去焦点时，地址栏编辑框数据会应用）
    * @param [in] callback 事件处理的回调函数，请参考 EventCallback 声明
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachPathChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventPathChanged, callback, callbackID); }

    /** 监听地址栏上用户在子路径按钮上的点击事件
    * @param [in] callback 事件处理的回调函数，请参考 EventCallback 声明
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachPathClick(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventPathClick, callback, callbackID); }

    /** 预提交回调：用户回车/失焦时，先交给业务层确认（或规范化路径）
    *   返回false表示拒绝提交，地址栏保持原路径且不派发 kEventPathChanged；
    *   回调中可以修改 newPath（例如把文件路径规范化为其父目录）。
    */
    typedef std::function<bool (DString& newPath)> AddressBarPathChangingEvent;
    void AttachPathChanging(const AddressBarPathChangingEvent& callback);

    /** 解除预提交回调
    */
    void DetachPathChanging();

    /** 监听地址栏上的回车事件
    * @param [in] callback 事件处理的回调函数，请参考 EventCallback 声明
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachReturn(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventReturn, callback, callbackID); }

    /** 监听地址栏上的ESC事件
    * @param [in] callback 事件处理的回调函数，请参考 EventCallback 声明
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachEsc(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventEsc, callback, callbackID); }

protected:
    /** 初始化
    */
    virtual void OnInit() override;

    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

    /** 语言切换：刷新“此电脑”等文本
    */
    virtual void OnLanguageChanged(bool bRedraw) override;

private:
    /** 路径分段信息
    */
    struct PathSegment
    {
        DString m_displayName;  //显示名称
        DString m_fullPath;     //完整的本地路径
        bool m_bRoot;           //是否为根节点（盘符、UNC根、Unix根）
        bool m_bDrive;          //是否为Windows盘符根
        bool m_bComputer;       //是否为“此电脑”（路径为空时的占位显示）
    };

    /** 保证第nIndex个路径段的控件已经创建（复用已有控件）
    */
    void EnsureSubPathControls(size_t nIndex);

    /** 更新第nIndex个路径段控件的显示内容
    */
    void UpdateSubPathControls(size_t nIndex, const PathSegment& segment, bool bShowSeparator, bool bCurrent);

    /** 根据路径构建路径段列表
    */
    void BuildPathSegments(const DString& addressPath, std::vector<PathSegment>& segments) const;

    /** 面包屑按钮之间移动键盘焦点
    */
    void MoveSubPathFocus(size_t nIndex, bool bLeft);

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

    /** 判断控件是否在this的子树内
    */
    bool IsChildControl(const Control* pControl) const;

    /** 用户点击了某个子路径
    */
    void OnClickedSubPath(const DString& filePath);

    /** 设置显示地址控件
    * @param [in] addressPath 需要显示的路径
    * @param [in] bForce true表示即使路径相同也强制刷新（用于刷新磁盘卷名等）
    * @return 返回true表示显示路径发生变化
    */
    bool UpdateAddressBarControls(const DString& addressPath, bool bForce);

    /** 更新显示地址控件的状态（计算折叠与宽度）
    */
    void UpdateAddressBarControlsStatus();

    /** 提交路径（回车/失焦/拖放共用的入口）
    * @param [in] rawPath 用户输入或拖放得到的路径
    * @param [in] bHideEdit true表示提交后退出编辑状态
    * @return 返回true表示提交被接受
    */
    bool CommitAddressPath(const DString& rawPath, bool bHideEdit);

    /** 恢复编辑框文本为当前显示路径
    */
    void ResetEditText();

    /** 显示折叠路径的下拉菜单
    */
    void ShowCollapsedPathMenu();

    /** 更新自动补全浮层
    */
    void UpdateAutoCompletePopup();

    /** 打开自动补全浮层
    */
    void OpenAutoCompletePopup();

    /** 关闭自动补全浮层
    */
    void CloseAutoCompletePopup();

    /** 生成自动补全候选列表
    */
    void BuildAutoCompleteCandidates(const DString& input);

    /** 编辑框按键预处理（浮层打开时拦截上下/回车/Esc）
    * @return true 表示按键已处理，不再传递给编辑框
    */
    bool HandleEditKeyDown(const EventArgs& msg);

    /** 拖放数据到达
    */
    bool OnDropData(const EventArgs& msg);

private:
    /** 地址栏编辑框控件
    */
    RichEdit* m_pRichEdit;

    /** 地址栏选择界面控件
    */
    HBox* m_pBarBox;

    /** 编辑框右侧的“恢复当前路径”按钮
    */
    Button* m_pResetBtn;

    /** 中间折叠段（"…"）按钮
    */
    Button* m_pCollapseBtn;

    /** 当前显示的路径段信息（与m_pBarBox中的HBox一一对应）
    */
    std::vector<PathSegment> m_pathSegments;

    /** 路径段对应的容器控件（复用，不删除）
    */
    std::vector<HBox*> m_subPathBoxes;

    /** 被折叠的路径段（供"…"下拉菜单使用）
    */
    std::vector<PathSegment> m_collapsedSegments;

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

    /** 编辑框右侧“恢复当前路径”按钮的Class
    */
    UiString m_resetBtnClass;

    /** 地址栏子路径的容器（HBox）Class
    */
    UiString m_subPathHBoxClass;

    /** 地址栏子路径按钮的Class
    */
    UiString m_subPathBtnClass;

    /** 地址栏根路径的Class（"/"路径）
    */
    UiString m_subPathRootClass;

    /** 地址栏盘符根路径的Class（Windows盘符）
    */
    UiString m_subPathDriveClass;

    /** 地址栏当前路径（最后一段）的强调Class
    */
    UiString m_subPathCurrentClass;

    /** 折叠段（"…"）按钮的Class
    */
    UiString m_collapseBtnClass;

    /** 地址栏路径分隔符的Class
    */
    UiString m_pathSeparatorClass;

    /** 路径为空时的显示文本
    */
    UiString m_emptyDisplayText;

    /** 路径为空时显示文本的多语言ID
    */
    UiString m_emptyDisplayTextId;

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

    /** 右侧空白宽度（DIP）
    */
    int32_t m_nRightPadding;

    /** 预提交回调
    */
    AddressBarPathChangingEvent m_pathChangingCallback;

    /** 路径历史（最近使用的在最前面）
    */
    std::vector<UiString> m_pathHistory;

    /** 路径历史最多保留条数
    */
    int32_t m_nMaxHistory;

    /** 是否记录路径历史
    */
    bool m_bHistoryEnabled;

    /** 是否支持拖放导航
    */
    bool m_bEnableDropNavigation;

    /** 自动补全浮层（关闭后由框架销毁并置空）
    */
    AutoCompleteFlyout* m_pFlyout;

    /** 自动补全候选列表
    */
    std::vector<UiString> m_candidates;

    /** 自动补全刷新防重入标记
    */
    bool m_bUpdatingPopup;

    /** 用户是否已用上下键导航候选
    */
    bool m_bUserNavigated;
};

}//namespace ui

#endif //UI_CONTROL_ADDRESS_BAR_H_
