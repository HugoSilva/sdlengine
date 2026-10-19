#pragma once

#include <entt.hpp>
#include <signal/dispatcher.hpp>

class EventManager {
 public:
  static inline entt::Dispatcher* GetEventDispatcher() { return &event_dispatcher; }

 private:
  EventManager() {}
  static entt::Dispatcher event_dispatcher;
};