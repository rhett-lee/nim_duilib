// Image 异步解码与取消逻辑的单元测试
//
// 覆盖内容：
//  1. 多帧图片(GIF)：异步加载后的初始状态、立即取消、解码部分帧后取消、
//     取消后恢复解码(从下一个未解码帧继续)、全部解码完成后的状态；
//  2. 静态图片(WEBP)：在解码阶段按目标区域降采样(避免先解出原始大图)；
//  3. 静态图片异步解码(WEBP/ICO)：未解码完成时返回 nullptr 但不是解码错误，
//     取消后不产生任何可用位图，正常解码合并后位图可用；
//  4. ImageInfo::GetBitmap 的尺寸容差与内联缩放缓存。
//
// 说明：
//  "切换视图/更换图片时取消图片加载"在解码层的表现是：解码任务的 IsAborted
//  回调返回 true 后，本次解码不再产生新的帧数据；已解码的帧保持不变，
//  恢复解码时从下一个未解码帧继续。本测试用 IsAborted 回调精确模拟该过程。
//
// 本测试不创建窗口，但需要 duilib 的资源目录(编译期宏 DUILIB_TEST_RESOURCE_DIR，
// 默认指向源码树的 bin/resources)，并需要渲染工厂来创建位图。

#if defined(_WIN32)
#include <crtdbg.h>
#include <windows.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ImageManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Image/ImageDecoder.h"
#include "duilib/Image/ImageInfo.h"
#include "duilib/Image/ImageLoadParam.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#include <cstdio>
#include <iostream>
#include <memory>

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

/** 测试图片的完整路径(位于 bin/resources/themes/default/render 目录) */
DString GetImagePath(const DString& fileName)
{
    return DString(_T(DUILIB_TEST_RESOURCE_DIR)) + _T("/themes/default/render/") + fileName;
}

/** 加载一个图片对象
* @param [in] fileName 图片文件名(位于 themes/default/render 目录)
* @param [in] bAsyncDecode 是否启用异步解码
* @param [in] maxDestSize 最大目标区域(为空表示不限制，不启用解码期降采样)
* @param [in] srcWidth 图片的固定宽度(空字符串表示使用图片原始宽度)
* @param [in] srcHeight 图片的固定高度(空字符串表示使用图片原始高度)
* @param [in] nIconSize 图标的目标尺寸(仅对ICO有效，0表示使用默认值)
*/
std::shared_ptr<ui::ImageInfo> LoadImage(const DString& fileName,
                                         bool bAsyncDecode,
                                         const ui::UiSize& maxDestSize = ui::UiSize(),
                                         const DString& srcWidth = DString(),
                                         const DString& srcHeight = DString(),
                                         uint32_t nIconSize = 0)
{
    //参数：DPI自适应关闭、加载DPI 100%、是否异步解码、不使用ICO动画、禁用断言(测试环境无界面)
    ui::ImageLoadParam loadParam(srcWidth, srcHeight, false, 100, bAsyncDecode,
                                 false, 1000, nIconSize, 30.0f, false, DString());
    if ((maxDestSize.cx > 0) || (maxDestSize.cy > 0)) {
        loadParam.SetMaxDestRectSize(maxDestSize);
    }
    ui::ImageLoadPath loadPath;
    loadPath.m_imageFullPath = ui::FilePath(GetImagePath(fileName));
    loadPath.m_pathType = ui::ImageLoadPathType::kLocalPath;
    loadParam.SetImageLoadPath(loadPath);

    bool bFromCache = false;
    return ui::GlobalManager::Instance().Image().GetImage(loadParam, nullptr, bFromCache);
}

