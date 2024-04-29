#include "ColorPicker.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Control/ColorPickerRegular.h"
#include "duilib/Control/ColorPickerStatard.h"
#include "duilib/Control/ColorPickerStatardGray.h"
#include "duilib/Control/ColorPickerCustom.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

const std::wstring ColorPicker::kClassName = L"ColorPicker";

ColorPicker::ColorPicker():
	m_pNewColor(nullptr),
	m_pOldColor(nullptr),
	m_pRegularPicker(nullptr),
	m_pStatardPicker(nullptr),
	m_pStatardGrayPicker(nullptr),
	m_pCustomPicker(nullptr)
{
}

ColorPicker::~ColorPicker()
{
}

std::wstring ColorPicker::GetSkinFolder()
{
	return L"public";
}

std::wstring ColorPicker::GetSkinFile()
{
	return L"color/color_picker.xml";
}

std::wstring ColorPicker::GetWindowClassName() const
{
	return kClassName;
}

LRESULT ColorPicker::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	UiColor selectedColor;
	if (wParam == 0) {
		//只有点击"确认"按钮的时候，才保存所选择的颜色
		if (m_pNewColor != nullptr) {
			std::wstring bkColor = m_pNewColor->GetBkColor();
			if (!bkColor.empty()) {
				selectedColor = m_pNewColor->GetUiColor(bkColor);
			}
		}
	}	
	m_selectedColor = selectedColor;
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

void ColorPicker::AttachSelectColor(const EventCallback& callback)
{
	m_colorCallback = callback;
}

void ColorPicker::AttachWindowClose(const EventCallback& callback)
{
	__super::AttachWindowClose(callback);
}

void ColorPicker::OnInitWindow()
{
	m_pNewColor = dynamic_cast<Label*>(FindControl(L"color_picker_new_color"));
	m_pOldColor = dynamic_cast<Label*>(FindControl(L"color_picker_old_color"));

	ASSERT(m_pNewColor != nullptr);
	ASSERT(m_pOldColor != nullptr);

	m_pRegularPicker = dynamic_cast<ColorPickerRegular*>(FindControl(L"color_picker_regular"));
	m_pStatardPicker = dynamic_cast<ColorPickerStatard*>(FindControl(L"color_picker_standard"));
	m_pStatardGrayPicker = dynamic_cast<ColorPickerStatardGray*>(FindControl(L"color_picker_standard_gray"));
	m_pCustomPicker = dynamic_cast<ColorPickerCustom*>(FindControl(L"color_picker_custom"));

	if (m_pRegularPicker != nullptr) {
		m_pRegularPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			UiColor newColor((uint32_t)args.wParam);
			OnSelectColor(newColor);
			return true;
			});
	}
	if (m_pStatardPicker != nullptr) {
		m_pStatardPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			UiColor newColor((uint32_t)args.wParam);
			OnSelectColor(newColor);
			if (m_pStatardGrayPicker != nullptr) {
				m_pStatardGrayPicker->SelectColor(UiColor());
			}
			return true;
			});
	}
	if (m_pStatardGrayPicker != nullptr) {
		m_pStatardGrayPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			UiColor newColor((uint32_t)args.wParam);
			OnSelectColor(newColor);
			if (m_pStatardPicker != nullptr) {
				m_pStatardPicker->SelectColor(UiColor());
			}
			return true;
			});
	}
	if (m_pCustomPicker != nullptr) {
		m_pCustomPicker->AttachSelectColor([this](const ui::EventArgs& args) {
			UiColor newColor((uint32_t)args.wParam);
			OnSelectColor(newColor);
			return true;
			});
	}

	TabBox* pTabBox = dynamic_cast<TabBox*>(FindControl(L"color_picker_tab"));
	if (pTabBox != nullptr) {
		pTabBox->AttachTabSelect([this](const ui::EventArgs& args) {
			UiColor selectedColor;
			if (m_pNewColor != nullptr) {
				std::wstring bkColor = m_pNewColor->GetBkColor();
				if (!bkColor.empty()) {
					selectedColor = m_pNewColor->GetUiColor(bkColor);
				}				
			}
			if (args.wParam == 0) {
				//常用颜色
				if (m_pRegularPicker != nullptr) {
					m_pRegularPicker->SelectColor(selectedColor);
				}				
			}
			else if (args.wParam == 1) {
				//标准颜色
				if (m_pStatardPicker != nullptr) {
					m_pStatardPicker->SelectColor(selectedColor);
				}
				if (m_pStatardGrayPicker != nullptr) {
					m_pStatardGrayPicker->SelectColor(selectedColor);
				}
			}
			else if (args.wParam == 2) {
				//自定义颜色
				if (m_pCustomPicker != nullptr) {
					m_pCustomPicker->SelectColor(selectedColor);
				}
			}
			return true;
			});
	}

	//确定按钮
	Button* pButton = dynamic_cast<Button*>(FindControl(L"color_picker_ok"));
	if (pButton != nullptr) {
		pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
			this->CloseWnd(0);
			return true;
			});
	}
	//取消按钮
	pButton = dynamic_cast<Button*>(FindControl(L"color_picker_cancel"));
	if (pButton != nullptr) {
		pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
			this->CloseWnd(2);
			return true;
			});
	}

	//选择：屏幕取色
	pButton = dynamic_cast<Button*>(FindControl(L"color_picker_choose"));
	if (pButton != nullptr) {
		pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
			OnPickColorFromScreen();
			return true;
			});
	}
}

