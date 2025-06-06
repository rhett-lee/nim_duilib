#include "ImageManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Image/ImageDecoder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FileUtil.h"

namespace ui 
{

/** 加载多帧图片所需的参数(多线程加载多帧图片时复用这些参数)
*/
struct LoadImageParam
{
    explicit LoadImageParam(const ImageLoadAttribute& loadAtrribute):
        m_nThreadIdentifier(-1),
        m_nFrameCount(0),
        m_imageLoadAtrribute(loadAtrribute),
        m_bEnableImageDpiScale(false),
        m_nImageDpiScale(0),
        m_nWindowDpiScale(0)
    {
    }
    int32_t m_nThreadIdentifier;
    uint32_t m_nFrameCount;
    std::vector<uint8_t> m_fileData;
    ImageLoadAttribute m_imageLoadAtrribute;
    bool m_bEnableImageDpiScale;
    uint32_t m_nImageDpiScale;
    uint32_t m_nWindowDpiScale;
};

ImageManager::ImageManager():
    m_bDpiScaleAllImages(true),
    m_bAutoMatchScaleImage(true)
{
}

ImageManager::~ImageManager()
{
}

std::shared_ptr<ImageInfo> ImageManager::GetImage(const Window* pWindow,
                                                  const ImageLoadAttribute& loadAtrribute,
                                                  StdClosure asyncLoadCallback)
{
    const DpiManager& dpi = (pWindow != nullptr) ? pWindow->Dpi() : GlobalManager::Instance().Dpi();
    //查找对应关系：LoadKey ->(多对一) ImageKey ->(一对一) SharedImage
    DString loadKey = loadAtrribute.GetCacheKey(dpi.GetScale());
    auto iter = m_loadKeyMap.find(loadKey);
    if (iter != m_loadKeyMap.end()) {
        const DString& imageKey = iter->second;
        auto it = m_imageMap.find(imageKey);
        if (it != m_imageMap.end()) {
            std::shared_ptr<ImageInfo> sharedImage = it->second.lock();
            if (sharedImage) {
                //从缓存中，找到有效图片资源，直接返回
                return sharedImage;
            }
        }
    }    

    //重新加载资源    
    std::unique_ptr<ImageInfo> imageInfo;
    bool isIcon = false;
    if (GlobalManager::Instance().Icon().IsIconString(loadAtrribute.GetImageFullPath())) {
        //加载ICON
        isIcon = true;
        LoadIconData(pWindow, loadAtrribute, imageInfo);
    }

    //加载多帧图片所需的参数(多线程加载多帧图片时复用这些参数)
    std::shared_ptr<LoadImageParam> spLoadImageParam;
    bool isDpiScaledImageFile = false; //该图片是否为DPI自适应的图片（不是DPI为96的原始图片）
    if (!isIcon) {
        DString imageFullPath = loadAtrribute.GetImageFullPath();
        bool isUseZip = GlobalManager::Instance().Zip().IsUseZip();
        DString dpiImageFullPath;
        uint32_t nImageDpiScale = 0;
        //仅在DPI缩放图片功能开启的情况下，查找对应DPI的图片是否存在
        const bool bEnableImageDpiScale = IsDpiScaleAllImages();
        if (bEnableImageDpiScale && GetDpiScaleImageFullPath(dpi.GetScale(), isUseZip, imageFullPath, dpiImageFullPath, nImageDpiScale)) {
            //标记DPI自适应图片属性，如果路径不同，说明已经选择了对应DPI下的文件
            isDpiScaledImageFile = true;
            imageFullPath = dpiImageFullPath;
            ASSERT(!imageFullPath.empty());
            ASSERT(nImageDpiScale > 100);
        }
        else {
            nImageDpiScale = 100; //原始图片，未经DPI缩放
            isDpiScaledImageFile = false;
        }
        //加载图片的KEY
        ImageLoadAttribute realLoadAttribute = loadAtrribute;
        realLoadAttribute.SetImageFullPath(imageFullPath);
        DString imageKey;
        if (isDpiScaledImageFile) {
            //有对应DPI的图片文件
            imageKey = realLoadAttribute.GetCacheKey(nImageDpiScale);
        }
        else {
            //无对应DPI缩放比的图片文件
            imageKey = realLoadAttribute.GetCacheKey(0);
        }

        //根据imageKey查询缓存
        if (!imageKey.empty()) {
            auto it = m_imageMap.find(imageKey);
            if (it != m_imageMap.end()) {
                std::shared_ptr<ImageInfo> sharedImage = it->second.lock();
                if ((sharedImage != nullptr) && (sharedImage->GetLoadDpiScale() == dpi.GetScale())) {
                    //与请求的DPI缩放百分比相同
                    return sharedImage;
                }
            }
        }

        //从内存数据加载文件
        std::vector<uint8_t> fileData;
        FilePath imageFilePath(imageFullPath);
        if (isUseZip && !imageFilePath.IsAbsolutePath()) {
            GlobalManager::Instance().Zip().GetZipData(imageFilePath, fileData);
        }
        else {
            FileUtil::ReadFileData(imageFilePath, fileData);
        }
        ASSERT(!fileData.empty());

        imageInfo.reset();
        if (!fileData.empty()) {
            ImageDecoder imageDecoder;
            ImageLoadAttribute imageLoadAtrribute(loadAtrribute);
            if (isDpiScaledImageFile) {
                imageLoadAtrribute.SetNeedDpiScale(false);
            }
            const int32_t nThreadIdentifier = ThreadIdentifier::kThreadWorker;
            bool bLoadAllFrames = true;
            bool bHasWorkerThread = GlobalManager::Instance().Thread().HasThread(nThreadIdentifier);
            if (bHasWorkerThread) {
                //如果存在Worker线程，则采用多线程异步加载多帧图片（如GIF图片等）
                bLoadAllFrames = false;
            }
            uint32_t nFrameCount = 0;
            const uint32_t nWindowDpiScale = dpi.GetScale();
            imageInfo = imageDecoder.LoadImageData(fileData, 
                                                   imageLoadAtrribute, 
                                                   bEnableImageDpiScale, nImageDpiScale, nWindowDpiScale,
                                                   bLoadAllFrames, nFrameCount);
            if (imageInfo != nullptr) {
                imageInfo->SetImageKey(imageKey);
                if (!bLoadAllFrames && (nFrameCount > 1)) {
                    //启动多线程加载多帧图片时，设置参数
                    spLoadImageParam = std::make_shared<LoadImageParam>(imageLoadAtrribute);
                    spLoadImageParam->m_nThreadIdentifier = nThreadIdentifier;
                    spLoadImageParam->m_nFrameCount = nFrameCount;
                    spLoadImageParam->m_fileData.swap(fileData);
                    spLoadImageParam->m_bEnableImageDpiScale = bEnableImageDpiScale;
                    spLoadImageParam->m_nImageDpiScale = nImageDpiScale;
                    spLoadImageParam->m_nWindowDpiScale = nWindowDpiScale;
                }
            }            
        }
    }    
    std::shared_ptr<ImageInfo> sharedImage;
    if (imageInfo != nullptr) {
        DString imageKey = imageInfo->GetImageKey();
        uint32_t nWindowDpiScale = dpi.GetScale();
        sharedImage = SaveImageInfo(imageInfo.release(), loadKey, nWindowDpiScale, isDpiScaledImageFile);
        if ((spLoadImageParam != nullptr) && (spLoadImageParam->m_nThreadIdentifier >= 0)) {
            //启动多线程加载多帧图片, 在子线程中加载完成后，更新图片数据
            auto loadImageTask = [this, spLoadImageParam, loadKey, imageKey, nWindowDpiScale, isDpiScaledImageFile, asyncLoadCallback]() {
                    //该函数的代码在子线程中执行
                    uint32_t nFrameCount = 0;
                    ImageDecoder imageDecoder;
                    std::unique_ptr<ImageInfo> pNewImageInfo = nullptr;
                    pNewImageInfo = imageDecoder.LoadImageData(spLoadImageParam->m_fileData,
                                                               spLoadImageParam->m_imageLoadAtrribute,
                                                               spLoadImageParam->m_bEnableImageDpiScale,
                                                               spLoadImageParam->m_nImageDpiScale,
                                                               spLoadImageParam->m_nWindowDpiScale,
                                                               true, nFrameCount);
                    if ((pNewImageInfo != nullptr) && (&GlobalManager::Instance().Image() == this)) {
                        //发送到UI线程，更新图片数据，然后刷新界面显示
                        std::shared_ptr<ImageInfo> spNewSharedImage;
                        spNewSharedImage.reset(pNewImageInfo.release());
                        auto updateUiTask = [this, spNewSharedImage, loadKey, imageKey, nWindowDpiScale, isDpiScaledImageFile, asyncLoadCallback]() {
                                //该函数的代码在UI线程中执行                                
                                if (&GlobalManager::Instance().Image() == this) {
                                    bool bUpdated = UpdateImageInfo(spNewSharedImage, loadKey, imageKey, nWindowDpiScale, isDpiScaledImageFile);
                                    //ASSERT_UNUSED_VARIABLE(bUpdated);
                                    if (bUpdated && (asyncLoadCallback != nullptr)) {
                                        //加载成功后，回调函数
                                        asyncLoadCallback();
                                    }
                                }
                            };

                        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, updateUiTask);
                    }
                };
            GlobalManager::Instance().Thread().PostTask(spLoadImageParam->m_nThreadIdentifier, loadImageTask);
        }
    }
    return sharedImage;
}

