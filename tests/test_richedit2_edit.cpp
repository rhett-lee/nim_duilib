// RichEdit2 编辑功能的单元测试
//
// 覆盖范围：
//   1) 文本替换：SetText / AppendText 后内容、行数、光标定位的一致性；
//   2) 插入字符：InsertText 与键盘输入（OnChar）；
//   3) 插入行：回车键在光标处插入换行符；
//   4) 删除字符：退格键（kVK_BACK）与删除键（kVK_DELETE）；
//   5) 删除行：选中整行（含换行符）后删除；
//   6) 替换文本行：选中一行文本后替换；
//   7) 方向键：左/右/上/下移动光标，以及"按下再按下"的往返一致性；
//   8) 滚动切换视图：滚动偏移改变后，按可视区域构建的绘制缓存与全量缓存逐像素一致，
//      且滚动后的画面等于滚动前的画面按偏移量平移；
//   9) 每次编辑后重绘（RichEdit2::Paint）不崩溃，且光标位置随内容正确变化；
//  10) 系统剪贴板：Copy/Cut/Paste/CanPaste 的内容与选区变化，以及与只读、
//      密码、单行、数字、字符集、长度限制等选项的交互；
//  11) 文本查找：FindRichText 的正反向查找、大小写与全字匹配、查找范围边界与异常参数。
//
// 说明：测试使用离屏渲染 + 轻量窗口桩对象，不创建真实窗口，可在命令行（含 CI）执行。
// 滚动条需要真实窗口才会生效，因此第 8 项在绘制缓存层验证"按可视区域构建 + 滚动绘制"。

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Core/Window.h"
#include "duilib/Control/RichEdit2.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/Clipboard.h"
#include "duilib/Utils/FilePath.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#if defined(_WIN32)
    #include <windows.h>
#endif

#ifndef WM_CHAR
    #define WM_CHAR 0x0102
#endif

namespace {

int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const std::function<std::string()>& message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message() << std::endl;
    }
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })
#define CHECK_MSG(condition, msg) Check((condition), [&]() { return std::string(msg); })

/** 把文本转换为便于在日志中阅读的形式（换行显示为 \n） */
std::string ToReadable(const DStringW& text)
{
    std::string result;
    for (const wchar_t ch : text) {
        if (ch == L'\n') {
            result += "\\n";
        }
        else if (ch == L'\r') {
            result += "\\r";
        }
        else if (ch == L'\t') {
            result += "\\t";
        }
        else if ((ch >= 0x20) && (ch < 0x7F)) {
            result.push_back((char)ch);
        }
        else {
            char buf[16] = { 0 };
            ::snprintf(buf, sizeof(buf), "\\u%04X", (uint32_t)ch);
            result += buf;
        }
    }
    return result;
}

/** 离屏渲染的窗口桩对象：只为控件提供 IRender */
class TestWindow : public ui::Window
{
public:
    explicit TestWindow(ui::IRender* pRender) : m_pRender(pRender) {}
    virtual ui::IRender* GetRender() const override { return m_pRender; }
private:
    ui::IRender* m_pRender;
};

/** 暴露控件的输入与绘制接口，便于在测试中直接驱动 */
class TestRichEdit2 : public ui::RichEdit2
{
public:
    explicit TestRichEdit2(ui::Window* pWindow) : ui::RichEdit2(pWindow) {}
    using ui::RichEdit2::Paint;
    using ui::RichEdit2::OnKeyDown;
    using ui::RichEdit2::OnKeyUp;
    using ui::RichEdit2::OnChar;
    using ui::RichEdit2::ButtonDown;
    using ui::RichEdit2::ButtonUp;
    using ui::RichEdit2::ButtonDoubleClick;
    using ui::RichEdit2::MouseMove;
    using ui::RichEdit2::FindRichText;
};

constexpr int32_t kWidth = 800;
constexpr int32_t kHeight = 600;

/** 测试环境：离屏渲染 + 窗口桩 + RichEdit2 */
struct TestEnv
{
    std::unique_ptr<ui::IRender> m_pRender;
    std::unique_ptr<TestWindow> m_pWindow;
    std::unique_ptr<TestRichEdit2> m_pRichEdit;
    ui::UiRect m_rcControl;

    ui::IRender* Render() const { return m_pRender.get(); }
    TestRichEdit2& Edit() const { return *m_pRichEdit; }
};

bool InitEnv(TestEnv& env)
{
    env.m_pRender.reset();
    env.m_pWindow.reset();
    env.m_pRichEdit.reset();

    ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory == nullptr) {
        return false;
    }
    env.m_pRender = pRenderFactory->CreateOffscreenRender(0, 0);
    if ((env.m_pRender == nullptr) || !env.m_pRender->Resize(kWidth, kHeight)) {
        return false;
    }

    //配置字体回退，保证中英文与 emoji 都能正常绘制
    ui::IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
    DString fallbackFontNames;
    for (const DString& fontName : std::vector<DString>{ _T("Microsoft YaHei"), _T("SimSun"), _T("Segoe UI Emoji") }) {
        if (!pFontMgr->HasFontName(fontName)) {
            continue;
        }
        if (!fallbackFontNames.empty()) {
            fallbackFontNames += _T(",");
        }
        fallbackFontNames += fontName;
    }
    ui::GlobalManager::Instance().Font().SetFallbackFontFamilyNames(fallbackFontNames);

    env.m_pWindow.reset(new TestWindow(env.m_pRender.get()));
    env.m_pRichEdit.reset(new TestRichEdit2(env.m_pWindow.get()));
    env.m_pRichEdit->SetWindow(env.m_pWindow.get());
    env.m_pRichEdit->Init();

    ui::UiFont fontInfo;
    fontInfo.m_fontName = _T("Microsoft YaHei");
    fontInfo.m_fontSize = 16;
    env.m_pRichEdit->SetFontInfo(fontInfo);
    env.m_pRichEdit->SetTextColor(_T("#FF202020"));
    env.m_pRichEdit->SetTextPadding(ui::UiPadding(4, 4, 4, 4), false);
    env.m_pRichEdit->SetMultiLine(true);
    env.m_pRichEdit->SetWordWrap(true);
    env.m_pRichEdit->SetWantReturn(true);

    env.m_rcControl = ui::UiRect(0, 0, kWidth, kHeight);
    env.m_pRichEdit->SetPos(env.m_rcControl);
    return true;
}

/** 模拟按键（KeyDown） */
void SendKey(TestRichEdit2& richEdit, ui::VirtualKeyCode vkCode, uint32_t modifierKey = 0)
{
    ui::EventArgs msg;
    msg.eventType = ui::kEventKeyDown;
    msg.vkCode = vkCode;
    msg.modifierKey = modifierKey;
    msg.wParam = 0;
    richEdit.OnKeyDown(msg);
}

/** 模拟字符输入（WM_CHAR） */
void SendChar(TestRichEdit2& richEdit, wchar_t ch)
{
    ui::EventArgs msg;
    msg.eventType = ui::kEventChar;
    msg.eventData = WM_CHAR;
    msg.wParam = (WPARAM)(uint16_t)ch;
    richEdit.OnChar(msg);
}

/** 模拟按键抬起（KeyUp），用于清除 Shift/Ctrl 的选择起始位置 */
void SendKeyUp(TestRichEdit2& richEdit, ui::VirtualKeyCode vkCode)
{
    ui::EventArgs msg;
    msg.eventType = ui::kEventKeyUp;
    msg.vkCode = vkCode;
    msg.wParam = 0;
    richEdit.OnKeyUp(msg);
}

/** 构造鼠标事件的参数（必须设置 Sender，否则控件会认为事件已失效） */
ui::EventArgs MakeMouseArgs(TestRichEdit2& richEdit, ui::EventType eventType,
                            const ui::UiPoint& ptMouse, uint32_t modifierKey = 0)
{
    ui::EventArgs msg;
    msg.eventType = eventType;
    msg.ptMouse = ptMouse;
    msg.modifierKey = modifierKey;
    msg.SetSender(&richEdit);
    return msg;
}

/** 模拟鼠标左键按下 */
void SendMouseDown(TestRichEdit2& richEdit, const ui::UiPoint& ptMouse, uint32_t modifierKey = 0)
{
    const ui::EventArgs msg = MakeMouseArgs(richEdit, ui::kEventMouseButtonDown, ptMouse, modifierKey);
    richEdit.ButtonDown(msg);
}

/** 模拟鼠标移动 */
void SendMouseMove(TestRichEdit2& richEdit, const ui::UiPoint& ptMouse, uint32_t modifierKey = 0)
{
    const ui::EventArgs msg = MakeMouseArgs(richEdit, ui::kEventMouseMove, ptMouse, modifierKey);
    richEdit.MouseMove(msg);
}

/** 模拟鼠标左键抬起 */
void SendMouseUp(TestRichEdit2& richEdit, const ui::UiPoint& ptMouse, uint32_t modifierKey = 0)
{
    const ui::EventArgs msg = MakeMouseArgs(richEdit, ui::kEventMouseButtonUp, ptMouse, modifierKey);
    richEdit.ButtonUp(msg);
}

/** 模拟鼠标左键双击 */
void SendMouseDoubleClick(TestRichEdit2& richEdit, const ui::UiPoint& ptMouse, uint32_t modifierKey = 0)
{
    const ui::EventArgs msg = MakeMouseArgs(richEdit, ui::kEventMouseDoubleClick, ptMouse, modifierKey);
    richEdit.ButtonDoubleClick(msg);
}

#if defined(_WIN32)
/** 在当前线程的键盘状态表中临时按下某个键，用于测试 Shift/Ctrl 组合键
 *
 * 说明：ui::Window::IsKeyDown 对 key_down/char 事件只识别 Alt/FirstPress 等标志，
 * Shift/Ctrl 状态是从当前线程的键盘状态表读取的（GetKeyState），
 * 因此需要用 SetKeyboardState 模拟，构造完 EventArgs 中的 modifierKey 并不生效。
 */
class ScopedKeyPressed
{
public:
    explicit ScopedKeyPressed(int32_t vkCode) : m_vkCode(vkCode)
    {
        ::GetKeyboardState(m_oldState);
        BYTE newState[256] = { 0 };
        ::GetKeyboardState(newState);
        newState[m_vkCode] = (BYTE)(newState[m_vkCode] | 0x80);
        m_bSetOk = (::SetKeyboardState(newState) != FALSE);
    }
    ~ScopedKeyPressed()
    {
        ::SetKeyboardState(m_oldState);
    }
    bool IsEffective() const
    {
        return m_bSetOk && ((::GetKeyState(m_vkCode) & 0x8000) != 0);
    }
private:
    int32_t m_vkCode;
    BYTE m_oldState[256] = { 0 };
    bool m_bSetOk = false;
};
#endif

#if defined(_WIN32)
/** 系统剪贴板内容的备份与恢复（RAII）
 *
 * 按键值对剪贴板的所有格式做备份，测试结束（包括提前返回）时按原格式恢复，
 * 避免单元测试破坏用户剪贴板中的文本、图片、文件等数据。
 * 无法识别/无法备份的格式（如 CF_METAFILEPICT、延迟渲染格式）会被跳过。
 */
class ClipboardBackup
{
public:
    ClipboardBackup()
    {
        if (!::OpenClipboard(nullptr)) {
            return;
        }
        m_bAvailable = true;
        UINT nFormat = 0;
        while ((nFormat = ::EnumClipboardFormats(nFormat)) != 0) {
            HANDLE hData = ::GetClipboardData(nFormat);
            if (hData == nullptr) {
                //延迟渲染的格式无法直接备份，跳过
                continue;
            }
            FormatData data;
            data.m_nFormat = nFormat;
            if (!CopyFormat(nFormat, hData, data)) {
                continue;
            }
            m_formatList.push_back(std::move(data));
        }
        ::CloseClipboard();
    }

    ~ClipboardBackup()
    {
        Restore();
        for (const FormatData& data : m_formatList) {
            if (data.m_hBitmap != nullptr) {
                ::DeleteObject(data.m_hBitmap);
            }
            if (data.m_hMetaFile != nullptr) {
                ::DeleteEnhMetaFile(data.m_hMetaFile);
            }
        }
    }

    ClipboardBackup(const ClipboardBackup&) = delete;
    ClipboardBackup& operator=(const ClipboardBackup&) = delete;

    /** 备份时是否可以访问剪贴板（不可访问时测试不会修改剪贴板，也无需恢复） */
    bool IsAvailable() const { return m_bAvailable; }

private:
    struct FormatData
    {
        UINT m_nFormat = 0;
        std::vector<uint8_t> m_bytes;      //HGLOBAL 格式的数据副本（文本、文件、DIB 等）
        std::vector<uint8_t> m_logPalette; //CF_PALETTE 的 LOGPALETTE 数据
        HBITMAP m_hBitmap = nullptr;       //CF_BITMAP 的位图副本
        HENHMETAFILE m_hMetaFile = nullptr;//CF_ENHMETAFILE 的图元文件副本
    };

    bool CopyFormat(UINT nFormat, HANDLE hData, FormatData& data)
    {
        if (nFormat == CF_BITMAP) {
            data.m_hBitmap = (HBITMAP)::CopyImage(hData, IMAGE_BITMAP, 0, 0, 0);
            return data.m_hBitmap != nullptr;
        }
        if (nFormat == CF_ENHMETAFILE) {
            data.m_hMetaFile = ::CopyEnhMetaFileW((HENHMETAFILE)hData, nullptr);
            return data.m_hMetaFile != nullptr;
        }
        if (nFormat == CF_PALETTE) {
            const HPALETTE hPalette = (HPALETTE)hData;
            const UINT nCount = ::GetPaletteEntries(hPalette, 0, 0, nullptr);
            if (nCount == 0) {
                return false;
            }
            const size_t nBytes = sizeof(LOGPALETTE) + ((size_t)nCount - 1) * sizeof(PALETTEENTRY);
            data.m_logPalette.assign(nBytes, 0);
            LOGPALETTE* pLogPalette = (LOGPALETTE*)data.m_logPalette.data();
            pLogPalette->palVersion = 0x0300;
            pLogPalette->palNumEntries = (WORD)nCount;
            return ::GetPaletteEntries(hPalette, 0, nCount, pLogPalette->palPalEntry) == nCount;
        }
        if (nFormat == CF_METAFILEPICT) {
            //METAFILEPICT 内含图元文件句柄，备份与恢复较复杂，跳过
            return false;
        }
        const SIZE_T nSize = ::GlobalSize(hData);
        if (nSize == 0) {
            return false;
        }
        const void* pData = ::GlobalLock(hData);
        if (pData == nullptr) {
            return false;
        }
        data.m_bytes.resize(nSize);
        ::memcpy(data.m_bytes.data(), pData, nSize);
        ::GlobalUnlock(hData);
        return true;
    }

    void Restore()
    {
        if (!m_bAvailable) {
            return;
        }
        if (!::OpenClipboard(nullptr)) {
            return;
        }
        ::EmptyClipboard();
        for (const FormatData& data : m_formatList) {
            if (data.m_hBitmap != nullptr) {
                HBITMAP hBitmap = (HBITMAP)::CopyImage(data.m_hBitmap, IMAGE_BITMAP, 0, 0, 0);
                if ((hBitmap != nullptr) && (::SetClipboardData(CF_BITMAP, hBitmap) == nullptr)) {
                    ::DeleteObject(hBitmap);
                }
            }
            else if (data.m_hMetaFile != nullptr) {
                HENHMETAFILE hMetaFile = ::CopyEnhMetaFileW(data.m_hMetaFile, nullptr);
                if ((hMetaFile != nullptr) && (::SetClipboardData(CF_ENHMETAFILE, hMetaFile) == nullptr)) {
                    ::DeleteEnhMetaFile(hMetaFile);
                }
            }
            else if (!data.m_logPalette.empty()) {
                HPALETTE hPalette = ::CreatePalette((const LOGPALETTE*)data.m_logPalette.data());
                if ((hPalette != nullptr) && (::SetClipboardData(CF_PALETTE, hPalette) == nullptr)) {
                    ::DeleteObject(hPalette);
                }
            }
            else if (!data.m_bytes.empty()) {
                HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, data.m_bytes.size());
                if (hMem == nullptr) {
                    continue;
                }
                void* pDst = ::GlobalLock(hMem);
                if (pDst == nullptr) {
                    ::GlobalFree(hMem);
                    continue;
                }
                ::memcpy(pDst, data.m_bytes.data(), data.m_bytes.size());
                ::GlobalUnlock(hMem);
                if (::SetClipboardData(data.m_nFormat, hMem) == nullptr) {
                    ::GlobalFree(hMem);
                }
            }
        }
        ::CloseClipboard();
    }