void ColorPicker::OnSelectColor(const UiColor& newColor)
{
	UiColor oldColor;
	if (m_pNewColor != nullptr) {
		std::wstring colorString = m_pNewColor->GetBkColor();
		if (!colorString.empty()) {
			oldColor = m_pNewColor->GetUiColor(colorString);
		}
		m_pNewColor->SetBkColor(newColor);
		m_pNewColor->SetText(m_pNewColor->GetBkColor());

		//文本颜色，使用反色
		UiColor textColor = UiColor(255 - newColor.GetR(), 255 - newColor.GetG(), 255 - newColor.GetB());
		m_pNewColor->SetStateTextColor(kControlStateNormal, m_pNewColor->GetColorString(textColor));
	}
	if (m_colorCallback != nullptr) {
		EventArgs args;
		args.wParam = newColor.GetARGB();
		args.lParam = oldColor.GetARGB();
		m_colorCallback(args);
	}
}

void ColorPicker::SetSelectedColor(const UiColor& color)
{
	if (m_pNewColor != nullptr) {
		m_pNewColor->SetBkColor(color);
		m_pNewColor->SetText(m_pNewColor->GetBkColor());

		//文本颜色，使用反色
		UiColor textColor = UiColor(255 - color.GetR(), 255 - color.GetG(), 255 - color.GetB());
		m_pNewColor->SetStateTextColor(kControlStateNormal, m_pNewColor->GetColorString(textColor));
	}
	if (m_pOldColor != nullptr) {
		m_pOldColor->SetBkColor(color);
		m_pOldColor->SetText(m_pOldColor->GetBkColor());

		//文本颜色，使用反色
		UiColor textColor = UiColor(255 - color.GetR(), 255 - color.GetG(), 255 - color.GetB());
		m_pOldColor->SetStateTextColor(kControlStateNormal, m_pOldColor->GetColorString(textColor));
	}
	if (m_pCustomPicker != nullptr) {
		m_pCustomPicker->SelectColor(color);
	}
	if (m_pRegularPicker != nullptr) {
		m_pRegularPicker->SelectColor(color);
	}
	if (m_pStatardPicker != nullptr) {
		m_pStatardPicker->SelectColor(color);
	}
	if (m_pStatardGrayPicker != nullptr) {
		m_pStatardGrayPicker->SelectColor(color);
	}
}

UiColor ColorPicker::GetSelectedColor() const
{
	return m_selectedColor;
}

