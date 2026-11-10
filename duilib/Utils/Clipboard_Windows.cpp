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
            //注意：GetClipboardData 失败时返回 NULL，不是 INVALID_HANDLE_VALUE
            if (h != nullptr) {
                wchar_t* buf = (wchar_t*)::GlobalLock(h);
                if (buf != nullptr) {
                    //直接按C字符串赋值即可(会截断到第一个'\0')，无需先按GlobalSize构造再拷贝一次
                    text = buf;
                    ::GlobalUnlock(h);
                }
            }
        }
        else if (::IsClipboardFormatAvailable(CF_TEXT)) {
            HANDLE h = ::GetClipboardData(CF_TEXT);
            //注意：GetClipboardData 失败时返回 NULL，不是 INVALID_HANDLE_VALUE
            if (h != nullptr) {
                char* buf = (char*)::GlobalLock(h);
                if (buf != nullptr) {
                    //直接按C字符串转换(会截断到第一个'\0')，避免尾部的填充0被转换进结果字符串
                    text = StringConvert::MBCSToUnicode2(buf, ::strlen(buf));
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
    if (lpStr == nullptr) {
        //GlobalLock失败时不能访问内存，需要释放后返回失败
        ::GlobalFree(hMem);
        ::CloseClipboard();
        return false;
    }
    ::memcpy(lpStr, text.c_str(), len * sizeof(wchar_t));
    lpStr[len] = wchar_t(0);
    ::GlobalUnlock(hMem);
    if (::SetClipboardData(CF_UNICODETEXT, hMem) == nullptr) {
        //设置剪贴板数据失败时，内存所有权未转移，需要释放
        ::GlobalFree(hMem);
        ::CloseClipboard();
        return false;
    }
    ::CloseClipboard();
    return true;
}

bool Clipboard::SetClipboardText(const DStringA& text)
{
    return SetClipboardText(StringConvert::UTF8ToWString(text));
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
