#ifndef UI_CORE_CONTROL_DROP_TARGET_IMPL_WINDOWS_H_
#define UI_CORE_CONTROL_DROP_TARGET_IMPL_WINDOWS_H_

#include "duilib/Core/ControlDropTarget.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui 
{
/** 控件的拖放支持接口的实现（Windows）
*/
class ControlDropTargetImpl_Windows : public ControlDropTarget_Windows
{
public:
    explicit ControlDropTargetImpl_Windows(Control* pControl);
    virtual ~ControlDropTargetImpl_Windows();

public:
    /** IDropTarget::DragEnter
    */
    virtual int32_t DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

    /** IDropTarget::DragOver
    */
    virtual int32_t DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

    /** IDropTarget::DragLeave
    */
    virtual int32_t DragLeave(void) override;

    /** IDropTarget::Drop
    */
    virtual int32_t Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override;

public:
    /** 解析pDataObj内容，分离文本和文件路径数据
    */
    static void ParseWindowsDataObject(void* pDataObj, std::vector<DString>& textList, std::vector<DString>& fileList);

private:
    /** 清理拖放状态
    */
    void ClearDragStatus();

private:
    /** 关联的控件
    */
    ControlPtr m_pControl;

    /** 关联的IDataObject对象
    */
    void* m_pDataObj;

    /** 关联的文本数据
    */
    std::vector<DString> m_textList;

    /** 关联的文件数据
    */
    std::vector<DString> m_fileList;
};

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN

#endif // UI_CORE_CONTROL_DROP_TARGET_IMPL_WINDOWS_H_