std::shared_ptr<ImageInfo> ImageManager::SaveImageInfo(ImageInfo* pImageInfo, const DString& loadKey, uint32_t nWindowDpiScale, bool isDpiScaledImageFile)
{
    if (pImageInfo == nullptr) {
        return nullptr;
    }
    std::shared_ptr<ImageInfo> sharedImage;
    DString imageKey = pImageInfo->GetImageKey();
    sharedImage.reset(pImageInfo, &OnImageInfoDestroy);
    sharedImage->SetLoadKey(loadKey);
    sharedImage->SetLoadDpiScale(nWindowDpiScale);
    if (isDpiScaledImageFile) {
        //使用了DPI自适应的图片，做标记（必须为true时才能修改这个值）
        sharedImage->SetBitmapSizeDpiScaled(isDpiScaledImageFile);
    }
    if (imageKey.empty()) {
        imageKey = loadKey;
    }

    //保存对应关系：LoadKey ->(多对一) ImageKey ->(一对一) SharedImage
    m_loadKeyMap[loadKey] = imageKey;
    m_imageMap[imageKey] = sharedImage;

#ifdef _DEBUG
    //DString log = _T("Loaded Image: ") + imageKey + _T("\n");
    //::OutputDebugString(log.c_str());
#endif
    return sharedImage;
}