private:
    bool m_bAvailable = false;
    std::vector<FormatData> m_formatList;
};
#else
/** 非 Windows 平台：剪贴板只支持文本，按文本备份与恢复 */
class ClipboardBackup
{
public:
    ClipboardBackup() { ui::Clipboard::GetClipboardText(m_oldText); }
    ~ClipboardBackup() { ui::Clipboard::SetClipboardText(m_oldText); }
    bool IsAvailable() const { return true; }
private:
    DStringW m_oldText;
};
#endif

/** 把控件恢复到本组测试的默认状态（每个新增用例开头调用） */
void ResetEdit(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();
    richEdit.SetReadOnly(false);
    richEdit.SetMultiLine(true);
    richEdit.SetWordWrap(true);
    richEdit.SetWantReturn(true);
    richEdit.SetWantTab(false);
    richEdit.SetWantCtrlReturn(true);
    richEdit.SetPasswordMode(false);
    richEdit.SetShowPassword(false);
    richEdit.SetNumberOnly(false);
    richEdit.SetMinNumber((std::numeric_limits<int64_t>::min)());
    richEdit.SetMaxNumber((std::numeric_limits<int64_t>::max)());
    richEdit.SetLimitText(0);
    richEdit.SetLimitChars(DString());
    richEdit.SetTextHAlignType(ui::HorAlignType::kAlignLeft);
    richEdit.SetTextVAlignType(ui::VerAlignType::kAlignTop);
    richEdit.SetRowSpacingMul(1.0f);
    richEdit.SetRowSpacingAdd(0.0f);
    richEdit.SetZoomPercent(100);
    richEdit.SetEnableDragOut(true);
    richEdit.SetUndoLimit(64);
    //先制造一次有效的滚动事件，把文本控件内部缓存的滚动偏移重置为 0
    //（源码：ScrollBox 删除滚动条后，再调用 SetScrollPos 不会触发 OnScrollOffsetChanged，
    //  RichEditData 中缓存的滚动偏移会保留，导致后续 PosFromChar 坐标整体偏移）
    richEdit.EnableScrollBar(true, false);
    ui::ScrollBar* pVScrollBar = richEdit.GetVScrollBar();
    if (pVScrollBar != nullptr) {
        pVScrollBar->SetScrollRange(1000);
        richEdit.SetScrollPosY(100);
        richEdit.SetScrollPosY(0);
    }
    richEdit.EnableScrollBar(false, false);
    richEdit.SetText(DString());
    richEdit.EmptyUndoBuffer();
}

/** 重绘一帧（并清空背景） */
void PaintFrame(TestEnv& env)
{
    env.Render()->Clear(ui::UiColor(0xFFFFFFFF));
    env.Edit().Paint(env.Render(), env.m_rcControl);
}

/** 统计画面中的非白像素数量（用于确认确实绘制出了文字） */
int32_t CountInkPixels(TestEnv& env)
{
    std::vector<uint32_t> pixels((size_t)kWidth * kHeight, 0);
    if (!env.Render()->ReadPixels(env.m_rcControl, pixels.data(), (uint32_t)(pixels.size() * sizeof(uint32_t)))) {
        return -1;
    }
    int32_t nInk = 0;
    for (const uint32_t color : pixels) {
        if ((color & 0x00FFFFFFu) != 0x00FFFFFFu) {
            ++nInk;
        }
    }
    return nInk;
}

/** 读取整幅画面的像素 */
bool ReadFrame(ui::IRender* pRender, const ui::UiRect& rc, std::vector<uint32_t>& pixels);

/** 逐像素比较两幅画面，返回不同的像素数 */
int32_t DiffPixels(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b);

/** 获取光标处的字符位置（无选区时返回光标位置） */
int32_t GetCaretChar(TestRichEdit2& richEdit)
{
    int32_t nSelStart = -1;
    int32_t nSelEnd = -1;
    richEdit.GetSel(nSelStart, nSelEnd);
    CHECK_MSG(nSelStart == nSelEnd, "caret should have no selection");
    return nSelEnd;
}

/** 校验控件文本与期望一致 */
void CheckText(TestEnv& env, const DStringW& expected, const std::string& tag)
{
    const DStringW actual = env.Edit().GetTextW();
    Check(actual == expected, [&]() {
        return tag + ": expected [" + ToReadable(expected) + "], actual [" + ToReadable(actual) + "]";
    });
}

/** 校验 PosFromChar / CharFromPos 往返一致（鼠标定位与方向键的基础数据）
 *
 * 说明：换行符（\r、\n）、最后一个字符以及文本末尾位置，PosFromChar 返回的是
 * 行尾/文本尾的边界坐标，CharFromPos 对边界点按"所在行的字符区间"归位，
 * 两者的边界语义不对等（属于框架既有行为），因此这些位置不做严格往返校验。
 */
void CheckPosRoundTrip(TestEnv& env, const std::string& tag)
{
    const DStringW text = env.Edit().GetTextW();
    const int32_t nLength = (int32_t)text.size();
    int32_t nMismatch = 0;
    std::string firstMismatch;
    for (int32_t nChar = 0; nChar < nLength; ++nChar) {
        if ((text[(size_t)nChar] == L'\n') || (text[(size_t)nChar] == L'\r')) {
            continue;
        }
        if (nChar >= (nLength - 1)) {
            //最后一个字符：边界语义不同，不校验
            continue;
        }
        const ui::UiPoint pt = env.Edit().PosFromChar(nChar);
        const int32_t nBack = env.Edit().CharFromPos(pt);
        if (nBack != nChar) {
            ++nMismatch;
            if (firstMismatch.empty()) {
                firstMismatch = "char=" + std::to_string(nChar) + " -> pt=(" +
                                std::to_string(pt.x) + "," + std::to_string(pt.y) + ") -> char=" +
                                std::to_string(nBack);
            }
        }
    }
    Check(nMismatch == 0, [&]() {
        return tag + ": PosFromChar/CharFromPos round trip, mismatch=" + std::to_string(nMismatch) +
               " first: " + firstMismatch;
    });
}

/** 校验字符位置单调递增（同一行内 x 递增，跨行时 y 递增） */
void CheckCaretMonotonic(TestEnv& env, const std::string& tag)
{
    const DStringW text = env.Edit().GetTextW();
    const int32_t nLength = (int32_t)text.size();
    int32_t nBad = 0;
    std::string detail;
    for (int32_t nChar = 0; (nChar + 1) <= nLength; ++nChar) {
        //字符 0 与字符 nLength 也要比较
        const ui::UiPoint pt0 = env.Edit().PosFromChar(nChar);
        const ui::UiPoint pt1 = env.Edit().PosFromChar(nChar + 1);
        const bool bOk = (pt1.y > pt0.y) || ((pt1.y == pt0.y) && (pt1.x >= pt0.x));
        if (!bOk) {
            ++nBad;
            if (detail.empty()) {
                detail = "char " + std::to_string(nChar) + " (" + std::to_string(pt0.x) + "," + std::to_string(pt0.y) +
                         ") -> (" + std::to_string(pt1.x) + "," + std::to_string(pt1.y) + ")";
            }
        }
    }
    Check(nBad == 0, [&]() { return tag + ": caret position not monotonic, bad=" + std::to_string(nBad) + " " + detail; });
}

//==== 测试用例 ====

/** 替换文本：SetText / AppendText 的内容、行数与光标定位 */
void TestReplaceText(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();

    const DStringW text1 = L"第一行 first\nsecond 第二行\nthird line 3";
    richEdit.SetText(text1);
    CheckText(env, text1, "SetText");
    CHECK_MSG(richEdit.GetLineCount() == 3, "SetText should produce 3 lines");
    CHECK_MSG(richEdit.GetTextLength() == (int32_t)text1.size(), "GetTextLength mismatch");
    CheckPosRoundTrip(env, "SetText");
    CheckCaretMonotonic(env, "SetText");

    //再次替换全文，内容与布局都要更新
    const DStringW text2 = L"替换后的文本 replaced text";
    richEdit.SetText(text2);
    CheckText(env, text2, "SetText(again)");
    CHECK_MSG(richEdit.GetLineCount() == 1, "SetText(again) should produce 1 line");
    CheckPosRoundTrip(env, "SetText(again)");

    //追加文本
    const int32_t nAppendPos = richEdit.AppendText(_T("\n追加的行"), false, false);
    CheckText(env, text2 + L"\n追加的行", "AppendText");
    CHECK_MSG(richEdit.GetLineCount() == 2, "AppendText should produce 2 lines");
    CHECK_MSG(nAppendPos == richEdit.GetTextLength(), "AppendText should return the new text length");

    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "text should be painted after SetText/AppendText");
}

/** 插入字符：InsertText 与键盘输入 */
void TestInsertChar(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("abc\ndef"));
    //在位置 1 插入两个字符
    const int32_t nPos = richEdit.InsertText(1, _T("XY"), false);
    CheckText(env, L"aXYbc\ndef", "InsertText");
    CHECK_MSG(nPos == 3, "InsertText should return the position after inserted text");
    CheckPosRoundTrip(env, "InsertText");

    //在开头插入
    richEdit.InsertText(0, _T("<"), false);
    CheckText(env, L"<aXYbc\ndef", "InsertText(at 0)");

    //键盘输入一个字符（At 4：'X' 之后）
    richEdit.SetSel(4, 4);
    SendChar(richEdit, L'中');
    CheckText(env, L"<aXY中bc\ndef", "OnChar");
    CHECK_MSG(GetCaretChar(richEdit) == 5, "caret should move after the inserted char");

    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "text should be painted after inserting chars");
    CheckPosRoundTrip(env, "insert char");
}

/** 插入行：回车键在光标处插入换行符（换行符为 Windows 风格的 "\r\n"） */
void TestInsertLine(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("hello world"));
    richEdit.SetSel(5, 5);
    SendKey(richEdit, ui::kVK_RETURN);
    CheckText(env, L"hello\r\n world", "Enter key");
    CHECK_MSG(richEdit.GetLineCount() == 2, "Enter should insert a new line");
    CHECK_MSG(GetCaretChar(richEdit) == 7, "caret should be at the beginning of the new line");

    //新行首再插入一空行
    SendKey(richEdit, ui::kVK_RETURN);
    CheckText(env, L"hello\r\n\r\n world", "Enter key(2)");
    CHECK_MSG(richEdit.GetLineCount() == 3, "Enter twice should produce 3 lines");

    CheckPosRoundTrip(env, "insert line");
    CheckCaretMonotonic(env, "insert line");
    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "text should be painted after inserting a line");
}

/** 删除字符：退格键与删除键 */
void TestDeleteChar(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("abcd"));
    richEdit.SetSel(2, 2);
    SendKey(richEdit, ui::kVK_BACK);
    CheckText(env, L"acd", "Backspace");
    CHECK_MSG(GetCaretChar(richEdit) == 1, "caret should move back after Backspace");

    SendKey(richEdit, ui::kVK_DELETE);
    CheckText(env, L"ad", "Delete");
    CHECK_MSG(GetCaretChar(richEdit) == 1, "caret should stay after Delete");

    //行首退格：删除换行符，与上一行合并（'\n' 行结尾）
    richEdit.SetText(_T("ab\ncd"));
    richEdit.SetSel(3, 3);
    SendKey(richEdit, ui::kVK_BACK);
    CheckText(env, L"abcd", "Backspace at line start (LF)");
    CHECK_MSG(richEdit.GetLineCount() == 1, "Backspace at line start should merge lines");

    //行首退格：删除换行符，与上一行合并（"\r\n" 行结尾）
    richEdit.SetText(_T("ab\r\ncd"));
    richEdit.SetSel(4, 4);
    SendKey(richEdit, ui::kVK_BACK);
    CheckText(env, L"abcd", "Backspace at line start (CRLF)");
    CHECK_MSG(richEdit.GetLineCount() == 1, "Backspace at line start(CRLF) should merge lines");

    //行尾删除：删除换行符，与下一行合并
    richEdit.SetText(_T("ab\ncd"));
    richEdit.SetSel(2, 2);
    SendKey(richEdit, ui::kVK_DELETE);
    CheckText(env, L"abcd", "Delete at line end");

    //删除全部字符
    richEdit.SetSel(0, 4);
    SendKey(richEdit, ui::kVK_DELETE);
    CheckText(env, L"", "Delete all");
    PaintFrame(env);

    CheckPosRoundTrip(env, "delete char");
}

/** 删除行：选中整行（含换行符）后删除 */
void TestDeleteLine(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("l1\nl2\nl3"));
    //删除中间一行："l2\n" 位于 [3,6)
    richEdit.SetSel(3, 6);
    SendKey(richEdit, ui::kVK_DELETE);
    CheckText(env, L"l1\nl3", "delete middle line");
    CHECK_MSG(richEdit.GetLineCount() == 2, "middle line should be removed");

    //删除第一行
    richEdit.SetSel(0, 3);
    SendKey(richEdit, ui::kVK_DELETE);
    CheckText(env, L"l3", "delete first line");
    CHECK_MSG(richEdit.GetLineCount() == 1, "first line should be removed");

    //删除最后一行（含前一个换行符）
    richEdit.SetText(_T("l1\nl2\nl3"));
    richEdit.SetSel(5, 8);
    SendKey(richEdit, ui::kVK_DELETE);
    CheckText(env, L"l1\nl2", "delete last line");
    CHECK_MSG(richEdit.GetLineCount() == 2, "last line should be removed");

    CheckPosRoundTrip(env, "delete line");
    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "text should be painted after deleting lines");
}

/** 替换文本行：选中一行文本后替换 */
void TestReplaceTextLine(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("line one\nline two\nline three"));
    //替换第二行文本（不含换行符）：[9,17)
    richEdit.SetSel(9, 17);
    const bool bRet = richEdit.ReplaceSel(_T("新的一行"), true);
    CHECK_MSG(bRet, "ReplaceSel should succeed");
    CheckText(env, L"line one\n新的一行\nline three", "replace line");
    CHECK_MSG(richEdit.GetLineCount() == 3, "line count should not change");
    CHECK_MSG(GetCaretChar(richEdit) == 13, "caret should be after the replaced text");

    CheckPosRoundTrip(env, "replace line");
    CheckCaretMonotonic(env, "replace line");

    //替换整行（含换行符）为两行内容
    richEdit.SetText(_T("a\nb\nc"));
    richEdit.SetSel(2, 4);
    richEdit.ReplaceSel(_T("B1\nB2"), true);
    CheckText(env, L"a\nB1\nB2c", "replace line with two lines");
    CHECK_MSG(richEdit.GetLineCount() == 3, "line count should increase by one");

    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "text should be painted after replacing a line");
}

/** 方向键：上/下/左/右移动光标 */
void TestArrowKeys(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();

    //构造会自动换行的多行文本，确保上下方向键跨的是逻辑行
    DStringW longLine;
    for (int32_t i = 0; i < 200; ++i) {
        longLine += L"ab";
    }
    const DStringW text = longLine + L"\n" + longLine;
    richEdit.SetText(text);

    //左/右：每次移动一个字符
    richEdit.SetSel(10, 10);
    SendKey(richEdit, ui::kVK_RIGHT);
    CHECK_MSG(GetCaretChar(richEdit) == 11, "RIGHT should move caret +1");
    SendKey(richEdit, ui::kVK_LEFT);
    CHECK_MSG(GetCaretChar(richEdit) == 10, "LEFT should move caret -1");

    //左边界：位置 0 再按左键不动
    richEdit.SetSel(0, 0);
    SendKey(richEdit, ui::kVK_LEFT);
    CHECK_MSG(GetCaretChar(richEdit) == 0, "LEFT at position 0 should stay");

    //右边界：末尾再按右键不动
    const int32_t nTextLen = (int32_t)text.size();
    richEdit.SetSel(nTextLen, nTextLen);
    SendKey(richEdit, ui::kVK_RIGHT);
    CHECK_MSG(GetCaretChar(richEdit) == nTextLen, "RIGHT at end should stay");

    //上/下：光标移动到相邻逻辑行中 x 坐标最接近的位置
    for (const int32_t nStart : { 0, 20, 200, 400 }) {
        richEdit.SetSel(nStart, nStart);
        const ui::UiPoint ptStart = richEdit.PosFromChar(nStart);

        SendKey(richEdit, ui::kVK_DOWN);
        const int32_t nDown = GetCaretChar(richEdit);
        const ui::UiPoint ptDown = richEdit.PosFromChar(nDown);
        CHECK_MSG(ptDown.y > ptStart.y, "DOWN should move caret to a lower row");
        CHECK_MSG(ptDown.x >= ptStart.x - 24 && ptDown.x <= ptStart.x + 24,
                  "DOWN should keep caret x position");

        //再按向上键，回到原来的行（同一 x 位置）
        SendKey(richEdit, ui::kVK_UP);
        const int32_t nUp = GetCaretChar(richEdit);
        const ui::UiPoint ptUp = richEdit.PosFromChar(nUp);
        CHECK_MSG(ptUp.y == ptStart.y, "UP after DOWN should return to the original row");
        CHECK_MSG(ptUp.x == ptStart.x, "UP after DOWN should return to the original x position");
        CHECK_MSG(nUp == nStart, "UP after DOWN should return to the original char");

        //重新定位，验证向上移动
        richEdit.SetSel(nStart, nStart);
        SendKey(richEdit, ui::kVK_UP);
        const int32_t nUp2 = GetCaretChar(richEdit);
        const ui::UiPoint ptUp2 = richEdit.PosFromChar(nUp2);
        if (ptStart.y > richEdit.PosFromChar(0).y) {
            CHECK_MSG(ptUp2.y < ptStart.y, "UP should move caret to an upper row");
            CHECK_MSG(ptUp2.x >= ptStart.x - 24 && ptUp2.x <= ptStart.x + 24,
                      "UP should keep caret x position");
        }
        else {
            CHECK_MSG(nUp2 == nStart, "UP at the first row should stay");
        }

        //重绘一帧，确认方向键操作后绘制正常
        PaintFrame(env);
        CHECK_MSG(CountInkPixels(env) > 0, "text should be painted after arrow keys");
    }

    //编辑后方向键仍然正确（插入字符后向右移动）
    richEdit.SetSel(0, 0);
    SendChar(richEdit, L'X');
    SendKey(richEdit, ui::kVK_RIGHT);
    CHECK_MSG(GetCaretChar(richEdit) == 2, "RIGHT after inserting a char should move +1");
    CheckPosRoundTrip(env, "arrow keys after edit");
}