/** 多帧图片(GIF)的异步解码与取消逻辑 */
void TestGifAsyncDecodeAndCancel()
{
    const std::shared_ptr<ui::ImageInfo> pImageInfo = LoadImage(_T("gif_test.gif"), true);
    CHECK(pImageInfo != nullptr);
    if (pImageInfo == nullptr) {
        return;
    }
    CHECK(pImageInfo->IsMultiFrameImage());
    const uint32_t nFrameCount = pImageInfo->GetFrameCount();
    CHECK(nFrameCount >= 3);

    const std::shared_ptr<ui::IImage> pImageData = pImageInfo->GetImageData();
    CHECK(pImageData != nullptr);
    if (pImageData == nullptr) {
        return;
    }
    CHECK(pImageData->GetImageType() == ui::ImageType::kImageAnimation);
    //异步加载阶段：不预先解码任何帧
    CHECK(pImageData->IsAsyncDecodeEnabled());
    CHECK(!pImageData->IsAsyncDecodeFinished());
    CHECK(pImageData->GetDecodedFrameIndex() == (uint32_t)-1);

    //用例1：立即取消(模拟加载刚开始就切换视图)：不产生任何已解码帧
    bool bDecodeError = false;
    CHECK(pImageData->AsyncDecode(0, []() { return true; }, &bDecodeError));
    CHECK(!bDecodeError);
    CHECK(!pImageData->MergeAsyncDecodeData());
    CHECK(pImageData->GetDecodedFrameIndex() == (uint32_t)-1);
    CHECK(!pImageData->IsAsyncDecodeFinished());
    CHECK(!pImageInfo->IsFrameDataReady(0));

    //用例2：解码1帧后取消(模拟解码过程中切换视图)
    int32_t nAbortCalls = 0;
    CHECK(pImageData->AsyncDecode(nFrameCount - 1, [&nAbortCalls]() {
        //第一次检查返回false(允许解码一帧)，之后返回true(中止解码)
        return (++nAbortCalls > 1);
    }, &bDecodeError));
    CHECK(!bDecodeError);
    //解码结果需要合并后才可见
    CHECK(pImageData->GetDecodedFrameIndex() == (uint32_t)-1);
    CHECK(pImageData->MergeAsyncDecodeData());
    CHECK(pImageData->GetDecodedFrameIndex() == 0);
    CHECK(!pImageData->IsAsyncDecodeFinished());
    CHECK(pImageInfo->IsFrameDataReady(0));
    const ui::AnimationFramePtr pFrame0 = pImageInfo->GetFrame(0, ui::UiSize());
    CHECK((pFrame0 != nullptr) && !pFrame0->m_bDataPending && (pFrame0->m_pBitmap != nullptr));

    //用例3：取消后恢复解码：从第1帧继续，解码到第2帧
    if (nFrameCount >= 3) {
        CHECK(pImageData->AsyncDecode(2, nullptr, &bDecodeError));
        CHECK(!bDecodeError);
        CHECK(pImageData->MergeAsyncDecodeData());
        CHECK(pImageData->GetDecodedFrameIndex() == 2);
        CHECK(pImageInfo->IsFrameDataReady(2));
    }

    //用例4：解码到最后一帧后，异步解码完成
    CHECK(pImageData->AsyncDecode(nFrameCount - 1, nullptr, &bDecodeError));
    CHECK(!bDecodeError);
    CHECK(pImageData->MergeAsyncDecodeData());
    CHECK(pImageData->GetDecodedFrameIndex() == (nFrameCount - 1));
    CHECK(pImageData->IsAsyncDecodeFinished());
    CHECK(pImageInfo->IsFrameDataReady(nFrameCount - 1));
}

