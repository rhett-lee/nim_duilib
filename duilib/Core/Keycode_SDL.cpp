#include "Keycode.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

namespace ui
{

VirtualKeyCode Keycode::GetVirtualKeyCode(SDL_Keycode sdlKeycode)
{
    //参考：https://wiki.libsdl.org/SDL2/SDL_ScancodeAndKeycode
    VirtualKeyCode vkCode = VirtualKeyCode::kVK_None;
    switch (sdlKeycode)
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

SDL_Keycode Keycode::GetSDLKeyCode(VirtualKeyCode virtualKeycode)
{
    SDL_Keycode vkCode = SDLK_UNKNOWN;
    switch (virtualKeycode)
    {
    case VirtualKeyCode::kVK_None:          // 0x00000000u /* 0 */
        vkCode = SDLK_UNKNOWN;  
        break;
    case VirtualKeyCode::kVK_RETURN:        // 0x0000000du /* '\r' */
        vkCode = SDLK_RETURN;
        break;
    case VirtualKeyCode::kVK_ESCAPE:        // 0x0000001bu /* '\x1B' */
        vkCode = SDLK_ESCAPE;
        break;
    case VirtualKeyCode::kVK_BACK:          // 0x00000008u /* '\b' */
        vkCode = SDLK_BACKSPACE;
        break;
    case VirtualKeyCode::kVK_TAB:           // 0x00000009u /* '\t' */
        vkCode = SDLK_TAB;
        break;
    case VirtualKeyCode::kVK_SPACE:         // 0x00000020u /* ' ' */
        vkCode = SDLK_SPACE;
        break;
    //case VirtualKeyCode::kVK_1:             // 0x00000021u /* '!' */
    //    //转回
    //    vkCode = SDLK_EXCLAIM;
    //    break;
    //case VirtualKeyCode::kVK_OEM_7:       // 0x00000022u /* '"' */
    //    //转回
    //    vkCode = SDLK_DBLAPOSTROPHE;
    //    break;
    //case VirtualKeyCode::kVK_3:             // 0x00000023u /* '#' */
    //    //转回
    //    vkCode = SDLK_HASH;
    //    break;
    //case VirtualKeyCode::kVK_4:             // 0x00000024u /* '$' */
    //    //转回
    //    vkCode = SDLK_DOLLAR;
    //    break;
    //case VirtualKeyCode::kVK_5:             // 0x00000025u /* '%' */
    //    //转回
    //    vkCode = SDLK_PERCENT;
    //    break;
    //case VirtualKeyCode::kVK_7:             // 0x00000026u /* '&' */
    //    //转回
    //    vkCode = SDLK_AMPERSAND;
    //    break;
    case VirtualKeyCode::kVK_OEM_7:         // 0x00000027u /* '\'' */
        //转回
        vkCode = SDLK_APOSTROPHE;
        break;
    //case VirtualKeyCode::kVK_9:             // 0x00000028u /* '(' */
    //    //转回
    //    vkCode = SDLK_LEFTPAREN;
    //    break;
    //case VirtualKeyCode::kVK_0:             // 0x00000029u /* ')' */
    //    //转回
    //    vkCode = SDLK_RIGHTPAREN;
    //    break;
    //case VirtualKeyCode::kVK_8:             // 0x0000002au /* '*' */
    //    //转回
    //    vkCode = SDLK_ASTERISK;
    //    break;
    //case VirtualKeyCode::kVK_OEM_PLUS:      // 0x0000002bu /* '+' */
    //    vkCode = SDLK_PLUS;
    //    break;
    case VirtualKeyCode::kVK_OEM_COMMA:     // 0x0000002cu /* ',' */
        vkCode = SDLK_COMMA;
        break;
    //case VirtualKeyCode::kVK_OEM_MINUS:     // 0x0000002du /* '-' */
    //    vkCode = SDLK_MINUS;
    //    break;
    case VirtualKeyCode::kVK_OEM_PERIOD:    // 0x0000002eu /* '.' */
        vkCode = SDLK_PERIOD;
        break;
    case VirtualKeyCode::kVK_OEM_2:         // 0x0000002fu /* '/' */
        vkCode = SDLK_SLASH;
        break;
    case VirtualKeyCode::kVK_0:             // 0x00000030u /* '0' */
        vkCode = SDLK_0;
        break;
    case VirtualKeyCode::kVK_1:             // 0x00000031u /* '1' */
        vkCode = SDLK_1;
        break;
    case VirtualKeyCode::kVK_2:             // 0x00000032u /* '2' */
        vkCode = SDLK_2;
        break;
    case VirtualKeyCode::kVK_3:             // 0x00000033u /* '3' */
        vkCode = SDLK_3;
        break;
    case VirtualKeyCode::kVK_4:             // 0x00000034u /* '4' */
        vkCode = SDLK_4;
        break;
    case VirtualKeyCode::kVK_5:             // 0x00000035u /* '5' */
        vkCode = SDLK_5;
        break;
    case VirtualKeyCode::kVK_6:             // 0x00000036u /* '6' */
        vkCode = SDLK_6;
        break;
    case VirtualKeyCode::kVK_7:             // 0x00000037u /* '7' */
        vkCode = SDLK_7;
        break;
    case VirtualKeyCode::kVK_8:             // 0x00000038u /* '8' */
        vkCode = SDLK_8;
        break;
    case VirtualKeyCode::kVK_9:             // 0x00000039u /* '9' */
        vkCode = SDLK_9;
        break;
    //case VirtualKeyCode::kVK_OEM_1:         // 0x0000003au /* ':' */
    //    //转回
    //    vkCode = SDLK_COLON;
    //    break;
    case VirtualKeyCode::kVK_OEM_1:         // 0x0000003bu /* ';' */
        vkCode = SDLK_SEMICOLON;
        break;
    //case VirtualKeyCode::kVK_OEM_COMMA:     // 0x0000003cu /* '<' */
    //    //转回
    //    vkCode = SDLK_LESS;
    //    break;
    case VirtualKeyCode::kVK_OEM_PLUS:      // 0x0000003du /* '=' */
        vkCode = SDLK_EQUALS;
        break;
    //case VirtualKeyCode::kVK_OEM_PERIOD:    // 0x0000003eu /* '>' */
    //    //转回
    //    vkCode = SDLK_GREATER;
    //    break;
    //case VirtualKeyCode::kVK_OEM_2:         // 0x0000003fu /* '?' */
    //    转回
    //    vkCode = SDLK_QUESTION;
    //    break;
    //case VirtualKeyCode::kVK_2:             // 0x00000040u /* '@' */
    //    //转回
    //    vkCode = SDLK_AT;
    //    break;
    case VirtualKeyCode::kVK_OEM_4:         // 0x0000005bu /* '[' */
        vkCode = SDLK_LEFTBRACKET;
        break;
    case VirtualKeyCode::kVK_OEM_5:         // 0x0000005cu /* '\\' */
        vkCode = SDLK_BACKSLASH;
        break;
    case VirtualKeyCode::kVK_OEM_6:         // 0x0000005du /* ']' */
        vkCode = SDLK_RIGHTBRACKET;
        break;
    //case VirtualKeyCode::kVK_6:             // 0x0000005eu /* '^' */
    //    //转回
    //    vkCode = SDLK_CARET;
    //    break;
    case VirtualKeyCode::kVK_OEM_MINUS:     // 0x0000005fu /* '_' */
        //转回S
        vkCode = SDLK_UNDERSCORE;
        break;
    case VirtualKeyCode::kVK_OEM_3:         // 0x00000060u /* '`' */
        vkCode = SDLK_GRAVE;
        break;
    case VirtualKeyCode::kVK_A:             // 0x00000061u /* 'a' */
        vkCode = SDLK_A;
        break;
    case VirtualKeyCode::kVK_B:             // 0x00000062u /* 'b' */
        vkCode = SDLK_B;
        break;
    case VirtualKeyCode::kVK_C:             // 0x00000063u /* 'c' */
        vkCode = SDLK_C;
        break;
    case VirtualKeyCode::kVK_D:             // 0x00000064u /* 'd' */
        vkCode = SDLK_D;
        break;
    case VirtualKeyCode::kVK_E:             // 0x00000065u /* 'e' */
        vkCode = SDLK_E;
        break;
    case VirtualKeyCode::kVK_F:             // 0x00000066u /* 'f' */
        vkCode = SDLK_F;
        break;
    case VirtualKeyCode::kVK_G:             // 0x00000067u /* 'g' */
        vkCode = SDLK_G;
        break;
    case VirtualKeyCode::kVK_H:             // 0x00000068u /* 'h' */
        vkCode = SDLK_H;
        break;
    case VirtualKeyCode::kVK_I:             // 0x00000069u /* 'i' */
        vkCode = SDLK_I;
        break;
    case VirtualKeyCode::kVK_J:             // 0x0000006au /* 'j' */
        vkCode = SDLK_J;
        break;
    case VirtualKeyCode::kVK_K:             // 0x0000006bu /* 'k' */
        vkCode = SDLK_K;
        break;
    case VirtualKeyCode::kVK_L:             // 0x0000006cu /* 'l' */
        vkCode = SDLK_L;
        break;
    case VirtualKeyCode::kVK_M:             // 0x0000006du /* 'm' */
        vkCode = SDLK_M;
        break;
    case VirtualKeyCode::kVK_N:             // 0x0000006eu /* 'n' */
        vkCode = SDLK_N;
        break;
    case VirtualKeyCode::kVK_O:             // 0x0000006fu /* 'o' */
        vkCode = SDLK_O;
        break;
    case VirtualKeyCode::kVK_P:             // 0x00000070u /* 'p' */
        vkCode = SDLK_P;
        break;
    case VirtualKeyCode::kVK_Q:             // 0x00000071u /* 'q' */
        vkCode = SDLK_Q;
        break;
    case VirtualKeyCode::kVK_R:             // 0x00000072u /* 'r' */
        vkCode = SDLK_R;
        break;
    case VirtualKeyCode::kVK_S:             // 0x00000073u /* 's' */
        vkCode = SDLK_S;
        break;
    case VirtualKeyCode::kVK_T:             // 0x00000074u /* 't' */
        vkCode = SDLK_T;
        break;
    case VirtualKeyCode::kVK_U:             // 0x00000075u /* 'u' */
        vkCode = SDLK_U;
        break;
    case VirtualKeyCode::kVK_V:             // 0x00000076u /* 'v' */
        vkCode = SDLK_V;
        break;
    case VirtualKeyCode::kVK_W:             // 0x00000077u /* 'w' */
        vkCode = SDLK_W;
        break;
    case VirtualKeyCode::kVK_X:             // 0x00000078u /* 'x' */
        vkCode = SDLK_X;
        break;
    case VirtualKeyCode::kVK_Y:             // 0x00000079u /* 'y' */
        vkCode = SDLK_Y;
        break;
    case VirtualKeyCode::kVK_Z:             // 0x0000007au /* 'z' */
        vkCode = SDLK_Z;
        break;
    //case VirtualKeyCode::kVK_OEM_4:         // 0x0000007bu /* '{' */
    //    //转回
    //    vkCode = SDLK_LEFTBRACE;
    //    break;
    //case VirtualKeyCode::kVK_OEM_5:         // 0x0000007cu /* '|' */
    //    //转回
    //    vkCode = SDLK_PIPE;
    //    break;
    //case VirtualKeyCode::kVK_OEM_6:         // 0x0000007du /* '}' */
    //    //转回
    //    vkCode = SDLK_RIGHTBRACE;
    //    break;
    //case VirtualKeyCode::kVK_OEM_3:         // 0x0000007eu /* '~' */
    //    //转回
    //    vkCode = SDLK_TILDE;
    //    break;
    case VirtualKeyCode::kVK_DELETE:        // 0x0000007fu /* '\x7F' */
        vkCode = SDLK_DELETE;
        break;
    //case SDLK_PLUSMINUS:    // 0x000000b1u /* '±' */
    //    //找不到
    //    ASSERT(0);
    //    break;
    case VirtualKeyCode::kVK_CAPITAL:       // 0x40000039u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK) */
        vkCode = SDLK_CAPSLOCK;
        break;
    case VirtualKeyCode::kVK_F1:            // 0x4000003au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1) */
        vkCode = SDLK_F1;
        break;
    case VirtualKeyCode::kVK_F2:            // 0x4000003bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2) */
        vkCode = SDLK_F2;
        break;
    case VirtualKeyCode::kVK_F3:            // 0x4000003cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3) */
        vkCode = SDLK_F3;
        break;
    case VirtualKeyCode::kVK_F4:            // 0x4000003du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4) */
        vkCode = SDLK_F4;
        break;
    case VirtualKeyCode::kVK_F5:            // 0x4000003eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5) */
        vkCode = SDLK_F5;
        break;
    case VirtualKeyCode::kVK_F6:            // 0x4000003fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6) */
        vkCode = SDLK_F6;
        break;
    case VirtualKeyCode::kVK_F7:            // 0x40000040u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7) */
        vkCode = SDLK_F7;
        break;
    case VirtualKeyCode::kVK_F8:            // 0x40000041u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8) */
        vkCode = SDLK_F8;
        break;
    case VirtualKeyCode::kVK_F9:            // 0x40000042u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9) */
        vkCode = SDLK_F9;
        break;
    case VirtualKeyCode::kVK_F10:           // 0x40000043u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10) */
        vkCode = SDLK_F10;
        break;
    case VirtualKeyCode::kVK_F11:           // 0x40000044u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11) */
        vkCode = SDLK_F11;
        break;
    case VirtualKeyCode::kVK_F12:           // 0x40000045u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12) */
        vkCode = SDLK_F12;
        break;
    case VirtualKeyCode::kVK_SNAPSHOT:      // 0x40000046u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN) */
        vkCode = SDLK_PRINTSCREEN;
        break;
    case VirtualKeyCode::kVK_SCROLL:        // 0x40000047u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK) */
        vkCode = SDLK_SCROLLLOCK;
        break;
    case VirtualKeyCode::kVK_PAUSE:         // 0x40000048u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE) */
        vkCode = SDLK_PAUSE;
        break;
    case VirtualKeyCode::kVK_INSERT:        // 0x40000049u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT) */
        vkCode = SDLK_INSERT;
        break;
    case VirtualKeyCode::kVK_HOME:          // 0x4000004au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME) */
        vkCode = SDLK_HOME;
        break;
    case VirtualKeyCode::kVK_PRIOR:         // 0x4000004bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP) */
        vkCode = SDLK_PAGEUP;
        break;
    case VirtualKeyCode::kVK_END:           // 0x4000004du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END) */
        vkCode = SDLK_END;
        break;
    case VirtualKeyCode::kVK_NEXT:          // 0x4000004eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN) */
        vkCode = SDLK_PAGEDOWN;
        break;
    case VirtualKeyCode::kVK_RIGHT:         // 0x4000004fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT) */
        vkCode = SDLK_RIGHT;
        break;
    case VirtualKeyCode::kVK_LEFT:          // 0x40000050u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT) */
        vkCode = SDLK_LEFT;
        break;
    case VirtualKeyCode::kVK_DOWN:          // 0x40000051u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN) */
        vkCode = SDLK_DOWN;
        break;
    case VirtualKeyCode::kVK_UP:            // 0x40000052u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP) */
        vkCode = SDLK_UP;
        break;
    case VirtualKeyCode::kVK_NUMLOCK:       // 0x40000053u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR) */
        //转回
        vkCode = SDLK_NUMLOCKCLEAR;
        break;
    case VirtualKeyCode::kVK_DIVIDE:        // 0x40000054u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE) */
        vkCode = SDLK_KP_DIVIDE;
        break;
    case VirtualKeyCode::kVK_MULTIPLY:      // 0x40000055u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY) */
        vkCode = SDLK_KP_MULTIPLY;
        break;
    case VirtualKeyCode::kVK_SUBTRACT:      // 0x40000056u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS) */
        vkCode = SDLK_KP_MINUS;
        break;
    case VirtualKeyCode::kVK_ADD:           // 0x40000057u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS) */
        vkCode = SDLK_KP_PLUS;
        break;
    case VirtualKeyCode::kVK_SEPARATOR:     // 0x40000058u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER) */
        vkCode = SDLK_KP_ENTER;
        break;
    case VirtualKeyCode::kVK_NUMPAD1:       // 0x40000059u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1) */
        vkCode = SDLK_KP_1;
        break;
    case VirtualKeyCode::kVK_NUMPAD2:       // 0x4000005au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2) */
        vkCode = SDLK_KP_2;
        break;
    case VirtualKeyCode::kVK_NUMPAD3:       // 0x4000005bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3) */
        vkCode = SDLK_KP_3;
        break;
    case VirtualKeyCode::kVK_NUMPAD4:       // 0x4000005cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4) */
        vkCode = SDLK_KP_4;
        break;
    case VirtualKeyCode::kVK_NUMPAD5:       // 0x4000005du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5) */
        vkCode = SDLK_KP_5;
        break;
    case VirtualKeyCode::kVK_NUMPAD6:       // 0x4000005eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6) */
        vkCode = SDLK_KP_6;
        break;
    case VirtualKeyCode::kVK_NUMPAD7:       // 0x4000005fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7) */
        vkCode = SDLK_KP_7;
        break;
    case VirtualKeyCode::kVK_NUMPAD8:       // 0x40000060u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8) */
        vkCode = SDLK_KP_8;
        break;
    case VirtualKeyCode::kVK_NUMPAD9:       // 0x40000061u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9) */
        vkCode = SDLK_KP_9;
        break;
    case VirtualKeyCode::kVK_NUMPAD0:       // 0x40000062u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0) */
        vkCode = SDLK_KP_0;
        break;
    case VirtualKeyCode::kVK_DECIMAL:       // 0x40000063u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD) */
        vkCode = SDLK_KP_PERIOD;
        break;
    //case SDLK_APPLICATION:  // 0x40000065u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION) */
    //    ASSERT(0);
    //    break;
    //case SDLK_POWER:        // 0x40000066u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER) */
    //    ASSERT(0);
    //    break;
    //case SDLK_KP_EQUALS:    // 0x40000067u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS) */
    //    ASSERT(0);
    //    break;
    case VirtualKeyCode::kVK_F13:           // 0x40000068u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13) */
        vkCode = SDLK_F13;
        break;
    case VirtualKeyCode::kVK_F14:           // 0x40000069u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14) */
        vkCode = SDLK_F14;
        break;
    case VirtualKeyCode::kVK_F15:           // 0x4000006au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15) */
        vkCode = SDLK_F15;
        break;
    case VirtualKeyCode::kVK_F16:           // 0x4000006bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16) */
        vkCode = SDLK_F16;
        break;
    case VirtualKeyCode::kVK_F17:           // 0x4000006cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17) */
        vkCode = SDLK_F17;
        break;
    case VirtualKeyCode::kVK_F18:           // 0x4000006du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18) */
        vkCode = SDLK_F18;
        break;
    case VirtualKeyCode::kVK_F19:           // 0x4000006eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19) */
        vkCode = SDLK_F19;
        break;
    case VirtualKeyCode::kVK_F20:           // 0x4000006fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20) */
        vkCode = SDLK_F20;
        break;
    case VirtualKeyCode::kVK_F21:           // 0x40000070u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21) */
        vkCode = SDLK_F21;
        break;
    case VirtualKeyCode::kVK_F22:           // 0x40000071u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22) */
        vkCode = SDLK_F22;
        break;
    case VirtualKeyCode::kVK_F23:           // 0x40000072u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23) */
        vkCode = SDLK_F23;
        break;
    case VirtualKeyCode::kVK_F24:           // 0x40000073u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24) */
        vkCode = SDLK_F24;
        break;
    //case SDLK_EXECUTE:                    // 0x40000074u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE) */
    //    ASSERT(0);
    //    break;
    case VirtualKeyCode::kVK_HELP:          // 0x40000075u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP) */
        vkCode = SDLK_HELP;
        break;
    case VirtualKeyCode::kVK_MENU:          // 0x40000076u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU) */
        vkCode = SDLK_MENU;
        break;
    //case SDLK_SELECT:       // 0x40000077u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT) */
    //case SDLK_STOP:         // 0x40000078u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP) */
    //case SDLK_AGAIN:        // 0x40000079u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN) */
    //case SDLK_UNDO:         // 0x4000007au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO) */
    //case SDLK_CUT:          // 0x4000007bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT) */
    //case SDLK_COPY:         // 0x4000007cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY) */
    //case SDLK_PASTE:        // 0x4000007du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE) */
    //case SDLK_FIND:         // 0x4000007eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND) */
    //    ASSERT(0);
    //    break;
    case VirtualKeyCode::kVK_VOLUME_MUTE:       // 0x4000007fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE) */
        vkCode = SDLK_MUTE;
        break;
    case VirtualKeyCode::kVK_VOLUME_UP:         // 0x40000080u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP) */
        vkCode = SDLK_VOLUMEUP;
        break;
    case VirtualKeyCode::kVK_VOLUME_DOWN:       // 0x40000081u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN) */
        vkCode = SDLK_VOLUMEDOWN;
        break;
    //case SDLK_KP_COMMA:     // 0x40000085u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA) */
    //case SDLK_KP_EQUALSAS400:// 0x40000086u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400) */
    //case SDLK_ALTERASE:     // 0x40000099u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE) */
    //case SDLK_SYSREQ:       // 0x4000009au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ) */
    //case SDLK_CANCEL:       // 0x4000009bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL) */
    //case SDLK_CLEAR:        // 0x4000009cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR) */
    //case SDLK_PRIOR:        // 0x4000009du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR) */
    //case SDLK_RETURN2:      // 0x4000009eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2) */
    //case SDLK_SEPARATOR:    // 0x4000009fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR) */
    //case SDLK_OUT:          // 0x400000a0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT) */
    //case SDLK_OPER:         // 0x400000a1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER) */
    //case SDLK_CLEARAGAIN:   // 0x400000a2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN) */
    //case SDLK_CRSEL:        // 0x400000a3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL) */
    //case SDLK_EXSEL:        // 0x400000a4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL) */
    //case SDLK_KP_00:        // 0x400000b0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00) */
    //case SDLK_KP_000:       // 0x400000b1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000) */
    //case SDLK_THOUSANDSSEPARATOR:   // 0x400000b2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR) */
    //case SDLK_DECIMALSEPARATOR:     // 0x400000b3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR) */
    //case SDLK_CURRENCYUNIT:         // 0x400000b4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT) */
    //case SDLK_CURRENCYSUBUNIT:      // 0x400000b5u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT) */
    //case SDLK_KP_LEFTPAREN:         // 0x400000b6u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN) */
    //case SDLK_KP_RIGHTPAREN:        // 0x400000b7u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN) */
    //case SDLK_KP_LEFTBRACE:         // 0x400000b8u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE) */
    //case SDLK_KP_RIGHTBRACE:        // 0x400000b9u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE) */
    //case SDLK_KP_TAB:               // 0x400000bau /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB) */
    //case SDLK_KP_BACKSPACE: // 0x400000bbu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE) */
    //case SDLK_KP_A:         // 0x400000bcu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A) */
    //case SDLK_KP_B:         // 0x400000bdu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B) */
    //case SDLK_KP_C:         // 0x400000beu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C) */
    //case SDLK_KP_D:         // 0x400000bfu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D) */
    //case SDLK_KP_E:         // 0x400000c0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E) */
    //case SDLK_KP_F:         // 0x400000c1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F) */
    //case SDLK_KP_XOR:       // 0x400000c2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR) */
    //case SDLK_KP_POWER:     // 0x400000c3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER) */
    //case SDLK_KP_PERCENT:   // 0x400000c4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT) */
    //case SDLK_KP_LESS:      // 0x400000c5u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS) */
    //case SDLK_KP_GREATER:   // 0x400000c6u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER) */
    //case SDLK_KP_AMPERSAND: // 0x400000c7u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND) */
    //case SDLK_KP_DBLAMPERSAND:  // 0x400000c8u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND) */
    //case SDLK_KP_VERTICALBAR:   // 0x400000c9u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR) */
    //case SDLK_KP_DBLVERTICALBAR: // 0x400000cau /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR) */
    //case SDLK_KP_COLON:         // 0x400000cbu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON) */
    //case SDLK_KP_HASH:          // 0x400000ccu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH) */
    //case SDLK_KP_SPACE:         // 0x400000cdu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE) */
    //case SDLK_KP_AT:            // 0x400000ceu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT) */
    //case SDLK_KP_EXCLAM:        // 0x400000cfu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM) */
    //case SDLK_KP_MEMSTORE:      // 0x400000d0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE) */
    //case SDLK_KP_MEMRECALL:     // 0x400000d1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL) */
    //case SDLK_KP_MEMCLEAR:      // 0x400000d2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR) */
    //case SDLK_KP_MEMADD:        // 0x400000d3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD) */
    //case SDLK_KP_MEMSUBTRACT:   // 0x400000d4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT) */
    //case SDLK_KP_MEMMULTIPLY:   // 0x400000d5u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY) */
    //case SDLK_KP_MEMDIVIDE:     // 0x400000d6u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE) */
    //case SDLK_KP_PLUSMINUS:     // 0x400000d7u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS) */
    //case SDLK_KP_CLEAR:         // 0x400000d8u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR) */
    //case SDLK_KP_CLEARENTRY:    // 0x400000d9u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY) */
    //case SDLK_KP_BINARY:        // 0x400000dau /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY) */
    //case SDLK_KP_OCTAL:         // 0x400000dbu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL) */
    //case SDLK_KP_DECIMAL:       // 0x400000dcu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL) */
    //case SDLK_KP_HEXADECIMAL:   // 0x400000ddu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL) */
    //    ASSERT(0);
    //    break;
    case VirtualKeyCode::kVK_LCONTROL:          // 0x400000e0u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL) */
        vkCode = SDLK_LCTRL;
        break;
    case VirtualKeyCode::kVK_LSHIFT:            // 0x400000e1u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT) */
        vkCode = SDLK_LSHIFT;
        break;
    case VirtualKeyCode::kVK_LMENU:             // 0x400000e2u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT) */
        vkCode = SDLK_LALT;
        break;
    case VirtualKeyCode::kVK_LWIN:              // 0x400000e3u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI) */
        vkCode = SDLK_LGUI;
        break;
    case VirtualKeyCode::kVK_RCONTROL:          // 0x400000e4u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL) */
        vkCode = SDLK_RCTRL;
        break;
    case VirtualKeyCode::kVK_RSHIFT:            // 0x400000e5u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT) */
        vkCode = SDLK_RSHIFT;
        break;
    case VirtualKeyCode::kVK_RMENU:             // 0x400000e6u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT) */
        vkCode = SDLK_RALT;
        break;
    case VirtualKeyCode::kVK_RWIN:              // 0x400000e7u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI) */
        vkCode = SDLK_RGUI;
        break;
    //case SDLK_MODE:             // 0x40000101u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE) */
    //case SDLK_SLEEP:            // 0x40000102u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP) */
    //case SDLK_WAKE:             // 0x40000103u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WAKE) */
    //case SDLK_CHANNEL_INCREMENT:// 0x40000104u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_INCREMENT) */
    //case SDLK_CHANNEL_DECREMENT:// 0x40000105u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_DECREMENT) */
    //    ASSERT(0);
    //    break;
    //case SDLK_MEDIA_PLAY:       // 0x40000106u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY) */
    //case SDLK_MEDIA_PAUSE:      // 0x40000107u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PAUSE) */
    //case SDLK_MEDIA_RECORD:     // 0x40000108u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_RECORD) */
    //case SDLK_MEDIA_FAST_FORWARD://0x40000109u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_FAST_FORWARD) */
    //case SDLK_MEDIA_REWIND:     // 0x4000010au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_REWIND) */
    //case SDLK_MEDIA_NEXT_TRACK: // 0x4000010bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_NEXT_TRACK) */
    //case SDLK_MEDIA_PREVIOUS_TRACK://0x4000010cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PREVIOUS_TRACK) */
    //case SDLK_MEDIA_STOP:       // 0x4000010du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_STOP) */
    //case SDLK_MEDIA_EJECT:      // 0x4000010eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_EJECT) */
    //case SDLK_MEDIA_PLAY_PAUSE: // 0x4000010fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY_PAUSE) */
    //case SDLK_MEDIA_SELECT:     // 0x40000110u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_SELECT) */
    //case SDLK_AC_NEW:           // 0x40000111u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_NEW) */
    //case SDLK_AC_OPEN:          // 0x40000112u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_OPEN) */
    //case SDLK_AC_CLOSE:         // 0x40000113u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_CLOSE) */
    //case SDLK_AC_EXIT:          // 0x40000114u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_EXIT) */
    //case SDLK_AC_SAVE:          // 0x40000115u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SAVE) */
    //case SDLK_AC_PRINT:         // 0x40000116u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PRINT) */
    //case SDLK_AC_PROPERTIES:    // 0x40000117u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PROPERTIES) */
    //case SDLK_AC_SEARCH:        // 0x40000118u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH) */
    //case SDLK_AC_HOME:          // 0x40000119u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME) */
    //case SDLK_AC_BACK:          // 0x4000011au /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK) */
    //case SDLK_AC_FORWARD:       // 0x4000011bu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD) */
    //case SDLK_AC_STOP:          // 0x4000011cu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP) */
    //case SDLK_AC_REFRESH:       // 0x4000011du /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH) */
    //case SDLK_AC_BOOKMARKS:     // 0x4000011eu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS) */
    //case SDLK_SOFTLEFT:         // 0x4000011fu /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTLEFT) */
    //case SDLK_SOFTRIGHT:        // 0x40000120u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTRIGHT) */
    //case SDLK_CALL:             //  0x40000121u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALL) */
    //case SDLK_ENDCALL:          // 0x40000122u /* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ENDCALL) */
    //    ASSERT(0);
    //    break;
    default:
        break;
    }
    return vkCode;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL
