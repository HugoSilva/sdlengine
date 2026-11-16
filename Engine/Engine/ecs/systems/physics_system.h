#pragma once

#include "base_system.h"
#include <core/physics_material_manager.h>
#include <ecs/ecs_manager.h>

class PhysicsSystem : public BaseSystem {
 public:
  PhysicsSystem() : BaseSystem() { m_world = new b2World{m_gravity}; }

  // The step comes from the frame's fixed timestep, not from the frame delta:
  // Box2D is only stable when the time it advances is constant.
  void UpdateSimulation(const float step) override {
    m_world->Step(step, m_velocity_iterations, m_position_iterations);

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
              rigid.m_rigidbody->Update(transform.position, transform.rotation);
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
          rigid.m_rigidbody->Update(transform.position, transform.rotation);
        });
  }

 private:
  b2Vec2 m_gravity{0.0f, -10.0f};
  b2World* m_world;
  int32 m_velocity_iterations = 6;
  int32 m_position_iterations = 2;
};