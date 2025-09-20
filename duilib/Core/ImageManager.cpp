#include "ImageManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Image/ImageLoadParam.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FileUtil.h"

namespace ui 
{
ImageManager::ImageManager():
    m_bDpiScaleAllImages(true),
    m_bAutoMatchScaleImage(true),
    m_bImageAsyncLoad(true)
{
}

ImageManager::~ImageManager()
{
}

std::shared_ptr<ImageInfo> ImageManager::GetImage(const ImageLoadParam& loadParam)
{
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    const DString loadKey = loadParam.GetLoadKey(loadParam.GetLoadDpiScale());
    auto iter = m_imageInfoMap.find(loadKey);
    if (iter != m_imageInfoMap.end()) {
        std::shared_ptr<ImageInfo> spImageInfo = iter->second.lock();
        if (spImageInfo != nullptr) {
            //从缓存中，找到有效图片资源，直接返回
            return spImageInfo;
        }
    }

    //重新加载资源
    //计算设置的比例, 影响加载的缩放百分比（通过width='300'或者width='300%'这种形式设置的图片属性）
    uint32_t nImageFixedWidth = 0;
    uint32_t nImageFixedHeight = 0;
    const bool bHasFixedSize = loadParam.GetImageFixedSize(nImageFixedWidth, nImageFixedHeight, true);

    float fImageFixedWidthPercent = 1.0f;
    float fImageFixedHeightPercent = 1.0f;
    const bool bHasFixedPercent = loadParam.GetImageFixedPercent(fImageFixedWidthPercent, fImageFixedHeightPercent, false);

    const ImageLoadPath& imageLoadPath = loadParam.GetImageLoadPath();
    DString imageFullPath = imageLoadPath.m_imageFullPath;              //图片的路径（本地路径或者压缩包内相对路径）
    uint32_t nImageDpiScale = 100;                                      //原始图片，未经DPI缩放时，DPI缩放比例是100
    const bool isUseZip = GlobalManager::Instance().Zip().IsUseZip();   //是否使用Zip压缩包
    const bool bEnableImageDpiScale = IsDpiScaleAllImages() &&          //仅在DPI缩放图片功能开启的情况下，查找对应DPI的图片是否存在
                                      (loadParam.GetDpiScaleOption() != ImageLoadParam::DpiScaleOption::kOff); //图片属性：dpi_scale="false"，只使用原图，不需要缩放
    if (bEnableImageDpiScale && 
        ((imageLoadPath.m_pathType == ImageLoadPathType::kLocalResPath) ||
         (imageLoadPath.m_pathType == ImageLoadPathType::kZipResPath))) {
        //只有在资源目录下的文件，才执行查找适配DPI图片的功能
        DString dpiImageFullPath;
        uint32_t dpiImageDpiScale = nImageDpiScale;
        if (GetDpiScaleImageFullPath(loadParam.GetLoadDpiScale(), isUseZip, imageFullPath, dpiImageFullPath, dpiImageDpiScale)) {
            //标记DPI自适应图片属性，如果路径不同，说明已经选择了对应DPI下的文件            
            ASSERT((dpiImageDpiScale != 0) && !dpiImageFullPath.empty());
            if ((dpiImageDpiScale != 0) && !dpiImageFullPath.empty()) {
                imageFullPath = dpiImageFullPath;
                nImageDpiScale = dpiImageDpiScale;
                ASSERT(!imageFullPath.empty());
            }
        }
    }

    float fImageSizeScale = 1.0f;
    if (bEnableImageDpiScale) {
        //加载的比例（按相对原图来计算，确保各个DPI适配图的显示效果相同）
        //1.如果图片宽高用于评估显示空间的大小：必须按照DPI缩放比来缩放，这样才能在不同DPI下界面显示效果相同
        //2.如果不需要用图片的宽度和高度评估显示空间大小，那么这个加载比例只影响图片显示效果，不影响布局
        fImageSizeScale = static_cast<float>(loadParam.GetLoadDpiScale()) / static_cast<float>(nImageDpiScale);
        if (bHasFixedSize || bHasFixedPercent) {
            //如果设置了图片的width或者height属性，只使用原图，不需要缩放（因为加载后要执行缩放操作）
            fImageSizeScale = 1.0f;
        }
    }

    std::shared_ptr<IImage> spImageData;
    //查询缓存，如果缓存存在，则可共享图片资源，无需重复加载
    const DString imageKey = imageFullPath;
    auto iterImageData = m_imageDataMap.find(imageKey);
    if (iterImageData != m_imageDataMap.end()) {
        spImageData = iterImageData->second.lock();
        if (spImageData != nullptr) {
            if (!ImageUtil::IsSameImageScale(spImageData->GetImageSizeScale(), fImageSizeScale)) {
                //在动态切换DPI后，比例会发生变化，需要重新加载，不可共享原来加载的图片
                m_imageDataMap.erase(iterImageData);
                spImageData.reset();
            }
        }
    }

    if (spImageData == nullptr) {
        //从内存数据加载图片
        ImageDecoderFactory& ImageDecoders = GlobalManager::Instance().ImageDecoders();
        std::vector<uint8_t> fileData;
        if (imageLoadPath.m_pathType != ImageLoadPathType::kVirtualPath) {
            //实体图片文件，必须有图片数据用于解码图片
            FilePath imageFilePath(imageFullPath);
            if (isUseZip && !imageFilePath.IsAbsolutePath()) {
                GlobalManager::Instance().Zip().GetZipData(imageFilePath, fileData);
            }
            else {
                FileUtil::ReadFileData(imageFilePath, fileData);
            }
            ASSERT(!fileData.empty());
            if (fileData.empty()) {
                //加载失败
                return nullptr;
            }
        }
        ImageDecodeParam decodeParam;
        decodeParam.m_imagePath = imageFullPath;//前面的流程，当是本地文件时，已经确保文件存在
        if (!fileData.empty()) {
            decodeParam.m_pFileData = std::make_shared<std::vector<uint8_t>>();
            decodeParam.m_pFileData->swap(fileData);
        }
        decodeParam.m_fImageSizeScale = fImageSizeScale;
        decodeParam.m_bExternalImagePath = (imageLoadPath.m_pathType == ImageLoadPathType::kLocalPath) ? true : false;

        decodeParam.m_bAsyncDecode = loadParam.IsAsyncDecodeEnabled();    //是否支持多线程图片解码 
        decodeParam.m_bIconAsAnimation = loadParam.IsIconAsAnimation();   //ICO格式相关参数
        decodeParam.m_nIconSize = loadParam.GetIconSize();                //ICO格式相关参数
        decodeParam.m_nIconFrameDelayMs = loadParam.GetIconFrameDelayMs();//ICO格式相关参数
        decodeParam.m_fPagMaxFrameRate = loadParam.GetPagMaxFrameRate();  //PAG格式相关参数
        decodeParam.m_bLoadAllFrames = true; //所有多帧图片相关参数

        //加载图片     
        std::unique_ptr<IImage> pImageData = ImageDecoders.LoadImageData(decodeParam);
        if (pImageData == nullptr) {
            //加载失败
            return nullptr;
        }
        ASSERT((pImageData->GetWidth() > 0) && (pImageData->GetHeight() > 0));
        if ((pImageData->GetWidth() <= 0) || (pImageData->GetHeight() <= 0)) {
            //加载失败
            return nullptr;
        }
        //赋值, 添加到容器(替换删除函数)
        ASSERT(imageKey == imageFullPath);
        spImageData.reset(pImageData.release(), ImageManager::OnImageDataDestroy);//TODO：待验证，或许有平台兼容性问题
        m_imageDataMap[imageKey] = spImageData;       
    }
    if (spImageData == nullptr) {
        //加载失败
        return nullptr;
    }
    //计算ImageInfo的宽度和高度（注意：与图片的实际宽度和高度值可能不相同）
    //ImageInfo的宽度和高度: 影响布局
    //图片的宽度和高度：用于绘制
    int32_t nImageInfoWidth = spImageData->GetWidth();
    int32_t nImageInfoHeight = spImageData->GetHeight();
    const float fRealImageSizeScale = spImageData->GetImageSizeScale(); //实际加载的缩放比例
    if ((nImageDpiScale != 100) && ImageUtil::IsValidImageScale(fRealImageSizeScale)) {
        const float fSizeScale = static_cast<float>(loadParam.GetLoadDpiScale()) / 100.0f;
        //用的是图片自适应图片（非原图），需要用原图大小来计算ImageInfo大小
        int32_t nCalcSize = static_cast<int32_t>(nImageInfoWidth * 1.0f / fRealImageSizeScale + 0.5f);
        nCalcSize = static_cast<int32_t>(nCalcSize * 100.0f / nImageDpiScale + 0.5f);//原图大小
        nCalcSize = ImageUtil::GetScaledImageSize((uint32_t)nCalcSize, fSizeScale);
        if (nCalcSize > 0) {
            nImageInfoWidth = nCalcSize;
        }
        nCalcSize = static_cast<int32_t>(nImageInfoHeight * 1.0f / fRealImageSizeScale + 0.5f);
        nCalcSize = static_cast<int32_t>(nCalcSize * 100.0f / nImageDpiScale + 0.5f);//原图大小
        nCalcSize = ImageUtil::GetScaledImageSize((uint32_t)nCalcSize, fSizeScale);
        if (nCalcSize > 0) {
            nImageInfoHeight = nCalcSize;
        }
    }

    if (bHasFixedSize || bHasFixedPercent) {
        //有设置图片属性：通过width='300'或者width='300%'这种形式设置的图片属性
        bool bFixedWidthSet = (nImageFixedWidth > 0) || ImageUtil::NeedResizeImage(fImageFixedWidthPercent);
        bool bFixedHeightSet = (nImageFixedHeight > 0) || ImageUtil::NeedResizeImage(fImageFixedHeightPercent);
        if (bFixedWidthSet && bFixedHeightSet) {
            //宽度和高度均设置
            if (nImageFixedWidth > 0) {
                nImageInfoWidth = nImageFixedWidth;
            }
            else if (ImageUtil::NeedResizeImage(fImageFixedWidthPercent)) {
                nImageInfoWidth = ImageUtil::GetScaledImageSize((uint32_t)nImageInfoWidth, fImageFixedWidthPercent);
            }

            if (nImageFixedHeight > 0) {
                nImageInfoHeight = nImageFixedHeight;
            }
            else if (ImageUtil::NeedResizeImage(fImageFixedHeightPercent)) {
                nImageInfoHeight = ImageUtil::GetScaledImageSize((uint32_t)nImageInfoHeight, fImageFixedHeightPercent);
            }
        }
        else if (bFixedWidthSet) {
            //只设置了宽度，高度同比例缩放
            int32_t nOldImageInfoWidth = nImageInfoWidth;
            if (nImageFixedWidth > 0) {
                nImageInfoWidth = nImageFixedWidth;
            }
            else if (ImageUtil::NeedResizeImage(fImageFixedWidthPercent)) {
                nImageInfoWidth = ImageUtil::GetScaledImageSize((uint32_t)nImageInfoWidth, fImageFixedWidthPercent);
            }
            float fNewScale = static_cast<float>(nImageInfoWidth) / nOldImageInfoWidth;
            nImageInfoHeight = ImageUtil::GetScaledImageSize((uint32_t)nImageInfoHeight, fNewScale);
        }
        else if (bFixedHeightSet) {
            //只设置了高度，宽度同比例缩放
            int32_t nOldImageInfoHeight = nImageInfoHeight;
            if (nImageFixedHeight > 0) {
                nImageInfoHeight = nImageFixedHeight;
            }
            else if (ImageUtil::NeedResizeImage(fImageFixedHeightPercent)) {
                nImageInfoHeight = ImageUtil::GetScaledImageSize((uint32_t)nImageInfoHeight, fImageFixedHeightPercent);
            }
            float fNewScale = static_cast<float>(nImageInfoHeight) / nOldImageInfoHeight;
            nImageInfoWidth = ImageUtil::GetScaledImageSize((uint32_t)nImageInfoWidth, fNewScale);
        }
    }
    bool bBitmapSizeDpiScaled = bEnableImageDpiScale;
    std::shared_ptr<ImageInfo> imageInfo(new ImageInfo,&ImageManager::OnImageInfoDestroy);
    imageInfo->SetImageKey(imageKey);
    if (imageInfo->SetImageData(loadParam, nImageInfoWidth, nImageInfoHeight, spImageData, bBitmapSizeDpiScaled)) {
        ASSERT(loadKey == imageInfo->GetLoadKey());
        m_imageInfoMap[loadKey] = imageInfo;
    }
    else {
        ASSERT(0);
        imageInfo.reset();
    }
    return imageInfo;
}

void ImageManager::OnImageInfoDestroy(ImageInfo* pImageInfo)
{
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    ASSERT(pImageInfo != nullptr);
    ImageManager& imageManager = GlobalManager::Instance().Image();
    if (pImageInfo != nullptr) {
        DString loadKey = pImageInfo->GetLoadKey();
        if (!loadKey.empty()) {            
            auto iter = imageManager.m_imageInfoMap.find(loadKey);
            if (iter != imageManager.m_imageInfoMap.end()) {
                imageManager.m_imageInfoMap.erase(iter);
            }
        }
        delete pImageInfo;
#ifdef _DEBUG
        //DString log = _T("Removed ImageInfo: ") + loadKey + _T("\n");
        //::OutputDebugString(log.c_str());
#endif
    }
}

void ImageManager::OnImageDataDestroy(IImage* pImage)
{
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    ASSERT(pImage != nullptr);
    ImageManager& imageManager = GlobalManager::Instance().Image();
    if (pImage != nullptr) {
        auto iter = imageManager.m_imageDataMap.begin();
        while (iter != imageManager.m_imageDataMap.end()) {
            if (iter->second.expired()) {
#ifdef _DEBUG
                //DString log = _T("Removed ImageData: ") + iter->first + _T("\n");
                //::OutputDebugString(log.c_str());
#endif
                iter = imageManager.m_imageDataMap.erase(iter);
            }
            else {
                ++iter;
            }
        }
        delete pImage;
    }
}

void ImageManager::RemoveAllImages()
{
    m_imageDataMap.clear();
    m_delayReleaseImageList.clear();
    m_imageInfoMap.clear();
}

void ImageManager::ReleaseImage(const std::shared_ptr<IImage>& pImageData)
{
    if (pImageData != nullptr) {
        TImageData imageData;
        imageData.m_pImage = pImageData;
        imageData.m_releaseTime = std::chrono::steady_clock::now();
        m_delayReleaseImageList.push_back(imageData);
        //TODO: 检查并释放图片资源

    }
}

void ImageManager::SetDpiScaleAllImages(bool bEnable)
{
    m_bDpiScaleAllImages = bEnable;
}

bool ImageManager::IsDpiScaleAllImages() const
{
    return m_bDpiScaleAllImages;
}

void ImageManager::SetAutoMatchScaleImage(bool bAutoMatchScaleImage)
{
    m_bAutoMatchScaleImage = bAutoMatchScaleImage;
}

bool ImageManager::IsAutoMatchScaleImage() const
{
    return m_bAutoMatchScaleImage;
}

void ImageManager::SetImageAsyncLoad(bool bImageAsyncLoad)
{
    m_bImageAsyncLoad = bImageAsyncLoad;
}

bool ImageManager::IsImageAsyncLoad() const
{
    return m_bImageAsyncLoad;
}

bool ImageManager::GetDpiScaleImageFullPath(uint32_t dpiScale,
                                            bool bIsUseZip,
                                            const DString& imageFullPath,
                                            DString& dpiImageFullPath,
                                            uint32_t& nImageDpiScale) const
{
    nImageDpiScale = 0;
    if (FindDpiScaleImageFullPath(dpiScale, bIsUseZip, imageFullPath, dpiImageFullPath)) {
        nImageDpiScale = dpiScale;
        return true;
    }
    dpiImageFullPath.clear();
    if (!IsAutoMatchScaleImage() || (dpiScale < 115)) {
        //智能匹配功能关闭, 或当前DPI无需智能匹配缩放百分比
        return false;
    }

    DString dpiFullPath;
    std::vector<uint32_t> allScales = {125, 150, 175, 200, 225, 250, 300};
    std::vector<std::pair<uint32_t, DString>> allDpiImagePath;
    for (auto scale : allScales) {
        if (FindDpiScaleImageFullPath(scale, bIsUseZip, imageFullPath, dpiFullPath)) {
            allDpiImagePath.push_back({ scale, dpiFullPath });
        }
    }

    dpiImageFullPath.clear();
    size_t nCount = allDpiImagePath.size();
    for (size_t index = 0; index < nCount; ++index) {
        uint32_t nScale = allDpiImagePath[index].first;
        const DString& sPath = allDpiImagePath[index].second;
        if (nScale > dpiScale) {
            if (index == 0) {
                //第一个
                dpiImageFullPath = sPath;
                nImageDpiScale = nScale;
                break;
            }
            else {
                //在两个中间, 选一个最接近的
                uint32_t nLastScale = allDpiImagePath[index - 1].first;
                ASSERT(nLastScale <= dpiScale);
                float diffScaleLast = ((float)dpiScale - (float)nLastScale) / (float)nLastScale;
                float diffScale = ((float)nScale - (float)dpiScale) / (float)nScale;
                if (diffScaleLast < diffScale) {
                    dpiImageFullPath = allDpiImagePath[index - 1].second;
                    nImageDpiScale = allDpiImagePath[index - 1].first;
                }
                else {
                    dpiImageFullPath = sPath;
                    nImageDpiScale = nScale;
                }
                break;
            }
        }
        else if (index == (nCount - 1)) {
            //最后一个
            dpiImageFullPath = sPath;
            nImageDpiScale = nScale;
        }
    }
    return !dpiImageFullPath.empty();
}

bool ImageManager::FindDpiScaleImageFullPath(uint32_t dpiScale,
                                             bool bIsUseZip,
                                             const DString& imageFullPath,
                                             DString& dpiImageFullPath) const
{
    dpiImageFullPath.clear();
    if ((dpiScale == 100) || (dpiScale == 0)) {
        //当前DPI无需缩放
        return false;
    }
    dpiImageFullPath = GetDpiScaledPath(dpiScale, imageFullPath);
    if (dpiImageFullPath.empty()) {
        return false;
    }

    bool bExists = false;
    if (bIsUseZip) {
        bExists = GlobalManager::Instance().Zip().IsZipResExist(FilePath(dpiImageFullPath));
    }
    else {
        bExists = FilePath(dpiImageFullPath).IsExistsPath();
    }
    if (!bExists) {
        dpiImageFullPath.clear();
    }
    return bExists;
}

DString ImageManager::GetDpiScaledPath(uint32_t dpiScale, const DString& imageFullPath) const
{
    DString strPathDir;
    DString strPathFileName;
    std::list<DString> strPathList = StringUtil::Split(imageFullPath, _T("\\"));
    for (auto it = strPathList.begin(); it != strPathList.end(); ++it) {
        auto itTemp = it;
        if (++itTemp == strPathList.end()) {
            strPathFileName = *it;
        }
        else {
            strPathDir += *it + _T("\\");
        }
    }

    size_t iPointPos = strPathFileName.rfind('.');
    ASSERT(iPointPos != DString::npos);
    if (iPointPos == DString::npos) {
        return DString();
    }
    DString strFileExtension = strPathFileName.substr(iPointPos, strPathFileName.size() - iPointPos);
    DString strFile = strPathFileName.substr(0, iPointPos);
    //返回指定DPI下的图片，举例DPI缩放百分比为120（即放大到120%）的图片："image.png" 对应于 "image@120.png"
    strPathFileName = StringUtil::Printf(_T("%s%s%d%s"), strFile.c_str(), _T("@"), dpiScale, strFileExtension.c_str());
    DString strNewFilePath = strPathDir + strPathFileName;
    return strNewFilePath;
}

}

