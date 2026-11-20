// Image 目录纯逻辑接口的单元测试
//
// 覆盖内容（不创建窗口、不依赖图片资源）：
//  1. ImageUtil：缩放比例判断、尺寸/偏移换算、像素行翻转、像素数据缩放、
//     最佳加载比例计算（含“图片单边大于目标区域”时不应放大的回归用例）；
//  2. ImageLoadParam：固定宽高/百分比解析、DPI 缩放、缓存 KEY 拼装；
//  3. ImageAttribute：图片属性字符串解析、dest/source/corner/margin/平铺参数、
//     以及 ModifyAttribute 不覆盖 DPI 开关的行为；
//  4. ImageAttribute::CalculateAdaptiveRect / HasValidImageRect 静态计算；
//  5. StateImage：状态图片的设置、查询、保底图片、删除与按名称查找；
//  6. ImageDecoderUtil：ICO 目录信息解析与图标项选择规则。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Core/DpiManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/ImageAttribute.h"
#include "duilib/Image/ImageDecoderUtil.h"
#include "duilib/Image/ImageLoadParam.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Image/StateImage.h"
#include "duilib/Utils/FilePath.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const char* expression)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << std::endl;
    }
}

#define CHECK(condition) Check((condition), #condition)

void CheckInt(int64_t actual, int64_t expected, const char* expression)
{
    ++g_checkCount;
    if (actual != expected) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << ": actual=" << actual
                  << ", expected=" << expected << std::endl;
    }
}

#define CHECK_INT(actual, expected) CheckInt((int64_t)(actual), (int64_t)(expected), #actual " == " #expected)

void CheckFloat(float actual, float expected, float tolerance, const char* expression)
{
    ++g_checkCount;
    if (std::fabs(actual - expected) > tolerance) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << ": actual=" << actual
                  << ", expected=" << expected << std::endl;
    }
}

#define CHECK_FLOAT(actual, expected, tolerance) \
    CheckFloat((float)(actual), (float)(expected), (float)(tolerance), #actual " == " #expected)

/** 输出当前测试小节的标题，便于定位失败用例 */
void Section(const char* name)
{
    std::cerr << "--- " << name << std::endl;
}

template<typename T1, typename T2>
bool StringEquals(const T1& actual, const T2& expected)
{
    return actual == expected;
}

#define CHECK_STR(actual, expected) Check(StringEquals((actual), (expected)), #actual " == " #expected)

/** ImageUtil 的缩放比例与尺寸/偏移换算 */
void TestImageUtilScalar()
{
    Section("ImageUtil scalar");

    // 缩放比例判断
    CHECK(!ui::ImageUtil::NeedResizeImage(0.0f));
    CHECK(!ui::ImageUtil::NeedResizeImage(0.0005f));
    CHECK(!ui::ImageUtil::NeedResizeImage(1.0f));
    CHECK(!ui::ImageUtil::NeedResizeImage(1.0005f));
    CHECK(ui::ImageUtil::NeedResizeImage(1.5f));
    CHECK(ui::ImageUtil::NeedResizeImage(0.5f));

    CHECK(!ui::ImageUtil::IsValidImageScale(0.0f));
    CHECK(!ui::ImageUtil::IsValidImageScale(0.0005f));
    CHECK(ui::ImageUtil::IsValidImageScale(0.01f));

    CHECK(ui::ImageUtil::IsSameImageScale(1.0f, 1.0005f));
    CHECK(!ui::ImageUtil::IsSameImageScale(1.0f, 1.01f));

    // 按比例缩放尺寸（向上取整）
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(100, 1.0f), 100);
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(100, 1.5f), 150);
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(101, 1.5f), 152);
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(0, 2.0f), 0);

    // 按比例缩放偏移（四舍五入，负数对称）
    CHECK_INT(ui::ImageUtil::GetScaledImageOffset(10, 1.0f), 10);
    CHECK_INT(ui::ImageUtil::GetScaledImageOffset(10, 1.5f), 15);
    CHECK_INT(ui::ImageUtil::GetScaledImageOffset(11, 1.5f), 17);
    CHECK_INT(ui::ImageUtil::GetScaledImageOffset(-11, 1.5f), -17);

    // 按 DPI 百分比缩放尺寸
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(100, 100, 100), 100);
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(100, 150, 100), 150);
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(101, 150, 100), 152);
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(100, 150, 200), 75);
    CHECK_INT(ui::ImageUtil::GetScaledImageSize(100, 150, 0), 100);
}

