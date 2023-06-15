#include "BitmapHelper.h"
#include "duilib/RenderGdiPlus/Bitmap_GDI.h"
#include "duilib/Core/GlobalManager.h"
#include <algorithm>

namespace ui {

namespace BitmapHelper {

    //将图像以中心为圆点，旋转角度dAngle(0 - 360度)
    HBITMAP RotateHBitmapAroundCenter(HBITMAP hBitmap, double dAngle)
    {
        dAngle = dAngle * 3.14159265 / 180.0;
        // Create a memory DC compatible with the display
        HDC hDC = CreateCompatibleDC(NULL);

        // Select the bitmap into the DC
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);

        // Get the bitmap dimensions
        BITMAP bm = { 0 };
        if (0 == GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm))
        {
            return nullptr;
        }

        // Create a new bitmap to hold the rotated image
        HBITMAP hRotatedBitmap = CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);

        // Select the new bitmap into a memory DC
        HDC hMemDC = CreateCompatibleDC(hDC);
        HBITMAP hOldMemBitmap = (HBITMAP)SelectObject(hMemDC, hRotatedBitmap);

        // Set the background color of the new bitmap to white
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        RECT rc = { 0, 0, bm.bmWidth, bm.bmHeight };
        FillRect(hMemDC, &rc, hBrush);
        DeleteObject(hBrush);

        // Calculate the center point of the bitmap
        POINT ptCenter = { bm.bmWidth / 2, bm.bmHeight / 2 };

        // Rotate the bitmap around its center point
        XFORM xform;
        xform.eM11 = (FLOAT)cos(dAngle);
        xform.eM12 = (FLOAT)sin(dAngle);
        xform.eM21 = (FLOAT)-sin(dAngle);
        xform.eM22 = (FLOAT)cos(dAngle);
        xform.eDx = (FLOAT)(ptCenter.x * (1 - cos(dAngle)) + ptCenter.y * sin(dAngle));
        xform.eDy = (FLOAT)(ptCenter.y * (1 - cos(dAngle)) - ptCenter.x * sin(dAngle));
        SetGraphicsMode(hMemDC, GM_ADVANCED);
        SetWorldTransform(hMemDC, &xform);

        // Draw the original bitmap onto the new bitmap with the rotated transform
        ::BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

        // Clean up
        SelectObject(hMemDC, hOldMemBitmap);
        DeleteDC(hMemDC);
        SelectObject(hDC, hOldBitmap);
        DeleteDC(hDC);

        // Return the rotated bitmap
        return hRotatedBitmap;
    }

    IBitmap* RotateBitmapAroundCenter(const IBitmap* pBitmap, double dAngle)
    {
        ASSERT(pBitmap != nullptr);
        if (pBitmap == nullptr) {
            return nullptr;
        }
        const Bitmap_GDI* pGdiBitmap = dynamic_cast<const Bitmap_GDI*>(pBitmap);
        ASSERT(pGdiBitmap != nullptr);
        if (pGdiBitmap == nullptr) {
            return nullptr;
        }
        HBITMAP hBitmap = pGdiBitmap->GetHBitmap();
        ASSERT(hBitmap != nullptr);
        if (hBitmap == nullptr) {
            return nullptr;
        }
        HBITMAP hRotatedBitmap = RotateHBitmapAroundCenter(hBitmap, dAngle);
        ASSERT(hRotatedBitmap != nullptr);
        if (hRotatedBitmap == nullptr) {
            return nullptr;
        }
        return new Bitmap_GDI(hRotatedBitmap, true);
    }

    IRender* CreateRenderObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight)
    {
        ASSERT((srcRenderWidth > 0) && (srcRenderHeight > 0) && (hSrcDc != nullptr) && (srcDcWidth > 0) && (srcDcHeight > 0));
        if ((srcRenderWidth <= 0) || (srcRenderHeight <= 0) || (hSrcDc == nullptr) || (srcDcWidth <= 0) || (srcDcHeight <= 0)) {
            return nullptr;
        }
        std::unique_ptr<IBitmap> bitmap;
        IRender* pRender = nullptr;
        IRenderFactory* pRenderFactory = GlobalManager::GetRenderFactory();
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
 
        BITMAPINFO bmi;
        ::ZeroMemory(&bmi, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = srcDcWidth;
        bmi.bmiHeader.biHeight = -srcDcHeight;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = srcDcWidth * srcDcHeight * 4;

        void* pBits = nullptr;
        HBITMAP hBitmap = ::CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

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
        IRenderFactory* pRenderFactory = GlobalManager::GetRenderFactory();
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
            pRender->DrawImage(rect, pBitmap, rect, rect, UiRect());
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

    HBITMAP GetHBitmap(IBitmap* pBitmap)
    {
        ui::Bitmap_GDI* pGdiBitmap = dynamic_cast<Bitmap_GDI*>(pBitmap);
        ASSERT(pGdiBitmap != nullptr);
        HBITMAP hBitmap = nullptr;
        if (pGdiBitmap != nullptr) {
            hBitmap = pGdiBitmap->GetHBitmap();
        }
        return hBitmap;
    }

    HBITMAP DetachHBitmap(IBitmap* pBitmap)
    {
        ui::Bitmap_GDI* pGdiBitmap = dynamic_cast<Bitmap_GDI*>(pBitmap);
        ASSERT(pGdiBitmap != nullptr);
        HBITMAP hBitmap = nullptr;
        if (pGdiBitmap != nullptr) {
            hBitmap = pGdiBitmap->DetachHBitmap();
        }
        return hBitmap;
    }
    
} //namespace BitmapHelper

} //namespace ui 
