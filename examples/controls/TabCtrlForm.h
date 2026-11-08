#ifndef EXAMPLES_TABCTRL_FORM_H_
#define EXAMPLES_TABCTRL_FORM_H_

// duilib
#include "duilib/duilib.h"

/** 标签栏控件（TabCtrl）测试窗口
*
*  覆盖 E-7 三项能力：
*   ① 垂直标签：运行时 SetTabCtrlOrientation 在水平/垂直之间切换
*   ② 固定标签：TabCtrlItem::SetFixed（追加固定标签、固定/取消选中项、删除、重置）
*   ③ 溢出滚动：分水平、垂直两个用例，都提供"加满溢出 / 开关 / 滚动定位"按钮，
*      并实时显示可视尺寸、内容尺寸、溢出量、滚动位置与滚动条状态（可断言）
*  另外演示标签角标（数字 / 99+ / 小红点 / 关闭按钮）与程序化增删标签
*  （AddItem / AddItemAt / RemoveItemAt）及固定块的锚定约束。
*
*  覆盖 C1–C10 新增能力（卡片 ⑥–⑭，每项功能都可以手工测试）：
*   C1 TabBox ↔ TabCtrl 双向同步（tab_box_name，含同步事件计数，可验证无死循环）
*   C2 关闭请求事件 kEventTabClose（只请求不删除）/ closable / 中键关闭开关
*   C3 选中项自动滚入视野
*   C4 键盘导航（Ctrl+Tab / Ctrl+Shift+Tab / Ctrl+1..9 / 方向键）
*   C5 滚轮切标签（默认关闭，可开启）/ 拖动排序与插入提示
*   C6 溢出箭头按钮（与浮动滚动条二选一）与翻页滚动
*   C7 等宽填充 tab_item_justify 与 min_width / max_width
*   C8 新建标签按钮 new_tab_button + kEventTabNew
*   C9 便捷 API（AddTabItemToEnd / Get/SetSelectedTabItem / SelectNext/PrevTab）
*   C10 垂直模式的分割线与边距适配（卡片 ① 切到垂直、卡片 ④）
*
*  【多语言约定】所有测试窗口都必须支持简体中文与英文双语：
*   - 皮肤 XML 里一律用 text_id / title_id / tooltip_text_id，不写死 text / title / tooltip_text；
*   - 文案统一登记到 bin/resources/lang/zh_CN-examples.txt 与 en_US-examples.txt，
*     两份文件的键集合必须完全一致；
*   - 代码动态生成的文本走 TabCtrlLang()（即 GlobalManager::Lang().GetStringByID），
*     并覆写 OnLanguageChanged() 重建这些动态文本。
*/
class TabCtrlForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;

public:
    TabCtrlForm();
    virtual ~TabCtrlForm() override;

    /** 资源相关接口
    * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
    * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 语言切换事件：XML 中的 text_id 由框架自动刷新，这里重建代码生成的动态文本
    * @return 返回false表示不再触发窗口的kWindowLanguageChangedMsg事件
    */
    virtual bool OnLanguageChanged() override;