/** 像素数据行翻转 */
void TestFlipPixelBits()
{
    Section("ImageUtil::FlipPixelBits");

    // 2 列 3 行：翻转后第 0 行与第 2 行交换，中间行不变
    uint32_t pixels[6] = {
        0x01020304, 0x11121314,
        0x21222324, 0x31323334,
        0x41424344, 0x51525354
    };
    CHECK(ui::ImageUtil::FlipPixelBits(reinterpret_cast<uint8_t*>(pixels), sizeof(pixels), 2, 3));
    CHECK_INT(pixels[0], 0x41424344);
    CHECK_INT(pixels[1], 0x51525354);
    CHECK_INT(pixels[2], 0x21222324);
    CHECK_INT(pixels[3], 0x31323334);
    CHECK_INT(pixels[4], 0x01020304);
    CHECK_INT(pixels[5], 0x11121314);

    // 单行：无需交换
    uint32_t singleRow[2] = { 0xAABBCCDD, 0xEEFF0011 };
    CHECK(ui::ImageUtil::FlipPixelBits(reinterpret_cast<uint8_t*>(singleRow), sizeof(singleRow), 2, 1));
    CHECK_INT(singleRow[0], 0xAABBCCDD);
    CHECK_INT(singleRow[1], 0xEEFF0011);

    // 1 列 2 行：上下交换
    uint32_t column[2] = { 0x01010101, 0x02020202 };
    CHECK(ui::ImageUtil::FlipPixelBits(reinterpret_cast<uint8_t*>(column), sizeof(column), 1, 2));
    CHECK_INT(column[0], 0x02020202);
    CHECK_INT(column[1], 0x01010101);
}

/** 像素数据缩放 */
void TestResizeImageData()
{
    Section("ImageUtil::ResizeImageData");

    // 2x2 纯色图片放大到 4x4：纯色不应被改变
    const uint32_t color = 0xFF332211;
    std::vector<uint32_t> src(4, color);

    uint32_t nNewWidth = 0;
    uint32_t nNewHeight = 0;
    std::vector<uint8_t> out;
    CHECK(ui::ImageUtil::ResizeImageData(reinterpret_cast<const uint8_t*>(src.data()), src.size() * sizeof(uint32_t),
                                         2, 2, 2.0f, out, nNewWidth, nNewHeight));
    CHECK_INT(nNewWidth, 4);
    CHECK_INT(nNewHeight, 4);
    CHECK_INT(out.size(), (size_t)4 * 4 * 4);
    bool bAllSame = true;
    for (size_t i = 0; i + 4 <= out.size(); i += 4) {
        uint32_t value = 0;
        memcpy(&value, out.data() + i, sizeof(value));
        if (value != color) {
            bAllSame = false;
            break;
        }
    }
    CHECK(bAllSame);

    // 缩放比例为 1.0 时数据原样拷贝
    std::vector<uint8_t> copied;
    nNewWidth = 0;
    nNewHeight = 0;
    CHECK(ui::ImageUtil::ResizeImageData(reinterpret_cast<const uint8_t*>(src.data()), src.size() * sizeof(uint32_t),
                                         2, 2, 1.0f, copied, nNewWidth, nNewHeight));
    CHECK_INT(nNewWidth, 2);
    CHECK_INT(nNewHeight, 2);
    CHECK_INT(copied.size(), src.size() * sizeof(uint32_t));
    CHECK(memcmp(copied.data(), src.data(), copied.size()) == 0);

    // 4x4 缩小到 2x2（输出尺寸由 GetScaledImageSize 向上取整决定）
    std::vector<uint32_t> large(16, color);
    std::vector<uint8_t> small;
    nNewWidth = 0;
    nNewHeight = 0;
    CHECK(ui::ImageUtil::ResizeImageData(reinterpret_cast<const uint8_t*>(large.data()), large.size() * sizeof(uint32_t),
                                         4, 4, 0.5f, small, nNewWidth, nNewHeight));
    CHECK_INT(nNewWidth, 2);
    CHECK_INT(nNewHeight, 2);
    CHECK_INT(small.size(), (size_t)2 * 2 * 4);

    // 输出缓冲区重载：尺寸不变时同样执行拷贝
    uint32_t outPixels[4] = { 0, 0, 0, 0 };
    CHECK(ui::ImageUtil::ResizeImageData(reinterpret_cast<const uint8_t*>(src.data()), src.size() * sizeof(uint32_t),
                                         2, 2, reinterpret_cast<uint8_t*>(outPixels), sizeof(outPixels), 2, 2));
    CHECK_INT(outPixels[0], color);
    CHECK_INT(outPixels[3], color);
}

