#ifndef UI_UTILS_PROCESS_SINGLETON_DATA_H_
#define UI_UTILS_PROCESS_SINGLETON_DATA_H_

#include "duilib/duilib_defs.h"

#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <cstring>

namespace ui
{
/** 跨进程单例的实现（通信协议部分）
*  负责进程间传递数据的序列化与反序列化：
*  数据包 = 协议头(ProtocolHeader) + 负载(各个参数以单元分隔符 0x1E 连接)。
*/
class DUILIB_API ProcessSingletonData
{
public:
    /** 协议头定义
    */
    struct ProtocolHeader {
        uint32_t dwMagic;
        uint32_t dwVersion;
        uint32_t dwDataLength;
        uint32_t dwReserved;
    };
    
    /** 协议头的Magic数字
    */
    static constexpr uint32_t PROTOCOL_MAGIC = 0xC0DAFE12;
    
    /** 协议版本号
    */
    static constexpr uint32_t PROTOCOL_VERSION = 202503;
    
    /** 每次通信数据的最大长度
    */
    static constexpr size_t MAX_DATA_SIZE = 4096;
    
public:
    /** 序列化协议：把参数列表打包成可发送的数据包
    * @param [in] vecArgs 需要序列化的参数，单条数据最大1024字节，数据总长度最大4096字节（MAX_DATA_SIZE）
    * @return 返回序列化后的数据包（协议头 + 负载）
    * @throw std::invalid_argument 单条参数超过1024字节时抛出
    * @throw std::length_error 参数总长度超过 MAX_DATA_SIZE 时抛出
    */
    static std::string SerializeArguments(const std::vector<std::string>& vecArgs) 
    {
        std::stringstream ssData;
        for (const auto& strArg : vecArgs) {
            if (strArg.length() > 1024) {
                throw std::invalid_argument("Argument exceeds maximum length");
            }
            ssData << strArg << '\x1E';  // 使用单元分隔符
        }

        std::string strPayload = ssData.str();
        if (strPayload.size() > MAX_DATA_SIZE) {
            throw std::length_error("Total arguments exceed buffer size");
        }

        ProtocolHeader stHeader{
            PROTOCOL_MAGIC,
            PROTOCOL_VERSION,
            static_cast<uint32_t>(strPayload.size()),
            0
        };

        std::string strPackage(reinterpret_cast<char*>(&stHeader), sizeof(stHeader));
        strPackage += strPayload;
        return strPackage;
    }

    /** 反序列化协议：把收到的数据包解析为参数列表
    * @param [in] strPackage 收到的数据包（协议头 + 负载）
    * @return 返回解析出的参数列表；参数为空时返回空列表
    * @throw std::runtime_error 数据包长度不足、魔数不匹配、协议版本过高或负载长度超限时抛出
    */
    static std::vector<std::string> DeserializeData(const std::string& strPackage) 
    {
        if (strPackage.size() < sizeof(ProtocolHeader)) {
            throw std::runtime_error("Invalid package header");
        }

        ProtocolHeader stHeader;
        memcpy(&stHeader, strPackage.data(), sizeof(stHeader));

        if (stHeader.dwMagic != PROTOCOL_MAGIC) {
            throw std::runtime_error("Protocol magic mismatch");
        }

        if (stHeader.dwVersion > PROTOCOL_VERSION) {
            throw std::runtime_error("Unsupported protocol version");
        }

        if (stHeader.dwDataLength > MAX_DATA_SIZE) {
            throw std::runtime_error("Data length exceeds limit");
        }

        std::vector<std::string> vecArgs;
        std::string strPayload = strPackage.substr(sizeof(stHeader), stHeader.dwDataLength);
        std::string::size_type nPos = 0;
        while ((nPos = strPayload.find('\x1E')) != std::string::npos) {
            vecArgs.push_back(strPayload.substr(0, nPos));
            strPayload.erase(0, nPos + 1);
        }
        return vecArgs;
    }
};

}

#endif // UI_UTILS_PROCESS_SINGLETON_DATA_H_
