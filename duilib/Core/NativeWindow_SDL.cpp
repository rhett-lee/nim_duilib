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
        ASSERT(iter->second->m_sdlWindow == SDL_GetWindowFromID(id));
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
    ASSERT(m_pOwner != nullptr);
    if (m_pOwner == nullptr) {
        return false;
    }
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }
    bool bHandled = false;
    LRESULT lResult = 0;
    switch (sdlEvent.type) {
    case SDL_EVENT_WINDOW_RESIZED:
        {
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
            lResult = m_pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_RESIZED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
        {
            bool bPaint = m_pOwner->OnNativePreparePaint();
            if (bPaint) {
                IRender* pRender = m_pOwner->OnNativeGetRender();
                ASSERT(pRender != nullptr);
                if (pRender != nullptr) {
                    NativeWindowRenderPaint renderPaint;
                    renderPaint.m_pNativeWindow = this;
                    renderPaint.m_pOwner = m_pOwner;
                    renderPaint.m_nativeMsg = NativeMsg(SDL_EVENT_WINDOW_EXPOSED, 0, 0);
                    renderPaint.m_bHandled = bHandled;
                    bPaint = pRender->PaintAndSwapBuffers(&renderPaint);
                    bHandled = renderPaint.m_bHandled;
                }
            }
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        {
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.motion.x;
            pt.y = (int32_t)sdlEvent.motion.y;
            uint32_t modifierKey = 0;
            //GetModifiers(); SDL 没有开放获取按键的数据
            lResult = m_pOwner->OnNativeMouseMoveMsg(pt, modifierKey, false, NativeMsg(SDL_EVENT_MOUSE_MOTION, 0, 0), bHandled);
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
            lResult = m_pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_WHEEL, 0, 0), bHandled);
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
                lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_DOWN, 0, 0), bHandled);
            }
            else if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
                //鼠标右键
                lResult = m_pOwner->OnNativeMouseRButtonDownMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_DOWN, 0, 0), bHandled);
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
                    lResult = m_pOwner->OnNativeMouseLButtonDbClickMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
                else {
                    lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
            }
            else if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
                //鼠标右键
                if (bDoubleClick) {
                    lResult = m_pOwner->OnNativeMouseRButtonDbClickMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
                else {
                    lResult = m_pOwner->OnNativeMouseRButtonUpMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
            }
        }
        break;
    case SDL_EVENT_KEY_DOWN:
        {            
            VirtualKeyCode vkCode = GetVirtualKeyCode(sdlEvent.key.key);
            uint32_t modifierKey = GetModifiers(sdlEvent.key.mod);
            lResult = m_pOwner->OnNativeKeyDownMsg(vkCode, modifierKey, NativeMsg(SDL_EVENT_KEY_DOWN, sdlEvent.key.key, sdlEvent.key.mod), bHandled);
        }
        break;
    case SDL_EVENT_KEY_UP:
    {
        VirtualKeyCode vkCode = GetVirtualKeyCode(sdlEvent.key.key);
        uint32_t modifierKey = GetModifiers(sdlEvent.key.mod);
        lResult = m_pOwner->OnNativeKeyUpMsg(vkCode, modifierKey, NativeMsg(SDL_EVENT_KEY_UP, sdlEvent.key.key, sdlEvent.key.mod), bHandled);
    }
    break;
    default:
        break;
    }

                //WM_PAINT:             SDL_EVENT_WINDOW_EXPOSED
    //WM_SHOWWINDOW         SDL_EVENT_WINDOW_SHOWN,SDL_EVENT_WINDOW_HIDDEN
    //WM_SET_FOCUS:         SDL_EVENT_WINDOW_FOCUS_GAINED
    //WM_KILL_FOCUS:        SDL_EVENT_WINDOW_FOCUS_LOST

    //WM_SIZE：              SDL_EVENT_WINDOW_RESIZED
    
                //WM_MOUSEMOVE:         SDL_EVENT_MOUSE_MOTION
                //WM_MOUSEWHEEL:        SDL_EVENT_MOUSE_WHEEL
    //
    //WM_KEYDOWN:           SDL_EVENT_KEY_DOWN
    //WM_KEYUP:             SDL_EVENT_KEY_UP
    //WM_CHAR:              SDL_EVENT_TEXT_INPUT
    //

    //WM_GETMINMAXINFO: 需要主动设置
    //WM_SETCURSOR：好像没有开放事件接口
    //WM_CLOSE：             SDL_EVENT_WINDOW_CLOSE_REQUESTED
    //WM_NCCALCSIZE： 没开放
                //WM_NCHITTEST：         SDL_EVENT_WINDOW_HIT_TEST，有个回调函数
    //WM_DPICHANGED： 没开放
    return true;
}

//系统菜单延迟显示的定时器ID
#define UI_SYS_MEMU_TIMER_ID 711

NativeWindow_SDL::NativeWindow_SDL(INativeWindow* pOwner):
    m_pOwner(pOwner),
    m_sdlWindow(nullptr),
    m_sdlRenderer(nullptr),

    m_hWnd(nullptr),
    m_hResModule(nullptr),
    m_hDcPaint(nullptr),
    m_bIsLayeredWindow(false),
    m_nLayeredWindowAlpha(255),
    m_nLayeredWindowOpacity(255),
    m_bUseSystemCaption(false),
    m_bMouseCapture(false),
    m_bCloseing(false),
    m_closeParam(kWindowCloseNormal),
    m_bFakeModal(false),
    m_bFullScreen(false),
    m_dwLastStyle(0),
    m_ptLastMousePos(-1, -1),
    m_pfnOldWndProc(nullptr),
    m_bDoModal(false),
    m_bCenterWindow(false),
    m_bCloseByEsc(false),
    m_bCloseByEnter(false),
    m_bSnapLayoutMenu(false),
    m_bEnableSysMenu(true),
    m_nSysMenuTimerId(0)
{
    ASSERT(m_pOwner != nullptr);
    m_rcLastWindowPlacement = { sizeof(WINDOWPLACEMENT), };

    
}

NativeWindow_SDL::~NativeWindow_SDL()
{
    ASSERT(m_sdlWindow == nullptr);
    ClearNativeWindow();
}

bool NativeWindow_SDL::CreateWnd(NativeWindow_SDL* pParentWindow, const WindowCreateParam& createParam)
{
    ASSERT(m_sdlWindow == nullptr);
    if (m_sdlWindow != nullptr) {
        return false;
    }
    ASSERT(m_sdlRenderer == nullptr);
    if (m_sdlRenderer != nullptr) {
        return false;
    }
    
    //保存参数
    m_createParam = createParam;
    SDL_WindowFlags windowFlags = SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_TRANSPARENT | SDL_WINDOW_RESIZABLE;
    int nRet = SDL_CreateWindowAndRenderer("Hello SDL", 640, 480, windowFlags, &m_sdlWindow, &m_sdlRenderer);
    ASSERT(nRet == 0);
    ASSERT(m_sdlWindow != nullptr);
    ASSERT(m_sdlRenderer != nullptr);
    if (m_sdlWindow != nullptr) {
        //初始化
        InitNativeWindow();

        if (m_pOwner != nullptr) {
            bool bHandled = false;
            m_pOwner->OnNativeCreateWndMsg(false, NativeMsg(0, 0, 0), bHandled);
        }

        //更新最大化/最小化按钮的风格
        UpdateMinMaxBoxStyle();
    }
    return (m_sdlWindow != nullptr);
}