/** 最佳加载缩放比例（包含单边超出目标区域的回归用例） */
void TestGetBestImageScale()
{
    Section("ImageUtil::GetBestImageScale");

    float fScale = 0.0f;

    // 图片比目标区域小：取较大的缩放比例
    CHECK(ui::ImageUtil::GetBestImageScale(ui::UiSize(200, 400), 100, 100, 10.0f, fScale));
    CHECK_FLOAT(fScale, 4.0f, 1e-4f);

    // 图片宽度大于目标区域、高度小于目标区域：应缩小而不是放大
    // （原实现使用 || 判断，会得到 4.0 的放大的错误结果）
    fScale = 0.0f;
    CHECK(ui::ImageUtil::GetBestImageScale(ui::UiSize(200, 400), 400, 100, 10.0f, fScale));
    CHECK_FLOAT(fScale, 0.5f, 1e-4f);

    // 图片两个方向都大于目标区域：取较小的缩放比例
    fScale = 0.0f;
    CHECK(ui::ImageUtil::GetBestImageScale(ui::UiSize(200, 200), 400, 400, 10.0f, fScale));
    CHECK_FLOAT(fScale, 0.5f, 1e-4f);

    // 单边等于目标区域，另一边小于目标区域：仍按“可放大”处理
    fScale = 0.0f;
    CHECK(ui::ImageUtil::GetBestImageScale(ui::UiSize(200, 400), 200, 100, 10.0f, fScale));
    CHECK_FLOAT(fScale, 4.0f, 1e-4f);

    // 最大缩放比例限制
    fScale = 0.0f;
    CHECK(ui::ImageUtil::GetBestImageScale(ui::UiSize(400, 400), 100, 100, 1.5f, fScale));
    CHECK_FLOAT(fScale, 1.5f, 1e-4f);

    // 最小缩放比例限制（1/16）
    fScale = 0.0f;
    CHECK(ui::ImageUtil::GetBestImageScale(ui::UiSize(1, 1), 1000, 1000, 10.0f, fScale));
    CHECK_FLOAT(fScale, 1.0f / 16.0f, 1e-4f);

    // 仅设置宽度/高度
    fScale = 0.0f;
    CHECK(ui::ImageUtil::GetBestImageScale(ui::UiSize(200, 0), 100, 50, 10.0f, fScale));
    CHECK_FLOAT(fScale, 2.0f, 1e-4f);
    fScale = 0.0f;
    CHECK(ui::ImageUtil::GetBestImageScale(ui::UiSize(0, 80), 100, 50, 10.0f, fScale));
    CHECK_FLOAT(fScale, 1.6f, 1e-4f);

    // 目标区域和图片尺寸无效时返回 false，且不修改输出参数
    fScale = 7.0f;
    CHECK(!ui::ImageUtil::GetBestImageScale(ui::UiSize(0, 0), 100, 100, 10.0f, fScale));
    CHECK_FLOAT(fScale, 7.0f, 1e-6f);
    CHECK(!ui::ImageUtil::GetBestImageScale(ui::UiSize(100, 100), 0, 100, 10.0f, fScale));
    CHECK(!ui::ImageUtil::GetBestImageScale(ui::UiSize(100, 100), 100, -1, 10.0f, fScale));
}