//==== 滚动切换视图 ====

/** 构造 nLines 行的文本（每行以换行符结尾） */
DStringW MakeTextLines(int32_t nLines)
{
    DStringW text;
    for (int32_t i = 0; i < nLines; ++i) {
        text += L"行 Line " + std::to_wstring(i) + L" 中文文本 test";
        if ((i + 1) < nLines) {
            text += L'\n';
        }
    }
    return text;
}

/** 滚动条滚动：滚动 N 像素后，画面应等于滚动前的画面上移 N 像素 */
void TestScrollByScrollBar(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();
    ui::IRender* pRender = env.Render();

    const int32_t nLines = 300;
    richEdit.SetText(MakeTextLines(nLines));
    richEdit.SetSel(0, 0);

    //计算单行高度（第 1 行与第 2 行首字符的 y 坐标差）
    const DStringW line0 = L"行 Line 0 中文文本 test";
    const ui::UiPoint pt0 = richEdit.PosFromChar(0);
    const ui::UiPoint pt1 = richEdit.PosFromChar((int32_t)line0.size() + 1);
    const int32_t nRowHeight = pt1.y - pt0.y;
    CHECK_MSG(nRowHeight > 0, "row height should be positive");

    //离屏环境没有布局引擎，为滚动条手动设置滚动范围（正常运行时由布局计算）
    richEdit.EnableScrollBar(true, false);
    ui::ScrollBar* pVScrollBar = richEdit.GetVScrollBar();
    CHECK_MSG(pVScrollBar != nullptr, "vertical scroll bar should be created");
    if (pVScrollBar == nullptr) {
        return;
    }
    const int64_t nContentHeight = (int64_t)nRowHeight * nLines;
    pVScrollBar->SetScrollRange(nContentHeight - kHeight);
    richEdit.SetScrollPosY(0);
    CHECK_MSG(richEdit.GetScrollOffset().cy == 0, "scroll offset should be 0 initially");

    PaintFrame(env);
    std::vector<uint32_t> pixelsBefore;
    CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixelsBefore), "ReadPixels(scroll=0) should succeed");
    CHECK_MSG(CountInkPixels(env) > 0, "text should be painted at scroll=0");

    //滚动 5 行
    const int32_t nScrollY = nRowHeight * 5;
    richEdit.SetScrollPosY(nScrollY);
    CHECK_MSG(richEdit.GetScrollOffset().cy == nScrollY, "scroll offset should be updated");

    PaintFrame(env);
    std::vector<uint32_t> pixelsAfter;
    CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixelsAfter), "ReadPixels(scrolled) should succeed");
    CHECK_MSG(CountInkPixels(env) > 0, "text should still be painted after scrolling");

    //比较中间的像素：两端是文本区域的裁剪边界（滚动后内容会移入内边距区域而不绘制）
    const int32_t nMargin = 8;
    int32_t nShiftMismatch = 0;
    std::string mismatchDetail;
    for (int32_t y = nMargin; (y + nScrollY + nMargin) < kHeight; ++y) {
        for (int32_t x = 0; x < kWidth; ++x) {
            const size_t nBefore = (size_t)(y + nScrollY) * kWidth + x;
            const size_t nAfter = (size_t)y * kWidth + x;
            if (pixelsBefore[nBefore] != pixelsAfter[nAfter]) {
                ++nShiftMismatch;
                if (mismatchDetail.size() < 200) {
                    char buf[64] = { 0 };
                    ::snprintf(buf, sizeof(buf), "(%d,%d) ", x, y);
                    mismatchDetail += buf;
                }
            }
        }
    }
    Check(nShiftMismatch == 0, [&]() {
        return "scrolling by " + std::to_string(nScrollY) + " pixels should shift the view, mismatch=" +
               std::to_string(nShiftMismatch) + " " + mismatchDetail;
    });

    //滚动回顶部，画面应恢复
    richEdit.SetScrollPosY(0);
    PaintFrame(env);
    std::vector<uint32_t> pixelsReset;
    CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixelsReset), "ReadPixels(reset) should succeed");
    CHECK_MSG(DiffPixels(pixelsBefore, pixelsReset) == 0, "scrolling back to top should restore the view");
}

/** 横向滚动：滚动后新出现的区域必须绘制文字（回归：缓存按可视宽度裁剪导致的空白） */
void TestHScrollPaint(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();
    ui::IRender* pRender = env.Render();

    const ui::UiRect rcOldControl = env.m_rcControl;
    env.m_rcControl = ui::UiRect(0, 0, 200, 60);
    richEdit.EnableScrollBar(false, true);
    richEdit.SetPos(env.m_rcControl);
    richEdit.SetWordWrap(false);
    richEdit.SetText(_T("Horizontal scroll paint regression: 0123456789 ")
                     _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz"));
    richEdit.SetSel(0, 0);

    ui::ScrollBar* pHScrollBar = richEdit.GetHScrollBar();
    CHECK_MSG(pHScrollBar != nullptr, "horizontal scroll bar should be created");
    if (pHScrollBar == nullptr) {
        richEdit.EnableScrollBar(false, false);
        env.m_rcControl = rcOldControl;
        richEdit.SetPos(rcOldControl);
        return;
    }

    //离屏环境没有布局引擎，手动设置横向滚动范围（正常运行时由布局计算）
    const int32_t nViewWidth = (int32_t)env.m_rcControl.Width();
    const int32_t nScrollX = 40;
    const int32_t nMargin = 8;
    pHScrollBar->SetScrollRange(400);
    richEdit.SetScrollPosX(0);
    CHECK_MSG(richEdit.GetScrollOffset().cx == 0, "horizontal scroll offset should be 0 initially");

    PaintFrame(env);
    std::vector<uint32_t> pixelsBefore;
    CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixelsBefore), "ReadPixels(hscroll=0) should succeed");

    //滚动前：右侧区域应有文字
    int32_t nRightInkBefore = 0;
    for (int32_t y = nMargin; y < 28; ++y) {
        for (int32_t x = nViewWidth - nScrollX; x < nViewWidth; ++x) {
            if ((pixelsBefore[(size_t)y * nViewWidth + x] & 0x00FFFFFFu) != 0x00FFFFFFu) {
                ++nRightInkBefore;
            }
        }
    }
    CHECK_MSG(nRightInkBefore > 0, "text should be painted at the right side when hscroll=0");

    //横向滚动 40 像素：复用同一份绘制缓存
    richEdit.SetScrollPosX(nScrollX);
    CHECK_MSG(richEdit.GetScrollOffset().cx == nScrollX, "horizontal scroll offset should be updated");
    PaintFrame(env);
    std::vector<uint32_t> pixelsAfter;
    CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixelsAfter), "ReadPixels(hscrolled) should succeed");

    //回归点：滚动后，右侧新出现的区域必须有文字（修复前该区域为空白）
    int32_t nRightInkAfter = 0;
    for (int32_t y = nMargin; y < 28; ++y) {
        for (int32_t x = nViewWidth - nScrollX; x < nViewWidth; ++x) {
            if ((pixelsAfter[(size_t)y * nViewWidth + x] & 0x00FFFFFFu) != 0x00FFFFFFu) {
                ++nRightInkAfter;
            }
        }
    }
    CHECK_MSG(nRightInkAfter > 0, "text should be painted in the newly exposed area after horizontal scrolling");

    //滚动后的内容应与滚动前的内容整体左移一致
    int32_t nShiftMismatch = 0;
    for (int32_t y = nMargin; y < 28; ++y) {
        for (int32_t x = nMargin; (x + nScrollX + nMargin) < nViewWidth; ++x) {
            const size_t nBefore = (size_t)y * nViewWidth + (x + nScrollX);
            const size_t nAfter = (size_t)y * nViewWidth + x;
            if (pixelsBefore[nBefore] != pixelsAfter[nAfter]) {
                ++nShiftMismatch;
            }
        }
    }
    CHECK_MSG(nShiftMismatch == 0, "horizontal scrolling should shift the view, mismatch=" +
              std::to_string(nShiftMismatch));

    //滚动回左侧，画面应恢复
    richEdit.SetScrollPosX(0);
    PaintFrame(env);
    std::vector<uint32_t> pixelsReset;
    CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixelsReset), "ReadPixels(reset) should succeed");
    CHECK_MSG(DiffPixels(pixelsBefore, pixelsReset) == 0, "scrolling back to left should restore the view");

    //恢复测试环境
    richEdit.EnableScrollBar(false, false);
    env.m_rcControl = rcOldControl;
    richEdit.SetPos(rcOldControl);
}

/** 构造类似源码文件的文本：长短行混合、含空行，用于验证粘贴大文本后的滚动绘制 */
DStringW MakeSourceLikeText(int32_t nLines, std::vector<DStringW>& lineTextList)
{
    lineTextList.clear();
    lineTextList.reserve((size_t)nLines);
    for (int32_t i = 0; i < nLines; ++i) {
        DStringW line;
        switch (i % 6) {
        case 0:
            line = L"namespace ui {";
            break;
        case 1:
            line = L"    LabelImpl::LabelImpl(Control* pOwner) : m_pOwner(pOwner), m_pLabelOwner(dynamic_cast<LabelOwner*>(pOwner)) {";
            break;
        case 2:
            line = L"        ASSERT(pOwner != nullptr);";
            break;
        case 3:
            line = L"";
            break;
        case 4:
            line = L"}  // 第 " + std::to_wstring(i) + L" 行：中文注释 mixed English comment 0123456789";
            break;
        default:
            line = L"    return m_pLabelOwner->GetTextDrawRect(GetRect());";
            break;
        }
        lineTextList.push_back(line);
    }
    DStringW text;
    for (size_t i = 0; i < lineTextList.size(); ++i) {
        text += lineTextList[i];
        if ((i + 1) < lineTextList.size()) {
            text += L"\r\n"; //与剪贴板中的源码文本一致
        }
    }
    return text;
}

/** 统计画面中指定纵向范围内的非白像素数量 */
int32_t CountInkInBand(const std::vector<uint32_t>& pixels, int32_t width, int32_t height,
                       int32_t y0, int32_t y1)
{
    if (y0 < 0) {
        y0 = 0;
    }
    if (y1 > height) {
        y1 = height;
    }
    int32_t nInk = 0;
    for (int32_t y = y0; y < y1; ++y) {
        for (int32_t x = 0; x < width; ++x) {
            if ((pixels[(size_t)y * (size_t)width + (size_t)x] & 0x00FFFFFFu) != 0x00FFFFFFu) {
                ++nInk;
            }
        }
    }
    return nInk;
}

/** 粘贴大文本后滚动视图：每个可视的文本行都必须绘制出来（不能出现空白带） */
void TestPasteThenScroll(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();
    ui::IRender* pRender = env.Render();

    //模拟演示程序中的小尺寸编辑框（长行会被自动换行，每行可能占用多个物理行）
    const ui::UiRect rcOldControl = env.m_rcControl;
    env.m_rcControl = ui::UiRect(0, 0, 360, 180);
    richEdit.EnableScrollBar(true, false);
    richEdit.SetPos(env.m_rcControl);
    const int32_t nViewHeight = (int32_t)env.m_rcControl.Height();
    const int32_t nViewWidth = (int32_t)env.m_rcControl.Width();

    richEdit.SetText(_T(""));
    richEdit.SetWordWrap(false); //与演示程序一致：多行、不自动换行（超出宽度时裁剪）

    std::vector<DStringW> lineTextList;
    const DStringW text = MakeSourceLikeText(160, lineTextList);
    richEdit.SetSel(0, 0);
    richEdit.ReplaceSel(text, false); //与 Paste() 相同的文本替换路径
    CHECK_MSG(richEdit.GetTextW() == text, "pasted text should equal the source text");

    const int32_t nLineCount = richEdit.GetLineCount();
    CHECK_MSG(nLineCount >= (int32_t)lineTextList.size(), "line count should cover the pasted text");
    if (nLineCount < 2) {
        richEdit.SetPos(rcOldControl);
        env.m_rcControl = rcOldControl;
        return;
    }

    //非空行的行顶部（内容坐标，不含内边距）：用行内第一个真实字符的位置计算，
    //空行不参与校验（其位置由上下文决定，不利于逐行比对）
    const int32_t nTextTop = 4;
    std::vector<int32_t> checkLineIndex;
    std::vector<int32_t> checkLineTop;
    for (int32_t i = 0; i < nLineCount; ++i) {
        if (richEdit.GetLine(i, 0x10000).empty()) {
            continue;
        }
        checkLineIndex.push_back(i);
        checkLineTop.push_back(richEdit.PosFromChar(richEdit.LineIndex(i)).y - nTextTop);
    }
    CHECK_MSG(checkLineIndex.size() > 100, "most source lines should be non-empty");
    if (checkLineIndex.size() < 2) {
        richEdit.SetPos(rcOldControl);
        env.m_rcControl = rcOldControl;
        return;
    }
    const int32_t nRowHeight = checkLineTop[1] - checkLineTop[0];
    CHECK_MSG(nRowHeight > 0, "row height should be positive");
    if (nRowHeight <= 0) {
        richEdit.SetPos(rcOldControl);
        env.m_rcControl = rcOldControl;
        return;
    }

    //离屏环境没有布局引擎，为滚动条手动设置滚动范围（正常运行时由布局计算）
    ui::ScrollBar* pVScrollBar = richEdit.GetVScrollBar();
    CHECK_MSG(pVScrollBar != nullptr, "vertical scroll bar should be created");
    if (pVScrollBar == nullptr) {
        richEdit.SetPos(rcOldControl);
        env.m_rcControl = rcOldControl;
        return;
    }
    const int32_t nContentHeight = checkLineTop.back() + nRowHeight * 2;
    pVScrollBar->SetScrollRange(nContentHeight - nViewHeight);
    richEdit.SetScrollPosY(0);

    //在多个滚动位置检查：每个可视的非空行都必须有文字像素
    std::vector<int32_t> scrollPositions = { 0, 100, 300, 1200, 3000, nContentHeight - nViewHeight };
    std::vector<std::pair<int32_t, std::vector<uint32_t>>> pasteFrames;
    std::vector<uint32_t> pixels;
    for (const int32_t nScrollY : scrollPositions) {
        richEdit.SetScrollPosY(nScrollY);
        PaintFrame(env);
        CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixels), "ReadPixels should succeed");
        pasteFrames.emplace_back(nScrollY, pixels);

        int32_t nBlankLines = 0;
        std::string blankDetail;
        for (size_t i = 0; i < checkLineIndex.size(); ++i) {
            const int32_t nTop = checkLineTop[i] - nScrollY;
            const int32_t nBottom = ((i + 1) < checkLineTop.size()) ? (checkLineTop[i + 1] - nScrollY)
                                                                   : (nTop + nRowHeight);
            if ((nTop < 8) || (nBottom > (nViewHeight - 8))) {
                continue; //只检查完整可见的行
            }
            const int32_t nInk = CountInkInBand(pixels, nViewWidth, nViewHeight, nTop, nBottom);
            if (nInk <= 0) {
                ++nBlankLines;
                if (blankDetail.size() < 300) {
                    blankDetail += "line " + std::to_string(checkLineIndex[i]) + " y=" + std::to_string(nTop) + "; ";
                }
            }
        }
        Check(nBlankLines == 0, [&]() {
            return "scroll=" + std::to_string(nScrollY) + ": " + std::to_string(nBlankLines) +
                   " visible line(s) not painted: " + blankDetail;
        });
    }

    //全量排版（SetText）后，同样滚动位置的画面必须与粘贴路径一致
    richEdit.SetText(text);
    richEdit.SetScrollPosY(0);
    for (const auto& frame : pasteFrames) {
        richEdit.SetScrollPosY(frame.first);
        PaintFrame(env);
        CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixels), "ReadPixels(reference) should succeed");
        const int32_t nDiff = DiffPixels(frame.second, pixels);
        Check(nDiff == 0, [&]() {
            return "scroll=" + std::to_string(frame.first) + ": paint result after paste differs from SetText, diff=" +
                   std::to_string(nDiff);
        });
    }

    //恢复公共测试环境
    richEdit.EnableScrollBar(false, false);
    richEdit.SetPos(rcOldControl);
    env.m_rcControl = rcOldControl;
}

