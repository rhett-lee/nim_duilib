#include "ImageDecoder.h"
#include "duilib/Image/Image.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/PerformanceUtil.h"

#include "duilib/third_party/apng/decoder-apng.h"

#pragma warning (push)
#pragma warning (disable: 4244)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_TGA
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_GIF
#define STBI_NO_PNG
#include "duilib/third_party/stb_image/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_TRIANGLE
#pragma warning (disable: 4505)
    #include "duilib/third_party/stb_image/stb_image_resize2.h"
#pragma warning (pop)

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#pragma warning (push)
#pragma warning (disable: 4456 4244 4702)

#define NANOSVG_IMPLEMENTATION
#define NANOSVG_ALL_COLOR_KEYWORDS
#include "duilib/third_party/svg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "duilib/third_party/svg/nanosvgrast.h"

#pragma warning (pop)
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#pragma warning (push)
#pragma warning (disable: 4996)
    #include "duilib/third_party/cximage/ximage.h"
#pragma warning (pop)

#include "duilib/third_party/libwebp/src/webp/decode.h"
#include "duilib/third_party/libwebp/src/webp/demux.h"

#include "duilib/RenderSkia/SkiaHeaderBegin.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "include/core/SkStream.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "duilib/RenderSkia/SkiaHeaderEnd.h"

namespace ui 
{
/** 图片的DPI缩放尺寸计算函数
*/
namespace ImageLoader
{
    /** 获取图片加载后应当缩放的宽度和高度
    * @param [in] imageLoadAttribute 图片的加载属性信息
    * @param [in] bEnableDpiScale 是否允许按照DPI对图片大小进行缩放（此为功能开关）
    * @param [in] nImageDpiScale 图片数据对应的DPI缩放百分比
    * @param [in] nWindowDpiScale 显示目标窗口的DPI缩放百分比
    * @param [out] bDpiScaled 图片加载的时候，图片大小是否进行了DPI自适应操作
    * @param [in,out] nImageWidth 原始图片的宽度，返回计算后的宽度
    * @param [in,out] nImageHeight 原始图片的高度，返回计算后的高度
    */
    static void CalcImageLoadSize(const ImageLoadAttribute& imageLoadAttribute,
                                  bool bEnableDpiScale,
                                  uint32_t nImageDpiScale,
                                  uint32_t nWindowDpiScale,
                                  bool& bDpiScaled,
                                  uint32_t& nImageWidth, uint32_t& nImageHeight)
    {
        bDpiScaled = false;
        if (!bEnableDpiScale) {
            ASSERT(nImageDpiScale == 100);
            nImageDpiScale = 100;
        }
        ASSERT(nImageDpiScale != 0);
        if (nImageDpiScale == 0) {
            return;
        }
        ASSERT((nImageWidth > 0) && (nImageHeight > 0));
        if ((nImageWidth == 0) || (nImageHeight == 0)) {
            return;
        }

        uint32_t nOldImageWidth = nImageWidth;
        uint32_t nOldImageHeight = nImageHeight;
        //此处：如果只设置了宽度或者高度，那么会按锁定纵横比的方式对整个图片进行缩放
        if (!imageLoadAttribute.CalcImageLoadSize(nImageWidth, nImageHeight)) {
            nImageWidth = nOldImageWidth;
            nImageHeight = nOldImageHeight;
        }

        //加载图片时，按需对图片大小进行DPI自适应
        bool needDpiScale = bEnableDpiScale;
        if (imageLoadAttribute.HasSrcDpiScale()) {
            //如果配置文件中有设置scaledpi属性，则以配置文件中的设置为准
            needDpiScale = imageLoadAttribute.NeedDpiScale();
        }
        if (needDpiScale) {
            ASSERT(nImageDpiScale > 0);
            ASSERT(nWindowDpiScale > 0);
            if ((nWindowDpiScale != nImageDpiScale) && (nWindowDpiScale != 0) && (nImageDpiScale != 0)) {
                float scaleRatio = (float)nWindowDpiScale / (float)nImageDpiScale;
                nImageWidth = static_cast<int>(nImageWidth * scaleRatio);
                nImageHeight = static_cast<int>(nImageHeight * scaleRatio);
                bDpiScaled = true;
            }
        }
    }
}

/** 使用stb_image加载图片
*/
namespace STBImageLoader
{
    /** 从内存数据加载图片
    */
    bool LoadImageFromMemory(std::vector<uint8_t>& fileData, ImageDecoder::ImageData& imageData)
    {
        ASSERT(!fileData.empty());
        if (fileData.empty()) {
            return false;
        }
        const uint8_t* buffer = fileData.data();
        int len = (int)fileData.size();
        int nWidth = 0;
        int nHeight = 0;
        std::vector<uint8_t>& argbData = imageData.m_bitmapData;
        argbData.clear();
        ASSERT((buffer != nullptr) && (len > 0));
        if ((buffer == nullptr) || (len <= 0)) {
            return false;
        }
        constexpr const int desired_channels = 4; //返回的图像数据格式固定：RGBA，每个图像元素是4个字节
        int channels_in_file = 4;
        uint8_t* rgbaData = stbi_load_from_memory(buffer, len, &nWidth, &nHeight, &channels_in_file, desired_channels);
        if (rgbaData == nullptr) {
            return false;
        }

        ASSERT( (channels_in_file == 3) || (channels_in_file == 4));
        ASSERT((nWidth > 0) && (nHeight > 0));
        if (((channels_in_file == 3) || (channels_in_file == 4)) && 
            (nWidth > 0) && (nHeight > 0)) {
            argbData.resize((size_t)nHeight * nWidth * desired_channels);
            const size_t colorCount = (size_t)nHeight * nWidth;

            //数据格式转换：ABGR[alpha, blue, green, red] -> ARGB[alpha, red, green, blue]                
            for (size_t i = 0; i < colorCount; ++i) {
                size_t colorIndex = i * 4;
                if (channels_in_file == 4) {
                    argbData[colorIndex + 3] = rgbaData[colorIndex + 3]; //A, alpha
                }
                else {
                    argbData[colorIndex + 3] = 255; //A, alpha
                }
#ifdef DUILIB_BUILD_FOR_WIN
                //数据格式：Window平台BGRA，其他平台RGBA
                argbData[colorIndex + 0] = rgbaData[colorIndex + 2]; //B, blue
                argbData[colorIndex + 1] = rgbaData[colorIndex + 1]; //G, green
                argbData[colorIndex + 2] = rgbaData[colorIndex + 0]; //R, red
#else
                argbData[colorIndex + 0] = rgbaData[colorIndex + 0]; //R, red
                argbData[colorIndex + 1] = rgbaData[colorIndex + 1]; //G, green
                argbData[colorIndex + 2] = rgbaData[colorIndex + 2]; //B, blue
#endif
            }

            imageData.bFlipHeight = true;
            imageData.m_frameInterval = 0;
            imageData.m_imageWidth = nWidth;
            imageData.m_imageHeight = nHeight;
        }
        stbi_image_free(rgbaData);
        return !argbData.empty();
    }
}//STBImageLoader

/** 使用APNG加载图片
*/
namespace APNGImageLoader
{
    bool DecodeAPNG(APNGDATA* pngData, std::vector<ImageDecoder::ImageData>& imageData, int32_t& playCount)
    {
        ASSERT((pngData != nullptr) && (pngData->nWid > 0) && (pngData->nHei > 0) && (pngData->nFrames > 0));
        imageData.clear();
        playCount = pngData->nLoops;

        int nWid = pngData->nWid;
        int nHei = pngData->nHei;
        if ((nWid <= 0) || (nHei <= 0) || (pngData->nFrames < 1)) {
            return false;
        }

        //swap rgba to bgra and do premultiply
        uint8_t* p = pngData->pdata;
        if (p == nullptr) {
            return false;
        }
        int pixel_count = nWid * nHei * pngData->nFrames;
        for (int i = 0; i < pixel_count; ++i) {
            uint8_t a = p[3];
            uint8_t t = p[0];
            if (a)
            {
#ifdef DUILIB_BUILD_FOR_WIN
                //数据格式：Window平台BGRA，其他平台RGBA
                p[0] = (p[2] * a) / 255;
                p[1] = (p[1] * a) / 255;
                p[2] = (t * a) / 255;
#else
                p[0] = (p[0] * a) / 255;
                p[1] = (p[1] * a) / 255;
                p[2] = (p[2] * a) / 255;
#endif
            }
            else
            {
                memset(p, 0, 4);
            }
            p += 4;
        }

        p = pngData->pdata;

        imageData.resize(pngData->nFrames);
        for (int i = 0; i < pngData->nFrames; ++i) {
            const int imageDataSize = nWid * nHei * 4;
            ImageDecoder::ImageData& bitmapData = imageData[i];
            bitmapData.m_frameInterval = pngData->pDelay ? pngData->pDelay[i] : 0;
            bitmapData.bFlipHeight = true;
            bitmapData.m_imageWidth = nWid;
            bitmapData.m_imageHeight = nHei;
            bitmapData.m_bitmapData.resize(imageDataSize);
            ::memcpy(bitmapData.m_bitmapData.data(), p, imageDataSize);

            p += imageDataSize;
        }
        return true;
    }

