// BitmapAlpha 的单元测试：ClearAlpha 与 RestoreAlpha（两个重载）
//
// 覆盖范围（纯内存逻辑，不创建窗口、不依赖渲染后端）：
//   BA-01 ~ BA-08  ClearAlpha：整图清除、局部清除、越界裁剪、空矩形、单像素、边界值
//   BA-09 ~ BA-20  RestoreAlpha(alpha)：alpha==0 规则、alpha!=0 规则、阴影边距、
//                  越界裁剪、RGB 保持不变
//   BA-21 ~ BA-27  RestoreAlpha(无 alpha)：无条件置 255、阴影边距、越界裁剪
//   BA-28 ~ BA-30  ClearAlpha + RestoreAlpha 配合使用后的复原效果

#include "duilib/Render/BitmapAlpha.h"
#include "duilib/Core/UiRect.h"
#include "duilib/Core/UiPadding.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const char* message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message << std::endl;
    }
}
#define CHECK(expr) Check((expr), #expr)

/** 输出当前测试小节的标题，便于定位失败用例 */
void Section(const char* name)
{
    std::cerr << "--- " << name << std::endl;
}

/** 构造有规律的测试像素：Alpha/RGB 各不相同，便于检测误改 */
uint32_t PatternPixel(int32_t x, int32_t y)
{
    const uint32_t nAlpha = (uint32_t)((x * 37 + y * 91 + 17) & 0xFF);
    const uint32_t nRed = (uint32_t)((x * 13 + y * 7 + 3) & 0xFF);
    const uint32_t nGreen = (uint32_t)((x * 5 + y * 29 + 11) & 0xFF);
    const uint32_t nBlue = (uint32_t)((x * 17 + y * 3 + 23) & 0xFF);
    return (nAlpha << 24) | (nRed << 16) | (nGreen << 8) | nBlue;
}

/** 测试用的位图数据（32bpp，大小为 width * height * 4 字节） */
struct TestBitmap
{
    int32_t m_nWidth = 0;
    int32_t m_nHeight = 0;
    std::vector<uint8_t> m_bits;        //当前像素数据
    std::vector<uint8_t> m_snapshot;    //快照，用于判断区域外的数据是否被误改

    TestBitmap(int32_t nWidth, int32_t nHeight) :
        m_nWidth(nWidth),
        m_nHeight(nHeight),
        m_bits((size_t)nWidth * (size_t)nHeight * 4)
    {
        FillPattern();
    }

    uint8_t* Data() { return m_bits.data(); }

    const uint32_t* Pixels() const { return (const uint32_t*)m_bits.data(); }

    uint32_t* Pixels() { return (uint32_t*)m_bits.data(); }

    uint32_t Pixel(int32_t x, int32_t y) const
    {
        return Pixels()[(size_t)y * (size_t)m_nWidth + (size_t)x];
    }

    void SetPixel(int32_t x, int32_t y, uint32_t nValue)
    {
        Pixels()[(size_t)y * (size_t)m_nWidth + (size_t)x] = nValue;
    }

    uint8_t Alpha(int32_t x, int32_t y) const
    {
        return (uint8_t)(Pixel(x, y) >> 24);
    }

    uint32_t Rgb(int32_t x, int32_t y) const
    {
        return Pixel(x, y) & 0x00FFFFFFu;
    }

    /** 填充规律像素 */
    void FillPattern()
    {
        for (int32_t y = 0; y < m_nHeight; ++y) {
            for (int32_t x = 0; x < m_nWidth; ++x) {
                SetPixel(x, y, PatternPixel(x, y));
            }
        }
    }

    /** 把所有像素的 Alpha 通道设置为指定值（RGB 保持不变） */
    void SetAllAlpha(uint8_t nAlpha)
    {
        for (int32_t y = 0; y < m_nHeight; ++y) {
            for (int32_t x = 0; x < m_nWidth; ++x) {
                SetPixel(x, y, (Pixel(x, y) & 0x00FFFFFFu) | ((uint32_t)nAlpha << 24));
            }
        }
    }

    /** 保存快照 */
    void Snapshot() { m_snapshot = m_bits; }

    /** 与快照逐字节比较，返回与快照相同的数据 */
    bool SameAsSnapshot() const { return m_bits == m_snapshot; }

