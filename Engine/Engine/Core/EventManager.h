#pragma once

#include <vector>
#include <string>

#include "Event.h"

struct EventType {
	Event* event;
	std::string name;
};

class EventManager
{
public:
	static void Register(std::string name);

	template <typename Class, class... TArgs>
	static bool AddListener(std::string name, Class *obj, void (Class::*func)(TArgs... args))
	{
		for (std::vector<EventType>::iterator it = m_Events.begin(); it != m_Events.end(); ++it)
		{
			EventType e = *it;
			if (e.name.compare(name) == 0)
			{
				e.event->addListener(obj, func);
				return true;
			}
		}
		return false;
	}

	template<class... TArgs>
	static void Dispatch(std::string name, TArgs... args)
	{
		for (std::vector<EventType>::iterator it = m_Events.begin(); it != m_Events.end(); ++it)
		{
			EventType e = *it;
			if (e.name.compare(name) == 0)
			{
				e.event->execute(args...);
			}
		}
	}

private:
	EventManager() { }
	static std::vector<EventType> m_Events;
};