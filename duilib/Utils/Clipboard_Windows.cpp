#include "Clipboard.h"
#include "duilib/Utils/StringConvert.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

namespace ui
{
bool Clipboard::GetClipboardText(DStringW& text)
{
    text.clear();
    BOOL ret = ::OpenClipboard(nullptr);
    if (ret) {
        if (::IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            HANDLE h = ::GetClipboardData(CF_UNICODETEXT);
            if (h != INVALID_HANDLE_VALUE) {
                wchar_t* buf = (wchar_t*)::GlobalLock(h);
                if (buf != nullptr) {
                    DStringW str(buf, GlobalSize(h) / sizeof(wchar_t));
                    text = str;
                    ::GlobalUnlock(h);
                }
            }
        }
        else if (::IsClipboardFormatAvailable(CF_TEXT)) {
            HANDLE h = ::GetClipboardData(CF_TEXT);
            if (h != INVALID_HANDLE_VALUE) {
                char* buf = (char*)::GlobalLock(h);
                if (buf != nullptr) {
                    std::string str(buf, GlobalSize(h));
                    text = StringConvert::MBCSToUnicode(str);
                    ::GlobalUnlock(h);
                }
            }
        }
        ::CloseClipboard();
    }
    return ret != FALSE;
}

bool Clipboard::GetClipboardText(DStringA& text)
{
    DStringW textW;
    bool bRet = GetClipboardText(textW);
    text = StringConvert::WStringToUTF8(textW);
    return bRet;
}

bool Clipboard::SetClipboardText(const DStringW& text)
{
    if (!::OpenClipboard(nullptr)) {
        return false;
    }

    if (!::EmptyClipboard()) {
        ::CloseClipboard();
        return false;
    }

    size_t len = text.size();
    HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t));
    if (hMem == nullptr) {
        ::CloseClipboard();
        return false;
    }

    wchar_t* lpStr = (wchar_t*)::GlobalLock(hMem);
    ::memcpy(lpStr, text.c_str(), len * sizeof(wchar_t));
    lpStr[len] = wchar_t(0);
    ::GlobalUnlock(hMem);
    ::SetClipboardData(CF_UNICODETEXT, hMem);
    ::CloseClipboard();
    return true;
}

bool Clipboard::SetClipboardText(const DStringA& text)
{
    return SetClipboardText(StringConvert::UTF8ToWString(text));
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
