#include "ImageDecoder.h"
#include "duilib/Image/Image.h"
#include "duilib/Image/GdiPlusDefs.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/Bitmap_GDI.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/DpiManager.h"

#include "duilib/third_party/apng/decoder-apng.h"

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
#pragma warning (pop)

#pragma warning (push)
#pragma warning (disable: 4456 4244 4702)
#define NANOSVG_IMPLEMENTATION
#define NANOSVG_ALL_COLOR_KEYWORDS
#include "duilib/third_party/svg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "duilib/third_party/svg/nanosvgrast.h"
#pragma warning (pop)

namespace ui 
{

/** 使用Gdiplus加载图片
*/
namespace GdiplusImageLoader
{
	bool LoadImageByGdiplus(std::unique_ptr<Gdiplus::Bitmap>& pGdiplusBitmap, std::vector<ImageDecoder::ImageData>& imageData)
	{
		if (!pGdiplusBitmap) {
			ASSERT(!"ImageInfo::LoadImageByGdiplus: 失败");
			return false;
		}
		Gdiplus::Status status = pGdiplusBitmap->GetLastStatus();
		ASSERT((status == Gdiplus::Ok) && "ImageInfo::LoadImageByGdiplus: 失败");
		if (status != Gdiplus::Ok) {
			return false;
		}

		UINT nCount = pGdiplusBitmap->GetFrameDimensionsCount();
		std::unique_ptr<GUID[]> pDimensionIDs(new GUID[nCount]);
		pGdiplusBitmap->GetFrameDimensionsList(pDimensionIDs.get(), nCount);
		const size_t iFrameCount = pGdiplusBitmap->GetFrameCount(&pDimensionIDs.get()[0]);
		if (iFrameCount < 1) {
			return false;
		}
		imageData.resize(iFrameCount);
		if (iFrameCount > 1) {
			UINT iSize = pGdiplusBitmap->GetPropertyItemSize(PropertyTagFrameDelay);
			if (iSize > 0) {
				Gdiplus::PropertyItem* pPropertyItem = (Gdiplus::PropertyItem*)malloc(iSize);
				status = pGdiplusBitmap->GetPropertyItem(PropertyTagFrameDelay, iSize, pPropertyItem);
				ASSERT(status == Gdiplus::Ok);
				if (status == Gdiplus::Ok) {
					for (size_t i = 0; i < iFrameCount; ++i) {
						imageData[i].m_frameInterval = (((long*)(pPropertyItem->value))[i] * 10);
					}
				}
			}
		}

		Gdiplus::PixelFormat format = pGdiplusBitmap->GetPixelFormat();
		bool bAlphaChannel = (format & PixelFormatAlpha) != 0;
		if (!bAlphaChannel && ((format & PixelFormatIndexed) != 0)) {
			int nPalSize = pGdiplusBitmap->GetPaletteSize();
			if (nPalSize > 0) {
				Gdiplus::ColorPalette* palette = (Gdiplus::ColorPalette*)malloc(nPalSize);
				if (palette != nullptr) {
					status = pGdiplusBitmap->GetPalette(palette, nPalSize);
					if (status == Gdiplus::Ok) {
						bAlphaChannel = (palette->Flags & Gdiplus::PaletteFlagsHasAlpha) != 0;
					}
					free(palette);
				}
			}
		}

		for (size_t i = 0; i < iFrameCount; ++i) {
			status = pGdiplusBitmap->SelectActiveFrame(&Gdiplus::FrameDimensionTime, (UINT)i);
			ASSERT(status == Gdiplus::Ok);
			if (status != Gdiplus::Ok) {
				imageData.clear();
				break;
			}

			HBITMAP hBitmap = nullptr;
			status = pGdiplusBitmap->GetHBITMAP(Gdiplus::Color(), &hBitmap);
			ASSERT(status == Gdiplus::Ok);
			if (status != Gdiplus::Ok) {
				imageData.clear();
				break;
			}

			BITMAP bm = { 0 };
			::GetObject(hBitmap, sizeof(bm), &bm);			
			ASSERT((bm.bmBits != nullptr) && (bm.bmBitsPixel == 32) && (bm.bmHeight > 0) && (bm.bmWidth > 0));
			if ((bm.bmBits != nullptr) && (bm.bmBitsPixel == 32) && (bm.bmHeight > 0) && (bm.bmWidth > 0)) {
				const uint32_t imageDataSize = bm.bmHeight * bm.bmWidth * 4;
				if (!bAlphaChannel && (format == PixelFormat32bppARGB)) {
					LPBYTE imageBits = (LPBYTE)bm.bmBits;
					for (int ii = 0; ii < bm.bmHeight; ++ii) {
						for (int j = 0; j < bm.bmWidthBytes; j += 4) {
							int x = ii * bm.bmWidthBytes + j;
							if (imageBits[x + 3] != 255) {
								bAlphaChannel = true;
								break;
							}
						}
					}
				}
				ImageDecoder::ImageData& bitmapData = imageData[i];
				bitmapData.m_bAlphaChannel = bAlphaChannel;
				bitmapData.bFlipHeight = false;
				bitmapData.m_imageWidth = bm.bmWidth;
				bitmapData.m_imageHeight = bm.bmHeight;
				bitmapData.m_bitmapData.resize(imageDataSize);
				memcpy(bitmapData.m_bitmapData.data(), bm.bmBits, imageDataSize);
				::DeleteObject(hBitmap);
			}
			else {
				imageData.clear();
				::DeleteObject(hBitmap);
				break;
			}
		}
		return !imageData.empty();
    }

