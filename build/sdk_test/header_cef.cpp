// CEF 头文件用例：仅当包元数据声明 HAS_CEF=1 时参与编译。
// 若包声明支持 CEF 却未随包转发 CEFControl/ 与第三方头文件，本文件会编译失败。

#include "duilib/duilib_cef.h"

namespace sdk_test {

using CefManagerCheck = ui::CefManager;

void HeaderCefCompiled()
{
}

} // namespace sdk_test