    bool LoadImageFromMemory(const std::vector<uint8_t>& fileData, std::vector<ImageDecoder::ImageData>& imageData,
                             bool bLoadAllFrames, uint32_t& nOutFrameCount, int32_t& playCount)
    {
        ASSERT(!fileData.empty());
        if (fileData.empty()) {
            return false;
        }
        bool isLoaded = false;
        APNGDATA* apngData = LoadAPNG_from_memory((const char*)fileData.data(), fileData.size(), bLoadAllFrames, nOutFrameCount);
        if (apngData != nullptr) {
            isLoaded = DecodeAPNG(apngData, imageData, playCount);
            APNG_Destroy(apngData);
        }
        return isLoaded;
    }
}//APNGImageLoader

/** 加载SVG图片(NanoSvg)
*/
namespace NanoSvgImageLoader
{
    class SvgDeleter
    {
    public:
        inline void operator()(NSVGimage* x) const { nsvgDelete(x); }
    };

    class RasterizerDeleter
    {
    public:
        inline void operator()(NSVGrasterizer* x) const { nsvgDeleteRasterizer(x); }
    };

    /** 获取Svg图片的宽度和高度(仅解析xml，无渲染，速度快)
    */
    bool ImageSizeFromMemory(std::vector<uint8_t>& fileData, int32_t& nSvgImageWidth, int32_t& nSvgImageHeight)
    {
        ASSERT(!fileData.empty());
        if (fileData.empty()) {
            return false;
        }
        bool hasAppended = false;
        if (fileData.back() != '\0') {
            //确保是含尾0的字符串，避免越界访问内存
            fileData.push_back('\0');
            hasAppended = true;
        }
        char* pData = (char*)fileData.data();
        NSVGimage* svgData = nsvgParse(pData, "px", 96.0f);//传入"px"时，第三个参数dpi是不起作用的。
        if (hasAppended) {
            fileData.pop_back();
        }

        std::unique_ptr<NSVGimage, SvgDeleter> svg((NSVGimage*)svgData);
        int width = (int)(svg->width + 0.5f);
        int height = (int)(svg->height + 0.5f);
        if (width <= 0 || height <= 0) {
            return false;
        }
        nSvgImageWidth = width;
        nSvgImageHeight = height;
        return true;
    }

