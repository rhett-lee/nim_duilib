#include "IconControl.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

IconControl::IconControl(Window* pWindow):
    Control(pWindow)
{
}

IconControl::~IconControl()
{
    m_pBitmap.reset();
}

DString IconControl::GetType() const { return DUI_CTR_ICON_CONTROL; }

bool IconControl::SetIconData(int32_t nWidth, int32_t nHeight, const uint8_t* pPixelBits, int32_t nPixelBitsSize)
{
    ASSERT((pPixelBits != nullptr) && (nPixelBitsSize > 0) && (nWidth > 0) && (nHeight > 0));
    if ((pPixelBits == nullptr) || (nPixelBitsSize <= 0) || (nWidth <= 0) || (nHeight <= 0)) {
        return false;
    }
    ASSERT(nPixelBitsSize == nHeight * nWidth * (int32_t)sizeof(uint32_t));
    if (nPixelBitsSize != nHeight * nWidth * (int32_t)sizeof(uint32_t)) {
        return false;
    }
    if (m_pBitmap == nullptr) {
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            m_pBitmap.reset(pRenderFactory->CreateBitmap());
        }
    }
    ASSERT(m_pBitmap != nullptr);
    if (m_pBitmap == nullptr) {
        return false;
    }
    bool bRet = false;
    if (((int32_t)m_pBitmap->GetWidth() == nWidth) && ((int32_t)m_pBitmap->GetHeight() == nHeight)) {
        void* pBits = m_pBitmap->LockPixelBits();
        if (pBits != nullptr) {
            //复制图片数据到位图
            ::memcpy(pBits, pPixelBits, nWidth * nHeight * sizeof(uint32_t));
            m_pBitmap->UnLockPixelBits();
            bRet = true;
        }
    }
    else {
        bRet = m_pBitmap->Init(nWidth, nHeight, true, pPixelBits);
    }
    if (bRet) {
        //重绘图片
        Invalidate();
    }
    return bRet;
}

bool IconControl::HasIconData() const
{
    return (m_pBitmap != nullptr) && (m_pBitmap->GetWidth() > 0) && (m_pBitmap->GetHeight() > 0);
}

void IconControl::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if (HasIconData()) {
        UiRect rcDest = GetRect();
        rcDest.Deflate(GetControlPadding());

        UiRect rcSource;
        rcSource.left = 0;
        rcSource.top = 0;
        rcSource.right = rcSource.left + m_pBitmap->GetWidth();
        rcSource.bottom = rcSource.top + m_pBitmap->GetHeight();

        int32_t imageRatio = (int32_t)(100.0 * rcSource.Width() / rcSource.Height());
        int32_t rectRatio = (int32_t)(100.0 * rcDest.Width() / rcDest.Height());
        if (imageRatio != rectRatio) {
            //图片与区域的宽高比不同时，如果区域不匹配，则居中显示
            if (rcSource.Width() < rcDest.Width()) {
                rcDest.left = rcDest.left + (rcDest.Width() - rcSource.Width()) / 2;
                rcDest.right = rcDest.left + rcSource.Width();
            }
            if (rcSource.Height() < rcDest.Height()) {
                rcDest.top = rcDest.top + (rcDest.Height() - rcSource.Height()) / 2;
                rcDest.bottom = rcDest.bottom + rcSource.Height();
            }
        }
        pRender->DrawImage(rcPaint, m_pBitmap.get(), rcDest, rcSource);
    }    
}

}//namespace ui
