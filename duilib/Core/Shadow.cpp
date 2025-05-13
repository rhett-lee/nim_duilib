#include "Shadow.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/GlobalManager.h"

namespace ui 
{

class ShadowBox : public Box
{
    typedef Box BaseClass;
public:
    explicit ShadowBox(Window* pWindow):
        Box(pWindow)
    {
        //关闭控件自身的内边距，否则阴影绘制不出来
        SetEnableControlPadding(false);
    }
    virtual DString GetType() const override { return _T("ShadowBox"); }

    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override
    {
        if (pRender == nullptr) {
            return;
        }        
        UiRect rcPos = GetPosWithoutPadding();
        if (rcPaint.left >= rcPos.left && rcPaint.top >= rcPos.top && rcPaint.right <= rcPos.right && rcPaint.bottom <= rcPos.bottom) {
            //作为阴影，中间部分是空的，不需要处理重绘, 只填充圆角空隙
            FillRoundRect(pRender, rcPos);
            return;
        }
        else {
            BaseClass::Paint(pRender, rcPaint);
            FillRoundRect(pRender, rcPos);
        }
    };

    /** 当Box有圆角的时候，四个角采用填充色绘制背景，避免出现黑色背景
    */
    void FillRoundRect(IRender* pRender, const UiRect& rcPos)
    {
        if (pRender == nullptr) {
            return;
        }
        Control* pChildBox = GetItemAt(0);
        if (pChildBox == nullptr) {
            return;
        }
        if (!pChildBox->IsVisible() || !pChildBox->ShouldBeRoundRectFill()) {
            //如果不是圆角的，或者不可见的，就需要不填充
            return;
        }
        UiSize borderRound;
        float fRoundWidth = 0;
        float fRoundHeight = 0;
        if (pChildBox->GetBorderRound(fRoundWidth, fRoundHeight)) {
            borderRound.cx = (int32_t)(fRoundWidth + 0.5f);
            borderRound.cy = (int32_t)(fRoundHeight + 0.5f);
        }
        const int nRectSize = std::max(borderRound.cx, borderRound.cy);
        if (nRectSize <= 0) {
            return;
        }

        uint8_t uFade = 0xFF;
        Window* pWindow = GetWindow();
        if ((pWindow != nullptr) && (pWindow->IsLayeredWindow())) {
            uFade = pWindow->GetLayeredWindowAlpha();
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

UiSize Shadow::GetChildBoxBorderRound(const Box* pBox)
{
    UiSize rcSize{ 3, 3 };
    if (pBox != nullptr) {
        pBox->Dpi().ScaleSize(rcSize);
    }
    return rcSize;
}

Shadow::Shadow(Window* pWindow):
    m_bShadowAttached(true),
    m_bUseDefaultShadowAttached(true),
    m_isMaximized(false),
    m_pRoot(nullptr),
    m_pWindow(pWindow)
{
    ResetDefaultShadow();
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

    m_pRoot = new ShadowBox(pRoot->GetWindow());
    m_pRoot->AddItem(pRoot);
    DoAttachShadow(m_pRoot, pRoot, true, m_bUseDefaultImage, m_isMaximized);
    return m_pRoot;
}

void Shadow::DoAttachShadow(Box* pNewRoot, Box* pOrgRoot, bool bNewAttach, bool bUseDefaultImage, bool isMaximized) const
{
    ASSERT((pNewRoot != nullptr) && (pOrgRoot != nullptr));
    if ((pNewRoot == nullptr) || (pOrgRoot == nullptr)) {
        return;
    }

    pNewRoot->SetPadding((bNewAttach && !isMaximized) ? m_rcShadowCorner : UiPadding(0, 0, 0, 0), false);
    UiPadding rcShadowCorner = !isMaximized ? m_rcShadowCorner : m_rcShadowCornerBackup;
    if (pOrgRoot->GetFixedWidth().IsInt32()) {
        int32_t rootWidth = pOrgRoot->GetFixedWidth().GetInt32();
        if (bNewAttach) {
            rootWidth += (rcShadowCorner.left + rcShadowCorner.right);
        }
        pNewRoot->SetFixedWidth(UiFixedInt(rootWidth), true, false);
    }
    else {
        pNewRoot->SetFixedWidth(pOrgRoot->GetFixedWidth(), true, false);
    }
    if (pOrgRoot->GetFixedHeight().IsInt32()) {
        int32_t rootHeight = pOrgRoot->GetFixedHeight().GetInt32();
        if (bNewAttach) {
            rootHeight += (rcShadowCorner.top + rcShadowCorner.bottom);
        }       
        pNewRoot->SetFixedHeight(UiFixedInt(rootHeight), true, false);
    }
    else {
        pNewRoot->SetFixedHeight(pOrgRoot->GetFixedHeight(), true, false);
    }

    if (bUseDefaultImage) {
        pOrgRoot->SetBorderRound(bNewAttach ? Shadow::GetChildBoxBorderRound(pOrgRoot) : UiSize());
    }
    pNewRoot->SetBkImage(bNewAttach ? m_strImage : DString());
}

void Shadow::SetShadowAttached(bool bShadowAttached)
{
    m_bShadowAttached = bShadowAttached;

    //外部设置后，即更新为非默认值
    m_bUseDefaultShadowAttached = false;

    //如果已经调用了AttachShadow，需要进行些处理
    if (m_pRoot != nullptr) {
        Box* pOrgRoot = nullptr;
        if (m_pRoot->GetItemCount() > 0) {
            pOrgRoot = dynamic_cast<Box*>(m_pRoot->GetItemAt(0));
        }
        DoAttachShadow(m_pRoot, pOrgRoot, bShadowAttached, m_bUseDefaultImage, m_isMaximized);
    }
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

void Shadow::SetShadowImage(const DString &image)
{
    m_strImage = image;
    m_bUseDefaultImage = false;
}

const DString& Shadow::GetShadowImage() const
{
    return m_strImage;
}

void Shadow::SetShadowCorner(const UiPadding& padding, bool bNeedDpiScale)
{
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    if ((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0)) {
        m_rcShadowCorner = padding;
        if (bNeedDpiScale) {
            ASSERT(m_pWindow != nullptr);
            if (m_pWindow != nullptr) {
                m_pWindow->Dpi().ScalePadding(m_rcShadowCorner);
            }
        }
        m_rcShadowCornerBackup = m_rcShadowCorner;
    }    
}

UiPadding Shadow::GetShadowCorner() const
{
    if (m_bShadowAttached) {
        return m_rcShadowCorner;
    }
    else {
        return UiPadding(0, 0, 0, 0);
    }
}

void Shadow::ResetDefaultShadow()
{
    m_bUseDefaultImage = true;
    m_strImage = _T("file='public/shadow/bk_shadow.png' corner='30,30,30,30'");

    m_rcShadowCorner = { 14, 14, 14, 14 };
    ASSERT(m_pWindow != nullptr);
    if (m_pWindow != nullptr) {
        m_pWindow->Dpi().ScalePadding(m_rcShadowCorner);
    }
    m_rcShadowCornerBackup = m_rcShadowCorner;
}

void Shadow::MaximizedOrRestored(bool isMaximized)
{
    m_isMaximized = isMaximized;
    if (!m_bShadowAttached) {
        return;
    }

    if (isMaximized && m_pRoot) {
        m_rcShadowCorner = UiPadding(0, 0, 0, 0);
        m_pRoot->SetPadding(m_rcShadowCorner, false);
    }
    else if (!isMaximized && m_pRoot) {
        m_rcShadowCorner = m_rcShadowCornerBackup;
        m_pRoot->SetPadding(m_rcShadowCorner, false);
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

void Shadow::ChangeDpiScale(const DpiManager& dpi, uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == dpi.GetScale());
    if (nNewDpiScale != dpi.GetScale()) {
        return;
    }
    m_rcShadowCorner = dpi.GetScalePadding(m_rcShadowCorner, nOldDpiScale);
    m_rcShadowCornerBackup = dpi.GetScalePadding(m_rcShadowCornerBackup, nOldDpiScale);
}

}