    /** 从内存数据加载svg图片
    * @param [in] fileData 图片文件的数据，部分格式加载过程中内部有增加尾0的写操作
    * @param [in] imageLoadAttribute 图片加载属性, 包括图片路径等
    * @param [in] bEnableDpiScale 是否允许按照DPI对图片大小进行缩放（此为功能开关）
    * @param [in] nImageDpiScale 图片数据对应的DPI缩放百分比（比如：i.jpg为100，i@150.jpg为150）
    * @param [in] nWindowDpiScale 显示目标窗口的DPI缩放百分比
    */
    bool LoadImageFromMemory(std::vector<uint8_t>& fileData, 
                             const ImageLoadAttribute& imageLoadAttribute, 
                             bool bEnableDpiScale,
                             uint32_t nImageDpiScale,
                             uint32_t nWindowDpiScale,
                             ImageDecoder::ImageData& imageData,
                             bool& bDpiScaled)
    {
        ASSERT(!fileData.empty());
        if (fileData.empty()) {
            return false;
        }
        bDpiScaled = false;
        bool hasAppended = false;
        if (fileData.back() != '\0') {
            //确保是含尾0的字符串，避免越界访问内存
            fileData.push_back('\0');
            hasAppended = true;
        }
        char* pData = (char*)fileData.data();
        NSVGimage* svgData = nsvgParse(pData, "px", 96.0f);//传入"px"时，第三个参数dpi是不起作用的。
        if (hasAppended) {
            fileData.pop_back();
        }

        std::unique_ptr<NSVGimage, SvgDeleter> svg((NSVGimage*)svgData);
        int width = (int)(svg->width + 0.5f);
        int height = (int)(svg->height + 0.5f);
        if (width <= 0 || height <= 0) {
            return false;
        }

        //计算缩放后的大小
        uint32_t nImageWidth = (uint32_t)width;
        uint32_t nImageHeight = (uint32_t)height;
        ImageLoader::CalcImageLoadSize(imageLoadAttribute,
                                       bEnableDpiScale, nImageDpiScale, nWindowDpiScale, bDpiScaled,
                                       nImageWidth, nImageHeight);

        //svg的缩放，只能按比例，锁定纵横比的方式缩放
        float scaleX = 1.0f * nImageWidth / width;
        float scaleY = 1.0f * nImageHeight / height;
        float scale = (scaleX > scaleY) ? scaleX : scaleY; //取最大的缩放比
        width = static_cast<int>(width * scale);
        height = static_cast<int>(height * scale);
        std::unique_ptr<NSVGrasterizer, RasterizerDeleter> rast(nsvgCreateRasterizer());        
        if ((width <= 0) || (height <= 0) || !rast) {
            return false;
        }

        constexpr const int dataSize = 4;
        std::vector<uint8_t>& bitmapData = imageData.m_bitmapData;
        bitmapData.resize((size_t)height * width * dataSize);
        uint8_t* pBmpBits = bitmapData.data();
        if (pBmpBits == nullptr) {
            return false;
        }
        nsvgRasterize(rast.get(), svg.get(), 0, 0, scale, pBmpBits, width, height, width * dataSize);

#ifdef DUILIB_BUILD_FOR_WIN
        //数据格式：Window平台BGRA，其他平台RGBA
        // nanosvg内部已经做过alpha预乘，这里只做R和B的交换
        for (int y = 0; y < height; ++y) {
            unsigned char* row = &pBmpBits[y * width * dataSize];
            for (int x = 0; x < width; ++x) {
                //SVG    数据的各个颜色值：row[0]:R, row[1]: G, row[2]: B, row[3]: A
                //输出    数据的各个颜色值：row[0]:B, row[1]: G, row[2]: R, row[3]: A
                unsigned char r = row[0];
                row[0] = row[2];
                row[2] = r;
                row += 4;
            }
        }
#endif

        imageData.m_frameInterval = 0;
        imageData.bFlipHeight = true;
        imageData.m_imageWidth = width;
        imageData.m_imageHeight = height;
        return true;
    }
} //NanoSvgImageLoader

/** 加载SVG图片(Skia库的svg引擎，兼容性更好，功能更丰富)
*/
namespace SkiaSvgImageLoader
{
    /** 从内存数据加载svg图片
    * @param [in] fileData 图片文件的数据，部分格式加载过程中内部有增加尾0的写操作
    * @param [in] imageLoadAttribute 图片加载属性, 包括图片路径等
    * @param [in] bEnableDpiScale 是否允许按照DPI对图片大小进行缩放（此为功能开关）
    * @param [in] nImageDpiScale 图片数据对应的DPI缩放百分比（比如：i.jpg为100，i@150.jpg为150）
    * @param [in] nWindowDpiScale 显示目标窗口的DPI缩放百分比
    */
    bool LoadImageFromMemory(std::vector<uint8_t>& fileData,
                             const ImageLoadAttribute& imageLoadAttribute,
                             bool bEnableDpiScale,
                             uint32_t nImageDpiScale,
                             uint32_t nWindowDpiScale,
                             ImageDecoder::ImageData& imageData,
                             bool& bDpiScaled)
    {
        ASSERT(!fileData.empty());
        if (fileData.empty()) {
            return false;
        }

        bDpiScaled = false;
        std::unique_ptr<SkMemoryStream> spMemStream = SkMemoryStream::MakeDirect(fileData.data(), fileData.size());
        ASSERT(spMemStream != nullptr);
        if (spMemStream == nullptr) {
            return false;
        }
        sk_sp<SkSVGDOM> svgDom = SkSVGDOM::MakeFromStream(*spMemStream);
        ASSERT(svgDom != nullptr);
        if (svgDom == nullptr) {
            return false;
        }
        ASSERT(svgDom->getRoot() != nullptr);
        if (svgDom->getRoot() == nullptr) {
            return false;
        }
        spMemStream.reset();

        SkSize svgSize = svgDom->getRoot()->intrinsicSize(SkSVGLengthContext(SkSize::Make(0, 0)));
        //使用NanoSvg计算图片的宽度和高度（Skia的Svg封装没有提供相关功能）
        int32_t nSvgImageWidth = int32_t(svgSize.width() + 0.5f);
        int32_t nSvgImageHeight = int32_t(svgSize.height() + 0.5f);
        if ((nSvgImageWidth < 1) || (nSvgImageHeight < 1)) {
            auto viewBox = svgDom->getRoot()->getViewBox();
            if (viewBox.isValid()) {
                nSvgImageWidth = int32_t(viewBox->width() + 0.5f);
                nSvgImageHeight = int32_t(viewBox->height() + 0.5f);
            }
        }
        else {
            //如果viewBox不存在，则设置一个，否则图片缩放时存在异常（此处逻辑保持与NanoSvg保持一致）
            auto viewBox = svgDom->getRoot()->getViewBox();
            if (!viewBox.isValid()) {
                svgDom->getRoot()->setViewBox(SkRect::MakeIWH(nSvgImageWidth, nSvgImageHeight));
            }
        }
        if ((nSvgImageWidth < 1) || (nSvgImageHeight < 1)) {
            //如果图片中没有直接定义宽和高，利用NanoSvg库获取
            if (!NanoSvgImageLoader::ImageSizeFromMemory(fileData, nSvgImageWidth, nSvgImageHeight)) {
                return false;
            }
        }

        if ((nSvgImageWidth < 1) || (nSvgImageHeight < 1)) {
            return false;
        }

        //计算缩放后的大小
        uint32_t nImageWidth = (uint32_t)nSvgImageWidth;
        uint32_t nImageHeight = (uint32_t)nSvgImageHeight;
        ImageLoader::CalcImageLoadSize(imageLoadAttribute,
                                       bEnableDpiScale, nImageDpiScale, nWindowDpiScale, bDpiScaled,
                                       nImageWidth, nImageHeight);
        if ((nImageWidth < 1) || (nImageHeight < 1)) {
            return false;
        }

        //设置容器大小与图片大小一致(图片大小为DPI缩放后的大小)
        svgDom->getRoot()->setWidth(SkSVGLength((SkScalar)nImageWidth, SkSVGLength::Unit::kPX));
        svgDom->getRoot()->setHeight(SkSVGLength((SkScalar)nImageHeight, SkSVGLength::Unit::kPX));
        svgDom->setContainerSize(SkSize::Make(SkISize::Make((int32_t)nImageWidth, (int32_t)nImageHeight)));

        SkBitmap skBitmap;
#ifdef DUILIB_BUILD_FOR_WIN
        SkImageInfo info = SkImageInfo::Make((int32_t)nImageWidth, (int32_t)nImageHeight, SkColorType::kN32_SkColorType, SkAlphaType::kPremul_SkAlphaType);
#else
        SkImageInfo info = SkImageInfo::Make((int32_t)nImageWidth, (int32_t)nImageHeight, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
#endif
        skBitmap.allocPixels(info);
        SkCanvas canvas(skBitmap);
        svgDom->render(&canvas);

        constexpr const int dataSize = 4;
        std::vector<uint8_t>& bitmapData = imageData.m_bitmapData;
        bitmapData.resize((size_t)nImageHeight * nImageWidth * dataSize);
        ::memcpy(bitmapData.data(), skBitmap.getPixels(), (size_t)nImageHeight * nImageWidth * dataSize);
        
        imageData.m_frameInterval = 0;
        imageData.bFlipHeight = true;
        imageData.m_imageWidth = nImageWidth;
        imageData.m_imageHeight = nImageHeight;
        return true;
    }
} //SkiaSvgImageLoader

/** 使用cximage加载图片（只支持GIF和ICO）两种格式
* @param [in] isIconFile 如果为true表示是ICO文件，否则为GIF文件
* @param [in] iconSize 需要加载ICO图标的大小，因ICO文件中包含了各种大小的图标，加载的时候，只加载其中一个图标
* @param [in] bLoadAllFrames 对于多帧图片，是否加载全部帧（true加载全部帧，false仅加载第1帧）
* @param [out] nFrameCount 返回图片总的帧数
*/
namespace CxImageLoader
{
    bool LoadImageFromMemory(std::vector<uint8_t>& fileData, 
                             std::vector<ImageDecoder::ImageData>& imageData, 
                             bool isIconFile,
                             uint32_t iconSize,
                             bool bLoadAllFrames,
                             uint32_t& nOutFrameCount)
    {
        ASSERT(!fileData.empty());
        if (fileData.empty()) {
            return false;
        }
        uint32_t imagetype = isIconFile ? CXIMAGE_FORMAT_ICO : CXIMAGE_FORMAT_GIF;
        CxMemFile stream(fileData.data(), (uint32_t)fileData.size());
        CxImage cxImage(imagetype);
        cxImage.SetRetreiveAllFrames(true);        
        bool isLoaded = cxImage.Decode(&stream, imagetype);
        int32_t frameCount = cxImage.GetNumFrames();
        ASSERT(isLoaded && cxImage.IsValid() && (frameCount > 0));
        if (!isLoaded || !cxImage.IsValid() || (frameCount < 1)) {
            return false;
        }

        nOutFrameCount = (uint32_t)frameCount;
        if (!isIconFile && !bLoadAllFrames) {
            //GIF文件，只加载第1帧，不加载全部帧            
            frameCount = 1;
        }
        if (isIconFile) {
            //目前ICO文件，只加载1帧，未按多帧处理
            nOutFrameCount = 1;
        }

        //ICO
        std::vector<uint32_t> frameNumColors;  //用于记录ICO文件中，每个Frame的颜色数
        std::unique_ptr<CxImage> cxIcoImage;   //每个Frame的ICO文件提取接口
        std::unique_ptr<CxMemFile> cxIcoStream;//每个Frame的ICO文件数据流
        //
        uint32_t lastFrameDelay = 0;
        imageData.resize(frameCount);
        frameNumColors.resize(frameCount);
        for (int32_t index = 0; index < frameCount; ++index) {
            CxImage* cxFrame = nullptr;
            if (imagetype == CXIMAGE_FORMAT_GIF) {
                cxFrame = cxImage.GetFrame(index);
            }
            else {
                cxIcoStream = std::make_unique<CxMemFile>(fileData.data(), (uint32_t)fileData.size());
                cxIcoImage = std::make_unique<CxImage>(imagetype);
                cxIcoImage->SetFrame(index);
                if (cxIcoImage->Decode(cxIcoStream.get(), imagetype) && cxIcoImage->IsValid()) {
                    cxFrame = cxIcoImage.get();
                }
            }
            
            //ASSERT(cxFrame != nullptr);
            if (cxFrame == nullptr) {
                imageData.clear();
                return false;
            }
            uint32_t frameDelay = cxFrame->GetFrameDelay();
            if (frameDelay == 0) {
                frameDelay = lastFrameDelay;
            }
            else {
                lastFrameDelay = frameDelay;
            }
            frameNumColors[index] = cxFrame->GetNumColors();////2, 16, 256; 0 for RGB images.

            uint32_t nWidth = cxFrame->GetWidth();
            uint32_t nHeight = cxFrame->GetHeight();
            ASSERT((nWidth > 0) && (nHeight > 0));
            if ((nWidth == 0) && (nHeight == 0)) {
                imageData.clear();
                return false;
            }

            int32_t lPx = 0;
            int32_t lPy = 0;
            ImageDecoder::ImageData& bitmapData = imageData[index];
            bitmapData.m_bitmapData.resize((size_t)nHeight * nWidth * 4);
            RGBQUAD* pBit = (RGBQUAD*)bitmapData.m_bitmapData.data();
            for (lPy = 0; lPy < (int32_t)nHeight; ++lPy) {
                for (lPx = 0; lPx < (int32_t)nWidth; ++lPx) {
                    *pBit = cxFrame->GetPixelColor(lPx, lPy, true);
                    if (!cxFrame->AlphaIsValid() && !cxFrame->IsTransparent() && !cxFrame->AlphaPaletteIsEnabled()) {
                        //如果不含有Alpha通道，则填充A值为固定值
                        pBit->rgbReserved = 255;
                    }
                    else {
                        //图片含有Alpha通道
                        uint8_t a = pBit->rgbReserved;
                        if (!cxFrame->AlphaIsValid()) {
                            a = 255;
                        }

                        int32_t transIndex = cxFrame->GetTransIndex();//Gets the index used for transparency. Returns -1 for no transparancy.
                        int32_t bitCount = cxFrame->GetBpp();//1, 4, 8, 24.
                        int32_t numColors = cxFrame->GetNumColors();//2, 16, 256; 0 for RGB images.
                        if ((transIndex >= 0) && (bitCount < 24) && (numColors != 0) && (cxFrame->GetDIB() != nullptr)) {
                            RGBQUAD transColor = cxFrame->GetTransColor();
                            if ((transColor.rgbRed == pBit->rgbRed) &&
                                (transColor.rgbGreen == pBit->rgbGreen) &&
                                (transColor.rgbBlue == pBit->rgbBlue)) {
                                //透明色，标记Alpha通道为全透明
                                a = 0;
                            }
                        }                                                                
                        pBit->rgbReserved = a;

                        if ((a > 0) && (a < 255)) {
                            pBit->rgbRed = pBit->rgbRed * a / 255;
                            pBit->rgbGreen = pBit->rgbGreen * a / 255;
                            pBit->rgbBlue = pBit->rgbBlue * a / 255;
                        }
                    }
#ifdef DUILIB_BUILD_FOR_WIN
                    //数据格式：Window平台BGRA，其他平台RGBA
#else
                    //其他平台，交换R和B值
                    uint8_t r = pBit->rgbRed;
                    pBit->rgbRed = pBit->rgbBlue;
                    pBit->rgbBlue = r;
#endif
                    ++pBit;
                }
            }
            bitmapData.m_frameInterval = frameDelay * 10;
            bitmapData.m_imageWidth = nWidth;
            bitmapData.m_imageHeight = nHeight;
            bitmapData.bFlipHeight = false;
        }

        if (isIconFile) {
            //目前只支持加载一个ICO文件，后续再根据实际应用场景扩展(优先选择32位真彩的图片，然后选择256色的，然再选择16色的)
            bool isIconSizeValid = false;
            const size_t imageCount = imageData.size();
            for (size_t i = 0; i < imageCount; ++i) {
                const ImageDecoder::ImageData& icoData = imageData[i];
                if (icoData.m_imageWidth == iconSize) {
                    isIconSizeValid = true;
                    break;
                }
            }

            std::vector<uint32_t> colors = {0, 256, 16, 2};
            bool isFound = false;
            for (auto color : colors) {
                for (size_t i = 0; i < imageCount; ++i) {
                    const ImageDecoder::ImageData& icoData = imageData[i];
                    uint32_t numColors = frameNumColors[i];
                    if ((!isIconSizeValid || (icoData.m_imageWidth == iconSize)) && (numColors == color)) {
                        ImageDecoder::ImageData oneData = icoData;
                        imageData.resize(1);
                        imageData[0] = oneData;
                        isFound = true;
                        break;
                    }
                }
                if (isFound) {
                    break;
                }
            }
            if (imageData.size() > 1) {
                ImageDecoder::ImageData oneData = imageData.front();
                imageData.resize(1);
                imageData[0] = oneData;
            }
        }
        return !imageData.empty();
    }
}//CxImageLoader

/** 使用libWebP加载图片
*/
namespace WebPImageLoader
{
    bool LoadImageFromMemory(std::vector<uint8_t>& fileData, std::vector<ImageDecoder::ImageData>& imageData,
                             bool bLoadAllFrames, uint32_t& nOutFrameCount, int32_t& playCount)
    {
        ASSERT(!fileData.empty());
        if (fileData.empty()) {
            return false;
        }
        imageData.clear();
        playCount = 0;
        WebPData wd = { fileData.data() , fileData.size() };
        WebPDemuxer* demuxer = WebPDemux(&wd);
        if (demuxer == nullptr) {
            return false;
        }
        //uint32_t flags = WebPDemuxGetI(demuxer, WEBP_FF_FORMAT_FLAGS);
        //bool hasAnimation = flags & ANIMATION_FLAG;
        uint32_t loopCount = WebPDemuxGetI(demuxer, WEBP_FF_LOOP_COUNT);
        //uint32_t backGroundColor = WebPDemuxGetI(demuxer, WEBP_FF_BACKGROUND_COLOR);
        uint32_t frameCount = WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT);
        if (frameCount == 0) {
            return false;
        }
        nOutFrameCount = frameCount;
        if (!bLoadAllFrames) {
            //只加载第1帧，不加载全部帧
            frameCount = 1;
        }