    /** 与快照逐字节比较第 (x, y) 个像素 */
    bool PixelSameAsSnapshot(int32_t x, int32_t y) const
    {
        const size_t nOffset = ((size_t)y * (size_t)m_nWidth + (size_t)x) * 4;
        return std::memcmp(m_bits.data() + nOffset, m_snapshot.data() + nOffset, 4) == 0;
    }

    /** 判断区域内所有字节是否都等于指定值 */
    bool RegionAllBytesEqual(const ui::UiRect& rc, uint8_t nValue) const
    {
        for (int32_t y = rc.top; y < rc.bottom; ++y) {
            for (int32_t x = rc.left; x < rc.right; ++x) {
                const size_t nOffset = ((size_t)y * (size_t)m_nWidth + (size_t)x) * 4;
                for (size_t k = 0; k < 4; ++k) {
                    if (m_bits[nOffset + k] != nValue) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    /** 判断区域外的所有像素是否与快照一致 */
    bool OutsideSameAsSnapshot(const ui::UiRect& rc) const
    {
        for (int32_t y = 0; y < m_nHeight; ++y) {
            for (int32_t x = 0; x < m_nWidth; ++x) {
                const bool bInRect = (x >= rc.left) && (x < rc.right) && (y >= rc.top) && (y < rc.bottom);
                if (!bInRect && !PixelSameAsSnapshot(x, y)) {
                    return false;
                }
            }
        }
        return true;
    }
};

// ---------------------------------------------------------------------------
// ClearAlpha（BA-01 ~ BA-08）
// ---------------------------------------------------------------------------
void TestClearAlpha()
{
    Section("TestClearAlpha");
    const int32_t nWidth = 7;   //刻意使用非 4 对齐的宽度
    const int32_t nHeight = 5;

    //BA-01 整图清除（alpha = 0）：所有字节都被覆盖为 0（RGB 与 Alpha 都清零）
    {
        TestBitmap bitmap(nWidth, nHeight);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.ClearAlpha(ui::UiRect(0, 0, nWidth, nHeight), 0);
        CHECK(bitmap.RegionAllBytesEqual(ui::UiRect(0, 0, nWidth, nHeight), 0));
    }

    //BA-02 整图清除（alpha = 255）：所有字节都被覆盖为 255
    {
        TestBitmap bitmap(nWidth, nHeight);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.ClearAlpha(ui::UiRect(0, 0, nWidth, nHeight), 255);
        CHECK(bitmap.RegionAllBytesEqual(ui::UiRect(0, 0, nWidth, nHeight), 255));
    }

    //BA-03 整图清除（alpha = 77）：所有字节都被覆盖为 77
    {
        TestBitmap bitmap(nWidth, nHeight);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.ClearAlpha(ui::UiRect(0, 0, nWidth, nHeight), 77);
        CHECK(bitmap.RegionAllBytesEqual(ui::UiRect(0, 0, nWidth, nHeight), 77));
    }

    //BA-04 局部清除：只有矩形内被清除，矩形外保持原值
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(2, 1, 5, 4);
        alpha.ClearAlpha(rc, 0);
        CHECK(bitmap.RegionAllBytesEqual(rc, 0));
        CHECK(bitmap.OutsideSameAsSnapshot(rc));

        //局部清除时 alpha 也可以是非 0 值
        TestBitmap bitmap2(nWidth, nHeight);
        ui::BitmapAlpha alpha2(bitmap2.Data(), nWidth, nHeight, 4);
        alpha2.ClearAlpha(rc, 128);
        CHECK(bitmap2.RegionAllBytesEqual(rc, 128));
    }

    //BA-05 单像素清除
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(3, 2, 4, 3);
        alpha.ClearAlpha(rc, 0);
        CHECK(bitmap.Pixel(3, 2) == 0);
        CHECK(bitmap.OutsideSameAsSnapshot(rc));
    }

    //BA-06 指定的矩形超出位图边界：裁剪到整个位图后全部清除
    {
        TestBitmap bitmap(nWidth, nHeight);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.ClearAlpha(ui::UiRect(-3, -2, nWidth + 5, nHeight + 4), 0);
        CHECK(bitmap.RegionAllBytesEqual(ui::UiRect(0, 0, nWidth, nHeight), 0));
    }

    //BA-07 部分越界的矩形：只清除与位图相交的部分
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rcClipped(0, 0, 3, 2);
        alpha.ClearAlpha(ui::UiRect(-4, -4, 3, 2), 0);
        CHECK(bitmap.RegionAllBytesEqual(rcClipped, 0));
        CHECK(bitmap.OutsideSameAsSnapshot(rcClipped));
    }

    //BA-08 空矩形/反向矩形/完全在位图外：不做任何修改
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.ClearAlpha(ui::UiRect(2, 2, 2, 4), 0);       //宽度为 0
        alpha.ClearAlpha(ui::UiRect(2, 4, 5, 2), 0);       //高度为负（反向矩形）
        alpha.ClearAlpha(ui::UiRect(20, 20, 30, 30), 0);   //完全在位图外
        alpha.ClearAlpha(ui::UiRect(-30, -30, -20, -20), 0);
        CHECK(bitmap.SameAsSnapshot());
    }
}