	bool LoadImageFromMemory(std::vector<uint8_t>& fileData, std::vector<ImageDecoder::ImageData>& imageData)
	{
		ASSERT(!fileData.empty());
		if (fileData.empty()) {
			return false;
		}
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, fileData.size());
		if (hGlobal == nullptr) {
			return false;
		}
		unsigned char* pData = (unsigned char*)::GlobalLock(hGlobal);
		if (pData == nullptr) {
			::GlobalFree(hGlobal);
			return false;
		}
		memcpy(pData, fileData.data(), fileData.size());
		IStream* stream = nullptr;
		::CreateStreamOnHGlobal(hGlobal, FALSE, &stream);
		if (stream == nullptr) {
			::GlobalUnlock(hGlobal);
			::GlobalFree(hGlobal);
			return false;
		}
		std::unique_ptr<Gdiplus::Bitmap> pGdiplusBitmap(Gdiplus::Bitmap::FromStream(stream));
		bool isLoaded = LoadImageByGdiplus(pGdiplusBitmap, imageData);
		stream->Release();
		::GlobalUnlock(hGlobal);
		::GlobalFree(hGlobal);
		return isLoaded;
	}
}

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
		bool bAlphaChannel = false; 
		std::vector<uint8_t>& argbData = imageData.m_bitmapData;
		argbData.clear();
		ASSERT((buffer != nullptr) && (len > 0));
		if ((buffer == nullptr) || (len <= 0)) {
			return false;
		}
		const int desired_channels = 4; //返回的图像数据格式固定：RGBA，每个图像元素是4个字节
		int channels_in_file = 4;
		uint8_t* rgbaData = stbi_load_from_memory(buffer, len, &nWidth, &nHeight, &channels_in_file, desired_channels);
		if (rgbaData == nullptr) {
			return false;
		}

		ASSERT( (channels_in_file == 3) || (channels_in_file == 4));
		ASSERT((nWidth > 0) && (nHeight > 0));
		if (((channels_in_file == 3) || (channels_in_file == 4)) && 
			(nWidth > 0) && (nHeight > 0)) {
			bAlphaChannel = false;
			argbData.resize(nHeight * nWidth * desired_channels);
			const size_t colorCount = nHeight * nWidth;

			//数据格式转换：ABGR[alpha, blue, green, red] -> ARGB[alpha, red, green, blue]				
			for (size_t i = 0; i < colorCount; ++i) {
				size_t colorIndex = i * 4;
				if (channels_in_file == 4) {
					argbData[colorIndex + 3] = rgbaData[colorIndex + 3]; //A, alpha
					if (!bAlphaChannel && (argbData[colorIndex + 3] < 255)) {
						bAlphaChannel = true;
					}
				}
				else {
					argbData[colorIndex + 3] = 255; //A, alpha
				}
				argbData[colorIndex + 0] = rgbaData[colorIndex + 2]; //B, blue
				argbData[colorIndex + 1] = rgbaData[colorIndex + 1]; //G, green
				argbData[colorIndex + 2] = rgbaData[colorIndex + 0]; //R, red
			}

			imageData.bFlipHeight = true;
			imageData.m_bAlphaChannel = bAlphaChannel;
			imageData.m_frameInterval = 0;
			imageData.m_imageWidth = nWidth;
			imageData.m_imageHeight = nHeight;
		}
		stbi_image_free(rgbaData);
		return !argbData.empty();
	}
}