/** 屏幕取色预览控件
*/
class ScreenColorPreview : public Label
{
public:
	/** 绘制背景图片的入口函数
	* @param[in] pRender 指定绘制区域
	*/
	virtual void PaintBkImage(IRender* pRender) override
	{
		__super::PaintBkImage(pRender);
		if (pRender == nullptr) {
			return;
		}
		if (m_spBitmap == nullptr) {
			return;
		}
		if (m_spBitmap->GetWidth() < 1) {
			return;
		}
		UiRect rc = GetRect();
		UiRect rcPaint = GetPaintRect();
		IBitmap* pBitmap = m_spBitmap.get();
		UiRect rcDest = rc;
		rcDest.bottom = rcDest.top + rcDest.Width() * pBitmap->GetHeight() / pBitmap->GetWidth(); //保持与原图的宽高比

		UiRect rcSource;
		rcSource.left = 0;
		rcSource.top = 0;
		rcSource.right = rcSource.left + pBitmap->GetWidth();
		rcSource.bottom = rcSource.top + pBitmap->GetHeight();

		uint8_t uFade = 255;
		IMatrix* pMatrix = nullptr;
		if (pBitmap != nullptr) {		
			pRender->DrawImageRect(rcPaint, pBitmap, rcDest, rcSource, uFade, pMatrix);
			if (GetTopBorderSize() > 0) {
				pRender->DrawRect(rcDest, GetUiColor(GetBorderColor(kControlStateNormal)), GetTopBorderSize());
			}
		}

		//画预览中心点的圆圈
		UiPoint centerPt = rcDest.Center();
		int32_t radius = GlobalManager::Instance().Dpi().GetScaleInt(6);
		UiColor penColor = UiColor(UiColors::Brown);
		int32_t nWidth = GlobalManager::Instance().Dpi().GetScaleInt(2);
		pRender->DrawCircle(centerPt, radius, penColor, nWidth);
	}

	/** 获取预览位图抓取的大小（宽度和高度）
	*/
	void GetPreviewBitmapSize(int32_t& nPreviewWidth, int32_t& nPreviewHeight) const
	{
		nPreviewWidth = GetWidth() / 16;
		if ((nPreviewWidth % 2) != 0) {
			nPreviewWidth += 1;
		}
		nPreviewHeight = nPreviewWidth / 2;//宽高比为2：1
		if ((nPreviewHeight % 2) != 0) {
			nPreviewHeight += 1;
		}
	}

	/** 设置预览位图
	*/
	void SetPreviewBitmap(const std::shared_ptr<IBitmap>& spBitmap)
	{
		m_spBitmap = spBitmap;
		Invalidate();
	}

private:
	/** 屏幕取色预览位图
	*/
	std::shared_ptr<IBitmap> m_spBitmap;
};

/** 屏幕取色控件
*/
class ScreenColorPicker : public Control
{
public:
	ScreenColorPicker():
		m_hCursor(nullptr),
		m_pColorPreview(nullptr)
	{
	}

	virtual ~ScreenColorPicker()
	{
		if (m_hCursor != nullptr) {
			::DestroyCursor(m_hCursor);
			m_hCursor = nullptr;
		}
		m_pColorPreview = nullptr;
		m_selColor = UiColor();
	}

	/** 设置屏幕位图
	*/
	void SetBitmap(const std::shared_ptr<IBitmap>& spBitmap)
	{
		m_spBitmap = spBitmap;
	}

	/** 设置预览控件的接口
	*/
	void SetColorPreview(ScreenColorPreview* pColorPreview)
	{
		m_pColorPreview = pColorPreview;
	}

