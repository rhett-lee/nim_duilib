#ifndef UI_CONTROL_ADDRESS_BAR_PATH_UTIL_H_
#define UI_CONTROL_ADDRESS_BAR_PATH_UTIL_H_

#include "duilib/duilib_defs.h"
#include <vector>

namespace ui
{
/** 地址栏路径分段信息
*/
struct DUILIB_API AddressBarPathSegment
{
    DString m_displayName;  //显示名称
    DString m_fullPath;     //完整的本地路径
    bool m_bRoot;           //是否为根节点（盘符、UNC根、Unix根）
    bool m_bDrive;          //是否为Windows盘符根
    bool m_bComputer;       //是否为“此电脑”（路径为空时的占位显示）
};

/** 展开以"~"开头的用户主目录路径
*/
DUILIB_API DString ExpandAddressBarUserPath(const DString& path);

/** 根据路径构建地址栏的路径段列表
*   支持UNC路径(\\\\server\\share)、扩展长度路径(\\\\?\\)、以"~"开头的用户主目录路径与相对路径
* @param [in] addressPath 需要解析的路径
* @param [in] emptyDisplayText 路径为空时的占位显示文本（如“此电脑”），为空则不生成占位段
* @param [out] segments 解析得到的路径段列表
*/
DUILIB_API void BuildAddressBarPathSegments(const DString& addressPath,
                                            const DString& emptyDisplayText,
                                            std::vector<AddressBarPathSegment>& segments);

}//namespace ui

#endif //UI_CONTROL_ADDRESS_BAR_PATH_UTIL_H_
