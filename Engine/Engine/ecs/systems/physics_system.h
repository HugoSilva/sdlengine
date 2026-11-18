#pragma once

#include "base_system.h"
#include <core/physics_material_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/scene.h>

class PhysicsSystem : public BaseSystem {
 public:
  PhysicsSystem() = default;

  // The step comes from the frame's fixed timestep, not from the frame delta:
  // Box2D is only stable when the time it advances is constant.
  //
  // The world is the scene's, not the system's: a body belongs to the world
  // that built it, and the scene owns that world so it outlives the entities.
  // A system that owned a world would either be shared between scenes or leak
  // one per scene.
  void UpdateSimulation(rse::Scene& scene, const float step) override {
    b2World* world = scene.PhysicsWorld();
    world->Step(step, m_velocity_iterations, m_position_iterations);

    scene.Registry().view<TransformComponent, RigidBodyComponent>().each(
        [&, this](auto entity, TransformComponent& transform, RigidBodyComponent& rigid) {
          if (rigid.m_rigidbody == nullptr) {
            return;
          }

          // An entity or rigid body that is not enabled still owns its body,
          // but the body must leave the simulation: otherwise other entities
          // would keep colliding with something that is not in the scene.
          if (!scene.IsEntityEnabled(entity) || !scene.IsComponentEnabled<RigidBodyComponent>(entity)) {
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
            rigid.m_rigidbody->Init(world, material != nullptr ? material->material : rse::PhysicsMaterial{});
          }

          rigid.m_rigidbody->SetEnabled(true);
          rigid.m_rigidbody->Update(transform.position, transform.rotation);
        });
  }

 private:
  int32 m_velocity_iterations = 6;
  int32 m_position_iterations = 2;
};
