#ifndef UI_UTILS_DELEGATE_H_
#define UI_UTILS_DELEGATE_H_

#pragma once

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
	CEventSource& operator += (const EventCallback& item) 
	{
		push_back(item);
		return *this;
	}

	bool operator() (const ui::EventArgs& param) const
	{
		for (size_t index = 0; index < this->size(); ++index) {
			const EventCallback& callback = this->at(index);
			if (!callback(param)) {
				return false;
			}
		}
		return true;
	}

};

typedef std::map<EventType, CEventSource> EventMap;

}

#endif // UI_UTILS_DELEGATE_H_