int32_t NativeWindow_SDL::DoModal(NativeWindow_SDL* pParentWindow, const WindowCreateParam& createParam,
                              bool bCenterWindow, bool bCloseByEsc, bool bCloseByEnter)
{
    ASSERT(m_hWnd == nullptr);
    if (m_hWnd != nullptr) {
        return false;
    }
    m_hResModule = (HMODULE)createParam.m_platformData;
    if (m_hResModule == nullptr) {
        m_hResModule = ::GetModuleHandle(nullptr);
    }

    //保存参数
    m_createParam = createParam;
    m_bCenterWindow = bCenterWindow;
    m_bCloseByEsc = bCloseByEsc;
    m_bCloseByEnter = bCloseByEnter;

    //设置默认风格
    uint32_t dwStyle = createParam.m_dwStyle;
    if (dwStyle == 0) {
        dwStyle = WS_POPUPWINDOW;
        m_createParam.m_dwStyle = dwStyle;
    }

    //初始化层窗口属性
    m_bIsLayeredWindow = false;
    if (createParam.m_dwExStyle & WS_EX_LAYERED) {
        m_bIsLayeredWindow = true;
    }

    //窗口的位置和大小
    short x = 0;
    short y = 0;
    short cx = 0;
    short cy = 0;

    if (createParam.m_nX != kCW_USEDEFAULT) {
        x = (short)createParam.m_nX;
    }
    if (createParam.m_nY != kCW_USEDEFAULT) {
        y = (short)createParam.m_nY;
    }
    if (createParam.m_nWidth != kCW_USEDEFAULT) {
        cx = (short)createParam.m_nWidth;
    }
    if (createParam.m_nHeight != kCW_USEDEFAULT) {
        cy = (short)createParam.m_nHeight;
    }

    // 创建对话框资源结构体（对话框初始状态为可见状态）
    DLGTEMPLATE dlgTemplate = {
        WS_VISIBLE | dwStyle,
        createParam.m_dwExStyle,
        0,
        x, y, cx, cy
    };

    // 将对话框按钮添加到资源中
    constexpr const size_t nDataSize = sizeof(DLGTEMPLATE) + 32;
    HGLOBAL hResource = ::GlobalAlloc(GMEM_MOVEABLE, nDataSize);
    if (hResource == nullptr) {
        return -1;
    }
    LPDLGTEMPLATE lpDialogTemplate = (LPDLGTEMPLATE)::GlobalLock(hResource);
    if (lpDialogTemplate == nullptr) {
        ::GlobalFree(hResource);
        return -1;
    }
    ::memset(lpDialogTemplate, 0, nDataSize);
    *lpDialogTemplate = dlgTemplate;

    // 显示对话框
    HWND hParentWnd = nullptr;
    if (pParentWindow != nullptr) {
        hParentWnd = pParentWindow->GetHWND();
    }
    else {
        hParentWnd = ::GetActiveWindow();
    }
    //标记为模式对话框状态
    m_bDoModal = true;

    //显示模态对话框
    INT_PTR nRet = ::DialogBoxIndirectParam(GetResModuleHandle(), (LPDLGTEMPLATE)lpDialogTemplate, hParentWnd, NativeWindow_SDL::__DialogProc, (LPARAM)this);
    // 清理资源
    ::GlobalUnlock(hResource);
    ::GlobalFree(hResource);
    if (nRet != -1) {
        nRet = m_closeParam;
    }
    return (int32_t)nRet;
}

LRESULT NativeWindow_SDL::OnCreateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    bHandled = false;
    //初始化
    InitNativeWindow();

    if (m_pOwner != nullptr) {
        m_pOwner->OnNativeCreateWndMsg(false, NativeMsg(uMsg, wParam, lParam), bHandled);
    }

    //更新最大化/最小化按钮的风格
    UpdateMinMaxBoxStyle();

    return 0;
}

LRESULT NativeWindow_SDL::OnInitDialogMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    //初始化
    InitNativeWindow();

    if (m_pOwner != nullptr) {
        m_pOwner->OnNativeCreateWndMsg(true, NativeMsg(uMsg, wParam, lParam), bHandled);
    }

    //更新最大化/最小化按钮的风格
    UpdateMinMaxBoxStyle();

    if (m_bCenterWindow) {
        //窗口居中
        CenterWindow();
    }

    //标记为已经处理，不再调用窗口默认处理函数
    bHandled = true;
    return TRUE;
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

    //HWND hWnd = m_hWnd;
    //if (!::IsWindow(hWnd)) {
    //    return;
    //}

    ////检查并更新曾窗口属性
    //m_bIsLayeredWindow = false;
    //if (m_createParam.m_dwExStyle & WS_EX_LAYERED) {
    //    m_bIsLayeredWindow = true;
    //}
    //bool bChanged = false;
    //SetLayeredWindowStyle(m_bIsLayeredWindow, bChanged);

    ////初始化窗口相关DC
    //ASSERT(m_hDcPaint == nullptr);
    //m_hDcPaint = ::GetDC(hWnd);

    ////注册接受Touch消息
    ////RegisterTouchWindowWrapper(hWnd, 0);

    if (!m_createParam.m_windowTitle.empty()) {
        std::string windowTitle = StringUtil::TToUTF8(m_createParam.m_windowTitle);
        int nRet = SDL_SetWindowTitle(m_sdlWindow, windowTitle.c_str());
        ASSERT_UNUSED_VARIABLE(nRet == 0);
    }    
}

void NativeWindow_SDL::ClearNativeWindow()
{
    m_sdlRenderer = nullptr;//不需要单独释放，在SDL内部会释放资源
    if (m_sdlWindow != nullptr) {
        SDL_WindowID id = SDL_GetWindowID(m_sdlWindow);
        ClearWindowFromID(id, this);
        SDL_SetWindowHitTest(m_sdlWindow, nullptr, nullptr);
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
    }
}

HWND NativeWindow_SDL::GetHWND() const
{
    return m_hWnd;
}

void* NativeWindow_SDL::GetWindowHandle() const
{
    return m_sdlWindow;
}

bool NativeWindow_SDL::IsWindow() const
{
    return (m_sdlWindow != nullptr);
}

HMODULE NativeWindow_SDL::GetResModuleHandle() const
{
    return (m_hResModule != nullptr) ? m_hResModule : (::GetModuleHandle(nullptr));
}

HDC NativeWindow_SDL::GetPaintDC() const
{
    return m_hDcPaint;
}

void NativeWindow_SDL::CloseWnd(int32_t nRet)
{
    m_closeParam = nRet;
    m_bCloseing = true;

    m_sdlRenderer = nullptr;//不需要单独释放，在SDL内部会释放资源
    if (m_sdlWindow != nullptr) {
        ::SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
    }
}

void NativeWindow_SDL::Close()
{
    //SDL没有同步和异步之分，只有一种同步的关闭窗口的方法
    CloseWnd(0);
}

bool NativeWindow_SDL::IsClosingWnd() const
{
    return m_bCloseing;
}

int32_t NativeWindow_SDL::GetCloseParam() const
{
    return m_closeParam;
}

bool NativeWindow_SDL::SetLayeredWindow(bool bIsLayeredWindow, bool bRedraw)
{
    m_bIsLayeredWindow = bIsLayeredWindow;
    SetLayeredWindowStyle(bIsLayeredWindow);
    return true;
}

