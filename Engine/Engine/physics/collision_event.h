#pragma once

#include <cstdint>
#include <vector>

// One pair of entities whose fixtures began, or stopped, touching during a
// simulation step.
//
// The pair is unordered: it follows Box2D's fixture order, which is creation
// order, not a statement about the scene. An event names entities rather than
// bodies so a game system can look the components up without keeping a body
// pointer around.
struct CollisionEvent {
  std::uint32_t entity_a{0};
  std::uint32_t entity_b{0};
  // True when either fixture is a sensor: an overlap trigger, not a solid hit.
  bool sensor{false};
};

// The contact events produced since the current frame's simulation began.
//
// Box2D's callbacks run in the middle of a step, where creating or destroying
// bodies is forbidden, so they only record here. Game code reads the queue in a
// render-phase system, after every fixed step of the frame has run; the engine
// clears it at the start of the next frame, so a contact is delivered once and
// a display faster than the simulation does not replay it.
struct CollisionEvents {
  std::vector<CollisionEvent> begun;
  std::vector<CollisionEvent> ended;

  bool Empty() const { return begun.empty() && ended.empty(); }

  void Clear() {
    begun.clear();
    ended.clear();
  }
};