/** 方向键切换视图：光标移出可视区域时，视图应通过滚动跟随光标 */
void TestArrowKeysScrollView(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();
    const int32_t nLines = 300;
    richEdit.SetText(MakeTextLines(nLines));
    richEdit.SetSel(0, 0);

    const DStringW line0 = L"行 Line 0 中文文本 test";
    const int32_t nRowHeight = richEdit.PosFromChar((int32_t)line0.size() + 1).y - richEdit.PosFromChar(0).y;
    CHECK_MSG(nRowHeight > 0, "row height should be positive");

    richEdit.EnableScrollBar(true, false);
    ui::ScrollBar* pVScrollBar = richEdit.GetVScrollBar();
    CHECK_MSG(pVScrollBar != nullptr, "vertical scroll bar should be created");
    if ((pVScrollBar == nullptr) || (nRowHeight <= 0)) {
        return;
    }
    pVScrollBar->SetScrollRange((int64_t)nRowHeight * nLines - kHeight);
    richEdit.SetScrollPosY(0);
    PaintFrame(env);

    //向下移动 40 行：光标超出可视区域后，视图应向下滚动，且光标仍在可视区域内
    const int32_t nMoveCount = 40;
    for (int32_t i = 0; i < nMoveCount; ++i) {
        SendKey(richEdit, ui::kVK_DOWN);
        PaintFrame(env);
    }
    const int32_t nScrollY = richEdit.GetScrollOffset().cy;
    CHECK_MSG(nScrollY > 0, "view should scroll down when the caret moves below the visible area");
    const int32_t nCaretChar = GetCaretChar(richEdit);
    const int32_t nCaretY = richEdit.PosFromChar(nCaretChar).y - nScrollY;
    CHECK_MSG((nCaretY >= 0) && (nCaretY < kHeight), "caret should stay inside the visible area after scrolling");
    //40 次向下移动后，光标应位于第 41 行的行首
    int32_t nExpectedChar = 0;
    for (int32_t i = 0; i < nMoveCount; ++i) {
        nExpectedChar += (int32_t)(L"行 Line " + std::to_wstring(i) + L" 中文文本 test").size() + 1;
    }
    CHECK_MSG(nCaretChar == nExpectedChar,
              "caret should be at the beginning of the 41st line after moving down 40 times");

    //向上移动回第一行：视图应滚动回顶部
    for (int32_t i = 0; i < nMoveCount; ++i) {
        SendKey(richEdit, ui::kVK_UP);
        PaintFrame(env);
    }
    CHECK_MSG(GetCaretChar(richEdit) == 0, "caret should return to the first line");
    CHECK_MSG(richEdit.GetScrollOffset().cy == 0, "view should scroll back to the top");
}

//==== 滚动切换视图（绘制缓存层）====
/** 构造 300 行、每行一个逻辑行的富文本 */
void BuildLineList(std::vector<DStringW>& lineTextList, std::vector<ui::RichTextData>& richTextDataList)
{
    const int32_t nTestLines = 300;
    lineTextList.clear();
    lineTextList.reserve(nTestLines);
    for (int32_t i = 0; i < nTestLines; ++i) {
        DStringW lineText = L"行 Line " + std::to_wstring(i) + L" 中文文本 test";
        if ((i + 1) < nTestLines) {
            lineText += L'\n';
        }
        lineTextList.push_back(lineText);
    }

    ui::SharePtr<ui::UiFontEx> pFontEx(new ui::UiFontEx);
    pFontEx->m_fontName = _T("Microsoft YaHei");
    pFontEx->m_fontSize = 16;

    richTextDataList.clear();
    richTextDataList.reserve(nTestLines);
    for (int32_t i = 0; i < nTestLines; ++i) {
        ui::RichTextData richTextData;
        richTextData.m_textView = std::wstring_view(lineTextList[(size_t)i]);
        richTextData.m_textColor = ui::UiColor(0xFF202020u);
        richTextData.m_pFontInfo = pFontEx;
        richTextData.m_textStyle = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_TOP |
                                   ui::DrawStringFormat::TEXT_WORD_WRAP;
        richTextDataList.push_back(richTextData);
    }
}

/** 计算每个 RichTextData 的行顶/行底（绝对坐标） */
void CalcLineTops(const std::vector<std::vector<ui::UiRect>>& lineRects,
                  std::vector<int32_t>& lineTop,
                  std::vector<int32_t>& lineBottom)
{
    const int32_t nLineCount = (int32_t)lineRects.size();
    lineTop.assign((size_t)nLineCount, 0);
    lineBottom.assign((size_t)nLineCount, 0);
    for (int32_t i = 0; i < nLineCount; ++i) {
        if (lineRects[(size_t)i].empty()) {
            lineTop[(size_t)i] = (i > 0) ? lineBottom[(size_t)i - 1] : 0;
            lineBottom[(size_t)i] = lineTop[(size_t)i];
            continue;
        }
        int32_t nTop = INT32_MAX;
        int32_t nBottom = INT32_MIN;
        for (const ui::UiRect& rowRect : lineRects[(size_t)i]) {
            nTop = (std::min)(nTop, (int32_t)rowRect.top);
            nBottom = (std::max)(nBottom, (int32_t)ui::CEILF(rowRect.bottom));
        }
        lineTop[(size_t)i] = nTop;
        lineBottom[(size_t)i] = nBottom;
    }
}

/** 读取整幅画面的像素 */
bool ReadFrame(ui::IRender* pRender, const ui::UiRect& rc, std::vector<uint32_t>& pixels)
{
    pixels.assign((size_t)rc.Width() * rc.Height(), 0);
    return pRender->ReadPixels(rc, pixels.data(), (uint32_t)(pixels.size() * sizeof(uint32_t)));
}

/** 逐像素比较两幅画面，返回不同的像素数 */
int32_t DiffPixels(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b)
{
    if (a.size() != b.size()) {
        return (int32_t)(std::max)(a.size(), b.size());
    }
    int32_t nDiff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            ++nDiff;
        }
    }
    return nDiff;
}

/** 滚动切换视图：按可视区域构建的绘制缓存，必须与全量缓存的绘制结果一致 */
void TestScrollViewSwitch(TestEnv& env)
{
    ui::IRender* pRender = env.Render();
    const ui::UiRect rcControl = env.m_rcControl;

    std::vector<DStringW> lineTextList;
    std::vector<ui::RichTextData> richTextDataList;
    BuildLineList(lineTextList, richTextDataList);

    std::vector<std::vector<ui::UiRect>> lineRects;
    pRender->MeasureRichText(rcControl, ui::UiSize(), richTextDataList, nullptr, nullptr, &lineRects);
    if ((lineRects.size() != richTextDataList.size()) || lineRects.empty()) {
        CHECK_MSG(false, "MeasureRichText should return one rect list per RichTextData");
        return;
    }
    std::vector<int32_t> lineTop;
    std::vector<int32_t> lineBottom;
    CalcLineTops(lineRects, lineTop, lineBottom);
    const int32_t nLineCount = (int32_t)lineTop.size();
    CHECK_MSG(lineTop[0] == 0, "first line top should be 0");

    //全量缓存（作为参考）
    std::shared_ptr<ui::DrawRichTextCache> spFullCache;
    pRender->CreateDrawRichTextCache(rcControl, ui::UiSize(), richTextDataList, spFullCache);
    if (spFullCache == nullptr) {
        CHECK_MSG(false, "CreateDrawRichTextCache failed");
        return;
    }

    const int32_t nViewHeight = (int32_t)rcControl.Height();
    std::vector<uint32_t> fullPixels;
    std::vector<uint32_t> windowPixels;
    for (const int32_t nScrollY : { 0, 22, 150, 1000, 3000 }) {
        //与 RichEditData::GetVisibleLineRange 相同的窗口选择规则：上下各预取一屏
        const int32_t nWindowTop = nScrollY - nViewHeight;
        const int32_t nWindowBottom = nScrollY + 2 * nViewHeight;
        size_t nFirstLine = (size_t)(std::lower_bound(lineTop.begin(), lineTop.end(), nWindowTop) - lineTop.begin());
        if (nFirstLine > 0) {
            --nFirstLine;
        }
        size_t nLastLine = (size_t)(std::upper_bound(lineTop.begin(), lineTop.end(), nWindowBottom) - lineTop.begin());
        if (nLastLine > 0) {
            --nLastLine;
        }
        if (nLastLine < nFirstLine) {
            nLastLine = nFirstLine;
        }
        if (nLastLine >= (size_t)nLineCount) {
            nLastLine = (size_t)nLineCount - 1;
        }

        std::vector<ui::RichTextData> windowList(richTextDataList.begin() + (std::ptrdiff_t)nFirstLine,
                                                 richTextDataList.begin() + (std::ptrdiff_t)nLastLine + 1);
        ui::RichTextDrawWindow drawWindow;
        drawWindow.m_nFirstLineNumber = (uint32_t)nFirstLine;
        drawWindow.m_nFirstRowIndex = (uint32_t)nFirstLine; //每行一个逻辑行
        drawWindow.m_nFirstRowTop = lineTop[nFirstLine];
        drawWindow.m_nBottom = ((nLastLine + 1) < (size_t)nLineCount) ? lineTop[nLastLine + 1] : lineBottom[nLastLine];
        drawWindow.m_nTotalRowCount = (uint32_t)nLineCount;

        std::shared_ptr<ui::DrawRichTextCache> spWindowCache;
        pRender->CreateDrawRichTextCacheInRange(rcControl, ui::UiSize(), windowList, drawWindow, spWindowCache);
        CHECK_MSG(spWindowCache != nullptr, "CreateDrawRichTextCacheInRange should build a cache");

        //全量缓存绘制
        pRender->Clear(ui::UiColor(0xFFFFFFFF));
        pRender->DrawRichTextCacheData(spFullCache, rcControl, ui::UiSize(0, nScrollY), std::vector<int32_t>(), 255);
        CHECK_MSG(ReadFrame(pRender, rcControl, fullPixels), "ReadPixels(full) should succeed");

        //窗口缓存绘制
        pRender->Clear(ui::UiColor(0xFFFFFFFF));
        if (spWindowCache != nullptr) {
            pRender->DrawRichTextCacheData(spWindowCache, rcControl, ui::UiSize(0, nScrollY), std::vector<int32_t>(), 255);
        }
        CHECK_MSG(ReadFrame(pRender, rcControl, windowPixels), "ReadPixels(window) should succeed");

        const int32_t nDiff = DiffPixels(fullPixels, windowPixels);
        Check(nDiff == 0, [&]() {
            return "scroll=" + std::to_string(nScrollY) + " window=[" + std::to_string(nFirstLine) + "," +
                   std::to_string(nLastLine) + "]: window cache differs from full cache, diff pixels=" +
                   std::to_string(nDiff);
        });
    }

    //滚动偏移改变视图：滚动 11 像素后，画面应等于滚动前的画面上移 11 像素
    const int32_t nShift = 11;
    pRender->Clear(ui::UiColor(0xFFFFFFFF));
    pRender->DrawRichTextCacheData(spFullCache, rcControl, ui::UiSize(0, 0), std::vector<int32_t>(), 255);
    std::vector<uint32_t> beforePixels;
    CHECK_MSG(ReadFrame(pRender, rcControl, beforePixels), "ReadPixels(scroll=0) should succeed");

    pRender->Clear(ui::UiColor(0xFFFFFFFF));
    pRender->DrawRichTextCacheData(spFullCache, rcControl, ui::UiSize(0, nShift), std::vector<int32_t>(), 255);
    std::vector<uint32_t> afterPixels;
    CHECK_MSG(ReadFrame(pRender, rcControl, afterPixels), "ReadPixels(scroll=11) should succeed");

    int32_t nShiftMismatch = 0;
    for (int32_t y = 0; (y + nShift) < kHeight; ++y) {
        for (int32_t x = 0; x < kWidth; ++x) {
            const size_t nBefore = (size_t)(y + nShift) * kWidth + x;
            const size_t nAfter = (size_t)y * kWidth + x;
            if (beforePixels[nBefore] != afterPixels[nAfter]) {
                ++nShiftMismatch;
            }
        }
    }
    Check(nShiftMismatch == 0, [&]() {
        return "scroll should shift the view by " + std::to_string(nShift) +
               " pixels, mismatch=" + std::to_string(nShiftMismatch);
    });
    CHECK_MSG(DiffPixels(beforePixels, afterPixels) > 0, "scroll should change the rendered content");
}

/** 编辑后重绘：内容变化后，绘制结果必须随之变化（绘制缓存正确失效） */
void TestPaintAfterEdit(TestEnv& env)
{
    TestRichEdit2& richEdit = env.Edit();
    ui::IRender* pRender = env.Render();

    richEdit.SetText(_T("AAAA"));
    PaintFrame(env);
    std::vector<uint32_t> pixelsBefore;
    CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixelsBefore), "ReadPixels(before) should succeed");
    const int32_t nInkBefore = CountInkPixels(env);
    CHECK_MSG(nInkBefore > 0, "text should be painted");

    //在末尾追加内容：画面应发生变化
    richEdit.SetSel(4, 4);
    richEdit.ReplaceSel(_T("BBBBBBBB"), true);
    PaintFrame(env);
    std::vector<uint32_t> pixelsAfter;
    CHECK_MSG(ReadFrame(pRender, env.m_rcControl, pixelsAfter), "ReadPixels(after) should succeed");
    CHECK_MSG(DiffPixels(pixelsBefore, pixelsAfter) > 0, "paint result should change after editing");
    CHECK_MSG(CountInkPixels(env) > nInkBefore, "ink pixels should increase after appending text");

    //清空文本：不应崩溃，画面应为空
    richEdit.SetText(_T(""));
    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) == 0, "empty text should not paint anything");
}

/** 只读模式：所有编辑入口都不改变文本，SetText 仍可设置文本 */
void TestReadOnly(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("abc"));
    richEdit.SetReadOnly(true);
    CHECK_MSG(richEdit.IsReadOnly(), "SetReadOnly(true) should take effect");

    richEdit.SetSel(1, 1);
    SendChar(richEdit, L'x');
    CheckText(env, L"abc", "readonly: OnChar");

    richEdit.InsertText(1, _T("Y"), true);
    CheckText(env, L"abc", "readonly: InsertText");

    const bool bReplaced = richEdit.ReplaceSel(_T("Z"), true);
    CHECK_MSG(!bReplaced, "readonly: ReplaceSel should return false");
    CheckText(env, L"abc", "readonly: ReplaceSel");

    richEdit.SetSel(2, 2);
    SendKey(richEdit, ui::kVK_BACK);
    CheckText(env, L"abc", "readonly: Backspace");
    SendKey(richEdit, ui::kVK_DELETE);
    CheckText(env, L"abc", "readonly: Delete");

    richEdit.SetSelAll();
    richEdit.Clear();
    CheckText(env, L"abc", "readonly: Clear");

    richEdit.SetText(_T("readonly text"));
    CheckText(env, L"readonly text", "readonly: SetText should still work");
    CHECK_MSG(!richEdit.CanUndo(), "readonly: CanUndo should be false");

    richEdit.SetReadOnly(false);
    richEdit.SetSel(0, 0);
    SendChar(richEdit, L'X');
    CheckText(env, L"Xreadonly text", "readonly off: OnChar should work");
}

