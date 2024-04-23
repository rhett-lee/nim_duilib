// Copyright (c) 2010-2011, duilib develop team(www.duilib.com).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or 
// without modification, are permitted provided that the 
// following conditions are met.
//
// Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above 
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials 
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "duilib_config.h"

#ifdef UILIB_IMPL_WINSDK

#if defined _M_IX86
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#endif //UILIB_IMPL_WINSDK

#include <commctrl.h>
#include <stddef.h>
#include <richedit.h>
#include <assert.h>
#include <crtdbg.h>
#include <malloc.h>
#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <unordered_set>
#include <queue>
#include <functional>

#include "duilib_defs.h"
#include "Utils/Macros.h"
#include "Utils/VersionHelpers.h"
#include "Utils/ApiWrapper.h"
#include "Utils/Delegate.h"
#include "Utils/StringUtil.h"
#include "Utils/OnScreenKeyboardManager.h"
#include "Utils/WinImplBase.h"
#include "Utils/ShadowWnd.h"

#include "Image/Image.h"
#include "Animation/AnimationPlayer.h"
#include "Animation/AnimationManager.h"

#include "Render/IRender.h"
#include "Render/AutoClip.h"

#include "duilib/Core/UiColor.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Core/UiRect.h"
#include "duilib/Core/UiSize.h"
#include "duilib/Core/UiPoint.h"
#include "Core/WindowBuilder.h"
#include "Core/GlobalManager.h"
#include "Core/Window.h"
#include "Core/Placeholder.h"
#include "Core/Control.h"
#include "Core/Box.h"
#include "Core/ScrollBar.h"
#include "Core/ControlDragable.h"

#include "Box/VBox.h"
#include "Box/HBox.h"
#include "Box/TileBox.h"
#include "Box/TabBox.h"
#include "Box/ScrollBox.h"
#include "Box/ListBox.h"
#include "Box/VirtualListBox.h"

#include "Control/Combo.h"
#include "Control/ComboButton.h"
#include "Control/FilterCombo.h"
#include "Control/CheckCombo.h"
#include "Control/TreeView.h"

#include "Control/Label.h"
#include "Control/Button.h"
#include "Control/CheckBox.h"
#include "Control/Option.h"
#include "Control/Split.h"

#include "Control/Progress.h"
#include "Control/CircleProgress.h"
#include "Control/Slider.h"

#include "Control/RichEdit.h"
#include "Control/RichText.h"
#include "Control/Menu.h"
#include "Control/DateTime.h"
#include "Control/Line.h"
#include "Control/IPAddress.h"
#include "Control/HotKey.h"
#include "Control/HyperLink.h"
#include "Control/ListCtrl.h"
#include "Control/PropertyGrid.h"
#include "Control/TabCtrl.h"

#include "Control/ColorPicker.h"
#include "Control/ColorControl.h"
#include "Control/ColorSlider.h"
#include "Control/ColorPickerRegular.h"
#include "Control/ColorPickerStatard.h"
#include "Control/ColorPickerStatardGray.h"
#include "Control/ColorPickerCustom.h"
