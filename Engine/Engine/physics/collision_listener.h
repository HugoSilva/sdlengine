#pragma once

#include <cstdint>

#include <box2d/box2d.h>

#include "collision_event.h"
#include "rigidbody.h"

namespace rse {
// Fills the scene's collision queue from Box2D's contact callbacks.
//
// The callbacks run in the middle of a step, where creating or destroying
// bodies is forbidden, so the listener only records the pair. The bodies carry
// the Rigidbody that owns them as user data, which is how a contact becomes two
// entity ids without a lookup table.
class CollisionListener : public b2ContactListener {
 public:
  explicit CollisionListener(CollisionEvents* events) : m_events(events) {}

  void BeginContact(b2Contact* contact) override { Record(contact, false); }
  void EndContact(b2Contact* contact) override { Record(contact, true); }

 private:
  static Rigidbody* BodyOf(b2Fixture* fixture) {
    if (fixture == nullptr) {
      return nullptr;
    }

    return reinterpret_cast<Rigidbody*>(fixture->GetBody()->GetUserData().pointer);
  }

  void Record(b2Contact* contact, bool ended) {
    if (m_events == nullptr || contact == nullptr) {
      return;
    }

    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();

    const Rigidbody* rigid_a = BodyOf(fixture_a);
    const Rigidbody* rigid_b = BodyOf(fixture_b);

    // A body the engine did not build has no entity to name.
    if (rigid_a == nullptr || rigid_b == nullptr) {
      return;
    }

    const CollisionEvent event{rigid_a->GetEntity(), rigid_b->GetEntity(),
                               fixture_a->IsSensor() || fixture_b->IsSensor()};

    if (ended) {
      m_events->ended.push_back(event);
    } else {
      m_events->begun.push_back(event);
    }
  }

  CollisionEvents* m_events{nullptr};
};
}  // namespace rse