/** 文本长度限制：超限时整次修改被丢弃，并触发 kEventTextLimit */
void TestLimitText(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("ab"));
    richEdit.SetLimitText(5);
    CHECK_MSG(richEdit.GetLimitText() == 5, "GetLimitText should return 5");

    int32_t nLimitCount = 0;
    richEdit.AttachTextLimit([&nLimitCount](const ui::EventArgs&) { ++nLimitCount; return true; }, 1);

    richEdit.SetSel(2, 2);
    SendChar(richEdit, L'c');
    SendChar(richEdit, L'd');
    SendChar(richEdit, L'e');
    CheckText(env, L"abcde", "limit: input up to limit should succeed");
    CHECK_MSG(nLimitCount == 0, "limit: no text-limit event when not exceeded");

    //正好达到限制后再输入一个字符：本次修改被丢弃
    richEdit.SetSel(5, 5);
    SendChar(richEdit, L'f');
    CheckText(env, L"abcde", "limit: OnChar over limit should be dropped");
    CHECK_MSG(nLimitCount == 1, "limit: OnChar over limit should fire kEventTextLimit");

    //InsertText 跨越限制：整个插入被丢弃
    richEdit.InsertText(5, _T("XY"), true);
    CheckText(env, L"abcde", "limit: InsertText over limit should be dropped");
    CHECK_MSG(nLimitCount == 2, "limit: InsertText over limit should fire kEventTextLimit");

    //替换一段文本时，替换后跨过限制：整个替换被丢弃
    richEdit.SetSel(0, 5);
    richEdit.ReplaceSel(_T("123456"), true);
    CheckText(env, L"abcde", "limit: multi-char replace over limit should be dropped");
    CHECK_MSG(nLimitCount == 3, "limit: multi-char replace over limit should fire kEventTextLimit");

    //替换后长度不超过限制：成功，且不触发超限事件
    richEdit.SetSel(0, 5);
    const bool bRet = richEdit.ReplaceSel(_T("1234"), true);
    CHECK_MSG(bRet, "limit: replace within limit should succeed");
    CheckText(env, L"1234", "limit: replace within limit");
    CHECK_MSG(nLimitCount == 3, "limit: no event when replace within limit");

    //SetLimitText 缩小限制：立即截断已有文本（源码：RichEditData::SetText 中 TruncateLimitText）
    richEdit.SetLimitText(3);
    CheckText(env, L"123", "limit: SetLimitText should truncate existing text");
    CHECK_MSG(nLimitCount == 3, "limit: truncation by SetLimitText should not fire text-limit event");

    //SetText 超限：静默截断
    richEdit.SetText(_T("abcdefgh"));
    CheckText(env, L"abc", "limit: SetText should be truncated silently");
    CHECK_MSG(nLimitCount == 3, "limit: truncation by SetText should not fire text-limit event");

    //边界：正好达到限制
    richEdit.SetLimitText(5);
    richEdit.SetText(_T("12345"));
    SendChar(richEdit, L'6');
    CheckText(env, L"12345", "limit: exactly at limit should reject next char");
    CHECK_MSG(nLimitCount == 4, "limit: exactly at limit should fire text-limit event");

    //SetLimitText(0) 表示不限制（源码：仅当 limit > 0 时检查长度）
    richEdit.SetLimitText(0);
    richEdit.SetText(_T("123"));
    richEdit.SetSel(3, 3);
    SendChar(richEdit, L'4');
    SendChar(richEdit, L'5');
    SendChar(richEdit, L'6');
    CheckText(env, L"123456", "limit: SetLimitText(0) should remove the limit");
    CHECK_MSG(richEdit.GetTextLength() == 6, "limit: text length should be 6 when unlimited");
    CHECK_MSG(nLimitCount == 4, "limit: no event when limit is disabled");

    richEdit.DetachEvent(ui::kEventTextLimit);
}

/** 允许字符集：仅 OnChar 输入路径过滤，InsertText/SetText 不过滤 */
void TestLimitChars(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetLimitChars(_T("#0123456789ABCDEF"));
    CHECK_MSG(richEdit.GetLimitChars() == _T("#0123456789ABCDEF"), "GetLimitChars should return the configured chars");

    richEdit.SetText(_T(""));
    richEdit.SetSel(0, 0);

    SendChar(richEdit, L'G');
    CheckText(env, L"", "limit chars: invalid char should be rejected by OnChar");
    SendChar(richEdit, L'A');
    CheckText(env, L"A", "limit chars: valid char 'A'");
    SendChar(richEdit, L'1');
    CheckText(env, L"A1", "limit chars: valid char '1'");
    SendChar(richEdit, L' ');
    CheckText(env, L"A1", "limit chars: space should be rejected by OnChar");

    //源码：InsertText -> ReplaceSel -> RichEditData::ReplaceText，不检查 limit_chars
    richEdit.InsertText(2, _T("XYZ"), false);
    CheckText(env, L"A1XYZ", "limit chars: InsertText should bypass limit_chars (source semantics)");

    //源码：SetText 不检查 limit_chars
    richEdit.SetText(_T("g"));
    CheckText(env, L"g", "limit chars: SetText should bypass limit_chars (source semantics)");

    //清空限制后，任意字符都可以输入
    richEdit.SetLimitChars(DString());
    CHECK_MSG(richEdit.GetLimitChars().empty(), "GetLimitChars should be empty after clearing");
    richEdit.SetSel(1, 1);
    SendChar(richEdit, L'G');
    CheckText(env, L"gG", "limit chars: any char should be accepted after clearing limit");
}

/** 密码模式：真实文本可读，显示使用密码字符，可切换显示/隐藏与退出密码模式 */
void TestPasswordMode(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetPasswordMode(true);
    CHECK_MSG(richEdit.IsPasswordMode(), "SetPasswordMode(true) should take effect");

    //密码模式会删除换行符和TAB字符（源码：RemoveInvalidPasswordChar）
    richEdit.SetText(_T("a\nb\tc"));
    CheckText(env, L"abc", "password: SetText should remove newline/tab");

    richEdit.SetText(_T("secret123"));
    CheckText(env, L"secret123", "password: GetText should return real text");
    CHECK_MSG(richEdit.GetTextLength() == 9, "password: GetTextLength should return real length");
    CHECK_MSG(richEdit.GetLine(0, 100) == _T("secret123"), "password: GetLine should return real text");

    //PosFromChar / CharFromPos 在密码模式下仍可用
    const ui::UiPoint pt2 = richEdit.PosFromChar(2);
    CHECK_MSG(richEdit.CharFromPos(pt2) == 2, "password: PosFromChar/CharFromPos round trip should work");

    richEdit.SetPasswordChar(L'#');
    CHECK_MSG(richEdit.GetPasswordChar() == L'#', "password: SetPasswordChar/GetPasswordChar");

    //显示密码前后的绘制结果应不同，且都不崩溃、有像素
    richEdit.SetShowPassword(false);
    CHECK_MSG(!richEdit.IsShowPassword(), "password: SetShowPassword(false)");
    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "password: masked text should be painted");
    std::vector<uint32_t> maskedPixels;
    CHECK_MSG(ReadFrame(env.Render(), env.m_rcControl, maskedPixels), "password: ReadPixels(masked)");

    richEdit.SetShowPassword(true);
    CHECK_MSG(richEdit.IsShowPassword(), "password: SetShowPassword(true)");
    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "password: shown text should be painted");
    std::vector<uint32_t> shownPixels;
    CHECK_MSG(ReadFrame(env.Render(), env.m_rcControl, shownPixels), "password: ReadPixels(shown)");
    CHECK_MSG(DiffPixels(maskedPixels, shownPixels) > 0, "password: masked and shown text should differ");
    CheckText(env, L"secret123", "password: ShowPassword should not change text");

    richEdit.SetSel(1, 4);
    CHECK_MSG(richEdit.GetSelText() == _T("ecr"), "password: GetSelText should return real selected text");

    richEdit.SetPasswordMode(false);
    CHECK_MSG(!richEdit.IsPasswordMode(), "password: SetPasswordMode(false) should take effect");
    CheckText(env, L"secret123", "password: text should be kept after disabling password mode");
    richEdit.SetSel(0, 0);
    SendChar(richEdit, L'X');
    CheckText(env, L"Xsecret123", "password off: editing should work");
    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "password off: text should be painted");
}

/** 只允许数字：非法字符拒绝输入，超上下限被夹取，数字调整/循环 */
void TestNumberOnly(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetNumberOnly(true);
    CHECK_MSG(richEdit.IsNumberOnly(), "SetNumberOnly(true) should take effect");
    richEdit.SetMinNumber(0);
    richEdit.SetMaxNumber(100);

    richEdit.SetText(_T(""));
    richEdit.SetSel(0, 0);
    SendChar(richEdit, L'a');
    CheckText(env, L"", "number only: non-digit should be rejected");
    SendChar(richEdit, L'-');
    CheckText(env, L"", "number only: '-' should be rejected when min >= 0");
    SendChar(richEdit, L'5');
    CheckText(env, L"5", "number only: digit '5'");
    SendChar(richEdit, L'x');
    CheckText(env, L"5", "number only: non-digit after digit should be rejected");
    SendChar(richEdit, L'0');
    CheckText(env, L"50", "number only: digits should append");

    //超过最大值：OnTextChanged 中夹取到 max_number
    richEdit.SetMinNumber(0);
    richEdit.SetMaxNumber(10);
    richEdit.SetText(_T("1"));
    richEdit.SetSel(1, 1);
    SendChar(richEdit, L'5');
    CheckText(env, L"10", "number only: value above max should be clamped to max");
    CHECK_MSG(richEdit.GetTextNumber() == 10, "number only: GetTextNumber should be 10");

    //低于最小值：SetText 后夹取到 min_number
    richEdit.SetMinNumber(5);
    richEdit.SetMaxNumber(99);
    richEdit.SetText(_T("2"));
    CheckText(env, L"5", "number only: value below min should be clamped to min");
    CHECK_MSG(richEdit.GetTextNumber() == 5, "number only: GetTextNumber should be 5");

    //SetTextNumber / GetTextNumber
    richEdit.SetMinNumber(0);
    richEdit.SetMaxNumber(99);
    richEdit.SetTextNumber(-5);
    CheckText(env, L"0", "number only: SetTextNumber below min should be clamped");
    CHECK_MSG(richEdit.GetTextNumber() == 0, "number only: GetTextNumber after clamping");

    richEdit.SetTextNumber(50);
    CHECK_MSG(richEdit.GetTextNumber() == 50, "number only: SetTextNumber(50)");
    richEdit.AdjustTextNumber(1);
    CHECK_MSG(richEdit.GetTextNumber() == 51, "number only: AdjustTextNumber(+1)");

    //达到最大值后 +1：循环到最小值（源码：nDelta==1 && old==max 时取 min）
    richEdit.SetMinNumber(10);
    richEdit.SetMaxNumber(51);
    richEdit.SetTextNumber(51);
    richEdit.AdjustTextNumber(1);
    CHECK_MSG(richEdit.GetTextNumber() == 10, "number only: AdjustTextNumber(+1) at max should wrap to min");

    //达到最小值后 -1：循环到最大值
    richEdit.AdjustTextNumber(-1);
    CHECK_MSG(richEdit.GetTextNumber() == 51, "number only: AdjustTextNumber(-1) at min should wrap to max");

    richEdit.SetNumberOnly(false);
    CHECK_MSG(!richEdit.IsNumberOnly(), "SetNumberOnly(false) should take effect");
    richEdit.SetText(_T("12a"));
    CheckText(env, L"12a", "number only off: non-digit text should be allowed");
}

/** 对齐方式：水平/垂直对齐改变字符坐标，且 PosFromChar/CharFromPos 往返仍成立 */
void TestTextAlign(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("abcdef"));
    richEdit.SetSel(0, 0);
    const ui::UiPoint ptLeft = richEdit.PosFromChar(0);
    CHECK_MSG(richEdit.GetHAlignType() == ui::HorAlignType::kAlignLeft, "align: default should be left");

    richEdit.SetTextHAlignType(ui::HorAlignType::kAlignCenter);
    const ui::UiPoint ptCenter = richEdit.PosFromChar(0);
    CHECK_MSG(richEdit.GetHAlignType() == ui::HorAlignType::kAlignCenter, "align: h-align center");
    CHECK_MSG(ptCenter.x > ptLeft.x, "align: center x should be larger than left x");
    CHECK_MSG(richEdit.CharFromPos(richEdit.PosFromChar(1)) == 1, "align: round trip should hold when centered");

    richEdit.SetTextHAlignType(ui::HorAlignType::kAlignRight);
    const ui::UiPoint ptRight = richEdit.PosFromChar(0);
    CHECK_MSG(richEdit.GetHAlignType() == ui::HorAlignType::kAlignRight, "align: h-align right");
    CHECK_MSG(ptRight.x > ptCenter.x, "align: right x should be larger than center x");
    CHECK_MSG(richEdit.CharFromPos(richEdit.PosFromChar(1)) == 1, "align: round trip should hold when right aligned");

    //垂直对齐
    richEdit.SetTextHAlignType(ui::HorAlignType::kAlignLeft);
    const ui::UiPoint ptTop = richEdit.PosFromChar(0);
    richEdit.SetTextVAlignType(ui::VerAlignType::kAlignCenter);
    const ui::UiPoint ptVMiddle = richEdit.PosFromChar(0);
    CHECK_MSG(richEdit.GetVAlignType() == ui::VerAlignType::kAlignCenter, "align: v-align center");
    CHECK_MSG(ptVMiddle.y > ptTop.y, "align: middle y should be larger than top y");
    CHECK_MSG(richEdit.CharFromPos(richEdit.PosFromChar(1)) == 1, "align: round trip should hold when v-centered");

    richEdit.SetTextVAlignType(ui::VerAlignType::kAlignBottom);
    const ui::UiPoint ptBottom = richEdit.PosFromChar(0);
    CHECK_MSG(richEdit.GetVAlignType() == ui::VerAlignType::kAlignBottom, "align: v-align bottom");
    CHECK_MSG(ptBottom.y > ptVMiddle.y, "align: bottom y should be larger than middle y");
    CHECK_MSG(richEdit.CharFromPos(richEdit.PosFromChar(1)) == 1, "align: round trip should hold when bottom aligned");

    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "align: text should be painted");
}

/** 单行/多行与按键选项：回车、TAB、Ctrl+回车 */
void TestLineAndKeyOptions(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    int32_t nReturnCount = 0;
    int32_t nTabCount = 0;
    richEdit.AttachReturn([&nReturnCount](const ui::EventArgs&) { ++nReturnCount; return true; }, 1);
    richEdit.AttachTab([&nTabCount](const ui::EventArgs&) { ++nTabCount; return true; }, 1);

    //单行模式：want_return=true 时回车既不插入换行，也不触发 kEventReturn
    //（源码：OnInputChar 中仅在 !m_bWantReturn 时 SendEvent(kEventReturn)）
    richEdit.SetText(_T("abc"));
    richEdit.SetMultiLine(false);
    CHECK_MSG(!richEdit.IsMultiLine(), "keys: SetMultiLine(false)");
    richEdit.SetSel(1, 1);
    SendKey(richEdit, ui::kVK_RETURN);
    CheckText(env, L"abc", "keys: single line enter should not insert newline");
    CHECK_MSG(nReturnCount == 0, "keys: single line with want_return=true should not fire kEventReturn");

    //want_return=false：不插入换行，触发 kEventReturn
    richEdit.SetMultiLine(true);
    richEdit.SetWantReturn(false);
    richEdit.SetSel(1, 1);
    SendKey(richEdit, ui::kVK_RETURN);
    CheckText(env, L"abc", "keys: want_return=false should not insert newline");
    CHECK_MSG(nReturnCount == 1, "keys: want_return=false should fire kEventReturn");

    //want_return=true：插入换行
    richEdit.SetWantReturn(true);
    richEdit.SetSel(1, 1);
    SendKey(richEdit, ui::kVK_RETURN);
    CheckText(env, L"a\r\nbc", "keys: want_return=true should insert newline");

    //want_tab=false：TAB 不输入文本，触发 kEventTab
    richEdit.SetWantTab(false);
    richEdit.SetSel(0, 0);
    SendKey(richEdit, ui::kVK_TAB);
    CheckText(env, L"a\r\nbc", "keys: want_tab=false should not insert tab");
    CHECK_MSG(nTabCount == 1, "keys: want_tab=false should fire kEventTab");

    //TAB 字符经 OnChar 进入时会被过滤（GetInputTextW 过滤 0x00~0x1F 控制符）
    SendChar(richEdit, L'\t');
    CheckText(env, L"a\r\nbc", "keys: OnChar('\\t') should be filtered");

    //want_tab=true：TAB 作为文本输入（源码：OnKeyDown -> OnInputChar 路径）
    richEdit.SetWantTab(true);
    SendKey(richEdit, ui::kVK_TAB);
    CheckText(env, L"\ta\r\nbc", "keys: want_tab=true should insert tab via OnKeyDown");
    CHECK_MSG(nTabCount == 1, "keys: want_tab=true should not fire kEventTab");

#if defined(_WIN32)
    //Ctrl+回车：want_ctrl_return=false 不插入并触发 kEventReturn，true 时插入换行
    ScopedKeyPressed ctrlKey(ui::kVK_CONTROL);
    CHECK_MSG(ctrlKey.IsEffective(), "keys: simulate Ctrl key down should take effect");

    richEdit.SetWantCtrlReturn(false);
    richEdit.SetSel(0, 0);
    SendKey(richEdit, ui::kVK_RETURN);
    CheckText(env, L"\ta\r\nbc", "keys: want_ctrl_return=false should not insert newline");
    CHECK_MSG(nReturnCount == 2, "keys: want_ctrl_return=false should fire kEventReturn");

    richEdit.SetWantCtrlReturn(true);
    SendKey(richEdit, ui::kVK_RETURN);
    CheckText(env, L"\r\n\ta\r\nbc", "keys: want_ctrl_return=true should insert newline");
    CHECK_MSG(nReturnCount == 2, "keys: want_ctrl_return=true should not fire kEventReturn");
#endif
    SendKeyUp(richEdit, ui::kVK_CONTROL);

    richEdit.DetachEvent(ui::kEventReturn);
    richEdit.DetachEvent(ui::kEventTab);
}

