#ifndef EXAMPLES_TIMELINE_FORM_H_
#define EXAMPLES_TIMELINE_FORM_H_

// duilib
#include "duilib/duilib.h"

/** Timeline 垂直时间轴演示窗口
 *  演示数据绑定、多状态节点、连接线样式、正序/倒序、节点点击事件、动态增删与清空。
*/
class TimelineForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    TimelineForm();
    virtual ~TimelineForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

private:
    /** 演示动态添加条目
    */
    void AddItem();

    /** 演示删除末个条目
    */
    void RemoveItem();

    /** 演示清空（展示空数据提示）
    */
    void ClearItems();

    /** 演示正序/倒序切换
    */
    void ToggleReverse();
};

#endif //EXAMPLES_TIMELINE_FORM_H_
