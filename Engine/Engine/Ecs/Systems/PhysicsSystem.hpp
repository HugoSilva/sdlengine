#pragma once

#include "../ECSManager.hpp"
#include "BaseSystem.hpp"

class PhysicsSystem : public BaseSystem {
 public:
  PhysicsSystem() : BaseSystem() { m_world = new b2World{m_gravity}; }

  void Update(const float alpha) {
    m_world->Step(m_time_step, m_velocity_iterations, m_position_iterations);

    rse::ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
        [&, this](auto entity, TransformComponent& transform, RigidBodyComponent& rigid) {
          if (!rigid.m_rigidbody->m_init) {
            rigid.m_rigidbody->Init(m_world);
          }
          rigid.m_rigidbody->Update(transform.position);
        });
  }

 private:
  b2Vec2 m_gravity{0.0f, -10.0f};
  b2World* m_world;
  float m_time_step = 1.0f / 6000.0f;
  int32 m_velocity_iterations = 6;
  int32 m_position_iterations = 2;
};