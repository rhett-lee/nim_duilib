#include "WindowDropTarget.h"
#include "duilib/Core/ControlDropTarget.h"
#include "duilib/Core/Window.h"
#include <ole2.h>

namespace ui 
{

/** IDropTarget 的实现
*/
class WindowDropTargetImpl : public IDropTarget {
public:
	explicit WindowDropTargetImpl(WindowDropTarget* pWindowDropTarget):
		m_nRef(0),
		m_pWindowDropTarget(pWindowDropTarget),
		m_pDataObj(nullptr),
		m_pHoverDropTarget(nullptr)
	{
	}

	~WindowDropTargetImpl()
	{
		if (m_pDataObj != nullptr) {
			m_pDataObj->Release();
			m_pDataObj = nullptr;
		}
	}

	// IUnkown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
													 void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;
		if (riid == __uuidof(IUnknown)) {
			*ppvObject = (IUnknown*)this;
			hr = S_OK;
		}
		else if (riid == __uuidof(IDropTarget)) {
			*ppvObject = (IDropTarget*)this;
			hr = S_OK;
		}
		if (SUCCEEDED(hr)) {
			AddRef();
		}
		return hr;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return ++m_nRef;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		ULONG uRet = --m_nRef;
		if (uRet == 0) {
			delete this;
		}
		return uRet;
	}

	// IDropTarget
	virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj,
												DWORD grfKeyState,
												POINTL pt,
												DWORD* pdwEffect)
	{
		m_pDataObj = pDataObj;
		if (m_pDataObj != nullptr) {
			m_pDataObj->AddRef();
		}
		OnDragOver(grfKeyState, pt, pdwEffect);
		//需要返回S_OK, 否则本次拖放就终止了，无法再继续拖放操作
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState,
											   POINTL pt,
											   DWORD* pdwEffect)
	{
		return OnDragOver(grfKeyState, pt, pdwEffect);
	}

	HRESULT OnDragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		HRESULT hr = S_FALSE;
		if (!IsValidControlDropTarget(m_pHoverDropTarget)) {
			m_pHoverDropTarget = nullptr;
		}
		ControlDropTarget* pHoverDropTarget = GetControlDropTarget(UiPoint(pt.x, pt.y));
		if (pHoverDropTarget == nullptr) {
			m_pHoverDropTarget = nullptr;
			return hr;
		}		
		if (pHoverDropTarget == m_pHoverDropTarget) {
			uint32_t dwEffect = DROPEFFECT_NONE;
			if (pdwEffect != nullptr) {
				dwEffect = *pdwEffect;
			}
			hr = pHoverDropTarget->DragOver(grfKeyState, UiPoint(pt.x, pt.y), &dwEffect);
			if (pdwEffect != nullptr) {
				*pdwEffect = dwEffect;
			}
		}
		else {
			if (m_pHoverDropTarget != nullptr) {
				m_pHoverDropTarget->DragLeave();
			}
			m_pHoverDropTarget = pHoverDropTarget;
			uint32_t dwEffect = DROPEFFECT_NONE;
			if (pdwEffect != nullptr) {
				dwEffect = *pdwEffect;
			}
			hr = pHoverDropTarget->DragEnter(m_pDataObj, grfKeyState, UiPoint(pt.x, pt.y), &dwEffect);
			if (pdwEffect != nullptr) {
				*pdwEffect = dwEffect;
			}
		}
		return hr;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		if (m_pDataObj != nullptr) {
			m_pDataObj->Release();
			m_pDataObj = nullptr;
		}
		m_pHoverDropTarget = nullptr;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj,
										   DWORD grfKeyState,
										   POINTL pt,
										   DWORD* pdwEffect)
	{
		if (*pdwEffect == DROPEFFECT_NONE) {
			return S_FALSE;
		}
		HRESULT hr = S_FALSE;
		if (IsValidControlDropTarget(m_pHoverDropTarget)) {
			uint32_t dwEffect = DROPEFFECT_NONE;
			if (pdwEffect != nullptr) {
				dwEffect = *pdwEffect;
			}
			hr = m_pHoverDropTarget->Drop(pDataObj, grfKeyState, UiPoint(pt.x, pt.y), &dwEffect);
			if (pdwEffect != nullptr) {
				*pdwEffect = dwEffect;
			}
		}
		if (m_pDataObj != nullptr) {
			m_pDataObj->Release();
			m_pDataObj = nullptr;
		}
		m_pHoverDropTarget = nullptr;
		return hr;
	}

	/** 找到某个点坐标对应的控件接口
	* @param [in] screenPt 屏幕坐标点
	*/
	ControlDropTarget* GetControlDropTarget(const UiPoint& screenPt) const
	{
		ASSERT(m_pWindowDropTarget != nullptr);
		if (m_pWindowDropTarget == nullptr) {
			return nullptr;
		}
		int32_t itemCount = (int32_t)m_pWindowDropTarget->m_dropTargets.size();
		for (int32_t index = itemCount - 1; index >= 0; --index) {
			ControlDropTarget* pControlDropTarget = m_pWindowDropTarget->m_dropTargets[index];
			if (pControlDropTarget->ContainsPt(screenPt)) {
				return pControlDropTarget;
			}
		}
		return nullptr;
	}

	/** 判断一个控件是否还在容器中
	*/
	bool IsValidControlDropTarget(ControlDropTarget* pControlDropTarget) const
	{
		if (pControlDropTarget == nullptr) {
			return false;
		}
		auto iter = std::find(m_pWindowDropTarget->m_dropTargets.begin(),
							  m_pWindowDropTarget->m_dropTargets.end(),
			                  pControlDropTarget);
		return iter != m_pWindowDropTarget->m_dropTargets.end();
	}

