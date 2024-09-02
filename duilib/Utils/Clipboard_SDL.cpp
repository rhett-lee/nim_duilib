#include "Clipboard.h"
#include "duilib/Utils/StringUtil.h"

#ifdef DUILIB_BUILD_FOR_SDL
#include <SDL3/SDL.h>

namespace ui
{
bool Clipboard::GetClipboardText(DStringW& text)
{
    DStringA textA;
    bool bRet = GetClipboardText(textA);
    text = StringUtil::UTF8ToUTF16(textA);
    return bRet;
}

bool Clipboard::GetClipboardText(DStringA& text)
{
    text.clear();
    if (SDL_HasClipboardText() == SDL_TRUE) {
        char* szTemp = SDL_GetClipboardText();
        if (szTemp != nullptr) {
            text = szTemp;
            SDL_free(szTemp);
            szTemp = nullptr;
        }
    }
    return true;
}

bool Clipboard::SetClipboardText(const DStringW& text)
{
    return SetClipboardText(StringUtil::UTF16ToUTF8(text));
}

bool Clipboard::SetClipboardText(const DStringA& text)
{
    SDL_bool nRet = SDL_SetClipboardText(text.c_str());
    return nRet == SDL_TRUE;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_SDL
