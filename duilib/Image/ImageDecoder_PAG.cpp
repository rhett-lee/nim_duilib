#include "ImageDecoder_PAG.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_PAG.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"

#ifdef DUILIB_IMAGE_SUPPORT_LIB_PAG

namespace ui
{
ImageDecoder_PAG::ImageDecoder_PAG()
{
}

ImageDecoder_PAG::~ImageDecoder_PAG()
{
}

DString ImageDecoder_PAG::GetFormatName() const
{
    return _T("PAG");
}

bool ImageDecoder_PAG::CanDecode(const DString& imageFilePath) const
{
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt == _T("PAG")) {
        return true;
    }
    return false;
}

bool ImageDecoder_PAG::CanDecode(const uint8_t* data, size_t dataLen) const
{
    //PAG格式，无数据签名
    return (data != nullptr) && (dataLen > 0);
}

std::unique_ptr<IImage> ImageDecoder_PAG::LoadImageData(const ImageDecodeParam& decodeParam)
{
    if ((decodeParam.m_pFileData == nullptr) || decodeParam.m_pFileData->empty()) {
        return nullptr;
    }
    std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
    const DString& filePath = decodeParam.m_imagePath;
    bool bLoadAllFrames = decodeParam.m_bLoadAllFrames;
    float fPagMaxFrameRate = decodeParam.m_fPagMaxFrameRate;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    const std::string& pagFilePwd = decodeParam.m_pagFilePwd;
    const UiSize& rcMaxDestRectSize = decodeParam.m_rcMaxDestRectSize;
    Image_PAG* pImagePAG = new Image_PAG;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImagePAG);
    if (!pImagePAG->LoadImageFromMemory(filePath,
                                        fileData,
                                        bLoadAllFrames,
                                        fPagMaxFrameRate,
                                        fImageSizeScale,
                                        pagFilePwd,
                                        rcMaxDestRectSize)) {
        //ASSERT(0);
        return nullptr;
    }
    if (!bLoadAllFrames || (pImagePAG->GetFrameCount() == 1)) {
        //单帧，加载位图图片
        return Image_Bitmap::MakeImage(pAnimationImage);
    }
    else {
        //多帧图片
        std::unique_ptr<IImage> pImage(new Image_Animation(pAnimationImage));
        return pImage;
    }
}

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_LIB_PAG
