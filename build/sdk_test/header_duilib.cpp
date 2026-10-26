// 头文件用例：单独包含公开的伞形头文件。
// 该文件参与编译即证明 include/duilib/duilib.h 及其依赖在发布包内自包含。

#include "duilib/duilib.h"

namespace sdk_test {

void HeaderDuilibCompiled()
{
    // 引用一个公开类型，避免空翻译单元；不产生运行期依赖。
    static_assert(sizeof(ui::Control) > 0, "ui::Control 必须可见");
}

} // namespace sdk_test
