#include "UrlBarUtil.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

namespace
{
/** 是否为ASCII字母
*/
bool IsAsciiAlpha(DString::value_type ch)
{
    return ((ch >= _T('a')) && (ch <= _T('z'))) || ((ch >= _T('A')) && (ch <= _T('Z')));
}

/** 是否为ASCII数字
*/
bool IsAsciiDigit(DString::value_type ch)
{
    return (ch >= _T('0')) && (ch <= _T('9'));
}

/** 判断文本是否以明确的URL协议开头（用于地址栏输入归一化）
*   带"://"的协议，或者已知的无"//"协议（mailto、about、data等）才视为明确协议；
*   避免把"localhost:8080"这类"主机:端口"误判为协议。
*/
bool HasExplicitProtocol(const DString& text)
{
    const DString scheme = UrlGetScheme(text);
    if (scheme.empty()) {
        return false;
    }
    //带"://"的协议：如 https://、file:///
    if (text.find(_T("://")) == scheme.size()) {
        return true;
    }
    //已知的无"//"协议
    return (scheme == _T("about")) || (scheme == _T("mailto")) || (scheme == _T("data")) ||
           (scheme == _T("tel")) || (scheme == _T("javascript")) || (scheme == _T("blob")) ||
           (scheme == _T("view-source")) || (scheme == _T("file")) || (scheme == _T("ftp")) ||
           (scheme == _T("ws")) || (scheme == _T("wss"));
}

/** 是否为IPv4地址（仅由数字和点组成，且包含3个点）
*/
bool IsIpv4Host(const DString& host)
{
    int32_t nDotCount = 0;
    for (DString::value_type ch : host) {
        if (ch == _T('.')) {
            ++nDotCount;
        }
        else if (!IsAsciiDigit(ch)) {
            return false;
        }
    }
    return nDotCount == 3;
}
}//namespace

DString UrlGetScheme(const DString& text)
{
    const DString t = StringUtil::Trim(text.c_str());
    const size_t nColon = t.find(_T(':'));
    if ((nColon == DString::npos) || (nColon < 2)) {
        //协议名至少2个字符，避免将Windows盘符（"C:"）识别为协议
        return DString();
    }
    for (size_t i = 0; i < nColon; ++i) {
        const DString::value_type ch = t[i];
        if (i == 0) {
            if (!IsAsciiAlpha(ch)) {
                return DString();
            }
        }
        else {
            if (!IsAsciiAlpha(ch) && !IsAsciiDigit(ch) &&
                (ch != _T('+')) && (ch != _T('-')) && (ch != _T('.'))) {
                return DString();
            }
        }
    }
    return StringUtil::MakeLowerString(t.substr(0, nColon));
}

bool UrlHasScheme(const DString& text)
{
    return !UrlGetScheme(text).empty();
}

DString UrlGetHost(const DString& text)
{
    const DString t = StringUtil::Trim(text.c_str());
    const size_t nSchemeEnd = t.find(_T("://"));
    if (nSchemeEnd == DString::npos) {
        return DString();
    }
    size_t nStart = nSchemeEnd + 3;
    if (nStart >= t.size()) {
        return DString();
    }
    size_t nEnd = t.find_first_of(_T("/?#"), nStart);
    DString authority = (nEnd == DString::npos) ? t.substr(nStart) : t.substr(nStart, nEnd - nStart);

    //去掉用户信息（user:password@host）
    const size_t nAt = authority.rfind(_T('@'));
    if (nAt != DString::npos) {
        authority = authority.substr(nAt + 1);
    }
    if (authority.empty()) {
        return DString();
    }
    //IPv6地址：[::1]:8080
    if (authority[0] == _T('[')) {
        const size_t nBracketEnd = authority.find(_T(']'));
        if (nBracketEnd != DString::npos) {
            return StringUtil::MakeLowerString(authority.substr(0, nBracketEnd + 1));
        }
        return StringUtil::MakeLowerString(authority);
    }
    //去掉端口
    const size_t nColon = authority.find(_T(':'));
    if (nColon != DString::npos) {
        authority = authority.substr(0, nColon);
    }
    return StringUtil::MakeLowerString(authority);
}

