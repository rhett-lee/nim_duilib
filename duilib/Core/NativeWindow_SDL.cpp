#include "NativeWindow_SDL.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

#include "duilib/Utils/ApiWrapper_Windows.h"
#include <CommCtrl.h>
#include <Olectl.h>
#include <VersionHelpers.h>

namespace ui {

//窗口指针与SDL窗口ID的映射关系，用于转接消息
std::unordered_map<SDL_WindowID, NativeWindow_SDL*> NativeWindow_SDL::s_windowIDMap;

void NativeWindow_SDL::SetWindowFromID(SDL_WindowID id, NativeWindow_SDL* pNativeWindow)
{
    ASSERT(id != 0);
    if (id == 0) {
        return;
    }
    ASSERT(pNativeWindow != nullptr);
    if (pNativeWindow == nullptr) {
        return;
    }
    ASSERT(pNativeWindow->m_sdlWindow != nullptr);
    if (pNativeWindow->m_sdlWindow == nullptr) {
        return;
    }
    ASSERT(SDL_GetWindowFromID(id) == pNativeWindow->m_sdlWindow);
    if (SDL_GetWindowFromID(id) != pNativeWindow->m_sdlWindow) {
        return;
    }
    s_windowIDMap[id] = pNativeWindow;
}

void NativeWindow_SDL::ClearWindowFromID(SDL_WindowID id, NativeWindow_SDL* pNativeWindow)
{
    ASSERT(id != 0);
    NativeWindow_SDL* pOldNativeWindow = nullptr;
    auto iter = s_windowIDMap.find(id);
    if (iter != s_windowIDMap.end()) {
        pOldNativeWindow = iter->second;
        s_windowIDMap.erase(iter);
    }
    if (pNativeWindow != nullptr) {
        ASSERT(pNativeWindow == pOldNativeWindow);
    }
}

VirtualKeyCode NativeWindow_SDL::GetVirtualKeyCode(SDL_Keycode sdlKeycode)
{
    //参考：https://wiki.libsdl.org/SDL2/SDL_ScancodeAndKeycode
    VirtualKeyCode vkCode = VirtualKeyCode::kVK_None;
    switch(sdlKeycode)
    {
    case SDLK_UNKNOWN:      // 0x00000000u /* 0 */
        vkCode = VirtualKeyCode::kVK_None;
        break;
    case SDLK_RETURN:       // 0x0000000du /* '\r' */
        vkCode = VirtualKeyCode::kVK_RETURN;
        break;
    case SDLK_ESCAPE:       // 0x0000001bu /* '\x1B' */
        vkCode = VirtualKeyCode::kVK_ESCAPE;
        break;
    case SDLK_BACKSPACE:    // 0x00000008u /* '\b' */
        vkCode = VirtualKeyCode::kVK_BACK;
        break;
    case SDLK_TAB:          // 0x00000009u /* '\t' */
        vkCode = VirtualKeyCode::kVK_TAB;
        break;
    case SDLK_SPACE:        // 0x00000020u /* ' ' */
        vkCode = VirtualKeyCode::kVK_SPACE;
        break;
    case SDLK_EXCLAIM:      // 0x00000021u /* '!' */
        //转回
        vkCode = VirtualKeyCode::kVK_1;
        break;
    case SDLK_DBLAPOSTROPHE:// 0x00000022u /* '"' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_7;
        break;
    case SDLK_HASH:         // 0x00000023u /* '#' */
        //转回
        vkCode = VirtualKeyCode::kVK_3;
        break;
    case SDLK_DOLLAR:       // 0x00000024u /* '$' */
        //转回
        vkCode = VirtualKeyCode::kVK_4;
        break;
    case SDLK_PERCENT:      // 0x00000025u /* '%' */
        //转回
        vkCode = VirtualKeyCode::kVK_5;
        break;
    case SDLK_AMPERSAND:    // 0x00000026u /* '&' */
        //转回
        vkCode = VirtualKeyCode::kVK_7;
        break;
    case SDLK_APOSTROPHE:   // 0x00000027u /* '\'' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_7;
        break;
    case SDLK_LEFTPAREN:    // 0x00000028u /* '(' */
        //转回
        vkCode = VirtualKeyCode::kVK_9;
        break;
    case SDLK_RIGHTPAREN:   // 0x00000029u /* ')' */
        //转回
        vkCode = VirtualKeyCode::kVK_0;
        break;
    case SDLK_ASTERISK:     // 0x0000002au /* '*' */
        //转回
        vkCode = VirtualKeyCode::kVK_8;
        break;
    case SDLK_PLUS:         // 0x0000002bu /* '+' */
        vkCode = VirtualKeyCode::kVK_OEM_PLUS;
        break;
    case SDLK_COMMA:        // 0x0000002cu /* ',' */
        vkCode = VirtualKeyCode::kVK_OEM_COMMA;
        break;
    case SDLK_MINUS:        // 0x0000002du /* '-' */
        vkCode = VirtualKeyCode::kVK_OEM_MINUS;
        break;
    case SDLK_PERIOD:       // 0x0000002eu /* '.' */
        vkCode = VirtualKeyCode::kVK_OEM_PERIOD;
        break;
    case SDLK_SLASH:        // 0x0000002fu /* '/' */
        vkCode = VirtualKeyCode::kVK_OEM_2;
        break;
    case SDLK_0:            // 0x00000030u /* '0' */
        vkCode = VirtualKeyCode::kVK_0;
        break;
    case SDLK_1:            // 0x00000031u /* '1' */
        vkCode = VirtualKeyCode::kVK_1;
        break;
    case SDLK_2:            // 0x00000032u /* '2' */
        vkCode = VirtualKeyCode::kVK_2;
        break;
    case SDLK_3:            // 0x00000033u /* '3' */
        vkCode = VirtualKeyCode::kVK_3;
        break;
    case SDLK_4:            // 0x00000034u /* '4' */
        vkCode = VirtualKeyCode::kVK_4;
        break;
    case SDLK_5:            // 0x00000035u /* '5' */
        vkCode = VirtualKeyCode::kVK_5;
        break;
    case SDLK_6:            // 0x00000036u /* '6' */
        vkCode = VirtualKeyCode::kVK_6;
        break;
    case SDLK_7:            // 0x00000037u /* '7' */
        vkCode = VirtualKeyCode::kVK_7;
        break;
    case SDLK_8:            // 0x00000038u /* '8' */
        vkCode = VirtualKeyCode::kVK_8;
        break;
    case SDLK_9:            // 0x00000039u /* '9' */
        vkCode = VirtualKeyCode::kVK_9;
        break;
    case SDLK_COLON:        // 0x0000003au /* ':' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_1;
        break;
    case SDLK_SEMICOLON:    // 0x0000003bu /* ';' */
        vkCode = VirtualKeyCode::kVK_OEM_1;
        break;
    case SDLK_LESS:         // 0x0000003cu /* '<' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_COMMA;
        break;
    case SDLK_EQUALS:       // 0x0000003du /* '=' */
        vkCode = VirtualKeyCode::kVK_OEM_PLUS;
        break;
    case SDLK_GREATER:      // 0x0000003eu /* '>' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_PERIOD;
        break;
    case SDLK_QUESTION:     // 0x0000003fu /* '?' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_2;
        break;
    case SDLK_AT:           // 0x00000040u /* '@' */
        //转回
        vkCode = VirtualKeyCode::kVK_2;
        break;
    case SDLK_LEFTBRACKET:  // 0x0000005bu /* '[' */
        vkCode = VirtualKeyCode::kVK_OEM_4;
        break;
    case SDLK_BACKSLASH:    // 0x0000005cu /* '\\' */
        vkCode = VirtualKeyCode::kVK_OEM_5;
        break;
    case SDLK_RIGHTBRACKET: // 0x0000005du /* ']' */
        vkCode = VirtualKeyCode::kVK_OEM_6;
        break;
    case SDLK_CARET:        // 0x0000005eu /* '^' */
        //转回
        vkCode = VirtualKeyCode::kVK_6;
        break;
    case SDLK_UNDERSCORE:   // 0x0000005fu /* '_' */
        //转回S
        vkCode = VirtualKeyCode::kVK_OEM_MINUS;
        break;
    case SDLK_GRAVE:        // 0x00000060u /* '`' */
        vkCode = VirtualKeyCode::kVK_OEM_3;
        break;
    case SDLK_A:            // 0x00000061u /* 'a' */
        vkCode = VirtualKeyCode::kVK_A;
        break;
    case SDLK_B:            // 0x00000062u /* 'b' */
        vkCode = VirtualKeyCode::kVK_B;
        break;
    case SDLK_C:            // 0x00000063u /* 'c' */
        vkCode = VirtualKeyCode::kVK_C;
        break;
    case SDLK_D:            // 0x00000064u /* 'd' */
        vkCode = VirtualKeyCode::kVK_D;
        break;
    case SDLK_E:            // 0x00000065u /* 'e' */
        vkCode = VirtualKeyCode::kVK_E;
        break;
    case SDLK_F:            // 0x00000066u /* 'f' */
        vkCode = VirtualKeyCode::kVK_F;
        break;
    case SDLK_G:            // 0x00000067u /* 'g' */
        vkCode = VirtualKeyCode::kVK_G;
        break;
    case SDLK_H:            // 0x00000068u /* 'h' */
        vkCode = VirtualKeyCode::kVK_H;
        break;
    case SDLK_I:            // 0x00000069u /* 'i' */
        vkCode = VirtualKeyCode::kVK_I;
        break;
    case SDLK_J:            // 0x0000006au /* 'j' */
        vkCode = VirtualKeyCode::kVK_J;
        break;
    case SDLK_K:            // 0x0000006bu /* 'k' */
        vkCode = VirtualKeyCode::kVK_K;
        break;
    case SDLK_L:            // 0x0000006cu /* 'l' */
        vkCode = VirtualKeyCode::kVK_L;
        break;
    case SDLK_M:            // 0x0000006du /* 'm' */
        vkCode = VirtualKeyCode::kVK_M;
        break;
    case SDLK_N:            // 0x0000006eu /* 'n' */
        vkCode = VirtualKeyCode::kVK_N;
        break;
    case SDLK_O:            // 0x0000006fu /* 'o' */
        vkCode = VirtualKeyCode::kVK_O;
        break;
    case SDLK_P:            // 0x00000070u /* 'p' */
        vkCode = VirtualKeyCode::kVK_P;
        break;
    case SDLK_Q:            // 0x00000071u /* 'q' */
        vkCode = VirtualKeyCode::kVK_Q;
        break;
    case SDLK_R:            // 0x00000072u /* 'r' */
        vkCode = VirtualKeyCode::kVK_R;
        break;
    case SDLK_S:            // 0x00000073u /* 's' */
        vkCode = VirtualKeyCode::kVK_S;
        break;
    case SDLK_T:            // 0x00000074u /* 't' */
        vkCode = VirtualKeyCode::kVK_T;
        break;
    case SDLK_U:            // 0x00000075u /* 'u' */
        vkCode = VirtualKeyCode::kVK_U;
        break;
    case SDLK_V:            // 0x00000076u /* 'v' */
        vkCode = VirtualKeyCode::kVK_V;
        break;
    case SDLK_W:            // 0x00000077u /* 'w' */
        vkCode = VirtualKeyCode::kVK_W;
        break;
    case SDLK_X:            // 0x00000078u /* 'x' */
        vkCode = VirtualKeyCode::kVK_X;
        break;
    case SDLK_Y:            // 0x00000079u /* 'y' */
        vkCode = VirtualKeyCode::kVK_Y;
        break;
    case SDLK_Z:            // 0x0000007au /* 'z' */
        vkCode = VirtualKeyCode::kVK_Z;
        break;
    case SDLK_LEFTBRACE:    // 0x0000007bu /* '{' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_4;
        break;
    case SDLK_PIPE:         // 0x0000007cu /* '|' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_5;
        break;
    case SDLK_RIGHTBRACE:   // 0x0000007du /* '}' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_6;
        break;
    case SDLK_TILDE:        // 0x0000007eu /* '~' */
        //转回
        vkCode = VirtualKeyCode::kVK_OEM_3;
        break;
    case SDLK_DELETE:       // 0x0000007fu /* '\x7F' */
        vkCode = VirtualKeyCode::kVK_DELETE;
        break;
    case SDLK_PLUSMINUS:    // 0x000000b1u /* '±' */
        //找不到
        ASSERT(0);
        break;
    case SDLK_CAPSLOCK:     // 0x40000039u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK) */
        vkCode = VirtualKeyCode::kVK_CAPITAL;
        break;
    case SDLK_F1:           // 0x4000003au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1) */
        vkCode = VirtualKeyCode::kVK_F1;
        break;
    case SDLK_F2:           // 0x4000003bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2) */
        vkCode = VirtualKeyCode::kVK_F2;
        break;
    case SDLK_F3:           // 0x4000003cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3) */
        vkCode = VirtualKeyCode::kVK_F3;
        break;
    case SDLK_F4:           // 0x4000003du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4) */
        vkCode = VirtualKeyCode::kVK_F4;
        break;
    case SDLK_F5:           // 0x4000003eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5) */
        vkCode = VirtualKeyCode::kVK_F5;
        break;
    case SDLK_F6:           // 0x4000003fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6) */
        vkCode = VirtualKeyCode::kVK_F6;
        break;
    case SDLK_F7:           // 0x40000040u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7) */
        vkCode = VirtualKeyCode::kVK_F7;
        break;
    case SDLK_F8:           // 0x40000041u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8) */
        vkCode = VirtualKeyCode::kVK_F8;
        break;
    case SDLK_F9:           // 0x40000042u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9) */
        vkCode = VirtualKeyCode::kVK_F9;
        break;
    case SDLK_F10:          // 0x40000043u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10) */
        vkCode = VirtualKeyCode::kVK_F10;
        break;
    case SDLK_F11:          // 0x40000044u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11) */
        vkCode = VirtualKeyCode::kVK_F11;
        break;
    case SDLK_F12:          // 0x40000045u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12) */
        vkCode = VirtualKeyCode::kVK_F12;
        break;
    case SDLK_PRINTSCREEN:  // 0x40000046u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN) */
        vkCode = VirtualKeyCode::kVK_SNAPSHOT;
        break;
    case SDLK_SCROLLLOCK:   // 0x40000047u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK) */
        vkCode = VirtualKeyCode::kVK_SCROLL;
        break;
    case SDLK_PAUSE:        // 0x40000048u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE) */
        vkCode = VirtualKeyCode::kVK_PAUSE;
        break;
    case SDLK_INSERT:       // 0x40000049u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT) */
        vkCode = VirtualKeyCode::kVK_INSERT;
        break;
    case SDLK_HOME:         // 0x4000004au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME) */
        vkCode = VirtualKeyCode::kVK_HOME;
        break;
    case SDLK_PAGEUP:       // 0x4000004bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP) */
        vkCode = VirtualKeyCode::kVK_PRIOR;
        break;
    case SDLK_END:          // 0x4000004du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END) */
        vkCode = VirtualKeyCode::kVK_END;
        break;
    case SDLK_PAGEDOWN:     // 0x4000004eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN) */
        vkCode = VirtualKeyCode::kVK_NEXT;
        break;
    case SDLK_RIGHT:        // 0x4000004fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT) */
        vkCode = VirtualKeyCode::kVK_RIGHT;
        break;
    case SDLK_LEFT:         // 0x40000050u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT) */
        vkCode = VirtualKeyCode::kVK_LEFT;
        break;
    case SDLK_DOWN:         // 0x40000051u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN) */
        vkCode = VirtualKeyCode::kVK_DOWN;
        break;
    case SDLK_UP:           // 0x40000052u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP) */
        vkCode = VirtualKeyCode::kVK_UP;
        break;
    case SDLK_NUMLOCKCLEAR: // 0x40000053u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR) */
        //转回
        vkCode = VirtualKeyCode::kVK_NUMLOCK;
        break;
    case SDLK_KP_DIVIDE:    // 0x40000054u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE) */
        vkCode = VirtualKeyCode::kVK_DIVIDE;
        break;
    case SDLK_KP_MULTIPLY:  // 0x40000055u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY) */
        vkCode = VirtualKeyCode::kVK_MULTIPLY;
        break;
    case SDLK_KP_MINUS:     // 0x40000056u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS) */
        vkCode = VirtualKeyCode::kVK_SUBTRACT;
        break;
    case SDLK_KP_PLUS:      // 0x40000057u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS) */
        vkCode = VirtualKeyCode::kVK_ADD;
        break;
    case SDLK_KP_ENTER:     // 0x40000058u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER) */
        vkCode = VirtualKeyCode::kVK_SEPARATOR;
        break;
    case SDLK_KP_1:         // 0x40000059u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1) */
        vkCode = VirtualKeyCode::kVK_NUMPAD1;
        break;
    case SDLK_KP_2:         // 0x4000005au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2) */
        vkCode = VirtualKeyCode::kVK_NUMPAD2;
        break;
    case SDLK_KP_3:         // 0x4000005bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3) */
        vkCode = VirtualKeyCode::kVK_NUMPAD3;
        break;
    case SDLK_KP_4:         // 0x4000005cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4) */
        vkCode = VirtualKeyCode::kVK_NUMPAD4;
        break;
    case SDLK_KP_5:         // 0x4000005du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5) */
        vkCode = VirtualKeyCode::kVK_NUMPAD5;
        break;
    case SDLK_KP_6:         // 0x4000005eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6) */
        vkCode = VirtualKeyCode::kVK_NUMPAD6;
        break;
    case SDLK_KP_7:         // 0x4000005fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7) */
        vkCode = VirtualKeyCode::kVK_NUMPAD7;
        break;
    case SDLK_KP_8:         // 0x40000060u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8) */
        vkCode = VirtualKeyCode::kVK_NUMPAD8;
        break;
    case SDLK_KP_9:         // 0x40000061u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9) */
        vkCode = VirtualKeyCode::kVK_NUMPAD9;
        break;
    case SDLK_KP_0:         // 0x40000062u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0) */
        vkCode = VirtualKeyCode::kVK_NUMPAD0;
        break;
    case SDLK_KP_PERIOD:    // 0x40000063u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD) */
        vkCode = VirtualKeyCode::kVK_DECIMAL;
        break;
    case SDLK_APPLICATION:  // 0x40000065u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION) */
        ASSERT(0);
        break;
    case SDLK_POWER:        // 0x40000066u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER) */
        ASSERT(0);
        break;
    case SDLK_KP_EQUALS:    // 0x40000067u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS) */
        ASSERT(0);
        break;
    case SDLK_F13:          // 0x40000068u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13) */
        vkCode = VirtualKeyCode::kVK_F13;
        break;
    case SDLK_F14:          // 0x40000069u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14) */
        vkCode = VirtualKeyCode::kVK_F14;
        break;
    case SDLK_F15:          // 0x4000006au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15) */
        vkCode = VirtualKeyCode::kVK_F15;
        break;
    case SDLK_F16:          // 0x4000006bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16) */
        vkCode = VirtualKeyCode::kVK_F16;
        break;
    case SDLK_F17:          // 0x4000006cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17) */
        vkCode = VirtualKeyCode::kVK_F17;
        break;
    case SDLK_F18:          // 0x4000006du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18) */
        vkCode = VirtualKeyCode::kVK_F18;
        break;
    case SDLK_F19:          // 0x4000006eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19) */
        vkCode = VirtualKeyCode::kVK_F19;
        break;
    case SDLK_F20:          // 0x4000006fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20) */
        vkCode = VirtualKeyCode::kVK_F20;
        break;
    case SDLK_F21:          // 0x40000070u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21) */
        vkCode = VirtualKeyCode::kVK_F21;
        break;
    case SDLK_F22:          // 0x40000071u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22) */
        vkCode = VirtualKeyCode::kVK_F22;
        break;
    case SDLK_F23:          // 0x40000072u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23) */
        vkCode = VirtualKeyCode::kVK_F23;
        break;
    case SDLK_F24:          // 0x40000073u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24) */
        vkCode = VirtualKeyCode::kVK_F24;
        break;
    case SDLK_EXECUTE:      // 0x40000074u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE) */
        ASSERT(0);
        break;
    case SDLK_HELP:         // 0x40000075u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP) */
        vkCode = VirtualKeyCode::kVK_HELP;
        break;
    case SDLK_MENU:         // 0x40000076u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU) */
        vkCode = VirtualKeyCode::kVK_MENU;
        break;
    case SDLK_SELECT:       // 0x40000077u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT) */
    case SDLK_STOP:         // 0x40000078u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP) */
    case SDLK_AGAIN:        // 0x40000079u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN) */
    case SDLK_UNDO:         // 0x4000007au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO) */
    case SDLK_CUT:          // 0x4000007bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT) */
    case SDLK_COPY:         // 0x4000007cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY) */
    case SDLK_PASTE:        // 0x4000007du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE) */
    case SDLK_FIND:         // 0x4000007eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND) */
        ASSERT(0);
        break;
    case SDLK_MUTE:         // 0x4000007fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE) */
        vkCode = VirtualKeyCode::kVK_VOLUME_MUTE;
        break;
    case SDLK_VOLUMEUP:     // 0x40000080u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP) */
        vkCode = VirtualKeyCode::kVK_VOLUME_UP;
        break;
    case SDLK_VOLUMEDOWN:   // 0x40000081u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN) */
        vkCode = VirtualKeyCode::kVK_VOLUME_DOWN;
        break;
    case SDLK_KP_COMMA:     // 0x40000085u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA) */
    case SDLK_KP_EQUALSAS400:// 0x40000086u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400) */
    case SDLK_ALTERASE:     // 0x40000099u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE) */
    case SDLK_SYSREQ:       // 0x4000009au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ) */
    case SDLK_CANCEL:       // 0x4000009bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL) */
    case SDLK_CLEAR:        // 0x4000009cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR) */
    case SDLK_PRIOR:        // 0x4000009du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR) */
    case SDLK_RETURN2:      // 0x4000009eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2) */
    case SDLK_SEPARATOR:    // 0x4000009fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR) */
    case SDLK_OUT:          // 0x400000a0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT) */
    case SDLK_OPER:         // 0x400000a1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER) */
    case SDLK_CLEARAGAIN:   // 0x400000a2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN) */
    case SDLK_CRSEL:        // 0x400000a3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL) */
    case SDLK_EXSEL:        // 0x400000a4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL) */
    case SDLK_KP_00:        // 0x400000b0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00) */
    case SDLK_KP_000:       // 0x400000b1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000) */
    case SDLK_THOUSANDSSEPARATOR:   // 0x400000b2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR) */
    case SDLK_DECIMALSEPARATOR:     // 0x400000b3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR) */
    case SDLK_CURRENCYUNIT:         // 0x400000b4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT) */
    case SDLK_CURRENCYSUBUNIT:      // 0x400000b5u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT) */
    case SDLK_KP_LEFTPAREN:         // 0x400000b6u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN) */
    case SDLK_KP_RIGHTPAREN:        // 0x400000b7u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN) */
    case SDLK_KP_LEFTBRACE:         // 0x400000b8u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE) */
    case SDLK_KP_RIGHTBRACE:        // 0x400000b9u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE) */
    case SDLK_KP_TAB:               // 0x400000bau /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB) */
    case SDLK_KP_BACKSPACE: // 0x400000bbu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE) */
    case SDLK_KP_A:         // 0x400000bcu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A) */
    case SDLK_KP_B:         // 0x400000bdu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B) */
    case SDLK_KP_C:         // 0x400000beu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C) */
    case SDLK_KP_D:         // 0x400000bfu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D) */
    case SDLK_KP_E:         // 0x400000c0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E) */
    case SDLK_KP_F:         // 0x400000c1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F) */
    case SDLK_KP_XOR:       // 0x400000c2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR) */
    case SDLK_KP_POWER:     // 0x400000c3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER) */
    case SDLK_KP_PERCENT:   // 0x400000c4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT) */
    case SDLK_KP_LESS:      // 0x400000c5u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS) */
    case SDLK_KP_GREATER:   // 0x400000c6u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER) */
    case SDLK_KP_AMPERSAND: // 0x400000c7u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND) */
    case SDLK_KP_DBLAMPERSAND:  // 0x400000c8u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND) */
    case SDLK_KP_VERTICALBAR:   // 0x400000c9u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR) */
    case SDLK_KP_DBLVERTICALBAR: // 0x400000cau /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR) */
    case SDLK_KP_COLON:         // 0x400000cbu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON) */
    case SDLK_KP_HASH:          // 0x400000ccu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH) */
    case SDLK_KP_SPACE:         // 0x400000cdu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE) */
    case SDLK_KP_AT:            // 0x400000ceu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT) */
    case SDLK_KP_EXCLAM:        // 0x400000cfu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM) */
    case SDLK_KP_MEMSTORE:      // 0x400000d0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE) */
    case SDLK_KP_MEMRECALL:     // 0x400000d1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL) */
    case SDLK_KP_MEMCLEAR:      // 0x400000d2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR) */
    case SDLK_KP_MEMADD:        // 0x400000d3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD) */
    case SDLK_KP_MEMSUBTRACT:   // 0x400000d4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT) */
    case SDLK_KP_MEMMULTIPLY:   // 0x400000d5u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY) */
    case SDLK_KP_MEMDIVIDE:     // 0x400000d6u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE) */
    case SDLK_KP_PLUSMINUS:     // 0x400000d7u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS) */
    case SDLK_KP_CLEAR:         // 0x400000d8u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR) */
    case SDLK_KP_CLEARENTRY:    // 0x400000d9u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY) */
    case SDLK_KP_BINARY:        // 0x400000dau /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY) */
    case SDLK_KP_OCTAL:         // 0x400000dbu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL) */
    case SDLK_KP_DECIMAL:       // 0x400000dcu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL) */
    case SDLK_KP_HEXADECIMAL:   // 0x400000ddu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL) */
        ASSERT(0);
        break;
    case SDLK_LCTRL:            // 0x400000e0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL) */
        vkCode = VirtualKeyCode::kVK_LCONTROL;
        break;
    case SDLK_LSHIFT:           // 0x400000e1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT) */
        vkCode = VirtualKeyCode::kVK_LSHIFT;
        break;
    case SDLK_LALT:             // 0x400000e2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT) */
        vkCode = VirtualKeyCode::kVK_LMENU;
        break;
    case SDLK_LGUI:             // 0x400000e3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI) */
        vkCode = VirtualKeyCode::kVK_LWIN;
        break;
    case SDLK_RCTRL:            // 0x400000e4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL) */
        vkCode = VirtualKeyCode::kVK_RCONTROL;
        break;
    case SDLK_RSHIFT:           // 0x400000e5u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT) */
        vkCode = VirtualKeyCode::kVK_RSHIFT;
        break;
    case SDLK_RALT:             // 0x400000e6u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT) */
        vkCode = VirtualKeyCode::kVK_RMENU;
        break;
    case SDLK_RGUI:             // 0x400000e7u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI) */
        vkCode = VirtualKeyCode::kVK_RWIN;
        break;
    case SDLK_MODE:             // 0x40000101u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE) */
    case SDLK_SLEEP:            // 0x40000102u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP) */
    case SDLK_WAKE:             // 0x40000103u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WAKE) */
    case SDLK_CHANNEL_INCREMENT:// 0x40000104u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_INCREMENT) */
    case SDLK_CHANNEL_DECREMENT:// 0x40000105u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_DECREMENT) */
        ASSERT(0);
        break;
    case SDLK_MEDIA_PLAY:       // 0x40000106u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY) */
    case SDLK_MEDIA_PAUSE:      // 0x40000107u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PAUSE) */
    case SDLK_MEDIA_RECORD:     // 0x40000108u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_RECORD) */
    case SDLK_MEDIA_FAST_FORWARD://0x40000109u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_FAST_FORWARD) */
    case SDLK_MEDIA_REWIND:     // 0x4000010au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_REWIND) */
    case SDLK_MEDIA_NEXT_TRACK: // 0x4000010bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_NEXT_TRACK) */
    case SDLK_MEDIA_PREVIOUS_TRACK://0x4000010cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PREVIOUS_TRACK) */
    case SDLK_MEDIA_STOP:       // 0x4000010du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_STOP) */
    case SDLK_MEDIA_EJECT:      // 0x4000010eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_EJECT) */
    case SDLK_MEDIA_PLAY_PAUSE: // 0x4000010fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY_PAUSE) */
    case SDLK_MEDIA_SELECT:     // 0x40000110u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_SELECT) */
    case SDLK_AC_NEW:           // 0x40000111u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_NEW) */
    case SDLK_AC_OPEN:          // 0x40000112u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_OPEN) */
    case SDLK_AC_CLOSE:         // 0x40000113u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_CLOSE) */
    case SDLK_AC_EXIT:          // 0x40000114u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_EXIT) */
    case SDLK_AC_SAVE:          // 0x40000115u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SAVE) */
    case SDLK_AC_PRINT:         // 0x40000116u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PRINT) */
    case SDLK_AC_PROPERTIES:    // 0x40000117u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PROPERTIES) */
    case SDLK_AC_SEARCH:        // 0x40000118u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH) */
    case SDLK_AC_HOME:          // 0x40000119u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME) */
    case SDLK_AC_BACK:          // 0x4000011au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK) */
    case SDLK_AC_FORWARD:       // 0x4000011bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD) */
    case SDLK_AC_STOP:          // 0x4000011cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP) */
    case SDLK_AC_REFRESH:       // 0x4000011du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH) */
    case SDLK_AC_BOOKMARKS:     // 0x4000011eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS) */
    case SDLK_SOFTLEFT:         // 0x4000011fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTLEFT) */
    case SDLK_SOFTRIGHT:        // 0x40000120u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTRIGHT) */
    case SDLK_CALL:             //  0x40000121u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALL) */
    case SDLK_ENDCALL:          // 0x40000122u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ENDCALL) */
        ASSERT(0);
        break;
    default:
        break;
    }

    //保持与Windows一致(Shift、Control、Alt三个键，不区分左右)
    if ((vkCode == VirtualKeyCode::kVK_LCONTROL) || (vkCode == VirtualKeyCode::kVK_RCONTROL)) {
        vkCode = VirtualKeyCode::kVK_CONTROL;
    }
    else if ((vkCode == VirtualKeyCode::kVK_LSHIFT) || (vkCode == VirtualKeyCode::kVK_RSHIFT)) {
        vkCode = VirtualKeyCode::kVK_SHIFT;
    }
    else if ((vkCode == VirtualKeyCode::kVK_LMENU) || (vkCode == VirtualKeyCode::kVK_RMENU)) {
        vkCode = VirtualKeyCode::kVK_MENU;
    }
    return vkCode;
}

uint32_t NativeWindow_SDL::GetModifiers(SDL_Keymod keymod)
{
    uint32_t modifierKey = 0;
    if (keymod & SDL_KMOD_CTRL) {
        modifierKey |= ModifierKey::kControl;
    }
    if (keymod & SDL_KMOD_SHIFT) {
        modifierKey |= ModifierKey::kShift;
    }
    if (keymod & SDL_KMOD_ALT) {
        modifierKey |= ModifierKey::kAlt;
    }
    if (keymod & SDL_KMOD_GUI) {
        modifierKey |= ModifierKey::kWin;
    }
    return modifierKey;
}

NativeWindow_SDL* NativeWindow_SDL::GetWindowFromID(SDL_WindowID id)
{
    ASSERT(id != 0);
    auto iter = s_windowIDMap.find(id);
    if (iter != s_windowIDMap.end()) {
        return iter->second;
    }
    return nullptr;
}

SDL_WindowID NativeWindow_SDL::GetWindowIdFromEvent(const SDL_Event& sdlEvent)
{
    if ((sdlEvent.type >= SDL_EVENT_WINDOW_FIRST) && (sdlEvent.type <= SDL_EVENT_WINDOW_LAST)) {
        //窗口事件
        return sdlEvent.window.windowID;
    }

    SDL_WindowID windowID = 0;
    switch (sdlEvent.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        //键盘事件
        windowID = sdlEvent.key.windowID;
        break;
    case SDL_EVENT_TEXT_EDITING:
        //键盘事件
        windowID = sdlEvent.edit.windowID;
        break;
    case SDL_EVENT_TEXT_INPUT:
        //键盘事件
        windowID = sdlEvent.text.windowID;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        //鼠标事件
        windowID = sdlEvent.motion.windowID;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        //鼠标事件
        windowID = sdlEvent.button.windowID;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        //鼠标事件
        windowID = sdlEvent.wheel.windowID;
        break;
    default:
        if ((sdlEvent.type > SDL_EVENT_USER) && (sdlEvent.type < SDL_EVENT_LAST)) {
            //用户自定义消息
            windowID = sdlEvent.user.windowID;
        }
        break;
    }

    return windowID;
}

/** 绘制的辅助类
*/
class NativeWindowRenderPaint :
    public IRenderPaint
{
public:
    NativeWindow_SDL* m_pNativeWindow = nullptr;
    INativeWindow* m_pOwner = nullptr;
    NativeMsg m_nativeMsg;
    bool m_bHandled = false;

public:
    /** 通过回调接口，完成绘制
    * @param [in] rcPaint 需要绘制的区域（客户区坐标）
    */
    virtual bool DoPaint(const UiRect& rcPaint) override
    {
        if (m_pOwner != nullptr) {
            m_pOwner->OnNativePaintMsg(rcPaint, m_nativeMsg, m_bHandled);
            return true;
        }
        return false;
    }

    /** 回调接口，获取当前窗口的透明度值
    */
    virtual uint8_t GetLayeredWindowAlpha() override
    {
        return m_pNativeWindow->GetLayeredWindowAlpha();
    }
};

bool NativeWindow_SDL::OnSDLWindowEvent(const SDL_Event& sdlEvent)
{
    INativeWindow* pOwner = m_pOwner;
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return false;
    }
    //接口的生命周期标志
    std::weak_ptr<WeakFlag> ownerFlag = pOwner->GetWeakFlag();

