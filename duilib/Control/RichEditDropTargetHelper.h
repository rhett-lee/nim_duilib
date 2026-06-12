#ifndef UI_CONTROL_RICHEDIT_DROP_TARGET_HELPER_H_
#define UI_CONTROL_RICHEDIT_DROP_TARGET_HELPER_H_

#include "duilib/Core/ControlDropTarget.h"

namespace ui 
{

class RichEdit2;

/** RichEdit控件的拖放目标类的辅助函数
 */
class RichEditDropTargetHelper
{
public:
    /** 构造函数
     * @param [in] pRichEdit 关联的RichEdit2控件指针
     */
    explicit RichEditDropTargetHelper(RichEdit2* pRichEdit, const std::vector<DString>& dropTextList);

public:
    /** 检查文本是否满足拖入条件
     * @param [in] clientPt 客户区坐标
     * @return 文本有效返回true，否则返回false
     * @note 检查单行/多行模式、数字模式、限制字符等
     */
    bool CheckDropText(const UiPoint& clientPt) const;

    /** 检查是否需要滚动视图，按需滚动
     * @param [in] clientPt 客户区坐标
     */
    void CheckTextScroll(const UiPoint& clientPt);

private:
    /** 关联的RichEdit2控件指针
    */
    RichEdit2* m_pRichEdit;

    /** 拖入的文本列表
    */
    const std::vector<DString>& m_dropTextList;
};

} // namespace ui

#endif // UI_CONTROL_RICHEDIT_DROP_TARGET_HELPER_H_