/** ImageLoadParam 的宽高解析、DPI 缩放与缓存 KEY */
void TestImageLoadParam()
{
    Section("ImageLoadParam");

    // 默认构造：没有固定宽高
    ui::ImageLoadParam defaultParam;
    uint32_t nWidth = 99;
    uint32_t nHeight = 99;
    CHECK(!defaultParam.HasImageFixedSize());
    CHECK(!defaultParam.GetImageFixedSize(nWidth, nHeight));
    CHECK_INT(nWidth, 0);
    CHECK_INT(nHeight, 0);
    float fWidthPercent = 0.0f;
    float fHeightPercent = 0.0f;
    CHECK(!defaultParam.HasImageFixedPercent());
    CHECK(!defaultParam.GetImageFixedPercent(fWidthPercent, fHeightPercent));
    CHECK_FLOAT(fWidthPercent, 1.0f, 1e-6f);
    CHECK_FLOAT(fHeightPercent, 1.0f, 1e-6f);
    CHECK_INT(defaultParam.GetLoadDpiScale(), 100);

    // 固定像素与百分比混合
    ui::ImageLoadParam mixedParam(_T("300"), _T("30%"), true, 100, false, false, 1000, 0, 30.0f, true, _T(""));
    nWidth = 0;
    nHeight = 0;
    CHECK(mixedParam.HasImageFixedSize());
    CHECK(mixedParam.GetImageFixedSize(nWidth, nHeight));
    CHECK_INT(nWidth, 300);
    CHECK_INT(nHeight, 0);
    CHECK(mixedParam.GetImageFixedPercent(fWidthPercent, fHeightPercent));
    CHECK_FLOAT(fWidthPercent, 1.0f, 1e-6f);
    CHECK_FLOAT(fHeightPercent, 0.3f, 1e-5f);

    // 固定像素按 DPI 缩放；关闭 DPI 自适应后不缩放
    ui::ImageLoadParam dpiParam(_T("100"), _T(""), true, 150, false, false, 1000, 0, 30.0f, true, _T(""));
    nWidth = 0;
    nHeight = 0;
    CHECK(dpiParam.GetImageFixedSize(nWidth, nHeight));
    CHECK_INT(nWidth, 150);
    ui::ImageLoadParam noDpiParam(_T("100"), _T(""), false, 150, false, false, 1000, 0, 30.0f, true, _T(""));
    nWidth = 0;
    nHeight = 0;
    CHECK(noDpiParam.GetImageFixedSize(nWidth, nHeight));
    CHECK_INT(nWidth, 100);

    // 加载 KEY
    ui::ImageLoadPath loadPath;
    loadPath.m_imageFullPath = ui::FilePath(_T("a.png"));
    ui::ImageLoadParam keyParam;
    keyParam.SetImageLoadPath(loadPath);
    CHECK_STR(keyParam.GetLoadKey(100), _T("5:a.png#1"));
    CHECK_STR(keyParam.GetLoadKey(150), _T("5:a.png@150#1"));
    CHECK(keyParam.IsSvgImageFile() == false);

    ui::ImageLoadParam fullParam(_T("30"), _T("20%"), true, 100, false, false, 1000, 0, 30.0f, true,
                                 _T("red|blue"));
    fullParam.SetImageLoadPath(loadPath);
    CHECK_STR(fullParam.GetLoadKey(100), _T("5:a.png#1$2:303:20%&8:red|blue"));

    // GetLoadDpiScale 对 0 值做兜底
    ui::ImageLoadParam zeroDpiParam;
    zeroDpiParam.SetLoadDpiScale(0);
    CHECK_INT(zeroDpiParam.GetLoadDpiScale(), 100);

    // SVG 后缀识别
    ui::ImageLoadPath svgPath;
    svgPath.m_imageFullPath = ui::FilePath(_T("image.SVG"));
    ui::ImageLoadParam svgParam;
    svgParam.SetImageLoadPath(svgPath);
    CHECK(svgParam.IsSvgImageFile());
}

