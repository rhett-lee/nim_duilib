#include "duilib/Utils/SystemNotification.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "duilib/Utils/StringConvert.h"

#include <SDL3/SDL.h>
#if defined(__has_include)
    #if __has_include(<SDL3/SDL_notification.h>)
        #include <SDL3/SDL_notification.h>
        //当前使用的SDL版本包含系统通知API（头文件标注 since SDL 3.6.0）
        #define DUILIB_SDL_HAS_NOTIFICATION 1
    #endif
#endif

namespace ui
{

bool SystemNotification::IsSupported()
{
#ifdef DUILIB_SDL_HAS_NOTIFICATION
    return true;
#else
    return false;
#endif
}

bool SystemNotification::RequestPermission()
{
#ifdef DUILIB_SDL_HAS_NOTIFICATION
    return SDL_RequestNotificationPermission();
#else
    return false;
#endif
}

uint32_t SystemNotification::Show(const DString& title, const DString& content,
                                  SystemNotificationPriority priority, uint32_t timeoutMs)
{
    UNUSED_VARIABLE(timeoutMs);
#ifdef DUILIB_SDL_HAS_NOTIFICATION
    std::string titleUTF8 = StringConvert::TToUTF8(title);
    if (titleUTF8.empty()) {
        return 0;
    }
    std::string contentUTF8 = StringConvert::TToUTF8(content);

    SDL_NotificationID notificationId = 0;
    if (priority == SystemNotificationPriority::kNormal) {
        notificationId = SDL_ShowNotification(titleUTF8.c_str(), contentUTF8.c_str(), nullptr, nullptr, 0);
    }
    else {
        SDL_PropertiesID props = SDL_CreateProperties();
        if (props == 0) {
            return 0;
        }
        SDL_SetStringProperty(props, SDL_PROP_NOTIFICATION_TITLE_STRING, titleUTF8.c_str());
        if (!contentUTF8.empty()) {
            SDL_SetStringProperty(props, SDL_PROP_NOTIFICATION_MESSAGE_STRING, contentUTF8.c_str());
        }
        SDL_SetNumberProperty(props, SDL_PROP_NOTIFICATION_PRIORITY_NUMBER, static_cast<Sint64>(priority));
        notificationId = SDL_ShowNotificationWithProperties(props);
        SDL_DestroyProperties(props);
    }
    return notificationId;
#else
    //当前使用的SDL版本不包含系统通知API（SDL 3.6.0之前）
    UNUSED_VARIABLE(title);
    UNUSED_VARIABLE(content);
    UNUSED_VARIABLE(priority);
    return 0;
#endif
}

bool SystemNotification::Remove(uint32_t notificationId)
{
#ifdef DUILIB_SDL_HAS_NOTIFICATION
    if (notificationId == 0) {
        return false;
    }
    return SDL_RemoveNotification(notificationId);
#else
    UNUSED_VARIABLE(notificationId);
    return false;
#endif
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_SDL