    //消息首先转给过滤器(全部消息)
    bool bHandled = false;
    LRESULT lResult = pOwner->OnNativeWindowMessage(sdlEvent.type, (WPARAM)&sdlEvent, 0, bHandled);
    if (bHandled || ownerFlag.expired()) {
        return true;
    }

    if (sdlEvent.type != SDL_EVENT_WINDOW_DESTROYED) {
        if (m_sdlWindow == nullptr) {
            return false;
        }
    }

    //派发消息到处理函数
    switch (sdlEvent.type) {
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
            UiSize newWindowSize;
            SDL_GetWindowSize(m_sdlWindow, &newWindowSize.cx, &newWindowSize.cy);
            ASSERT(sdlEvent.window.data1 == newWindowSize.cx);
            ASSERT(sdlEvent.window.data2 == newWindowSize.cy);
        }
        break;
    case SDL_EVENT_WINDOW_RESIZED:
        {
            //窗口大小改变
            WindowSizeType sizeType = WindowSizeType::kSIZE_RESTORED;
            SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
            if (nFlags & SDL_WINDOW_FULLSCREEN) {
                sizeType = WindowSizeType::kSIZE_MAXIMIZED;
            }
            else if (nFlags & SDL_WINDOW_MAXIMIZED) {
                sizeType = WindowSizeType::kSIZE_MAXIMIZED;
            }
            else if (nFlags & SDL_WINDOW_MINIMIZED) {
                sizeType = WindowSizeType::kSIZE_MINIMIZED;
            }
            UiSize newWindowSize;
            newWindowSize.cx = sdlEvent.window.data1;
            newWindowSize.cy = sdlEvent.window.data2;
            lResult = pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_RESIZED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_MINIMIZED:
        {
            //窗口最小化
            WindowSizeType sizeType = WindowSizeType::kSIZE_MINIMIZED;
            UiSize newWindowSize;
            SDL_GetWindowSize(m_sdlWindow, &newWindowSize.cx, &newWindowSize.cy);            
            lResult = pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_MINIMIZED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_MAXIMIZED:
        {
            //窗口最大化
            WindowSizeType sizeType = WindowSizeType::kSIZE_MAXIMIZED;
            UiSize newWindowSize;
            SDL_GetWindowSize(m_sdlWindow, &newWindowSize.cx, &newWindowSize.cy);
            lResult = pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_MAXIMIZED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_RESTORED:
        {
            //窗口还原
            WindowSizeType sizeType = WindowSizeType::kSIZE_RESTORED;
            UiSize newWindowSize;
            SDL_GetWindowSize(m_sdlWindow, &newWindowSize.cx, &newWindowSize.cy);
            lResult = pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_RESTORED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_MOVED:
        {
            //窗口移动
            UiPoint ptTopLeft;
            ptTopLeft.x = sdlEvent.window.data1;
            ptTopLeft.y = sdlEvent.window.data2;
            lResult = pOwner->OnNativeMoveMsg(ptTopLeft, NativeMsg(SDL_EVENT_WINDOW_MOVED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
        {
            //窗口绘制
            bool bPaint = pOwner->OnNativePreparePaint();
            if (bPaint && !ownerFlag.expired()) {
                IRender* pRender = pOwner->OnNativeGetRender();
                ASSERT(pRender != nullptr);
                if (pRender != nullptr) {
                    NativeWindowRenderPaint renderPaint;
                    renderPaint.m_pNativeWindow = this;
                    renderPaint.m_pOwner = pOwner;
                    renderPaint.m_nativeMsg = NativeMsg(SDL_EVENT_WINDOW_EXPOSED, 0, 0);
                    renderPaint.m_bHandled = bHandled;
                    bPaint = pRender->PaintAndSwapBuffers(&renderPaint);
                    bHandled = renderPaint.m_bHandled;
                }
            }
        }
        break;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
        //不需要处理，Windows没有这个消息
        break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        {
            //鼠标离开窗口
            lResult = pOwner->OnNativeMouseLeaveMsg(NativeMsg(SDL_EVENT_WINDOW_MOUSE_LEAVE, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        {
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.motion.x;
            pt.y = (int32_t)sdlEvent.motion.y;
            uint32_t modifierKey = 0;
            //GetModifiers(); SDL 没有开放获取按键的数据
            lResult = pOwner->OnNativeMouseMoveMsg(pt, modifierKey, false, NativeMsg(SDL_EVENT_MOUSE_MOTION, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        {
#ifdef DUILIB_BUILD_FOR_WIN
            #define WHEEL_DELTA     120
#else
#endif
            int32_t wheelDelta = (int32_t)(sdlEvent.wheel.y * WHEEL_DELTA);
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.wheel.mouse_x;
            pt.y = (int32_t)sdlEvent.wheel.mouse_y;
            uint32_t modifierKey = 0;
            //GetModifiers(uMsg, wParam, lParam, modifierKey);
            lResult = pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_WHEEL, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.button.x;
            pt.y = (int32_t)sdlEvent.button.y;
            uint32_t modifierKey = 0;
            //GetModifiers(uMsg, wParam, lParam, modifierKey);

            if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
                //鼠标左键
                lResult = pOwner->OnNativeMouseLButtonDownMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_DOWN, 0, 0), bHandled);
            }
            else if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
                //鼠标右键
                lResult = pOwner->OnNativeMouseRButtonDownMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_DOWN, 0, 0), bHandled);
            }
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.button.x;
            pt.y = (int32_t)sdlEvent.button.y;
            uint32_t modifierKey = 0;
            //GetModifiers(uMsg, wParam, lParam, modifierKey);

            bool bDoubleClick = (sdlEvent.button.clicks == 2) ? true : false;//是否为双击
            if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
                //鼠标左键
                if (bDoubleClick) {
                    lResult = pOwner->OnNativeMouseLButtonDbClickMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
                else {
                    lResult = pOwner->OnNativeMouseLButtonUpMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
            }
            else if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
                //鼠标右键
                if (bDoubleClick) {
                    lResult = pOwner->OnNativeMouseRButtonDbClickMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
                else {
                    lResult = pOwner->OnNativeMouseRButtonUpMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
            }
        }
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        {
            //获取键盘输入焦点
            INativeWindow* pLostFocusWindow = nullptr;//此参数得不到（目前无影响，代码中目前没有使用这个参数的）
            lResult = pOwner->OnNativeSetFocusMsg(pLostFocusWindow, NativeMsg(SDL_EVENT_WINDOW_FOCUS_GAINED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            INativeWindow* pSetFocusWindow = nullptr;//此参数得不到，只能间接获取（这个参数代码中使用的较多，需要获取到）
            SDL_Window* pKeyboardFocus = SDL_GetKeyboardFocus();
            if (pKeyboardFocus != nullptr) {
                SDL_WindowID id = SDL_GetWindowID(pKeyboardFocus);
                if (id != 0) {
                    NativeWindow_SDL* pNativeWindow = GetWindowFromID(id);
                    if ((pNativeWindow != nullptr) && (pNativeWindow != this)) {
                        pSetFocusWindow = pNativeWindow->m_pOwner;
                    }
                }
            }
            lResult = pOwner->OnNativeKillFocusMsg(pSetFocusWindow, NativeMsg(SDL_EVENT_WINDOW_FOCUS_LOST, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_KEY_DOWN:
        {            
            VirtualKeyCode vkCode = GetVirtualKeyCode(sdlEvent.key.key);
            uint32_t modifierKey = GetModifiers(sdlEvent.key.mod);
            lResult = pOwner->OnNativeKeyDownMsg(vkCode, modifierKey, NativeMsg(SDL_EVENT_KEY_DOWN, sdlEvent.key.key, sdlEvent.key.mod), bHandled);
        }
        break;
    case SDL_EVENT_KEY_UP:
        {
            VirtualKeyCode vkCode = GetVirtualKeyCode(sdlEvent.key.key);
            uint32_t modifierKey = GetModifiers(sdlEvent.key.mod);
            lResult = pOwner->OnNativeKeyUpMsg(vkCode, modifierKey, NativeMsg(SDL_EVENT_KEY_UP, sdlEvent.key.key, sdlEvent.key.mod), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        {
            //回调关闭事件
            bHandled = false;
            pOwner->OnNativeWindowCloseMsg(0, NativeMsg(SDL_EVENT_WINDOW_CLOSE_REQUESTED, 0, 0), bHandled);
            if (bHandled && !ownerFlag.expired()) {
                //拦截了关闭事件, 恢复关闭前的状态
                m_bCloseing = false;
                m_closeParam = kWindowCloseNormal;
            }
            else if(!ownerFlag.expired()) {
                //PreClose事件
                pOwner->OnNativePreCloseWindow();

                //关闭窗口
                if (!ownerFlag.expired()) {
                    ClearNativeWindow();
                }
            }
        }
        break;
    case SDL_EVENT_WINDOW_DESTROYED:
        {
            //窗口已经销毁
            SDL_WindowID windowID = NativeWindow_SDL::GetWindowIdFromEvent(sdlEvent);
            ClearWindowFromID(windowID, this);

            //窗口已经关闭（关闭后）
            pOwner->OnNativePostCloseWindow();
            if (!ownerFlag.expired()) {
                OnFinalMessage();
            }
        }
        break;
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        {
            //窗口显示的DPI变化
            float fNewDpiScale = SDL_GetWindowDisplayScale(m_sdlWindow);
            if (fNewDpiScale > 0.01f) {
                uint32_t nNewDPI = (uint32_t)(fNewDpiScale * 96);//新的DPI值
                if (nNewDPI != pOwner->OnNativeGetDpi().GetDPI()) {
                    //DPI发生变化
                    uint32_t nOldDpiScale = pOwner->OnNativeGetDpi().GetScale();
                    pOwner->OnNativeProcessDpiChangedMsg(nNewDPI, UiRect());
                    if (!ownerFlag.expired() && (nOldDpiScale != pOwner->OnNativeGetDpi().GetScale())) {
                        m_ptLastMousePos = pOwner->OnNativeGetDpi().GetScalePoint(m_ptLastMousePos, nOldDpiScale);
                    }
                }
            }            
        }
        break;
    case SDL_EVENT_WINDOW_SHOWN:
        {
            lResult = pOwner->OnNativeShowWindowMsg(true, NativeMsg(SDL_EVENT_WINDOW_SHOWN, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_HIDDEN:
        {
            lResult = pOwner->OnNativeShowWindowMsg(false, NativeMsg(SDL_EVENT_WINDOW_HIDDEN, 0, 0), bHandled);
        }
        break;
    default:
        break;
    }
    return true;
}

NativeWindow_SDL::NativeWindow_SDL(INativeWindow* pOwner):
    m_pOwner(pOwner),
    m_sdlWindow(nullptr),
    m_sdlRenderer(nullptr),
    m_bIsLayeredWindow(false),
    m_nLayeredWindowOpacity(255),
    m_bUseSystemCaption(false),
    m_bMouseCapture(false),
    m_bCloseing(false),
    m_closeParam(kWindowCloseNormal),
    m_bFakeModal(false),
    m_bFullScreen(false),
    m_ptLastMousePos(-1, -1)
{
    ASSERT(m_pOwner != nullptr);    
}

NativeWindow_SDL::~NativeWindow_SDL()
{
    ASSERT(m_sdlWindow == nullptr);
    ClearNativeWindow();
}

bool NativeWindow_SDL::CreateWnd(NativeWindow_SDL* pParentWindow,
                                 const WindowCreateParam& createParam,
                                 const WindowCreateAttributes& createAttributes)
{
    ASSERT(m_sdlWindow == nullptr);
    if (m_sdlWindow != nullptr) {
        return false;
    }
    ASSERT(m_sdlRenderer == nullptr);
    if (m_sdlRenderer != nullptr) {
        return false;
    }
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) == -1) {
            SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
            return false;
        }
    }

    //保存参数
    m_createParam = createParam;

    //设置默认风格
    if (m_createParam.m_dwStyle == 0) {
        m_createParam.m_dwStyle = kWS_OVERLAPPEDWINDOW;
    }

    //同步XML文件中Window的属性，在创建窗口的时候带着这些属性
    SyncCreateWindowAttributes(createAttributes);

    //创建属性
    SDL_PropertiesID props = SDL_CreateProperties();
    SetCreateWindowProperties(props, pParentWindow, createAttributes);
    m_sdlWindow = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }

    m_sdlRenderer = SDL_CreateRenderer(m_sdlWindow, nullptr);
    ASSERT(m_sdlRenderer != nullptr);
    if (m_sdlRenderer == nullptr) {
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
        return false;
    }

    //初始化
    InitNativeWindow();

    if (m_pOwner != nullptr) {
        bool bHandled = false;
        m_pOwner->OnNativeCreateWndMsg(false, NativeMsg(0, 0, 0), bHandled);
    }

    //更新最大化/最小化按钮的风格
    UpdateMinMaxBoxStyle();
    return true;
}

int32_t NativeWindow_SDL::DoModal(NativeWindow_SDL* pParentWindow,
                                  const WindowCreateParam& createParam,
                                  const WindowCreateAttributes& createAttributes,
                                  bool /*bCenterWindow*/, bool /*bCloseByEsc*/, bool /*bCloseByEnter*/)
{
    ASSERT(m_sdlWindow == nullptr);
    if (m_sdlWindow != nullptr) {
        return false;
    }
    ASSERT(m_sdlRenderer == nullptr);
    if (m_sdlRenderer != nullptr) {
        return false;
    }
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) == -1) {
            SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
            return false;
        }
    }

    //保存参数
    m_createParam = createParam;

    //设置默认风格
    if (m_createParam.m_dwStyle == 0) {
        m_createParam.m_dwStyle = kWS_POPUPWINDOW;
    }

    //同步XML文件中Window的属性，在创建窗口的时候带着这些属性
    SyncCreateWindowAttributes(createAttributes);











    


    
    return 0;
}

void NativeWindow_SDL::SyncCreateWindowAttributes(const WindowCreateAttributes& createAttributes)
{
    m_bUseSystemCaption = false;
    if (createAttributes.m_bUseSystemCaptionDefined && createAttributes.m_bUseSystemCaption) {
        //使用系统标题栏
        if (m_createParam.m_dwStyle & kWS_POPUP) {
            //弹出式窗口
            m_createParam.m_dwStyle |= (kWS_CAPTION | kWS_SYSMENU);
        }
        else {
            m_createParam.m_dwStyle |= (kWS_CAPTION | kWS_SYSMENU | kWS_MINIMIZEBOX | kWS_MAXIMIZEBOX);
        }
        m_bUseSystemCaption = true;
    }

    //初始化层窗口属性
    m_bIsLayeredWindow = false;
    if (createAttributes.m_bIsLayeredWindowDefined) {
        if (createAttributes.m_bIsLayeredWindow) {
            m_bIsLayeredWindow = true;
            m_createParam.m_dwExStyle |= kWS_EX_LAYERED;
        }
        else {
            m_createParam.m_dwExStyle &= ~kWS_EX_LAYERED;
        }
    }
    else if (m_createParam.m_dwExStyle & kWS_EX_LAYERED) {
        m_bIsLayeredWindow = true;
    }

    //如果使用系统标题栏，关闭层窗口
    if (createAttributes.m_bUseSystemCaptionDefined && createAttributes.m_bUseSystemCaption) {
        m_bIsLayeredWindow = false;
        m_createParam.m_dwExStyle &= ~kWS_EX_LAYERED;
    }

    //如果设置了不透明度，则设置为层窗口
    if (createAttributes.m_bLayeredWindowOpacityDefined && (createAttributes.m_nLayeredWindowOpacity != 255)) {
        m_createParam.m_dwExStyle |= kWS_EX_LAYERED;
        m_bIsLayeredWindow = true;
    }
    if (createAttributes.m_bLayeredWindowAlphaDefined && (createAttributes.m_nLayeredWindowAlpha != 255)) {
        m_createParam.m_dwExStyle |= kWS_EX_LAYERED;
        m_bIsLayeredWindow = true;
    }

    if (createAttributes.m_bInitSizeDefined) {
        if (createAttributes.m_szInitSize.cx > 0) {
            m_createParam.m_nWidth = createAttributes.m_szInitSize.cx;
        }
        if (createAttributes.m_szInitSize.cy > 0) {
            m_createParam.m_nHeight = createAttributes.m_szInitSize.cy;
        }
    }
}

void NativeWindow_SDL::SetCreateWindowProperties(SDL_PropertiesID props, NativeWindow_SDL* pParentWindow,
                                                 const WindowCreateAttributes& createAttributes)
{
    //设置关闭窗口的时候，不自动退出消息循环
    SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "false");

    //需要提前设置这个属性，在无边框的情况下，可以保持调整窗口大小的功能
    SDL_SetHint("SDL_BORDERLESS_RESIZABLE_STYLE", "true");

    if (!m_createParam.m_windowTitle.empty()) {
        std::string windowTitle = StringUtil::TToUTF8(m_createParam.m_windowTitle);
        SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, windowTitle.c_str());
    }

    //窗口的位置和大小
    int32_t x = kCW_USEDEFAULT;
    int32_t y = kCW_USEDEFAULT;
    int32_t cx = kCW_USEDEFAULT;
    int32_t cy = kCW_USEDEFAULT;

    if ((m_createParam.m_nX != kCW_USEDEFAULT) && (m_createParam.m_nY != kCW_USEDEFAULT)) {
        x = m_createParam.m_nX;
        y = m_createParam.m_nY;
    }
    if ((m_createParam.m_nWidth != kCW_USEDEFAULT) && (m_createParam.m_nHeight != kCW_USEDEFAULT)) {
        cx = m_createParam.m_nWidth;
        cy = m_createParam.m_nHeight;
    }
    if (x != kCW_USEDEFAULT) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
    }
    if (y != kCW_USEDEFAULT) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);
    }
    if (cx != kCW_USEDEFAULT) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, cx);
    }
    if (cy != kCW_USEDEFAULT) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, cy);
    }

    //父窗口
    if ((pParentWindow != nullptr) && (pParentWindow->m_sdlWindow != nullptr)) {
        SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_PARENT_POINTER, pParentWindow->m_sdlWindow);
    }

    //窗口属性
    SDL_WindowFlags windowFlags = SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE;

    //创建的时候，窗口保持隐藏状态，需要调用API显示窗口，避免创建窗口的时候闪烁
    windowFlags |= SDL_WINDOW_HIDDEN;

    if (!m_bUseSystemCaption && m_bIsLayeredWindow) {
        //设置透明属性，这个属性必须在创建窗口时传入，窗口创建完成后，不支持修改
        windowFlags |= SDL_WINDOW_TRANSPARENT;
    }

    //如果是弹出窗口，并且无阴影和标题栏，则默认为无边框
    const bool bPopupWindow = m_createParam.m_dwStyle & kWS_POPUP;
    bool bUseSystemCaption = createAttributes.m_bUseSystemCaptionDefined && createAttributes.m_bUseSystemCaption;
    bool bShadowAttached = createAttributes.m_bShadowAttachedDefined && createAttributes.m_bShadowAttached;
    if (!bUseSystemCaption) {
        //只要没有使用系统标题栏，就需要设置此属性，否则窗口就会带系统标题栏
        windowFlags |= SDL_WINDOW_BORDERLESS;
    }

    if (bPopupWindow && (m_createParam.m_dwExStyle & kWS_EX_TOOLWINDOW)) {
        windowFlags |= SDL_WINDOW_UTILITY;
    }
    else if (bPopupWindow && (pParentWindow != nullptr)) {
        //SDL窗口的父子窗口关系：在系统层面，并未设置父子关系，所以弹出窗口需要设置这个属性，避免任务栏出现多个窗口的现象
        //但由于SDL未使用系统的父子窗口关系，弹出窗口后，主窗口便失去焦点，任务栏上显示的是非激活状态，体验不佳
        windowFlags |= SDL_WINDOW_UTILITY;
    }
    if (bPopupWindow && (m_createParam.m_dwExStyle & kWS_EX_NOACTIVATE)) {
        windowFlags |= SDL_WINDOW_NOT_FOCUSABLE;
    }
    SDL_SetNumberProperty(props, "flags", windowFlags);
}

SDL_HitTestResult SDLCALL NativeWindow_SDL_HitTest(SDL_Window* win,
                                                   const SDL_Point* area,
                                                   void* data)
{
    NativeWindow_SDL* pWindow = (NativeWindow_SDL*)data;
    if (pWindow == nullptr) {
        return SDL_HITTEST_NORMAL;
    }
    return (SDL_HitTestResult)pWindow->SDL_HitTest(win, area, data);
}

int32_t NativeWindow_SDL::SDL_HitTest(SDL_Window* win, const SDL_Point* area, void* data)
{
    if (((NativeWindow_SDL*)data != this) ||
        (area == nullptr) || (win != GetWindowHandle()) ||
        IsUseSystemCaption() || (m_pOwner == nullptr)) {
        return SDL_HITTEST_NORMAL;
    }

    //全屏模式, 只返回客户区域属性
    if (IsWindowFullScreen()) {
        return SDL_HITTEST_NORMAL;
    }

    UiPoint pt;
    pt.x = area->x;
    pt.y = area->y;

    UiRect rcClient;
    GetClientRect(rcClient);

    //客户区域，排除掉阴影部分区域
    UiPadding rcCorner;
    m_pOwner->OnNativeGetShadowCorner(rcCorner);
    rcClient.Deflate(rcCorner);

    if (!IsWindowMaximized()) {
        //非最大化状态
        UiRect rcSizeBox = m_pOwner->OnNativeGetSizeBox();
        if (pt.y < rcClient.top + rcSizeBox.top) {
            if (pt.y >= rcClient.top) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return SDL_HITTEST_RESIZE_TOPLEFT;//在窗口边框的左上角。
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return SDL_HITTEST_RESIZE_TOPRIGHT;//在窗口边框的右上角
                }
                else {
                    return SDL_HITTEST_RESIZE_TOP;//在窗口的上水平边框中
                }
            }
            else {
                return SDL_HITTEST_NORMAL;//在工作区中
            }
        }
        else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
            if (pt.y <= rcClient.bottom) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return SDL_HITTEST_RESIZE_BOTTOMLEFT;//在窗口边框的左下角
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return SDL_HITTEST_RESIZE_BOTTOMRIGHT;//在窗口边框的右下角
                }
                else {
                    return SDL_HITTEST_RESIZE_BOTTOM;//在窗口的下水平边框中
                }
            }
            else {
                return SDL_HITTEST_NORMAL;//在工作区中
            }
        }

        if (pt.x < rcClient.left + rcSizeBox.left) {
            if (pt.x >= rcClient.left) {
                return SDL_HITTEST_RESIZE_LEFT;//在窗口的左边框
            }
            else {
                return SDL_HITTEST_NORMAL;//在工作区中
            }
        }
        if (pt.x > rcClient.right - rcSizeBox.right) {
            if (pt.x <= rcClient.right) {
                return SDL_HITTEST_RESIZE_RIGHT;//在窗口的右边框中
            }
            else {
                return SDL_HITTEST_NORMAL;//在工作区中
            }
        }
    }

    UiRect rcCaption;
    m_pOwner->OnNativeGetCaptionRect(rcCaption);
    //标题栏区域的矩形范围
    UiRect rcCaptionRect;
    rcCaptionRect.left = rcClient.left + rcCaption.left;
    rcCaptionRect.right = rcClient.right - rcCaption.right;
    rcCaptionRect.top = rcClient.top + rcCaption.top;
    rcCaptionRect.bottom = rcClient.top + rcCaption.bottom;
    if (rcCaptionRect.ContainsPt(pt)) {
        //在标题栏范围内（SDL不支持HTSYSMENU和HTMAXBUTTON，此处实现与Windows默认的不同）
        if (m_pOwner->OnNativeIsPtInCaptionBarControl(pt)) {
            return SDL_HITTEST_NORMAL;//在工作区中（放在标题栏上的控件，视为工作区）
        }
        else {
            return SDL_HITTEST_DRAGGABLE;//在标题栏中
        }
    }
    //其他，在工作区中
    return SDL_HITTEST_NORMAL;
}

