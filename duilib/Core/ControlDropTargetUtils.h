#ifndef UI_CORE_CONTROL_DROP_TARGET_UTILS_H_
#define UI_CORE_CONTROL_DROP_TARGET_UTILS_H_

#include "duilib/Core/UiTypes.h"

namespace ui 
{
/** 控件的拖放支持的辅助函数
*/
class ControlDropTargetUtils
{
public:
    /** 判断文件后缀是否满足配置要求
    */
    static bool IsFilteredFileTypes(const DString& fileTypes, const std::vector<DString>& fileList);

    /** 过滤掉不支持的文件类型
    */
    static void RemoveUnsupportedFiles(std::vector<DString>& fileList, const DString& fileTypes);

    /** 判断两个扩展名是否相同
    */
    static bool IsSameFileType(const DString& ext1, const DString& ext2);
};

} // namespace ui

#endif // UI_CORE_CONTROL_DROP_TARGET_UTILS_H_