	/** 获取选择的颜色值
	*/
	UiColor GetSelColor() const 
	{
		return m_selColor;
	}

private:
	/** 绘制背景图片的入口函数
	* @param[in] pRender 指定绘制区域
	*/
	virtual void PaintBkImage(IRender* pRender) override
	{
		__super::PaintBkImage(pRender);
		if (pRender == nullptr) {
			return;
		}
		if (m_spBitmap == nullptr) {
			return;
		}
		UiRect rc = GetRect();
		UiRect rcPaint = GetPaintRect();
		IBitmap* pBitmap = m_spBitmap.get();
		UiRect rcDest = rc;
		UiRect rcSource;
		rcSource.left = 0;
		rcSource.top = 0;
		rcSource.right = rcSource.left + rc.Width();
		rcSource.bottom = rcSource.top + rc.Height();
		uint8_t uFade = 255;
		IMatrix* pMatrix = nullptr;

		if (pBitmap != nullptr) {
			pRender->DrawImageRect(rcPaint, pBitmap, rcDest, rcSource, uFade, pMatrix);
		}
	}

	/** 设置光标形状
	*/
	virtual bool OnSetCursor(const EventArgs& /*msg*/) override
	{
		if (m_hCursor == nullptr) {
			std::wstring cursorPath = StringHelper::JoinFilePath(GlobalManager::Instance().GetResourcePath(), L"public\\color\\dropcur.cur");
			m_hCursor = (HCURSOR)::LoadImage(NULL, cursorPath.c_str(), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
		}
		if (m_hCursor != nullptr) {
			::SetCursor(m_hCursor);
		}
		return true;
	}

	/** 鼠标左键按下，选择颜色
	*/
	virtual bool ButtonDown(const EventArgs& msg)
	{
		bool bRet = __super::ButtonDown(msg);

		//更新选择颜色
		m_selColor = GetMousePosColor(msg.ptMouse);

		Window* pWindow = GetWindow();
		if (pWindow != nullptr) {
			pWindow->CloseWnd();
		}
		return bRet;
	}

	/** 鼠标移动, 更新光标所在位置的颜色到预览控件
	*/
	virtual bool MouseMove(const EventArgs& msg) override
	{
		if (m_pColorPreview == nullptr) {
			return true;
		}
		const int32_t offset = ui::GlobalManager::Instance().Dpi().GetScaleInt(36);
		const UiRect rcPickker = GetRect();
		const UiRect rcPreview = m_pColorPreview->GetRect();
		UiRect rcPreviewNew;
		if ((msg.ptMouse.x + offset + rcPreview.Width()) > rcPickker.right) {
			//在左边显示
			rcPreviewNew.left = msg.ptMouse.x - offset - rcPreview.Width();
		}
		else {
			//在右边显示
			rcPreviewNew.left = msg.ptMouse.x + offset;				
		}
		rcPreviewNew.right = rcPreviewNew.left + rcPreview.Width();

		if ((msg.ptMouse.y + offset + rcPreview.Height()) > rcPickker.bottom) {
			//在上边显示
			rcPreviewNew.top = msg.ptMouse.y - offset - rcPreview.Height();
		}
		else {
			//在下边显示
			rcPreviewNew.top = msg.ptMouse.y + offset;
		}
		rcPreviewNew.bottom = rcPreviewNew.top + rcPreview.Height();
		m_pColorPreview->SetPos(rcPreviewNew);

		//更新预览位图
		std::shared_ptr<IBitmap> spBitmap = GetMousePosBitmap(msg.ptMouse);
		m_pColorPreview->SetPreviewBitmap(spBitmap);

		//更新预览颜色
		UiColor selColor = GetMousePosColor(msg.ptMouse);
		if (!selColor.IsEmpty()) {
			//设置背景颜色
			m_pColorPreview->SetBkColor(selColor);

			//设置文本
			std::wstring text = m_pColorPreview->GetColorString(selColor);
			m_pColorPreview->SetText(text);
			m_pColorPreview->SetAttribute(L"text_align", L"hcenter,vcenter");
			m_pColorPreview->SetTextPadding(UiPadding(0, m_pColorPreview->GetHeight() / 2, 0, 0), false);
			//设置文本颜色
			UiColor textColor = UiColor(255 - selColor.GetR(), 255 - selColor.GetG(), 255 - selColor.GetB());
			m_pColorPreview->SetStateTextColor(kControlStateNormal, m_pColorPreview->GetColorString(textColor));
		}
		return true;
	}

	/** 获取鼠标所在位置的颜色值
	*/
	UiColor GetMousePosColor(const UiPoint& pt) const
	{
		UiColor selColor;
		const UiRect rcPickker = GetRect();
		if (m_spBitmap != nullptr) {
			uint32_t* pPixelBits = (uint32_t*)m_spBitmap->LockPixelBits();
			if (pPixelBits != nullptr) {
				const int32_t nWidth = (int32_t)m_spBitmap->GetWidth();
				const int32_t nHeight = (int32_t)m_spBitmap->GetHeight();
				int32_t nColumn = pt.x - rcPickker.left;
				int32_t nRow = pt.y - rcPickker.top;
				if (nColumn >= nWidth) {
					nColumn = nWidth - 1;
				}
				if (nColumn < 0) {
					nColumn = 0;
				}
				if (nRow >= nHeight) {
					nRow = nHeight - 1;
				}
				if (nRow < 0) {
					nRow = 0;
				}
				int32_t colorXY = nRow * nWidth + nColumn;
				ASSERT(colorXY < nWidth * nHeight);
				uint32_t colorValue = pPixelBits[colorXY];
				selColor = UiColor(colorValue);
				selColor = UiColor(selColor.GetR(), selColor.GetG(), selColor.GetB());
			}
			m_spBitmap->UnLockPixelBits();
		}
		return selColor;
	}

	/** 获取鼠标所在位置周围的位图
	*/
	std::shared_ptr<IBitmap> GetMousePosBitmap(const UiPoint& pt) const
	{
		std::shared_ptr<IBitmap> spBitmap;
		if (m_pColorPreview == nullptr) {
			return spBitmap;
		}
		int32_t nPreviewWidth = 0;
		int32_t nPreviewHeight = 0;
		m_pColorPreview->GetPreviewBitmapSize(nPreviewWidth, nPreviewHeight);
		if ((nPreviewWidth <= 0) || (nPreviewHeight <= 0)) {
			return spBitmap;
		}

		IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
		ASSERT(pRenderFactory != nullptr);
		if (pRenderFactory != nullptr) {
			spBitmap.reset(pRenderFactory->CreateBitmap());
		}
		if (spBitmap == nullptr) {
			return spBitmap;
		}
		if (!spBitmap->Init(nPreviewWidth, nPreviewHeight, true, nullptr)) {
			return nullptr;
		}
		uint32_t* pDestPixelBits = (uint32_t*)spBitmap->LockPixelBits();
		if (pDestPixelBits == nullptr) {
			return nullptr;
		}

		int32_t destColorIndex = 0;
		const UiRect rcPickker = GetRect();
		if (m_spBitmap != nullptr) {
			uint32_t* pPixelBits = (uint32_t*)m_spBitmap->LockPixelBits();
			if (pPixelBits != nullptr) {
				const int32_t nWidth = (int32_t)m_spBitmap->GetWidth();
				const int32_t nHeight = (int32_t)m_spBitmap->GetHeight();
				for (int32_t y = pt.y - nPreviewHeight / 2; y < (pt.y + nPreviewHeight / 2); ++y) {
					for (int32_t x = pt.x - nPreviewWidth / 2; x < (pt.x + nPreviewWidth / 2); ++x ) {					
						int32_t nColumn = x - rcPickker.left;
						int32_t nRow = y - rcPickker.top;
						if (nColumn >= nWidth) {
							nColumn = nWidth - 1;
						}
						if (nColumn < 0) {
							nColumn = 0;
						}
						if (nRow >= nHeight) {
							nRow = nHeight - 1;
						}
						if (nRow < 0) {
							nRow = 0;
						}
						int32_t colorXY = nRow * nWidth + nColumn;
						ASSERT(colorXY < nWidth * nHeight);
						pDestPixelBits[++destColorIndex] = pPixelBits[colorXY];
					}
				}
			}
			ASSERT(destColorIndex == (int32_t)(spBitmap->GetWidth() * spBitmap->GetHeight()));
			m_spBitmap->UnLockPixelBits();
		}
		spBitmap->UnLockPixelBits();
		return spBitmap;
	}

private:
	/** 光标句柄
	*/
	HCURSOR m_hCursor;

	/** 屏幕位图
	*/
	std::shared_ptr<IBitmap> m_spBitmap;

	/** 预览控件的接口
	*/
	ScreenColorPreview* m_pColorPreview;

	/** 选择的颜色
	*/
	UiColor m_selColor;
};

/** 从屏幕取色的窗口（全屏）
*/
class ScreenColorPickerWnd : public WindowImplBase
{
public:
	ScreenColorPickerWnd(): m_pScreenColorPicker(nullptr)
	{
	}

