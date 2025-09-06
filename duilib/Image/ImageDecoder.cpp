#include "ImageDecoder.h"
#include "duilib/Image/Image.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Image/ImageUtil.h"

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

#pragma warning (push)
#pragma warning (disable: 4996)
    #include "duilib/third_party/cximage/ximage.h"
#pragma warning (pop)

#include <set>

namespace ui 
{

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

            imageData.m_frameInterval = 0;
            imageData.m_imageWidth = nWidth;
            imageData.m_imageHeight = nHeight;
        }
        stbi_image_free(rgbaData);
        return !argbData.empty();
    }
}//STBImageLoader

/** 使用cximage加载图片（只支持GIF和ICO）两种格式
* @param [in] isIconFile 如果为true表示是ICO文件，否则为GIF文件
* @param [in] iconSize 需要加载ICO图标的大小，因ICO文件中包含了各种大小的图标，加载的时候，只加载其中一个图标
* @param [in] bLoadAllFrames 对于多帧图片，是否加载全部帧（true加载全部帧，false仅加载第1帧）
* @param [out] nFrameCount 返回图片总的帧数
*/
namespace CxImageLoader
{
    // 从widthList中找到与nIconSize最匹配的图标大小
    uint32_t FindBestIconSize(const std::vector<uint32_t>& widthList, uint32_t nIconSize)
    {
        if (widthList.empty()) {
            // 处理空容器情况，这里返回0作为默认值
            return 0;
        }

        // 检查是否有完全匹配的值
        auto exactMatch = std::find(widthList.begin(), widthList.end(), nIconSize);
        if (exactMatch != widthList.end()) {
            return *exactMatch;
        }

        // 找到第一个大于nIconSize的值
        auto greaterIt = std::upper_bound(widthList.begin(), widthList.end(), nIconSize);

        if (greaterIt != widthList.end()) {
            // 如果存在大于目标值的元素，比较它与前一个元素哪个更接近
            if (greaterIt == widthList.begin()) {
                return *greaterIt;
            }

            uint32_t prev = *(greaterIt - 1);
            uint32_t curr = *greaterIt;

            // 优先选择较大的值，如果两者距离相等
            if (nIconSize - prev < curr - nIconSize) {
                return prev;
            }
            else {
                return curr;
            }
        }
        else {
            // 所有元素都小于目标值，返回最大的元素
            return widthList.back();
        }
    }

    bool LoadImageFromMemory(const std::vector<uint8_t>& fileData,
                             bool bLoadAllFrames,
                             uint32_t iconSize,
                             std::vector<ImageDecoder::ImageData>& imageData)
    {
        ASSERT(!fileData.empty());
        if (fileData.empty()) {
            return false;
        }
        uint32_t imagetype = CXIMAGE_FORMAT_ICO;
        CxMemFile stream((uint8_t*)fileData.data(), (uint32_t)fileData.size());
        CxImage cxImage(imagetype);
        cxImage.SetRetreiveAllFrames(true);
        bool isLoaded = cxImage.Decode(&stream, imagetype);
        int32_t frameCount = cxImage.GetNumFrames();
        ASSERT(isLoaded && cxImage.IsValid() && (frameCount > 0));
        if (!isLoaded || !cxImage.IsValid() || (frameCount < 1)) {
            return false;
        }

        //ICO
        std::vector<uint32_t> frameNumColors;  //用于记录ICO文件中，每个Frame的颜色数
        std::unique_ptr<CxImage> cxIcoImage;   //每个Frame的ICO文件提取接口
        std::unique_ptr<CxMemFile> cxIcoStream;//每个Frame的ICO文件数据流
        //
        uint32_t lastFrameDelay = 0;
        imageData.clear();
        imageData.resize(frameCount);
        frameNumColors.resize(frameCount);
        for (int32_t index = 0; index < frameCount; ++index) {
            CxImage* cxFrame = nullptr;
            cxIcoStream = std::make_unique<CxMemFile>((uint8_t*)fileData.data(), (uint32_t)fileData.size());
            cxIcoImage = std::make_unique<CxImage>(imagetype);
            cxIcoImage->SetFrame(index);
            if (cxIcoImage->Decode(cxIcoStream.get(), imagetype) && cxIcoImage->IsValid()) {
                cxFrame = cxIcoImage.get();
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
            if ((nWidth == 0) || (nHeight == 0)) {
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
        }

        if (!bLoadAllFrames && !imageData.empty()) {
            //只加载一个ICO图片，后续再根据实际应用场景扩展(优先选择32位真彩的图片，然后选择256色的，然再选择16色的)
            std::set<uint32_t> widthSet;
            const size_t imageCount = imageData.size();
            for (size_t i = 0; i < imageCount; ++i) {
                const ImageDecoder::ImageData& icoData = imageData[i];
                widthSet.insert(icoData.m_imageWidth);
            }
            std::vector<uint32_t> widthList;
            for (auto v : widthSet) {
                widthList.push_back(v);
            }
            uint32_t nBestIconSize = FindBestIconSize(widthList, iconSize);
            if (nBestIconSize == 0) {
                nBestIconSize = iconSize;
            }
            bool isIconSizeValid = false;
            for (size_t i = 0; i < imageCount; ++i) {
                const ImageDecoder::ImageData& icoData = imageData[i];
                if (icoData.m_imageWidth == nBestIconSize) {
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
                    if ((!isIconSizeValid || (icoData.m_imageWidth == nBestIconSize)) && (numColors == color)) {
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

        for (ImageDecoder::ImageData& bitmapData : imageData) {
            //CxImage加载的数据，需要翻转，以屏幕左上角为顶点
            if (!bitmapData.m_bitmapData.empty()) {
                ImageUtil::FlipPixelBits(bitmapData.m_bitmapData.data(), bitmapData.m_bitmapData.size(), bitmapData.m_imageWidth, bitmapData.m_imageHeight);
            }
        }
        return !imageData.empty();
    }
}//CxImageLoader

} // namespace ui

