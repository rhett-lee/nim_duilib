#include "ImageManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Image/ImageLoadParam.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FileUtil.h"

#ifdef DUILIB_BUILD_FOR_WIN
    #define OUTPUT_IMAGE_LOG 1
#endif

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

std::shared_ptr<ImageInfo> ImageManager::GetImage(const ImageLoadParam& loadParam, bool& bFromCache)
{
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    bFromCache = false;
    const DString loadKey = loadParam.GetLoadKey(loadParam.GetLoadDpiScale());
    auto iter = m_imageInfoMap.find(loadKey);
    if (iter != m_imageInfoMap.end()) {
        std::shared_ptr<ImageInfo> spImageInfo = iter->second.lock();
        if (spImageInfo != nullptr) {
            //从缓存中，找到有效图片资源，直接返回
            bFromCache = true;
            return spImageInfo;
        }
    }

    //重新加载资源
    const ImageLoadPath& imageLoadPath = loadParam.GetImageLoadPath();
    DString imageFullPath = imageLoadPath.m_imageFullPath;              //图片的路径（本地路径或者压缩包内相对路径）
    uint32_t nImageFileDpiScale = 100;                                  //原始图片，未经DPI缩放时，DPI缩放比例是100
    const bool isUseZip = GlobalManager::Instance().Zip().IsUseZip();   //是否使用Zip压缩包
    const bool bEnableImageDpiScale = (IsDpiScaleAllImages() &&         //仅在DPI缩放图片功能开启的情况下，查找对应DPI的图片是否存在
                                      (loadParam.GetLoadDpiScaleOption() != DpiScaleOption::kOff)) || //图片属性：load_scale="false"，只使用原图，不需要缩放
                                      loadParam.IsSvgImageFile();       // SVG图片，始终开启DPI自适应（因关闭时，部分矢量缩放的逻辑失效，导致绘制异常）
    if (bEnableImageDpiScale && 
        ((imageLoadPath.m_pathType == ImageLoadPathType::kLocalResPath) ||
         (imageLoadPath.m_pathType == ImageLoadPathType::kZipResPath))) {
        //只有在资源目录下的文件，才执行查找适配DPI图片的功能
        DString dpiImageFullPath;
        uint32_t dpiImageDpiScale = nImageFileDpiScale;
        if (GetDpiScaleImageFullPath(loadParam.GetLoadDpiScale(), isUseZip, imageFullPath, dpiImageFullPath, dpiImageDpiScale)) {
            //标记DPI自适应图片属性，如果路径不同，说明已经选择了对应DPI下的文件            
            ASSERT((dpiImageDpiScale != 0) && !dpiImageFullPath.empty());
            if ((dpiImageDpiScale != 0) && !dpiImageFullPath.empty()) {
                imageFullPath = dpiImageFullPath;
                nImageFileDpiScale = dpiImageDpiScale;
                ASSERT(!imageFullPath.empty());
            }
        }
    }

    float fImageSizeScale = 1.0f;
    //计算设置的比例, 影响加载的缩放百分比（通过width='300'或者width='300%'这种形式设置的图片属性）
    const bool bHasFixedSize = loadParam.HasImageFixedSize();
    const bool bHasFixedPercent = loadParam.HasImageFixedPercent();
    if (bEnableImageDpiScale && !bHasFixedSize && !bHasFixedPercent) {
        //加载的比例（按相对原图来计算，确保各个DPI适配图的显示效果相同）
        //1.如果图片宽高用于评估显示空间的大小：必须按照DPI缩放比来缩放，这样才能在不同DPI下界面显示效果相同
        //2.如果不需要用图片的宽度和高度评估显示空间大小，那么这个加载比例只影响图片显示效果，不影响布局
        //3.如果设置了图片的width或者height属性，只使用原图，不需要缩放（因为加载后要执行缩放操作）
        fImageSizeScale = static_cast<float>(loadParam.GetLoadDpiScale()) / static_cast<float>(nImageFileDpiScale);
    }

    std::shared_ptr<IImage> spImageData;
    //查询缓存，如果缓存存在，则可共享图片资源，无需重复加载
    const DString imageKey = imageFullPath;
    auto iterImageData = m_imageDataMap.find(imageKey);
    if (iterImageData != m_imageDataMap.end()) {
        spImageData = iterImageData->second.m_pImage.lock();
        if (spImageData != nullptr) {
            if (!ImageUtil::IsSameImageScale(iterImageData->second.m_fImageSizeScale, fImageSizeScale)) {
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
        if (nImageFileDpiScale == 100) {//针对DPI自适应的原图，不开启该项优化，避免计算原图大小时出现异常
            decodeParam.m_rcMaxDestRectSize = loadParam.GetMaxDestRectSize();
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
        ASSERT(pImageData != nullptr);
        if (pImageData == nullptr) {
            //加载失败
            return nullptr;
        }
        //////////////TODO
#ifdef  OUTPUT_IMAGE_LOG
        /*if (1) {
            float fScale = pImageData->GetImageSizeScale();
            int32_t w = pImageData->GetWidth();
            int32_t h = pImageData->GetHeight();
            int32_t ww = (int32_t)(w / fScale + 0.5f);
            int32_t hh = (int32_t)(h / fScale + 0.5f);
            fScale = 0;
        }*/
#endif
        //////////////
        ASSERT((pImageData->GetWidth() > 0) && (pImageData->GetHeight() > 0));
        if ((pImageData->GetWidth() <= 0) || (pImageData->GetHeight() <= 0)) {
            //加载失败
            return nullptr;
        }
        //赋值, 添加到容器(替换删除函数)
        ASSERT(imageKey == imageFullPath);
        spImageData.reset(pImageData.release(), ImageManager::CallImageDataDestroy);//TODO：待验证，或许有平台兼容性问题
        OnImageDataCreate(imageKey, spImageData, fImageSizeScale);        
    }
    if (spImageData != nullptr) {
        std::shared_ptr<ImageInfo> imageInfo(new ImageInfo, &ImageManager::CallImageInfoDestroy);
        imageInfo->SetImageKey(imageKey);
        bool bRet = imageInfo->SetImageData(loadParam, spImageData, bEnableImageDpiScale, nImageFileDpiScale);
        ASSERT(bRet);
        if (bRet) {
            ASSERT(loadKey == imageInfo->GetLoadKey());
            OnImageInfoCreate(imageInfo);
            return imageInfo;
        }
    }
    return nullptr;
}

void ImageManager::CallImageInfoDestroy(ImageInfo* pImageInfo)
{
    ImageManager& imageManager = GlobalManager::Instance().Image();
    imageManager.OnImageInfoDestroy(pImageInfo);
}

void ImageManager::CallImageDataDestroy(IImage* pImage)
{
    ImageManager& imageManager = GlobalManager::Instance().Image();
    imageManager.OnImageDataDestroy(pImage);
}

void ImageManager::OnImageInfoCreate(std::shared_ptr<ImageInfo>& pImageInfo)
{
    ASSERT(pImageInfo != nullptr);
    if (pImageInfo != nullptr) {
        DString loadKey = pImageInfo->GetLoadKey();
        ASSERT(!loadKey.empty());
        if (!loadKey.empty()) {
            m_imageInfoMap[loadKey] = pImageInfo;
#ifdef OUTPUT_IMAGE_LOG
            DString log = _T("Created ImageInfo: ") + loadKey + _T("\n");
            ::OutputDebugString(log.c_str());
#endif
        }
    }
}

void ImageManager::OnImageInfoDestroy(ImageInfo* pImageInfo)
{
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    ASSERT(pImageInfo != nullptr);
    if (pImageInfo != nullptr) {
        DString loadKey = pImageInfo->GetLoadKey();
        ASSERT(!loadKey.empty());
        if (!loadKey.empty()) {            
            auto iter = m_imageInfoMap.find(loadKey);
            if (iter != m_imageInfoMap.end()) {
                m_imageInfoMap.erase(iter);
            }
        }
        delete pImageInfo;
#ifdef OUTPUT_IMAGE_LOG
        DString log = _T("Removed ImageInfo: ") + loadKey + _T("\n");
        ::OutputDebugString(log.c_str());
#endif
    }
}

void ImageManager::OnImageDataCreate(const DString& imageKey, std::shared_ptr<IImage>& pImage, float fImageSizeScale)
{
    ASSERT(!imageKey.empty() && (pImage != nullptr));
    if (!imageKey.empty() && (pImage != nullptr)) {
        m_imageDataMap[imageKey] = TImageData(pImage, fImageSizeScale);
#ifdef OUTPUT_IMAGE_LOG
        DString log = _T("Created ImageData: ") + imageKey + _T("\n");
        ::OutputDebugString(log.c_str());
#endif
    }
}

void ImageManager::OnImageDataDestroy(IImage* pImage)
{
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    ASSERT(pImage != nullptr);
    if (pImage != nullptr) {
        auto iter = m_imageDataMap.begin();
        while (iter != m_imageDataMap.end()) {
            if (iter->second.m_pImage.expired()) {
#ifdef OUTPUT_IMAGE_LOG
                DString log = _T("Removed ImageData: ") + iter->first + _T("\n");
                ::OutputDebugString(log.c_str());
#endif
                iter = m_imageDataMap.erase(iter);
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
        TReleaseImageData imageData;
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
                                            uint32_t& nImageFileDpiScale) const
{
    nImageFileDpiScale = 0;
    if (FindDpiScaleImageFullPath(dpiScale, bIsUseZip, imageFullPath, dpiImageFullPath)) {
        nImageFileDpiScale = dpiScale;
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
                nImageFileDpiScale = nScale;
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
                    nImageFileDpiScale = allDpiImagePath[index - 1].first;
                }
                else {
                    dpiImageFullPath = sPath;
                    nImageFileDpiScale = nScale;
                }
                break;
            }
        }
        else if (index == (nCount - 1)) {
            //最后一个
            dpiImageFullPath = sPath;
            nImageFileDpiScale = nScale;
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

void ImageManager::AddDelayPaintData(Control* pControl, Image* pImage, const DString& imageKey)
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT((pControl != nullptr) && (pImage != nullptr) && !imageKey.empty());
    if ((pControl == nullptr) || (pImage == nullptr) || imageKey.empty()) {
        return;
    }
    RemoveDelayPaintData(pImage); //一个Image资源，只保留一条记录

    TImageDelayPaintData delayPaint;
    delayPaint.m_pControl = pControl;
    delayPaint.m_pImage = pImage;
    delayPaint.m_imageKey = imageKey;
    m_delayPaintImageList.push_back(delayPaint);
}

void ImageManager::RemoveDelayPaintData(Control* pControl)
{
    GlobalManager::Instance().AssertUIThread();
    GlobalManager::Instance().AssertUIThread();
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return;
    }
    auto iter = m_delayPaintImageList.begin();
    while (iter != m_delayPaintImageList.end()) {
        if ((iter->m_pControl == pControl) || (iter->m_pControl == nullptr)) {
            iter = m_delayPaintImageList.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void ImageManager::RemoveDelayPaintData(Image* pImage)
{
    GlobalManager::Instance().AssertUIThread();
    GlobalManager::Instance().AssertUIThread();
    ASSERT((pImage != nullptr));
    if (pImage == nullptr) {
        return;
    }
    auto iter = m_delayPaintImageList.begin();
    while (iter != m_delayPaintImageList.end()) {
        if ((iter->m_pImage == pImage) || (iter->m_pImage == nullptr)) {
            iter = m_delayPaintImageList.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void ImageManager::DelayPaintImage(const DString& imageKey)
{
    GlobalManager::Instance().AssertUIThread();
    GlobalManager::Instance().AssertUIThread();
    ASSERT(!imageKey.empty());
    if (imageKey.empty()) {
        return;
    }
    auto iter = m_delayPaintImageList.begin();
    while (iter != m_delayPaintImageList.end()) {
        if ((iter->m_pControl == nullptr) || (iter->m_pImage == nullptr) || (iter->m_imageKey == imageKey)) {
            ControlPtrT<Image> pImage = iter->m_pImage;
            iter = m_delayPaintImageList.erase(iter);
            if (pImage != nullptr) {
                pImage->RedrawImage();
            }
        }
        else {
            ++iter;
        }
    }
}

} //namespace ui
