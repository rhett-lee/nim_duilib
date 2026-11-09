#ifndef EXAMPLES_ITEM_H_
#define EXAMPLES_ITEM_H_

// duilib
#include "duilib/duilib.h"

#include <functional>

// 从 ui::ListBoxItem 中继承所有可用功能
class Item : public ui::ControlDragableT<ui::ListBoxItem>
{
public:
    explicit Item(ui::Window* pWindow);
    virtual ~Item() override;

    /** 提供外部调用来初始化 item 数据
    * @param [in] img 图标文件名（相对皮肤目录）
    * @param [in] title 任务名称
    * @param [in] nProgress 进度值，取值范围 [0, 100]
    */
    void InitSubControls(const DString& img, const DString& title, int32_t nProgress);

    /** 设置"本项被删除后"的回调，用于让列表窗口刷新统计信息
    */
    void SetOnRemoved(std::function<void()> onRemoved);

    /** 更新任务名称（语言切换时使用）
    */
    void SetTitleText(const DString& title);

private:
    /** 删除事件
    */
    bool OnRemove(const ui::EventArgs& args);

private:
    /** 列表容器
    */
    ui::ListBox* m_pListBox;

    /** 图片
    */
    ui::Control* m_pImageControl;

    /** 文字标题
    */
    ui::Label* m_pTitleLabel;

    /** 进度百分比文字
    */
    ui::Label* m_pPctLabel;

    /** 进度条
    */
    ui::Progress* m_pProgressControl;

    /** 删除按钮
    */
    ui::Button* m_pDelBtn;

    /** 本项被删除后的回调
    */
    std::function<void()> m_onRemoved;
};

#endif //#define EXAMPLES_ITEM_H_
