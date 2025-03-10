#ifndef UI_CORE_KEYCODE_H_
#define UI_CORE_KEYCODE_H_

#include "duilib/duilib_defs.h"

namespace ui {

/*
* WM_KEYUP/DOWN/CHAR HIWORD(lParam) flags, 当按下Win键的时候，有此标志
*/
#define kKF_EXTENDED    0x0100

/** 虚拟键编码(和Windows系统定义一致，WinUser.h)
*/
enum VirtualKeyCode
{
    kVK_None        = 0x00, //无虚拟键
    kVK_LBUTTON     = 0x01, //鼠标左键
    kVK_RBUTTON     = 0x02, //鼠标右键
    kVK_CANCEL      = 0x03, //控制中断处理
    kVK_MBUTTON     = 0x04, //鼠标中键
    kVK_XBUTTON1    = 0x05, //X1 鼠标按钮
    kVK_XBUTTON2    = 0x06, //X2 鼠标按钮
    //- = 0x07, //0x07
    kVK_BACK        = 0x08, //BACKSPACE 键
    kVK_TAB         = 0x09, //Tab 键
    //- = 0x0A//-0B, //预留
    kVK_CLEAR       = 0x0C, //CLEAR 键
    kVK_RETURN      = 0x0D, //Enter 键
    //- = 0x0E//-0F, //未分配
    kVK_SHIFT       = 0x10, //SHIFT 键
    kVK_CONTROL     = 0x11, //CTRL 键
    kVK_MENU        = 0x12, //Alt 键
    kVK_PAUSE       = 0x13, //PAUSE 键
    kVK_CAPITAL     = 0x14, //CAPS LOCK 键
    kVK_KANA        = 0x15, //IME Kana 模式
    kVK_HANGUL      = 0x15, //IME Hanguel 模式
    kVK_IME_ON      = 0x16, //IME 打开
    kVK_JUNJA       = 0x17, //IME Junja 模式
    kVK_FINAL       = 0x18, //IME 最终模式
    kVK_HANJA       = 0x19, //IME Hanja 模式
    kVK_KANJI       = 0x19, //IME Kanji 模式
    kVK_IME_OFF     = 0x1A, //IME 关闭
    kVK_ESCAPE      = 0x1B, //ESC 键
    kVK_CONVERT     = 0x1C, //IME 转换
    kVK_NONCONVERT  = 0x1D, //IME 不转换
    kVK_ACCEPT      = 0x1E, //IME 接受
    kVK_MODECHANGE  = 0x1F, //IME 模式更改请求
    kVK_SPACE       = 0x20, //空格键
    kVK_PRIOR       = 0x21, //PAGE UP 键
    kVK_NEXT        = 0x22, //PAGE DOWN 键
    kVK_END         = 0x23, //END 键
    kVK_HOME        = 0x24, //HOME 键
    kVK_LEFT        = 0x25, //LEFT ARROW 键
    kVK_UP          = 0x26, //UP ARROW 键
    kVK_RIGHT       = 0x27, //RIGHT ARROW 键
    kVK_DOWN        = 0x28, //DOWN ARROW 键
    kVK_SELECT      = 0x29, //SELECT 键
    kVK_PRINT       = 0x2A, //PRINT 键
    kVK_EXECUTE     = 0x2B, //EXECUTE 键
    kVK_SNAPSHOT    = 0x2C, //PRINT SCREEN 键
    kVK_INSERT      = 0x2D, //INS 键
    kVK_DELETE      = 0x2E, //DEL 键
    kVK_HELP        = 0x2F, //HELP 键
    kVK_0           = 0x30, //0 键
    kVK_1           = 0x31, //1 键
    kVK_2           = 0x32, //2 键
    kVK_3           = 0x33, //3 键
    kVK_4           = 0x34, //4 键
    kVK_5           = 0x35, //5 键
    kVK_6           = 0x36, //6 键
    kVK_7           = 0x37, //7 键
    kVK_8           = 0x38, //8 键
    kVK_9           = 0x39, //9 键
    //- = 0x3A//-40, //Undefined
    kVK_A           = 0x41, //A 键
    kVK_B           = 0x42, //B 键
    kVK_C           = 0x43, //C 键
    kVK_D           = 0x44, //D 键
    kVK_E           = 0x45, //E 键
    kVK_F           = 0x46, //F 键
    kVK_G           = 0x47, //G 键
    kVK_H           = 0x48, //H 键
    kVK_I           = 0x49, //I 键
    kVK_J           = 0x4A, //J 键
    kVK_K           = 0x4B, //K 键
    kVK_L           = 0x4C, //L 键
    kVK_M           = 0x4D, //M 键
    kVK_N           = 0x4E, //N 键
    kVK_O           = 0x4F, //O 键
    kVK_P           = 0x50, //P 键
    kVK_Q           = 0x51, //Q 键
    kVK_R           = 0x52, //R 键
    kVK_S           = 0x53, //S 键
    kVK_T           = 0x54, //T 键
    kVK_U           = 0x55, //U 键
    kVK_V           = 0x56, //V 键
    kVK_W           = 0x57, //W 键
    kVK_X           = 0x58, //X 键
    kVK_Y           = 0x59, //Y 键
    kVK_Z           = 0x5A, //Z 键
    kVK_LWIN        = 0x5B, //左 Windows 键
    kVK_RWIN        = 0x5C, //右侧 Windows 键
    kVK_APPS        = 0x5D, //应用程序密钥
    //- = 0x5E, //预留
    kVK_SLEEP       = 0x5F, //计算机休眠键
    kVK_NUMPAD0     = 0x60, //数字键盘 0 键
    kVK_NUMPAD1     = 0x61, //数字键盘 1 键
    kVK_NUMPAD2     = 0x62, //数字键盘 2 键
    kVK_NUMPAD3     = 0x63, //数字键盘 3 键
    kVK_NUMPAD4     = 0x64, //数字键盘 4 键
    kVK_NUMPAD5     = 0x65, //数字键盘 5 键
    kVK_NUMPAD6     = 0x66, //数字键盘 6 键
    kVK_NUMPAD7     = 0x67, //数字键盘 7 键
    kVK_NUMPAD8     = 0x68, //数字键盘 8 键
    kVK_NUMPAD9     = 0x69, //数字键盘 9 键
    kVK_MULTIPLY    = 0x6A, //乘号键 '*'
    kVK_ADD         = 0x6B, //加号键 '+'
    kVK_SEPARATOR   = 0x6C, //分隔符键, 小键盘 Enter
    kVK_SUBTRACT    = 0x6D, //减号键 '-'
    kVK_DECIMAL     = 0x6E, //句点键 '.'
    kVK_DIVIDE      = 0x6F, //除号键 '/'
    kVK_F1          = 0x70, //F1 键
    kVK_F2          = 0x71, //F2 键
    kVK_F3          = 0x72, //F3 键
    kVK_F4          = 0x73, //F4 键
    kVK_F5          = 0x74, //F5 键
    kVK_F6          = 0x75, //F6 键
    kVK_F7          = 0x76, //F7 键
    kVK_F8          = 0x77, //F8 键
    kVK_F9          = 0x78, //F9 键
    kVK_F10         = 0x79, //F10 键
    kVK_F11         = 0x7A, //F11 键
    kVK_F12         = 0x7B, //F12 键
    kVK_F13         = 0x7C, //F13 键
    kVK_F14         = 0x7D, //F14 键
    kVK_F15         = 0x7E, //F15 键
    kVK_F16         = 0x7F, //F16 键
    kVK_F17         = 0x80, //F17 键
    kVK_F18         = 0x81, //F18 键
    kVK_F19         = 0x82, //F19 键
    kVK_F20         = 0x83, //F20 键
    kVK_F21         = 0x84, //F21 键
    kVK_F22         = 0x85, //F22 键
    kVK_F23         = 0x86, //F23 键
    kVK_F24         = 0x87, //F24 键
    //- = 0x88//-8F, //保留
    kVK_NUMLOCK     = 0x90, //NUM LOCK 键
    kVK_SCROLL      = 0x91, //SCROLL LOCK 键
    //- = 0x92//-96, //OEM 特有
    //- = 0x97//-9F, //未分配
    kVK_LSHIFT      = 0xA0, //左 SHIFT 键
    kVK_RSHIFT      = 0xA1, //右 SHIFT 键
    kVK_LCONTROL    = 0xA2, //左 Ctrl 键
    kVK_RCONTROL    = 0xA3, //右 Ctrl 键
    kVK_LMENU       = 0xA4, //左 ALT 键
    kVK_RMENU       = 0xA5, //右 ALT 键
    kVK_BROWSER_BACK        = 0xA6, //浏览器后退键
    kVK_BROWSER_FORWARD     = 0xA7, //浏览器前进键
    kVK_BROWSER_REFRESH     = 0xA8, //浏览器刷新键
    kVK_BROWSER_STOP        = 0xA9, //浏览器停止键
    kVK_BROWSER_SEARCH      = 0xAA, //浏览器搜索键
    kVK_BROWSER_FAVORITES   = 0xAB, //浏览器收藏键
    kVK_BROWSER_HOME        = 0xAC, //浏览器“开始”和“主页”键
    kVK_VOLUME_MUTE         = 0xAD, //静音键
    kVK_VOLUME_DOWN         = 0xAE, //音量减小键
    kVK_VOLUME_UP           = 0xAF, //音量增加键
    kVK_MEDIA_NEXT_TRACK    = 0xB0, //下一曲目键
    kVK_MEDIA_PREV_TRACK    = 0xB1, //上一曲目键
    kVK_MEDIA_STOP          = 0xB2, //停止媒体键
    kVK_MEDIA_PLAY_PAUSE    = 0xB3, //播放/暂停媒体键
    kVK_LAUNCH_MAIL         = 0xB4, //启动邮件键
    kVK_LAUNCH_MEDIA_SELECT = 0xB5, //选择媒体键
    kVK_LAUNCH_APP1         = 0xB6, //启动应用程序 1 键
    kVK_LAUNCH_APP2         = 0xB7, //启动应用程序 2 键
    //- = 0xB8//-B9, //预留
    kVK_OEM_1               = 0xBA, //用于杂项字符；它可能因键盘而异。 对于美国标准键盘，键;:
    kVK_OEM_PLUS            = 0xBB, //对于任何国家/地区，键+
    kVK_OEM_COMMA           = 0xBC, //对于任何国家/地区，键,
    kVK_OEM_MINUS           = 0xBD, //对于任何国家/地区，键//-
    kVK_OEM_PERIOD          = 0xBE, //对于任何国家/地区，键.
    kVK_OEM_2               = 0xBF, //用于杂项字符；它可能因键盘而异。 对于美国标准键盘，键/?
    kVK_OEM_3               = 0xC0, //用于杂项字符；它可能因键盘而异。 对于美国标准键盘，键`~
    //- = 0xC1//-DA, //保留
    kVK_OEM_4               = 0xDB, //用于杂项字符；它可能因键盘而异。 对于美国标准键盘，键[{
    kVK_OEM_5               = 0xDC, //用于杂项字符；它可能因键盘而异。 对于美国标准键盘，键\\|
    kVK_OEM_6               = 0xDD, //用于杂项字符；它可能因键盘而异。 对于美国标准键盘，键]}
    kVK_OEM_7               = 0xDE, //用于杂项字符；它可能因键盘而异。 对于美国标准键盘，键'"
    kVK_OEM_8               = 0xDF, //用于杂项字符；它可能因键盘而异。
    //- = 0xE0, //预留
    //- = 0xE1, //OEM 特有
    kVK_OEM_102             = 0xE2, //美国标准键盘上的 <> 键，或非美国 102 键键盘上的 \\| 键
    //- = 0xE3//-E4, //OEM 特有
    kVK_PROCESSKEY          = 0xE5, //IME PROCESS 键
    //- = 0xE6, //OEM 特有
    kVK_PACKET              = 0xE7, //用于将 Unicode 字符当作键击传递。 kVK_PACKET 键是用于非键盘输入法的 32 位虚拟键值的低位字。 有关更多信息，请参阅 KEYBDINPUT、SendInput、WM_KEYDOWN 和 WM_KEYUP 中的注释
    //- = 0xE8, //未分配
    //- = 0xE9//-F5, //OEM 特有
    kVK_ATTN                = 0xF6, //Attn 键
    kVK_CRSEL               = 0xF7, //CrSel 键
    kVK_EXSEL               = 0xF8, //ExSel 键
    kVK_EREOF               = 0xF9, //Erase EOF 键
    kVK_PLAY                = 0xFA, //Play 键
    kVK_ZOOM                = 0xFB, //Zoom 键
    kVK_NONAME              = 0xFC, //预留
    kVK_PA1                 = 0xFD, //PA1 键
    kVK_OEM_CLEAR           = 0xFE, //Clear 键
};

#ifdef DUILIB_BUILD_FOR_SDL

    //VirtualKeyCode 与 SDL的 SDL_Keycode相互转换
    typedef uint32_t SDL_Keycode;

    class Keycode
    {
    public:
        /** SDL_Keycode 转换为 ui::VirtualKeyCode(有丢失，SDL的内容比VirtualKeyCode多)
        */
        static VirtualKeyCode GetVirtualKeyCode(SDL_Keycode sdlKeycode);

        /** ui::VirtualKeyCode 转换为 SDL_Keycode
        */
        static SDL_Keycode GetSDLKeyCode(VirtualKeyCode vkCode);
    };

#endif

} // namespace ui

#endif // UI_CORE_KEYCODE_H_

