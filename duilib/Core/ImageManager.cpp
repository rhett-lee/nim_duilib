#include "ImageManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Image/ImageDecoder.h"
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
    m_bAutoMatchScaleImage(true)
{
}

ImageManager::~ImageManager()
{
}

std::shared_ptr<ImageInfo> ImageManager::GetImage(const ImageLoadParam& loadParam, StdClosure asyncLoadCallback)
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
    DString imageFullPath = loadParam.GetImageFullPath();               //图片的路径（本地路径或者压缩包内相对路径）
    uint32_t nImageDpiScale = 100;                                      //原始图片，未经DPI缩放时，DPI缩放比例是100
    const bool isUseZip = GlobalManager::Instance().Zip().IsUseZip();   //是否使用Zip压缩包
    const bool bEnableImageDpiScale = IsDpiScaleAllImages();            //仅在DPI缩放图片功能开启的情况下，查找对应DPI的图片是否存在
    if (bEnableImageDpiScale) {
        DString dpiImageFullPath;
        uint32_t dpiImageDpiScale = nImageDpiScale;
        if (GetDpiScaleImageFullPath(loadParam.GetLoadDpiScale(), isUseZip, imageFullPath, dpiImageFullPath, dpiImageDpiScale)) {
            //标记DPI自适应图片属性，如果路径不同，说明已经选择了对应DPI下的文件
            imageFullPath = dpiImageFullPath;
            nImageDpiScale = dpiImageDpiScale;
            ASSERT(!imageFullPath.empty());
            ASSERT(nImageDpiScale != 0);
        }
    }
    if (nImageDpiScale == 0) {
        nImageDpiScale = 100;
    }
    float fImageSizeScale = static_cast<float>(loadParam.GetLoadDpiScale()) / static_cast<float>(nImageDpiScale);//加载的比例

    std::shared_ptr<IImage> spImageData;
    //查询缓存，如果缓存存在，则可共享图片资源，无需重复加载
    const DString imageKey = imageFullPath;
    auto iterImageData = m_imageDataMap.find(imageKey);
    if (iterImageData != m_imageDataMap.end()) {
        spImageData = iterImageData->second.lock();
        if (spImageData != nullptr) {
            ASSERT(ImageUtil::IsSameImageScale(spImageData->GetImageSizeScale(), fImageSizeScale));
            if (!ImageUtil::IsSameImageScale(spImageData->GetImageSizeScale(), fImageSizeScale)) {
                //未知错误
                return nullptr;
            }
        }
    }

    if (spImageData == nullptr) {
        //从内存数据加载图片
        ImageDecoderFactory& ImageDecoders = GlobalManager::Instance().ImageDecoders();
        std::vector<uint8_t> fileData;
        if (!ImageDecoders.IsVirtualImageFile(imageFullPath)) {
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

        //从内存数据加载图片     
        IImageDecoder::ExtraParam extraParam;
        extraParam.m_nIconSize = loadParam.GetIconSize();
        extraParam.m_fPagMaxFrameRate = loadParam.GetPagMaxFrameRate();
        extraParam.m_bLoadAllFrames = true;
        std::unique_ptr<IImage> pImageData = ImageDecoders.LoadImageData(imageFullPath, fileData, fImageSizeScale, &extraParam);
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
    std::shared_ptr<ImageInfo> imageInfo(new ImageInfo,&ImageManager::OnImageInfoDestroy);
    imageInfo->SetImageKey(imageKey);
    if (imageInfo->SetImageData(loadParam, spImageData, asyncLoadCallback)) {
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

