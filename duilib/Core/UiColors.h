#ifndef UI_CORE_UICOLORS_H_
#define UI_CORE_UICOLORS_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <vector>
#include <string>

namespace ui 
{
namespace UiColors
{
    /** 常见颜色值常量(ARGB格式)
    */
    enum
    {
        AliceBlue = 0xFFF0F8FF,     //爱丽丝蓝,接近浅蓝灰色或钢青色
        AntiqueWhite = 0xFFFAEBD7,  //古董白
        Aqua = 0xFF00FFFF,          //浅绿色
        Aquamarine = 0xFF7FFFD4,    //海蓝宝石色，碧绿色，介于绿色和蓝色之间
        Azure = 0xFFF0FFFF,         //蔚蓝色的
        Beige = 0xFFF5F5DC,         //米色
        Bisque = 0xFFFFE4C4,        //桔黄色
        Black = 0xFF000000,         //黑色
        BlanchedAlmond = 0xFFFFEBCD,//白杏色
        Blue = 0xFF0000FF,          //蓝色
        BlueViolet = 0xFF8A2BE2,    //蓝紫罗兰色
        Brown = 0xFFA52A2A,         //棕色，褐色
        BurlyWood = 0xFFDEB887,     //实木色
        CadetBlue = 0xFF5F9EA0,     //军蓝色
        Chartreuse = 0xFF7FFF00,    //黄绿色
        Chocolate = 0xFFD2691E,     //巧克力色
        Coral = 0xFFFF7F50,         //珊瑚色
        CornflowerBlue = 0xFF6495ED,//菊兰色
        Cornsilk = 0xFFFFF8DC,      //米绸色
        Crimson = 0xFFDC143C,       //暗红色的
        Cyan = 0xFF00FFFF,          //青色
        DarkBlue = 0xFF00008B,      //深蓝色
        DarkCyan = 0xFF008B8B,      //深青色
        DarkGoldenrod = 0xFFB8860B, //深金黄黄
        DarkGray = 0xFFA9A9A9,      //深灰色
        DarkGreen = 0xFF006400,     //深绿色
        DarkKhaki = 0xFFBDB76B,     //暗卡其色，深黄褐色
        DarkMagenta = 0xFF8B008B,   //深品红色，暗洋红
        DarkOliveGreen = 0xFF556B2F,//暗橄榄绿
        DarkOrange = 0xFFFF8C00,    //深橙色
        DarkOrchid = 0xFF9932CC,    //暗紫色
        DarkRed = 0xFF8B0000,       //深红色
        DarkSalmon = 0xFFE9967A,    //暗肉色
        DarkSeaGreen = 0xFF8FBC8B,  //深海蓝色
        DarkSlateBlue = 0xFF483D8B, //深灰蓝色
        DarkSlateGray = 0xFF2F4F4F, //暗绿色
        DarkTurquoise = 0xFF00CED1, //暗宝石绿色
        DarkViolet = 0xFF9400D3,    //暗紫罗兰色
        DeepPink = 0xFFFF1493,      //深粉红色
        DeepSkyBlue = 0xFF00BFFF,   //深天蓝色
        DimGray = 0xFF696969,       //暗灰色
        DodgerBlue = 0xFF1E90FF,    //闪兰色
        Firebrick = 0xFFB22222,     //火砖色
        FloralWhite = 0xFFFFFAF0,   //花白色
        ForestGreen = 0xFF228B22,   //森林绿
        Fuchsia = 0xFFFF00FF,       //紫红色
        Gainsboro = 0xFFDCDCDC,     //淡灰色
        GhostWhite = 0xFFF8F8FF,    //幽灵白色
        Gold = 0xFFFFD700,          //金色
        Goldenrod = 0xFFDAA520,     //金麒麟色
        Gray = 0xFF808080,          //灰色
        Green = 0xFF008000,         //绿色
        GreenYellow = 0xFFADFF2F,   //黄绿色
        Honeydew = 0xFFF0FFF0,      //蜜色
        HotPink = 0xFFFF69B4,       //热粉红色
        IndianRed = 0xFFCD5C5C,     //印第安红
        Indigo = 0xFF4B0082,        //靛蓝色
        Ivory = 0xFFFFFFF0,         //象牙色
        Khaki = 0xFFF0E68C,         //黄褐色
        Lavender = 0xFFE6E6FA,      //薰衣草色，淡紫色
        LavenderBlush = 0xFFFFF0F5, //淡紫红色
        LawnGreen = 0xFF7CFC00,     //草绿色
        LemonChiffon = 0xFFFFFACD,  //柠檬绸色
        LightBlue = 0xFFADD8E6,     //淡蓝色
        LightCoral = 0xFFF08080,    //淡珊瑚色
        LightCyan = 0xFFE0FFFF,     //淡青色
        LightGoldenrodYellow = 0xFFFAFAD2,  //亮菊黄
        LightGray = 0xFFD3D3D3,     //浅灰色
        LightGreen = 0xFF90EE90,    //淡绿色
        LightPink = 0xFFFFB6C1,     //浅粉红
        LightSalmon = 0xFFFFA07A,   //亮肉色
        LightSeaGreen = 0xFF20B2AA, //浅海蓝色
        LightSkyBlue = 0xFF87CEFA,  //亮天蓝色
        LightSlateGray = 0xFF778899,//亮蓝灰
        LightSteelBlue = 0xFFB0C4DE,//亮钢蓝色
        LightYellow = 0xFFFFFFE0,   //浅黄色
        Lime = 0xFF00FF00,          //石灰色
        LimeGreen = 0xFF32CD32,     //酸橙绿色
        Linen = 0xFFFAF0E6,         //亚麻色
        Magenta = 0xFFFF00FF,       //洋红色
        Maroon = 0xFF800000,        //紫褐色
        MediumAquamarine = 0xFF66CDAA,  //中碧绿色
        MediumBlue = 0xFF0000CD,    //中蓝色
        MediumOrchid = 0xFFBA55D3,  //中兰花紫
        MediumPurple = 0xFF9370DB,  //中紫色
        MediumSeaGreen = 0xFF3CB371,    //中海洋绿
        MediumSlateBlue = 0xFF7B68EE,   //中板岩蓝
        MediumSpringGreen = 0xFF00FA9A, //中春绿色
        MediumTurquoise = 0xFF48D1CC,   //中绿宝石色
        MediumVioletRed = 0xFFC71585,   //中紫罗兰色
        MidnightBlue = 0xFF191970,      //深夜蓝
        MintCream = 0xFFF5FFFA,         //薄荷色
        MistyRose = 0xFFFFE4E1,         //薄雾玫瑰色
        Moccasin = 0xFFFFE4B5,          //鹿皮色
        NavajoWhite = 0xFFFFDEAD,       //纳瓦白
        Navy = 0xFF000080,              //海军蓝
        OldLace = 0xFFFDF5E6,           //浅米色
        Olive = 0xFF808000,             //橄榄色
        OliveDrab = 0xFF6B8E23,         //深绿褐色
        Orange = 0xFFFFA500,            //橙色
        OrangeRed = 0xFFFF4500,         //橙红色
        Orchid = 0xFFDA70D6,            //兰花紫色
        PaleGoldenrod = 0xFFEEE8AA,     //淡黄色
        PaleGreen = 0xFF98FB98,         //淡绿色
        PaleTurquoise = 0xFFAFEEEE,     //苍绿色
        PaleVioletRed = 0xFFDB7093,     //浅紫罗兰红
        PapayaWhip = 0xFFFFEFD5,        //番木色
        PeachPuff = 0xFFFFDAB9,         //桃色
        Peru = 0xFFCD853F,              //秘鲁色
        Pink = 0xFFFFC0CB,              //粉红色
        Plum = 0xFFDDA0DD,              //李子色
        PowderBlue = 0xFFB0E0E6,        //粉蓝色
        Purple = 0xFF800080,            //紫色
        Red = 0xFFFF0000,               //红色
        RosyBrown = 0xFFBC8F8F,     //玫瑰棕色
        RoyalBlue = 0xFF4169E1,     //皇家蓝色
        SaddleBrown = 0xFF8B4513,   //重褐色
        Salmon = 0xFFFA8072,        //鲑鱼色，三文鱼色
        SandyBrown = 0xFFF4A460,    //沙棕色
        SeaGreen = 0xFF2E8B57,      //海绿色
        SeaShell = 0xFFFFF5EE,      //海贝色
        Sienna = 0xFFA0522D,        //黄土赭色
        Silver = 0xFFC0C0C0,        //银色
        SkyBlue = 0xFF87CEEB,       //天蓝色
        SlateBlue = 0xFF6A5ACD,     //石板蓝色
        SlateGray = 0xFF708090,     //石板灰色
        Snow = 0xFFFFFAFA,          //雪白色
        SpringGreen = 0xFF00FF7F,   //春绿色
        SteelBlue = 0xFF4682B4,     //钢青色
        Tan = 0xFFD2B48C,           //棕褐色
        Teal = 0xFF008080,          //青色
        Thistle = 0xFFD8BFD8,       //蓟色，是一种紫色
        Tomato = 0xFFFF6347,        //番茄色
        Transparent = 0x00FFFFFF,   //透明的
        Turquoise = 0xFF40E0D0,     //绿松色
        Violet = 0xFFEE82EE,        //紫罗兰色
        Wheat = 0xFFF5DEB3,         //小麦色, 淡黄色
        White = 0xFFFFFFFF,         //白色
        WhiteSmoke = 0xFFF5F5F5,    //白烟色
        Yellow = 0xFFFFFF00,        //黄色
        YellowGreen = 0xFF9ACD32    //黄绿色
    };

    /** 常见颜色值，对应的字符串常量
    */
    void GetUiColorsString(std::vector<std::pair<std::wstring, int32_t>>& uiColors);

}// namespace UiColors
} // namespace ui

#endif // UI_CORE_UICOLORS_H_
