#include "Keyboard.h"

namespace ui
{
int16_t Keyboard::GetKeyState(VirtualKeyCode nVirtKey)
{
#ifdef DUILIB_PLATFORM_WIN
    return ::GetKeyState(nVirtKey);
#else
    ASSERT(false);
    return 0;
#endif
}

int16_t Keyboard::GetAsyncKeyState(VirtualKeyCode nVirtKey)
{
#ifdef DUILIB_PLATFORM_WIN
    return ::GetAsyncKeyState(nVirtKey);
#else
    ASSERT(false);
    return 0;
#endif
}

bool Keyboard::IsKeyDown(VirtualKeyCode nVirtKey)
{
#ifdef DUILIB_PLATFORM_WIN
    return (::GetKeyState(nVirtKey) & 0x8000) != 0;
#else
    ASSERT(false);
    return 0;
#endif
}

bool Keyboard::IsNumLockOn()
{
#ifdef DUILIB_PLATFORM_WIN
    return (::GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
#else
    ASSERT(false);
    return 0;
#endif
}

bool Keyboard::IsScrollLockOn()
{
#ifdef DUILIB_PLATFORM_WIN
    return (::GetKeyState(VK_SCROLL) & 0x0001) != 0;
#else
    ASSERT(false);
    return 0;
#endif
}

bool Keyboard::IsCapsLockOn()
{
#ifdef DUILIB_PLATFORM_WIN
    return (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
#else
    ASSERT(false);
    return 0;
#endif
}

DString Keyboard::GetKeyName(VirtualKeyCode nVirtKey, bool fExtended)
{
#ifdef DUILIB_PLATFORM_WIN
    UINT nScanCode = ::MapVirtualKeyEx(nVirtKey, 0, ::GetKeyboardLayout(0));
    switch (nVirtKey)
    {
        // Keys which are "extended" (except for Return which is Numeric Enter as extended)
    case kVK_INSERT:
    case kVK_DELETE:
    case kVK_HOME:
    case kVK_END:
    case kVK_NEXT:  // Page down
    case kVK_PRIOR: // Page up
    case kVK_LEFT:
    case kVK_RIGHT:
    case kVK_UP:
    case kVK_DOWN:
        nScanCode |= 0x100; // Add extended bit
        break;
    default:
        break;
}
    if (fExtended) {
        nScanCode |= 0x01000000L;
    }

    TCHAR szStr[MAX_PATH] = { 0 };
    ::GetKeyNameText(nScanCode << 16, szStr, MAX_PATH);
    return DString(szStr);
#else
    #error "Keyboard::GetKeyName"
#endif
}

DString Keyboard::GetKeyName(LPARAM lParam)
{
#ifdef DUILIB_PLATFORM_WIN
    TCHAR szStr[MAX_PATH] = { 0 };
    ::GetKeyNameText((LONG)lParam, szStr, MAX_PATH);
    DString keyName = szStr;
    return keyName;
#else
    #error "Keyboard::GetKeyName"
#endif
}

} // namespace ui