        imageData.resize(frameCount);

        // libwebp's index start with 1
        for (int frame_idx = 1; frame_idx <= (int)frameCount; ++frame_idx) {
            WebPIterator iter;
            int ret = WebPDemuxGetFrame(demuxer, frame_idx, &iter);
            ASSERT(ret != 0);
            if (ret == 0) {
                imageData.clear();
                WebPDemuxReleaseIterator(&iter);
                break;
            }
            int width = 0;
            int hight = 0;
            uint8_t* decode_data = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
            //数据格式：Window平台BGRA，其他平台RGBA
            decode_data = WebPDecodeBGRA(iter.fragment.bytes, iter.fragment.size, &width, &hight);
#else
            decode_data = WebPDecodeRGBA(iter.fragment.bytes, iter.fragment.size, &width, &hight);
#endif
            
            ASSERT((decode_data != nullptr) && (width > 0) && (hight > 0));
            if ((decode_data == nullptr) || (width <= 0) || (hight <= 0)) {
                imageData.clear();
                WebPDemuxReleaseIterator(&iter);
                break;
            }
            ImageDecoder::ImageData& bitmapData = imageData[(size_t)frame_idx - 1];
            const size_t dataSize = (size_t)width * hight * 4;
            bitmapData.m_bitmapData.resize(dataSize);
            ::memcpy(bitmapData.m_bitmapData.data(), decode_data, dataSize);
            bitmapData.m_imageWidth = width;
            bitmapData.m_imageHeight = hight;
            bitmapData.m_frameInterval = iter.duration;

            WebPDemuxReleaseIterator(&iter);
        }
        WebPDemuxDelete(demuxer);
        playCount = (int32_t)loopCount;
        return !imageData.empty();
    }
}

ImageDecoder::ImageFormat ImageDecoder::GetImageFormat(const DString& path)
{
    ImageFormat imageFormat = ImageFormat::kUnknown;
    if (path.size() < 4) {
        return imageFormat;
    }
    DString fileExt;
    size_t pos = path.rfind(_T("."));
    if ((pos != DString::npos) && ((pos + 1) < path.size())) {
        fileExt = path.substr(pos + 1, DString::npos);
        fileExt = StringUtil::MakeUpperString(fileExt);
    }
    if (fileExt == _T("PNG")) {
        imageFormat = ImageFormat::kPNG;
    }
    else if (fileExt == _T("SVG")) {
        imageFormat = ImageFormat::kSVG;
    }
    else if (fileExt == _T("GIF")) {
        imageFormat = ImageFormat::kGIF;
    }
    else if (fileExt == _T("WEBP")) {
        imageFormat = ImageFormat::kWEBP;
    }
    else if ((fileExt == _T("JPEG")) || (fileExt == _T("JPG"))) {
        imageFormat = ImageFormat::kJPEG;
    }
    else if (fileExt == _T("BMP")) {
        imageFormat = ImageFormat::kBMP;
    }
    else if (fileExt == _T("ICO")) {
        imageFormat = ImageFormat::kICO;
    }
    else if (fileExt == _T("CUR")) {
        imageFormat = ImageFormat::kICO;
    }
    return imageFormat;
}

std::unique_ptr<ImageInfo> ImageDecoder::LoadImageData(std::vector<uint8_t>& fileData,                                                       
                                                       const ImageLoadAttribute& imageLoadAttribute,                                                       
                                                       bool bEnableDpiScale, uint32_t nImageDpiScale, uint32_t nWindowDpiScale,
                                                       bool bLoadAllFrames, uint32_t& nFrameCount)
{
    ASSERT(!fileData.empty() && imageLoadAttribute.HasImageFullPath());
    if (fileData.empty() || !imageLoadAttribute.HasImageFullPath()) {
        return nullptr;
    }
    IRenderFactory* pRenderFactroy = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactroy != nullptr);
    if (pRenderFactroy == nullptr) {
        return nullptr;
    }

