#ifndef UI_CORE_CONTROL_DROP_TARGET_H_
#define UI_CORE_CONTROL_DROP_TARGET_H_

#pragma once

#include "duilib/Core/UiTypes.h"

namespace ui 
{
class Control;

/** 控件的拖放支持
*/
class ControlDropTarget
{
public:
	ControlDropTarget();
	ControlDropTarget(const ControlDropTarget& r) = delete;
	ControlDropTarget& operator=(const ControlDropTarget& r) = delete;
	virtual ~ControlDropTarget();

public:
	/** 设置关联的控件接口
	 * @param [in] pControl 控件接口
	 */
	void SetControl(Control* pControl);

	/** 获取控件接口
	*/
	Control* GetControl() const;

	/** 判断点坐标是否包含在该控件中
	* @param [in] screenPt 屏幕坐标点
	*/
	bool ContainsPt(const UiPoint& screenPt) const;

public:

    //IDropTarget::DragEnter
    virtual int32_t DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect);

    //IDropTarget::DragOver
    virtual int32_t DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect);

    //IDropTarget::DragLeave
    virtual int32_t DragLeave(void);

    //IDropTarget::Drop
    virtual int32_t Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect);

private:
	/** 关联的控件接口
	*/
	Control* m_pControl;
};

} // namespace ui

#endif // UI_CORE_CONTROL_DROP_TARGET_H_
