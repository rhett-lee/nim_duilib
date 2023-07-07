#include "Shadow.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/DpiManager.h"
#include "duilib/Render/IRender.h"

namespace ui 
{

class ShadowBox : public Box
{
public:
	virtual std::wstring GetType() const override { return L"ShadowBox"; }

	virtual void Paint(IRender* pRender, const UiRect& rcPaint) override
	{
		if (pRender == nullptr) {
			return;
		}		
		UiRect rcPos = GetPaddingPos();		
		if (rcPaint.left >= rcPos.left && rcPaint.top >= rcPos.top && rcPaint.right <= rcPos.right && rcPaint.bottom <= rcPos.bottom) {
			//作为阴影，中间部分是空的，不需要处理重绘, 只填充圆角空隙
			FillRoundRect(pRender, rcPos);
			return;
		}
		else {
			__super::Paint(pRender, rcPaint);
			FillRoundRect(pRender, rcPos);
		}
	};

	/** 当Box有圆角的时候，四个角采用填充色绘制背景，避免出现黑色背景
	*/
	void FillRoundRect(IRender* pRender, const UiRect& rcPos)
	{
		Control* pChildBox = GetItemAt(0);
		if (pChildBox == nullptr) {
			return;
		}
		if (!pChildBox->IsVisible() || !pChildBox->ShouldBeRoundRectFill()) {
			//如果不是圆角的，或者不可见的，就需要不填充
			return;
		}
		UiSize borderRound = pChildBox->GetBorderRound();
		const int nRectSize = std::max(borderRound.cx, borderRound.cy);
		if (nRectSize <= 0) {
			return;
		}

		uint8_t uFade = 0xFF;
		Window* pWindow = GetWindow();
		if ((pWindow != nullptr) && (pWindow->IsLayeredWindow())) {
			uFade = pWindow->GetWindowAlpha();
		}

		if (pChildBox->GetAlpha() != 0xFF) {
			uFade = static_cast<uint8_t>((int32_t)uFade * pChildBox->GetAlpha() / 0xFF);
		}
		
		UiRect fillRect;
		//左上角
		fillRect = UiRect(rcPos.left, rcPos.top, rcPos.left + nRectSize, rcPos.top + nRectSize);
		pRender->FillRect(fillRect, m_bkColor, uFade);

		//右上角
		fillRect = UiRect(rcPos.right - nRectSize, rcPos.top, rcPos.right, rcPos.top + nRectSize);
		pRender->FillRect(fillRect, m_bkColor, uFade);

		//左下角
		fillRect = UiRect(rcPos.left, rcPos.bottom - nRectSize, rcPos.left + nRectSize, rcPos.bottom);
		pRender->FillRect(fillRect, m_bkColor, uFade);

		//右下角
		fillRect = UiRect(rcPos.right - nRectSize, rcPos.bottom - nRectSize, rcPos.right, rcPos.bottom);
		pRender->FillRect(fillRect, m_bkColor, uFade);
	}

private:
	/** 背景色
	*/
	UiColor m_bkColor = UiColor(UiColors::LightGray);
};

UiSize Shadow::GetChildBoxBorderRound()
{
	return { 3, 3 };
}

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
		pRoot->SetBorderRound(Shadow::GetChildBoxBorderRound());
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
	}
	else if (!isMaximized && m_pRoot) {
		m_rcShadowCorner = m_rcShadowCornerBackup;
		m_pRoot->GetLayout()->SetPadding(m_rcShadowCorner, false);
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