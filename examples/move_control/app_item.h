#ifndef EXAMPLES_APP_ITEM_H_
#define EXAMPLES_APP_ITEM_H_

// duilib
#include "duilib/duilib.h"

#define APP_HEIGHT 90
#define APP_WIDTH  90
#define EACH_LINE  6

//app的具体信息，这里假定有id，name，_icon,_isFrequent自行拓展
struct AppItem
{
    std::string m_id;
    DString m_name;
    DString m_icon;
    bool m_isFrequent=false;
};

//App UI类
class AppItemUi : public ui::ControlDragableT<ui::VBox>
{
public:
    static AppItemUi* Create(const AppItem& item, Box* p);

    explicit AppItemUi(ui::Window* pWindow);
    virtual void OnInit() override;
    void SetAppdata(const AppItem& item,bool refresh);
    inline const AppItem& getAppData() const { return m_app_data; }

    /** 常用控件的容器
    */
    void SetFrequentBox(ui::Box* pFrequentBox);

private:
    /** 目标位置指示控件
    * @param [in] pTargetBox 即将拖入的目标Box控件接口
    */
    virtual Control* CreateDestControl(Box* pTargetBox) override;

    /** 控件所属容器拖动完成事件（在同一个容器/或者不同容器内）
    * @param [in] pOldBox 控件原来所属的容器接口
    * @param [in] nOldItemIndex 原来的子项索引号
    * @param [in] pNewBox 控件最新所属的容器接口
    * @param [in] nNewItemIndex 最新的子项索引号
    */
    virtual void OnItemBoxChanged(Box* pOldBox, size_t nOldItemIndex, 
                                  Box* pNewBox, size_t nNewItemIndex) override;

private:
    AppItem m_app_data;
    ui::Control* m_app_icon = nullptr;
    ui::Label* m_app_name = nullptr;

    /** 常用控件的容器
    */
    Box* m_pFrequentBox = nullptr;
};
#endif //EXAMPLES_APP_ITEM_H_