bool NativeWindow_SDL::SetLayeredWindowStyle(bool bIsLayeredWindow) const
{
    //SDL_WINDOW_TRANSPARENT 这个属性，不支持修改？？？
    if (bIsLayeredWindow) {

    }
    else {

    }
    return true;
}

bool NativeWindow_SDL::IsLayeredWindow() const
{
    return m_bIsLayeredWindow;
}

void NativeWindow_SDL::UpdateMinMaxBoxStyle() const
{
    //更新最大化/最小化按钮的风格
    bool bMinimizeBox = false;
    bool bMaximizeBox = false;
    if (!IsUseSystemCaption() && (m_pOwner != nullptr) && m_pOwner->OnNativeHasMinMaxBox(bMinimizeBox, bMaximizeBox)) {
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
    }
}

void NativeWindow_SDL::SetLayeredWindowAlpha(int32_t nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    m_nLayeredWindowAlpha = static_cast<uint8_t>(nAlpha);
}

uint8_t NativeWindow_SDL::GetLayeredWindowAlpha() const
{
    return m_nLayeredWindowAlpha;
}

void NativeWindow_SDL::SetLayeredWindowOpacity(int32_t nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    m_nLayeredWindowOpacity = static_cast<uint8_t>(nAlpha);
    if (m_nLayeredWindowOpacity == 255) {
        COLORREF crKey = 0;
        BYTE bAlpha = 0;
        DWORD dwFlags = LWA_ALPHA | LWA_COLORKEY;
        bool bAttributes = ::GetLayeredWindowAttributes(m_hWnd, &crKey, &bAlpha, &dwFlags) != FALSE;
        if (bAttributes) {
            bool bRet = ::SetLayeredWindowAttributes(m_hWnd, 0, m_nLayeredWindowOpacity, LWA_ALPHA) != FALSE;
            ASSERT_UNUSED_VARIABLE(bRet);
        }
    }
    else {
        //必须先设置为分层窗口，然后才能设置成功
        SetLayeredWindow(true, false);
        bool bRet = ::SetLayeredWindowAttributes(m_hWnd, 0, m_nLayeredWindowOpacity, LWA_ALPHA) != FALSE;
        ASSERT_UNUSED_VARIABLE(bRet);
    }
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
    ASSERT((pParentWindow != nullptr) && (pParentWindow->GetHWND() != nullptr));
    if (pParentWindow != nullptr) {
        auto hOwnerWnd = GetWindowOwner();
        ASSERT(::IsWindow(hOwnerWnd));
        ASSERT_UNUSED_VARIABLE(hOwnerWnd == pParentWindow->GetHWND());
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(false);
        }
    }
    ShowWindow(kSW_SHOW_NORMAL);
    m_bFakeModal = true;
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
    if (IsWindowFocused()) {
        SetOwnerWindowFocus();
    }
}

bool NativeWindow_SDL::IsFakeModal() const
{
    return m_bFakeModal;
}

bool NativeWindow_SDL::IsDoModal() const
{
    return m_bDoModal;
}

void NativeWindow_SDL::CenterWindow()
{
    //ASSERT(IsWindow());
    //ASSERT((::GetWindowLong(m_hWnd, GWL_STYLE) & WS_CHILD) == 0);
    //UiRect rcDlg;
    //GetWindowRect(rcDlg);
    //UiRect rcArea;
    //UiRect rcCenter;
    //HWND hWnd = GetHWND();
    //HWND hWndCenter = GetWindowOwner();
    //if (hWndCenter != nullptr) {
    //    hWnd = hWndCenter;
    //}

    //// 处理多显示器模式下屏幕居中
    //UiRect rcMonitor;
    //GetMonitorRect(hWnd, rcMonitor, rcArea);
    //if (hWndCenter == nullptr) {
    //    rcCenter = rcArea;
    //}
    //else if (::IsIconic(hWndCenter)) {
    //    rcCenter = rcArea;
    //}
    //else {
    //    GetWindowRect(hWndCenter, rcCenter);
    //}

    //int DlgWidth = rcDlg.right - rcDlg.left;
    //int DlgHeight = rcDlg.bottom - rcDlg.top;

    //// Find dialog's upper left based on rcCenter
    //int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    //int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    //// The dialog is outside the screen, move it inside
    //if (xLeft < rcArea.left) {
    //    xLeft = rcArea.left;
    //}
    //else if (xLeft + DlgWidth > rcArea.right) {
    //    xLeft = rcArea.right - DlgWidth;
    //}
    //if (yTop < rcArea.top) {
    //    yTop = rcArea.top;
    //}
    //else if (yTop + DlgHeight > rcArea.bottom) {
    //    yTop = rcArea.bottom - DlgHeight;
    //}
    //::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void NativeWindow_SDL::ToTopMost()
{
    ASSERT(IsWindow());
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void NativeWindow_SDL::BringToTop()
{
    ASSERT(IsWindow());
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

bool NativeWindow_SDL::SetWindowForeground()
{
    ASSERT(IsWindow());
    if (::GetForegroundWindow() != m_hWnd) {
        ::SetForegroundWindow(m_hWnd);
    }
    return ::GetForegroundWindow() == m_hWnd;
}

bool NativeWindow_SDL::IsWindowForeground() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetForegroundWindow());
}

bool NativeWindow_SDL::SetWindowFocus()
{
    ASSERT(IsWindow());
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
    return ::GetFocus() == m_hWnd;
}

bool NativeWindow_SDL::KillWindowFocus()
{
    ASSERT(IsWindow());
    if (::GetFocus() == m_hWnd) {
        ::SetFocus(nullptr);
    }
    return ::GetFocus() != m_hWnd;
}

bool NativeWindow_SDL::IsWindowFocused() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetFocus());
}

bool NativeWindow_SDL::SetOwnerWindowFocus()
{
    HWND hwndParent = GetWindowOwner();
    if (hwndParent != nullptr) {
        ::SetFocus(hwndParent);
        return ::GetFocus() == hwndParent;
    }
    return false;
}

void NativeWindow_SDL::CheckSetWindowFocus()
{
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
}

LRESULT NativeWindow_SDL::SendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(IsWindow());
    return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT NativeWindow_SDL::PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(IsWindow());
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void NativeWindow_SDL::PostQuitMsg(int32_t nExitCode)
{
    ::PostQuitMessage(nExitCode);
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

bool NativeWindow_SDL::EnableWindow(bool bEnable)
{
    return ::EnableWindow(m_hWnd, bEnable ? TRUE : false) != FALSE;
}

bool NativeWindow_SDL::IsWindowEnabled() const
{
    return ::IsWindow(m_hWnd) && ::IsWindowEnabled(m_hWnd) != FALSE;
}

bool NativeWindow_SDL::IsWindowVisible() const
{
    return ::IsWindow(m_hWnd) && ::IsWindowVisible(m_hWnd) != FALSE;
}

bool NativeWindow_SDL::SetWindowPos(const NativeWindow_SDL* pInsertAfterWindow,
                                InsertAfterFlag insertAfterFlag,
                                int32_t X, int32_t Y, int32_t cx, int32_t cy,
                                uint32_t uFlags)
{
    ASSERT(IsWindow());
    HWND hWndInsertAfter = HWND_TOP;
    if (!(uFlags & kSWP_NOZORDER)) {
        if (pInsertAfterWindow != nullptr) {
            hWndInsertAfter = pInsertAfterWindow->GetHWND();
        }
        else {
            hWndInsertAfter = (HWND)insertAfterFlag;
        }
    }
    return ::SetWindowPos(m_hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags) != FALSE;
}

bool NativeWindow_SDL::MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint)
{
    ASSERT(IsWindow());
    return ::MoveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint ? TRUE : FALSE) != FALSE;
}

