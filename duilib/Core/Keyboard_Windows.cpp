#include "Keyboard.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

namespace ui
{
bool Keyboard::IsKeyDown(VirtualKeyCode nVirtKey)
{
    return (::GetKeyState(nVirtKey) & 0x8000) != 0;
}

bool Keyboard::IsNumLockOn()
{
    return (::GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
}

bool Keyboard::IsScrollLockOn()
{
    return (::GetKeyState(VK_SCROLL) & 0x0001) != 0;
}

bool Keyboard::IsCapsLockOn()
{
    return (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
}

DString Keyboard::GetKeyName(VirtualKeyCode nVirtKey, bool fExtended)
{
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
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
