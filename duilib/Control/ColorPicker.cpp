#include "ColorPicker.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Control/ColorPickerRegular.h"
#include "duilib/Control/ColorPickerStatard.h"
#include "duilib/Control/ColorPickerStatardGray.h"
#include "duilib/Control/ColorPickerCustom.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Utils/ScreenCapture.h"

namespace ui
{
/** 颜色预览的控件
*/
class ColorPreviewLabel: public Label
{
    typedef Label BaseClass;
public:
    explicit ColorPreviewLabel(Window* pWindow): Label(pWindow)
    {
    }
    virtual ~ColorPreviewLabel() override = default;

    /** 绘制背景色，增加黑灰格子背景，用于透明颜色的预览
    */
    virtual void PaintBkColor(IRender* pRender) override
    {
        if (pRender == nullptr) {
            return;
        }
        //绘制透明棋盘的格子
        UiRect rc = GetRect();
        const int32_t nGridSize = Dpi().GetScaleInt(8);
        int32_t nRows = rc.Width() / nGridSize + 1;
        int32_t nCols = rc.Height() / nGridSize + 1;
        for (int32_t i = 0; i < nRows; ++i) {
            for (int32_t j = 0; j < nCols; ++j) {
                UiRect rect;
                rect.left = rc.left + i * nGridSize;
                rect.top = rc.top + j * nGridSize;
                rect.right = rect.left + nGridSize;
                rect.bottom = rect.top + nGridSize;
                if (j % 2) {
                    pRender->FillRect(rect, (i % 2) == 1 ? UiColor(UiColors::DarkGray) : UiColor(UiColors::White));
                }
                else {
                    pRender->FillRect(rect, (i % 2) == 0 ? UiColor(UiColors::DarkGray) : UiColor(UiColors::White));
                }
            }
        }

        //绘制实际显示的颜色
        BaseClass::PaintBkColor(pRender);
    }
};

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

DString ColorPicker::GetSkinFolder()
{
    return _T("public");
}

DString ColorPicker::GetSkinFile()
{
    return _T("color/color_picker.xml");
}

LRESULT ColorPicker::OnWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled)
{
    UiColor selectedColor;
    if (wParam == kWindowCloseOK) {
        //只有点击"确认"按钮的时候，才保存所选择的颜色
        if (m_pNewColor != nullptr) {
            DString bkColor = m_pNewColor->GetBkColor();
            if (!bkColor.empty()) {
                selectedColor = m_pNewColor->GetUiColor(bkColor);
            }
        }
    }    
    m_selectedColor = selectedColor;
    return BaseClass::OnWindowCloseMsg(wParam, nativeMsg, bHandled);
}

void ColorPicker::AttachSelectColor(const EventCallback& callback)
{
    m_colorCallback = callback;
}

void ColorPicker::AttachWindowClose(const EventCallback& callback)
{
    BaseClass::AttachWindowClose(callback);
}

Control* ColorPicker::CreateControl(const DString& strClass)
{
    if (strClass == _T("ColorPreviewLabel")) {
        return new ColorPreviewLabel(this);
    }
    return nullptr;
}