/** ImageAttribute 的属性字符串解析 */
void TestImageAttribute()
{
    Section("ImageAttribute");

    const ui::DpiManager& dpi = ui::GlobalManager::Instance().Dpi();

    ui::ImageAttribute attribute;
    const DString imageString = _T("file='res/a.png' name='img1' width='30' height='40' ")
                                    _T("dest='1,2,3,4' source='5,6,7,8' corner='9,10,11,12' ")
                                    _T("halign='center' valign='bottom' fade='300' play_count='5' ")
                                    _T("icon_size='64' icon_as_animation='true' icon_frame_delay='-5' ")
                                    _T("auto_play='false' adaptive_dest_rect='true' window_shadow_mode='true' ")
                                    _T("xtiled='true' ytiled='true' tiled_margin='3' tiled_padding='1,2,3,4' ")
                                    _T("margin='2,4,6,8' svg_replace_colors='red|blue' ")
                                    _T("assert='false' async_load='true' dpi_scale='false' dest_scale='false' ")
                                    _T("pag_max_frame_rate='12.5'");
    attribute.InitByImageString(imageString, dpi);

    CHECK_STR(attribute.m_sImageString, imageString);
    CHECK_STR(attribute.m_sImagePath, _T("res/a.png"));
    CHECK_STR(attribute.m_sImageName, _T("img1"));
    CHECK_STR(attribute.m_srcWidth, _T("30"));
    CHECK_STR(attribute.m_srcHeight, _T("40"));

    ui::UiRect rcSource = attribute.GetImageSourceRect();
    CHECK_INT(rcSource.left, 5);
    CHECK_INT(rcSource.top, 6);
    CHECK_INT(rcSource.right, 7);
    CHECK_INT(rcSource.bottom, 8);

    ui::UiRect rcCorner = attribute.GetImageCorner();
    CHECK(attribute.HasImageCorner());
    CHECK_INT(rcCorner.left, 9);
    CHECK_INT(rcCorner.top, 10);
    CHECK_INT(rcCorner.right, 11);
    CHECK_INT(rcCorner.bottom, 12);

    CHECK_STR(attribute.m_hAlign, _T("center"));
    CHECK_STR(attribute.m_vAlign, _T("bottom"));
    CHECK_INT(attribute.m_nAlpha, 255); // 300 被截断到 255
    CHECK_INT(attribute.m_nPlayCount, 5);
    CHECK_INT(attribute.m_nIconSize, 64);
    CHECK(attribute.m_bIconAsAnimation);
    CHECK_INT(attribute.m_nIconFrameDelayMs, 1000); // 非法值回落为默认值
    CHECK(!attribute.m_bAutoPlay);
    CHECK(attribute.m_bAdaptiveDestRect);
    CHECK(attribute.m_bWindowShadowMode);
    CHECK(attribute.IsTiledDraw());

    ui::TiledDrawParam tiledParam = attribute.GetTiledDrawParam(dpi);
    CHECK(tiledParam.m_bTiledX);
    CHECK(tiledParam.m_bTiledY);
    CHECK_INT(tiledParam.m_nTiledMarginX, 3);
    CHECK_INT(tiledParam.m_nTiledMarginY, 3);
    CHECK_INT(tiledParam.m_rcTiledPadding.left, 1);
    CHECK_INT(tiledParam.m_rcTiledPadding.top, 2);
    CHECK_INT(tiledParam.m_rcTiledPadding.right, 3);
    CHECK_INT(tiledParam.m_rcTiledPadding.bottom, 4);

    ui::UiMargin margin = attribute.GetImageMargin(dpi);
    CHECK_INT(margin.left, 2);
    CHECK_INT(margin.top, 4);
    CHECK_INT(margin.right, 6);
    CHECK_INT(margin.bottom, 8);

    CHECK_STR(attribute.m_svgReplaceColors, _T("red|blue"));
    CHECK(!attribute.IsAssertEnabled());
    CHECK(attribute.m_bAsyncLoad);
    CHECK(!attribute.m_bImageDpiScaleEnabled);
    CHECK(!attribute.m_bDestDpiScaleEnabled);
    CHECK_FLOAT(attribute.m_fPagMaxFrameRate, 12.5f, 1e-6f);

    CHECK(attribute.HasDestRect());
    ui::UiRect rcDest = attribute.GetImageDestRect(100, 100, dpi);
    CHECK_INT(rcDest.left, 1);
    CHECK_INT(rcDest.top, 2);
    CHECK_INT(rcDest.right, 3);
    CHECK_INT(rcDest.bottom, 4);

    // ModifyAttribute：仅更新出现的属性，且不重置 DPI 开关
    attribute.ModifyAttribute(_T("width='50'"), dpi);
    CHECK_STR(attribute.m_srcWidth, _T("50"));
    CHECK_STR(attribute.m_srcHeight, _T("40"));
    CHECK(!attribute.m_bImageDpiScaleEnabled);
    CHECK(!attribute.m_bDestDpiScaleEnabled);
    CHECK_STR(attribute.m_sImagePath, _T("res/a.png"));

    // 未指定完整 dest 时，按图片尺寸补全 right/bottom
    ui::ImageAttribute partialDest;
    partialDest.InitByImageString(_T("file='res/b.png' dest='10,20'"), dpi);
    CHECK(partialDest.HasDestRect());
    ui::UiRect rcPartial = partialDest.GetImageDestRect(30, 40, dpi);
    CHECK_INT(rcPartial.left, 10);
    CHECK_INT(rcPartial.top, 20);
    CHECK_INT(rcPartial.right, 40);
    CHECK_INT(rcPartial.bottom, 60);

    // 不含属性的字符串：路径保持原值，且没有可选属性
    ui::ImageAttribute plainPath;
    plainPath.InitByImageString(_T("res/c.png"), dpi);
    CHECK_STR(plainPath.GetImageName(), _T(""));
    CHECK_STR(plainPath.m_sImagePath, _T("res/c.png"));
    CHECK(!plainPath.HasDestRect());
    CHECK(!plainPath.HasImageCorner());
    CHECK(!plainPath.IsTiledDraw());
    CHECK_INT(plainPath.m_nAlpha, 255);

    // HasValidImageRect
    CHECK(!ui::ImageAttribute::HasValidImageRect(ui::UiRect()));
    CHECK(!ui::ImageAttribute::HasValidImageRect(ui::UiRect(10, 10, 5, 20)));
    CHECK(ui::ImageAttribute::HasValidImageRect(ui::UiRect(0, 0, 1, 1)));
    CHECK(ui::ImageAttribute::HasValidImageRect(ui::UiRect(10, 20, 30, 60)));
}