	/** 以下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
	 * GetSkinFolder		接口设置你要绘制的窗口皮肤资源路径
	 * GetSkinFile			接口设置你要绘制的窗口的 xml 描述文件
	 * GetWindowClassName	接口设置窗口唯一的类名称
	 */
	virtual std::wstring GetSkinFolder() override { return L"public";}
	virtual std::wstring GetSkinFile() override { return L"color/screen_color_picker.xml"; }
	virtual std::wstring GetWindowClassName() const override { return L"ScreenColorPicker"; }

	/** 当要创建的控件不是标准的控件名称时会调用该函数
	* @param [in] strClass 控件名称
	* @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
	*/
	virtual Control* CreateControl(const std::wstring& strClass)
	{
		if (strClass == L"ScreenColorPicker") {
			if (m_pScreenColorPicker == nullptr) {
				m_pScreenColorPicker = new ScreenColorPicker;
				if (m_spBitmap != nullptr) {
					m_pScreenColorPicker->SetBitmap(m_spBitmap);
					m_spBitmap.reset();
				}
			}
			return m_pScreenColorPicker;
		}
		else if (strClass == L"ScreenColorPreview") {
			ScreenColorPreview* pScreenColorPreview = new ScreenColorPreview;
			if (m_pScreenColorPicker != nullptr) {
				m_pScreenColorPicker->SetColorPreview(pScreenColorPreview);
			}
			return pScreenColorPreview;
		}
		return nullptr;
	}

