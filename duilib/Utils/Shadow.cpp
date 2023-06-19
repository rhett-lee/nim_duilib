#include "Shadow.h"
#include "duilib/Core/Box.h"
#include "duilib/Utils/DpiManager.h"

namespace ui 
{

class ShadowBox : public Box
{
public:
	virtual std::wstring GetType() const override { return L"ShadowBox"; }

	virtual void Paint(IRender* pRender, const UiRect& rcPaint) override
	{
		// 作为阴影，中间部分是空的，不需要处理重绘
		auto rcPos = GetPaddingPos();
		if (rcPaint.left >= rcPos.left && rcPaint.top >= rcPos.top && rcPaint.right <= rcPos.right && rcPaint.bottom <= rcPos.bottom) {
			return;
		}
		__super::Paint(pRender, rcPaint);
	};
};

Shadow::Shadow():
	m_bShadowAttached(true),
	m_bUseDefaultShadowAttached(true),
	m_pRoot(nullptr)
{
	ResetDefaultShadow();
}

void Shadow::SetShadowAttached(bool bShadowAttached)
{
	m_bShadowAttached = bShadowAttached; 
	//外部设置后，即更新为非默认值
	m_bUseDefaultShadowAttached = false;
}

bool Shadow::IsShadowAttached() const
{ 
	return m_bShadowAttached;
}

bool Shadow::IsUseDefaultShadowAttached() const
{
	return m_bUseDefaultShadowAttached;
}

void Shadow::SetUseDefaultShadowAttached(bool isDefault)
{
	m_bUseDefaultShadowAttached = isDefault;
}

void Shadow::SetShadowImage(const std::wstring &image)
{
	m_strImage = image;
	m_bUseDefaultImage = false;
}

const std::wstring& Shadow::GetShadowImage() const
{
	return m_strImage;
}

void Shadow::SetShadowCorner(const UiRect &rect, bool bNeedDpiScale)
{
	m_rcShadowCorner = rect;
	if (bNeedDpiScale)
		DpiManager::GetInstance()->ScaleRect(m_rcShadowCorner);
	m_rcShadowCornerBackup = m_rcShadowCorner;
}

UiRect Shadow::GetShadowCorner() const
{
	if (m_bShadowAttached) {
		return m_rcShadowCorner;
	}
	else {
		return UiRect(0, 0, 0, 0);
	}
}

void Shadow::ResetDefaultShadow()
{
	m_bUseDefaultImage = true;
	m_strImage = L"file='public/shadow/bk_shadow.png' corner='30,30,30,30'";

	m_rcShadowCorner = { 14, 14, 14, 14 };
	DpiManager::GetInstance()->ScaleRect(m_rcShadowCorner);
	m_rcShadowCornerBackup = m_rcShadowCorner;
}

Box* Shadow::AttachShadow(Box* pRoot)
{
	if (!m_bShadowAttached) {
		return pRoot;
	}
	ASSERT(m_pRoot == nullptr);
	if (m_pRoot != nullptr) {
		return pRoot;
	}

	if (pRoot == nullptr) {
		return nullptr;
	}

	m_pRoot = new ShadowBox();
	m_pRoot->GetLayout()->SetPadding(m_rcShadowCorner, false);

	int rootWidth = pRoot->GetFixedWidth();
	if (rootWidth > 0) {
		rootWidth += m_rcShadowCorner.left + m_rcShadowCorner.right;
	}
	m_pRoot->SetFixedWidth(rootWidth, true, false);

	int rootHeight = pRoot->GetFixedHeight();
	if (rootHeight > 0) {
		rootHeight += m_rcShadowCorner.top + m_rcShadowCorner.bottom;
	}
	m_pRoot->SetFixedHeight(rootHeight, false);

	if (m_bUseDefaultImage)	{
		UiSize size(3, 3);
		pRoot->SetBorderRound(size);
	}

	m_pRoot->AddItem(pRoot);
	m_pRoot->SetBkImage(m_strImage);

	return m_pRoot;
}

void Shadow::MaximizedOrRestored(bool isMaximized)
{
	if (!m_bShadowAttached) {
		return;
	}

	if (isMaximized && m_pRoot) {
		m_rcShadowCorner = UiRect(0, 0, 0, 0);
		m_pRoot->GetLayout()->SetPadding(m_rcShadowCorner, false);

		if (m_bUseDefaultImage)
		{
			Control* control = m_pRoot->GetItemAt(0);
			if (control != nullptr) {
				UiSize size(0, 0);
				control->SetBorderRound(size);
			}			
		}
	}
	else if (!isMaximized && m_pRoot) {
		m_rcShadowCorner = m_rcShadowCornerBackup;
		m_pRoot->GetLayout()->SetPadding(m_rcShadowCorner, false);

		if (m_bUseDefaultImage)
		{
			Control* control = m_pRoot->GetItemAt(0);
			if (control != nullptr) {
				UiSize size(3, 3);
				control->SetBorderRound(size);
			}			
		}
	}
}

ui::Control* Shadow::GetRoot()
{
	return m_pRoot;
}

void Shadow::ClearImageCache()
{
	if (m_pRoot) {
		m_pRoot->ClearImageCache();
	}	
}

}