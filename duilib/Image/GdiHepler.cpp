#include "GdiHepler.h"
#include <algorithm>

namespace ui {
namespace GdiHelper {

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
