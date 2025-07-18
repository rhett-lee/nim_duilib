#ifndef EXAMPLES_DRAGDROP_MANAGER_H_
#define EXAMPLES_DRAGDROP_MANAGER_H_

#include "browser/BrowserBox.h"

class DragForm;

/** 多标签浏览器管理类，用于统一管理多标签拖拽功能
*/
class DragDropManager : public virtual ui::SupportWeakCallback
{
public:
    DragDropManager();
    DragDropManager(const DragDropManager&) = delete;
    DragDropManager& operator=(const DragDropManager&) = delete;

    /** 单例对象
    */
    static DragDropManager* GetInstance();

private:
    ~DragDropManager();

public:
    /** 当前是否正在拖拽浏览器盒子
    */
    bool IsDragingBorwserBox() const;

    /** 开始执行拖拽浏览器盒子的操作
    * @param [in] browserBox 浏览器盒子
    * @param [in] spIBitmap 浏览器盒子生成的位图
    * @param [in] ptOffset 拖拽图像相对于鼠标的偏移
    * @return bool true 成功，false 失败
    */
    bool StartDragBorwserBox(BrowserBox* browserBox, std::shared_ptr<ui::IBitmap> spIBitmap, const ui::UiPoint& ptOffset);

    /** 结束执行拖拽浏览器盒子的操作
    * @param [in] bSuccess true表示按成功操作，false表示取消拖动操作
    */
    void EndDragBorwserBox(bool bSuccess);

    /** 更新拖拽窗口的位置
    */
    void UpdateDragFormPos();

private:
    /** 当下正在被拖拽的浏览器盒子
    */
    BrowserBox* m_pDragingBox;

    /** 拖动过程中的小窗口
    */
    DragForm* m_pDragForm;
};

#endif //EXAMPLES_DRAGDROP_MANAGER_H_