void NativeWindow_SDL::InitNativeWindow()
{
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return;
    }

    SDL_WindowID id = SDL_GetWindowID(m_sdlWindow);
    SetWindowFromID(id, this);

    //设置Hit Test函数
    if (!IsUseSystemCaption()) {
        int nRet = SDL_SetWindowHitTest(m_sdlWindow, NativeWindow_SDL_HitTest, this);
        ASSERT_UNUSED_VARIABLE(nRet == 0);
    }
    else {
        int nRet = SDL_SetWindowHitTest(m_sdlWindow, nullptr, nullptr);
        ASSERT_UNUSED_VARIABLE(nRet == 0);
    }

    if (!m_createParam.m_windowTitle.empty()) {
        std::string windowTitle = StringUtil::TToUTF8(m_createParam.m_windowTitle);
        int nRet = SDL_SetWindowTitle(m_sdlWindow, windowTitle.c_str());
        ASSERT_UNUSED_VARIABLE(nRet == 0);
    }    
}

void NativeWindow_SDL::ClearNativeWindow()
{
    if (m_sdlWindow != nullptr) {    
        SDL_SetWindowHitTest(m_sdlWindow, nullptr, nullptr);
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
    }
    if (m_sdlRenderer != nullptr) {
        SDL_DestroyRenderer(m_sdlRenderer);
        m_sdlRenderer = nullptr;
    }
}

