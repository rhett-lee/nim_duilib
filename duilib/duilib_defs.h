#ifndef DUILIB_DEFS_H_
#define DUILIB_DEFS_H_

#include "duilib/duilib_config.h"
#include <vector>

#define DUI_NOSET_VALUE        -1

namespace ui
{
    //定义所有控件类型
    #define  DUI_CTR_CONTROL                         (_T("Control"))
    #define  DUI_CTR_CONTROL_DRAGABLE                (_T("ControlDragable"))
    #define  DUI_CTR_LABEL                           (_T("Label"))
    #define  DUI_CTR_RICHTEXT                        (_T("RichText"))
    #define  DUI_CTR_HYPER_LINK                      (_T("HyperLink"))
    #define  DUI_CTR_BUTTON                          (_T("Button"))
    #define  DUI_CTR_OPTION                          (_T("Option"))
    #define  DUI_CTR_CHECKBOX                        (_T("CheckBox"))
    #define  DUI_CTR_SPLIT                           (_T("Split"))

    #define  DUI_CTR_LABELBOX                        (_T("LabelBox"))
    #define  DUI_CTR_LABELHBOX                       (_T("LabelHBox"))
    #define  DUI_CTR_LABELVBOX                       (_T("LabelVBox"))

    #define  DUI_CTR_BUTTONBOX                       (_T("ButtonBox"))
    #define  DUI_CTR_BUTTONHBOX                      (_T("ButtonHBox"))
    #define  DUI_CTR_BUTTONVBOX                      (_T("ButtonVBox"))

    #define  DUI_CTR_CHECKBOXBOX                     (_T("CheckBoxBox"))
    #define  DUI_CTR_CHECKBOXHBOX                    (_T("CheckBoxHBox"))
    #define  DUI_CTR_CHECKBOXVBOX                    (_T("CheckBoxVBox"))

    #define  DUI_CTR_OPTIONBOX                       (_T("OptionBox"))
    #define  DUI_CTR_OPTIONHBOX                      (_T("OptionHBox"))
    #define  DUI_CTR_OPTIONVBOX                      (_T("OptionVBox"))

    #define  DUI_CTR_SPLITBOX                        (_T("SplitBox"))

    #define  DUI_CTR_BOX                             (_T("Box"))
    #define  DUI_CTR_HBOX                            (_T("HBox"))
    #define  DUI_CTR_VBOX                            (_T("VBox"))
    #define  DUI_CTR_VTILE_BOX                       (_T("VTileBox"))
    #define  DUI_CTR_HTILE_BOX                       (_T("HTileBox"))

    #define  DUI_CTR_BOX_DRAGABLE                    (_T("BoxDragable"))
    #define  DUI_CTR_HBOX_DRAGABLE                   (_T("HBoxDragable"))
    #define  DUI_CTR_VBOX_DRAGABLE                   (_T("VBoxDragable"))

    #define  DUI_CTR_GROUP_BOX                       (_T("GroupBox"))
    #define  DUI_CTR_GROUP_HBOX                      (_T("GroupHBox"))
    #define  DUI_CTR_GROUP_VBOX                      (_T("GroupVBox"))

    #define  DUI_CTR_SCROLLBOX                       (_T("ScrollBox"))
    #define  DUI_CTR_HSCROLLBOX                      (_T("HScrollBox"))
    #define  DUI_CTR_VSCROLLBOX                      (_T("VScrollBox"))
    #define  DUI_CTR_HTILE_SCROLLBOX                 (_T("HTileScrollBox"))
    #define  DUI_CTR_VTILE_SCROLLBOX                 (_T("VTileScrollBox"))

    #define  DUI_CTR_LISTBOX_ITEM                    (_T("ListBoxItem"))
    #define  DUI_CTR_HLISTBOX                        (_T("HListBox"))
    #define  DUI_CTR_VLISTBOX                        (_T("VListBox"))
    #define  DUI_CTR_HTILE_LISTBOX                   (_T("HTileListBox"))
    #define  DUI_CTR_VTILE_LISTBOX                   (_T("VTileListBox"))

    #define  DUI_CTR_LISTCTRL                        (_T("ListCtrl"))
    #define  DUI_CTR_PROPERTY_GRID                   (_T("PropertyGrid"))

    #define  DUI_CTR_VIRTUAL_VLISTBOX                (_T("VirtualVListBox"))
    #define  DUI_CTR_VIRTUAL_HLISTBOX                (_T("VirtualHListBox"))
    #define  DUI_CTR_VIRTUAL_HTILE_LISTBOX           (_T("VirtualHTileListBox"))
    #define  DUI_CTR_VIRTUAL_VTILE_LISTBOX           (_T("VirtualVTileListBox"))

