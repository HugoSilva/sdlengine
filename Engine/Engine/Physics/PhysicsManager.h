#pragma once

#include <box2d/box2d.h>

#include "../Components/Sprite.hpp"
#include "Rigidbody.h"
#include "Updatable.h"

class PhysicsManager : public physics::Updatable {
 public:
  static void Init();
  static void Add(Rigidbody* rb);
  void Update() override;
  static void UpdateObjects();
  static void Clean();
  static b2World* GetWorldObject();

 private:
  PhysicsManager() {}
  static std::vector<Rigidbody*> m_rigidbodies;
  static b2Vec2 m_gravity;
  static b2World* m_world;

  static float time_step;
  static int32 velocity_iterations;
  static int32 position_iterations;
};