/** 静态图片(WEBP)在解码阶段按目标区域降采样 */
void TestWebpStaticDecodeScale()
{
    //该图片原始大小为 600 x 600(静态图片)
    const std::shared_ptr<ui::ImageInfo> pImageInfo = LoadImage(_T("webp_test2.webp"), false,
                                                                ui::UiSize(200, 200));
    CHECK(pImageInfo != nullptr);
    if (pImageInfo == nullptr) {
        return;
    }
    CHECK(!pImageInfo->IsMultiFrameImage());
    const std::shared_ptr<ui::IImage> pImageData = pImageInfo->GetImageData();
    CHECK(pImageData != nullptr);
    if (pImageData == nullptr) {
        return;
    }
    CHECK(pImageData->GetImageType() == ui::ImageType::kImageBitmap);
    //按 200x200 的目标区域降采样，宽高均不超过 200(允许 1 像素的取整误差)
    CHECK(pImageData->GetImageSizeScale() < 1.0f);
    CHECK(pImageData->GetWidth() <= 201);
    CHECK(pImageData->GetHeight() <= 201);

    bool bDecodeError = false;
    const std::shared_ptr<ui::IBitmap> pBitmap = pImageInfo->GetBitmap(&bDecodeError, nullptr);
    CHECK(pBitmap != nullptr);
    CHECK(!bDecodeError);
    if (pBitmap != nullptr) {
        CHECK(pBitmap->GetWidth() <= 201);
        CHECK(pBitmap->GetHeight() <= 201);
        CHECK(pBitmap->GetWidth() > 0);
        CHECK(pBitmap->GetHeight() > 0);
    }
    //再次获取时使用第一次的位图缓存
    CHECK(pImageInfo->GetBitmap(&bDecodeError, nullptr) == pBitmap);
}

/** 静态图片(WEBP)的异步解码与取消逻辑 */
void TestWebpStaticAsyncDecodeAndCancel()
{
    const std::shared_ptr<ui::ImageInfo> pImageInfo = LoadImage(_T("webp_test2.webp"), true,
                                                                ui::UiSize(200, 200));
    CHECK(pImageInfo != nullptr);
    if (pImageInfo == nullptr) {
        return;
    }
    const std::shared_ptr<ui::IImage> pImageData = pImageInfo->GetImageData();
    CHECK(pImageData != nullptr);
    if (pImageData == nullptr) {
        return;
    }
    CHECK(pImageData->GetImageType() == ui::ImageType::kImageBitmap);
    CHECK(pImageData->IsAsyncDecodeEnabled());
    CHECK(!pImageData->IsAsyncDecodeFinished());
    CHECK(pImageData->GetDecodedFrameIndex() == (uint32_t)-1);

    //用例1：异步解码尚未完成时，位图不可用，但不算解码错误
    bool bDecodeError = true;
    CHECK(pImageInfo->GetBitmap(&bDecodeError, nullptr) == nullptr);
    CHECK(!bDecodeError);

    //用例2：立即取消：仍然没有可用位图，且不算解码错误
    bool bAsyncDecodeError = false;
    CHECK(pImageData->AsyncDecode(0, []() { return true; }, &bAsyncDecodeError));
    CHECK(!bAsyncDecodeError);
    CHECK(!pImageData->MergeAsyncDecodeData());
    CHECK(!pImageData->IsAsyncDecodeFinished());
    bDecodeError = true;
    CHECK(pImageInfo->GetBitmap(&bDecodeError, nullptr) == nullptr);
    CHECK(!bDecodeError);

    //用例3：正常解码并合并后，位图可用，且异步解码完成
    CHECK(pImageData->AsyncDecode(0, nullptr, &bAsyncDecodeError));
    CHECK(!bAsyncDecodeError);
    CHECK(pImageData->MergeAsyncDecodeData());
    CHECK(pImageData->IsAsyncDecodeFinished());
    bDecodeError = true;
    const std::shared_ptr<ui::IBitmap> pBitmap = pImageInfo->GetBitmap(&bDecodeError, nullptr);
    CHECK(pBitmap != nullptr);
    CHECK(!bDecodeError);
    if (pBitmap != nullptr) {
        CHECK(pBitmap->GetWidth() <= 201);
        CHECK(pBitmap->GetHeight() <= 201);
    }
}