    std::vector<ImageData> imageData;
    bool bDpiScaled = false; //是否根据DPI做过按比例缩放操作
    int32_t playCount = -1;

    PerformanceUtil::Instance().BeginStat(_T("DecodeImageData"));
    bool isLoaded = DecodeImageData(fileData, imageLoadAttribute, 
                                    bLoadAllFrames, bEnableDpiScale, nImageDpiScale, nWindowDpiScale,
                                    imageData, nFrameCount, playCount, bDpiScaled);
    PerformanceUtil::Instance().EndStat(_T("DecodeImageData"));
    if (!isLoaded || imageData.empty()) {
        return nullptr;
    }

    ImageFormat imageFormat = GetImageFormat(imageLoadAttribute.GetImageFullPath());
    if (imageFormat != ImageFormat::kSVG) {
        //加载图片的时候，应该未做过DPI自适应
        ASSERT(!bDpiScaled);
        //计算缩放后的大小
        const ImageData& image = imageData[0];
        uint32_t nImageWidth = image.m_imageWidth;
        uint32_t nImageHeight = image.m_imageHeight;
        ImageLoader::CalcImageLoadSize(imageLoadAttribute,
                                       bEnableDpiScale, nImageDpiScale, nWindowDpiScale, bDpiScaled,
                                       nImageWidth, nImageHeight);
        if ((nImageWidth != image.m_imageWidth) ||
            (nImageHeight != image.m_imageHeight)) {
            //加载图像后，根据配置属性，进行大小调整(用算法对原图缩放，图片质量显示效果会好些)
            PerformanceUtil::Instance().BeginStat(_T("ResizeImageData"));
            if (!ResizeImageData(imageData, nImageWidth, nImageHeight)) {
                bDpiScaled = false;
            }
            PerformanceUtil::Instance().EndStat(_T("ResizeImageData"));
        }
    }

