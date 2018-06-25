#pragma once

#include <map>
#include "EventHandler.h"

class Event {

	typedef std::map<int, EventHandlerBase*> EventHandlerMap;
	EventHandlerMap handlers;
	int count;

public:

	template <typename Class>
	void addListener(Class *obj, void (Class::*func)(void)) {
		handlers[count] = new EventHandler<Class>(obj, func);
		count++;
	}

	void execute() {
		for (EventHandlerMap::iterator it = handlers.begin(); it != handlers.end(); ++it) {
			it->second->execute();
		}
	}

};