void* NativeWindow_SDL::GetWindowHandle() const
{
    return m_sdlWindow;
}

bool NativeWindow_SDL::IsWindow() const
{
    return (m_sdlWindow != nullptr);
}

#ifdef DUILIB_BUILD_FOR_WIN

HWND NativeWindow_SDL::GetHWND() const
{
    if (!IsWindow()) {
        return nullptr;
    }
    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
    HWND hWnd = (HWND)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if (!::IsWindow(hWnd)) {
        hWnd = nullptr;
    }
    return hWnd;
}

HMODULE NativeWindow_SDL::GetResModuleHandle() const
{
    if (!IsWindow()) {
        return nullptr;
    }
    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
    HMODULE hModule = (HMODULE)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);
    ASSERT(hModule != nullptr);
    if (hModule == nullptr) {
        hModule = ::GetModuleHandle(nullptr);
    }
    return hModule;
}

HDC NativeWindow_SDL::GetPaintDC() const
{
    if (!IsWindow()) {
        return nullptr;
    }
    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
    HDC hDC = (HDC)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HDC_POINTER, nullptr);
    ASSERT(hDC != nullptr);
    return hDC;
}

#endif //DUILIB_BUILD_FOR_WIN

void NativeWindow_SDL::CloseWnd(int32_t nRet)
{
    m_closeParam = nRet;
    m_bCloseing = true;

    //异步关闭
    if (m_sdlWindow != nullptr) {
        SDL_Event sdlEvent;
        sdlEvent.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
        sdlEvent.common.timestamp = 0;
        sdlEvent.window.data1 = 0;
        sdlEvent.window.data2 = 0;
        sdlEvent.window.windowID = SDL_GetWindowID(m_sdlWindow);
        int nRetE = SDL_PushEvent(&sdlEvent);
        ASSERT_UNUSED_VARIABLE(nRetE > 0);
    }
}

