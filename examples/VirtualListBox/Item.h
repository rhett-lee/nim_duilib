#ifndef EXAMPLES_ITEM_H_
#define EXAMPLES_ITEM_H_

// duilib
#include "duilib/duilib.h"

// 从 ui::ListBoxItem 中继承所有可用功能
class Item : public ui::ListBoxItem
{
public:
    explicit Item(ui::Window* pWindow);
    virtual ~Item() override;

    /** 提供外部调用来初始化 item 数据
    * @param[in] img 图标资源名
    * @param[in] title 任务标题
    * @param[in] nDataIndex 数据元素索引号
    * @param[in] nProgress 任务进度，范围：[0, 100]
    */
    void InitSubControls(const DString& img, const DString& title, size_t nDataIndex, int32_t nProgress);

private:

    /** 删除事件
    */
    bool OnRemove(const ui::EventArgs& args);

private:
    /** 图片
    */
    ui::Control* m_pImageControl;

    /** 文字标题
    */
    ui::Label* m_pTitleLabel;

    /** 进度条
    */
    ui::Progress* m_pProgressControl;

    /** 进度文本
    */
    ui::Label* m_pPercentLabel;

    /** 删除按钮
    */
    ui::Button* m_pDelBtn;

    /** 数据元素索引号
    */
    size_t m_nDataIndex;

    /** 子控件是否已经查找并完成事件绑定
    */
    bool m_bSubControlsInited;
};

#endif //EXAMPLES_ITEM_H_