    #define  DUI_CTR_TABBOX                          (_T("TabBox"))

    #define  DUI_CTR_TREENODE                        (_T("TreeNode"))
    #define  DUI_CTR_TREEVIEW                        (_T("TreeView"))
    #define  DUI_CTR_DIRECTORY_TREE                  (_T("DirectoryTree"))

    #define  DUI_CTR_RICHEDIT                        (_T("RichEdit"))
    #define  DUI_CTR_COMBO                           (_T("Combo"))
    #define  DUI_CTR_COMBO_BUTTON                    (_T("ComboButton"))
    #define  DUI_CTR_FILTER_COMBO                    (_T("FilterCombo"))
    #define  DUI_CTR_CHECK_COMBO                     (_T("CheckCombo"))
    #define  DUI_CTR_SLIDER                          (_T("Slider"))
    #define  DUI_CTR_PROGRESS                        (_T("Progress"))
    #define  DUI_CTR_CIRCLEPROGRESS                  (_T("CircleProgress"))
    #define  DUI_CTR_SCROLLBAR                       (_T("ScrollBar"))

    #define  DUI_CTR_MENU                            (_T("Menu"))
    #define  DUI_CTR_SUB_MENU                        (_T("SubMenu"))
    #define  DUI_CTR_MENU_ITEM                       (_T("MenuItem"))
    #define  DUI_CTR_MENU_LISTBOX                    (_T("MenuListBox"))

    #define  DUI_CTR_DATETIME                        (_T("DateTime"))
    #define  DUI_CTR_CEF                             (_T("CefControl"))
    #define  DUI_CTR_WEBVIEW2                        (_T("WebView2Control"))

    //颜色相关的部分控件
    #define  DUI_CTR_COLOR_CONTROL                   (_T("ColorControl"))
    #define  DUI_CTR_COLOR_SLIDER                    (_T("ColorSlider"))
    #define  DUI_CTR_COLOR_PICKER_REGULAR            (_T("ColorPickerRegular"))
    #define  DUI_CTR_COLOR_PICKER_STANDARD           (_T("ColorPickerStatard"))
    #define  DUI_CTR_COLOR_PICKER_STANDARD_GRAY      (_T("ColorPickerStatardGray"))
    #define  DUI_CTR_COLOR_PICKER_CUSTOM             (_T("ColorPickerCustom"))

    #define  DUI_CTR_LINE                            (_T("Line"))
    #define  DUI_CTR_IPADDRESS                       (_T("IPAddress"))
    #define  DUI_CTR_HOTKEY                          (_T("HotKey"))
    #define  DUI_CTR_TAB_CTRL                        (_T("TabCtrl"))
    #define  DUI_CTR_TAB_CTRL_ITEM                   (_T("TabCtrlItem"))
    #define  DUI_CTR_ICON_CONTROL                    (_T("IconControl"))
    #define  DUI_CTR_ADDRESS_BAR                     (_T("AddressBar"))

    // 窗口标题栏按钮：最大化、最小化、关闭、还原、全屏窗口的名字，代码中写死的
    #define  DUI_CTR_CAPTION_BAR                     (_T("window_caption_bar"))
    #define  DUI_CTR_BUTTON_CLOSE                    (_T("closebtn"))
    #define  DUI_CTR_BUTTON_MIN                      (_T("minbtn"))
    #define  DUI_CTR_BUTTON_MAX                      (_T("maxbtn"))
    #define  DUI_CTR_BUTTON_RESTORE                  (_T("restorebtn"))
    #define  DUI_CTR_BUTTON_FULLSCREEN               (_T("fullscreenbtn"))

    //EventType事件类型对应的字符串定义
    #define  EVENTSTR_ALL                (_T("all"))
    #define  EVENTSTR_KEYDOWN            (_T("keydown"))
    #define  EVENTSTR_KEYUP              (_T("keyup"))
    #define  EVENTSTR_CHAR               (_T("char"))
    
    #define  EVENTSTR_SETFOCUS           (_T("setfocus"))
    #define  EVENTSTR_KILLFOCUS          (_T("killfocus"))
    #define  EVENTSTR_SETCURSOR          (_T("setcursor"))