bool NativeWindow_SDL::SetWindowIcon(const FilePath& iconFilePath)
{
    ASSERT(IsWindow());
    if (!::IsWindow(m_hWnd)) {
        return false;
    }

    return true;
}

bool NativeWindow_SDL::SetWindowIcon(const std::vector<uint8_t>& iconFileData)
{

    return true;
}

void NativeWindow_SDL::SetText(const DString& strText)
{
    ASSERT(IsWindow());
#ifdef DUILIB_UNICODE
    ::SetWindowText(m_hWnd, strText.c_str());
#else
    //strText是UTF-8编码
    DString localText = StringUtil::TToLocal(strText);
    ::SetWindowText(m_hWnd, localText.c_str());
#endif
}

void NativeWindow_SDL::SetCapture()
{
    ::SetCapture(m_hWnd);
    m_bMouseCapture = true;
}

void NativeWindow_SDL::ReleaseCapture()
{
    if (m_bMouseCapture) {
        ::ReleaseCapture();
        m_bMouseCapture = false;
    }
}

bool NativeWindow_SDL::IsCaptured() const
{
    return m_bMouseCapture;
}

bool NativeWindow_SDL::SetWindowRoundRectRgn(const UiRect& rcWnd, const UiSize& szRoundCorner, bool bRedraw)
{
    ASSERT((szRoundCorner.cx > 0) && (szRoundCorner.cy > 0));
    if ((szRoundCorner.cx <= 0) || (szRoundCorner.cy <= 0)) {
        return false;
    }
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
    int nRet = ::SetWindowRgn(GetHWND(), hRgn, bRedraw ? TRUE : FALSE);
    ::DeleteObject(hRgn);//TODO: 检查是否需要删除，按MSDN说法，是不需要删除的。
    return nRet != 0;
}

void NativeWindow_SDL::ClearWindowRgn(bool bRedraw)
{
    ASSERT(IsWindow());
    ::SetWindowRgn(GetHWND(), nullptr, bRedraw ? TRUE : FALSE);
}

void NativeWindow_SDL::Invalidate(const UiRect& rcItem)
{
    RECT rc = { rcItem.left, rcItem.top, rcItem.right, rcItem.bottom };
    ::InvalidateRect(m_hWnd, &rc, FALSE);
    // Invalidating a layered window will not trigger a WM_PAINT message,
    // thus we have to post WM_PAINT by ourselves.
    if (IsLayeredWindow()) {
        ::PostMessage(m_hWnd, WM_PAINT, 0, 0);
    }
}

bool NativeWindow_SDL::UpdateWindow() const
{
    bool bRet = false;
    if ((m_hWnd != nullptr) && ::IsWindow(m_hWnd)) {
        bRet = ::UpdateWindow(m_hWnd) != FALSE;
    }
    return bRet;
}

LRESULT NativeWindow_SDL::OnPaintMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    //回调准备绘制函数
    LRESULT lResult = 0;
    bHandled = false;
    bool bPaint = m_pOwner->OnNativePreparePaint();
    RECT rectUpdate = { 0, };
    if (!::GetUpdateRect(m_hWnd, &rectUpdate, FALSE)) {
        bPaint = false;
    }
    if (bPaint) {
        IRender* pRender = m_pOwner->OnNativeGetRender();
        ASSERT(pRender != nullptr);
        if (pRender != nullptr) {
            NativeWindowRenderPaint renderPaint;
            renderPaint.m_pNativeWindow = this;
            renderPaint.m_pOwner = m_pOwner;
            renderPaint.m_nativeMsg = NativeMsg(uMsg, wParam, lParam);
            renderPaint.m_bHandled = bHandled;
            bPaint = pRender->PaintAndSwapBuffers(&renderPaint);
            bHandled = renderPaint.m_bHandled;
        }
    }
    if (!bPaint) {
        PAINTSTRUCT ps = { 0, };
        ::BeginPaint(m_hWnd, &ps);
        ::EndPaint(m_hWnd, &ps);
    }
    return lResult;
}

void NativeWindow_SDL::KeepParentActive()
{
    HWND hWndParent = GetHWND();
    if (::IsWindow(hWndParent)) {
        while (::GetParent(hWndParent) != NULL) {
            hWndParent = ::GetParent(hWndParent);
        }
    }
    if (::IsWindow(hWndParent)) {
        ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
    }
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

    int nXPos = 0;
    int nYPos = 0;
    int nRet = SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        rcWindow.left = nXPos;
        rcWindow.top = nYPos;
    }

    int nWidth = 0;
    int nHeight = 0;
    nRet = SDL_GetWindowSize(m_sdlWindow, &nWidth, &nHeight);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        rcWindow.right = rcWindow.left + nWidth;
        rcWindow.bottom = rcWindow.top + nHeight;
    }
}

void NativeWindow_SDL::ScreenToClient(UiPoint& pt) const
{
    int nXPos = 0;
    int nYPos = 0;
    int nRet = SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        pt.x -= nXPos;
        pt.y -= nYPos;
    }
}

void NativeWindow_SDL::ClientToScreen(UiPoint& pt) const
{
    int nXPos = 0;
    int nYPos = 0;
    int nRet = SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        pt.x += nXPos;
        pt.y += nYPos;
    }
}

void NativeWindow_SDL::GetCursorPos(UiPoint& pt) const
{
    POINT ptPos;
    ::GetCursorPos(&ptPos);
    pt = { ptPos.x, ptPos.y };
}