/** 使用APNG加载图片
*/
namespace APNGImageLoader
{
	bool DecodeAPNG(APNGDATA* pngData, std::vector<ImageDecoder::ImageData>& imageData, int32_t& playCount)
	{
		ASSERT((pngData != nullptr) && (pngData->nWid > 0) && (pngData->nHei > 0) && (pngData->nFrames > 0));
		imageData.clear();
		playCount = pngData->nLoops;

		int nWid = pngData->nWid;
		int nHei = pngData->nHei;
		if ((nWid <= 0) || (nHei <= 0) || (pngData->nFrames < 1)) {
			return false;
		}

		bool bAlphaChannel = false;
		//swap rgba to bgra and do premultiply
		uint8_t* p = pngData->pdata;
		int pixel_count = nWid * nHei * pngData->nFrames;
		for (int i = 0; i < pixel_count; ++i) {
			BYTE a = p[3];
			BYTE t = p[0];
			if (a)
			{
				p[0] = (p[2] * a) / 255;
				p[1] = (p[1] * a) / 255;
				p[2] = (t * a) / 255;
			}
			else
			{
				memset(p, 0, 4);
			}
			if (!bAlphaChannel && (p[3] != 255)) {
				bAlphaChannel = true;
			}
			p += 4;
		}

		p = pngData->pdata;

		imageData.resize(pngData->nFrames);
		for (int i = 0; i < pngData->nFrames; ++i) {
			const int imageDataSize = nWid * nHei * 4;
			ImageDecoder::ImageData& bitmapData = imageData[i];
			bitmapData.m_frameInterval = pngData->pDelay ? pngData->pDelay[i] : 0;
			bitmapData.bFlipHeight = true;
			bitmapData.m_bAlphaChannel = bAlphaChannel;
			bitmapData.m_imageWidth = nWid;
			bitmapData.m_imageHeight = nHei;
			bitmapData.m_bitmapData.resize(imageDataSize);
			memcpy(bitmapData.m_bitmapData.data(), p, imageDataSize);

			p += imageDataSize;
		}
		return true;
	}

	bool LoadImageFromMemory(std::vector<uint8_t>& fileData, std::vector<ImageDecoder::ImageData>& imageData, int32_t& playCount)
	{
		ASSERT(!fileData.empty());
		if (fileData.empty()) {
			return false;
		}
		bool isLoaded = false;
		APNGDATA* apngData = LoadAPNG_from_memory((const char*)fileData.data(), fileData.size());
		if (apngData != nullptr) {
			isLoaded = DecodeAPNG(apngData, imageData, playCount);
			APNG_Destroy(apngData);
		}		
		return isLoaded;
	}
}