    std::unique_ptr<ImageInfo> imageInfo(new ImageInfo);
    std::vector<IBitmap*> frameBitmaps;
    std::vector<int> frameIntervals;
    uint32_t imageWidth = 0;
    uint32_t imageHeight = 0;
    for (const ImageData& bitmapData : imageData) {
        ASSERT(bitmapData.m_bitmapData.size() == ((size_t)bitmapData.m_imageWidth * bitmapData.m_imageHeight * 4));
        if (bitmapData.m_bitmapData.size() != ((size_t)bitmapData.m_imageWidth * bitmapData.m_imageHeight * 4)) {
            return nullptr;
        }
        frameIntervals.push_back(bitmapData.m_frameInterval);
        if (imageWidth == 0) {
            imageWidth = bitmapData.m_imageWidth;
        }
        if (imageHeight == 0) {
            imageHeight = bitmapData.m_imageHeight;
        }

        IBitmap* pBitmap = pRenderFactroy->CreateBitmap();
        ASSERT(pBitmap != nullptr);
        if (pBitmap == nullptr) {
            return nullptr;
        }
        pBitmap->Init(bitmapData.m_imageWidth, bitmapData.m_imageHeight, bitmapData.bFlipHeight, bitmapData.m_bitmapData.data());
        frameBitmaps.push_back(pBitmap);
    }
    imageInfo->SetFrameBitmap(frameBitmaps);
    if (frameIntervals.size() > 1) {
        imageInfo->SetFrameInterval(frameIntervals);
    }
    //多帧图片时，以第一帧图片作为图片的大小信息
    imageInfo->SetImageSize(imageWidth, imageHeight);
    imageInfo->SetPlayCount(playCount);
    imageInfo->SetBitmapSizeDpiScaled(bDpiScaled);
    return imageInfo;
}