/** 静态图片(ICO，使用 Image_DelayBitmap 延迟解码)的异步解码与取消逻辑 */
void TestIcoAsyncDecodeAndCancel()
{
    //指定图标的目标尺寸为 48(与ICO中的48x48条目一致)
    const std::shared_ptr<ui::ImageInfo> pImageInfo = LoadImage(_T("icon_test.ico"), true,
                                                                ui::UiSize(),
                                                                DString(), DString(), 48);
    CHECK(pImageInfo != nullptr);
    if (pImageInfo == nullptr) {
        return;
    }
    CHECK(!pImageInfo->IsMultiFrameImage());
    const std::shared_ptr<ui::IImage> pImageData = pImageInfo->GetImageData();
    CHECK(pImageData != nullptr);
    if (pImageData == nullptr) {
        return;
    }
    CHECK(pImageData->GetImageType() == ui::ImageType::kImageBitmap);
    CHECK(pImageData->IsAsyncDecodeEnabled());
    CHECK(!pImageData->IsAsyncDecodeFinished());
    CHECK(pImageData->GetDecodedFrameIndex() == (uint32_t)-1);

    //用例1：尚未解码时返回 nullptr，但不是解码错误
    bool bDecodeError = false;
    CHECK(pImageInfo->GetBitmap(&bDecodeError, nullptr) == nullptr);
    CHECK(!bDecodeError);

    //用例2：立即取消：不产生位图，且不算解码错误(Image_DelayBitmap 在取消时返回false)
    bool bAsyncDecodeError = false;
    CHECK(!pImageData->AsyncDecode(0, []() { return true; }, &bAsyncDecodeError));
    CHECK(!bAsyncDecodeError);
    CHECK(!pImageData->MergeAsyncDecodeData());
    CHECK(!pImageData->IsAsyncDecodeFinished());
    CHECK(pImageData->GetDecodedFrameIndex() == (uint32_t)-1);

    //用例3：正常解码并合并后，位图可用，且异步解码完成
    CHECK(pImageData->AsyncDecode(0, nullptr, &bAsyncDecodeError));
    CHECK(!bAsyncDecodeError);
    CHECK(pImageData->MergeAsyncDecodeData());
    CHECK(pImageData->IsAsyncDecodeFinished());
    CHECK(pImageData->GetDecodedFrameIndex() == 0);
    bDecodeError = true;
    const std::shared_ptr<ui::IBitmap> pBitmap = pImageInfo->GetBitmap(&bDecodeError, nullptr);
    CHECK(pBitmap != nullptr);
    CHECK(!bDecodeError);
}

/** 尺寸相差 1 像素(在容差范围内)时，直接复用原始位图，不做缩放 */
void TestImageInfoBitmapSizeTolerance()
{
    //autumn.png 原始大小为 180 x 186
    const std::shared_ptr<ui::ImageInfo> pImageInfo = LoadImage(_T("autumn.png"), false,
                                                                ui::UiSize(),
                                                                _T("180"), _T("187"));
    CHECK(pImageInfo != nullptr);
    if (pImageInfo == nullptr) {
        return;
    }
    CHECK(pImageInfo->GetWidth() == 180);
    CHECK(pImageInfo->GetHeight() == 187);
    bool bDecodeError = false;
    const std::shared_ptr<ui::IBitmap> pBitmap = pImageInfo->GetBitmap(&bDecodeError, nullptr);
    CHECK(pBitmap != nullptr);
    CHECK(!bDecodeError);
    if (pBitmap != nullptr) {
        //容差范围内不缩放，保持原始位图尺寸
        CHECK(pBitmap->GetWidth() == 180);
        CHECK(pBitmap->GetHeight() == 186);
    }
}