	/** 退出全屏状态
	*/
	virtual void OnWindowExitFullScreen() override
	{
		__super::OnWindowExitFullScreen();
		CloseWnd();
	}

	/** 窗口类名
	*/
	static std::wstring ClassName() { return L"ScreenColorPicker"; }

	/** 抓取屏幕位图
	*/
	void ScreenCapture()
	{
		std::shared_ptr<IBitmap> spBitmap;
		IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
		ASSERT(pRenderFactory != nullptr);
		if (pRenderFactory != nullptr) {
			spBitmap.reset(pRenderFactory->CreateBitmap());
		}
		if (spBitmap == nullptr) {
			return;
		}

		//抓取屏幕位图
		int32_t xScreen = GetSystemMetrics(SM_XVIRTUALSCREEN);
		int32_t yScreen = GetSystemMetrics(SM_YVIRTUALSCREEN);
		int32_t cxScreen = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int32_t cyScreen = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		if ((cxScreen <= 0) || (cyScreen <= 0)) {
			return;
		}
		HDC hdcSrc = ::GetDC(NULL); // 获取屏幕句柄
		if (hdcSrc == nullptr) {
			return;
		}
		HDC hdcDst = ::CreateCompatibleDC(hdcSrc); // 创建一个兼容屏幕的DC
		if (hdcDst == nullptr) {
			::ReleaseDC(NULL, hdcSrc);
			return;
		}

		LPVOID pBits = nullptr;
		HBITMAP hBitmap = CreateBitmap(cxScreen, cyScreen, true, &pBits);
		if (hBitmap == nullptr) {
			::ReleaseDC(NULL, hdcSrc); // 释放句柄
			::DeleteDC(hdcDst);
			return;
		}
		::SelectObject(hdcDst, hBitmap);
		::BitBlt(hdcDst, 0, 0, cxScreen, cyScreen, hdcSrc, xScreen, yScreen, SRCCOPY); // 复制屏幕内容到位图
		::ReleaseDC(NULL, hdcSrc); // 释放句柄
		::DeleteDC(hdcDst);

		if (!spBitmap->Init(cxScreen, cyScreen, true, pBits)) {
			spBitmap.reset();
		}
		::DeleteObject(hBitmap);
		m_spBitmap = spBitmap;
	}

