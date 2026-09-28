#ifndef EXAMPLES_MY_FORM_H_
#define EXAMPLES_MY_FORM_H_

#include "duilib/duilib.h"

// 基类 ui::WindowImplBase 声明在 duilib/Utils/WinImplBase.h（文件名不带 Window 前缀）
// 复制本文件后：把 MyForm 替换为实际的窗口类名，同步替换 MY_FORM_H_ 宏。
class MyForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MyForm();
    virtual ~MyForm() override;

    // 返回皮肤目录名：bin/resources/themes/default/<此处返回值>/
    virtual DString GetSkinFolder() override;
    // 返回布局文件名：bin/resources/themes/default/<skin_folder>/<此处返回值>
    virtual DString GetSkinFile() override;

    // 窗口创建完成、控件已就绪，在这里查找控件并绑定事件
    virtual void OnInitWindow() override;
};

#endif // EXAMPLES_MY_FORM_H_
