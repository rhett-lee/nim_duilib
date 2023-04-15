#include "GdiHepler.h"
#include "duilib/Core/GdiPlusDefs.h"
#include <algorithm>

namespace ui {
namespace GdiHelper {

    Gdiplus::Bitmap* CreateBitmapFromHBITMAP(HBITMAP hBitmap) 
    {
        BITMAP bmp = { 0 };
        if (0 == GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bmp))
        {
            return nullptr;
        }
        // Although we can get bitmap data address by bmp.bmBits member of BITMAP  
        // which is got by GetObject function sometime, 
        // we can determine the bitmap data in the HBITMAP is arranged bottom-up  
        // or top-down, so we should always use GetDIBits to get bitmap data. 
        BYTE* piexlsSrc = NULL;
        LONG cbSize = bmp.bmWidthBytes * bmp.bmHeight;
        piexlsSrc = new BYTE[cbSize];
        BITMAPINFO bmpInfo = { 0 };
        // We should initialize the first six members of BITMAPINFOHEADER structure. 
        // A bottom-up DIB is specified by setting the height to a positive number,  
        // while a top-down DIB is specified by setting the height to a negative number. 
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
        bmpInfo.bmiHeader.biHeight = bmp.bmHeight;  // 正数，说明数据从下到上，如未负数，则从上到下
        bmpInfo.bmiHeader.biPlanes = bmp.bmPlanes;
        bmpInfo.bmiHeader.biBitCount = bmp.bmBitsPixel;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        HDC hdcScreen = CreateDC(L"DISPLAY", NULL, NULL, NULL);
        LONG cbCopied = GetDIBits(hdcScreen, hBitmap, 0, bmp.bmHeight, piexlsSrc, &bmpInfo, DIB_RGB_COLORS);
        DeleteDC(hdcScreen);
        if (0 == cbCopied)
        {
            delete[] piexlsSrc;
            return nullptr;
        }
        // Create an GDI+ Bitmap has the same dimensions with hbitmap 
        Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(bmp.bmWidth, bmp.bmHeight, PixelFormat32bppPARGB);
        if (pBitmap == nullptr)
        {
            return nullptr;
        }
        // Access to the Gdiplus::Bitmap's pixel data 
        Gdiplus::BitmapData bitmapData = { 0 };
        Gdiplus::Rect rect(0, 0, bmp.bmWidth, bmp.bmHeight);
        if (Gdiplus::Ok != pBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &bitmapData))
        {
            delete pBitmap;
            return nullptr;
        }
        BYTE* pixelsDest = (BYTE*)bitmapData.Scan0;
        int nLinesize = bmp.bmWidth * sizeof(UINT);
        int nHeight = bmp.bmHeight;
        // Copy pixel data from HBITMAP by bottom-up. 
        for (int y = 0; y < nHeight; y++)
        {
            // 从下到上复制数据，因为前面设置高度时是正数。
            memcpy_s(
                (pixelsDest + y * nLinesize),
                nLinesize,
                (piexlsSrc + (nHeight - y - 1) * nLinesize),
                nLinesize);
        }
        // Copy the data in temporary buffer to pBitmap 
        if (Gdiplus::Ok != pBitmap->UnlockBits(&bitmapData))
        {
            delete pBitmap;
            pBitmap = nullptr;
        }
        delete[] piexlsSrc;
        return pBitmap;
    }

    //将图像以中心为圆点，旋转角度dAngle(0 - 360度)
    HBITMAP RotateBitmapAroundCenter(HBITMAP hBitmap, double dAngle)
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
        BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

        // Clean up
        SelectObject(hMemDC, hOldMemBitmap);
        DeleteDC(hMemDC);
        SelectObject(hDC, hOldBitmap);
        DeleteDC(hDC);

        // Return the rotated bitmap
        return hRotatedBitmap;
    }

    bool GetBitmapWidthHeight(HBITMAP hBitmap, int& imageWidth, int& imageHeight)
    {
        BITMAP bm = { 0 };
        if (0 == GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm))
        {
            return false;
        }
        imageWidth = (int)bm.bmWidth;
        imageHeight = (int)bm.bmHeight;
        return true;
    }
}
}