void NativeWindow_SDL::MapWindowDesktopRect(UiRect& rc) const
{
    ASSERT(IsWindow());
    HWND hwndFrom = GetHWND();
    HWND hwndTo = HWND_DESKTOP;
    POINT pts[2];
    pts[0].x = rc.left;
    pts[0].y = rc.top;
    pts[1].x = rc.right;
    pts[1].y = rc.bottom;
    ::MapWindowPoints((hwndFrom), (hwndTo), &pts[0], 2);
    rc.left = pts[0].x;
    rc.top = pts[0].y;
    rc.right = pts[1].x;
    rc.bottom = pts[1].y;
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

bool NativeWindow_SDL::GetModifiers(UINT message, WPARAM wParam, LPARAM lParam, uint32_t& modifierKey) const
{
    //逻辑修改，需要同步给函数：Window::IsKeyDown
    bool bRet = true;
    modifierKey = ModifierKey::kNone;
    switch (message) {
    case WM_CHAR:
        if (0 == (lParam & (1 << 30))) {
            modifierKey |= ModifierKey::kFirstPress;
        }
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (0 == (lParam & (1 << 30))) {
            modifierKey |= ModifierKey::kFirstPress;
        }
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;
    case WM_MOUSEWHEEL:
        {
            WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
            if (fwKeys & MK_CONTROL) {
                modifierKey |= ModifierKey::kControl;
            }
            if (fwKeys & MK_SHIFT) {
                modifierKey |= ModifierKey::kShift;
            }
        }
        break;
    case WM_MOUSEHOVER:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        if (wParam & MK_CONTROL) {
            modifierKey |= ModifierKey::kControl;
        }
        if (wParam & MK_SHIFT) {
            modifierKey |= ModifierKey::kShift;
        }
        break;
    case WM_HOTKEY:
        {
            uint16_t nMod = (uint16_t)LOWORD(lParam);
            if (nMod & MOD_ALT) {
                modifierKey |= ModifierKey::kAlt;
            }
            else if (nMod & MOD_CONTROL) {
                modifierKey |= ModifierKey::kControl;
            }
            else if (nMod & MOD_SHIFT) {
                modifierKey |= ModifierKey::kShift;
            }
            else if (nMod & MOD_WIN) {
                modifierKey |= ModifierKey::kWin;
            }
        }
        break;
    default:
        bRet = false;
        break;
    }
    ASSERT(bRet);
    return bRet;
}

INativeWindow* NativeWindow_SDL::WindowBaseFromPoint(const UiPoint& pt)
{
    NativeWindow_SDL* pWindow = nullptr;
    HWND hWnd = ::WindowFromPoint({ pt.x, pt.y });
    if (::IsWindow(hWnd)) {
        if (hWnd == m_hWnd) {
            pWindow = this;
        }
        else {
            pWindow = reinterpret_cast<NativeWindow_SDL*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if ((pWindow != nullptr) && (pWindow->m_hWnd != hWnd)) {
                pWindow = nullptr;
            }
        }
    }
    INativeWindow* pNativeWindow = nullptr;
    if (pWindow != nullptr) {
        pNativeWindow = pWindow->m_pOwner;
    }
    return pNativeWindow;
}


int32_t NativeWindow_SDL::SetWindowHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    ASSERT(IsWindow());
    return (int32_t)::SendMessage(GetHWND(), WM_SETHOTKEY, MAKEWORD(wVirtualKeyCode, wModifiers), 0);
}

bool NativeWindow_SDL::GetWindowHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
{
    ASSERT(IsWindow());
    DWORD dw = (DWORD)::SendMessage(GetHWND(), HKM_GETHOTKEY, 0, 0L);
    wVirtualKeyCode = LOBYTE(LOWORD(dw));
    wModifiers = HIBYTE(LOWORD(dw));
    return dw != 0;
}

bool NativeWindow_SDL::RegisterHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers, int32_t id)
{
    ASSERT(IsWindow());
    if (wVirtualKeyCode != 0) {
        UINT fsModifiers = 0;
        if (wModifiers & HOTKEYF_ALT)     fsModifiers |= MOD_ALT;
        if (wModifiers & HOTKEYF_CONTROL) fsModifiers |= MOD_CONTROL;
        if (wModifiers & HOTKEYF_SHIFT)   fsModifiers |= MOD_SHIFT;
        if (wModifiers & HOTKEYF_EXT)     fsModifiers |= MOD_WIN;

#ifndef MOD_NOREPEAT
        if (::IsWindows7OrGreater()) {
            fsModifiers |= 0x4000;
        }
#else
        fsModifiers |= MOD_NOREPEAT;
#endif

        LRESULT lResult = ::RegisterHotKey(this->GetHWND(), id, fsModifiers, wVirtualKeyCode);
        ASSERT(lResult != 0);
        if (lResult != 0) {
            auto iter = std::find(m_hotKeyIds.begin(), m_hotKeyIds.end(), id);
            if (iter != m_hotKeyIds.end()) {
                m_hotKeyIds.erase(iter);
            }
            m_hotKeyIds.push_back(id);
            return true;
        }
    }
    return false;
}

bool NativeWindow_SDL::UnregisterHotKey(int32_t id)
{
    ASSERT(IsWindow());
    auto iter = std::find(m_hotKeyIds.begin(), m_hotKeyIds.end(), id);
    if (iter != m_hotKeyIds.end()) {
        m_hotKeyIds.erase(iter);
    }
    return ::UnregisterHotKey(GetHWND(), id);
}

/** 窗口句柄的属性名称
*/
static const wchar_t* sPropName = L"DuiLibWindow"; // 属性名称

LRESULT CALLBACK NativeWindow_SDL::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NativeWindow_SDL* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<NativeWindow_SDL*>(lpcs->lpCreateParams);
        if (pThis != nullptr) {
            pThis->m_hWnd = hWnd;
        }
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
        ::SetPropW(hWnd, sPropName, (HANDLE)pThis);
    }
    else {
        pThis = reinterpret_cast<NativeWindow_SDL*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
#ifdef _DEBUG
        //校验是否一致
        ASSERT((NativeWindow_SDL*)::GetPropW(hWnd, sPropName) == pThis);
#endif
        if (uMsg == WM_NCDESTROY && pThis != nullptr) {            
            LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
            ::SetPropW(hWnd, sPropName, NULL);
            ASSERT(hWnd == pThis->GetHWND());
            pThis->OnFinalMessage();
            return lRes;
        }
    }

    if (pThis != nullptr) {
        ASSERT(hWnd == pThis->GetHWND());
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    }
    else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}


INT_PTR CALLBACK NativeWindow_SDL::__DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG) {
        //这是对话框的第一个消息
        NativeWindow_SDL* pThis = reinterpret_cast<NativeWindow_SDL*>(lParam);
        ASSERT(pThis != nullptr);
        if (pThis != nullptr) {
            ASSERT(pThis->m_hWnd == nullptr);
            pThis->m_hWnd = hWnd;            
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
            ::SetPropW(hWnd, sPropName, (HANDLE)pThis);

            ASSERT(pThis->m_pfnOldWndProc == nullptr);

            //替换对话框的窗口处理函数
            pThis->m_pfnOldWndProc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
            ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LPARAM)(WNDPROC)NativeWindow_SDL::__DialogWndProc);

            //派发此消息
            pThis->WindowMessageProc(uMsg, wParam, lParam);
            ::SetFocus(hWnd);
            return TRUE;
        }
    }
    else if (uMsg == WM_COMMAND) {
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            NativeWindow_SDL* pThis = reinterpret_cast<NativeWindow_SDL*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
#ifdef _DEBUG
            //校验是否一致
            ASSERT((NativeWindow_SDL*)::GetPropW(hWnd, sPropName) == pThis);
#endif
            ASSERT(pThis != nullptr);
            if (pThis != nullptr) {
                if (pThis->m_bCloseByEsc && (LOWORD(wParam) == IDCANCEL)) {
                    pThis->CloseWnd(kWindowCloseCancel);
                }
                else if (pThis->m_bCloseByEnter && (LOWORD(wParam) == IDOK)) {
                    pThis->CloseWnd(kWindowCloseOK);
                }
            }
            return TRUE;
        }
    }    
    return FALSE;
}

LRESULT CALLBACK NativeWindow_SDL::__DialogWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NativeWindow_SDL* pThis = reinterpret_cast<NativeWindow_SDL*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
#ifdef _DEBUG
    //校验是否一致
    ASSERT((NativeWindow_SDL*)::GetPropW(hWnd, sPropName) == pThis);
