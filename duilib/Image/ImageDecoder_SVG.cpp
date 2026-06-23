#include "ImageDecoder_SVG.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Image/Image_Svg.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
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
        int width = (int)std::ceil(svg->width);
        int height = (int)std::ceil(svg->height);
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
    * @param [in] svgReplaceColorCallback SVG格式替换颜色实现的回调函数
    */
    virtual std::shared_ptr<IBitmap> GetBitmap(const UiSize& szImageSize, SvgReplaceColorCallbackFunction svgReplaceColorCallback) override
    {
        const uint32_t nImageWidth = szImageSize.cx > 0 ? (uint32_t)szImageSize.cx : m_nImageWidth;
        const uint32_t nImageHeight = szImageSize.cy > 0 ? (uint32_t)szImageSize.cy : m_nImageHeight;
        ASSERT((nImageWidth > 0) && (nImageHeight > 0));
        if ((nImageWidth <= 0) || (nImageHeight <= 0)) {
            return nullptr;
        }

        //检查颜色值是否变化，如果变化则需要重新生成图片
        if (!m_svgReplaceTextList.empty() && !m_svgText.empty()) {
            SvgReplaceColorCallbackFunction replaceColorCallback = svgReplaceColorCallback;
            if (replaceColorCallback == nullptr) {
                replaceColorCallback = m_svgReplaceColorCallback;
            }
            if (CheckReplacedSvgColorChanged(replaceColorCallback, m_svgReplaceTextList)) {
                DStringA svgText = SvgImageImpl::GetReplacedSvgText(m_svgText, m_svgReplaceTextList, nullptr);
                std::unique_ptr<SkMemoryStream> spMemStream = SkMemoryStream::MakeCopy(svgText.data(), svgText.size());
                uint32_t nLoadedImageWidth = 0;
                uint32_t nLoadedImageHeight = 0;
                float fImageSizeScale = m_fImageSizeScale;
                sk_sp<SkSVGDOM> svgDom = SvgImageImpl::LoadSvgImage(spMemStream.get(), fImageSizeScale, false,
                                                                    nLoadedImageWidth, nLoadedImageHeight);
                ASSERT(svgDom != nullptr);
                ASSERT(nLoadedImageWidth == m_nImageWidth);
                ASSERT(nLoadedImageHeight == m_nImageHeight);
                if ((svgDom != nullptr) && (nLoadedImageWidth == m_nImageWidth) && (nLoadedImageHeight == m_nImageHeight)) {
                    //重现加载成功
                    m_svgDom = svgDom;
                    m_pBitmap.reset();
                }
            }
        }

        if ((m_pBitmap != nullptr) &&
            (m_pBitmap->GetWidth() == nImageWidth) &&
            (m_pBitmap->GetHeight() == nImageHeight)) {
            //使用缓存位图
            return m_pBitmap;
        }

        ASSERT(m_svgDom != nullptr);
        if (m_svgDom == nullptr) {
            return nullptr;
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
    //SVG源码中需要替换的内容（颜色等）
    struct SvgReplaceText
    {
        DStringA srcText;   //源字符串
        DStringA destText;  //目标字符串
        bool bColor;        //是否为颜色值
        DString colorName;  //颜色名称（当bColor时使用）
        UiColor colorValue; //颜色值（当bColor时使用）
    };

    //生成替换文本的列表
    static void CreateSvgReplaceTextList(SvgReplaceColorCallbackFunction svgReplaceColorCallback,
                                         const DString& strSvgReplaceColor,
                                         std::vector<SvgImageImpl::SvgReplaceText>& svgReplaceTextList)
    {
        svgReplaceTextList.clear();
        if (strSvgReplaceColor.empty() || (svgReplaceColorCallback == nullptr)) {
            return;
        }
        DStringA svgReplaceColor = StringConvert::TToUTF8(strSvgReplaceColor);
        std::list<DStringA> colorsList = StringUtil::Split(svgReplaceColor, ";");
        for (DStringA colorPair : colorsList) {
            StringUtil::Trim(colorPair);
            std::list<DStringA> colors = StringUtil::Split(colorPair, "|");
            if (colors.size() == 2) {
                DStringA srcColor = *colors.begin();
                DStringA destColor = *colors.rbegin();

                StringUtil::Trim(srcColor);
                StringUtil::Trim(destColor);

                ASSERT(!srcColor.empty() && !destColor.empty());
                if (!srcColor.empty() && !destColor.empty()) {
                    DString colorName = StringConvert::UTF8ToT(destColor);
                    UiColor colorValue = svgReplaceColorCallback(colorName);
                    if (!colorValue.IsEmpty()) {
                        //颜色值
                        SvgImageImpl::SvgReplaceText replaceText;
                        replaceText.srcText = srcColor;
                        replaceText.destText = destColor;
                        replaceText.bColor = true;
                        replaceText.colorName = colorName;
                        replaceText.colorValue = colorValue;
                        svgReplaceTextList.emplace_back(std::move(replaceText));
                    }
                    else {
                        //非颜色值
                        SvgImageImpl::SvgReplaceText replaceText;
                        replaceText.srcText = srcColor;
                        replaceText.destText = destColor;
                        replaceText.bColor = false;
                        svgReplaceTextList.emplace_back(std::move(replaceText));
                    }
                }
            }
        }
    }

    //生成替换文本（颜色）后的svg源码文本
    static DStringA GetReplacedSvgText(DStringA svgText, std::vector<SvgReplaceText>& svgReplaceTextList, bool* pExecReplaced)
    {
        for (const SvgReplaceText& replaceText : svgReplaceTextList) {
            if (replaceText.bColor) {
                //颜色值：替换为目标颜色值, 格式为：rgba(r,g,b,a)，举例：fill="rgba(181,181,181,0.5)"
                DStringA rgbaColor = StringUtil::Printf("rgba(%d,%d,%d,%.02f)",
                                                        (int32_t)replaceText.colorValue.GetR(),
                                                        (int32_t)replaceText.colorValue.GetG(),
                                                        (int32_t)replaceText.colorValue.GetB(),
                                                        (float)replaceText.colorValue.GetA() / 255.0f);
                if (replaceText.srcText != rgbaColor) {
                    StringUtil::ReplaceAll(replaceText.srcText, rgbaColor, svgText);
                    if (pExecReplaced) {
                        *pExecReplaced = true;
                    }
                }                
            }
            else {
                //非颜色值：直接替换
                if (replaceText.srcText != replaceText.destText) {
                    StringUtil::ReplaceAll(replaceText.srcText, replaceText.destText, svgText);
                    if (pExecReplaced) {
                        *pExecReplaced = true;
                    }
                }                
            }
        }
        return svgText;
    }

    /** 检测被替换的颜色值是否变化，如果变化则需要重新加载svg图片
    * @return 变化返回true，无变化返回false
    */
    static bool CheckReplacedSvgColorChanged(SvgReplaceColorCallbackFunction svgReplaceColorCallback,
                                             std::vector<SvgReplaceText>& svgReplaceTextList)
    {
        if (svgReplaceColorCallback == nullptr) {
            return false;
        }
        bool bColorChanged = false;
        for (SvgReplaceText& replaceText : svgReplaceTextList) {
            if (replaceText.bColor) {
                DString colorName = StringConvert::UTF8ToT(replaceText.destText);
                UiColor colorValue = svgReplaceColorCallback(colorName);
                if (!colorValue.IsEmpty() && colorValue != replaceText.colorValue) {
                    //更新颜色值（一般是主题变化时，才会变化）
                    replaceText.colorValue = colorValue;
                    bColorChanged = true;
                    break;
                }
            }
        }
        return bColorChanged;
    }

    /** 加载SVG图片
    */
    static sk_sp<SkSVGDOM> LoadSvgImage(SkMemoryStream* spMemStream, float fImageSizeScale, bool bAssertEnabled,
                                        uint32_t& nImageWidth, uint32_t& nImageHeight)
    {
        ASSERT(spMemStream != nullptr);
        if (spMemStream == nullptr) {
            return nullptr;
        }
        sk_sp<SkSVGDOM> svgDom = SkSVGDOM::MakeFromStream(*spMemStream);
        if (bAssertEnabled) {
            ASSERT(svgDom != nullptr);
        }
        if (svgDom == nullptr) {
            return nullptr;
        }
        if (bAssertEnabled) {
            ASSERT(svgDom->getRoot() != nullptr);
        }
        if (svgDom->getRoot() == nullptr) {
            return nullptr;
        }
        SkSize svgSize = svgDom->getRoot()->intrinsicSize(SkSVGLengthContext(SkSize::Make(0, 0)));
        int32_t nSvgImageWidth = int32_t(std::ceil(svgSize.width()));
        int32_t nSvgImageHeight = int32_t(std::ceil(svgSize.height()));
        if ((nSvgImageWidth < 1) || (nSvgImageHeight < 1)) {
            auto viewBox = svgDom->getRoot()->getViewBox();
            if (viewBox.has_value()) {
                nSvgImageWidth = int32_t(std::ceil(viewBox->width()));
                nSvgImageHeight = int32_t(std::ceil(viewBox->height()));
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
            const void* pData = spMemStream->getMemoryBase();
            const size_t nDataLen = spMemStream->getLength();
            if ((pData != nullptr) && (nDataLen > 0)) {
                std::vector<uint8_t> fileData;
                fileData.resize(nDataLen);
                memcpy(fileData.data(), pData, nDataLen);
                if (!NanoSvgDecoder::ImageSizeFromMemory(fileData, nSvgImageWidth, nSvgImageHeight)) {
                    if (bAssertEnabled) {
                        ASSERT(0);
                    }
                }
            }            
        }
        ASSERT((nSvgImageWidth > 0) && (nSvgImageHeight > 0));
        if ((nSvgImageWidth < 1) || (nSvgImageHeight < 1)) {
            return nullptr;
        }

        //计算缩放后的大小
        nImageWidth = ImageUtil::GetScaledImageSize((uint32_t)nSvgImageWidth, fImageSizeScale);
        nImageHeight = ImageUtil::GetScaledImageSize((uint32_t)nSvgImageHeight, fImageSizeScale);
        ASSERT((nImageHeight > 0) && (nImageHeight > 0));
        if ((nImageWidth < 1) || (nImageHeight < 1)) {
            return nullptr;
        }
        return svgDom;
    }

public:
    //宽度
    uint32_t m_nImageWidth = 0;

    //高度
    uint32_t m_nImageHeight = 0;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;

public:
    //SVG源码文本
    DStringA m_svgText;

    //SVG源码中需要替换的内容（颜色等）
    std::vector<SvgReplaceText> m_svgReplaceTextList;

    //用于替换SVG格式颜色值参数的回调函数
    SvgReplaceColorCallbackFunction m_svgReplaceColorCallback;

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

    //当有需要从SVG中替换的文本时，需要记录SVG源码及替换规则
    std::vector<SvgImageImpl::SvgReplaceText> svgReplaceTextList;
    SvgImageImpl::CreateSvgReplaceTextList(decodeParam.m_svgReplaceColorCallback,
                                           decodeParam.m_svgReplaceColors,
                                           svgReplaceTextList);

    std::unique_ptr<SkMemoryStream> spMemStream;
    if (svgReplaceTextList.empty()) {
        spMemStream = SkMemoryStream::MakeCopy(fileData.data(), fileData.size());
    }
    else {
        //替换文本
        bool bExecReplaced = false;
        DStringA svgText((const DStringA::value_type*)fileData.data(), fileData.size());
        DStringA newSvgText = SvgImageImpl::GetReplacedSvgText(svgText, svgReplaceTextList, &bExecReplaced);
        if (decodeParam.m_bAssertEnabled && bExecReplaced) {
            ASSERT(newSvgText != svgText);
        }
        if (newSvgText == svgText) {
            //没有替换(配置有错误)
            svgReplaceTextList.clear();
        }
        spMemStream = SkMemoryStream::MakeCopy(newSvgText.data(), newSvgText.size());
    }

    uint32_t nImageWidth = 0;
    uint32_t nImageHeight = 0;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    sk_sp<SkSVGDOM> svgDom = SvgImageImpl::LoadSvgImage(spMemStream.get(), fImageSizeScale, decodeParam.m_bAssertEnabled,
                                                        nImageWidth, nImageHeight);
    if (svgDom == nullptr) {
        return nullptr;
    }

    SvgImageImpl* pSvgImageImpl = new SvgImageImpl;
    std::shared_ptr<ISvgImage> pSvgImage(pSvgImageImpl);
    pSvgImageImpl->m_nImageWidth = nImageWidth;
    pSvgImageImpl->m_nImageHeight = nImageHeight;
    pSvgImageImpl->m_fImageSizeScale = fImageSizeScale;
    pSvgImageImpl->m_svgDom = svgDom;

    //保存需要替换的源数据
    pSvgImageImpl->m_svgReplaceTextList.swap(svgReplaceTextList);
    pSvgImageImpl->m_svgReplaceColorCallback = decodeParam.m_svgReplaceColorCallback;
    if (!pSvgImageImpl->m_svgReplaceTextList.empty()) {
        DStringA svgText((const DStringA::value_type*)fileData.data(), fileData.size());
        pSvgImageImpl->m_svgText.swap(svgText);
    }

    std::unique_ptr<IImage> pImage = Image_Svg::MakeImage(pSvgImage);
    return pImage;
}

} //namespace ui
