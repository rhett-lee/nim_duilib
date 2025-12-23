#ifndef UI_CORE_CONTROL_FINDER_H_
#define UI_CORE_CONTROL_FINDER_H_

#include "duilib/Core/UiPoint.h"
#include "duilib/Core/ControlPtrT.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace ui 
{
// Flags for Control::GetControlFlags()
#define UIFLAG_DEFAULT       0x00000000        // 默认状态
#define UIFLAG_TABSTOP       0x00000001        // 标识控件是否在收到 TAB 切换焦点时允许设置焦点

// Flags for FindControl()
#define UIFIND_ALL           0x00000000
#define UIFIND_VISIBLE       0x00000001
#define UIFIND_ENABLED       0x00000002
#define UIFIND_HITTEST       0x00000004
#define UIFIND_TOP_FIRST     0x00000008
#define UIFIND_DRAG_DROP     0x00000010
#define UIFIND_TOOLTIP       0x00000020
#define UIFIND_ME_FIRST      0x80000000

class Control;
class Box;

typedef struct tagFINDTABINFO
{
    Control* pFocus;
    Control* pLast;
    bool bForward;
    bool bNextIsIt;
} FINDTABINFO;

/** Window辅助查找Control相关功能的封装
*/
class UILIB_API ControlFinder
{
public:
    ControlFinder();
    ~ControlFinder();
    
public:
    /** 设置Box根节点
    */
    void SetRoot(Box* pRoot);

    /** 根据坐标查找指定控件
     * @param[in] pt 指定坐标
     * @return 返回控件指针
     */
    Control* FindControl(const UiPoint& pt) const;

    /** 根据坐标查找支持ToolTip的控件(需要单独的函数支持，因为当控件的mouse_enabled="false" 或者 容器的mouse_child="false"时，FindControl函数查找不到这个控件)
     * @param[in] pt 指定坐标
     * @return 返回控件指针
     */
    Control* FindToolTipControl(const UiPoint& pt) const;

    /** 根据坐标查找可以响应WM_CONTEXTMENU的控件
     * @param[in] pt 指定坐标
     * @return 返回控件指针
     */
    Control* FindContextMenuControl(const UiPoint* pt) const;

    /** 查找可以支持拖放的Box容器
    * @param [in] pt 指定坐标
    * @param [in] nDropInId 拖放的ID值（每个控件可以设置一个ID，来接收拖放, 0表示不可以拖入，非0表示可以拖入）
    */
    Box* FindDroppableBox(const UiPoint& pt, uint8_t nDropInId) const;

    /** 根据控件名称查找控件
     * @param [in] pAncestor 父祖级别的控件
     * @param [in] strName 控件名称
     * @return 返回控件指针
     */
    Control* FindControlInCache(Control* pAncestor, const DString& strName) const;

    /** 根据坐标查找子控件
     * @param [in] pParent 要搜索的控件
     * @param [in] pt 要查找的坐标
     * @return 返回控件指针
     */
    Control* FindSubControlByPoint(Control* pParent, const UiPoint& pt) const;

    /** 根据名字查找子控件
     * @param [in] pParent 要搜索的控件
     * @param [in] strName 要查找的名称
     * @return 返回控件指针
     */
    Control* FindSubControlByName(Control* pParent, const DString& strName) const;

    /** 添加一个控件，对控件名称做索引
    */
    void AddControl(Control* pControl);

    /**@brief 回收控件（通过AddControl函数添加的控件）
     * @param[in] pControl 控件指针
     */
    void RemoveControl(Control* pControl);

    /** 清除状态
    */
    void Clear();

public:
    static Control* FindControlFromPoint(Control* pThis, void* pData);
    static Control* FindControlFromTab(Control* pThis, void* pData);
    static Control* FindControlFromUpdate(Control* pThis, void* pData);
    static Control* FindControlFromName(Control* pThis, void* pData);
    static Control* FindContextMenuControl(Control* pThis, void* pData);
    static Control* FindControlFromDroppableBox(Control* pThis, void* pData);

private:
    /** 根节点
    */
    Box* m_pRoot;

    /** 控件的name与控件之间的映射，用于快速查找控件
    */
    std::unordered_map<DString, std::vector<ControlPtr>> m_controlNameMap;
};

} // namespace ui

#endif // UI_CORE_CONTROL_FINDER_H_