void ColorPicker::OnInitWindow()
{
    m_pNewColor = dynamic_cast<Label*>(FindControl(_T("color_picker_new_color")));
    m_pOldColor = dynamic_cast<Label*>(FindControl(_T("color_picker_old_color")));

    ASSERT(m_pNewColor != nullptr);
    ASSERT(m_pOldColor != nullptr);

    m_pRegularPicker = dynamic_cast<ColorPickerRegular*>(FindControl(_T("color_picker_regular")));
    m_pStatardPicker = dynamic_cast<ColorPickerStatard*>(FindControl(_T("color_picker_standard")));
    m_pStatardGrayPicker = dynamic_cast<ColorPickerStatardGray*>(FindControl(_T("color_picker_standard_gray")));
    m_pCustomPicker = dynamic_cast<ColorPickerCustom*>(FindControl(_T("color_picker_custom")));

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

    TabBox* pTabBox = dynamic_cast<TabBox*>(FindControl(_T("color_picker_tab")));
    if (pTabBox != nullptr) {
        pTabBox->AttachTabSelect([this](const ui::EventArgs& args) {
            UiColor selectedColor;
            if (m_pNewColor != nullptr) {
                DString bkColor = m_pNewColor->GetBkColor();
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
    Button* pButton = dynamic_cast<Button*>(FindControl(_T("color_picker_ok")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            this->CloseWnd(kWindowCloseOK);
            return true;
            });
    }
    //取消按钮
    pButton = dynamic_cast<Button*>(FindControl(_T("color_picker_cancel")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            this->CloseWnd(kWindowCloseCancel);
            return true;
            });
    }

    //选择：屏幕取色
    pButton = dynamic_cast<Button*>(FindControl(_T("color_picker_choose")));
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
        DString colorString = m_pNewColor->GetBkColor();
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
    typedef Label BaseClass;
public:
    explicit ScreenColorPreview(Window* pWindow):
        Label(pWindow)
    {
    }
    /** 绘制背景图片的入口函数
    * @param[in] pRender 指定绘制区域
    */
    virtual void PaintBkImage(IRender* pRender) override
    {
        BaseClass::PaintBkImage(pRender);
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
        if (pBitmap == nullptr) {
            return;
        }
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
        int32_t radius = Dpi().GetScaleInt(6);
        UiColor penColor = UiColor(UiColors::Brown);
        int32_t nWidth = Dpi().GetScaleInt(2);
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
    typedef Control BaseClass;
public:
    explicit ScreenColorPicker(Window* pWindow):
        Control(pWindow),
        m_cursorId(0),
        m_pColorPreview(nullptr)
    {
    }

    virtual ~ScreenColorPicker() override
    {
        m_pColorPreview = nullptr;
        m_selColor = UiColor();
    }

    /** 设置控件指定属性
     */
    virtual void SetAttribute(const DString& strName, const DString& strValue) override
    {
        if (strName == _T("cursor_file")) {
            m_cursorFile = strValue;
        }
        else {
            BaseClass::SetAttribute(strName, strValue);
        }
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
        BaseClass::PaintBkImage(pRender);
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
        if (m_cursorId != 0) {
            GlobalManager::Instance().Cursor().SetCursorByID(m_cursorId);
        }
        else if (!m_cursorFile.empty()) {
            if (GlobalManager::Instance().Cursor().SetImageCursor(GetWindow(), FilePath(m_cursorFile.c_str()))) {
                m_cursorId = GlobalManager::Instance().Cursor().GetCursorID();
            }
        }
        
        return true;
    }

    /** 鼠标左键按下，选择颜色
    */
    virtual bool ButtonDown(const EventArgs& msg) override
    {
        bool bRet = BaseClass::ButtonDown(msg);
        if (msg.IsSenderExpired()) {
            return false;
        }

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
        const int32_t offset = Dpi().GetScaleInt(36);
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
            DString text = m_pColorPreview->GetColorString(selColor);
            m_pColorPreview->SetText(text);
            m_pColorPreview->SetAttribute(_T("text_align"), _T("hcenter,vcenter"));
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
#ifdef DUILIB_BUILD_FOR_WIN
                //SDL_PIXELFORMAT_BGRA32
                selColor = UiColor(selColor.GetR(), selColor.GetG(), selColor.GetB());
#else
                //SDL_PIXELFORMAT_RGBA32
                selColor = UiColor(selColor.GetB(), selColor.GetG(), selColor.GetR());
#endif
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
                        ASSERT(destColorIndex < (int32_t)spBitmap->GetWidth() * (int32_t)spBitmap->GetHeight());
                        pDestPixelBits[destColorIndex++] = pPixelBits[colorXY];
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
    /** 光标ID
    */
    CursorID m_cursorId;

    /** 光标的文件名
    */
    UiString m_cursorFile;

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
    typedef WindowImplBase BaseClass;
public:
    ScreenColorPickerWnd(): m_pScreenColorPicker(nullptr)
    {
    }

    /** 以下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
     * GetSkinFolder        接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile            接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override { return _T("public");}
    virtual DString GetSkinFile() override { return _T("color/screen_color_picker.xml"); }

    /** 当要创建的控件不是标准的控件名称时会调用该函数
    * @param [in] strClass 控件名称
    * @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
    */
    virtual Control* CreateControl(const DString& strClass) override
    {
        if (strClass == _T("ScreenColorPicker")) {
            if (m_pScreenColorPicker == nullptr) {
                m_pScreenColorPicker = new ScreenColorPicker(this);
                if (m_spBitmap != nullptr) {
                    m_pScreenColorPicker->SetBitmap(m_spBitmap);
                    m_spBitmap.reset();
                }
            }
            return m_pScreenColorPicker;
        }
        else if (strClass == _T("ScreenColorPreview")) {
            ScreenColorPreview* pScreenColorPreview = new ScreenColorPreview(this);
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
        BaseClass::OnWindowExitFullScreen();
        CloseWnd();
    }

    /** 抓取屏幕位图
    */
    bool ScreenCapture(const Window* pWindow)
    {
        m_spBitmap = ScreenCapture::CaptureBitmap(pWindow);
        return m_spBitmap != nullptr;
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
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(FindControl(_T("color_picker_choose_hide")));
    if (pCheckBox != nullptr) {
        bHideWindow = pCheckBox->IsSelected();
    }
#ifdef DUILIB_BUILD_FOR_SDL
    bHideWindow = false;
#else
    //SDL的实现，如果窗口隐藏，则所有子窗口都会被隐藏，所以不能隐藏本窗口
    if (bHideWindow) {
        //隐藏本窗口
        ShowWindow(kSW_HIDE);

        //父窗口不隐藏
        Window* pParentWnd = GetParentWindow();
        if (pParentWnd != nullptr) {
            pParentWnd->ShowWindow(kSW_SHOW_NORMAL);
            pParentWnd->SetWindowForeground();
            pParentWnd->UpdateWindow();
        }
        UpdateWindow();
    }
#endif

    //抓取屏幕位图
    ScreenColorPickerWnd* pScreenColorPicker = new ScreenColorPickerWnd;    
    if (!pScreenColorPicker->ScreenCapture(this)) {
        delete pScreenColorPicker;
        return;
    }
    WindowCreateParam createWndParam;
    createWndParam.m_dwStyle = kWS_POPUP;
    createWndParam.m_dwExStyle = kWS_EX_TRANSPARENT;
    createWndParam.m_bCenterWindow = true;
    pScreenColorPicker->CreateWnd(nullptr, createWndParam);
    pScreenColorPicker->ShowWindow(ui::kSW_SHOW_NORMAL);
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
            this->ShowWindow(ui::kSW_SHOW_NORMAL);
            InvalidateAll();
            UpdateWindow();
            
            //父窗口重绘
            Window* pParentWnd = GetParentWindow();
            if (pParentWnd != nullptr) {
                pParentWnd->InvalidateAll();
                pParentWnd->UpdateWindow();
            }
        }
        return true;
        });
}

} // namespace ui