/** ImageAttribute::CalculateAdaptiveRect */
void TestCalculateAdaptiveRect()
{
    Section("ImageAttribute::CalculateAdaptiveRect");

    // 图片比例 2:1，目标区域 100x100：以宽度为准，高度 50，垂直居中
    ui::UiRect rc = ui::ImageAttribute::CalculateAdaptiveRect(200, 100, ui::UiRect(0, 0, 100, 100),
                                                              _T("center"), _T("center"));
    CHECK_INT(rc.left, 0);
    CHECK_INT(rc.top, 25);
    CHECK_INT(rc.right, 100);
    CHECK_INT(rc.bottom, 75);

    // 右下对齐
    rc = ui::ImageAttribute::CalculateAdaptiveRect(200, 100, ui::UiRect(0, 0, 100, 100),
                                                   _T("right"), _T("bottom"));
    CHECK_INT(rc.left, 0);
    CHECK_INT(rc.top, 50);
    CHECK_INT(rc.right, 100);
    CHECK_INT(rc.bottom, 100);

    // 图片比例 1:2，目标区域 200x200：以高度为准，宽度 100，水平居中
    rc = ui::ImageAttribute::CalculateAdaptiveRect(100, 200, ui::UiRect(10, 20, 210, 220),
                                                   _T("center"), _T("top"));
    CHECK_INT(rc.left, 60);
    CHECK_INT(rc.top, 20);
    CHECK_INT(rc.right, 160);
    CHECK_INT(rc.bottom, 220);

    // 无效图片尺寸：返回空大小，使用左上对齐时位置保持为目标区域左上角
    rc = ui::ImageAttribute::CalculateAdaptiveRect(0, 100, ui::UiRect(10, 20, 110, 120),
                                                   _T("left"), _T("top"));
    CHECK_INT(rc.left, 10);
    CHECK_INT(rc.top, 20);
    CHECK_INT(rc.right, 10);
    CHECK_INT(rc.bottom, 20);
}

