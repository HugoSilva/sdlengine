#include "EventManager.h"

std::vector<EventType> EventManager::m_Events;

void EventManager::Register(std::string name) {
	for (std::vector<EventType>::iterator it = m_Events.begin(); it != m_Events.end(); ++it) {
		EventType e = *it;
		if (e.name.compare(name) == 0)
			return;
	}
	EventType e;
	e.event = new Event();
	e.name = name;
	m_Events.push_back(e);
}

template <typename Class>
bool EventManager::AddListener(std::string name, Class *obj, void (Class::*func)(void)) {
	for (vector<EventType>::iterator it = m_Events.begin(); it != m_Events.end(); ++it) {
		EventType e = *it;
		if (e.name.compare(name) == 0) {
			e.event->addListener(obj, func);
			return true;
		}
	}
	return false;
}

void EventManager::Dispatch(std::string name) {
	for (std::vector<EventType>::iterator it = m_Events.begin(); it != m_Events.end(); ++it) {
		EventType e = *it;
		if (e.name.compare(name) == 0) {
			e.event->execute();
		}
	}
}