bool ImageManager::UpdateImageInfo(std::shared_ptr<ImageInfo> spNewSharedImage, const DString& loadKey, const DString& imageKey,
                                   uint32_t nWindowDpiScale, bool isDpiScaledImageFile)
{
    GlobalManager::Instance().AssertUIThread();
    //校验
    if (spNewSharedImage->GetFrameCount() <= 1) {
        return false;
    }
    auto iter = m_loadKeyMap.find(loadKey);
    if ((iter == m_loadKeyMap.end()) || (iter->second != imageKey)) {
        //KEY发生变化了，不更新
        return false;
    }

    auto iterShared = m_imageMap.find(imageKey);
    if (iterShared == m_imageMap.end()) {
        //KEY发生变化了，不更新
        return false;
    }
    std::shared_ptr<ImageInfo> spOldSharedImage = iterShared->second.lock();
    if (spOldSharedImage == nullptr) {
        //无数据，不更新
        return false;
    }

    spNewSharedImage->SetLoadKey(loadKey);
    spNewSharedImage->SetImageKey(imageKey);
    spNewSharedImage->SetLoadDpiScale(nWindowDpiScale);
    if (isDpiScaledImageFile) {
        //使用了DPI自适应的图片，做标记（必须为true时才能修改这个值）
        spNewSharedImage->SetBitmapSizeDpiScaled(isDpiScaledImageFile);
    }
    //校验通过后，更新数据    
    bool bUpdated = spOldSharedImage->SwapImageData(*spNewSharedImage);
    spOldSharedImage.reset();
    return bUpdated;
}

