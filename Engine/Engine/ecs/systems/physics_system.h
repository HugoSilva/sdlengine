#pragma once

#include "base_system.h"
#include <core/physics_material_manager.h>
#include <ecs/ecs_manager.h>

class PhysicsSystem : public BaseSystem {
 public:
  PhysicsSystem() : BaseSystem() { m_world = new b2World{m_gravity}; }

  void Update(const float alpha) {
    m_world->Step(m_time_step, m_velocity_iterations, m_position_iterations);

    rse::ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
        [&, this](auto entity, TransformComponent& transform, RigidBodyComponent& rigid) {
          if (rigid.m_rigidbody == nullptr) {
            return;
          }

          // An entity or rigid body that is not enabled still owns its body,
          // but the body must leave the simulation: otherwise other entities
          // would keep colliding with something that is not in the scene.
          if (!rse::ECSManager::IsEntityEnabled(entity) ||
              !rse::ECSManager::IsComponentEnabled<RigidBodyComponent>(entity)) {
            if (rigid.m_rigidbody->m_init) {
              // Keep the last simulated position on the transform, then take
              // the body out of the world's simulation.
              rigid.m_rigidbody->Update(transform.position);
              rigid.m_rigidbody->SetEnabled(false);
            }
            return;
          }

          if (!rigid.m_rigidbody->m_init) {
            const rse::PhysicsMaterialAsset* material =
                rse::PhysicsMaterialManager::GetOrLoad(rigid.m_rigidbody->GetMaterial());
            rigid.m_rigidbody->Init(m_world, material != nullptr ? material->material : rse::PhysicsMaterial{});
          }

          rigid.m_rigidbody->SetEnabled(true);
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