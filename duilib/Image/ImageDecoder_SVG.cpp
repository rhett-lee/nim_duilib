#include "ImageDecoder_SVG.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Image/Image_Svg.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Core/GlobalManager.h"
#include <cmath>

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

#include "duilib/RenderSkia/SkiaHeaderBegin.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "include/core/SkStream.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "duilib/RenderSkia/SkiaHeaderEnd.h"

namespace ui
{
/** 加载SVG图片(NanoSvg)
*/
namespace NanoSvgDecoder
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
    bool ImageSizeFromMemory(const std::vector<uint8_t>& data, int32_t& nSvgImageWidth, int32_t& nSvgImageHeight)
    {
        std::vector<uint8_t> fileData = data;//此处需要复制数据，因为在解析的过程中，会破坏原来的数据
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
        int width = (int)std::ceilf(svg->width);
        int height = (int)std::ceilf(svg->height);
        if (width <= 0 || height <= 0) {
            return false;
        }
        nSvgImageWidth = width;
        nSvgImageHeight = height;
        return true;
    }
}

/** SVG矢量图片接口的实现
*/
class SvgImageImpl : public ISvgImage
{
public:
    virtual ~SvgImageImpl() override = default;

    /** 获取图片宽度
    */
    virtual uint32_t GetWidth() const override { return m_nImageWidth; }

    /** 获取图片高度
    */
    virtual uint32_t GetHeight() const override { return m_nImageHeight; }

    /** 原图加载的宽度和高度缩放比例(1.0f表示无缩放)
    */
    virtual float GetImageSizeScale() const override { return m_fImageSizeScale; }

    /** 获取指定大小的位图，矢量缩放
    * @param [in] szImageSize 代表获取图片的宽度(cx)和高度(cy)
    */
    virtual std::shared_ptr<IBitmap> GetBitmap(const UiSize& szImageSize) override
    {
        const uint32_t nImageWidth = szImageSize.cx > 0 ? (uint32_t)szImageSize.cx : m_nImageWidth;
        const uint32_t nImageHeight = szImageSize.cy > 0 ? (uint32_t)szImageSize.cy : m_nImageHeight;
        ASSERT((nImageWidth > 0) && (nImageHeight > 0));
        if ((nImageWidth <= 0) || (nImageHeight <= 0)) {
            return nullptr;
        }

        if ((m_pBitmap != nullptr) &&
            (m_pBitmap->GetWidth() == nImageWidth) &&
            (m_pBitmap->GetHeight() == nImageHeight)) {
            //使用缓存位图
            return m_pBitmap;
        }

        //生成位图，矢量缩放
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory == nullptr) {
            return nullptr;
        }
        std::shared_ptr<IBitmap> pBitmap(pRenderFactory->CreateBitmap());
        ASSERT(pBitmap != nullptr);
        if (pBitmap == nullptr) {
            return nullptr;
        }

        SkBitmap skBitmap;
#ifdef DUILIB_BUILD_FOR_WIN
        SkImageInfo info = SkImageInfo::Make((int32_t)nImageWidth, (int32_t)nImageHeight, SkColorType::kN32_SkColorType, SkAlphaType::kPremul_SkAlphaType);
#else
        SkImageInfo info = SkImageInfo::Make((int32_t)nImageWidth, (int32_t)nImageHeight, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
#endif
        skBitmap.allocPixels(info);
        SkCanvas canvas(skBitmap);

        //设置容器大小与图片大小一致(图片大小为DPI缩放后的大小)
        m_svgDom->getRoot()->setWidth(SkSVGLength((SkScalar)nImageWidth, SkSVGLength::Unit::kPX));
        m_svgDom->getRoot()->setHeight(SkSVGLength((SkScalar)nImageHeight, SkSVGLength::Unit::kPX));
        m_svgDom->setContainerSize(SkSize::Make(SkISize::Make((int32_t)nImageWidth, (int32_t)nImageHeight)));

        //绘制到位图
        m_svgDom->render(&canvas);
        if (!pBitmap->Init(nImageWidth, nImageHeight, skBitmap.getPixels())) {
            pBitmap.reset();
        }
        //记录缓存位图，避免每次都重新生成位图
        m_pBitmap = pBitmap;
        return pBitmap;
    }

public:
    //宽度
    uint32_t m_nImageWidth = 0;

    //高度
    uint32_t m_nImageHeight = 0;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;

    //内存数据流
    std::unique_ptr<SkMemoryStream> m_spMemStream;

    //SVG DOM
    sk_sp<SkSVGDOM> m_svgDom;

private:
    //缓存位图
    std::shared_ptr<IBitmap> m_pBitmap;
};

ImageDecoder_SVG::ImageDecoder_SVG()
{
}

ImageDecoder_SVG::~ImageDecoder_SVG()
{
}

DString ImageDecoder_SVG::GetFormatName() const
{
    return _T("SVG");
}