#endif
    ASSERT(pThis != nullptr);
    if (uMsg == WM_NCDESTROY && pThis != nullptr) {
        if (pThis->m_pfnOldWndProc != nullptr) {
            ::SetWindowLongPtr((hWnd), GWLP_WNDPROC, (LPARAM)(WNDPROC)(pThis->m_pfnOldWndProc));
            pThis->m_pfnOldWndProc = nullptr;
        }
        LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
        ::SetPropW(hWnd, sPropName, NULL);
        ASSERT(hWnd == pThis->GetHWND());
        pThis->OnFinalMessage();
        return lRes;
    }
    else if (pThis != nullptr) {
        ASSERT(hWnd == pThis->GetHWND());
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    }
    return ::DefDlgProc(hWnd, uMsg, wParam, lParam);
}

LRESULT NativeWindow_SDL::WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    HWND hWnd = m_hWnd;
    INativeWindow* pOwner = m_pOwner;
    if (pOwner == nullptr) {
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
        return lResult;
    }
    //接口的生命周期标志
    std::weak_ptr<WeakFlag> ownerFlag = pOwner->GetWeakFlag();

    //消息首先转给过滤器(全部消息)
    bool bHandled = false;
    if (!bHandled && !ownerFlag.expired()) {
        lResult = pOwner->OnNativeWindowMessage(uMsg, wParam, lParam, bHandled);
    }

    if (m_bDoModal && (uMsg == WM_KEYDOWN) && (wParam == VK_ESCAPE)) {
        //模态对话框，按ESC键时，关闭
        CloseWnd(kWindowCloseCancel);
        return 0;
    }

    //第三优先级：内部处理的消息，处理后，不再派发
    if (!bHandled && !ownerFlag.expired()) {
        lResult = ProcessInternalMessage(uMsg, wParam, lParam, bHandled);
    }

    //第四优先级：内部处理函数，优先保证自身功能正常
    if (!bHandled && !ownerFlag.expired()) {
        lResult = ProcessWindowMessage(uMsg, wParam, lParam, bHandled);
    }

    bool bCloseMsg = false;
    if (!bHandled && !ownerFlag.expired()) {
        if ((uMsg == WM_CLOSE) || ((uMsg == WM_SYSCOMMAND) && (GET_SC_WPARAM(wParam) == SC_CLOSE))) {
            //窗口即将关闭（关闭前）
            StopSysMenuTimer();

            bCloseMsg = true;
            pOwner->OnNativePreCloseWindow();
        }
    }
    if (uMsg == WM_CLOSE) {
        StopSysMenuTimer();

        m_closeParam = (int32_t)wParam;
    }

    //第五优先级：系统默认的窗口函数
    if (!bHandled && !ownerFlag.expired() && ::IsWindow(hWnd)) {
        if (bCloseMsg && m_bDoModal) {
            //模态对话框
            ::EndDialog(hWnd, wParam);
            lResult = 0;
            bHandled = true;
        }
        else {
            lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
        }        
    }

    if (!bHandled && !ownerFlag.expired() && (uMsg == WM_DESTROY)) {
        //窗口已经关闭（关闭后）
        pOwner->OnNativePostCloseWindow();
    }

    return lResult;
}

LRESULT NativeWindow_SDL::CallDefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_pfnOldWndProc != nullptr) {
        return ::CallWindowProc(m_pfnOldWndProc, m_hWnd, uMsg, wParam, lParam);
    }
    return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

LRESULT NativeWindow_SDL::ProcessInternalMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    bool bInternalMsg = true;
    switch (uMsg)
    {
    case WM_NCACTIVATE:         lResult = OnNcActivateMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCCALCSIZE:         lResult = OnNcCalcSizeMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCHITTEST:          lResult = OnNcHitTestMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_GETMINMAXINFO:      lResult = OnGetMinMaxInfoMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_ERASEBKGND:         lResult = OnEraseBkGndMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_DPICHANGED:         lResult = OnDpiChangedMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_WINDOWPOSCHANGING:  lResult = OnWindowPosChangingMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_NOTIFY:             lResult = OnNotifyMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_COMMAND:            lResult = OnCommandMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLOREDIT:       lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLORSTATIC:     lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled); break;
    case WM_TOUCH:              lResult = OnTouchMsg(uMsg, wParam, lParam, bHandled); break;

        break;

    case WM_CREATE:     lResult = OnCreateMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_INITDIALOG: lResult = OnInitDialogMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_TIMER:
        {
            if (wParam == m_nSysMenuTimerId) {
                //系统菜单延迟显示的定时器触发
                ::KillTimer(m_hWnd, m_nSysMenuTimerId);
                m_nSysMenuTimerId = 0;

                POINT pt;
                ::GetCursorPos(&pt);
                ShowWindowSysMenu(m_hWnd, pt);
                bHandled = true;
            }
        }
        break;
    default:
        bInternalMsg = false;
        break;
    }//end of switch

    if (bInternalMsg && !bHandled) {
        //调用窗口函数，然后不再继续传递此消息
        bHandled = true;
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
    }
    return lResult;
}

LRESULT NativeWindow_SDL::OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCACTIVATE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    LRESULT lResult = 0;
    if (IsWindowMinimized()) {
        bHandled = false;
    }
    else {
        //MSDN: wParam 参数为 FALSE 时，应用程序应返回 TRUE 以指示系统应继续执行默认处理
        bHandled = true;
        lResult = (wParam == FALSE) ? TRUE : FALSE;
    }
    return lResult;
}

LRESULT NativeWindow_SDL::OnNcCalcSizeMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCCALCSIZE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    //截获，让系统不处理此消息
    bHandled = true;
    return 0;
}

LRESULT NativeWindow_SDL::OnNcHitTestMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCHITTEST);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    bHandled = true;
    UiPoint pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    ScreenToClient(pt);

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
                    return HTTOPLEFT;//在窗口边框的左上角。
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTTOPRIGHT;//在窗口边框的右上角
                }
                else {
                    return HTTOP;//在窗口的上水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
            if (pt.y <= rcClient.bottom) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTBOTTOMLEFT;//在窗口边框的左下角
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTBOTTOMRIGHT;//在窗口边框的右下角
                }
                else {
                    return HTBOTTOM;//在窗口的下水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }

        if (pt.x < rcClient.left + rcSizeBox.left) {
            if (pt.x >= rcClient.left) {
                return HTLEFT;//在窗口的左边框
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        if (pt.x > rcClient.right - rcSizeBox.right) {
            if (pt.x <= rcClient.right) {
                return HTRIGHT;//在窗口的右边框中
            }
            else {
                return HTCLIENT;//在工作区中
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
        //在标题栏范围内
        UiRect sysMenuRect;
        m_pOwner->OnNativeGetSysMenuRect(sysMenuRect);
        sysMenuRect.Offset(rcClient.left, rcClient.top);
        sysMenuRect.Intersect(rcCaptionRect);
        if (!sysMenuRect.IsEmpty()) {
            if (sysMenuRect.ContainsPt(pt)) {                
                return HTSYSMENU;//在系统菜单矩形区域内
            }
        }

        //是否支持显示贴靠布局菜单
        bool bPtInMaximizeRestoreButton = false;        
        if (IsEnableSnapLayoutMenu()) {
            bPtInMaximizeRestoreButton = m_pOwner->OnNativeIsPtInMaximizeRestoreButton(pt);
        }
        if (bPtInMaximizeRestoreButton) {
            return HTMAXBUTTON; //在最大化按钮或者还原按钮上，显示贴靠布局菜单
        }
        else if (m_pOwner->OnNativeIsPtInCaptionBarControl(pt)) {
            return HTCLIENT;//在工作区中（放在标题栏上的控件，视为工作区）
        }
        else {
            return HTCAPTION;//在标题栏中
        }
    }
    //其他，在工作区中
    return HTCLIENT;
}

LRESULT NativeWindow_SDL::OnGetMinMaxInfoMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_GETMINMAXINFO);
    bHandled = false;
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
    //UiRect rcWork;
    //UiRect rcMonitor;
    //GetMonitorRect(m_hWnd, rcMonitor, rcWork);
    //rcWork.Offset(-rcMonitor.left, -rcMonitor.top);

    ////最大化时，默认设置为当前屏幕的最大区域
    //lpMMI->ptMaxPosition.x = rcWork.left;
    //lpMMI->ptMaxPosition.y = rcWork.top;
    //lpMMI->ptMaxSize.x = rcWork.Width();
    //lpMMI->ptMaxSize.y = rcWork.Height();

    //if (m_pOwner->OnNativeGetMaxInfo(true).cx != 0) {
    //    lpMMI->ptMaxTrackSize.x = m_pOwner->OnNativeGetMaxInfo(true).cx;
    //}
    //if (m_pOwner->OnNativeGetMaxInfo(true).cy != 0) {
    //    lpMMI->ptMaxTrackSize.y = m_pOwner->OnNativeGetMaxInfo(true).cy;
    //}
    //if (m_pOwner->OnNativeGetMinInfo(true).cx != 0) {
    //    lpMMI->ptMinTrackSize.x = m_pOwner->OnNativeGetMinInfo(true).cx;
    //}
    //if (m_pOwner->OnNativeGetMinInfo(true).cy != 0) {
    //    lpMMI->ptMinTrackSize.y = m_pOwner->OnNativeGetMinInfo(true).cy;
    //}
    return 0;
}

LRESULT NativeWindow_SDL::OnEraseBkGndMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_ERASEBKGND);
    bHandled = true;
    return 1;
}

LRESULT NativeWindow_SDL::OnDpiChangedMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_DPICHANGED);
    bHandled = false;//需要重新测试

    uint32_t nNewDPI = HIWORD(wParam);
    UiRect rcNewWindow;
    const RECT* prcNewWindow = (RECT*)lParam;
    if (prcNewWindow != nullptr) {
        rcNewWindow.left = prcNewWindow->left;
        rcNewWindow.top = prcNewWindow->top;
        rcNewWindow.right = prcNewWindow->right;
        rcNewWindow.bottom = prcNewWindow->bottom;
    }
    uint32_t nOldDpiScale = m_pOwner->OnNativeGetDpi().GetScale();
    m_pOwner->OnNativeProcessDpiChangedMsg(nNewDPI, rcNewWindow);
    if (nOldDpiScale != m_pOwner->OnNativeGetDpi().GetScale()) {
        m_ptLastMousePos = m_pOwner->OnNativeGetDpi().GetScalePoint(m_ptLastMousePos, nOldDpiScale);
    }
    //更新窗口的位置和大小
    if (!rcNewWindow.IsEmpty()) {
        SetWindowPos(nullptr, InsertAfterFlag::kHWND_DEFAULT,
                     rcNewWindow.left, rcNewWindow.top, rcNewWindow.Width(), rcNewWindow.Height(),
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    return 0;
}

LRESULT NativeWindow_SDL::OnWindowPosChangingMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_WINDOWPOSCHANGING);
    bHandled = false;
    if (IsWindowMaximized()) {
        //最大化状态
        LPWINDOWPOS lpPos = (LPWINDOWPOS)lParam;
        if (lpPos->flags & SWP_FRAMECHANGED) // 第一次最大化，而不是最大化之后所触发的WINDOWPOSCHANGE
        {
            POINT pt = { 0, 0 };
            HMONITOR hMontorPrimary = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
            HMONITOR hMonitorTo = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);

            // 先把窗口最大化，再最小化，然后恢复，此时MonitorFromWindow拿到的HMONITOR不准确
            // 判断GetWindowRect的位置如果不正确（最小化时得到的位置信息是-38000），则改用normal状态下的位置，来获取HMONITOR
            UiRect rc;
            GetWindowRect(rc);
            if (rc.left < -10000 && rc.top < -10000 && rc.bottom < -10000 && rc.right < -10000) {
                WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
                ::GetWindowPlacement(m_hWnd, &wp);
                hMonitorTo = ::MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTOPRIMARY);
            }
            if (hMonitorTo != hMontorPrimary) {
                // 解决无边框窗口在双屏下面（副屏分辨率大于主屏）时，最大化不正确的问题
                MONITORINFO  miTo = { sizeof(miTo), 0 };
                ::GetMonitorInfo(hMonitorTo, &miTo);

                lpPos->x = miTo.rcWork.left;
                lpPos->y = miTo.rcWork.top;
                lpPos->cx = miTo.rcWork.right - miTo.rcWork.left;
                lpPos->cy = miTo.rcWork.bottom - miTo.rcWork.top;
            }
        }
    }
    return 0;
}

LRESULT NativeWindow_SDL::OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NOTIFY);
    bHandled = false;
    LPNMHDR lpNMHDR = (LPNMHDR)lParam;
    if (lpNMHDR != nullptr) {
        bHandled = true;
        return ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT NativeWindow_SDL::OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_COMMAND);
    bHandled = false;
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT NativeWindow_SDL::OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_CTLCOLOREDIT || uMsg == WM_CTLCOLORSTATIC);
    bHandled = false;
    // Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
    // Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT NativeWindow_SDL::OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_TOUCH);
    LRESULT lResult = 0;
    bHandled = false;
 
    return lResult;
}

LRESULT NativeWindow_SDL::OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{

    return 0;
}

