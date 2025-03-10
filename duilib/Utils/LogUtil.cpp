#include "LogUtil.h"
#include "duilib/Utils/StringUtil.h"
#include <chrono>

#ifdef DUILIB_BUILD_FOR_SDL
    #include <SDL3/SDL.h>
#endif

namespace ui 
{
/** 程序启动时的时间戳
*/
static std::chrono::steady_clock::time_point s_startTime = std::chrono::steady_clock::now();

DString LogUtil::GetTimeStamp()
{
    //从系统启动以来的时间间隔，时间精确到毫秒
    std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
    auto thisTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - s_startTime);
    uint64_t nTimeMs = (uint64_t)thisTime.count();
    uint32_t nHH = (uint32_t)((nTimeMs / 1000) / 60 / 60);
    uint32_t nMM = (uint32_t)((nTimeMs / 1000) / 60);
    uint32_t nSS = (uint32_t)(nTimeMs / 1000);
    uint32_t nMS = (uint32_t)(nTimeMs % 1000);
    return StringUtil::Printf(_T("%02u:%02u:%02u.%03u "), nHH, nMM, nSS, nMS);
}

void LogUtil::Output(const DString& log)
{
    DString logMsg = GetTimeStamp() + log;
#ifdef DUILIB_BUILD_FOR_WIN
    ::OutputDebugString(logMsg.c_str());
#elif defined (DUILIB_BUILD_FOR_SDL)
    SDL_Log("%s", logMsg.c_str());
#endif
}

void LogUtil::OutputLine(const DString& log)
{
#ifdef DUILIB_BUILD_FOR_WIN
    Output(log + _T("\r\n"));
#else
    Output(log + _T("\n"));
#endif
}

} // namespace ui 
