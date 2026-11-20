#ifndef UI_CONTROL_URL_BAR_UTIL_H_
#define UI_CONTROL_URL_BAR_UTIL_H_

#include "duilib/duilib_defs.h"

namespace ui
{
/** 获取文本中的URL协议名（不含冒号，统一为小写）
*   协议名要求以字母开头、长度不小于2，后续字符为字母、数字、'+'、'-'或'.'；
*   因此Windows盘符（如"C:"）不会被识别为协议。
* @param [in] text 待解析的文本
* @return 返回协议名；不包含协议时返回空字符串
*/
DUILIB_API DString UrlGetScheme(const DString& text);

/** 判断文本是否包含URL协议头（如 http://、https://、file://、about:）
*/
DUILIB_API bool UrlHasScheme(const DString& text);

/** 获取URL的主机名（不含协议、用户信息、端口与路径，统一为小写）
* @param [in] text 待解析的文本
* @return 返回主机名；无法解析时返回空字符串
*/
DUILIB_API DString UrlGetHost(const DString& text);

/** 判断输入文本是否像一个网址（用于决定是否自动补全"https://"）
*   判定规则：不含空白字符，且主机部分包含'.'、为"localhost"、为IPv4/IPv6地址或带端口。
*/
DUILIB_API bool UrlIsLikelyUrl(const DString& text);

/** 将本地文件路径转换为 file:// 形式的URL
*   路径中的反斜杠统一转换为斜杠，盘符路径转换为"file:///C:/..."，
*   UNC路径转换为"file://server/share/..."。
*/
DUILIB_API DString UrlMakeFileUrl(const DString& filePath);

/** 对文本进行UTF-8百分号编码（保留 A-Z、a-z、0-9、'-'、'_'、'.'、'~'）
*/
DUILIB_API DString UrlPercentEncode(const DString& text);

/** 归一化用户输入的网址
*   处理顺序：
*   1. 去除首尾空白，空文本返回空字符串；
*   2. 已包含明确的协议（带"://"，或 about:、mailto:、data: 等已知协议）时原样返回；
*      "localhost:8080"这类"主机:端口"不被当作协议，继续按网址处理；
*   3. 以"//"开头时补全为"https://"；
*   4. 本地绝对路径转换为 file:// URL；
*   5. 形如网址时补全为"https://"；
*   6. 配置了包含"%s"的搜索模板时，按搜索模板进行百分号编码后替换；
*   7. 其它情况（含空白）原样返回，否则补全为"https://"。
* @param [in] input 用户输入的文本
* @param [in] searchUrlTemplate 搜索URL模板，需包含"%s"占位符，可为空
* @return 返回归一化后的网址或搜索URL
*/
DUILIB_API DString UrlNormalizeInput(const DString& input, const DString& searchUrlTemplate);

}//namespace ui

#endif //UI_CONTROL_URL_BAR_UTIL_H_