void NativeWindow_SDL::Close()
{
    //同步关闭
    m_bCloseing = true;
    if (m_pOwner == nullptr) {
        return;
    }

    //回调关闭事件
    bool bHandled = false;
    m_pOwner->OnNativeWindowCloseMsg(0, NativeMsg(0, 0, 0), bHandled);
    if (bHandled) {
        //拦截了关闭事件
        m_bCloseing = false;
        return;
    }

    //PreClose事件
    if (m_pOwner) {
        m_pOwner->OnNativePreCloseWindow();
    }

    //关闭窗口
    ClearNativeWindow();
}

bool NativeWindow_SDL::IsClosingWnd() const
{
    return m_bCloseing;
}

int32_t NativeWindow_SDL::GetCloseParam() const
{
    return m_closeParam;
}

bool NativeWindow_SDL::IsLayeredWindow() const
{
    return m_bIsLayeredWindow;
}

void NativeWindow_SDL::UpdateMinMaxBoxStyle() const
{
    //TODO:
    
    //更新最大化/最小化按钮的风格
    bool bMinimizeBox = false;
    bool bMaximizeBox = false;
    /*if (!IsUseSystemCaption() && (m_pOwner != nullptr) && m_pOwner->OnNativeHasMinMaxBox(bMinimizeBox, bMaximizeBox)) {
        UINT oldStyleValue = (UINT)::GetWindowLong(GetHWND(), GWL_STYLE);
        UINT newStyleValue = oldStyleValue;
        if (bMinimizeBox) {
            newStyleValue |= WS_MINIMIZEBOX;
        }
        else {
            newStyleValue &= ~WS_MINIMIZEBOX;
        }
        if (bMaximizeBox) {
            newStyleValue |= WS_MAXIMIZEBOX;
        }
        else {
            newStyleValue &= ~WS_MAXIMIZEBOX;
        }
        if (newStyleValue != oldStyleValue) {
            ::SetWindowLong(GetHWND(), GWL_STYLE, newStyleValue);
        }
    }*/
}