/** Image_DelayBitmap 的尺寸语义：GetWidth()/GetHeight() 必须与最终解码出的位图尺寸一致
*   异步解码期间做降采样时，若 ImageInfo 按原图尺寸计算，会把已降采样的位图再放大回去
*/
void TestDelayBitmapScaleConsistency()
{
    //用例1：ICO，目标尺寸 40(ICO 中最近的条目为 48x48，加载时按 40/48 缩放)
    {
        const std::shared_ptr<ui::ImageInfo> pImageInfo = LoadImage(_T("icon_test.ico"), true,
                                                                    ui::UiSize(),
                                                                    DString(), DString(), 40);
        CHECK(pImageInfo != nullptr);
        if (pImageInfo == nullptr) {
            return;
        }
        const std::shared_ptr<ui::IImage> pImageData = pImageInfo->GetImageData();
        CHECK(pImageData != nullptr);
        if (pImageData == nullptr) {
            return;
        }
        CHECK(pImageData->GetImageSizeScale() < 1.0f);
        //尺寸为缩放后的尺寸，而不是原图(48x48)尺寸
        CHECK(pImageData->GetWidth() == 40);
        CHECK(pImageData->GetHeight() == 40);
        CHECK(pImageInfo->GetWidth() == 40);
        CHECK(pImageInfo->GetHeight() == 40);

        bool bDecodeError = false;
        CHECK(!pImageData->AsyncDecode(0, []() { return true; }, &bDecodeError));
        CHECK(!bDecodeError);
        CHECK(!pImageData->MergeAsyncDecodeData());
        CHECK(pImageInfo->GetBitmap(&bDecodeError, nullptr) == nullptr);
        CHECK(!bDecodeError);

        CHECK(pImageData->AsyncDecode(0, nullptr, &bDecodeError));
        CHECK(!bDecodeError);
        CHECK(pImageData->MergeAsyncDecodeData());
        const std::shared_ptr<ui::IBitmap> pBitmap = pImageInfo->GetBitmap(&bDecodeError, nullptr);
        CHECK(pBitmap != nullptr);
        CHECK(!bDecodeError);
        if (pBitmap != nullptr) {
            //位图尺寸与 ImageInfo 尺寸一致，不再做二次缩放
            CHECK(pBitmap->GetWidth() == 40);
            CHECK(pBitmap->GetHeight() == 40);
        }
    }

    //用例2：Common 解码器(BMP/PNG 等)的异步解码 + 解码期降采样
    //autumn.png 原始大小为 180 x 186，目标区域 90x93 → 缩放到 90x93
    {
        const std::shared_ptr<ui::ImageInfo> pImageInfo = LoadImage(_T("autumn.png"), true,
                                                                    ui::UiSize(90, 93));
        CHECK(pImageInfo != nullptr);
        if (pImageInfo == nullptr) {
            return;
        }
        const std::shared_ptr<ui::IImage> pImageData = pImageInfo->GetImageData();
        CHECK(pImageData != nullptr);
        if (pImageData == nullptr) {
            return;
        }
        CHECK(pImageData->GetImageSizeScale() < 1.0f);
        CHECK(pImageData->GetWidth() == 90);
        CHECK(pImageData->GetHeight() == 93);
        CHECK(pImageInfo->GetWidth() == 90);
        CHECK(pImageInfo->GetHeight() == 93);

        bool bDecodeError = false;
        CHECK(pImageInfo->GetBitmap(&bDecodeError, nullptr) == nullptr);
        CHECK(!bDecodeError);

        bool bAsyncDecodeError = false;
        CHECK(pImageData->AsyncDecode(0, nullptr, &bAsyncDecodeError));
        CHECK(!bAsyncDecodeError);
        CHECK(pImageData->MergeAsyncDecodeData());
        const std::shared_ptr<ui::IBitmap> pBitmap = pImageInfo->GetBitmap(&bDecodeError, nullptr);
        CHECK(pBitmap != nullptr);
        CHECK(!bDecodeError);
        if (pBitmap != nullptr) {
            CHECK(pBitmap->GetWidth() == 90);
            CHECK(pBitmap->GetHeight() == 93);
        }
    }
}

/** 动画图片(WEBP)的异步解码与取消逻辑 */
void TestWebpAnimatedAsyncDecodeAndCancel()
{
    //webp_test.webp 为动画图片(共5帧)
    const std::shared_ptr<ui::ImageInfo> pImageInfo = LoadImage(_T("webp_test.webp"), true);
    CHECK(pImageInfo != nullptr);
    if (pImageInfo == nullptr) {
        return;
    }
    CHECK(pImageInfo->IsMultiFrameImage());
    const uint32_t nFrameCount = pImageInfo->GetFrameCount();
    CHECK(nFrameCount >= 3);

    const std::shared_ptr<ui::IImage> pImageData = pImageInfo->GetImageData();
    CHECK(pImageData != nullptr);
    if (pImageData == nullptr) {
        return;
    }
    CHECK(pImageData->GetImageType() == ui::ImageType::kImageAnimation);
    CHECK(pImageData->IsAsyncDecodeEnabled());
    CHECK(!pImageData->IsAsyncDecodeFinished());
    CHECK(pImageData->GetDecodedFrameIndex() == (uint32_t)-1);

    //用例1：立即取消：不产生任何已解码帧
    bool bDecodeError = false;
    CHECK(pImageData->AsyncDecode(0, []() { return true; }, &bDecodeError));
    CHECK(!bDecodeError);
    CHECK(!pImageData->MergeAsyncDecodeData());
    CHECK(pImageData->GetDecodedFrameIndex() == (uint32_t)-1);
    CHECK(!pImageInfo->IsFrameDataReady(0));

    //用例2：解码1帧后取消
    int32_t nAbortCalls = 0;
    CHECK(pImageData->AsyncDecode(nFrameCount - 1, [&nAbortCalls]() {
        return (++nAbortCalls > 1);
    }, &bDecodeError));
    CHECK(!bDecodeError);
    CHECK(pImageData->MergeAsyncDecodeData());
    CHECK(pImageData->GetDecodedFrameIndex() == 0);
    CHECK(pImageInfo->IsFrameDataReady(0));

    //用例3：取消后恢复解码到第2帧
    if (nFrameCount >= 3) {
        CHECK(pImageData->AsyncDecode(2, nullptr, &bDecodeError));
        CHECK(!bDecodeError);
        CHECK(pImageData->MergeAsyncDecodeData());
        CHECK(pImageData->GetDecodedFrameIndex() == 2);
        CHECK(pImageInfo->IsFrameDataReady(2));
    }
}

} //namespace

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

    const DString resourcePath = _T(DUILIB_TEST_RESOURCE_DIR);
    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(ui::FilePath(resourcePath)))) {
        std::cerr << "[FAIL] GlobalManager::Startup failed, resource path: "
                  << ui::StringConvert::UTF16ToUTF8(resourcePath.c_str(), resourcePath.size()) << std::endl;
        ++g_failedCount;
    }
    else {
        struct GlobalShutdown
        {
            ~GlobalShutdown() { ui::GlobalManager::Instance().Shutdown(); }
        } shutdownGuard;

        ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
        Check(pRenderFactory != nullptr, "GetRenderFactory() is null");
        if (pRenderFactory != nullptr) {
            //测试环境没有消息循环，无法执行延迟释放任务：
            //禁止图片进入延迟释放队列(立即释放)，避免缓存中残留旧参数的图片数据
            ui::GlobalManager::Instance().Image().SetReleaseImageCallback(
                [](const std::shared_ptr<ui::IImage>&, const DString&) { return false; });

            //清理全局缓存，保证每个用例解析到的图片状态是独立的
            ui::GlobalManager::Instance().Image().RemoveAllImages();
            TestGifAsyncDecodeAndCancel();
            TestWebpStaticDecodeScale();
            TestWebpStaticAsyncDecodeAndCancel();
            TestIcoAsyncDecodeAndCancel();
            TestWebpAnimatedAsyncDecodeAndCancel();
            TestImageInfoBitmapSizeTolerance();
            TestDelayBitmapScaleConsistency();
        }
    }

    std::cout << "image_async: " << g_checkCount << " checks, "
              << g_failedCount << " failed" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "image_async: FAILED" << std::endl;
        return 1;
    }
    std::cout << "image_async: PASSED" << std::endl;
    return 0;
}