/** StateImage 的状态图片管理（不涉及绘制） */
void TestStateImage()
{
    Section("StateImage");

    const ui::DpiManager& dpi = ui::GlobalManager::Instance().Dpi();
    ui::StateImage stateImage;
    CHECK(!stateImage.HasImage());
    CHECK(!stateImage.HasHoveredImage());
    CHECK(stateImage.GetStateImage(ui::kControlStateNormal) == nullptr);
    CHECK_STR(stateImage.GetImageString(ui::kControlStateNormal), _T(""));
    CHECK_INT(stateImage.GetImageFade(ui::kControlStateNormal), 255);

    stateImage.SetImageString(ui::kControlStateNormal,
                              _T("file='res/normal.png' name='normalImg' source='0,0,10,10'"), dpi);
    CHECK(stateImage.HasImage());
    CHECK(!stateImage.HasHoveredImage());
    CHECK_STR(stateImage.GetImagePath(ui::kControlStateNormal), _T("res/normal.png"));
    CHECK_INT(stateImage.GetImageFade(ui::kControlStateNormal), 255);
    CHECK(stateImage.GetStateImage(ui::kControlStateNormal) != nullptr);
    CHECK(stateImage.FindImageByName(_T("normalImg")) == stateImage.GetStateImage(ui::kControlStateNormal));
    CHECK(stateImage.FindImageByName(_T("")) == nullptr);
    CHECK(stateImage.GetEstimateImage() == stateImage.GetStateImage(ui::kControlStateNormal));

    stateImage.SetImageString(ui::kControlStateHovered,
                              _T("file='res/hover.png' source='0,0,10,10' fade='128'"), dpi);
    CHECK(stateImage.HasHoveredImage());
    CHECK_INT(stateImage.GetImageFade(ui::kControlStateHovered), 128);
    CHECK(stateImage.AreImageSourceRectsEqual(ui::kControlStateNormal, ui::kControlStateHovered));

    // 修改为不同的源区域后，源区域不再相同
    stateImage.SetImageString(ui::kControlStateHovered,
                              _T("file='res/hover.png' source='0,0,12,10' fade='128'"), dpi);
    CHECK(!stateImage.AreImageSourceRectsEqual(ui::kControlStateNormal, ui::kControlStateHovered));

    // 设置为空字符串时删除该状态的图片
    stateImage.SetImageString(ui::kControlStateHovered, _T(""), dpi);
    CHECK(!stateImage.HasHoveredImage());
    CHECK(stateImage.GetStateImage(ui::kControlStateHovered) == nullptr);

    // Normal 图片被删除后，没有任何状态图片
    stateImage.SetImageString(ui::kControlStateNormal, _T(""), dpi);
    CHECK(!stateImage.HasImage());
    CHECK(stateImage.GetEstimateImage() == nullptr);

    // 多个状态：按名称查找与 GetAllImages
    stateImage.SetImageString(ui::kControlStateNormal, _T("file='res/n.png' name='n'"), dpi);
    stateImage.SetImageString(ui::kControlStateDisabled, _T("file='res/d.png' name='d'"), dpi);
    std::vector<ui::Image*> allImages;
    stateImage.GetAllImages(allImages);
    CHECK_INT(allImages.size(), 2);
    CHECK(stateImage.FindImageByName(_T("d")) == stateImage.GetStateImage(ui::kControlStateDisabled));
}

void AppendUInt16LE(std::vector<uint8_t>& data, uint16_t value)
{
    data.push_back((uint8_t)(value & 0xFF));
    data.push_back((uint8_t)((value >> 8) & 0xFF));
}

void AppendUInt32LE(std::vector<uint8_t>& data, uint32_t value)
{
    data.push_back((uint8_t)(value & 0xFF));
    data.push_back((uint8_t)((value >> 8) & 0xFF));
    data.push_back((uint8_t)((value >> 16) & 0xFF));
    data.push_back((uint8_t)((value >> 24) & 0xFF));
}

/** 构造一个只包含目录信息的 ICO 文件数据 */
std::vector<uint8_t> MakeIcoDirectoryData()
{
    std::vector<uint8_t> data;
    AppendUInt16LE(data, 0); // reserved
    AppendUInt16LE(data, 1); // type: icon
    AppendUInt16LE(data, 4); // count

    const auto addEntry = [&data](uint8_t width, uint8_t height, uint8_t colorCount) {
        data.push_back(width);
        data.push_back(height);
        data.push_back(colorCount);
        data.push_back(0);        // reserved
        AppendUInt16LE(data, 1);  // planes
        AppendUInt16LE(data, 32); // bit count
        AppendUInt32LE(data, 0);  // bytes in resource
        AppendUInt32LE(data, 0);  // image offset
    };
    addEntry(16, 16, 0);
    addEntry(32, 32, 0);
    addEntry(32, 32, 16);
    addEntry(0, 0, 0); // 0 表示 256
    return data;
}