bool ImageDecoder_SVG::CanDecode(const DString& imageFilePath) const
{
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt == _T("SVG")) {
        return true;
    }
    return false;
}

bool ImageDecoder_SVG::CanDecode(const uint8_t* data, size_t dataLen) const
{
    // SVG是XML格式，检查开头是否为<?xml或<svg
    if ((data == nullptr) || (dataLen < 8)) {
        return false;
    }

    // 转换为字符串进行检查
    std::string headerStr(reinterpret_cast<const char*>(data), dataLen);

    // 检查常见的SVG开头
    return (headerStr.substr(0, 5) == "<?xml") || (headerStr.substr(0, 4) == "<svg");
}

std::unique_ptr<IImage> ImageDecoder_SVG::LoadImageData(const ImageDecodeParam& decodeParam)
{
    std::vector<uint8_t> fileData;
    if ((decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty()) {
        fileData = *decodeParam.m_pFileData;
    }
    else if (!decodeParam.m_imageFilePath.IsEmpty()){
        FileUtil::ReadFileData(decodeParam.m_imageFilePath, fileData);
        if (decodeParam.m_bAssertEnabled) {
            ASSERT(!fileData.empty());
        }        
        if (fileData.empty()) {
            return nullptr;
        }
    }
    else {
        ASSERT(0);
        return nullptr;
    }
    std::unique_ptr<SkMemoryStream> spMemStream = SkMemoryStream::MakeCopy(fileData.data(), fileData.size());
    ASSERT(spMemStream != nullptr);
    if (spMemStream == nullptr) {
        return nullptr;
    }
    sk_sp<SkSVGDOM> svgDom = SkSVGDOM::MakeFromStream(*spMemStream);
    if (decodeParam.m_bAssertEnabled) {
        ASSERT(svgDom != nullptr);
    }
    if (svgDom == nullptr) {
        return nullptr;
    }
    if (decodeParam.m_bAssertEnabled) {
        ASSERT(svgDom->getRoot() != nullptr);
    }
    if (svgDom->getRoot() == nullptr) {
        return nullptr;
    }
    spMemStream.reset();

    SkSize svgSize = svgDom->getRoot()->intrinsicSize(SkSVGLengthContext(SkSize::Make(0, 0)));    
    int32_t nSvgImageWidth = int32_t(std::ceilf(svgSize.width()));
    int32_t nSvgImageHeight = int32_t(std::ceilf(svgSize.height()));
    if ((nSvgImageWidth < 1) || (nSvgImageHeight < 1)) {
        auto viewBox = svgDom->getRoot()->getViewBox();
        if (viewBox.has_value()) {
            nSvgImageWidth = int32_t(std::ceilf(viewBox->width()));
            nSvgImageHeight = int32_t(std::ceilf(viewBox->height()));
        }
    }
    else {
        //如果viewBox不存在，则设置一个，否则图片缩放时存在异常（此处逻辑保持与NanoSvg保持一致）
        auto viewBox = svgDom->getRoot()->getViewBox();
        if (!viewBox.has_value()) {
            svgDom->getRoot()->setViewBox(SkRect::MakeIWH(nSvgImageWidth, nSvgImageHeight));
        }
    }
    if ((nSvgImageWidth < 1) || (nSvgImageHeight < 1)) {
        //如果图片中没有直接定义宽和高，使用NanoSvg计算图片的宽度和高度（Skia的Svg封装没有提供相关功能）
        if (!NanoSvgDecoder::ImageSizeFromMemory(fileData, nSvgImageWidth, nSvgImageHeight)) {
            if (decodeParam.m_bAssertEnabled) {
                ASSERT(0);
            }
            return nullptr;
        }
    }

    ASSERT((nSvgImageWidth > 0) && (nSvgImageHeight > 0));
    if ((nSvgImageWidth < 1) || (nSvgImageHeight < 1)) {
        return nullptr;
    }

    //计算缩放后的大小
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    uint32_t nImageWidth = ImageUtil::GetScaledImageSize((uint32_t)nSvgImageWidth, fImageSizeScale);
    uint32_t nImageHeight = ImageUtil::GetScaledImageSize((uint32_t)nSvgImageHeight, fImageSizeScale);
    ASSERT((nImageHeight > 0) && (nImageHeight > 0));
    if ((nImageWidth < 1) || (nImageHeight < 1)) {
        return nullptr;
    }

    SvgImageImpl* pSvgImageImpl = new SvgImageImpl;
    std::shared_ptr<ISvgImage> pSvgImage(pSvgImageImpl);    
    pSvgImageImpl->m_nImageWidth = nImageWidth;
    pSvgImageImpl->m_nImageHeight = nImageHeight;
    pSvgImageImpl->m_fImageSizeScale = fImageSizeScale;
    pSvgImageImpl->m_spMemStream = std::move(spMemStream);
    pSvgImageImpl->m_svgDom = svgDom;

    std::unique_ptr<IImage> pImage = Image_Svg::MakeImage(pSvgImage);
    return pImage;
}

} //namespace ui