void NativeWindow_SDL::SetLayeredWindowAlpha(int32_t nAlpha)
{
    SetLayeredWindowOpacity(nAlpha);
}

uint8_t NativeWindow_SDL::GetLayeredWindowAlpha() const
{
    return GetLayeredWindowOpacity();
}

void NativeWindow_SDL::SetLayeredWindowOpacity(int32_t nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    m_nLayeredWindowOpacity = static_cast<uint8_t>(nAlpha);
    float opacity = 1.0f;//完全不透明
    if (m_nLayeredWindowOpacity != 255) {
        //支持不透明度设置
        opacity = 1.0f * nAlpha / 255.0f;
    }
    int nRet = SDL_SetWindowOpacity(m_sdlWindow, opacity);
    ASSERT_UNUSED_VARIABLE(nRet == 0);
}

uint8_t NativeWindow_SDL::GetLayeredWindowOpacity() const
{
    return m_nLayeredWindowOpacity;
}

void NativeWindow_SDL::SetUseSystemCaption(bool bUseSystemCaption)
{
    m_bUseSystemCaption = bUseSystemCaption;
    if (IsUseSystemCaption()) {
        //使用系统默认标题栏, 需要增加标题栏风格
        if (IsWindow()) {
            int nRet = SDL_SetWindowBordered(m_sdlWindow, SDL_TRUE);
            ASSERT_UNUSED_VARIABLE(nRet == 0);
        }
        //关闭层窗口
        if (IsLayeredWindow()) {
            SetLayeredWindow(false, false);
        }

        //设置Hit Test函数为默认
        SDL_SetWindowHitTest(m_sdlWindow, nullptr, nullptr);
    }
    else {
        //需要提前设置这个属性，在无边框的情况下，可以保持调整窗口大小的功能
        SDL_SetHint("SDL_BORDERLESS_RESIZABLE_STYLE", "true"); 
        int nRet = SDL_SetWindowBordered(m_sdlWindow, SDL_FALSE);
        ASSERT_UNUSED_VARIABLE(nRet == 0);

        //设置HitTest函数
        nRet = SDL_SetWindowHitTest(m_sdlWindow, NativeWindow_SDL_HitTest, this);
        ASSERT_UNUSED_VARIABLE(nRet == 0);
    }
    m_pOwner->OnNativeUseSystemCaptionBarChanged();
}

bool NativeWindow_SDL::IsUseSystemCaption() const
{
    return m_bUseSystemCaption;
}

bool NativeWindow_SDL::ShowWindow(ShowWindowCommands nCmdShow)
{
    ASSERT(IsWindow());
    if (m_sdlWindow == nullptr) {
        return false;
    }
    int nRet = -1;
    switch(nCmdShow)
    {
    case kSW_HIDE:
        nRet = SDL_HideWindow(m_sdlWindow);
        break;
    case kSW_SHOW:
    case kSW_SHOW_NORMAL:
        {
            const SDL_bool bActivate = SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, SDL_TRUE);
            if (!bActivate) {
                SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "true");
                ASSERT(SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, SDL_TRUE) == SDL_TRUE);
            }
            nRet = SDL_ShowWindow(m_sdlWindow);
            if (!bActivate) {
                SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "false");
                ASSERT(SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, SDL_TRUE) == SDL_FALSE);
            }
        }        
        break;
    case kSW_SHOW_NA:
    case kSW_SHOW_NOACTIVATE:
        {
            const SDL_bool bActivate = SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, SDL_TRUE);
            if (bActivate) {
                SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "false");
                ASSERT(SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, SDL_TRUE) == SDL_FALSE);
            }
            nRet = SDL_ShowWindow(m_sdlWindow);
            if (bActivate) {
                SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "true");
                ASSERT(SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, SDL_TRUE) == SDL_TRUE);
            }
        }
        break;
    case kSW_MINIMIZE:
    case kSW_SHOW_MINIMIZED:
        nRet = SDL_MinimizeWindow(m_sdlWindow);
        break;
    case kSW_SHOW_MAXIMIZED:
        nRet = SDL_MaximizeWindow(m_sdlWindow);
        break;
    case kSW_SHOW_MIN_NOACTIVE:
        //不支持No Active
        nRet = SDL_MinimizeWindow(m_sdlWindow);
        break;
    case kSW_RESTORE:
        nRet = SDL_RestoreWindow(m_sdlWindow);
        break;
    default:
        ASSERT(false);
        break;
    }
    return nRet == 0;
}

void NativeWindow_SDL::ShowModalFake(NativeWindow_SDL* pParentWindow)
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    if (!pParentWindow->IsWindow()) {
        pParentWindow = nullptr;
    }
    if (pParentWindow != nullptr) {
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(false);
        }
    }
    ShowWindow(kSW_SHOW_NORMAL);
    if (pParentWindow != nullptr) {
        int nRet = SDL_SetWindowModalFor(m_sdlWindow, pParentWindow->m_sdlWindow);
        ASSERT_UNUSED_VARIABLE(nRet == 0);
        m_bFakeModal = true;
    }    
}

void NativeWindow_SDL::OnCloseModalFake(NativeWindow_SDL* pParentWindow)
{
    if (IsFakeModal()) {
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(true);
            pParentWindow->SetWindowFocus();
        }
        m_bFakeModal = false;
    }
}

bool NativeWindow_SDL::IsFakeModal() const
{
    return m_bFakeModal;
}

void NativeWindow_SDL::CenterWindow()
{  
    ASSERT(IsWindow());
    UiRect rcDlg;
    GetWindowRect(rcDlg);
    UiRect rcArea;
    UiRect rcCenter;

    SDL_Window* pCenterWindow = SDL_GetWindowParent(m_sdlWindow);

    // 处理多显示器模式下屏幕居中
    UiRect rcMonitor;
    GetMonitorRect(m_sdlWindow, rcMonitor, rcArea);
    if (pCenterWindow == nullptr) {
        rcCenter = rcArea;
    }
    else if (SDL_GetWindowFlags(m_sdlWindow) & SDL_WINDOW_MINIMIZED) {
        rcCenter = rcArea;
    }
    else {
        GetWindowRect(pCenterWindow, rcCenter);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // Find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // The dialog is outside the screen, move it inside
    if (xLeft < rcArea.left) {
        xLeft = rcArea.left;
    }
    else if (xLeft + DlgWidth > rcArea.right) {
        xLeft = rcArea.right - DlgWidth;
    }
    if (yTop < rcArea.top) {
        yTop = rcArea.top;
    }
    else if (yTop + DlgHeight > rcArea.bottom) {
        yTop = rcArea.bottom - DlgHeight;
    }
    SetWindowPos(nullptr, InsertAfterFlag(), xLeft, yTop, -1, -1, kSWP_NOSIZE | kSWP_NOZORDER | kSWP_NOACTIVATE);
}

void NativeWindow_SDL::ToTopMost()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    int nRet = SDL_SetWindowAlwaysOnTop(m_sdlWindow, SDL_TRUE);
    ASSERT_UNUSED_VARIABLE(nRet == 0);
}

void NativeWindow_SDL::BringToTop()
{
    SetWindowForeground();
}

bool NativeWindow_SDL::SetWindowForeground()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    int nRet = SDL_RaiseWindow(m_sdlWindow);
    ASSERT(nRet == 0);
//#ifdef _DEBUG
//    备注：这里有时候会出现条件不成立的情况，暂时未发现影响功能
//    auto pKeyboardFocus = SDL_GetKeyboardFocus();
//    ASSERT(pKeyboardFocus == m_sdlWindow);
//#endif
    return (nRet == 0) ? true : false;
}

bool NativeWindow_SDL::IsWindowForeground() const
{
    if (!IsWindow()) {
        return false;
    }
    return (SDL_GetKeyboardFocus() == m_sdlWindow) ? true : false;
}

bool NativeWindow_SDL::SetWindowFocus()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    SDL_Window* pKeyboardFocus = SDL_GetKeyboardFocus();
    if (pKeyboardFocus != m_sdlWindow) {
        SetWindowForeground();
        pKeyboardFocus = SDL_GetKeyboardFocus();
    }
    return (pKeyboardFocus == m_sdlWindow) ? true : false;
}

bool NativeWindow_SDL::IsWindowFocused() const
{
    if (!IsWindow()) {
        return false;
    }
    return (SDL_GetKeyboardFocus() == m_sdlWindow) ? true : false;
}

void NativeWindow_SDL::CheckSetWindowFocus()
{
    if (!IsWindowFocused()) {
        SetWindowFocus();
    }
}

LRESULT NativeWindow_SDL::PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT((uMsg > SDL_EVENT_USER) && (uMsg < SDL_EVENT_LAST));
    if ((uMsg <= SDL_EVENT_USER) || (uMsg >= SDL_EVENT_LAST)) {
        return -1;
    }
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return -1;
    }
    SDL_Event sdlEvent;
    sdlEvent.type = uMsg;
    sdlEvent.common.timestamp = 0;
    sdlEvent.user.reserved = 0;
    sdlEvent.user.timestamp = 0;
    sdlEvent.user.code = uMsg;
    sdlEvent.user.data1 = (void*)wParam;
    sdlEvent.user.data2 = (void*)lParam;
    sdlEvent.user.windowID = SDL_GetWindowID(m_sdlWindow);
    int nRet = SDL_PushEvent(&sdlEvent);
    ASSERT_UNUSED_VARIABLE(nRet > 0);
    return nRet > 0 ? 0 : -1;
}

void NativeWindow_SDL::PostQuitMsg(int32_t /*nExitCode*/)
{
    SDL_Event sdlEvent;
    sdlEvent.type = SDL_EVENT_QUIT;
    sdlEvent.common.timestamp = 0;
    int nRet = SDL_PushEvent(&sdlEvent);
    ASSERT_UNUSED_VARIABLE(nRet > 0);
}

bool NativeWindow_SDL::EnterFullScreen()
{
    ASSERT(IsWindow());
    if (m_sdlWindow == nullptr) {
        return false;
    }
    if (IsWindowMinimized()) {
        //最小化的时候，不允许激活全屏
        return false;
    }
    if (m_bFullScreen) {
        return true;
    }
    m_bFullScreen = true;

    int nRet = SDL_SetWindowFullscreen(m_sdlWindow, SDL_TRUE);
    ASSERT_UNUSED_VARIABLE(nRet == 0);

    m_lastWindowFlags = ::SDL_GetWindowFlags(m_sdlWindow);
    if (m_lastWindowFlags & SDL_WINDOW_RESIZABLE) {
        //需要去掉可调整窗口大小的属性
        SDL_SetWindowResizable(m_sdlWindow, SDL_FALSE);
    }
    
    m_pOwner->OnNativeWindowEnterFullScreen();
    return true;
}

bool NativeWindow_SDL::ExitFullScreen()
{
    ASSERT(IsWindow());
    if (m_sdlWindow == nullptr) {
        return false;
    }
    if (!m_bFullScreen) {
        return false;
    }

    int nRet = SDL_SetWindowFullscreen(m_sdlWindow, SDL_FALSE);
    ASSERT_UNUSED_VARIABLE(nRet == 0);

    if (m_lastWindowFlags & SDL_WINDOW_RESIZABLE) {
        //需要恢复可调整窗口大小的属性
        SDL_SetWindowResizable(m_sdlWindow, SDL_TRUE);
    }

    m_bFullScreen = false;
    m_pOwner->OnNativeWindowExitFullScreen();
    return true;
}

