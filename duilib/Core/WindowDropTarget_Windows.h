#ifndef UI_CORE_WINDOW_DROP_TARGET_WINDOWS_H_
#define UI_CORE_WINDOW_DROP_TARGET_WINDOWS_H_

#include "duilib/Core/ControlDropTarget.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include <oleidl.h>

namespace ui 
{
class NativeWindow_Windows;

/** 窗口的拖放支持
*/
class WindowDropTarget : public IDropTarget
{
public:
    explicit WindowDropTarget(NativeWindow_Windows* pNativeWindow);
    WindowDropTarget(const WindowDropTarget& r) = delete;
    WindowDropTarget& operator=(const WindowDropTarget& r) = delete;
    virtual ~WindowDropTarget();

public:
    // IUnkown 接口
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(/* [in] */ REFIID riid,
                                                     /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override;

    virtual ULONG STDMETHODCALLTYPE Release(void) override;

    // IDropTarget 接口
    virtual HRESULT STDMETHODCALLTYPE DragEnter(/* [unique][in] */ __RPC__in_opt IDataObject* pDataObj,
                                                /* [in] */ DWORD grfKeyState,
                                                /* [in] */ POINTL pt,
                                                /* [out][in] */ __RPC__inout DWORD* pdwEffect) override;

    virtual HRESULT STDMETHODCALLTYPE DragOver(/* [in] */ DWORD grfKeyState,
                                               /* [in] */ POINTL pt,
                                               /* [out][in] */ __RPC__inout DWORD* pdwEffect) override;

    virtual HRESULT STDMETHODCALLTYPE DragLeave(void) override;

    virtual HRESULT STDMETHODCALLTYPE Drop( /* [unique][in] */ __RPC__in_opt IDataObject* pDataObj,
                                            /* [in] */ DWORD grfKeyState,
                                            /* [in] */ POINTL pt,
                                            /* [out][in] */ __RPC__inout DWORD* pdwEffect) override;

public:
    /** 注册拖放接口
    */
    bool RegisterDragDrop();

    /** 注销拖放接口
    */
    bool UnregisterDragDrop();

protected: 
    /** DragOver 函数的实现
    */
    HRESULT OnDragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

    /** 找到某个点坐标对应的控件接口
    * @param [in] screenPt 屏幕坐标点
    */
    ControlPtrT<ControlDropTarget_Windows> GetControlDropTarget(const UiPoint& screenPt) const;

private:
    /** 引用计数
    */
    int32_t m_nRef;

    /** IDropTarget::DragEnter 传入的数据对象接口
    */
    IDataObject* m_pDataObj;

    /** 当前Hover状态的控件接口
    */
    ControlPtrT<ControlDropTarget_Windows> m_pHoverDropTarget;

    /** 关联的Native窗口实现
    */
    NativeWindow_Windows* m_pNativeWindow;

    /** 是否已经成功注册拖放接口
    */
    bool m_bRegisterDragDrop;
};

} // namespace ui

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#endif // UI_CORE_WINDOW_DROP_TARGET_WINDOWS_H_
