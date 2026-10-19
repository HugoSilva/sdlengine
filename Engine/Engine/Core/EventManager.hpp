#pragma once

#include <entt.hpp>
#include <signal/dispatcher.hpp>

class EventManager {
 public:
  static inline entt::Dispatcher* GetEventDispatcher() { return &eventDispatcher; }

 private:
  EventManager() {}
  static entt::Dispatcher eventDispatcher;
};