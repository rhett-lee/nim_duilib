#ifndef UI_CONTROL_RICHEDIT_DROP_TARGET_WINDOWS_H_
#define UI_CONTROL_RICHEDIT_DROP_TARGET_WINDOWS_H_

#include "duilib/Core/ControlDropTarget.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

namespace ui 
{

class RichEdit2;

/**
 * @brief RichEdit控件的拖放目标类（Windows平台）
 * @details 实现Windows平台下的文本和文件拖放功能，支持：
 *          - 文本拖放：支持单行/多行模式检查、数字模式检查、限制字符检查
 *          - 文件拖放：支持文件类型过滤
 *          - 光标跟随：拖放过程中光标动态跟随鼠标位置
 *          - 自动滚动：当拖放位置接近编辑框边缘时自动滚动视图
 */
class RichEditDropTarget_Windows : public ControlDropTarget_Windows
{
public:
    /** 构造函数
     * @param [in] pRichEdit 关联的RichEdit2控件指针
     */
    explicit RichEditDropTarget_Windows(RichEdit2* pRichEdit);

    /** 析构函数
     * @note 清理拖放状态
     */
    virtual ~RichEditDropTarget_Windows() override;

public:
    /** IDropTarget::DragEnter
     * @param [in] pDataObj 数据对象指针
     * @param [in] grfKeyState 键盘状态
     * @param [in] pt 屏幕坐标
     * @param [out] pdwEffect 拖放效果
     * @return 成功返回S_OK，失败返回S_FALSE
     */
    virtual int32_t DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

    /** IDropTarget::DragOver
     * @param [in] grfKeyState 键盘状态
     * @param [in] pt 屏幕坐标
     * @param [out] pdwEffect 拖放效果
     * @return 成功返回S_OK，失败返回S_FALSE
     * @note 根据拖放位置更新光标位置，并检查是否需要自动滚动
     */
    virtual int32_t DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

    /** IDropTarget::DragLeave
     * @return 成功返回S_OK
     * @note 拖放离开时恢复原来的选择状态
     */
    virtual int32_t DragLeave(void) override;

    /** IDropTarget::Drop
     * @param [in] pDataObj 数据对象指针
     * @param [in] grfKeyState 键盘状态
     * @param [in] pt 屏幕坐标
     * @param [out] pdwEffect 拖放效果
     * @return 成功返回S_OK，失败返回S_FALSE
     */
    virtual int32_t Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

private:
    /** 清理拖放状态
     */
    void ClearDragStatus();

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

    /** 关联的IDataObject对象
    */
    void* m_pDataObj;

    /** 拖入的文本列表
    */
    std::vector<DString> m_dropTextList;

    /** 拖入的文件列表
    */
    std::vector<DString> m_dropFileList;

    /** 拖放开始前的选择起始位置
    */
    int32_t m_nStartChar;

    /** 拖放开始前的选择结束位置
    */
    int32_t m_nEndChar;

    /** 拖放开始前的滚动位置
    */
    UiSize64 m_scrollPos;
};

} // namespace ui

#endif // defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#endif // UI_CONTROL_RICHEDIT_DROP_TARGET_WINDOWS_H_
