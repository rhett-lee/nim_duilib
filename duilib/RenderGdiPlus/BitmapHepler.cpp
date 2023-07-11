#include "BitmapHelper.h"
#include "duilib/Core/GlobalManager.h"
#include <algorithm>

namespace ui {

namespace BitmapHelper {

    IRender* CreateRenderObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight)
    {
        ASSERT((srcRenderWidth > 0) && (srcRenderHeight > 0) && (hSrcDc != nullptr) && (srcDcWidth > 0) && (srcDcHeight > 0));
        if ((srcRenderWidth <= 0) || (srcRenderHeight <= 0) || (hSrcDc == nullptr) || (srcDcWidth <= 0) || (srcDcHeight <= 0)) {
            return nullptr;
        }
        std::unique_ptr<IBitmap> bitmap;
        IRender* pRender = nullptr;
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            pRender = pRenderFactory->CreateRender();
            bitmap.reset(pRenderFactory->CreateBitmap());
        }
        ASSERT(pRender != nullptr);
        if (pRender == nullptr) {
            return nullptr;
        }
        ASSERT(bitmap != nullptr);
        if (bitmap == nullptr) {
            delete pRender;
            return nullptr;
        }

        HDC hMemDC = ::CreateCompatibleDC(hSrcDc);
        void* pBits = nullptr;
        HBITMAP hBitmap = CreateGDIBitmap(srcDcWidth, srcDcHeight, true, &pBits);
        ::SelectObject(hMemDC, hBitmap);

        //复制位图到内存DC
        ::BitBlt(hMemDC, 0, 0, srcDcWidth, srcDcHeight, hSrcDc, 0, 0, SRCCOPY);

        bitmap->Init(srcDcWidth, srcDcHeight, true, pBits);

        // 释放资源
        ::DeleteDC(hMemDC);
        ::DeleteObject(hBitmap);

        if (pRender->Resize(srcRenderWidth, srcRenderHeight)) {
            int dest_width = 0;
            int dest_height = 0;
            float scale = (float)srcDcWidth / (float)srcDcHeight;
            if (scale >= 1.0)
            {
                dest_width = srcRenderWidth;
                dest_height = (int)(srcRenderWidth * (float)srcDcHeight / (float)srcDcWidth);
            }
            else
            {
                dest_height = srcRenderHeight;
                dest_width = (int)(srcRenderHeight * (float)srcDcWidth / (float)srcDcHeight);
            }

            pRender->DrawImage(UiRect(0, 0, srcDcWidth, srcDcHeight),
                              bitmap.get(),
                              UiRect((srcRenderWidth - dest_width) / 2, 0, dest_width, dest_height),
                              UiRect(0, 0, 0, 0),
                              UiRect(0, 0, srcDcWidth, srcDcHeight),
                              UiRect(0, 0, 0, 0));
        }
        return pRender;
    }

    IRender* CreateRenderObject(IBitmap* pBitmap)
    {
        ASSERT(pBitmap != nullptr);
        if (pBitmap == nullptr) {
            return nullptr;
        }
        IRender* pRender = nullptr;
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            pRender = pRenderFactory->CreateRender();
        }
        ASSERT(pRender != nullptr);
        if (pRender == nullptr) {
            return nullptr;
        }
        if (pRender->Resize((int)pBitmap->GetWidth(), (int)pBitmap->GetHeight())) {
            UiRect rect = { 0, 0, (int)pBitmap->GetWidth(), (int)pBitmap->GetHeight() };
            pRender->DrawImage(rect, pBitmap, rect, UiRect(), rect, UiRect());
        }        
        return pRender;
    }

    IBitmap* CreateBitmapObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight)
    {
        IBitmap* pBitmap = nullptr;
        IRender* pRender = CreateRenderObject(srcRenderWidth, srcRenderHeight, hSrcDc, srcDcWidth, srcDcHeight);
        if (pRender != nullptr) {
            pBitmap = pRender->DetachBitmap();
            delete pRender;
            pRender = nullptr;
        }
        return pBitmap;
    }

    HBITMAP CreateGDIBitmap(IBitmap* pBitmap)
    {
        if (pBitmap == nullptr) {
            return nullptr;
        }
        void* pLockBits = pBitmap->LockPixelBits();
        if (pLockBits == nullptr) {
            return nullptr;
        }
        LPVOID pBitmapBits = nullptr;
        HBITMAP hBitmap = CreateGDIBitmap(pBitmap->GetWidth(), pBitmap->GetHeight(), true, &pBitmapBits);
        if ((hBitmap != nullptr) && (pBitmapBits != nullptr)){
            memcpy(pBitmapBits, pLockBits, pBitmap->GetWidth() * pBitmap->GetHeight() * 4);
        }        
        pBitmap->UnLockPixelBits();
        return hBitmap;
    }

    HBITMAP CreateGDIBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits)
    {
        ASSERT((nWidth > 0) && (nHeight > 0));
        if (nWidth == 0 || nHeight == 0) {
            return nullptr;
        }

        BITMAPINFO bmi = { 0 };
        ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = nWidth;
        if (flipHeight) {
            bmi.bmiHeader.biHeight = -nHeight;//负数表示位图方向：从上到下，左上角为圆点
        }
        else {
            bmi.bmiHeader.biHeight = nHeight; //正数表示位图方向：从下到上，左下角为圆点
        }
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = nWidth * nHeight * sizeof(DWORD);

        HDC hdc = ::GetDC(NULL);
        HBITMAP hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, pBits, NULL, 0);
        ::ReleaseDC(NULL, hdc);
        return hBitmap;
    }
    
} //namespace BitmapHelper

} //namespace ui 