/** ICO 目录信息解析与选择 */
void TestIcoDirectoryParsing()
{
    Section("ImageDecoderUtil ICO");

    const std::vector<uint8_t> icoData = MakeIcoDirectoryData();
    std::vector<ui::ImageDecoderUtil::IcoImageInfo> iconInfoList;
    CHECK(ui::ImageDecoderUtil::GetIcoImageInfo(icoData, iconInfoList));
    CHECK_INT(iconInfoList.size(), 4);
    CHECK_INT(iconInfoList[0].m_nWidth, 16);
    CHECK_INT(iconInfoList[0].m_nHeight, 16);
    CHECK_INT(iconInfoList[0].m_nColorCount, 0);
    CHECK_INT(iconInfoList[1].m_nWidth, 32);
    CHECK_INT(iconInfoList[2].m_nColorCount, 16);
    CHECK_INT(iconInfoList[3].m_nWidth, 256);
    CHECK_INT(iconInfoList[3].m_nHeight, 256);

    // 选择规则：优先最匹配的尺寸，然后按颜色数 0/256/16/2 的优先级
    CHECK_INT(ui::ImageDecoderUtil::SelectIcoImageInfo(iconInfoList, 32), 1);
    CHECK_INT(ui::ImageDecoderUtil::SelectIcoImageInfo(iconInfoList, 16), 0);
    // 20 距离 16 更近
    CHECK_INT(ui::ImageDecoderUtil::SelectIcoImageInfo(iconInfoList, 20), 0);
    // 40 距离 32 更近
    CHECK_INT(ui::ImageDecoderUtil::SelectIcoImageInfo(iconInfoList, 40), 1);
    // 超出所有尺寸：选择最大的 256
    CHECK_INT(ui::ImageDecoderUtil::SelectIcoImageInfo(iconInfoList, 300), 3);

    // 只有 32 尺寸、颜色数 16 的图标项时，仍能选中
    std::vector<ui::ImageDecoderUtil::IcoImageInfo> singleColorList;
    ui::ImageDecoderUtil::IcoImageInfo info;
    info.m_nWidth = 32;
    info.m_nHeight = 32;
    info.m_nColorCount = 16;
    singleColorList.push_back(info);
    CHECK_INT(ui::ImageDecoderUtil::SelectIcoImageInfo(singleColorList, 32), 0);

    // 空列表
    std::vector<ui::ImageDecoderUtil::IcoImageInfo> emptyList;
    CHECK_INT(ui::ImageDecoderUtil::SelectIcoImageInfo(emptyList, 32), -1);

    // 非法数据：长度不足、图标数为 0、目录项不完整
    std::vector<uint8_t> tooShort = { 0, 0, 1, 0, 1 };
    CHECK(!ui::ImageDecoderUtil::GetIcoImageInfo(tooShort, iconInfoList));

    std::vector<uint8_t> zeroCount = { 0, 0, 1, 0, 0, 0 };
    CHECK(!ui::ImageDecoderUtil::GetIcoImageInfo(zeroCount, iconInfoList));

    std::vector<uint8_t> truncated = icoData;
    truncated.resize(truncated.size() - 1);
    CHECK(!ui::ImageDecoderUtil::GetIcoImageInfo(truncated, iconInfoList));
}

} // namespace

int main(int /*argc*/, char* /*argv*/[])
{
    //关闭输出缓冲，便于程序异常时也能看到已产生的输出
    ::setvbuf(stdout, nullptr, _IONBF, 0);
#if defined(_WIN32) && defined(_DEBUG)
    //把断言输出重定向到标准错误，避免在无界面环境下弹出断言对话框导致进程挂起
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    TestImageUtilScalar();
    TestFlipPixelBits();
    TestResizeImageData();
    TestGetBestImageScale();
    TestImageLoadParam();
    TestImageAttribute();
    TestCalculateAdaptiveRect();
    TestStateImage();
    TestIcoDirectoryParsing();

    std::cout << "image_core: " << g_checkCount << " checks, "
              << g_failedCount << " failed" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "image_core: FAILED" << std::endl;
        return 1;
    }
    std::cout << "image_core: PASSED" << std::endl;
    return 0;
}