/** 加载SVG图片
*/
namespace SVGImageLoader
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

	/** 从内存数据加载图片
	*/
	bool LoadImageFromMemory(std::vector<uint8_t>& fileData, ImageDecoder::ImageData& imageData)
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
		NSVGimage* svgData = nsvgParse(pData, "px", 96.0f);
		if (hasAppended) {
			fileData.pop_back();
		}

		std::unique_ptr<NSVGimage, SvgDeleter> svg((NSVGimage*)svgData);
		int w = (int)svg->width;
		int h = (int)svg->height;
		std::unique_ptr<NSVGrasterizer, RasterizerDeleter> rast(nsvgCreateRasterizer());
		float scale = 1.0f;
		UINT dpiScale = DpiManager::GetInstance()->GetScale();
		if ((dpiScale != 100) && (dpiScale != 0)) {
			scale = (float)DpiManager::GetInstance()->GetScale() / 100;
			w = static_cast<int>(w * scale);
			h = static_cast<int>(h * scale);
		}
		if (w <= 0 || h <= 0 || !rast) {
			return false;
		}

		const int dataSize = 4;
		std::vector<uint8_t>& bitmapData = imageData.m_bitmapData;
		bitmapData.resize(h * w * dataSize);
		uint8_t* pBmpBits = bitmapData.data();
		nsvgRasterize(rast.get(), svg.get(), 0, 0, scale, pBmpBits, w, h, w * dataSize);

		// nanosvg内部已经做过alpha预乘，这里只做R和B的交换
		for (int y = 0; y < h; y++) {
			unsigned char* row = &pBmpBits[y * w * dataSize];
			for (int x = 0; x < w; x++) {
				int r = row[0], g = row[1], b = row[2], a = row[3];
				(void)a;
				(void)g;
				// 			if (a < 255) {
				// 				row[0] = (unsigned char)(b * a / 255);
				// 				row[1] = (unsigned char)(g * a / 255);
				// 				row[2] = (unsigned char)(r * a / 255);
				// 			}
				// 			else {
				row[0] = static_cast<unsigned char>(b);
				//row[1] = g;
				row[2] = static_cast<unsigned char>(r);
				//			}
				row += 4;
			}
		}

		imageData.m_frameInterval = 0;
		imageData.bFlipHeight = true;
		imageData.m_bAlphaChannel = true;
		imageData.m_imageWidth = w;
		imageData.m_imageHeight = h;
		return true;
	}
}

ImageDecoder::ImageFormat ImageDecoder::GetImageFormat(const std::wstring& path) const
{
	ImageDecoder::ImageFormat imageFormat = ImageFormat::kUnknown;
	if (path.size() < 4) {
		return imageFormat;
	}
	std::wstring fileExt;
	size_t pos = path.rfind(L".");
	if ((pos != std::wstring::npos) && ((pos + 1) < path.size())) {
		fileExt = path.substr(pos + 1, std::wstring::npos);
		fileExt = StringHelper::MakeUpperString(fileExt);
	}
	if (fileExt == L"PNG") {
		imageFormat = ImageFormat::kPNG;
	}
	else if (fileExt == L"SVG") {
		imageFormat = ImageFormat::kSVG;
	}
	else if (fileExt == L"GIF") {
		imageFormat = ImageFormat::kGIF;
	}
	else if (fileExt == L"WEBP") {
		imageFormat = ImageFormat::kWEBP;
	}
	else if ((fileExt == L"JPEG") || (fileExt == L"JPG")) {
		imageFormat = ImageFormat::kJPEG;
	}
	else if (fileExt == L"BMP") {
		imageFormat = ImageFormat::kBMP;
	}
	else if (fileExt == L"ICO") {
		imageFormat = ImageFormat::kICO;
	}
	return imageFormat;
}

