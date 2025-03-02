#ifndef UI_CEF_CONTROL_AUTO_UNREGISTER_H_
#define UI_CEF_CONTROL_AUTO_UNREGISTER_H_

#include "duilib/Core/Callback.h"
#include <unordered_map>
#include <functional>
#include <list>

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
        m_callbackList.emplace_back(cb);
    }
    void Add(CefUnregisterCallback&& cb)
    {
        m_callbackList.emplace_back(std::forward<CefUnregisterCallback>(cb));
    }

    ~CefAutoUnregister()
    {
        for (auto iter = m_callbackList.begin(); iter != m_callbackList.end(); iter++) {
            (*iter)();
        }
    }

private:
    std::list<CefUnregisterCallback> m_callbackList;
};
template<typename TCallback>
class CefUnregistedCallbackList : public virtual ui::SupportWeakCallback
{
public:
    CefUnregistedCallbackList() {}
    ~CefUnregistedCallbackList(){ m_elementList.clear(); };
    void Clear() { m_elementList.clear(); }
    CefUnregisterCallback AddCallback(const TCallback& cb)
    {
        auto new_cb = std::make_shared<TCallback>(cb);
        size_t cb_id = (size_t)new_cb.get();
        m_elementList.insert(std::make_pair(cb_id, new_cb));
        return ToWeakCallback([this, cb_id]() {
            m_elementList.erase(cb_id);
        });
    }
    CefUnregisterCallback AddCallback(TCallback&& cb)
    {
        auto new_cb = std::make_shared<TCallback>(std::forward<TCallback>(cb));
        size_t cb_id = (size_t)new_cb.get();
        m_elementList.insert(std::make_pair(cb_id, new_cb));
        return ToWeakCallback([this, cb_id]() {
            m_elementList.erase(cb_id);
        });
    }
    template<typename... TParams>
    void operator ()(const TParams&... params)
    {
        for (auto& it : m_elementList) {
            (*it.second)(params...);
        }
    }    
private:
    std::unordered_map<size_t, std::shared_ptr<TCallback>> m_elementList;
};

}

#endif //UI_CEF_CONTROL_AUTO_UNREGISTER_H_