    #define  EVENTSTR_MOUSEMOVE          (_T("mousemove"))
    #define  EVENTSTR_MOUSEENTER         (_T("mouseenter"))
    #define  EVENTSTR_MOUSELEAVE         (_T("mouseleave"))
    #define  EVENTSTR_MOUSEHOVER         (_T("mousehover"))

    #define  EVENTSTR_BUTTONDOWN         (_T("buttondown"))
    #define  EVENTSTR_BUTTONUP           (_T("buttonup"))
    #define  EVENTSTR_DOUBLECLICK        (_T("doubleclick"))
    #define  EVENTSTR_RBUTTONDOWN        (_T("rbuttondown"))
    #define  EVENTSTR_RBUTTONUP          (_T("rbuttonup"))
    #define  EVENTSTR_RDOUBLECLICK       (_T("rdoubleclick"))

    #define  EVENTSTR_SELECT             (_T("select"))
    #define  EVENTSTR_UNSELECT           (_T("unselect"))

    #define  EVENTSTR_CHECKED            (_T("checked"))
    #define  EVENTSTR_UNCHECK            (_T("uncheck"))
    
    #define  EVENTSTR_TAB_SELECT         (_T("tab_select"))

    #define  EVENTSTR_MENU               (_T("menu"))

    #define  EVENTSTR_EXPAND             (_T("expand"))
    #define  EVENTSTR_COLLAPSE           (_T("collapse"))

    #define  EVENTSTR_SCROLLWHEEL        (_T("scrollwheel"))
    #define  EVENTSTR_SCROLLCHANGE       (_T("scrollchange"))

    #define  EVENTSTR_VALUECHANGE        (_T("valuechange"))
    #define  EVENTSTR_RETURN             (_T("return"))
    #define  EVENTSTR_TAB                (_T("tab"))
    #define  EVENTSTR_WINDOWCLOSE        (_T("windowclose"))

    //水平对齐方式
    enum HorAlignType: int8_t
    {
        kHorAlignLeft   = 0,    // 靠左对齐
        kHorAlignCenter = 1,    // 水平居中
        kHorAlignRight  = 2     // 靠右对齐
    };

    //垂直对齐方式
    enum VerAlignType: int8_t
    {
        kVerAlignTop    = 0,    // 顶端对齐
        kVerAlignCenter = 1,    // 垂直居中
        kVerAlignBottom = 2     // 底部对齐
    };

    //控件图片类型
    enum StateImageType: int8_t
    {
        kStateImageBk               = 0,    //背景图片（Control在用）
        kStateImageFore             = 1,    //前景图片（Control在用）
        kStateImageSelectedBk       = 2,    //选择时的背景图片（CheckBox在用）
        kStateImageSelectedFore     = 3,    //选择时的前景图片（CheckBox在用）
        kStateImagePartSelectedBk   = 4,    //部分选择时的背景图片（CheckBox在用）
        kStateImagePartSelectedFore = 5,    //部分选择时的前景图片（CheckBox在用）
    };

    //控件状态
    enum ControlStateType: int8_t
    {
        kControlStateNormal     = 0,    // 普通状态
        kControlStateHot        = 1,    // 悬停状态
        kControlStatePushed     = 2,    // 按下状态
        kControlStateDisabled   = 3     // 禁用状态
    };

    //控件动画类型
    enum class AnimationType: int8_t
    {
        kAnimationNone              = 0,    //无动画
        kAnimationAlpha             = 1,    //透明度渐变动画
        kAnimationHeight            = 2,    //控件高度变化动画
        kAnimationWidth             = 3,    //控件宽度变化动画
        kAnimationHot               = 4,    //鼠标悬停状态的动画
        kAnimationInoutXFromLeft    = 5,    //控件的X坐标变化动画，从左侧
        kAnimationInoutXFromRight   = 6,    //控件的X坐标变化动画，从右侧
        kAnimationInoutYFromTop     = 7,    //控件的Y坐标变化动画，从上侧
        kAnimationInoutYFromBottom  = 8,    //控件的Y坐标变化动画，从下侧
    };

    //GIF图片帧类型
    enum GifFrameType: int8_t
    {
        kGifFrameFirst   = 0,   // 第一帧    
        kGifFrameCurrent = 1,   // 当前帧        
        kGifFrameLast    = 2    // 最后一帧
    };