/** 行间距与缩放：相邻行 y 间距变大；缩放后坐标按比例变化且行数不变 */
void TestRowSpacingAndZoom(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    //两行文本，第 1 行 3 个字符 + 换行符，第 2 行首字符索引为 4
    richEdit.SetText(_T("aaa\nbbb"));
    const int32_t nGapDefault = richEdit.PosFromChar(4).y - richEdit.PosFromChar(0).y;
    CHECK_MSG(nGapDefault > 0, "spacing: default row gap should be positive");

    richEdit.SetRowSpacingMul(2.0f);
    CHECK_MSG(richEdit.GetRowSpacingMul() == 2.0f, "spacing: GetRowSpacingMul");
    const int32_t nGapMul = richEdit.PosFromChar(4).y - richEdit.PosFromChar(0).y;
    CHECK_MSG(nGapMul > nGapDefault, "spacing: SetRowSpacingMul(2.0) should increase row gap");

    richEdit.SetRowSpacingMul(1.0f);
    richEdit.SetRowSpacingAdd(10.0f);
    CHECK_MSG(richEdit.GetRowSpacingAdd() == 10.0f, "spacing: GetRowSpacingAdd");
    const int32_t nGapAdd = richEdit.PosFromChar(4).y - richEdit.PosFromChar(0).y;
    CHECK_MSG(nGapAdd > nGapDefault, "spacing: SetRowSpacingAdd(10) should increase row gap");

    richEdit.SetRowSpacingAdd(0.0f);

    //缩放：三行文本，行数不变，行间距按比例增大
    richEdit.SetText(_T("abc\ndef\nghi"));
    const int32_t nLineCount = richEdit.GetLineCount();
    const ui::UiPoint pt0 = richEdit.PosFromChar(0);
    const ui::UiPoint pt1 = richEdit.PosFromChar(4);
    const int32_t nGap100 = pt1.y - pt0.y;
    CHECK_MSG(nGap100 > 0, "zoom: row gap at 100% should be positive");

    richEdit.SetZoomPercent(200);
    CHECK_MSG(richEdit.GetZoomPercent() == 200, "zoom: SetZoomPercent(200)");
    const int32_t nGap200 = richEdit.PosFromChar(4).y - richEdit.PosFromChar(0).y;
    CHECK_MSG(nGap200 > (nGap100 + nGap100 / 2), "zoom: row gap at 200% should be much larger");
    CHECK_MSG(richEdit.GetLineCount() == nLineCount, "zoom: line count should not change");

    richEdit.SetZoomPercent(100);
    CHECK_MSG(richEdit.GetZoomPercent() == 100, "zoom: restore 100%");
    const ui::UiPoint pt0Restored = richEdit.PosFromChar(0);
    const ui::UiPoint pt1Restored = richEdit.PosFromChar(4);
    CHECK_MSG(pt0Restored.x == pt0.x && pt0Restored.y == pt0.y, "zoom: position should be restored");
    CHECK_MSG(pt1Restored.x == pt1.x && pt1Restored.y == pt1.y, "zoom: position should be restored (line 2)");
}

/** 撤销/重做：bCanUndo 语义、CanUndo/Undo/Redo、EmptyUndoBuffer、SetUndoLimit */
void TestUndoRedo(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("abc"));
    CHECK_MSG(!richEdit.CanUndo(), "undo: SetText should clear the undo buffer");

    richEdit.InsertText(3, _T("d"), true);
    CheckText(env, L"abcd", "undo: InsertText(bCanUndo=true)");
    CHECK_MSG(richEdit.CanUndo(), "undo: CanUndo should be true after InsertText(bCanUndo=true)");
    CHECK_MSG(richEdit.Undo(), "undo: Undo should succeed");
    CheckText(env, L"abc", "undo: Undo should restore text");
    CHECK_MSG(richEdit.CanRedo(), "undo: CanRedo should be true after Undo");
    CHECK_MSG(richEdit.Redo(), "undo: Redo should succeed");
    CheckText(env, L"abcd", "undo: Redo should re-apply the edit");

    //bCanUndo=false 会清空撤销列表（源码：ReplaceText 中 ClearUndoList）
    richEdit.InsertText(0, _T("X"), false);
    CheckText(env, L"Xabcd", "undo: InsertText(bCanUndo=false)");
    CHECK_MSG(!richEdit.CanUndo(), "undo: CanUndo should be false after bCanUndo=false edit");
    CHECK_MSG(!richEdit.CanRedo(), "undo: CanRedo should be false after clearing undo buffer");
    CHECK_MSG(!richEdit.Undo(), "undo: Undo should return false when nothing to undo");

    //ReplaceSel 的 bCanUndo 语义
    richEdit.SetText(_T("hello"));
    richEdit.SetSel(0, 5);
    CHECK_MSG(richEdit.ReplaceSel(_T("hi"), true), "undo: ReplaceSel should succeed");
    CheckText(env, L"hi", "undo: ReplaceSel result");
    CHECK_MSG(richEdit.CanUndo(), "undo: CanUndo after ReplaceSel(bCanUndo=true)");
    CHECK_MSG(richEdit.Undo(), "undo: Undo ReplaceSel");
    CheckText(env, L"hello", "undo: Undo should restore replaced text");

    //EmptyUndoBuffer
    richEdit.InsertText(0, _T("A"), true);
    CHECK_MSG(richEdit.CanUndo(), "undo: CanUndo before EmptyUndoBuffer");
    richEdit.EmptyUndoBuffer();
    CHECK_MSG(!richEdit.CanUndo(), "undo: CanUndo should be false after EmptyUndoBuffer");
    CHECK_MSG(!richEdit.CanRedo(), "undo: CanRedo should be false after EmptyUndoBuffer");

    //连续 OnChar 合并为一次撤销（源码：bMergeUndo=true）
    richEdit.SetText(_T(""));
    richEdit.SetSel(0, 0);
    SendChar(richEdit, L'a');
    SendChar(richEdit, L'b');
    CheckText(env, L"ab", "undo: two OnChar inputs");
    CHECK_MSG(richEdit.CanUndo(), "undo: CanUndo after OnChar");
    CHECK_MSG(richEdit.Undo(), "undo: Undo merged OnChar");
    CheckText(env, L"", "undo: merged OnChar should be undone in one step");

    //SetUndoLimit(1)：只保留一步撤销
    richEdit.SetUndoLimit(1);
    richEdit.SetText(_T("abc"));
    richEdit.InsertText(0, _T("1"), true);
    richEdit.InsertText(1, _T("2"), true);
    CheckText(env, L"12abc", "undo: two edits with undo limit 1");
    CHECK_MSG(richEdit.Undo(), "undo: Undo with limit 1");
    CheckText(env, L"1abc", "undo: only the latest edit should be undone");
    CHECK_MSG(!richEdit.CanUndo(), "undo: only one undo step should be kept");
    richEdit.SetUndoLimit(64);
}

/** 选区与行查询 API：SetSel/GetSel/GetSelText/GetTextRange/行索引/GetLine */
void TestSelectionApis(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("0123456789"));
    richEdit.SetSel(2, 5);
    int32_t nStart = -1;
    int32_t nEnd = -1;
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 2 && nEnd == 5, "sel: SetSel(2,5)");
    CHECK_MSG(richEdit.HasSelText(), "sel: HasSelText should be true");
    CHECK_MSG(richEdit.GetSelText() == _T("234"), "sel: GetSelText");
    CHECK_MSG(richEdit.GetTextRange(1, 4) == _T("123"), "sel: GetTextRange(1,4)");
    CHECK_MSG(richEdit.GetTextRange(0, 10) == _T("0123456789"), "sel: GetTextRange full text");

    //越界与反向参数（源码：InternalSetSel 先交换，再夹取到 [0, nTextLen]）
    richEdit.SetSel(8, 3);
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 3 && nEnd == 8, "sel: SetSel(8,3) should swap to (3,8)");

    richEdit.SetSel(5, 100);
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 5 && nEnd == 10, "sel: SetSel(5,100) should clamp end to text length");

    richEdit.SetSel(100, 200);
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 10 && nEnd == 10, "sel: SetSel beyond text should collapse to end");

    //负的起始位置被当作"文本末尾"（源码：nStartChar < 0 -> nTextLen），之后交换
    richEdit.SetSel(-2, 3);
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 3 && nEnd == 10, "sel: SetSel(-2,3) negative start maps to text end");

    //(-1,0) 是特殊值：不改变光标位置
    richEdit.SetSel(4, 4);
    richEdit.SetSel(-1, 0);
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 4 && nEnd == 4, "sel: SetSel(-1,0) keeps current caret");

    CHECK_MSG(richEdit.SetSelAll() == 0, "sel: SetSelAll should return the selection start (0)");
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 0 && nEnd == 10, "sel: SetSelAll");
    richEdit.SetSelNone();
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == nEnd, "sel: SetSelNone should not keep a selection");

    //Clear 删除选区：删除 [2,4) 的 "23"
    richEdit.SetSel(2, 4);
    richEdit.Clear();
    CheckText(env, L"01456789", "sel: Clear should remove the selected range");
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 2 && nEnd == 2, "sel: caret should be at the deleted range start");

    //GetTextRange 越界/非法参数
    CHECK_MSG(richEdit.GetTextRange(5, 2).empty(), "sel: GetTextRange(5,2) should be empty");
    CHECK_MSG(richEdit.GetTextRange(-1, 3).empty(), "sel: GetTextRange(-1,3) should be empty");
    CHECK_MSG(richEdit.GetTextRange(0, 100).empty(), "sel: GetTextRange beyond text should be empty");
    CHECK_MSG(richEdit.GetTextRange(3, 3).empty(), "sel: empty range should return empty");

    //行查询 API（文本 "ab\ncde\nf"：行首 0/3/7）
    //注意：GetLine/LineLength 返回的行文本包含行尾的换行符（源码：GetRowText 不裁剪 '\n'）
    richEdit.SetText(_T("ab\ncde\nf"));
    CHECK_MSG(richEdit.GetLineCount() == 3, "line: GetLineCount");
    CHECK_MSG(richEdit.LineIndex(0) == 0, "line: LineIndex(0)");
    CHECK_MSG(richEdit.LineIndex(1) == 3, "line: LineIndex(1)");
    CHECK_MSG(richEdit.LineIndex(2) == 7, "line: LineIndex(2)");
    CHECK_MSG(richEdit.LineIndex(3) == -1, "line: LineIndex(out of range) should be -1");
    //源码：RowIndex 未实现"-1 表示当前插入行"（头文件注释与实现不一致），直接返回 -1
    CHECK_MSG(richEdit.LineIndex(-1) == -1, "line: LineIndex(-1) returns -1 (source semantics)");

    CHECK_MSG(richEdit.LineLength(0) == 3, "line: LineLength(0) includes the line break");
    CHECK_MSG(richEdit.LineLength(1) == 4, "line: LineLength(1) includes the line break");
    CHECK_MSG(richEdit.LineLength(2) == 1, "line: LineLength(2)");
    CHECK_MSG(richEdit.LineLength(3) == 0, "line: LineLength(out of range) should be 0");
    CHECK_MSG(richEdit.LineLength(-1) == 0, "line: LineLength(-1) returns 0 (source semantics)");

    CHECK_MSG(richEdit.LineFromChar(0) == 0, "line: LineFromChar(0)");
    CHECK_MSG(richEdit.LineFromChar(3) == 1, "line: LineFromChar(3)");
    CHECK_MSG(richEdit.LineFromChar(4) == 1, "line: LineFromChar(4)");
    CHECK_MSG(richEdit.LineFromChar(7) == 2, "line: LineFromChar(7)");
    CHECK_MSG(richEdit.LineFromChar(8) == 3, "line: LineFromChar(text length) returns line count");
    CHECK_MSG(richEdit.LineFromChar(-1) == 0, "line: LineFromChar(-1) returns 0 (source semantics)");

    CHECK_MSG(richEdit.GetLine(0, 100) == _T("ab\n"), "line: GetLine(0) keeps the line break");
    CHECK_MSG(richEdit.GetLine(1, 100) == _T("cde\n"), "line: GetLine(1) keeps the line break");
    CHECK_MSG(richEdit.GetLine(2, 100) == _T("f"), "line: GetLine(2)");
    CHECK_MSG(richEdit.GetLine(3, 100).empty(), "line: GetLine(out of range) should be empty");
    CHECK_MSG(richEdit.GetLine(-1, 100).empty(), "line: GetLine(-1) should be empty");
    CHECK_MSG(richEdit.GetLine(0, 1) == _T("a"), "line: GetLine max length should truncate");
    CHECK_MSG(richEdit.GetLine(0, 0).empty(), "line: GetLine max length 0 should be empty");
}

