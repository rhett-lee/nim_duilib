#include "ImageDecoder_Icon.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"

namespace ui
{
ImageDecoder_Icon::ImageDecoder_Icon()
{
}

ImageDecoder_Icon::~ImageDecoder_Icon()
{
}

DString ImageDecoder_Icon::GetFormatName() const
{
    return _T("ICON");
}

bool ImageDecoder_Icon::CanDecode(const DString& imageFilePath) const
{
    IconManager& iconManager = GlobalManager::Instance().Icon();
    if (iconManager.IsIconString(imageFilePath)) {
        uint32_t nIconID = iconManager.GetIconID(imageFilePath);
        if (!iconManager.IsImageString(nIconID)) {
            return true;
        }
    }
    return false;
}

bool ImageDecoder_Icon::CanDecode(const uint8_t* /*data*/, size_t /*dataLen*/) const
{
    return false;
}

std::unique_ptr<IImage> ImageDecoder_Icon::LoadImageData(const ImageDecodeParam& decodeParam)
{
    std::unique_ptr<IImage> pImage;
    IconManager& iconManager = GlobalManager::Instance().Icon();
    const DString imageFilePath = decodeParam.m_imageFilePath.NativePath(); //图片文件路径
    if (iconManager.IsIconString(imageFilePath)) {
        uint32_t nIconID = iconManager.GetIconID(imageFilePath);
        if (!iconManager.IsImageString(nIconID)) {
            IconBitmapData bitmapData;
            if (iconManager.GetIconBitmapData(nIconID, bitmapData)) {
                float fImageSizeScale = decodeParam.m_fImageSizeScale;
                pImage = Image_Bitmap::MakeImage(bitmapData.m_nBitmapWidth, bitmapData.m_nBitmapHeight, bitmapData.m_bitmapData.data(), fImageSizeScale);
            }
        }
    }
    return pImage;
}

} //namespace ui
