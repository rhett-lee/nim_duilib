#include "ProcessSingleton.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    #include "ProcessSingleton_Windows.h"
#elif defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
    #include "ProcessSingleton_Linux.h"
#elif defined (DUILIB_BUILD_FOR_MACOS)
    #include "ProcessSingleton_MacOS.h"
#endif

#include "duilib/Utils/StringConvert.h"

namespace ui
{
ProcessSingleton::ProcessSingleton(const std::string& strAppName):
    m_strAppName(strAppName), 
    m_bRunning(false) 
{
}

ProcessSingleton::~ProcessSingleton()
{
    m_bRunning = false;
}

bool ProcessSingleton::IsAnotherInstanceRunning()
{
    return PlatformCheckInstance();
}

bool ProcessSingleton::SendArgumentsToExistingInstance(const std::vector<std::string>& vecArgs)
{
    std::string strData;
    try {
        strData = ProcessSingletonData::SerializeArguments(vecArgs);
    }
    catch (const std::exception& ex) {
        LogError("Argument serialization failed: " + std::string(ex.what()));
        return false;
    }
    return PlatformSendData(strData);
}

void ProcessSingleton::StartListener(OnAlreadyRunningAppRelaunchEvent fnCallback)
{
    ASSERT(!m_bRunning);
    if (m_bRunning) {
        return;
    }
    m_fnCallback = fnCallback;
    m_bRunning = true;
    m_thListener = std::thread(&ProcessSingleton::PlatformListen, this);
}

void ProcessSingleton::LogError(const std::string& /*strMessage*/)
{
    //std::cerr << "[ERROR] " << strMessage << std::endl;
}

void ProcessSingleton::OnAlreadyRunningAppRelaunch(const std::vector<std::string>& args)
{
    if (m_fnCallback != nullptr) {
        DString line;
        std::vector<DString> argumentList;
        for (const std::string& v : args) {
            line = StringConvert::UTF8ToT(v);
            if (!line.empty()) {
                argumentList.push_back(line);
            }
        }
        m_fnCallback(argumentList);
    }
}

std::unique_ptr<ProcessSingleton> ProcessSingleton::Create(const DString& strAppName)
{
    ASSERT(!strAppName.empty());
    if (strAppName.empty()) {
        return nullptr;
    }
    return std::make_unique<ProcessSingletonImpl>(StringConvert::TToUTF8(strAppName));
}

} //namespace ui
