#ifndef EXAMPLES_APP_ITEM_UI_H_
#define EXAMPLES_APP_ITEM_UI_H_

// duilib
#include "duilib/duilib.h"

//app的具体信息，这里假定有id，name，_icon,_isFrequent自行拓展
struct AppItem
{
    std::string m_id;
    DString m_name;
    DString m_icon;
    bool m_isFrequent = false;
};

//App UI类
class AppItemUi : public ui::ControlDragableT<ui::VBox>
{
    typedef ui::ControlDragableT<ui::VBox> BaseClass;
public:
    static AppItemUi* Create(const AppItem& item, Box* p);

    explicit AppItemUi(ui::Window* pWindow);
    virtual void OnInit() override;

    /** 设置应用数据
    * @param [in] item 应用数据
    * @param [in] bRefresh true表示同时刷新界面显示（控件已经初始化完成时使用）
    */
    void SetAppData(const AppItem& item, bool bRefresh);

    /** 获取应用数据
    */
    inline const AppItem& GetAppData() const { return m_app_data; }

    /** 常用控件的容器
    */
    void SetFrequentBox(ui::Box* pFrequentBox);

private:
    /** 控件所属容器拖动完成事件（在同一个容器/或者不同容器内）
    * @param [in] pOldBox 控件原来所属的容器接口
    * @param [in] nOldItemIndex 原来的子项索引号
    * @param [in] pNewBox 控件最新所属的容器接口
    * @param [in] nNewItemIndex 最新的子项索引号
    */
    virtual void OnItemBoxChanged(Box* pOldBox, size_t nOldItemIndex,
                                  Box* pNewBox, size_t nNewItemIndex) override;

    /** 创建拖拽过程中的占位控件（即图标将要插入的位置）
    * @param [in] pTargetBox 目标容器
    * @return 返回占位控件
    */
    virtual ui::Control* CreateDestControl(Box* pTargetBox) override;

private:
    AppItem m_app_data;
    ui::Control* m_app_icon = nullptr;
    ui::Label* m_app_name = nullptr;

    /** 常用控件的容器
    */
    Box* m_pFrequentBox = nullptr;
};
#endif //EXAMPLES_APP_ITEM_UI_H_
