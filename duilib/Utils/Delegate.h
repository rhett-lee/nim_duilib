#ifndef UI_UTILS_DELEGATE_H_
#define UI_UTILS_DELEGATE_H_

#include "duilib/Core/EventArgs.h"
#include <functional>
#include <vector>
#include <string>
#include <map>

namespace ui 
{

typedef std::function<bool(const ui::EventArgs&)> EventCallback;

class CEventSource : public std::vector<EventCallback>
{
public:
    CEventSource& operator += (const EventCallback& callback)
    {
        ASSERT(callback != nullptr);
        if (callback != nullptr) {
            push_back(callback);
        }        
        return *this;
    }

    bool operator() (const ui::EventArgs& param) const
    {
        //支持在回调函数中，操作此容器
        for (size_t index = 0; index < this->size(); ++index) {
            if (param.IsSenderExpired()) {
                return false;
            }
            EventCallback callback = this->at(index);
            if ((callback == nullptr) || !callback(param)) {
                return false;
            }
        }
        return true;
    }

};

typedef std::map<EventType, CEventSource> EventMap;

}

#endif // UI_UTILS_DELEGATE_H_