	/** 获取选择的颜色值
	*/
	UiColor GetSelColor() const
	{
		UiColor selColor;
		if (m_pScreenColorPicker != nullptr) {
			selColor = m_pScreenColorPicker->GetSelColor();
		}
		return selColor;
	}

private:

	/** 创建位图
	*/
	HBITMAP CreateBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits)
	{
		ASSERT((nWidth > 0) && (nHeight > 0));
		if (nWidth == 0 || nHeight == 0) {
			return nullptr;
		}

		BITMAPINFO bmi = { 0 };
		::ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = nWidth;
		if (flipHeight) {
			bmi.bmiHeader.biHeight = -nHeight;//负数表示位图方向：从上到下，左上角为圆点
		}
		else {
			bmi.bmiHeader.biHeight = nHeight; //正数表示位图方向：从下到上，左下角为圆点
		}
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = nWidth * nHeight * sizeof(DWORD);

		HBITMAP hBitmap = nullptr;
		HDC hdc = ::GetDC(NULL);
		ASSERT(hdc != nullptr);
		if (hdc != nullptr) {
			hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, pBits, NULL, 0);
		}
		::ReleaseDC(NULL, hdc);
		return hBitmap;
	}

private:
	/** 位图显示控件
	*/
	ScreenColorPicker* m_pScreenColorPicker;

	/** 屏幕位图
	*/
	std::shared_ptr<IBitmap> m_spBitmap;
};

void ColorPicker::OnPickColorFromScreen()
{
	bool bHideWindow = true;
	CheckBox* pCheckBox = dynamic_cast<CheckBox*>(FindControl(L"color_picker_choose_hide"));
	if (pCheckBox != nullptr) {
		bHideWindow = pCheckBox->IsSelected();
	}
	if (bHideWindow) {
		//隐藏主窗口
		ShowWindow(false, false);
	}

	//抓取屏幕位图
	ScreenColorPickerWnd* pScreenColorPicker = new ScreenColorPickerWnd;	
	pScreenColorPicker->ScreenCapture();	
	pScreenColorPicker->CreateWnd(GetHWND(), ScreenColorPickerWnd::ClassName().c_str(), UI_WNDSTYLE_FRAME, WS_EX_TRANSPARENT);
	pScreenColorPicker->CenterWindow();
	pScreenColorPicker->ShowWindow();
	pScreenColorPicker->EnterFullScreen();
	pScreenColorPicker->AttachWindowClose([this, pScreenColorPicker, bHideWindow](const ui::EventArgs& /*args*/) {
		//更新选择的颜色值
		UiColor selectedColor = pScreenColorPicker->GetSelColor();
		if (!selectedColor.IsEmpty()) {
			//更新选择的颜色
			this->OnSelectColor(selectedColor);
			//更新常用颜色
			if (m_pRegularPicker != nullptr) {
				m_pRegularPicker->SelectColor(selectedColor);
			}
			//更新标准颜色
			if (m_pStatardPicker != nullptr) {
				m_pStatardPicker->SelectColor(selectedColor);
			}
			if (m_pStatardGrayPicker != nullptr) {
				m_pStatardGrayPicker->SelectColor(selectedColor);
			}
			//更新自定义颜色
			if (m_pCustomPicker != nullptr) {
				m_pCustomPicker->SelectColor(selectedColor);
			}
		}
		if (bHideWindow) {
			//显示主窗口
			this->ShowWindow(true, true);
		}
		return true;
		});
}

} // namespace ui