private:
    /** 刷新全部状态文字（初始化与语言切换时统一调用）
    */
    void RefreshAllState();

    /** 创建一个标签项（用模板 XML 填充外观）
    * @param [in] pTabCtrl 所属标签栏
    * @param [in] title 标签文字
    * @param [in] bFixed 是否为固定标签
    * @param [in] nFixedWidth 标签项宽度；传 0 表示沿用模板中的宽度（用于触发横向溢出）
    * @param [in] bAddToCtrl 是否立即加入标签栏（false 时由调用方自行 AddItem/AddTabItemToEnd）
    * @return 返回新建的标签项指针，失败返回 nullptr
    */
    ui::TabCtrlItem* CreateTabItem(ui::TabCtrl* pTabCtrl, const DString& title, bool bFixed,
                                   int32_t nFixedWidth = 0, bool bAddToCtrl = true);

    /** 找到指定名字的标签栏控件
    */
    ui::TabCtrl* FindTabCtrl(const DString& name);

    /** 绑定一个按钮的点击事件（找不到控件时静默跳过）
    * @param [in] name 按钮控件名
    * @param [in] callback 点击回调
    */
    void BindClick(const DString& name, const ui::EventCallback& callback);

    /** 更新固定标签卡片右侧的状态文字（固定标签数量 + 列表 + 选中项）
    */
    void UpdateFixedState();

    /** 更新方向卡片右侧的状态文字
    */
    void UpdateOrientState();

    /** 更新水平溢出滚动卡片的状态文字
    *
    *  显示：标签数、可视宽度、内容宽度、溢出量、滚动位置/滚动范围、滚动条是否有效。
    *  其中内容宽度由"可视宽度 + 滚动范围"推得（滚动范围即溢出的像素数）。
    */
    void UpdateHOverflowState();

    /** 更新垂直溢出滚动卡片的状态文字（含义同 UpdateHOverflowState，改为纵向）
    */
    void UpdateVOverflowState();

    /** 切换某个标签栏的溢出滚动开关，并同步按钮文字
    * @param [in] name 标签栏控件名
    * @param [in] btnName 开关按钮控件名
    */
    void ToggleOverflowScroll(const DString& name, const DString& btnName);

    /** 把标签栏填满到指定数量（已存在则补齐，不足则追加，超出则删除末尾）
    * @param [in] name 标签栏控件名
    * @param [in] nTarget 目标标签数量
    * @param [in] nStartIndex 起始编号（用于生成标签文字）
    * @param [in] nFixedWidth 新建标签项的固定宽度；传 0 表示用模板默认宽度
    * @return 返回true表示标签数量发生了变化
    */
    bool FillTabCtrl(const DString& name, size_t nTarget, int32_t& nStartIndex, int32_t nFixedWidth = 0);

    //======================= C1–C9 卡片的状态刷新 =======================

    /** 更新 C1（TabBox ↔ TabCtrl 双向同步）卡片的状态文字
    */
    void UpdateC1State();

    /** 更新 C2（关闭请求 / 可关闭 / 中键关闭）卡片的状态文字
    */
    void UpdateC2State();

    /** 更新 C3（选中项自动滚入视野）卡片的状态文字
    */
    void UpdateC3State();

    /** 更新 C4（键盘导航）卡片的状态文字
    */
    void UpdateC4State();

    /** 更新 C5（滚轮切标签 / 拖动排序）卡片的状态文字
    */
    void UpdateC5State();

    /** 更新 C6（溢出箭头按钮）卡片的状态文字
    */
    void UpdateC6State();

    /** 更新 C7（等宽填充 / min-max 宽度）卡片的状态文字
    */
    void UpdateC7State();

    /** 更新 C8（新建标签按钮）卡片的状态文字
    */
    void UpdateC8State();

    /** 更新 C9（便捷 API）卡片的状态文字
    */
    void UpdateC9State();

    //======================= ⑮–㉒ 界面美观与交互卡片的状态刷新 =======================

    /** 更新指示条滑动动画卡片的状态文字
    */
    void UpdateC15State();

    /** 更新选中态层次（加粗 / 淡底色）卡片的状态文字
    */
    void UpdateC16State();

    /** 更新选中 + 悬停反馈卡片的状态文字
    */
    void UpdateC17State();

    /** 更新垂直模式分隔线卡片的状态文字
    */
    void UpdateC18State();

    /** 延迟更新垂直模式分隔线卡片的状态文字（等待标签栏重新布局完成）
    */
    void DeferUpdateC18State();

    /** 更新底边线断开策略卡片的状态文字
    */
    void UpdateC19State();

    /** 更新焦点框 / 禁用态卡片的状态文字
    */
    void UpdateC20State();

    /** 更新浮动角标卡片的状态文字
    */
    void UpdateC21State();

    /** 更新关闭按钮显示策略卡片的状态文字
    */
    void UpdateC22State();

    /** 刷新所有"开关"按钮的文字（初始化与语言切换后调用）
    */
    void RefreshToggleButtons();

    /** 设置开关按钮的文字
    * @param [in] btnName 按钮控件名
    * @param [in] formatId 格式串的 STRID（其中包含一个 %s，用于填入开/关）
    * @param [in] bOn 当前是否为开启状态
    */
    void SetToggleText(const DString& btnName, const DString& formatId, bool bOn);

    /** 取标签栏当前选中项的文字，无选中项时返回"无"
    */
    DString GetSelectedTitle(ui::TabCtrl* pTabCtrl) const;

    /** 程序化新增标签时使用的自增序号
    */
    int32_t m_nTabIndex;

    /** C1：TabBox 选择事件次数（每次选择变化应为 1 次，可验证同步没有死循环）
    */
    int32_t m_nTabBoxSelectCount;

    /** C2：关闭请求次数与最后一次请求的标签索引
    */
    int32_t m_nCloseRequestCount;
    int32_t m_nCloseRequestIndex;

    /** C8：新建标签事件次数
    */
    int32_t m_nNewTabCount;

    /** C9：最近一次便捷 API 操作的描述
    */
    DString m_c9LastAction;
};

#endif //EXAMPLES_TABCTRL_FORM_H_
