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

bool ImageDecoder_Icon::CanDecode(const DString& imageFileString, bool& bVirtualFile) const
{
    IconManager& iconManager = GlobalManager::Instance().Icon();
    if (iconManager.IsIconString(imageFileString)) {
        uint32_t nIconID = iconManager.GetIconID(imageFileString);
        if (!iconManager.IsImageString(nIconID)) {
            bVirtualFile = true;
            return true;
        }
    }
    return false;
}

bool ImageDecoder_Icon::CanDecode(const uint8_t* /*data*/, size_t /*dataLen*/) const
{
    return false;
}

std::unique_ptr<IImage> ImageDecoder_Icon::LoadImageData(const DString& imageFileString,
                                                         std::vector<uint8_t>& /*data*/,
                                                         float fImageSizeScale,
                                                         const IImageDecoder::ExtraParam* /*pExtraParam*/)
{
    std::unique_ptr<IImage> pImage;
    IconManager& iconManager = GlobalManager::Instance().Icon();
    if (iconManager.IsIconString(imageFileString)) {
        uint32_t nIconID = iconManager.GetIconID(imageFileString);
        if (!iconManager.IsImageString(nIconID)) {
            IconBitmapData bitmapData;
            if (iconManager.GetIconBitmapData(nIconID, bitmapData)) {
                pImage = Image_Bitmap::MakeImage(bitmapData.m_nBitmapWidth, bitmapData.m_nBitmapHeight, bitmapData.m_bitmapData.data(), fImageSizeScale);
            }
        }
    }
    return pImage;
}

} //namespace ui
