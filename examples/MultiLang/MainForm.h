#ifndef MAIN_FORM_H_
#define MAIN_FORM_H_

#pragma once

// duilib
#include "duilib/duilib.h"

/** 应用程序的主窗口实现
*/
class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
    virtual ~MainForm();

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual std::wstring GetSkinFolder() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual std::wstring GetSkinFile() override;

    /** 创建窗口时被调用，由子类实现用以获取窗口唯一的类名称
    * @return 子类需实现并返回窗口唯一的类名称
    */
    virtual std::wstring GetWindowClassName() const override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 收到 WM_CLOSE 消息时该函数会被调用
     */
    virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    /** 窗口的类名
    */
    static const std::wstring kClassName;

private:
    /** 显示菜单
    * @param [in] point 显示位置坐标，屏幕坐标
    */
    void ShowPopupMenu(const ui::UiPoint& point);
};

#endif //MAIN_FORM_H_