    //光标: Windows平台可参考：https://learn.microsoft.com/zh-cn/windows/win32/menurc/about-cursors
    enum class CursorType : uint8_t
    {
        kCursorArrow    = 0,    // 箭头, 正常选择, XML文件中的名字："arrow"
        kCursorIBeam    = 1,    // “I”形状, 文本选择, XML文件中的名字："ibeam"
        kCursorHand     = 2,    // 手型, 链接选择, XML文件中的名字："hand"
        kCursorWait     = 3,    // 忙碌, XML文件中的名字："wait"
        kCursorCross    = 4,    // 十字线, XML文件中的名字："cross"
        kCursorSizeWE   = 5,    // 水平调整, XML文件中的名字："size_we"
        kCursorSizeNS   = 6,    // 垂直调整, XML文件中的名字："size_ns"
        kCursorSizeNWSE = 7,    // 对角线调整，西北-东南调整 1, XML文件中的名字："size_nwse"
        kCursorSizeNESW = 8,    // 对角线调整，东北-西南调整 2, XML文件中的名字： "size_nesw"
        kCursorSizeAll  = 9,    // 移动，四向调整, XML文件中的名字："size_all"
        kCursorNo       = 10,   // 禁止光标, XML文件中的名字："no"
        kCursorProgress = 11    // 进度，应用启动光标, XML文件中的名字："progress"
    };

    //窗口退出参数
    enum WindowCloseParam
    {
        kWindowCloseNormal  = 0,    //表示点击 "关闭" 按钮关闭本窗口(默认值)
        kWindowCloseOK      = 1,    //表示点击 "确认" 按钮关闭本窗口
        kWindowCloseCancel  = 2     //表示点击 "取消" 按钮关闭本窗口
    };

    //拖放类型
    enum ControlDropType
    {
        kControlDropTypeWindows = 0, //表示来自ControlDropTarget_Windows接口的事件
        kControlDropTypeSDL     = 1, //表示来自ControlDropTarget_SDL的事件
    };

    //Windows平台的拖放数据：相关的值，可以参考IDropTarget的接口声明
    struct ControlDropData_Windows
    {
        void* m_pDataObj;       //IDataObject*
        uint32_t m_grfKeyState; //键盘状态
        int32_t m_screenX;      //鼠标所在点的X坐标，屏幕坐标
        int32_t m_screenY;      //鼠标所在点的Y坐标，屏幕坐标
        uint32_t m_dwEffect;    //参数返回值
        int32_t m_hResult;      //函数返回值
        std::vector<DString> m_textList;    // 在m_pDataObj中包含的文本内容，每个元素代表一行
        std::vector<DString> m_fileList;    // 在m_pDataObj中包含的文本内容，每个元素代表一个文件路径
    };

    //SDL的拖放数据
    struct ControlDropData_SDL
    {
        bool m_bTextData;                   //true表示m_textList为有效数据，false表示m_fileList为有效数据
        std::vector<DString> m_textList;    // 在拖放操作中包含的文本内容，每个元素代表一行

        DString m_source;                   // 当m_bTextData为false时有效
        std::vector<DString> m_fileList;    // 在拖放操作中包含的文本内容，每个元素代表一个文件路径
    };

    //定义所有消息类型
    enum EventType: int8_t
    {
        kEventNone,
        kEventAll,                  //代表所有消息（无参数关联数据）
        kEventDestroy,              //控件销毁（最后一个消息）

        //键盘消息
        kEventKeyBegin,
        kEventKeyDown,              //Window类：当收到WM_KEYDOWN消息时触发，发送给Focus控件
        kEventKeyUp,                //Window类：当收到WM_KEYUP消息时触发，发送给WM_KEYDOWN事件中的那个Focus控件
        kEventChar,                 //Window类：当收到WM_CHAR消息时触发，发送给WM_KEYDOWN事件中的那个Focus控件
        kEventKeyEnd,

