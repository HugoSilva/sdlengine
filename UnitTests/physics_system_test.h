#pragma once

#include <memory>

#include <gtest/gtest.h>

#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/scene_io.h>
#include <ecs/systems/physics_system.h>

/*
Physics runs headless: PhysicsSystem owns its own b2World, so a test can step it
without a renderer. The first update is what creates each b2Body, which is also
the path a loaded scene takes -- a component that came back from a file has no
body until then.
*/
namespace {
using namespace rse;

class PhysicsSystemTest : public ::testing::Test {
 protected:
  void TearDown() override {
    SceneIO::Clear();
    PhysicsMaterialManager::Clean();
  }

  static uint32_t CreateBody(const char* name, float y, b2BodyType type) {
    const uint32_t entity = ECSManager::CreateEntity();
    ECSManager::AddComponent<BasicComponent>(entity, name, false, 0, 0);
    ECSManager::AddComponent<TransformComponent>(entity, glm::vec3(0.f, y, 0.f), glm::vec3(0.f), glm::vec3(1.f));

    Rigidbody* body = new Rigidbody(glm::vec2(0.f, y), glm::vec2(8.f, 8.f));
    *body->GetBodyType() = type;
    ECSManager::AddComponent<RigidBodyComponent>(entity, body);
    return entity;
  }
};

TEST_F(PhysicsSystemTest, ABodyIsNotSimulatedUntilTheFirstUpdate) {
  CreateBody("faller", 100.f, b2_dynamicBody);

  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        ASSERT_NE(rigid.m_rigidbody, nullptr);
        EXPECT_FALSE(rigid.m_rigidbody->m_init);
        EXPECT_FLOAT_EQ(transform.position.y, 100.f);
      });
}

TEST_F(PhysicsSystemTest, ABodyWithNoMaterialUsesTheDefaults) {
  CreateBody("faller", 100.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.Update(0.f);

  ECSManager::GetView<RigidBodyComponent>().each([](auto, RigidBodyComponent& rigid) {
    ASSERT_NE(rigid.m_rigidbody, nullptr);
    EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().density, PhysicsMaterial::kDefaultDensity);
    EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().friction, PhysicsMaterial::kDefaultFriction);
    EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().restitution, PhysicsMaterial::kDefaultRestitution);
  });
}

TEST_F(PhysicsSystemTest, ABodyUsesTheMaterialItReferences) {
  auto material = std::make_unique<PhysicsMaterialAsset>();
  material->id = "0123456789abcdef";
  material->name = "Ice";
  material->material.density = 0.9f;
  material->material.friction = 0.05f;
  material->material.restitution = 0.1f;
  PhysicsMaterialManager::Add(std::move(material));

  const uint32_t entity = CreateBody("faller", 100.f, b2_dynamicBody);
  RigidBodyComponent& rigid = ECSManager::GetComponent<RigidBodyComponent>(entity);
  ASSERT_NE(rigid.m_rigidbody, nullptr);
  rigid.m_rigidbody->SetMaterial("0123456789abcdef");

  PhysicsSystem physics;
  physics.Update(0.f);

  EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().density, 0.9f);
  EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().friction, 0.05f);
  EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().restitution, 0.1f);
}

TEST_F(PhysicsSystemTest, ADynamicBodyFalls) {
  CreateBody("faller", 100.f, b2_dynamicBody);

  PhysicsSystem physics;

  for (int step = 0; step < 60; ++step) {
    physics.Update(0.f);
  }

  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_LT(transform.position.y, 100.f);
      });
}

TEST_F(PhysicsSystemTest, AStaticBodyStaysWhereItWasPut) {
  CreateBody("floor", 100.f, b2_staticBody);

  PhysicsSystem physics;

  for (int step = 0; step < 600; ++step) {
    physics.Update(0.f);
  }

  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_FLOAT_EQ(transform.position.y, 100.f);
      });
}

TEST_F(PhysicsSystemTest, ADisabledBodyIsNotSimulatedUntilItIsEnabledAgain) {
  const uint32_t entity = CreateBody("faller", 100.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.Update(0.f);

  ECSManager::SetEntityEnabled(entity, false);
  physics.Update(0.f);

  float stopped_y = 0.f;
  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [&stopped_y](auto, TransformComponent& transform, RigidBodyComponent&) { stopped_y = transform.position.y; });

  for (int step = 0; step < 60; ++step) {
    physics.Update(0.f);
  }

  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [stopped_y](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_FLOAT_EQ(transform.position.y, stopped_y);
      });

  ECSManager::SetEntityEnabled(entity, true);

  for (int step = 0; step < 60; ++step) {
    physics.Update(0.f);
  }

  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [stopped_y](auto, TransformComponent& transform, RigidBodyComponent&) {
        EXPECT_LT(transform.position.y, stopped_y);
      });
}

TEST_F(PhysicsSystemTest, ADisabledRigidBodyComponentIsNotSimulatedUntilItIsEnabledAgain) {
  const uint32_t entity = CreateBody("faller", 100.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.Update(0.f);

  ECSManager::SetComponentEnabled<RigidBodyComponent>(entity, false);
  physics.Update(0.f);

  float stopped_y = 0.f;
  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [&stopped_y](auto, TransformComponent& transform, RigidBodyComponent&) { stopped_y = transform.position.y; });

  for (int step = 0; step < 60; ++step) {
    physics.Update(0.f);
  }

  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [stopped_y](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_FLOAT_EQ(transform.position.y, stopped_y);
      });

  ECSManager::SetComponentEnabled<RigidBodyComponent>(entity, true);

  for (int step = 0; step < 60; ++step) {
    physics.Update(0.f);
  }

  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [stopped_y](auto, TransformComponent& transform, RigidBodyComponent&) {
        EXPECT_LT(transform.position.y, stopped_y);
      });
}

// A body outlives the scene it was simulated in: emptying the registry has to
// destroy the b2Body (the world is still alive), and the next scene has to be
// able to create its own bodies without either one being freed twice.
TEST_F(PhysicsSystemTest, ClearingTheRegistryFreesTheBodyItSimulated) {
  CreateBody("first", 100.f, b2_dynamicBody);

  {
    PhysicsSystem physics;
    physics.Update(0.f);
  }

  SceneIO::Clear();
  EXPECT_EQ(ECSManager::GetView<RigidBodyComponent>().size(), 0u);

  CreateBody("second", 200.f, b2_dynamicBody);

  PhysicsSystem physics;

  for (int step = 0; step < 60; ++step) {
    physics.Update(0.f);
  }

  ECSManager::GetView<TransformComponent, RigidBodyComponent>().each(
      [](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_LT(transform.position.y, 200.f);
      });
}
}  // namespace