protected:
	/** 引用计数
	*/
	int32_t m_nRef;

	/** WindowDropTarget 接口
	*/
	WindowDropTarget* m_pWindowDropTarget;

	/** IDropTarget::DragEnter 传入的数据对象接口
	*/
	IDataObject* m_pDataObj;

	/** 当前Hover状态的控件接口
	*/
	ControlDropTarget* m_pHoverDropTarget;
};

WindowDropTarget::WindowDropTarget():
	m_pWindow(nullptr),
	m_pDropTargetImpl(nullptr)
{
}

WindowDropTarget::~WindowDropTarget()
{
	if (m_pDropTargetImpl != nullptr) {
		int32_t nRef = m_pDropTargetImpl->Release();
		ASSERT_UNUSED_VARIABLE(nRef == 0);
		m_pDropTargetImpl = nullptr;
	}
}

void WindowDropTarget::SetWindow(Window* pWindow)
{
	m_pWindow = pWindow;
}

bool WindowDropTarget::RegisterDragDrop(ControlDropTarget* pDropTarget)
{
	if (pDropTarget == nullptr) {
		return false;
	}
	auto iter = std::find(m_dropTargets.begin(), m_dropTargets.end(), pDropTarget);
	if (iter != m_dropTargets.end()) {
		m_dropTargets.erase(iter);
	}
	m_dropTargets.push_back(pDropTarget);
	if (m_pDropTargetImpl == nullptr) {
		ASSERT(m_pWindow != nullptr);
		if (m_pWindow != nullptr) {
			ASSERT(::IsWindow(m_pWindow->GetHWND()));
			m_pDropTargetImpl = new WindowDropTargetImpl(this);
			m_pDropTargetImpl->AddRef();
			HRESULT hr = ::RegisterDragDrop(m_pWindow->GetHWND(), m_pDropTargetImpl);
			ASSERT(hr == S_OK);
			if (hr != S_OK) {
				m_pDropTargetImpl->Release();
				m_pDropTargetImpl = nullptr;
			}
		}
	}
	return true;
}

bool WindowDropTarget::UnregisterDragDrop(ControlDropTarget* pDropTarget)
{
	if (pDropTarget == nullptr) {
		return false;
	}
	bool bRemoved = false;
	auto iter = std::find(m_dropTargets.begin(), m_dropTargets.end(), pDropTarget);
	if (iter != m_dropTargets.end()) {
		m_dropTargets.erase(iter);
		bRemoved = true;
	}
	if (m_dropTargets.empty()) {
		if (m_pDropTargetImpl != nullptr) {
			ASSERT(m_pWindow != nullptr);
			if (m_pWindow != nullptr) {
				ASSERT(::IsWindow(m_pWindow->GetHWND()));
				HRESULT hr = ::RevokeDragDrop(m_pWindow->GetHWND());
				ASSERT_UNUSED_VARIABLE(hr == S_OK);
			}
			m_pDropTargetImpl->Release();
			m_pDropTargetImpl = nullptr;
		}
	}
	return bRemoved;
}

void WindowDropTarget::Clear()
{
	if (!m_dropTargets.empty()) {
		if (m_pWindow != nullptr) {
			ASSERT(::IsWindow(m_pWindow->GetHWND()));
			HRESULT hr = ::RevokeDragDrop(m_pWindow->GetHWND());
			ASSERT_UNUSED_VARIABLE(hr == S_OK);
		}
	}
	m_dropTargets.clear();
	m_pWindow = nullptr;
}

} // namespace ui

