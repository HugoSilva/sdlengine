#pragma once

#include <entt.hpp>
#include <signal/dispatcher.hpp>

class EventManager
{
public:
	static inline entt::Dispatcher* getEventDispatcher() { return &eventDispatcher; }

private:
	EventManager() { }
	static entt::Dispatcher eventDispatcher;
};