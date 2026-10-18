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
    * @return 返回新建的标签项指针，失败返回 nullptr
    */
    ui::TabCtrlItem* CreateTabItem(ui::TabCtrl* pTabCtrl, const DString& title, bool bFixed, int32_t nFixedWidth = 0);

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

    /** 程序化新增标签时使用的自增序号
    */
    int32_t m_nTabIndex;
};

#endif //EXAMPLES_TABCTRL_FORM_H_