bool ImageDecoder::ResizeImageData(std::vector<ImageData>& imageData,
                                   uint32_t nNewWidth,
                                   uint32_t nNewHeight)
{
    ASSERT((nNewWidth > 0) && (nNewHeight > 0));
    if ((nNewWidth == 0) || (nNewHeight == 0)) {
        return false;
    }
    bool isSizeChanged = false;
    for (const ImageData& image : imageData) {
        if ((image.m_imageWidth != nNewWidth) || (image.m_imageHeight != nNewHeight)) {
            isSizeChanged = true;
            break;
        }
    }
    if (!isSizeChanged) {
        //图片大小未发生变化
        return false;
    }
    //可能会出现部分失败的情况(忽略这种情况)
    bool hasError = false;
    for (ImageData& image : imageData) {
        std::vector<uint8_t> resizedBitmapData;
        if ((image.m_imageWidth == nNewWidth) && (image.m_imageHeight == nNewHeight)) {
            //图片大小未发生变化
            continue;
        }
        resizedBitmapData.resize((size_t)nNewWidth * nNewHeight * 4);
        const unsigned char* input_pixels = image.m_bitmapData.data();
        int input_w = image.m_imageWidth;
        int input_h = image.m_imageHeight;
        int input_stride_in_bytes = 0;
        unsigned char* output_pixels = resizedBitmapData.data();
        int output_w = nNewWidth;
        int output_h = nNewHeight;
        int output_stride_in_bytes = 0;
        stbir_pixel_layout num_channels = STBIR_RGBA;
        unsigned char* result = stbir_resize_uint8_linear(input_pixels, input_w, input_h, input_stride_in_bytes,
                                                          output_pixels, output_w, output_h, output_stride_in_bytes,
                                                          num_channels);
        if (result != nullptr) {
            image.m_bitmapData.swap(resizedBitmapData);
            image.m_imageWidth = nNewWidth;
            image.m_imageHeight = nNewHeight;
        }
        else {
            //失败了
            hasError = true;
        }
    }
    return !hasError;
}