void ImageManager::LoadIconData(const Window* pWindow, 
                                const ImageLoadAttribute& loadAtrribute,
                                std::unique_ptr<ImageInfo>& imageInfo) const
{
    imageInfo.reset();
    //加载HICON句柄，作为图片，仅在Windows平台有这个句柄
    DString iconString = loadAtrribute.GetImageFullPath();
    bool bEnableDpiScale = IsDpiScaleAllImages();
    std::vector<uint8_t> bitmapData;
    uint32_t imageWidth = 0;
    uint32_t imageHeight = 0;
    bool bDpiScaled = false;
    if (GlobalManager::Instance().Icon().LoadIconData(iconString, 
                                                      pWindow, loadAtrribute, bEnableDpiScale,
                                                      bitmapData, 
                                                      imageWidth, imageHeight, bDpiScaled)) {
        ASSERT(bitmapData.size() == ((size_t)imageWidth * imageHeight * 4));
        IBitmap* pBitmap = nullptr;
        IRenderFactory* pRenderFactroy = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactroy != nullptr);
        if (pRenderFactroy != nullptr) {
            pBitmap = pRenderFactroy->CreateBitmap();
        }
        ASSERT(pBitmap != nullptr);
        if (pBitmap != nullptr) {
            pBitmap->Init(imageWidth, imageHeight, true, bitmapData.data());
            std::vector<IBitmap*> frameBitmaps;
            frameBitmaps.push_back(pBitmap);
            imageInfo.reset(new ImageInfo);
            imageInfo->SetFrameBitmap(frameBitmaps);
            imageInfo->SetImageSize(imageWidth, imageHeight);
            imageInfo->SetPlayCount(-1);
            imageInfo->SetBitmapSizeDpiScaled(bDpiScaled);
        }
    }
    ASSERT(imageInfo != nullptr);
}

void ImageManager::OnImageInfoDestroy(ImageInfo* pImageInfo)
{
    ASSERT(pImageInfo != nullptr);
    ImageManager& imageManager = GlobalManager::Instance().Image();
    if (pImageInfo != nullptr) {
        DString imageKey;
        DString loadKey = pImageInfo->GetLoadKey();
        if (!loadKey.empty()) {            
            auto iter = imageManager.m_loadKeyMap.find(loadKey);
            if (iter != imageManager.m_loadKeyMap.end()) {
                imageKey = iter->second;
                imageManager.m_loadKeyMap.erase(iter);
            }
            if (!imageKey.empty()) {
                auto it = imageManager.m_imageMap.find(imageKey);
                if (it != imageManager.m_imageMap.end()) {
                    imageManager.m_imageMap.erase(it);
                }
            }
        }
        delete pImageInfo;
#ifdef _DEBUG
        //DString log = _T("Removed Image: ") + imageKey + _T("\n");
        //::OutputDebugString(log.c_str());
#endif
    }    
}

void ImageManager::RemoveAllImages()
{
    m_imageMap.clear();
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

