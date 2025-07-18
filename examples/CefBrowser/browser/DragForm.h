#ifndef EXAMPLES_DRAG_FROM_H_
#define EXAMPLES_DRAG_FROM_H_

// duilib
#include "duilib/duilib.h"

/** 标签拖出过程中，显示的小窗口
*/
class DragForm: public ui::DragWindow
{
    typedef ui::DragWindow BaseClass;
public:
    DragForm();
    virtual ~DragForm() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual DString GetSkinFolder() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual DString GetSkinFile() override;
};

#endif //EXAMPLES_DRAG_FROM_H_