bool NativeWindow_SDL::IsWindowMaximized() const
{
    ASSERT(IsWindow());
    bool bWindowMaximized = false;
    if (m_sdlWindow != nullptr) {
        SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
        bWindowMaximized = nFlags & SDL_WINDOW_MAXIMIZED;
    }
    return bWindowMaximized;
}

bool NativeWindow_SDL::IsWindowMinimized() const
{
    ASSERT(IsWindow());
    bool bWindowMinimized = false;
    if (m_sdlWindow != nullptr) {
        SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
        bWindowMinimized = nFlags & SDL_WINDOW_MINIMIZED;
    }
    return bWindowMinimized;
}

bool NativeWindow_SDL::IsWindowFullScreen() const
{
    ASSERT(IsWindow());
    bool bFullScreen = false;
    if (m_sdlWindow != nullptr) {
        SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
        bFullScreen = nFlags & SDL_WINDOW_FULLSCREEN;
    }
    return bFullScreen;
}

bool NativeWindow_SDL::IsWindowVisible() const
{
    if (!IsWindow()) {
        return false;
    }
    SDL_WindowFlags windowFlags = SDL_GetWindowFlags(m_sdlWindow);
    return (windowFlags & SDL_WINDOW_HIDDEN) ? false : true;
}

bool NativeWindow_SDL::SetWindowPos(const NativeWindow_SDL* /*pInsertAfterWindow*/,
                                   InsertAfterFlag /*insertAfterFlag*/,
                                   int32_t X, int32_t Y, int32_t cx, int32_t cy,
                                   uint32_t uFlags)
{
    bool bRet = true;
    ASSERT(IsWindow());
    if (!(uFlags & kSWP_NOMOVE)) {
        int nRet = SDL_SetWindowPosition(m_sdlWindow, X, Y);
        ASSERT(nRet == 0);
        if (nRet != 0) {
            bRet = false;
        }
    }
    if (!(uFlags & kSWP_NOSIZE)) {
        int nRet = SDL_SetWindowSize(m_sdlWindow, cx, cy);
        ASSERT(nRet == 0);
        if (nRet != 0) {
            bRet = false;
        }
    }

    if (uFlags & kSWP_HIDEWINDOW) {
        if (!ShowWindow(ShowWindowCommands::kSW_HIDE)) {
            bRet = false;
        }
    }
    if (uFlags & kSWP_SHOWWINDOW) {
        if (!ShowWindow(ShowWindowCommands::kSW_SHOW)) {
            bRet = false;
        }
    }
    if (bRet) {
        SDL_SyncWindow(m_sdlWindow);
    }    
    return bRet;
}

bool NativeWindow_SDL::MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool /*bRepaint*/)
{
    ASSERT(IsWindow());
    int nRet = SDL_SetWindowPosition(m_sdlWindow, X, Y);
    ASSERT(nRet == 0);
    bool bRet = (nRet == 0) ? true : false;
    nRet = SDL_SetWindowSize(m_sdlWindow, nWidth, nHeight);
    ASSERT(nRet == 0);
    if (nRet != 0) {
        bRet = false;
    }
    SDL_SyncWindow(m_sdlWindow);
    return bRet;
}

bool NativeWindow_SDL::SetWindowIcon(const FilePath& iconFilePath)
{
    //TODO:
    ASSERT(IsWindow());
    if (!::IsWindow(GetHWND())) {
        return false;
    }

    return true;
}

bool NativeWindow_SDL::SetWindowIcon(const std::vector<uint8_t>& iconFileData)
{
    //TODO:

    return true;
}

void NativeWindow_SDL::SetText(const DString& strText)
{
    ASSERT(IsWindow());
    //转为UTF-8编码
    DStringA utf8Text = StringUtil::TToUTF8(strText);
    int nRet = SDL_SetWindowTitle(m_sdlWindow, utf8Text.c_str());
    ASSERT_UNUSED_VARIABLE(nRet == 0);
}

void NativeWindow_SDL::SetWindowMaximumSize(const UiSize& szMaxWindow)
{
    m_szMaxWindow = szMaxWindow;
    if (m_szMaxWindow.cx < 0) {
        m_szMaxWindow.cx = 0;
    }
    if (m_szMaxWindow.cy < 0) {
        m_szMaxWindow.cy = 0;
    }
    ASSERT(IsWindow());
    if (m_sdlWindow != nullptr) {
        SDL_SetWindowMaximumSize(m_sdlWindow, m_szMaxWindow.cx, m_szMaxWindow.cy);
    }
}

const UiSize& NativeWindow_SDL::GetWindowMaximumSize() const
{
    return m_szMaxWindow;
}

void NativeWindow_SDL::SetWindowMinimumSize(const UiSize& szMinWindow)
{
    m_szMinWindow = szMinWindow;
    if (m_szMinWindow.cx < 0) {
        m_szMinWindow.cx = 0;
    }
    if (m_szMinWindow.cy < 0) {
        m_szMinWindow.cy = 0;
    }
    ASSERT(IsWindow());
    if (m_sdlWindow != nullptr) {
        SDL_SetWindowMinimumSize(m_sdlWindow, m_szMinWindow.cx, m_szMinWindow.cy);
    }
}

const UiSize& NativeWindow_SDL::GetWindowMinimumSize() const
{
    return m_szMinWindow;
}

void NativeWindow_SDL::SetCapture()
{
    ASSERT(SDL_GetMouseFocus() == m_sdlWindow);
    if (SDL_GetMouseFocus() == m_sdlWindow) {
        int nRet = SDL_CaptureMouse(SDL_TRUE);
        ASSERT_UNUSED_VARIABLE(nRet == 0);
        if (nRet == 0) {
            m_bMouseCapture = true;
        }
    }
}

void NativeWindow_SDL::ReleaseCapture()
{
    if (m_bMouseCapture) {
        ASSERT(SDL_GetMouseFocus() == m_sdlWindow);
        if (SDL_GetMouseFocus() == m_sdlWindow) {
            int nRet = SDL_CaptureMouse(SDL_FALSE);
            ASSERT_UNUSED_VARIABLE(nRet == 0);
        }
        m_bMouseCapture = false;
    }
}

bool NativeWindow_SDL::IsCaptured() const
{
    return m_bMouseCapture;
}

void NativeWindow_SDL::Invalidate(const UiRect& rcItem)
{
#ifdef DUILIB_BUILD_FOR_WIN
    HWND hWnd = GetHWND();
    RECT rc = { rcItem.left, rcItem.top, rcItem.right, rcItem.bottom };
    ::InvalidateRect(hWnd, &rc, FALSE);
#else
    //暂时没有此功能, 只能发送一个绘制消息，触发界面绘制
    if (m_sdlWindow != nullptr) {
        SDL_Event sdlEvent;
        sdlEvent.type = SDL_EVENT_WINDOW_EXPOSED;
        sdlEvent.common.timestamp = 0;
        sdlEvent.window.data1 = 0;
        sdlEvent.window.data2 = 0;
        sdlEvent.window.windowID = SDL_GetWindowID(m_sdlWindow);
        int nRet = SDL_PushEvent(&sdlEvent);
        ASSERT_UNUSED_VARIABLE(nRet > 0);
    }
#endif
}

void NativeWindow_SDL::GetClientRect(UiRect& rcClient) const
{
    rcClient.Clear();
    ASSERT(IsWindow());
    int nWidth = 0;
    int nHeight = 0;
    int nRet = SDL_GetWindowSize(m_sdlWindow, &nWidth, &nHeight);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        rcClient.left = 0;
        rcClient.top = 0;
        rcClient.right = rcClient.left + nWidth;
        rcClient.bottom = rcClient.top + nHeight;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    {
        HWND hWnd = GetHWND();        
        if (!::IsIconic(hWnd) && ::IsWindowVisible(hWnd)) {
            //最小化的时候，或者隐藏的时候，不比对，两者不同
            RECT rect = { 0, };
            ::GetClientRect(hWnd, &rect);
            ASSERT(rcClient.left == rect.left);
            ASSERT(rcClient.top == rect.top);
            ASSERT(rcClient.right == rect.right);
            ASSERT(rcClient.bottom == rect.bottom);
        }
    }
#endif
}

void NativeWindow_SDL::GetWindowRect(UiRect& rcWindow) const
{
    GetWindowRect(m_sdlWindow, rcWindow);
}

void NativeWindow_SDL::GetWindowRect(SDL_Window* sdlWindow, UiRect& rcWindow) const
{
    rcWindow.Clear();
    ASSERT(sdlWindow != nullptr);
    if (sdlWindow == nullptr) {
        return;
    }

    //窗口的左上角坐标值（屏幕坐标）
    int nXPos = 0;
    int nYPos = 0;
    SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);

    //边框大小
    int nTopBorder = 0;
    int nLeftBorder = 0;
    int nBottomBorder = 0;
    int nRightBorder = 0;
    SDL_GetWindowBordersSize(m_sdlWindow, &nTopBorder, &nLeftBorder, &nBottomBorder, &nRightBorder);

    //客户区大小
    int nWidth = 0;
    int nHeight = 0;
    SDL_GetWindowSize(m_sdlWindow, &nWidth, &nHeight);

    rcWindow.left = nXPos - nLeftBorder;
    rcWindow.top = nYPos - nTopBorder;
    rcWindow.right = rcWindow.left + nWidth + nLeftBorder + nRightBorder;
    rcWindow.bottom = rcWindow.top + nHeight + nTopBorder + nBottomBorder;

#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    {
        HWND hWnd = GetHWND();
        RECT rect = { 0, };
        ::GetWindowRect(hWnd, &rect);
        ASSERT(rcWindow.left == rect.left);
        ASSERT(rcWindow.top == rect.top);
        ASSERT(rcWindow.right == rect.right);
        ASSERT(rcWindow.bottom == rect.bottom);
    }
#endif
}

void NativeWindow_SDL::ScreenToClient(UiPoint& pt) const
{
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    POINT ptWnd = { pt.x, pt.y };
#endif
    int nXPos = 0;
    int nYPos = 0;
    int nRet = SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        pt.x -= nXPos;
        pt.y -= nYPos;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    {
        HWND hWnd = GetHWND();
        ::ScreenToClient(hWnd, &ptWnd);
        ASSERT(ptWnd.x == pt.x);
        ASSERT(ptWnd.y == pt.y);
    }
#endif
}

void NativeWindow_SDL::ClientToScreen(UiPoint& pt) const
{
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    POINT ptWnd = { pt.x, pt.y };
#endif
    int nXPos = 0;
    int nYPos = 0;
    int nRet = SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        pt.x += nXPos;
        pt.y += nYPos;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    {
        HWND hWnd = GetHWND();
        ::ClientToScreen(hWnd, &ptWnd);
        ASSERT(ptWnd.x == pt.x);
        ASSERT(ptWnd.y == pt.y);
    }
#endif
}

void NativeWindow_SDL::GetCursorPos(UiPoint& pt) const
{
    float x = 0;
    float y = 0;
    SDL_GetMouseState(&x, &y);
    pt.x = (int32_t)x;
    pt.y = (int32_t)y;
}

bool NativeWindow_SDL::GetMonitorRect(UiRect& rcMonitor) const
{
    UiRect rcWork;
    return GetMonitorRect(m_sdlWindow, rcMonitor, rcWork);
}

bool NativeWindow_SDL::GetMonitorRect(SDL_Window* sdlWindow, UiRect& rcMonitor, UiRect& rcWork) const
{
    rcMonitor.Clear();
    rcWork.Clear();

    SDL_DisplayID displayID = 0;
    if (sdlWindow != nullptr) {
        displayID = SDL_GetDisplayForWindow(sdlWindow);
    }
    if (displayID == 0) {
        displayID = SDL_GetPrimaryDisplay();
    }
    ASSERT(displayID != 0);
    if (displayID == 0) {
        return false;
    }

    SDL_Rect rect = {0, };
    int32_t nRet = SDL_GetDisplayBounds(displayID, &rect);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        rcMonitor.left = rect.x;
        rcMonitor.top = rect.y;
        rcMonitor.right = rcMonitor.left + rect.w;
        rcMonitor.bottom = rcMonitor.top + rect.h;
    }

    nRet = SDL_GetDisplayUsableBounds(displayID, &rect);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        rcWork.left = rect.x;
        rcWork.top = rect.y;
        rcWork.right = rcWork.left + rect.w;
        rcWork.bottom = rcWork.top + rect.h;
    }
    return !rcMonitor.IsEmpty() && !rcWork.IsEmpty();
}