bool UrlIsLikelyUrl(const DString& text)
{
    DString t = StringUtil::Trim(text.c_str());
    if (t.empty()) {
        return false;
    }
    if (t.find_first_of(_T(" \t\r\n")) != DString::npos) {
        return false;
    }
    //包含反斜杠的文本是本地路径，不是网址
    if (t.find(_T('\\')) != DString::npos) {
        return false;
    }
    //以单个"/"开头的是本地路径，不是网址
    if ((t.size() >= 1) && (t[0] == _T('/')) &&
        ((t.size() == 1) || (t[1] != _T('/')))) {
        return false;
    }
    //协议相对地址："//example.com/path"
    if ((t.size() >= 2) && (t[0] == _T('/')) && (t[1] == _T('/'))) {
        t = t.substr(2);
    }

    size_t nHostEnd = t.find_first_of(_T("/?#"));
    DString host = (nHostEnd == DString::npos) ? t : t.substr(0, nHostEnd);
    const size_t nAt = host.rfind(_T('@'));
    if (nAt != DString::npos) {
        host = host.substr(nAt + 1);
    }
    if (host.empty()) {
        return false;
    }
    //IPv6地址
    if (host[0] == _T('[')) {
        return true;
    }
    bool bHasPort = false;
    const size_t nColon = host.find(_T(':'));
    if (nColon != DString::npos) {
        bHasPort = true;
        host = host.substr(0, nColon);
    }
    if (host.empty()) {
        return false;
    }
    const DString lowerHost = StringUtil::MakeLowerString(host);
    if (lowerHost == _T("localhost")) {
        return true;
    }
    if (IsIpv4Host(lowerHost)) {
        return true;
    }
    //域名：包含"."且不在首尾位置
    if ((lowerHost.find(_T('.')) != DString::npos) &&
        (lowerHost.front() != _T('.')) && (lowerHost.back() != _T('.'))) {
        return true;
    }
    //单标签主机名带端口（如 "server:8080"）
    return bHasPort;
}

DString UrlMakeFileUrl(const DString& filePath)
{
    const DString normalizedPath = FilePathUtil::NormalizeFilePath(filePath);
    DString path = normalizedPath.empty() ? filePath : normalizedPath;
    //URL中使用正斜杠
    for (DString::value_type& ch : path) {
        if (ch == _T('\\')) {
            ch = _T('/');
        }
    }
    if (path.empty()) {
        return DString();
    }
    if ((path.size() >= 2) && (path[0] == _T('/')) && (path[1] == _T('/'))) {
        //UNC路径："\\server\share" → "file://server/share"
        return DString(_T("file:")) + path;
    }
    if (path[0] == _T('/')) {
        //Unix绝对路径："/home/user" → "file:///home/user"
        return DString(_T("file://")) + path;
    }
    //Windows盘符路径："C:/dir/file" → "file:///C:/dir/file"
    return DString(_T("file:///")) + path;
}

DString UrlPercentEncode(const DString& text)
{
    static const char kHexChars[] = "0123456789ABCDEF";
    const std::string utf8Text = StringConvert::TToUTF8(text);
    std::string encoded;
    encoded.reserve(utf8Text.size() * 3);
    for (const char ch : utf8Text) {
        const unsigned char byte = static_cast<unsigned char>(ch);
        const bool bUnreserved = ((byte >= 'A') && (byte <= 'Z')) ||
                                 ((byte >= 'a') && (byte <= 'z')) ||
                                 ((byte >= '0') && (byte <= '9')) ||
                                 (byte == '-') || (byte == '_') || (byte == '.') || (byte == '~');
        if (bUnreserved) {
            encoded.push_back(ch);
        }
        else {
            encoded.push_back('%');
            encoded.push_back(kHexChars[(byte >> 4) & 0x0F]);
            encoded.push_back(kHexChars[byte & 0x0F]);
        }
    }
    return StringConvert::UTF8ToT(encoded);
}

DString UrlNormalizeInput(const DString& input, const DString& searchUrlTemplate)
{
    const DString text = StringUtil::Trim(input.c_str());
    if (text.empty()) {
        return DString();
    }
    //已包含明确的协议：原样返回
    if (HasExplicitProtocol(text)) {
        return text;
    }
    //协议相对地址
    if ((text.size() >= 2) && (text[0] == _T('/')) && (text[1] == _T('/'))) {
        return DString(_T("https:")) + text;
    }
    //本地绝对路径：转换为file:// URL
    FilePath localPath(text.c_str());
    if (localPath.IsAbsolutePath()) {
        return UrlMakeFileUrl(text);
    }
    //形如网址：补全https协议
    if (UrlIsLikelyUrl(text)) {
        return DString(_T("https://")) + text;
    }
    //使用搜索模板
    const DString urlTemplate = StringUtil::Trim(searchUrlTemplate.c_str());
    const size_t nPlaceholder = urlTemplate.find(_T("%s"));
    if (!urlTemplate.empty() && (nPlaceholder != DString::npos)) {
        DString result = urlTemplate;
        result.replace(nPlaceholder, 2, UrlPercentEncode(text));
        return result;
    }
    //无法识别且含空白：原样返回；否则按域名补全https
    if (text.find_first_of(_T(" \t\r\n")) != DString::npos) {
        return text;
    }
    return DString(_T("https://")) + text;
}

}//namespace ui
