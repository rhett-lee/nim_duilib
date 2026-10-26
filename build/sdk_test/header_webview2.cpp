// WebView2 头文件用例：仅当包元数据声明 HAS_WEBVIEW2=1 时参与编译。
// WebView2 的开发头文件只随 Windows 包转发，缺失时会在这里暴露。

#include "duilib/duilib_webview2.h"

namespace sdk_test {

using WebView2ManagerCheck = ui::WebView2Manager;

void HeaderWebView2Compiled()
{
}

} // namespace sdk_test
