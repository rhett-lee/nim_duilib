#include "Keyboard.h"
#include "duilib/Utils/StringConvert.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

namespace ui
{

bool Keyboard::IsKeyDown(VirtualKeyCode nVirtKey)
{
    bool bKeyDown = false;
    switch (nVirtKey) {
    case kVK_SHIFT:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_SHIFT) ? true : false;
        break;
    case kVK_LSHIFT:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_LSHIFT) ? true : false;
        break;
    case kVK_RSHIFT:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_RSHIFT) ? true : false;
        break;
    case kVK_CONTROL:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_CTRL) ? true : false;
        break;
    case kVK_LCONTROL:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_LCTRL) ? true : false;
        break;
    case kVK_RCONTROL:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_RCTRL) ? true : false;
        break;
    case kVK_MENU:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_ALT) ? true : false;
        break;
    case kVK_LMENU:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_LALT) ? true : false;
        break;
    case kVK_RMENU:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_RALT) ? true : false;
        break;
    case kVK_LWIN:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_LGUI) ? true : false;
        break;
    case kVK_RWIN:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_RGUI) ? true : false;
        break;
    case kVK_NUMLOCK:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_NUM) ? true : false;
        break;
    case kVK_SCROLL:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_SCROLL) ? true : false;
        break;
    case kVK_CAPITAL:
        bKeyDown = (SDL_GetModState() & SDL_KMOD_CAPS) ? true : false;
        break;
    case kVK_LBUTTON:
        {
            Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);
            bKeyDown = (mouse_state & SDL_BUTTON_LMASK) ? true : false;
        }
        break;
    case kVK_MBUTTON:
        {
            Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);
            bKeyDown = (mouse_state & SDL_BUTTON_MMASK) ? true : false;
        }
        break;
    case kVK_RBUTTON:
        {
            Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);
            bKeyDown = (mouse_state & SDL_BUTTON_RMASK) ? true : false;
        }
        break;
    case kVK_XBUTTON1:
        {
            Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);
            bKeyDown = (mouse_state & SDL_BUTTON_X1MASK) ? true : false;
        }
        break;
    case kVK_XBUTTON2:
        {
            Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);
            bKeyDown = (mouse_state & SDL_BUTTON_X2MASK) ? true : false;
        }
        break;
    default:
        {
            SDL_Keycode sdlKeycode = Keycode::GetSDLKeyCode(nVirtKey);
            if (sdlKeycode != SDLK_UNKNOWN) {
                SDL_Scancode sdlScancode = SDL_GetScancodeFromKey(sdlKeycode, nullptr);
                if (sdlScancode != SDL_SCANCODE_UNKNOWN) {
                    int numkeys = 0;
                    const bool* keyboardState = SDL_GetKeyboardState(&numkeys);
                    if ((keyboardState != nullptr) && (sdlKeycode < (SDL_Keycode)numkeys)) {
                        bKeyDown = keyboardState[sdlScancode];
                    }
                }
            }
        }
        break;
    }
    return bKeyDown;
}

bool Keyboard::IsNumLockOn()
{
    return (SDL_GetModState() & SDL_KMOD_NUM) ? true : false;
}

bool Keyboard::IsScrollLockOn()
{
    return (SDL_GetModState() & SDL_KMOD_SCROLL) ? true : false;
}

bool Keyboard::IsCapsLockOn()
{
    return (SDL_GetModState() & SDL_KMOD_CAPS) ? true : false;
}

DString Keyboard::GetKeyName(VirtualKeyCode nVirtKey, bool /*fExtended*/)
{
    DString keyName;
    SDL_Keycode sdlKeycode = Keycode::GetSDLKeyCode(nVirtKey);
    if (sdlKeycode != SDLK_UNKNOWN) {
        const char* szName = SDL_GetKeyName(sdlKeycode);
        if (szName != nullptr) {
            keyName = StringConvert::UTF8ToT(szName);
        }
    }
    return keyName;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL
