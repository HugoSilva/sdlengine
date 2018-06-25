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

	template <typename Class>
	static bool AddListener(std::string name, Class *obj, void (Class::*func)(void));

	static void Dispatch(std::string name);

private:
	EventManager() { }
	static std::vector<EventType> m_Events;
};