#ifndef UI_CORE_CURSOR_MANAGER_H_
#define UI_CORE_CURSOR_MANAGER_H_

#include "duilib/duilib_defs.h"
#include <map>

namespace ui {

/** 光标的ID（有效值是非零值）
*/
typedef std::size_t CursorID;

/** 设置光标
*/
class UILIB_API CursorManager
{
public:
    CursorManager();
    ~CursorManager();

    /** 设置光标
    * @param [in] cursorType 光标类型
    */
    bool SetCursor(CursorType cursorType);

    /** 设置图片为光标
    * @param [in] imagePath 光标图片的路径（后缀名为.cur）
    */
    bool SetImageCursor(const DString& imagePath);

    /** 显示或者光标光标
    * @param [in] bShow true表示显示光标，false表示隐藏光标
    */
    bool ShowCursor(bool bShow);

    /** 获取当前光标的光标ID
    */
    CursorID GetCursorID() const;

    /** 根据ID设置光标
    * @param [in] cursorId 光标的ID，该值由GetCursorID()返回
    */
    bool SetCursorByID(CursorID cursorId);

private:

#ifdef DUILIB_PLATFORM_WIN
    /** 已经加载的光标资源句柄
    */
    std::map<DString, HCURSOR> m_cursorMap;
#endif
};

} // namespace ui

#endif // UI_CORE_CURSOR_MANAGER_H_