LRESULT NativeWindow_SDL::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    switch (uMsg)
    {
    case WM_SIZE:
    {
        WindowSizeType sizeType = static_cast<WindowSizeType>(wParam);
        UiSize newWindowSize;
        newWindowSize.cx = (int)(short)LOWORD(lParam);
        newWindowSize.cy = (int)(short)HIWORD(lParam);
        lResult = m_pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOVE:
    {
        UiPoint ptTopLeft;
        ptTopLeft.x = (int)(short)LOWORD(lParam);   // horizontal position 
        ptTopLeft.y = (int)(short)HIWORD(lParam);   // vertical position 
        lResult = m_pOwner->OnNativeMoveMsg(ptTopLeft, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_SHOWWINDOW:
    {
        bool bShow = wParam != FALSE;
        lResult = m_pOwner->OnNativeShowWindowMsg(bShow, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_PAINT:
    {
        lResult = OnPaintMsg(uMsg, wParam, lParam, bHandled);
        break;
    }
    case WM_SETFOCUS:
    {
        INativeWindow* pLostFocusWindow = nullptr;
        HWND hWnd = (HWND)wParam;
        if ((hWnd != nullptr) && ::IsWindow(hWnd)) {
            NativeWindow_SDL* pThis = reinterpret_cast<NativeWindow_SDL*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            //校验是否一致
            if (pThis != nullptr) {
                if ((NativeWindow_SDL*)::GetPropW(hWnd, sPropName) == pThis) {
                    pLostFocusWindow = pThis->m_pOwner;
                }
            }
        }
        lResult = m_pOwner->OnNativeSetFocusMsg(pLostFocusWindow, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KILLFOCUS:
    {
        INativeWindow* pSetFocusWindow = nullptr;
        HWND hWnd = (HWND)wParam;
        if ((hWnd != nullptr) && ::IsWindow(hWnd)) {
            NativeWindow_SDL* pThis = reinterpret_cast<NativeWindow_SDL*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            //校验是否一致
            if (pThis != nullptr) {
                if ((NativeWindow_SDL*)::GetPropW(hWnd, sPropName) == pThis) {
                    pSetFocusWindow = pThis->m_pOwner;
                }
            }
        }
        lResult = m_pOwner->OnNativeKillFocusMsg(pSetFocusWindow, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_IME_STARTCOMPOSITION:
    {
        lResult = m_pOwner->OnNativeImeStartCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_IME_ENDCOMPOSITION:
    {
        lResult = m_pOwner->OnNativeImeEndCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT) {
            //只处理设置客户区的光标
            lResult = m_pOwner->OnNativeSetCursorMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        break;
    }
    case WM_CONTEXTMENU:
    {
        UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if ((pt.x != -1) && (pt.y != -1)) {
            ScreenToClient(pt);
        }
        lResult = m_pOwner->OnNativeContextMenuMsg(pt, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeKeyDownMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeKeyUpMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_CHAR:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeCharMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_HOTKEY:
    {
        int32_t hotkeyId = (int32_t)wParam;
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>((int32_t)(int16_t)HIWORD(lParam));
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeHotKeyMsg(hotkeyId, vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEWHEEL:
    {
        int32_t wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(pt);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEMOVE:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseMoveMsg(pt, modifierKey, false, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEHOVER:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseHoverMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSELEAVE:
    {
        lResult = m_pOwner->OnNativeMouseLeaveMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONUP:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONDBLCLK:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonDbClickMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONDOWN:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONUP:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONDBLCLK:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonDbClickMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_CAPTURECHANGED:
    {
        lResult = m_pOwner->OnNativeCaptureChangedMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_CLOSE:
    {
        lResult = m_pOwner->OnNativeWindowCloseMsg((uint32_t)wParam, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_NCMOUSEMOVE:
    {
        if (!IsUseSystemCaption()) {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(pt);
            uint32_t modifierKey = 0;
            lResult = m_pOwner->OnNativeMouseMoveMsg(pt, modifierKey, true, NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        break;
    }
    case WM_NCLBUTTONDOWN:
    {
        if (!IsUseSystemCaption() && (wParam == HTMAXBUTTON)) {
            //如果鼠标点击在最大化按钮上，截获此消息，避免Windows也触发最大化/还原命令
            bHandled = true; 
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(pt);
            uint32_t modifierKey = 0;
            lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        else if (!IsUseSystemCaption() && (wParam == HTSYSMENU) && IsEnableSysMenu()) {
            //鼠标点击在窗口菜单位置，启动定时器，延迟显示系统的窗口菜单
            StopSysMenuTimer();            
            m_nSysMenuTimerId = ::SetTimer(m_hWnd, UI_SYS_MEMU_TIMER_ID, 300, NULL);
        }
        break;
    }
    case WM_NCLBUTTONUP:
    {
        if (!IsUseSystemCaption() && (wParam == HTMAXBUTTON)) {
            //如果鼠标点击在最大化按钮上，截获此消息，避免Windows也触发最大化/还原命令
            bHandled = true;

            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(pt);
            uint32_t modifierKey = 0;
            lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        break;
    }
    case WM_NCRBUTTONUP:
    {
        bool bEnable = (wParam == HTCAPTION) || (wParam == HTMAXBUTTON) || (wParam == HTSYSMENU);
        if (bEnable && IsEnableSysMenu() && !IsUseSystemCaption()) {
            // 显示系统菜单
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if (ShowWindowSysMenu(m_hWnd, pt)) {
                bHandled = true;
            }
        }
        break;
    }
    default:
        break;
    }//end of switch
    return lResult;
}

void NativeWindow_SDL::StopSysMenuTimer()
{
    if (m_nSysMenuTimerId != 0) {
        ::KillTimer(m_hWnd, m_nSysMenuTimerId);
        m_nSysMenuTimerId = 0;
    }
}

bool NativeWindow_SDL::ShowWindowSysMenu(HWND hWnd, const POINT& pt) const
{
    HMENU hSysMenu = ::GetSystemMenu(hWnd, FALSE);
    if (hSysMenu == nullptr) {
        return false;
    }
    //更新菜单状态
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE;
    mii.fType = 0;

    // update the options
    mii.fState = MF_ENABLED;
    SetMenuItemInfo(hSysMenu, SC_RESTORE, FALSE, &mii);
    SetMenuItemInfo(hSysMenu, SC_SIZE, FALSE, &mii);
    SetMenuItemInfo(hSysMenu, SC_MOVE, FALSE, &mii);
    SetMenuItemInfo(hSysMenu, SC_MAXIMIZE, FALSE, &mii);
    SetMenuItemInfo(hSysMenu, SC_MINIMIZE, FALSE, &mii);

    mii.fState = MF_GRAYED;

    WINDOWPLACEMENT wp = { 0, };
    ::GetWindowPlacement(hWnd, &wp);

    switch (wp.showCmd)
    {
    case SW_SHOWMAXIMIZED:
        SetMenuItemInfo(hSysMenu, SC_SIZE, FALSE, &mii);
        SetMenuItemInfo(hSysMenu, SC_MOVE, FALSE, &mii);
        SetMenuItemInfo(hSysMenu, SC_MAXIMIZE, FALSE, &mii);
        SetMenuDefaultItem(hSysMenu, SC_CLOSE, FALSE);
        break;
    case SW_SHOWMINIMIZED:
        SetMenuItemInfo(hSysMenu, SC_MINIMIZE, FALSE, &mii);
        SetMenuDefaultItem(hSysMenu, SC_RESTORE, FALSE);
        break;
    case SW_SHOWNORMAL:
        SetMenuItemInfo(hSysMenu, SC_RESTORE, FALSE, &mii);
        SetMenuDefaultItem(hSysMenu, SC_CLOSE, FALSE);
        break;
    default:
        break;
    }

    // 在点击位置显示系统菜单
    int32_t nRet = ::TrackPopupMenu(hSysMenu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
    if (nRet != 0) {
        ::PostMessage(hWnd, WM_SYSCOMMAND, nRet, 0);
    }
    return true;
}

void NativeWindow_SDL::OnFinalMessage()
{
    if (m_pOwner) {
        m_pOwner->OnNativeFinalMessage();
    }
}

HWND NativeWindow_SDL::GetWindowOwner() const
{
    return ::GetWindow(m_hWnd, GW_OWNER);
}

void NativeWindow_SDL::SetEnableSnapLayoutMenu(bool bEnable)
{
    //仅Windows11才支持
    /*if (UiIsWindows11OrGreater()) {
        m_bSnapLayoutMenu = bEnable;
    }*/
}

bool NativeWindow_SDL::IsEnableSnapLayoutMenu() const
{
    return m_bSnapLayoutMenu;
}

void NativeWindow_SDL::SetEnableSysMenu(bool bEnable)
{
    m_bEnableSysMenu = bEnable;
}

bool NativeWindow_SDL::IsEnableSysMenu() const
{
    return m_bEnableSysMenu;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