/** 光标命令键：Home/End、Ctrl+Home/End、PageUp/PageDown、Shift+Left/Right */
void TestCaretCommandKeys(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    richEdit.SetText(_T("abc\ndef\nghi"));
    richEdit.SetSel(6, 6);
    int32_t nSelStart = -1;
    int32_t nSelEnd = -1;
    SendKey(richEdit, ui::kVK_HOME);
    CHECK_MSG(GetCaretChar(richEdit) == 4, "keys: Home should move caret to line start");
    SendKey(richEdit, ui::kVK_END);
    CHECK_MSG(GetCaretChar(richEdit) == 7, "keys: End should move caret to line end");

    richEdit.SetSel(0, 0);
    SendKey(richEdit, ui::kVK_HOME);
    CHECK_MSG(GetCaretChar(richEdit) == 0, "keys: Home at text start should stay");
    richEdit.SetSel(10, 10);
    SendKey(richEdit, ui::kVK_END);
    CHECK_MSG(GetCaretChar(richEdit) == 11, "keys: End on last line should go to text end");

#if defined(_WIN32)
    {
        ScopedKeyPressed ctrlKey(ui::kVK_CONTROL);
        CHECK_MSG(ctrlKey.IsEffective(), "keys: simulate Ctrl key down should take effect");

        richEdit.SetSel(6, 6);
        SendKey(richEdit, ui::kVK_CONTROL);
        SendKey(richEdit, ui::kVK_HOME);
        CHECK_MSG(GetCaretChar(richEdit) == 0, "keys: Ctrl+Home should move caret to text start");

        richEdit.SetSel(2, 2);
        SendKey(richEdit, ui::kVK_END);
        CHECK_MSG(GetCaretChar(richEdit) == 11, "keys: Ctrl+End should move caret to text end");
    }
    SendKeyUp(richEdit, ui::kVK_CONTROL);

    {
        //Shift + Left/Right 扩展选区（Ctrl 必须已抬起，否则会走 Ctrl+方向键分支）
        ScopedKeyPressed shiftKey(ui::kVK_SHIFT);
        CHECK_MSG(shiftKey.IsEffective(), "keys: simulate Shift key down should take effect");

        richEdit.SetText(_T("abcdef"));
        richEdit.SetSel(2, 2);
        SendKey(richEdit, ui::kVK_SHIFT);
        SendKey(richEdit, ui::kVK_RIGHT);
        richEdit.GetSel(nSelStart, nSelEnd);
        CHECK_MSG(nSelStart == 2 && nSelEnd == 3, "keys: Shift+Right should extend selection");
        CHECK_MSG(richEdit.GetSelText() == _T("c"), "keys: Shift+Right selected text");
        SendKey(richEdit, ui::kVK_RIGHT);
        richEdit.GetSel(nSelStart, nSelEnd);
        CHECK_MSG(nSelStart == 2 && nSelEnd == 4, "keys: second Shift+Right should extend selection");
        CHECK_MSG(richEdit.GetSelText() == _T("cd"), "keys: second Shift+Right selected text");
        SendKey(richEdit, ui::kVK_LEFT);
        richEdit.GetSel(nSelStart, nSelEnd);
        CHECK_MSG(nSelStart == 2 && nSelEnd == 3, "keys: Shift+Left should shrink selection");
        CHECK_MSG(richEdit.GetSelText() == _T("c"), "keys: Shift+Left selected text");
    }
    SendKeyUp(richEdit, ui::kVK_SHIFT);
#endif

    //PageDown/PageUp：光标按页移动，并滚动视图（离屏环境需手动设置滚动范围）
    const int32_t nLines = 100;
    DStringW text = MakeTextLines(nLines);
    richEdit.SetText(text);
    richEdit.SetSel(0, 0);
    richEdit.EnableScrollBar(true, false);
    PaintFrame(env);

    const DStringW line0 = L"行 Line 0 中文文本 test";
    const int32_t nRowHeight = richEdit.PosFromChar((int32_t)line0.size() + 1).y - richEdit.PosFromChar(0).y;
    CHECK_MSG(nRowHeight > 0, "keys: row height should be positive");

    ui::ScrollBar* pVScrollBar = richEdit.GetVScrollBar();
    CHECK_MSG(pVScrollBar != nullptr, "keys: vertical scroll bar should be created");
    if (pVScrollBar != nullptr) {
        pVScrollBar->SetScrollRange((int64_t)nRowHeight * nLines - kHeight);
        richEdit.SetScrollPosY(0);
        PaintFrame(env);
        richEdit.SetSel(0, 0);

        SendKey(richEdit, ui::kVK_NEXT);
        const int32_t nAfterPageDown1 = GetCaretChar(richEdit);
        CHECK_MSG(nAfterPageDown1 > 0, "keys: PageDown should move caret forward");

        //第 2 次 PageDown 后光标到达第 2 页，EnsureCharVisible 应滚动视图
        SendKey(richEdit, ui::kVK_NEXT);
        const int32_t nAfterPageDown2 = GetCaretChar(richEdit);
        CHECK_MSG(nAfterPageDown2 > nAfterPageDown1, "keys: second PageDown should move caret further");
        CHECK_MSG(richEdit.GetScrollOffset().cy > 0, "keys: PageDown should scroll the view");

        SendKey(richEdit, ui::kVK_PRIOR);
        const int32_t nAfterPageUp = GetCaretChar(richEdit);
        CHECK_MSG(nAfterPageUp < nAfterPageDown2, "keys: PageUp should move caret backward");
    }
}

/** 鼠标选区：单击定位、拖动选择、双击选词 */
void TestMouseSelection(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();
    richEdit.SetEnableDragOut(false);
    richEdit.SetText(_T("alpha beta gamma"));

    //单击定位：光标位置应与 CharFromPos 一致
    const ui::UiPoint ptClick = richEdit.PosFromChar(3);
    SendMouseDown(richEdit, ptClick);
    SendMouseUp(richEdit, ptClick);
    int32_t nStart = -1;
    int32_t nEnd = -1;
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == nEnd, "mouse: click should not select text");
    CHECK_MSG(nEnd == richEdit.CharFromPos(ptClick), "mouse: click caret should match CharFromPos");
    CHECK_MSG(nEnd == 3, "mouse: click should place caret at char 3");

    //拖动选择 [0,5)：按下 -> 移动 -> 抬起
    const ui::UiPoint ptStart = richEdit.PosFromChar(0);
    const ui::UiPoint ptEnd = richEdit.PosFromChar(5);
    SendMouseDown(richEdit, ptStart);
    SendMouseMove(richEdit, ptEnd);
    SendMouseUp(richEdit, ptEnd);
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 0 && nEnd == 5, "mouse: drag should select range [0,5)");
    CHECK_MSG(richEdit.GetSelText() == _T("alpha"), "mouse: drag selected text");

    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "mouse: text should be painted after drag selection");

    //双击选词：点击 "beta" 中间
    const ui::UiPoint ptWord = richEdit.PosFromChar(7);
    SendMouseDoubleClick(richEdit, ptWord);
    richEdit.GetSel(nStart, nEnd);
    CHECK_MSG(nStart == 6 && nEnd == 10, "mouse: double click should select the word range");
    CHECK_MSG(richEdit.GetSelText() == _T("beta"), "mouse: double click selected text");

    PaintFrame(env);
    CHECK_MSG(CountInkPixels(env) > 0, "mouse: text should be painted after double click");
}

/** 事件：TextChanged/SelChanged/Return/Tab/Zoom/TextLimit 的触发次数与 SetTextNoEvent 差异 */
void TestEditEvents(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    int32_t nTextChanged = 0;
    int32_t nSelChanged = 0;
    int32_t nReturn = 0;
    int32_t nTab = 0;
    int32_t nZoom = 0;
    int32_t nTextLimit = 0;
    richEdit.AttachTextChanged([&nTextChanged](const ui::EventArgs&) { ++nTextChanged; return true; }, 1);
    richEdit.AttachSelChanged([&nSelChanged](const ui::EventArgs&) { ++nSelChanged; return true; }, 1);
    richEdit.AttachReturn([&nReturn](const ui::EventArgs&) { ++nReturn; return true; }, 1);
    richEdit.AttachTab([&nTab](const ui::EventArgs&) { ++nTab; return true; }, 1);
    richEdit.AttachZoom([&nZoom](const ui::EventArgs&) { ++nZoom; return true; }, 1);
    richEdit.AttachTextLimit([&nTextLimit](const ui::EventArgs&) { ++nTextLimit; return true; }, 1);

    //TextChanged：内容变化才触发
    const int32_t nTextBase = nTextChanged;
    richEdit.SetText(_T("abc"));
    CHECK_MSG(nTextChanged == nTextBase + 1, "events: SetText should fire kEventTextChanged");
    richEdit.SetText(_T("abc"));
    CHECK_MSG(nTextChanged == nTextBase + 1, "events: SetText with same text should not fire event");

    //SetTextNoEvent：内容变化但不触发事件
    const int32_t nTextBase2 = nTextChanged;
    richEdit.SetTextNoEvent(_T("xyz"));
    CHECK_MSG(nTextChanged == nTextBase2, "events: SetTextNoEvent should not fire kEventTextChanged");
    CheckText(env, L"xyz", "events: SetTextNoEvent should still change text");
    richEdit.SetText(_T("xyz"));
    CHECK_MSG(nTextChanged == nTextBase2, "events: SetText with unchanged text should not fire event");

    const int32_t nTextBase3 = nTextChanged;
    richEdit.InsertText(3, _T("d"), false);
    CHECK_MSG(nTextChanged == nTextBase3 + 1, "events: InsertText should fire kEventTextChanged");

    //SelChanged：选择变化才触发
    const int32_t nSelBase = nSelChanged;
    richEdit.SetSel(0, 1);
    CHECK_MSG(nSelChanged == nSelBase + 1, "events: SetSel should fire kEventSelChanged");
    richEdit.SetSel(0, 1);
    CHECK_MSG(nSelChanged == nSelBase + 1, "events: unchanged selection should not fire kEventSelChanged");

    //Return / Tab
    richEdit.SetWantReturn(false);
    richEdit.SetSel(0, 0);
    SendKey(richEdit, ui::kVK_RETURN);
    CHECK_MSG(nReturn == 1, "events: kEventReturn should fire once when want_return=false");
    richEdit.SetWantReturn(true);
    SendKey(richEdit, ui::kVK_RETURN);
    CHECK_MSG(nReturn == 1, "events: want_return=true should not fire kEventReturn");

    richEdit.SetWantTab(false);
    SendKey(richEdit, ui::kVK_TAB);
    CHECK_MSG(nTab == 1, "events: kEventTab should fire once when want_tab=false");
    richEdit.SetWantTab(true);
    SendKey(richEdit, ui::kVK_TAB);
    CHECK_MSG(nTab == 1, "events: want_tab=true should not fire kEventTab");

    //Zoom：仅比例变化时触发
    const int32_t nZoomBase = nZoom;
    richEdit.SetZoomPercent(150);
    CHECK_MSG(nZoom == nZoomBase + 1, "events: SetZoomPercent should fire kEventZoom");
    richEdit.SetZoomPercent(150);
    CHECK_MSG(nZoom == nZoomBase + 1, "events: unchanged zoom should not fire kEventZoom");
    richEdit.SetZoomPercent(100);

    //TextLimit：超限才触发
    richEdit.SetLimitText(3);
    richEdit.SetText(_T("abc"));
    const int32_t nLimitBase = nTextLimit;
    SendChar(richEdit, L'd');
    CHECK_MSG(nTextLimit == nLimitBase + 1, "events: over-limit input should fire kEventTextLimit");
    CheckText(env, L"abc", "events: over-limit input should not change text");
    richEdit.SetLimitText(0);

    richEdit.DetachEvent(ui::kEventTextChanged);
    richEdit.DetachEvent(ui::kEventSelChanged);
    richEdit.DetachEvent(ui::kEventReturn);
    richEdit.DetachEvent(ui::kEventTab);
    richEdit.DetachEvent(ui::kEventZoom);
    richEdit.DetachEvent(ui::kEventTextLimit);
}

/** 系统剪贴板：Copy/Cut/Paste/CanPaste 的文本与选区变化，
 *  以及与只读、密码、单行、数字、字符集、长度限制等选项的交互。
 *
 * 说明：本组用例会读写系统剪贴板；开始前备份剪贴板的全部格式（文本、图片、文件等），
 * 结束后（含提前返回）自动恢复；若运行环境无法访问剪贴板，打印 WARN 后跳过。
 */