// ---------------------------------------------------------------------------
// RestoreAlpha(alpha)（BA-09 ~ BA-20）
// ---------------------------------------------------------------------------
void TestRestoreAlphaValue()
{
    Section("TestRestoreAlphaValue");
    const int32_t nWidth = 7;
    const int32_t nHeight = 5;

    //BA-09 alpha == 0：Alpha 为 0 的像素恢复为 255，其余像素保持原值；RGB 全部保持不变
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetPixel(1, 1, 0x00A1B2C3);  //Alpha 为 0，需要恢复
        bitmap.SetPixel(4, 3, 0x00000000);  //Alpha 为 0，RGB 也为 0
        bitmap.SetPixel(2, 2, 0x80A1B2C3);  //Alpha 为 128，保持原值
        bitmap.Snapshot();

        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(0, 0, nWidth, nHeight), ui::UiPadding(), 0);

        CHECK(bitmap.Pixel(1, 1) == 0xFFA1B2C3);
        CHECK(bitmap.Pixel(4, 3) == 0xFF000000);
        CHECK(bitmap.Pixel(2, 2) == 0x80A1B2C3);
        //所有像素的 RGB 通道保持不变
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                const uint32_t nNow = bitmap.Pixel(x, y);
                const uint32_t nOld = ((const uint32_t*)bitmap.m_snapshot.data())[(size_t)y * nWidth + x];
                CHECK((nNow & 0x00FFFFFFu) == (nOld & 0x00FFFFFFu));
            }
        }
    }

    //BA-10 alpha == 0 且只处理局部区域：区域外保持原值
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(0);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(1, 1, 4, 3);
        alpha.RestoreAlpha(rc, ui::UiPadding(), 0);
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                const bool bInRect = (x >= rc.left) && (x < rc.right) && (y >= rc.top) && (y < rc.bottom);
                CHECK(bitmap.Alpha(x, y) == (bInRect ? 255 : 0));
            }
        }
    }

    //BA-11 alpha != 0：等于 alpha 的像素恢复为 0，等于 0 的像素恢复为 255，其余保持原值
    {
        const uint8_t nAlphaValue = 200;
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetPixel(0, 0, 0xC8A1B2C3);  //等于 alpha，需要恢复为 0
        bitmap.SetPixel(5, 4, 0x00FFEEDD);  //等于 0，需要恢复为 255
        bitmap.SetPixel(3, 2, 0x40AABBCC);  //其他值，保持原值
        bitmap.Snapshot();

        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(0, 0, nWidth, nHeight), ui::UiPadding(), nAlphaValue);

        CHECK(bitmap.Pixel(0, 0) == 0x00A1B2C3);
        CHECK(bitmap.Pixel(5, 4) == 0xFFFFEEDD);
        CHECK(bitmap.Pixel(3, 2) == 0x40AABBCC);
    }

    //BA-12 alpha != 0 且只处理局部区域：区域外保持原值
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(200);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(2, 0, 6, 3);
        alpha.RestoreAlpha(rc, ui::UiPadding(), 200);
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                const bool bInRect = (x >= rc.left) && (x < rc.right) && (y >= rc.top) && (y < rc.bottom);
                CHECK(bitmap.Alpha(x, y) == (bInRect ? 0 : 200));
            }
        }
    }

    //BA-13 阴影边距：在裁剪后的矩形内再扣除边距，边距区域不做恢复
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(0);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(0, 0, nWidth, nHeight);
        //左1、上2、右1、下1 的边距：有效区域为 x∈[1,6), y∈[2,4)
        alpha.RestoreAlpha(rc, ui::UiPadding(1, 2, 1, 1), 0);
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                const bool bInPadding = (x >= 1) && (x < nWidth - 1) && (y >= 2) && (y < nHeight - 1);
                CHECK(bitmap.Alpha(x, y) == (bInPadding ? 255 : 0));
            }
        }
    }

    //BA-14 阴影边距大于矩形：区域为空，不做任何修改
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(0);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(2, 1, 5, 4), ui::UiPadding(3, 3, 3, 3), 0);
        CHECK(bitmap.SameAsSnapshot());
    }

    //BA-15 矩形与阴影边距共同裁剪后为空：不做任何修改
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(0);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(0, 0, 2, 2), ui::UiPadding(5, 5, 5, 5), 0);
        CHECK(bitmap.SameAsSnapshot());
    }

    //BA-16 矩形超出位图边界：裁剪后仅处理相交部分
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(0);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rcIntersect(0, 0, 4, 3);
        alpha.RestoreAlpha(ui::UiRect(-10, -10, 4, 3), ui::UiPadding(), 0);
        CHECK(bitmap.OutsideSameAsSnapshot(rcIntersect));
        CHECK(bitmap.Alpha(0, 0) == 255);
        CHECK(bitmap.Alpha(3, 2) == 255);
    }

    //BA-17 空矩形/反向矩形：不做任何修改
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(0);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(2, 2, 2, 4), ui::UiPadding(), 0);
        alpha.RestoreAlpha(ui::UiRect(4, 4, 2, 2), ui::UiPadding(), 0);
        CHECK(bitmap.SameAsSnapshot());
    }

    //BA-18 alpha == 255：Alpha 为 255 的像素被置为 0
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(255);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(0, 0, nWidth, nHeight), ui::UiPadding(), 255);
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                CHECK(bitmap.Alpha(x, y) == 0);
            }
        }
    }

    //BA-19 横向 1 像素的矩形（行跨度与有效宽度不同）
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(0);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(3, 0, 4, nHeight), ui::UiPadding(), 0);
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                CHECK(bitmap.Alpha(x, y) == (x == 3 ? 255 : 0));
            }
        }
    }

    //BA-20 纵向 1 像素的矩形
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(0);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(0, 2, nWidth, 3), ui::UiPadding(), 0);
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                CHECK(bitmap.Alpha(x, y) == (y == 2 ? 255 : 0));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// RestoreAlpha(无 alpha)（BA-21 ~ BA-27）
// ---------------------------------------------------------------------------
void TestRestoreAlphaOpaque()
{
    Section("TestRestoreAlphaOpaque");
    const int32_t nWidth = 7;
    const int32_t nHeight = 5;

    //BA-21 整图把 Alpha 置为 255，RGB 保持不变
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(0, 0, nWidth, nHeight), ui::UiPadding());
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                CHECK(bitmap.Alpha(x, y) == 255);
                CHECK(bitmap.Rgb(x, y) == (((const uint32_t*)bitmap.m_snapshot.data())[(size_t)y * nWidth + x] & 0x00FFFFFFu));
            }
        }
    }

    //BA-22 局部区域：区域外的像素保持原值
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(2, 1, 6, 4);
        alpha.RestoreAlpha(rc, ui::UiPadding());
        CHECK(bitmap.OutsideSameAsSnapshot(rc));
        CHECK(bitmap.Alpha(2, 1) == 255);
        CHECK(bitmap.Alpha(5, 3) == 255);
    }

    //BA-23 阴影边距：边距区域不处理
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(0, 0, nWidth, nHeight), ui::UiPadding(2, 1, 1, 2));
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                const bool bInRect = (x >= 2) && (x < nWidth - 1) && (y >= 1) && (y < nHeight - 2);
                if (bInRect) {
                    CHECK(bitmap.Alpha(x, y) == 255);
                }
                else {
                    CHECK(bitmap.PixelSameAsSnapshot(x, y));
                }
            }
        }
    }

    //BA-24 矩形越界：只处理相交部分
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(4, 3, nWidth + 10, nHeight + 10), ui::UiPadding());
        CHECK(bitmap.Alpha(4, 3) == 255);
        CHECK(bitmap.Alpha(nWidth - 1, nHeight - 1) == 255);
        CHECK(bitmap.PixelSameAsSnapshot(3, 3));
        CHECK(bitmap.PixelSameAsSnapshot(4, 2));
    }

    //BA-25 空矩形：不做任何修改
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(3, 3, 3, 4), ui::UiPadding());
        alpha.RestoreAlpha(ui::UiRect(5, 5, 3, 3), ui::UiPadding());
        CHECK(bitmap.SameAsSnapshot());
    }

    //BA-26 矩形完全在位图外：不做任何修改
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(nWidth + 5, 0, nWidth + 10, nHeight), ui::UiPadding());
        alpha.RestoreAlpha(ui::UiRect(-10, -10, -5, -5), ui::UiPadding());
        CHECK(bitmap.SameAsSnapshot());
    }

    //BA-27 Alpha 已经为 255 的像素再次处理，结果不变（幂等）
    {
        TestBitmap bitmap(nWidth, nHeight);
        bitmap.SetAllAlpha(255);
        bitmap.Snapshot();
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        alpha.RestoreAlpha(ui::UiRect(0, 0, nWidth, nHeight), ui::UiPadding());
        CHECK(bitmap.SameAsSnapshot());
    }
}