bool NativeWindow_SDL::GetMonitorWorkRect(UiRect& rcWork) const
{
    UiRect rcMonitor;
    return GetMonitorRect(m_sdlWindow, rcMonitor, rcWork);
}

bool NativeWindow_SDL::GetPrimaryMonitorWorkRect(UiRect& rcWork)
{
    rcWork.Clear();
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) == -1) {
            SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
            return false;
        }
    }

    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    ASSERT(displayID != 0);
    if (displayID == 0) {
        return false;
    }
    SDL_Rect rect = { 0, };
    int32_t nRet = SDL_GetDisplayUsableBounds(displayID, &rect);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        rcWork.left = rect.x;
        rcWork.top = rect.y;
        rcWork.right = rcWork.left + rect.w;
        rcWork.bottom = rcWork.top + rect.h;
    }
    return !rcWork.IsEmpty();
}

bool NativeWindow_SDL::GetMonitorWorkRect(const UiPoint& pt, UiRect& rcWork) const
{
    rcWork.Clear();
    SDL_Point point = {pt.x, pt.y};
    SDL_DisplayID displayID = SDL_GetDisplayForPoint(&point);
    if (displayID == 0) {
        displayID = SDL_GetPrimaryDisplay();
    }
    ASSERT(displayID != 0);
    if (displayID == 0) {
        return false;
    }

    SDL_Rect rect = { 0, };
    int32_t nRet = SDL_GetDisplayUsableBounds(displayID, &rect);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        rcWork.left = rect.x;
        rcWork.top = rect.y;
        rcWork.right = rcWork.left + rect.w;
        rcWork.bottom = rcWork.top + rect.h;
    }
    return !rcWork.IsEmpty();
}

const UiPoint& NativeWindow_SDL::GetLastMousePos() const
{
    return m_ptLastMousePos;
}

void NativeWindow_SDL::SetLastMousePos(const UiPoint& pt)
{
    m_ptLastMousePos = pt;
}

INativeWindow* NativeWindow_SDL::WindowBaseFromPoint(const UiPoint& pt)
{
    SDL_Window* pKeyboardFocus = SDL_GetKeyboardFocus();
    if (pKeyboardFocus != nullptr) {
        SDL_Window* sdlWindow = pKeyboardFocus;
        UiRect rcWindow;
        GetWindowRect(sdlWindow, rcWindow);
        if (rcWindow.ContainsPt(pt)) {
            NativeWindow_SDL* pWindow = GetWindowFromID(SDL_GetWindowID(sdlWindow));
            if (pWindow != nullptr) {
                return pWindow->m_pOwner;
            }
        }
    }
    SDL_Window* pMouseFocus = SDL_GetMouseFocus();
    if (pMouseFocus != nullptr) {
        SDL_Window* sdlWindow = pMouseFocus;
        UiRect rcWindow;
        GetWindowRect(sdlWindow, rcWindow);
        if (rcWindow.ContainsPt(pt)) {
            NativeWindow_SDL* pWindow = GetWindowFromID(SDL_GetWindowID(sdlWindow));
            if (pWindow != nullptr) {
                return pWindow->m_pOwner;
            }
        }
    }

    int nCount = 0;
    SDL_Window* const* pWindowList = SDL_GetWindows(&nCount);
    if ((nCount <= 0) || (pWindowList == nullptr)){
        return nullptr;
    }
    //TODO: 没考虑Z-Order的问题
    for (int nIndex = 0; nIndex < nCount; ++nIndex) {
        SDL_Window* sdlWindow = pWindowList[nIndex];
        if (sdlWindow != nullptr) {
            UiRect rcWindow;
            GetWindowRect(sdlWindow, rcWindow);
            if (rcWindow.ContainsPt(pt)) {
                NativeWindow_SDL* pWindow = GetWindowFromID(SDL_GetWindowID(sdlWindow));
                if (pWindow != nullptr) {
                    return pWindow->m_pOwner;
                }
            }
        }
    }
    return nullptr;
}

void NativeWindow_SDL::OnFinalMessage()
{
    if (m_pOwner) {
        m_pOwner->OnNativeFinalMessage();
    }
}

bool NativeWindow_SDL::SetLayeredWindow(bool bIsLayeredWindow, bool /*bRedraw*/)
{
    //不支持该功能
    ASSERT(bIsLayeredWindow == bIsLayeredWindow);
    //m_bIsLayeredWindow = bIsLayeredWindow;
    //SDL_WINDOW_TRANSPARENT 这个属性，不支持修改，所以此属性不支持修改，在创建窗口的时候已经设置正确的属性
    return true;
}

bool NativeWindow_SDL::IsDoModal() const
{
    //不支持该功能
    return false;
}

bool NativeWindow_SDL::KillWindowFocus()
{
    //不支持此功能
    return false;
}

bool NativeWindow_SDL::EnableWindow(bool /*bEnable*/)
{
    //未能提供此功能
    return true;
}

bool NativeWindow_SDL::IsWindowEnabled() const
{
    //未能提供此功能
    return true;
}

bool NativeWindow_SDL::SetWindowRoundRectRgn(const UiRect& /*rcWnd*/, const UiSize& /*szRoundCorner*/, bool /*bRedraw*/)
{
    //没有此功能
    return false;
}

void NativeWindow_SDL::ClearWindowRgn(bool /*bRedraw*/)
{
    //没有此功能
}

bool NativeWindow_SDL::UpdateWindow() const
{
    //没有此功能
    return false;
}

void NativeWindow_SDL::KeepParentActive()
{
    //不支持此功能
}

int32_t NativeWindow_SDL::SetWindowHotKey(uint8_t /*wVirtualKeyCode*/, uint8_t /*wModifiers*/)
{
    //不支持此功能
    ASSERT(0);
    return -1;
}

bool NativeWindow_SDL::GetWindowHotKey(uint8_t& /*wVirtualKeyCode*/, uint8_t& /*wModifiers*/) const
{
    //不支持此功能
    ASSERT(0);
    return false;
}

bool NativeWindow_SDL::RegisterHotKey(uint8_t /*wVirtualKeyCode*/, uint8_t /*wModifiers*/, int32_t /*id*/)
{
    //不支持此功能
    ASSERT(0);
    return false;
}

bool NativeWindow_SDL::UnregisterHotKey(int32_t /*id*/)
{
    //不支持此功能
    ASSERT(0);
    return false;
}

LRESULT NativeWindow_SDL::CallDefaultWindowProc(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    //不支持此功能
    ASSERT(0);
    return -1;
}

void NativeWindow_SDL::SetEnableSnapLayoutMenu(bool /*bEnable*/)
{
    //不支持此功能
}

bool NativeWindow_SDL::IsEnableSnapLayoutMenu() const
{
    //不支持此功能
    return false;
}

void NativeWindow_SDL::SetEnableSysMenu(bool /*bEnable*/)
{
    //不支持此功能
}

bool NativeWindow_SDL::IsEnableSysMenu() const
{
    //不支持此功能
    return false;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL

//
//LRESULT NativeWindow_SDL::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
//{
//    LRESULT lResult = 0;
//    bHandled = false;
//    switch (uMsg)
//    {
//
//    case WM_IME_STARTCOMPOSITION://不支持
//    {
//        lResult = m_pOwner->OnNativeImeStartCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_IME_ENDCOMPOSITION://不支持
//    {
//        lResult = m_pOwner->OnNativeImeEndCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_SETCURSOR://不支持（需要处理）
//    {
//        if (LOWORD(lParam) == HTCLIENT) {
//            //只处理设置客户区的光标
//            lResult = m_pOwner->OnNativeSetCursorMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
//        }
//        break;
//    }
//    case WM_CONTEXTMENU://不支持
//    {
//        UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
//        if ((pt.x != -1) && (pt.y != -1)) {
//            ScreenToClient(pt);
//        }
//        lResult = m_pOwner->OnNativeContextMenuMsg(pt, NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_CHAR:
//    {
//        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
//        uint32_t modifierKey = 0;
//        GetModifiers(uMsg, wParam, lParam, modifierKey);
//        lResult = m_pOwner->OnNativeCharMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_HOTKEY://不支持
//    {
//        int32_t hotkeyId = (int32_t)wParam;
//        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>((int32_t)(int16_t)HIWORD(lParam));
//        uint32_t modifierKey = 0;
//        GetModifiers(uMsg, wParam, lParam, modifierKey);
//        lResult = m_pOwner->OnNativeHotKeyMsg(hotkeyId, vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_MOUSEHOVER://不支持
//    {
//        UiPoint pt;
//        pt.x = GET_X_LPARAM(lParam);
//        pt.y = GET_Y_LPARAM(lParam);
//        uint32_t modifierKey = 0;
//        GetModifiers(uMsg, wParam, lParam, modifierKey);
//        lResult = m_pOwner->OnNativeMouseHoverMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//
//    case WM_CAPTURECHANGED://不支持
//    {
//        lResult = m_pOwner->OnNativeCaptureChangedMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    default:
//        break;
//    }//end of switch
//    return lResult;
//}


//bool NativeWindow_SDL::GetModifiers(UINT message, WPARAM wParam, LPARAM lParam, uint32_t& modifierKey) const
//{
//    //逻辑修改，需要同步给函数：Window::IsKeyDown
//    bool bRet = true;
//    modifierKey = ModifierKey::kNone;
//    switch (message) {
//    case WM_CHAR:
//        if (0 == (lParam & (1 << 30))) {
//            modifierKey |= ModifierKey::kFirstPress;
//        }
//        if (lParam & (1 << 29)) {
//            modifierKey |= ModifierKey::kAlt;
//        }
//        break;
//    case WM_KEYDOWN:
//    case WM_SYSKEYDOWN:
//        if (0 == (lParam & (1 << 30))) {
//            modifierKey |= ModifierKey::kFirstPress;
//        }
//        if (lParam & (1 << 29)) {
//            modifierKey |= ModifierKey::kAlt;
//        }
//        break;
//    case WM_KEYUP:
//    case WM_SYSKEYUP:
//        if (lParam & (1 << 29)) {
//            modifierKey |= ModifierKey::kAlt;
//        }
//        break;
//    case WM_MOUSEWHEEL:
//    {
//        WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
//        if (fwKeys & MK_CONTROL) {
//            modifierKey |= ModifierKey::kControl;
//        }
//        if (fwKeys & MK_SHIFT) {
//            modifierKey |= ModifierKey::kShift;
//        }
//    }
//    break;
//    case WM_MOUSEHOVER:
//    case WM_MOUSEMOVE:
//    case WM_LBUTTONDOWN:
//    case WM_LBUTTONUP:
//    case WM_LBUTTONDBLCLK:
//    case WM_RBUTTONDOWN:
//    case WM_RBUTTONUP:
//    case WM_RBUTTONDBLCLK:
//    case WM_MBUTTONDOWN:
//    case WM_MBUTTONUP:
//    case WM_MBUTTONDBLCLK:
//    case WM_XBUTTONDOWN:
//    case WM_XBUTTONUP:
//    case WM_XBUTTONDBLCLK:
//        if (wParam & MK_CONTROL) {
//            modifierKey |= ModifierKey::kControl;
//        }
//        if (wParam & MK_SHIFT) {
//            modifierKey |= ModifierKey::kShift;
//        }
//        break;
//    case WM_HOTKEY:
//    {
//        uint16_t nMod = (uint16_t)LOWORD(lParam);
//        if (nMod & MOD_ALT) {
//            modifierKey |= ModifierKey::kAlt;
//        }
//        else if (nMod & MOD_CONTROL) {
//            modifierKey |= ModifierKey::kControl;
//        }
//        else if (nMod & MOD_SHIFT) {
//            modifierKey |= ModifierKey::kShift;
//        }
//        else if (nMod & MOD_WIN) {
//            modifierKey |= ModifierKey::kWin;
//        }
//    }
//    break;
//    default:
//        bRet = false;
//        break;
//    }
//    ASSERT(bRet);
//    return bRet;
//}

//SDL源码：
//if (style & WS_POPUP) {
//    window->flags |= SDL_WINDOW_BORDERLESS;
//}
//else {
//    window->flags &= ~SDL_WINDOW_BORDERLESS;
//}
//if (style & WS_THICKFRAME) {
//    window->flags |= SDL_WINDOW_RESIZABLE;
//}
//else {
//    window->flags &= ~SDL_WINDOW_RESIZABLE;
//}