        //鼠标消息
        kEventMouseBegin,        
        kEventMouseEnter,           //Window类：当收到WM_MOUSEMOVE消息时触发，发送给MouseMove时，鼠标进入的那个控件，表明鼠标进入到这个控件内
        kEventMouseLeave,           //Window类：当收到WM_MOUSEMOVE消息时触发，发送给MouseMove时，鼠标进入的那个控件，表明鼠标已经离开这个控件
        kEventMouseMove,            //Window类：当收到WM_MOUSEMOVE消息时触发，发送给鼠标左键、右键、双击时记录的那个控件
        kEventMouseHover,           //Window类：当收到WM_MOUSEHOVER消息时触发，发送给MouseMove时，鼠标进入的那个控件，表明鼠标在这个控件内悬停
        kEventMouseWheel,           //Window类：当收到WM_MOUSEWHEEL消息时触发，发送给当前鼠标所在位置对应的控件
        kEventMouseButtonDown,      //Window类：当收到WM_LBUTTONDOWN消息时触发，发送给鼠标左键按下时对应的控件
        kEventMouseButtonUp,        //Window类：当收到WM_LBUTTONUP消息时触发，发送给鼠标左键按下时对应的控件
        kEventMouseDoubleClick,     //Window类：当收到WM_LBUTTONDBLCLK消息时触发，发送给当前鼠标所在位置对应的控件        
        kEventMouseRButtonDown,     //Window类：当收到WM_RBUTTONDOWN消息时触发，发送给鼠标右键按下时对应的控件
        kEventMouseRButtonUp,       //Window类：当收到WM_RBUTTONUP消息时触发，发送给鼠标右键按下时对应的控件
        kEventMouseRDoubleClick,    //Window类：当收到WM_RBUTTONDBLCLK消息时触发，发送给当前鼠标所在位置对应的控件
        kEventMouseMButtonDown,     //Window类：当收到WM_MBUTTONDOWN消息时触发，发送给鼠标右键按下时对应的控件
        kEventMouseMButtonUp,       //Window类：当收到WM_MBUTTONUP消息时触发，发送给鼠标右键按下时对应的控件
        kEventMouseMDoubleClick,    //Window类：当收到WM_MBUTTONDBLCLK消息时触发，发送给当前鼠标所在位置对应的控件
        kEventContextMenu,          //Window类：当收到WM_CONTEXTMENU消息时触发，发送给所有注册回调函数的控件
        kEventMouseEnd,

        //焦点/光标/输入法相关消息
        kEventSetFocus,             //Window类：发送给Focus控件，当控件获得焦点时触发事件（控件焦点不是窗口焦点，两者完全不同）
        kEventKillFocus,            //Window类：发送给Focus控件，当控件失去焦点时触发事件（控件焦点不是窗口焦点，两者完全不同）        
        kEventSetCursor,            //Window类：发送给当前鼠标所在控件，设置光标
        kEventCaptureChanged,       //Window类：发送给Focus控件，当收到WM_CAPTURECHANGED消息时触发
        kEventImeSetContext,        //Window类：发送给Focus控件，当收到系统WM_IME_SETCONTEXT消息时触发
        kEventImeStartComposition,  //Window类：发送给Focus控件，当收到系统WM_IME_STARTCOMPOSITION消息时触发        
        kEventImeComposition,       //Window类：发送给Focus控件，当收到系统WM_IME_COMPOSITION消息时触发
        kEventImeEndComposition,    //Window类：发送给Focus控件，当收到系统WM_IME_ENDCOMPOSITION消息时触发

        kEventWindowSetFocus,       //Window类：发送给焦点控件，当窗口获得焦点时触发事件；发送给接收者的回调函数（wParam: 已失去键盘焦点的窗口的句柄）
        kEventWindowKillFocus,      //Window类：发送给鼠标左键、右键按下时记录的控件、焦点控件，当窗口失去焦点时触发事件（主要用于恢复一些内部状态）；发送给接收者的回调函数（wParam: 接收键盘焦点的窗口的句柄）
        kEventWindowSize,           //Window类：发送给Focus控件，当窗口大小发生变化时触发事件
        kEventWindowMove,           //Window类：发送给Focus控件，当窗口大小发生变化时触发事件        
        kEventWindowCreate,         //Window类，当窗口创建完成时触发，wParam为1表示DoModal对话框，为0表示普通窗口
        kEventWindowClose,          //Window类，Combo控件：当窗口关闭（或者Combo的下拉框窗口关闭）时触发，wParam参数表示窗口关闭参数，参见enum WindowCloseParam
        kEventWindowFirstShown,     //Window类，当窗口第一次显示时回调此事件（必须在界面显示前设置回调，即当IsWindowFirstShown()返回false的情况下设置，否则没有机会再回调）

        //左键点击/右键点击事件
        kEventClick,                //Button类、ListBoxItem、Option、CheckBox等：当点击按钮（或者键盘回车）时触发
        kEventRClick,               //Control类：当鼠标右键弹起时触发

        //鼠标点击控件发生变化，发给旧的鼠标点击事件
        kEventMouseClickChanged,    //Window类：当收到鼠标按下相关消息时触发，发送给上次鼠标按下时的控件
        kEventMouseClickEsc,        //Window类：当收到键盘ESC键消息时触发，发送给鼠标按下时的控件