bool ImageDecoder::DecodeImageData(std::vector<uint8_t>& fileData,
                                   const ImageLoadAttribute& imageLoadAttribute,
                                   bool bLoadAllFrames,
                                   bool bEnableDpiScale,
                                   uint32_t nImageDpiScale,
                                   uint32_t nWindowDpiScale,
                                   std::vector<ImageData>& imageData,
                                   uint32_t& nFrameCount,
                                   int32_t& playCount,
                                   bool& bDpiScaled)
{
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return false;
    }

    imageData.clear();
    nFrameCount = 1;
    playCount = -1;    
    bDpiScaled = false;

    bool isLoaded = false;
    ImageFormat imageFormat = GetImageFormat(imageLoadAttribute.GetImageFullPath());
    switch (imageFormat) {
    case ImageFormat::kPNG:
        isLoaded = APNGImageLoader::LoadImageFromMemory(fileData, imageData, bLoadAllFrames, nFrameCount, playCount);
        break;
    case ImageFormat::kSVG:
        //SVG是矢量图，所以需要在加载过程中处理图片缩放，确保图片的质量是最高的
        imageData.resize(1);
        isLoaded = SkiaSvgImageLoader::LoadImageFromMemory(fileData, imageLoadAttribute,
                                                           bEnableDpiScale, nImageDpiScale, nWindowDpiScale,
                                                           imageData[0], bDpiScaled);
        break;
    case ImageFormat::kJPEG:
    case ImageFormat::kBMP:
        imageData.resize(1);
        isLoaded = STBImageLoader::LoadImageFromMemory(fileData, imageData[0]);
        break;    
    case ImageFormat::kGIF:
        isLoaded = CxImageLoader::LoadImageFromMemory(fileData, imageData, false, 0, bLoadAllFrames, nFrameCount);
        break;
    case ImageFormat::kICO:
        //加载的时候，可用指定加载的ICO图片大小（因一个ICO文件中，可包含各种大小的图片）
        isLoaded = CxImageLoader::LoadImageFromMemory(fileData, imageData, 
                                                      true, imageLoadAttribute.GetIconSize(), bLoadAllFrames, nFrameCount);
        break;
    case ImageFormat::kWEBP:
        isLoaded = WebPImageLoader::LoadImageFromMemory(fileData, imageData, bLoadAllFrames, nFrameCount, playCount);
        break;
    
    default:
        break;
    }
    if (!isLoaded) {
        nFrameCount = 0;
    }
    return isLoaded;
}

} // namespace ui

