#ifndef UI_CEF_CONTROL_AUTO_UNREGISTER_H_
#define UI_CEF_CONTROL_AUTO_UNREGISTER_H_

#include "duilib/Core/Callback.h"
#include <unordered_map>
#include <functional>

namespace ui {

typedef std::function<void()> CefUnregisterCallback;

class CefAutoUnregister
{
public:
    CefAutoUnregister()
    {

    }

    void Add(const CefUnregisterCallback& cb)
    {
        cb_list_.emplace_back(cb);
    }
    void Add(CefUnregisterCallback&& cb)
    {
        cb_list_.emplace_back(std::forward<CefUnregisterCallback>(cb));
    }

    ~CefAutoUnregister()
    {
        for (auto iter = cb_list_.begin(); iter != cb_list_.end(); iter ++)
            (*iter)();
    }

private:
    std::list<CefUnregisterCallback> cb_list_;
};
template<typename TCallback>
class CefUnregistedCallbackList : public virtual ui::SupportWeakCallback
{
public:
    CefUnregistedCallbackList() {}
    ~CefUnregistedCallbackList(){ element_list_.clear(); };
    void Clear() { element_list_.clear(); }
    CefUnregisterCallback AddCallback(const TCallback& cb)
    {
        auto new_cb = std::make_shared<TCallback>(cb);
        size_t cb_id = (size_t)new_cb.get();
        element_list_.insert(std::make_pair(cb_id, new_cb));
        return ToWeakCallback([this, cb_id]() {
            element_list_.erase(cb_id);
        });
    }
    CefUnregisterCallback AddCallback(TCallback&& cb)
    {
        auto new_cb = std::make_shared<TCallback>(std::forward<TCallback>(cb));
        size_t cb_id = (size_t)new_cb.get();
        element_list_.insert(std::make_pair(cb_id, new_cb));
        return ToWeakCallback([this, cb_id]() {
            element_list_.erase(cb_id);
        });
    }
    template<typename... TParams>
    void operator ()(const TParams&... params)
    {
        for (auto& it : element_list_)
        {
            (*it.second)(params...);
        }
    }    
private:
    std::unordered_map<size_t, std::shared_ptr<TCallback>> element_list_;
};

}

#endif //UI_CEF_CONTROL_AUTO_UNREGISTER_H_