void TestClipboard(TestEnv& env)
{
    //备份系统剪贴板的所有格式，测试结束时由析构函数恢复
    ClipboardBackup clipboardBackup;

    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    const DStringW sentinel = L"__duilib_clipboard_sentinel__";
    auto SetClip = [](const DStringW& text) { return ui::Clipboard::SetClipboardText(text); };
    auto GetClip = []() {
        DStringW text;
        ui::Clipboard::GetClipboardText(text);
        return text;
    };
    auto GetSelRange = [&richEdit](int32_t& nStart, int32_t& nEnd) {
        nStart = -1;
        nEnd = -1;
        richEdit.GetSel(nStart, nEnd);
    };
    if (!clipboardBackup.IsAvailable() || !SetClip(sentinel) || (GetClip() != sentinel)) {
        std::cout << "[WARN] clipboard: system clipboard is not available, tests skipped" << std::endl;
        return;
    }

    int32_t nTextChanged = 0;
    richEdit.AttachTextChanged([&nTextChanged](const ui::EventArgs&) { ++nTextChanged; return true; }, 1);

    int32_t nStart = -1;
    int32_t nEnd = -1;

    //1) Copy：选中文本写入剪贴板，不改变文本与选区，也不触发事件
    const DStringW text1 = L"hello world";
    richEdit.SetText(text1);
    richEdit.SetSel(0, 5);
    const int32_t nCopyBase = nTextChanged;
    richEdit.Copy();
    CHECK_MSG(GetClip() == L"hello", "clipboard: Copy should put the selected text into the clipboard");
    CheckText(env, text1, "clipboard: Copy should not change the text");
    GetSelRange(nStart, nEnd);
    CHECK_MSG(nStart == 0 && nEnd == 5, "clipboard: Copy should keep the selection");
    CHECK_MSG(nTextChanged == nCopyBase, "clipboard: Copy should not fire kEventTextChanged");

    //2) 只读模式下仍可复制
    richEdit.SetReadOnly(true);
    SetClip(sentinel);
    richEdit.Copy();
    CHECK_MSG(GetClip() == L"hello", "clipboard: Copy in read-only mode should still work");
    richEdit.SetReadOnly(false);

    //3) 无选区时复制：不修改剪贴板（与系统编辑框行为一致）
    SetClip(sentinel);
    richEdit.SetSel(3, 3);
    richEdit.Copy();
    CHECK_MSG(GetClip() == sentinel, "clipboard: Copy without selection should not change the clipboard");

    //4) 密码模式下禁止复制（避免密码泄漏）
    richEdit.SetPasswordMode(true);
    richEdit.SetText(L"secret");
    richEdit.SetSel(0, 6);
    SetClip(sentinel);
    richEdit.Copy();
    CHECK_MSG(GetClip() == sentinel, "clipboard: Copy in password mode should be ignored");
    richEdit.SetPasswordMode(false);

    //5) Cut：写入剪贴板、删除选中文本、光标回到选区起点、触发一次文本变化，且可撤销
    richEdit.SetText(text1);
    richEdit.SetSel(0, 5);
    SetClip(sentinel);
    const int32_t nCutBase = nTextChanged;
    richEdit.Cut();
    CHECK_MSG(GetClip() == L"hello", "clipboard: Cut should put the selected text into the clipboard");
    CheckText(env, L" world", "clipboard: Cut should remove the selected text");
    GetSelRange(nStart, nEnd);
    CHECK_MSG(nStart == 0 && nEnd == 0, "clipboard: Cut should collapse the selection to its start");
    CHECK_MSG(nTextChanged == nCutBase + 1, "clipboard: Cut should fire kEventTextChanged once");
    CHECK_MSG(richEdit.CanUndo(), "clipboard: Cut should be undoable");
    richEdit.Undo();
    CheckText(env, text1, "clipboard: Cut should be restored by Undo");

    //6) 无选区时剪切：不修改文本与剪贴板
    SetClip(sentinel);
    richEdit.SetSel(2, 2);
    richEdit.Cut();
    CHECK_MSG(GetClip() == sentinel, "clipboard: Cut without selection should not change the clipboard");
    CheckText(env, text1, "clipboard: Cut without selection should not change the text");

    //7) 只读/密码模式下禁止剪切
    richEdit.SetReadOnly(true);
    richEdit.SetSel(0, 5);
    SetClip(sentinel);
    richEdit.Cut();
    CHECK_MSG(GetClip() == sentinel, "clipboard: Cut in read-only mode should be ignored");
    CheckText(env, text1, "clipboard: Cut in read-only mode should not change the text");
    richEdit.SetReadOnly(false);

    richEdit.SetPasswordMode(true);
    richEdit.SetText(L"secret");
    richEdit.SetSel(0, 6);
    SetClip(sentinel);
    richEdit.Cut();
    CHECK_MSG(GetClip() == sentinel, "clipboard: Cut in password mode should be ignored");
    CheckText(env, L"secret", "clipboard: Cut in password mode should not change the text");
    richEdit.SetPasswordMode(false);

    //8) Paste：插入到光标位置，光标移到插入文本之后，触发一次文本变化
    richEdit.SetText(L"AB");
    richEdit.SetSel(1, 1);
    CHECK_MSG(SetClip(L"PASTE"), "clipboard: setup paste text failed");
    const int32_t nPasteBase = nTextChanged;
    richEdit.Paste();
    CheckText(env, L"APASTEB", "clipboard: Paste should insert at the caret");
    GetSelRange(nStart, nEnd);
    CHECK_MSG(nStart == 6 && nEnd == 6, "clipboard: Paste should place the caret after the pasted text");
    CHECK_MSG(nTextChanged == nPasteBase + 1, "clipboard: Paste should fire kEventTextChanged once");

    //9) Paste：替换当前选区
    richEdit.SetText(L"hello world");
    richEdit.SetSel(6, 11);
    SetClip(L"there");
    richEdit.Paste();
    CheckText(env, L"hello there", "clipboard: Paste should replace the selection");
    GetSelRange(nStart, nEnd);
    CHECK_MSG(nStart == 11 && nEnd == 11, "clipboard: Paste should place the caret at the end of the pasted text");

    //10) 空剪贴板：Paste 不修改文本，CanPaste 返回 false
    richEdit.SetText(L"abc");
    richEdit.SetSel(1, 1);
    SetClip(L"");
    richEdit.Paste();
    CheckText(env, L"abc", "clipboard: Paste with empty clipboard should do nothing");
    GetSelRange(nStart, nEnd);
    CHECK_MSG(nStart == 1 && nEnd == 1, "clipboard: empty paste should keep the caret");
    CHECK_MSG(!richEdit.CanPaste(), "clipboard: CanPaste should be false when the clipboard is empty");

    //11) 只读模式：禁止粘贴
    richEdit.SetReadOnly(true);
    SetClip(L"XYZ");
    richEdit.SetText(L"abc");
    richEdit.SetSel(3, 3);
    richEdit.Paste();
    CheckText(env, L"abc", "clipboard: Paste in read-only mode should be ignored");
    CHECK_MSG(!richEdit.CanPaste(), "clipboard: CanPaste should be false in read-only mode");
    richEdit.SetReadOnly(false);

    //12) 单行模式：粘贴时把换行符（\r\n、\r、\n）各规整为一个空格
    richEdit.SetMultiLine(false);
    richEdit.SetText(L"ab");
    richEdit.SetSel(2, 2);
    SetClip(L"X\r\nY\rZ\nW");
    richEdit.Paste();
    CheckText(env, L"abX Y Z W", "clipboard: Paste in single-line mode should replace line breaks with spaces");
    richEdit.SetMultiLine(true);

    //13) 密码模式：粘贴时移除换行符/TAB，且不记录撤销
    richEdit.SetPasswordMode(true);
    richEdit.SetText(L"");
    richEdit.EmptyUndoBuffer();
    SetClip(L"a\nb\tc");
    richEdit.Paste();
    CheckText(env, L"abc", "clipboard: Paste in password mode should remove newline/tab");
    CHECK_MSG(!richEdit.CanUndo(), "clipboard: Paste in password mode should not be undoable");
    richEdit.SetPasswordMode(false);

    //14) 数字模式：含非数字字符的剪贴板内容禁止粘贴
    richEdit.SetNumberOnly(true);
    richEdit.SetText(L"1");
    richEdit.SetSel(1, 1);
    SetClip(L"12a");
    CHECK_MSG(!richEdit.CanPaste(), "clipboard: CanPaste should be false for non-number text in number-only mode");
    richEdit.Paste();
    CheckText(env, L"1", "clipboard: non-number paste should be rejected");
    SetClip(L"-");
    CHECK_MSG(!richEdit.CanPaste(), "clipboard: a lone minus sign should not be pasteable");
    SetClip(L"-12");
    CHECK_MSG(richEdit.CanPaste(), "clipboard: leading minus followed by digits should be pasteable");
    richEdit.Paste();
    CheckText(env, L"1-12", "clipboard: number paste with a leading minus");
    richEdit.SetNumberOnly(false);

    //15) 字符集限制：剪贴板中有不允许的字符时禁止粘贴
    richEdit.SetLimitChars(_T("#0123456789"));
    SetClip(L"#12");
    CHECK_MSG(richEdit.CanPaste(), "clipboard: CanPaste should allow chars within limit_chars");
    richEdit.SetText(L"");
    richEdit.SetSel(0, 0);
    richEdit.Paste();
    CheckText(env, L"#12", "clipboard: Paste with allowed chars");
    SetClip(L"12A");
    CHECK_MSG(!richEdit.CanPaste(), "clipboard: CanPaste should be false when a char is not allowed");
    richEdit.Paste();
    CheckText(env, L"#12", "clipboard: Paste with a disallowed char should be rejected");
    richEdit.SetLimitChars(DString());

    //16) 长度限制：粘贴后超限时整次粘贴被丢弃，并触发 kEventTextLimit
    richEdit.SetLimitText(5);
    richEdit.SetText(L"abc");
    richEdit.EmptyUndoBuffer();
    richEdit.SetSel(3, 3);
    SetClip(L"de");
    richEdit.Paste();
    CheckText(env, L"abcde", "clipboard: Paste exactly up to the limit should succeed");
    int32_t nLimitCount = 0;
    richEdit.AttachTextLimit([&nLimitCount](const ui::EventArgs&) { ++nLimitCount; return true; }, 1);
    SetClip(L"f");
    richEdit.Paste();
    CheckText(env, L"abcde", "clipboard: Paste over the limit should be dropped");
    CHECK_MSG(nLimitCount == 1, "clipboard: Paste over the limit should fire kEventTextLimit once");
    richEdit.DetachEvent(ui::kEventTextLimit);
    richEdit.SetLimitText(0);

    //17) 多行文本：Copy 后 Paste 到空文本可完整还原（含换行符）
    const DStringW multiLine = L"line1\nline2\nline3";
    richEdit.SetText(multiLine);
    richEdit.SetSelAll();
    richEdit.Copy();
    CHECK_MSG(GetClip() == multiLine, "clipboard: multi-line Copy should keep the line breaks");
    richEdit.SetText(L"");
    richEdit.SetSel(0, 0);
    richEdit.Paste();
    CheckText(env, multiLine, "clipboard: multi-line copy/paste round trip");
    CHECK_MSG(richEdit.GetLineCount() == 3, "clipboard: pasted multi-line text should have 3 lines");

    //18) Windows 剪贴板常见的 CRLF 文本：粘贴后字符保持不变
    richEdit.SetText(L"");
    richEdit.SetSel(0, 0);
    SetClip(L"a\r\nb");
    richEdit.Paste();
    CheckText(env, L"a\r\nb", "clipboard: Paste CRLF text should keep the characters");
    CHECK_MSG(richEdit.GetLineCount() == 2, "clipboard: pasted CRLF text should have 2 lines");

#if defined(_WIN32)
    //19) 键盘快捷键：Ctrl+C / Ctrl+X / Ctrl+V / Shift+Insert
    {
        ScopedKeyPressed ctrl(VK_CONTROL);
        CHECK_MSG(ctrl.IsEffective(), "clipboard: cannot simulate the Ctrl key state");
        if (ctrl.IsEffective()) {
            richEdit.SetText(L"abc");
            richEdit.SetSel(0, 1);
            SetClip(sentinel);
            SendKey(richEdit, (ui::VirtualKeyCode)'C');
            CHECK_MSG(GetClip() == L"a", "clipboard: Ctrl+C should copy");
            SendKey(richEdit, (ui::VirtualKeyCode)'X');
            CHECK_MSG(GetClip() == L"a", "clipboard: Ctrl+X should cut into the clipboard");
            CheckText(env, L"bc", "clipboard: Ctrl+X should remove the selected text");
            SetClip(L"XY");
            richEdit.SetSel(2, 2);
            SendKey(richEdit, (ui::VirtualKeyCode)'V');
            CheckText(env, L"bcXY", "clipboard: Ctrl+V should paste");
        }
    }
    {
        ScopedKeyPressed shift(VK_SHIFT);
        CHECK_MSG(shift.IsEffective(), "clipboard: cannot simulate the Shift key state");
        if (shift.IsEffective()) {
            SetClip(L"Z");
            const int32_t nCaret = richEdit.GetTextLength();
            richEdit.SetSel(nCaret, nCaret);
            SendKey(richEdit, ui::kVK_INSERT);
            CheckText(env, L"bcXYZ", "clipboard: Shift+Insert should paste");
        }
    }
#endif

    richEdit.DetachEvent(ui::kEventTextChanged);
    //原有剪贴板内容由 clipboardBackup 的析构函数按原格式恢复
}

/** 文本查找：FindRichText 的正反向查找、大小写与全字匹配、查找范围边界与异常参数
 *
 * 参数约定（与 examples/RichEdit 中的用法一致）：
 *   - 正向查找（bFindDown = true）：chrg.cpMin < chrg.cpMax，在 [cpMin, cpMax) 内查找；
 *   - 反向查找（bFindDown = false）：chrg.cpMin 为起始位置，chrg.cpMax 为下界，
 *     即要求 chrg.cpMin > chrg.cpMax，在 [cpMax, cpMin) 内从后向前查找。
 */
void TestFindRichText(TestEnv& env)
{
    ResetEdit(env);
    TestRichEdit2& richEdit = env.Edit();

    auto Find = [&richEdit](const DString& findText, int32_t nMin, int32_t nMax,
                            bool bFindDown, bool bMatchCase, bool bMatchWholeWord,
                            int32_t& nFoundMin, int32_t& nFoundMax) {
        ui::FindTextParam findParam;
        findParam.findText = findText;
        findParam.chrg.cpMin = nMin;
        findParam.chrg.cpMax = nMax;
        findParam.bFindDown = bFindDown;
        findParam.bMatchCase = bMatchCase;
        findParam.bMatchWholeWord = bMatchWholeWord;
        ui::TextCharRange chrgText;
        const bool bFound = richEdit.FindRichText(findParam, chrgText);
        nFoundMin = chrgText.cpMin;
        nFoundMax = chrgText.cpMax;
        return bFound;
    };

    int32_t nFoundMin = -1;
    int32_t nFoundMax = -1;
    bool bFound = false;

    //1) 正向查找：返回第一个匹配项，范围外（区间结束之后）的匹配项不返回
    richEdit.SetText(L"hello world hello");
    bFound = Find(_T("hello"), 0, 17, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 5, "find: forward should return the first match");
    bFound = Find(_T("hello"), 6, 17, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 12 && nFoundMax == 17, "find: forward should start from chrg.cpMin");
    bFound = Find(_T("world"), 0, 11, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 6 && nFoundMax == 11, "find: match ending at chrg.cpMax should be found");
    bFound = Find(_T("world"), 0, 10, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound && nFoundMin == -1 && nFoundMax == -1, "find: match beyond chrg.cpMax should not be found");

    //2) 反向查找：cpMin 为起始位置，cpMax 为下界（cpMin > cpMax）
    bFound = Find(_T("hello"), 17, 0, false, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 12 && nFoundMax == 17, "find: backward should return the last match");
    bFound = Find(_T("hello"), 12, 0, false, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 5, "find: backward should not return matches after chrg.cpMin");

    //3) 大小写匹配
    richEdit.SetText(L"Hello HELLO hello");
    bFound = Find(_T("Hello"), 0, 17, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 5, "find: match case should distinguish Hello");
    bFound = Find(_T("hello"), 0, 17, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 12 && nFoundMax == 17, "find: match case should not find lowercase at 0");
    bFound = Find(_T("hello"), 0, 17, true, false, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 5, "find: ignore case should match the first occurrence");
    bFound = Find(_T("hello"), 17, 0, false, false, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 12 && nFoundMax == 17, "find: ignore case backward should return the last match");

    //4) 全字匹配：跳过包含目标词的更长单词
    richEdit.SetText(L"cat scatter cat");
    bFound = Find(_T("cat"), 0, 15, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 3, "find: whole word should skip the cat inside scatter");
    bFound = Find(_T("cat"), 4, 15, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 12 && nFoundMax == 15, "find: whole word should find the last cat");
    bFound = Find(_T("cat"), 0, 12, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 3, "find: whole word should ignore a match beyond chrg.cpMax");
    bFound = Find(_T("cat"), 4, 15, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 5 && nFoundMax == 8, "find: non-whole-word should find cat inside scatter");

    //5) 全字匹配：起始边界使用查找范围之外的字符判断
    richEdit.SetText(L"xabc abc");
    bFound = Find(_T("abc"), 1, 8, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 5 && nFoundMax == 8, "find: whole word should reject a match preceded by an alnum char");
    bFound = Find(_T("abc"), 1, 4, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: whole word should use the char before chrg.cpMin");

    //6) 全字匹配：结束边界使用查找范围之外的字符判断
    richEdit.SetText(L"abc abcx");
    bFound = Find(_T("abc"), 0, 7, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 3, "find: whole word should reject a match followed by an alnum char");
    bFound = Find(_T("abc"), 4, 8, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: whole word should use the char after chrg.cpMax");

    //7) 全字匹配：文本末尾与无字母数字边界的匹配文本
    richEdit.SetText(L"abc");
    bFound = Find(_T("abc"), 0, 3, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 3, "find: whole word at the end of text");
    bFound = Find(_T("abcd"), 0, 3, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: a longer word should not match");
    richEdit.SetText(L"a++b++");
    bFound = Find(_T("++"), 0, 6, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 1 && nFoundMax == 3, "find: whole word with non-alnum pattern");
    bFound = Find(_T("++"), 3, 6, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 4 && nFoundMax == 6, "find: whole word with non-alnum pattern (forward)");

    //8) 全字匹配：整段都是同一字符时没有词边界，不应匹配
    richEdit.SetText(L"aaaa");
    bFound = Find(_T("aa"), 0, 4, true, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: whole word should not match inside a run of the same char");

    //9) 反向 + 全字匹配：从后向前逐项校验词边界
    richEdit.SetText(L"cat scatter cat");
    bFound = Find(_T("cat"), 15, 0, false, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 12 && nFoundMax == 15, "find: backward whole word should find the last cat");
    bFound = Find(_T("cat"), 12, 0, false, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 3, "find: backward whole word should skip scatter");
    richEdit.SetText(L"abc abcx");
    bFound = Find(_T("abc"), 8, 0, false, true, true, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 3, "find: backward whole word should skip a mid-word match");

    //10) 跨行查找：匹配文本可以包含换行符
    richEdit.SetText(L"line1\nline2");
    bFound = Find(_T("1\nline"), 0, 11, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 4 && nFoundMax == 10, "find: match across lines");

    //11) 中文文本（与大小写无关，验证按字符索引返回）
    richEdit.SetText(L"中文测试中文");
    bFound = Find(_T("中文"), 0, 6, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 2, "find: Chinese text forward");
    bFound = Find(_T("中文"), 6, 0, false, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 4 && nFoundMax == 6, "find: Chinese text backward");

    //12) 查找范围参数异常与边界
    richEdit.SetText(L"hello");
    bFound = Find(_T("hello"), 0, 100, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 5, "find: chrg.cpMax beyond text should be clamped");
    bFound = Find(_T("hello"), 100, 0, false, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(bFound && nFoundMin == 0 && nFoundMax == 5, "find: chrg.cpMin beyond text should be clamped");
    bFound = Find(_T("hello"), 100, 200, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: range fully beyond text should not find anything");
    bFound = Find(_T("hello"), 2, 2, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: empty range should not find anything");
    bFound = Find(_T("hello"), -1, 5, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: negative range should not find anything");
    bFound = Find(_T(""), 0, 5, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: empty search text should not find anything");
    CHECK_MSG(nFoundMin == -1 && nFoundMax == -1, "find: failed search should reset the result range");

    //13) 空文本与不存在的文本
    richEdit.SetText(L"");
    bFound = Find(_T("a"), 0, 0, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: empty text should not find anything");
    richEdit.SetText(L"hello world");
    bFound = Find(_T("xyz"), 0, 11, true, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: missing text should not be found");
    bFound = Find(_T("xyz"), 11, 0, false, true, false, nFoundMin, nFoundMax);
    CHECK_MSG(!bFound, "find: missing text should not be found backward");

    //14) 查找不改变文本与选区
    richEdit.SetText(L"hello world");
    richEdit.SetSel(2, 4);
    Find(_T("world"), 0, 11, true, true, false, nFoundMin, nFoundMax);
    CheckText(env, L"hello world", "find: should not change the text");
    int32_t nSelStart = -1;
    int32_t nSelEnd = -1;
    richEdit.GetSel(nSelStart, nSelEnd);
    CHECK_MSG(nSelStart == 2 && nSelEnd == 4, "find: should not change the selection");
}

} // namespace

int main()
{
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(ui::FilePath(_T(DUILIB_TEST_RESOURCE_DIR))))) {
        std::cerr << "[FAIL] GlobalManager::Startup failed" << std::endl;
        return 1;
    }
    struct GlobalShutdown
    {
        ~GlobalShutdown() { ui::GlobalManager::Instance().Shutdown(); }
    } shutdownGuard;

    TestEnv env;
    if (!InitEnv(env)) {
        std::cerr << "[FAIL] InitEnv failed" << std::endl;
        return 1;
    }

    TestReplaceText(env);
    TestInsertChar(env);
    TestInsertLine(env);
    TestDeleteChar(env);
    TestDeleteLine(env);
    TestReplaceTextLine(env);
    TestArrowKeys(env);
    TestScrollByScrollBar(env);
    TestHScrollPaint(env);
    TestArrowKeysScrollView(env);
    TestScrollViewSwitch(env);
    TestPaintAfterEdit(env);
    TestPasteThenScroll(env);
    TestReadOnly(env);
    TestLimitText(env);
    TestLimitChars(env);
    TestPasswordMode(env);
    TestNumberOnly(env);
    TestTextAlign(env);
    TestLineAndKeyOptions(env);
    TestRowSpacingAndZoom(env);
    TestUndoRedo(env);
    TestSelectionApis(env);
    TestCaretCommandKeys(env);
    TestMouseSelection(env);
    TestEditEvents(env);
    TestClipboard(env);
    TestFindRichText(env);

    std::cout << "richedit2 edit tests: " << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
