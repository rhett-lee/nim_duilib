#ifndef EXAMPLES_ITEM_H_
#define EXAMPLES_ITEM_H_

// duilib
#include "duilib/duilib.h"

// 从 ui::ListBoxItem 中继承所有可用功能
class Item : public ui::ListBoxItem
{
public:
    explicit Item(ui::Window* pWindow);
    virtual ~Item();

    // 提供外部调用来初始化 item 数据
    void InitSubControls(const DString& img, const DString& title, size_t nDataIndex);
private:
    bool OnRemove(const ui::EventArgs& args);

private:
    ui::ListBox*    list_box_;

    ui::Control*    control_img_;
    ui::Label*        label_title_;
    ui::Progress*    progress_;
    ui::Button*        btn_del_;
    int64_t         t_time;
    size_t m_nDataIndex;
};

#endif //EXAMPLES_ITEM_H_
