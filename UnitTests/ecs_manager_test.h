#pragma once

#include <gtest/gtest.h>

#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/systems/base_system.h>

/*
The editor stops the simulation phase, not the whole frame: the grid and the
sprites still have to be submitted so a stopped scene shows what it contains.
These tests use counting systems to pin the split.
*/
namespace {

class CountingSystem : public BaseSystem {
 public:
  void Update(const float alpha) override { ++updates; }

  int updates{0};
};

class ECSManagerTest : public ::testing::Test {
 protected:
  void SetUp() override { ECSManager::Clean(); }
  void TearDown() override { ECSManager::Clean(); }
};

TEST_F(ECSManagerTest, DisablingSimulationStillRunsTheRenderPhases) {
  CountingSystem* pre_scene = new CountingSystem();
  CountingSystem* simulation = new CountingSystem();
  CountingSystem* scene = new CountingSystem();

  ECSManager::AddSystem(pre_scene, SystemPhase::kPreScene);
  ECSManager::AddSystem(simulation, SystemPhase::kSimulation);
  ECSManager::AddSystem(scene, SystemPhase::kScene);

  ECSManager::Update(0.f, false);

  EXPECT_EQ(pre_scene->updates, 1);
  EXPECT_EQ(simulation->updates, 0);
  EXPECT_EQ(scene->updates, 1);

  ECSManager::Update(0.f, true);

  EXPECT_EQ(pre_scene->updates, 2);
  EXPECT_EQ(simulation->updates, 1);
  EXPECT_EQ(scene->updates, 2);
}

TEST_F(ECSManagerTest, AnEntityIsEnabledUntilItIsDisabled) {
  const uint32_t entity = ECSManager::CreateEntity();

  EXPECT_TRUE(ECSManager::IsEntityEnabled(entity));
  EXPECT_FALSE(ECSManager::HasComponent<EntityEnabledComponent>(entity));

  ECSManager::SetEntityEnabled(entity, false);
  ASSERT_TRUE(ECSManager::HasComponent<EntityEnabledComponent>(entity));
  EXPECT_FALSE(ECSManager::IsEntityEnabled(entity));

  ECSManager::SetEntityEnabled(entity, true);
  EXPECT_TRUE(ECSManager::IsEntityEnabled(entity));

  ECSManager::DestroyEntity(entity);
}

TEST_F(ECSManagerTest, AComponentIsEnabledUntilItIsDisabled) {
  const uint32_t entity = ECSManager::CreateEntity();
  ECSManager::AddComponent<SpriteComponent>(entity, static_cast<rse::Sprite*>(nullptr));

  EXPECT_TRUE(ECSManager::IsComponentEnabled<SpriteComponent>(entity));
  EXPECT_FALSE(ECSManager::HasComponent<DisabledComponentsComponent>(entity));

  ECSManager::SetComponentEnabled<SpriteComponent>(entity, false);
  ASSERT_TRUE(ECSManager::HasComponent<DisabledComponentsComponent>(entity));
  EXPECT_FALSE(ECSManager::IsComponentEnabled<SpriteComponent>(entity));

  // One component's bit does not switch the others off.
  EXPECT_TRUE(ECSManager::IsComponentEnabled<AudioComponent>(entity));
  EXPECT_TRUE(ECSManager::IsComponentEnabled<RigidBodyComponent>(entity));

  ECSManager::SetComponentEnabled<SpriteComponent>(entity, true);
  EXPECT_TRUE(ECSManager::IsComponentEnabled<SpriteComponent>(entity));
  EXPECT_FALSE(ECSManager::HasComponent<DisabledComponentsComponent>(entity));

  ECSManager::DestroyEntity(entity);
}

TEST_F(ECSManagerTest, DisablingTwoComponentsKeepsTheMaskUntilBothAreBack) {
  const uint32_t entity = ECSManager::CreateEntity();

  ECSManager::SetComponentEnabled<SpriteComponent>(entity, false);
  ECSManager::SetComponentEnabled<AudioComponent>(entity, false);

  ASSERT_TRUE(ECSManager::HasComponent<DisabledComponentsComponent>(entity));
  EXPECT_FALSE(ECSManager::IsComponentEnabled<SpriteComponent>(entity));
  EXPECT_FALSE(ECSManager::IsComponentEnabled<AudioComponent>(entity));

  ECSManager::SetComponentEnabled<SpriteComponent>(entity, true);
  EXPECT_TRUE(ECSManager::HasComponent<DisabledComponentsComponent>(entity));
  EXPECT_TRUE(ECSManager::IsComponentEnabled<SpriteComponent>(entity));
  EXPECT_FALSE(ECSManager::IsComponentEnabled<AudioComponent>(entity));

  ECSManager::SetComponentEnabled<AudioComponent>(entity, true);
  EXPECT_FALSE(ECSManager::HasComponent<DisabledComponentsComponent>(entity));

  ECSManager::DestroyEntity(entity);
}

TEST_F(ECSManagerTest, TheDefaultUpdateRunsEveryPhase) {
  CountingSystem* simulation = new CountingSystem();
  CountingSystem* scene = new CountingSystem();

  ECSManager::AddSystem(simulation, SystemPhase::kSimulation);
  ECSManager::AddSystem(scene);

  ECSManager::Update(0.f);

  EXPECT_EQ(simulation->updates, 1);
  EXPECT_EQ(scene->updates, 1);
}
}  // namespace