        //选择/取消选择
        kEventSelect,               //CheckBox类、Option类、ListBox类、Combo类：当变成选中状态时触发
        kEventUnSelect,             //CheckBox类、Option类、ListBox类、Combo类：当变成非选中状态时触发

        //勾选/取消勾选
        kEventChecked,              //CheckBox类：当变成Checked状态时触发
        kEventUnCheck,              //CheckBox类：当变成UnCheck状态时触发

        //TAB页面选择
        kEventTabSelect,            //TabBox类，选中TAB页面, WPARAM是新页面ID，LPARAM是旧页面ID

        //树节点展开/收起
        kEventExpand,               //TreeNode类：当树节点展开时触发
        kEventCollapse,             //TreeNode类：当树节点收起时触发

        kEventZoom,                 //RichEdit类：当缩放比例发生变化时，wParam表示缩放百分比，比如200表示200%
        kEventTextChange,           //RichEdit类：当文本内容发生变化时触发
        kEventSelChange,            //ListCtrl类：选择项发生变化，RichEdit类：当文本选择内容发生变化时触发
        kEventReturn,               //ListCtrl、VirtualListBox、ListBoxItem、RichEdit、AddressBar类：当收到回车键时触发
        kEventEsc,                  //RichEdit类、AddressBar类：当收到ESC键时触发
        kEventTab,                  //RichEdit类：在WantTab为false时，当收到TAB键时触发
        kEventLinkClick,            //RichEdit类、RichText类：当点击到超级链接的数据上时触发, 可以通过WPARAM获取点击的URL，类型为const DStringW::value_type*

        kEventScrollChange,         //ScrollBox类：当滚动条位置发生变化时触发
        kEventValueChange,          //DateTime、Slider类：当值发生变化时触发, Slider类：WPARAM是新值，LPARAM是旧值
        kEventResize,               //Control类：当控件的大小和位置发生变化时触发
        kEventVisibleChange,        //Control类：当控件的Visible属性发生变化时触发, WPARAM是新状态(1表示可见，0表示不可见)
        kEventStateChange,          //Control类：当控件的State属性发生变化时触发, WPARAM是新状态，LPARAM是旧状态
        kEventSelectColor,          //ColorPicker类：当选择了新颜色时触发, WPARAM是新的颜色值，LPARAM是旧的颜色值

        kEventSplitDraged,          //Split类：通过拖动操作调整了控件的大小，WPARAM是第一个控件接口，LPARAM是第二个控件接口

        kEventEnterEdit,            //ListCtrl类：进入编辑状态, wParam 是接口：ListCtrlEditParam*
        kEventLeaveEdit,            //ListCtrl类：退出编辑状态, wParam 是接口：ListCtrlEditParam*

        kEventPathChanged,          //AddressBar类：当路径方式变化时触发
        kEventPathClick,            //AddressBar类：当用户点击路径按钮时触发

        kEventDropEnter,            //wParam 是ControlDropType，代表来源类型，
                                    //lParam 代表关联数据：当wParam为kControlDropTypeWindows时，lParam是ControlDropData_Windows的指针
        kEventDropOver,             //wParam 是ControlDropType，代表来源类型，
                                    //lParam 代表关联数据：当wParam为kControlDropTypeWindows时，lParam是ControlDropData_Windows的指针
        kEventDropLeave,            //无额外描述
        kEventDropData,             //wParam 是ControlDropType，代表来源类型，
                                    //lParam 代表关联数据：当wParam为kControlDropTypeWindows时，lParam是ControlDropData_Windows的指针
                                    //                   当wParam为kControlDropTypeSDL时，lParam是ControlDropData_SDL的指针

        kEventLast                  //无使用者
    };

    /** 热键组合键标志位
    */
    enum HotKeyModifiers
    {
        kHotKey_Shift = 0x01,   //Shift键
        kHotKey_Contrl = 0x02,  //Contrl键
        kHotKey_Alt = 0x04,     //Alt键
        kHotKey_Ext = 0x08      //扩展键
    };

    /** 鼠标操作的标志
    */
    enum VKFlag
    {
        kVkLButton  = 0x0001, //MK_LBUTTON,     //按下了鼠标左键
        kVkRButton  = 0x0002, //MK_RBUTTON,     //按下了鼠标右键
        kVkShift    = 0x0004, //MK_SHIFT,       //按下了 SHIFT 键
        kVkControl  = 0x0008  //MK_CONTROL,     //按下了 CTRL 键
    };

}// namespace ui

#endif //DUILIB_DEFS_H_


