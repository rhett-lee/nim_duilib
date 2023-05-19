#include "ImageDecoder.h"
#include "duilib/Image/Image.h"
#include "duilib/Image/GdiPlusDefs.h"
#include "duilib/Image/SvgUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/Bitmap_GDI.h"

namespace ui 
{
namespace ImageLoader
{
    std::unique_ptr<ImageInfo> LoadImageByBitmap(std::unique_ptr<Gdiplus::Bitmap>& pGdiplusBitmap, const std::wstring& imageFullPath)
    {
		if (pGdiplusBitmap == nullptr) {
			return nullptr;
		}
		Gdiplus::Status status;
		status = pGdiplusBitmap->GetLastStatus();
		ASSERT((status == Gdiplus::Ok) && "ImageInfo::LoadImageByBitmap: Ê§°Ü");
		if (status != Gdiplus::Ok) {
			return nullptr;
		}

		UINT nCount = pGdiplusBitmap->GetFrameDimensionsCount();
		std::unique_ptr<GUID[]> pDimensionIDs(new GUID[nCount]);
		pGdiplusBitmap->GetFrameDimensionsList(pDimensionIDs.get(), nCount);
		int iFrameCount = pGdiplusBitmap->GetFrameCount(&pDimensionIDs.get()[0]);

		std::unique_ptr<ImageInfo> imageInfo(new ImageInfo);
		if (iFrameCount > 1) {
			int iSize = pGdiplusBitmap->GetPropertyItemSize(PropertyTagFrameDelay);
			if (iSize > 0) {
				Gdiplus::PropertyItem* pPropertyItem = (Gdiplus::PropertyItem*)malloc(iSize);
				status = pGdiplusBitmap->GetPropertyItem(PropertyTagFrameDelay, iSize, pPropertyItem);
				ASSERT(status == Gdiplus::Ok);
				if (status == Gdiplus::Ok) {
					std::vector<int> frameIntervals;
					for (int i = 0; i < iFrameCount; i++) {
						frameIntervals.push_back(((long*)(pPropertyItem->value))[i] * 10);
					}
					imageInfo->SetFrameInterval(frameIntervals);
				}
			}
		}

		for (int i = 0; i < iFrameCount; i++) {
			status = pGdiplusBitmap->SelectActiveFrame(&Gdiplus::FrameDimensionTime, i);
			ASSERT(status == Gdiplus::Ok);
			if (status != Gdiplus::Ok) {
				return nullptr;
			}

			HBITMAP hBitmap = nullptr;
			status = pGdiplusBitmap->GetHBITMAP(Gdiplus::Color(), &hBitmap);
			ASSERT(status == Gdiplus::Ok);
			if (status != Gdiplus::Ok) {
				return nullptr;
			}

			BITMAP bm = { 0 };
			::GetObject(hBitmap, sizeof(bm), &bm);			
			LPBYTE imageBits = (LPBYTE)bm.bmBits;
			ASSERT(imageBits != nullptr);
			ASSERT(bm.bmBitsPixel == 32);
			if ((imageBits != nullptr) && (bm.bmBitsPixel == 32)) {
				IBitmap* pBitmap = nullptr;
				IRenderFactory* pRenderFactroy = GlobalManager::GetRenderFactory();
				ASSERT(pRenderFactroy != nullptr);
				if (pRenderFactroy != nullptr) {
					pBitmap = pRenderFactroy->CreateBitmap();
				}
				ASSERT(pBitmap != nullptr);
				if (pBitmap == nullptr) {
					return nullptr;
				}
				pBitmap->Init(bm.bmWidth, bm.bmHeight, false, imageBits);
				imageInfo->PushBackHBitmap(pBitmap);
				::DeleteObject(hBitmap);
			}
			else {
				::DeleteObject(hBitmap);
				return nullptr;
			}
		}

		imageInfo->SetImageSize(pGdiplusBitmap->GetWidth(), pGdiplusBitmap->GetHeight());
		imageInfo->SetImageFullPath(imageFullPath);
		Gdiplus::PixelFormat format = pGdiplusBitmap->GetPixelFormat();
		imageInfo->SetAlpha((format & PixelFormatAlpha) != 0);

		if ((format & PixelFormatIndexed) != 0) {
			int nPalSize = pGdiplusBitmap->GetPaletteSize();
			if (nPalSize > 0) {
				Gdiplus::ColorPalette* palette = (Gdiplus::ColorPalette*)malloc(nPalSize);
				if (palette != nullptr) {
					status = pGdiplusBitmap->GetPalette(palette, nPalSize);
					if (status == Gdiplus::Ok) {
						imageInfo->SetAlpha((palette->Flags & Gdiplus::PaletteFlagsHasAlpha) != 0);
					}
					free(palette);
				}
			}
		}

		if (format == PixelFormat32bppARGB) {
			for (int nFrameIndex = 0; nFrameIndex < iFrameCount; nFrameIndex++) {
				IBitmap* pBitmap = imageInfo->GetBitmap(nFrameIndex);
				//TODO: ¸ÄÐ´
				Bitmap_GDI* pGdiBitmap = dynamic_cast<Bitmap_GDI*>(pBitmap);
				ASSERT(pGdiBitmap != nullptr);
				HBITMAP hBitmap = pGdiBitmap->GetHBitmap();
				BITMAP bm = { 0 };
				::GetObject(hBitmap, sizeof(bm), &bm);
				LPBYTE imageBits = (LPBYTE)bm.bmBits;
				if (imageBits == nullptr) {
					continue;
				}
				for (int i = 0; i < bm.bmHeight; ++i) {
					for (int j = 0; j < bm.bmWidthBytes; j += 4) {
						int x = i * bm.bmWidthBytes + j;
						if (imageBits[x + 3] != 255) {
							imageInfo->SetAlpha(true);
							return imageInfo;
						}
					}
				}
			}

			imageInfo->SetAlpha(false);
			return imageInfo;
		}

		return imageInfo;
    }
}

std::unique_ptr<ImageInfo> ImageDecoder::LoadImageFile(const std::wstring& imageFullPath)
{
	if (SvgUtil::IsSvgFile(imageFullPath)) {
		return SvgUtil::LoadSvg(imageFullPath);
	}

	std::unique_ptr<Gdiplus::Bitmap> gdiplusBitmap(Gdiplus::Bitmap::FromFile(imageFullPath.c_str()));
	return ImageLoader::LoadImageByBitmap(gdiplusBitmap, imageFullPath);
}

std::unique_ptr<ImageInfo> ImageDecoder::LoadImageData(std::vector<unsigned char>& file_data, const std::wstring& imageFullPath)
{
	if (SvgUtil::IsSvgFile(imageFullPath)) {
		return SvgUtil::LoadSvg(file_data, imageFullPath);
	}

	if (file_data.empty()) {
		return nullptr;
	}

	HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, file_data.size());
	if (hGlobal == nullptr) {
		return nullptr;
	}
	unsigned char* pData = (unsigned char*)::GlobalLock(hGlobal);
	if (pData == nullptr) {
		::GlobalFree(hGlobal);
		return nullptr;
	}
	memcpy(pData, file_data.data(), file_data.size());
	::GlobalUnlock(hGlobal);

	IStream* stream = NULL;
	::GlobalLock(hGlobal);
	::CreateStreamOnHGlobal(hGlobal, FALSE, &stream);
	if (stream == NULL) {
		::GlobalUnlock(hGlobal);
		::GlobalFree(hGlobal);
		return nullptr;
	}
	else {
		std::unique_ptr<Gdiplus::Bitmap> gdiplusBitmap(Gdiplus::Bitmap::FromStream(stream));
		stream->Release();
		::GlobalUnlock(hGlobal);
		::GlobalFree(hGlobal);
		return ImageLoader::LoadImageByBitmap(gdiplusBitmap, imageFullPath);
	}
}

} // namespace ui