// ---------------------------------------------------------------------------
// ClearAlpha + RestoreAlpha 配合（BA-28 ~ BA-30）
// ---------------------------------------------------------------------------
void TestClearAndRestorePair()
{
    Section("TestClearAndRestorePair");
    const int32_t nWidth = 7;
    const int32_t nHeight = 5;

    //BA-28 alpha == 0：清除后整个区域变为透明，恢复后变为完全不透明，RGB 全部为 0
    {
        TestBitmap bitmap(nWidth, nHeight);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(1, 1, 6, 4);
        alpha.ClearAlpha(rc, 0);
        CHECK(bitmap.RegionAllBytesEqual(rc, 0));
        alpha.RestoreAlpha(rc, ui::UiPadding(), 0);
        for (int32_t y = rc.top; y < rc.bottom; ++y) {
            for (int32_t x = rc.left; x < rc.right; ++x) {
                CHECK(bitmap.Pixel(x, y) == 0xFF000000);
            }
        }
    }

    //BA-29 alpha != 0：清除后 Alpha 等于清除值，恢复后 Alpha 被置为 0（RGB 保留清除值）
    {
        TestBitmap bitmap(nWidth, nHeight);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(0, 0, nWidth, nHeight);
        alpha.ClearAlpha(rc, 128);
        CHECK(bitmap.RegionAllBytesEqual(rc, 128));
        alpha.RestoreAlpha(rc, ui::UiPadding(), 128);
        for (int32_t y = 0; y < nHeight; ++y) {
            for (int32_t x = 0; x < nWidth; ++x) {
                CHECK(bitmap.Alpha(x, y) == 0);
                CHECK(bitmap.Rgb(x, y) == 0x00808080u);
            }
        }
    }

    //BA-30 清除后由上层修改 Alpha 的像素，恢复时必须保留其新值（不能被统一置为 255）
    {
        TestBitmap bitmap(nWidth, nHeight);
        ui::BitmapAlpha alpha(bitmap.Data(), nWidth, nHeight, 4);
        const ui::UiRect rc(0, 0, nWidth, nHeight);
        alpha.ClearAlpha(rc, 0);
        bitmap.SetPixel(2, 2, 0x40010203);  //模拟渲染层写入的半透明像素
        alpha.RestoreAlpha(rc, ui::UiPadding(), 0);
        CHECK(bitmap.Pixel(2, 2) == 0x40010203);
        CHECK(bitmap.Pixel(3, 3) == 0xFF000000);
    }
}

} // namespace

int main(int /*argc*/, char* /*argv*/[])
{
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    TestClearAlpha();
    TestRestoreAlphaValue();
    TestRestoreAlphaOpaque();
    TestClearAndRestorePair();

    std::cout << "bitmap_alpha: " << g_checkCount << " checks, "
              << g_failedCount << " failed" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "bitmap_alpha: FAILED" << std::endl;
        return 1;
    }
    std::cout << "bitmap_alpha: PASSED" << std::endl;
    return 0;
}