std::unique_ptr<ImageInfo> ImageDecoder::LoadImageData(std::vector<uint8_t>& fileData,
											           const ImageLoadAttribute& imageLoadAttribute)
{
	std::wstring imageFullPath = imageLoadAttribute.GetImageFullPath();
	ASSERT(!fileData.empty() && !imageFullPath.empty());
	if (fileData.empty() || imageFullPath.empty()) {
		return nullptr;
	}
	IRenderFactory* pRenderFactroy = GlobalManager::GetRenderFactory();
	ASSERT(pRenderFactroy != nullptr);
	if (pRenderFactroy == nullptr) {
		return nullptr;
	}

	std::vector<ImageData> imageData;
	int32_t playCount = -1;
	ImageDecoder::ImageFormat imageFormat = GetImageFormat(imageFullPath);
	bool isLoaded = DecodeImageData(fileData, imageFormat, imageData, playCount);
	if (!isLoaded || imageData.empty()) {
		return nullptr;
	}

	std::unique_ptr<ImageInfo> imageInfo(new ImageInfo);
	std::vector<int> frameIntervals;
	bool bAlphaChannel = false;
	uint32_t imageWidth = 0;
	uint32_t imageHeight = 0;
	for (const ImageData& bitmapData : imageData) {
		ASSERT(bitmapData.m_bitmapData.size() == (bitmapData.m_imageWidth * bitmapData.m_imageHeight * 4));
		if (bitmapData.m_bitmapData.size() != (bitmapData.m_imageWidth * bitmapData.m_imageHeight * 4)) {
			return nullptr;
		}
		frameIntervals.push_back(bitmapData.m_frameInterval);
		if (!bAlphaChannel) {
			bAlphaChannel = bitmapData.m_bAlphaChannel;
		}
		if (imageWidth == 0) {
			imageWidth = bitmapData.m_imageWidth;
		}
		if (imageHeight == 0) {
			imageHeight = bitmapData.m_imageHeight;
		}

		IBitmap* pBitmap = pRenderFactroy->CreateBitmap();
		ASSERT(pBitmap != nullptr);
		if (pBitmap == nullptr) {
			return nullptr;
		}
		pBitmap->Init(bitmapData.m_imageWidth, bitmapData.m_imageHeight, bitmapData.bFlipHeight, bitmapData.m_bitmapData.data());
		imageInfo->PushBackHBitmap(pBitmap);
	}
	//多帧图片时，以第一帧图片作为图片的大小信息
	if (frameIntervals.size() > 1) {
		imageInfo->SetFrameInterval(frameIntervals);
	}	
	imageInfo->SetImageSize(imageWidth, imageHeight);
	imageInfo->SetImageFullPath(imageFullPath);
	imageInfo->SetAlpha(bAlphaChannel);
	imageInfo->SetPlayCount(playCount);
	return imageInfo;
}

bool ImageDecoder::DecodeImageData(std::vector<uint8_t>& fileData,
								   ImageDecoder::ImageFormat imageFormat,
								   std::vector<ImageData>& imageData,
								   int32_t& playCount)
{
	ASSERT(!fileData.empty());
	if (fileData.empty()) {
		return false;
	}

	playCount = -1;
	imageData.clear();

	bool isLoaded = false;
	switch (imageFormat) {
	case ImageFormat::kJPEG:
	case ImageFormat::kBMP:
		imageData.resize(1);
		isLoaded = STBImageLoader::LoadImageFromMemory(fileData, imageData[0]);
		break;
	case ImageFormat::kSVG:
		imageData.resize(1);
		isLoaded = SVGImageLoader::LoadImageFromMemory(fileData, imageData[0]);
		break;
	case ImageFormat::kGIF:
		break;
	case ImageFormat::kPNG:
		isLoaded = APNGImageLoader::LoadImageFromMemory(fileData, imageData, playCount);
		break;
	case ImageFormat::kWEBP:
		break;
	case ImageFormat::kICO:
		break;
	default:
		break;
	}
	if (!isLoaded) {
		isLoaded = GdiplusImageLoader::LoadImageFromMemory(fileData, imageData);
	}
	return isLoaded;
}

} // namespace ui

