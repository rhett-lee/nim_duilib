#ifndef UI_CONTROL_RICHEDIT_DROP_TARGET_SDL_H_
#define UI_CONTROL_RICHEDIT_DROP_TARGET_SDL_H_

#include "duilib/Core/ControlDropTarget.h"

#ifdef DUILIB_BUILD_FOR_SDL

namespace ui 
{

class RichEdit2;

/**
 * @brief RichEdit控件的拖放目标类（SDL平台）
 * @details 实现SDL平台下的文本和文件拖放功能，支持：
 *          - 文本拖放：支持单行/多行模式检查、数字模式检查、限制字符检查
 *          - 文件拖放：支持文件类型过滤
 *          - 光标跟随：拖放过程中光标动态跟随鼠标位置
 *          - 自动滚动：当拖放位置接近编辑框边缘时自动滚动视图
 */
class RichEditDropTarget_SDL : public ControlDropTarget_SDL
{
public:
    /** 构造函数
     * @param [in] pRichEdit 关联的RichEdit2控件指针
     */
    explicit RichEditDropTarget_SDL(RichEdit2* pRichEdit);

public:
    /** SDL事件：拖放开始
     * @param [in] pt 客户区坐标（此参数在SDL版本中未使用）
     * @return 成功返回S_OK，失败返回S_FALSE
     */
    virtual int32_t OnDropBegin(const UiPoint& pt) override;

    /** SDL事件：拖放位置改变
     * @param [in] pt 客户区坐标
     * @note 根据拖放位置更新光标位置，并检查是否需要自动滚动
     */
    virtual void OnDropPosition(const UiPoint& pt) override;

    /** SDL事件：拖放文本（OnDropTexts和OnDropFiles只会触发一个）
     * @param [in] textList 文本内容列表，每个元素代表一行文本
     * @param [in] pt 客户区坐标
     */
    virtual void OnDropTexts(const std::vector<DString>& textList, const UiPoint& pt) override;

    /** SDL事件：拖放文件（OnDropTexts和OnDropFiles只会触发一个）
     * @param [in] source 拖放源路径
     * @param [in] fileList 文件路径列表
     * @param [in] pt 客户区坐标
     */
    virtual void OnDropFiles(const DString& source, const std::vector<DString>& fileList, const UiPoint& pt) override;

    /** SDL事件：拖放离开
     * @note 拖放被取消或结束时调用，恢复原来的选择状态
     */
    virtual void OnDropLeave() override;

private:
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

#endif // DUILIB_BUILD_FOR_SDL

#endif // UI_CONTROL_RICHEDIT_DROP_TARGET_SDL_H_
