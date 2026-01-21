#include "WindowRgn_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "SkiaHeaderBegin.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"

#include "SkiaHeaderEnd.h"

namespace ui
{
// 从Skia的圆角路径生成HRGN，保证与drawRoundRect完全一致
static HRGN CreateHRGNFromSkiaRoundRect(const SkRect& skRect, SkScalar rx, SkScalar ry)
{
    // 创建Skia圆角路径（确保路径闭合）
    SkPath path;
    path.addRoundRect(skRect, rx, ry);
    path.close(); // 显式闭合路径，避免端点缺失

    // 使用SkPathMeasure离散化路径为密集点集
    SkPathMeasure pathMeasure(path, true); // true表示闭合路径
    std::vector<POINT> points;

    // 路径总长度
    SkScalar pathLength = pathMeasure.getLength();
    if (pathLength <= 0) {
        return nullptr;
    }

    // 离散化步长（越小越精细，建议1.0f，兼顾精度和性能）
    const SkScalar step = 1.0f;
    SkScalar distance = 0.0f;

    // 遍历路径，按步长提取点
    while (distance <= pathLength) {
        SkPoint pos;
        SkVector tan;
        // 获取指定距离处的点坐标和切线
        if (pathMeasure.getPosTan(distance, &pos, &tan)) {
            // 转换为Windows POINT（浮点转整数，四舍五入更精准）
            POINT pt = {
                static_cast<LONG>(std::round(pos.x())),
                static_cast<LONG>(std::round(pos.y()))
            };
            // 避免重复添加相同点
            if (points.empty() || !(points.back().x == pt.x && points.back().y == pt.y)) {
                points.push_back(pt);
            }
        }
        distance += step;
    }

    // 移除最后一个重复的起点（路径闭合时会重复）
    if ((points.size() >= 2) &&
        (points.front().x == points.back().x) &&
        (points.front().y == points.back().y)) {
        points.pop_back();
    }

    // 验证点集有效性（至少3个点才能形成多边形）
    if (points.size() < 3) {
        return nullptr;
    }

    // 创建多边形HRGN（ALTERNATE填充模式适配圆角）
    HRGN hRgn = ::CreatePolygonRgn(points.data(), static_cast<int>(points.size()), ALTERNATE);
    return hRgn;
}

bool WindowRgn::SetWindowRoundRectRgn(HWND hWnd, const UiRect& rcWnd, float rx, float ry, bool bRedraw)
{
    ASSERT(::IsWindow(hWnd));
    if (!::IsWindow(hWnd)) {
        return false;
    }
    SkRect skRect;
    skRect.fLeft = 0;
    skRect.fTop = 0;
    skRect.fRight = skRect.fLeft + rcWnd.Width();
    skRect.fBottom = skRect.fTop + rcWnd.Height();

    HRGN hRgn = CreateHRGNFromSkiaRoundRect(skRect, rx, ry);
    int nRet = ::SetWindowRgn(hWnd, hRgn, bRedraw ? TRUE : FALSE);
    return nRet != 0;
}

bool WindowRgn::SetWindowRectRgn(HWND hWnd, const UiRect& rcWnd, bool bRedraw)
{
    ASSERT(::IsWindow(hWnd));
    if (!::IsWindow(hWnd)) {
        return false;
    }
    HRGN hRgn = ::CreateRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);
    int nRet = ::SetWindowRgn(hWnd, hRgn, bRedraw ? TRUE : FALSE);
    return nRet != 0;
}

void WindowRgn::ClearWindowRgn(HWND hWnd, bool bRedraw)
{
    ASSERT(::IsWindow(hWnd));
    if (!::IsWindow(hWnd)) {
        return;
    }
    ::SetWindowRgn(hWnd, nullptr, bRedraw ? TRUE : FALSE);
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
