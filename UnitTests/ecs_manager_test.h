#pragma once

#include <gtest/gtest.h>

#include "scene_test_support.h"
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/entity.h>
#include <ecs/systems/base_system.h>

/*
The editor stops the simulation phase, not the whole frame: the grid and the
sprites still have to be submitted so a stopped scene shows what it contains.
These tests use counting systems to pin the split.
*/
namespace {

class CountingSystem : public BaseSystem {
 public:
  void Update(rse::Scene&, const float) override { ++updates; }

  int updates{0};
};

// Counts simulation steps, which is a different entry point from Update: the
// frame's render phases get an interpolation alpha, the simulation gets a step.
class SimulationCountingSystem : public BaseSystem {
 public:
  void UpdateSimulation(rse::Scene&, const float) override { ++steps; }

  int steps{0};
};

// Reads the scene it is handed, so a test can tell which world a system ran on
// rather than assuming it is the active one.
class CountingEntitiesSystem : public BaseSystem {
 public:
  void Update(rse::Scene& scene, const float) override { count = scene.EntityCount(); }

  std::size_t count{0};
};

class ECSManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ECSManager::Clean();
    TestScene().Clear();
  }

  void TearDown() override {
    ECSManager::Clean();
    TestScene().Clear();
  }
};

TEST_F(ECSManagerTest, TheRenderPhasesRunWithoutASimulationStep) {
  CountingSystem* pre_scene = new CountingSystem();
  SimulationCountingSystem* simulation = new SimulationCountingSystem();
  CountingSystem* scene = new CountingSystem();

  ECSManager::AddSystem(pre_scene, SystemPhase::kPreScene);
  ECSManager::AddSystem(simulation, SystemPhase::kSimulation);
  ECSManager::AddSystem(scene, SystemPhase::kScene);

  // A stopped scene: Core still draws it, but owes it no simulation step.
  ECSManager::Update(TestScene(), 1.0f);

  EXPECT_EQ(pre_scene->updates, 1);
  EXPECT_EQ(simulation->steps, 0);
  EXPECT_EQ(scene->updates, 1);

  // A playing scene: one step, then the frame that draws it.
  ECSManager::UpdateSimulation(TestScene(), 1.0f / 60.0f);
  ECSManager::Update(TestScene(), 0.0f);

  EXPECT_EQ(pre_scene->updates, 2);
  EXPECT_EQ(simulation->steps, 1);
  EXPECT_EQ(scene->updates, 2);
}

// The active scene is what ECSManager runs systems on, but a system is written
// against whichever scene it is given: that is what will let a play session run
// a copy while the editor keeps the saved one.
TEST_F(ECSManagerTest, ASystemRunsAgainstTheSceneItIsGiven) {
  Scene other;
  other.CreateEntity();
  other.CreateEntity();

  CountingEntitiesSystem system;
  system.Update(other, 1.0f);

  EXPECT_EQ(system.count, 2u);
  EXPECT_EQ(TestScene().EntityCount(), 0u);
}

TEST_F(ECSManagerTest, ANewEntityIsGivenAnIdentity) {
  const uint32_t first = TestScene().CreateEntity().Handle();
  const uint32_t second = TestScene().CreateEntity().Handle();

  ASSERT_TRUE(TestScene().Registry().has<UuidComponent>(first));
  ASSERT_TRUE(TestScene().Registry().has<UuidComponent>(second));

  const Uuid& first_id = TestScene().Registry().get<UuidComponent>(first).m_uuid;
  const Uuid& second_id = TestScene().Registry().get<UuidComponent>(second).m_uuid;

  EXPECT_FALSE(first_id.IsNil());
  EXPECT_FALSE(second_id.IsNil());
  EXPECT_NE(first_id, second_id);

  TestScene().Registry().destroy(first);
  TestScene().Registry().destroy(second);
}

TEST_F(ECSManagerTest, AnEntityIsEnabledUntilItIsDisabled) {
  const uint32_t entity = TestScene().CreateEntity().Handle();

  EXPECT_TRUE(TestScene().IsEntityEnabled(entity));
  EXPECT_FALSE(TestScene().Registry().has<EntityEnabledComponent>(entity));

  TestScene().SetEntityEnabled(entity, false);
  ASSERT_TRUE(TestScene().Registry().has<EntityEnabledComponent>(entity));
  EXPECT_FALSE(TestScene().IsEntityEnabled(entity));

  TestScene().SetEntityEnabled(entity, true);
  EXPECT_TRUE(TestScene().IsEntityEnabled(entity));

  TestScene().Registry().destroy(entity);
}

TEST_F(ECSManagerTest, AComponentIsEnabledUntilItIsDisabled) {
  const uint32_t entity = TestScene().CreateEntity().Handle();
  TestScene().Registry().assign<SpriteComponent>(entity, static_cast<rse::Sprite*>(nullptr));

  EXPECT_TRUE(TestScene().IsComponentEnabled<SpriteComponent>(entity));
  EXPECT_FALSE(TestScene().Registry().has<DisabledComponentsComponent>(entity));

  TestScene().SetComponentEnabled<SpriteComponent>(entity, false);
  ASSERT_TRUE(TestScene().Registry().has<DisabledComponentsComponent>(entity));
  EXPECT_FALSE(TestScene().IsComponentEnabled<SpriteComponent>(entity));

  // One component's bit does not switch the others off.
  EXPECT_TRUE(TestScene().IsComponentEnabled<AudioComponent>(entity));
  EXPECT_TRUE(TestScene().IsComponentEnabled<RigidBodyComponent>(entity));

  TestScene().SetComponentEnabled<SpriteComponent>(entity, true);
  EXPECT_TRUE(TestScene().IsComponentEnabled<SpriteComponent>(entity));
  EXPECT_FALSE(TestScene().Registry().has<DisabledComponentsComponent>(entity));

  TestScene().Registry().destroy(entity);
}

TEST_F(ECSManagerTest, DisablingTwoComponentsKeepsTheMaskUntilBothAreBack) {
  const uint32_t entity = TestScene().CreateEntity().Handle();

  TestScene().SetComponentEnabled<SpriteComponent>(entity, false);
  TestScene().SetComponentEnabled<AudioComponent>(entity, false);

  ASSERT_TRUE(TestScene().Registry().has<DisabledComponentsComponent>(entity));
  EXPECT_FALSE(TestScene().IsComponentEnabled<SpriteComponent>(entity));
  EXPECT_FALSE(TestScene().IsComponentEnabled<AudioComponent>(entity));

  TestScene().SetComponentEnabled<SpriteComponent>(entity, true);
  EXPECT_TRUE(TestScene().Registry().has<DisabledComponentsComponent>(entity));
  EXPECT_TRUE(TestScene().IsComponentEnabled<SpriteComponent>(entity));
  EXPECT_FALSE(TestScene().IsComponentEnabled<AudioComponent>(entity));

  TestScene().SetComponentEnabled<AudioComponent>(entity, true);
  EXPECT_FALSE(TestScene().Registry().has<DisabledComponentsComponent>(entity));

  TestScene().Registry().destroy(entity);
}

TEST_F(ECSManagerTest, UpdateRunsTheRenderPhasesButNotTheSimulation) {
  CountingSystem* pre_scene = new CountingSystem();
  SimulationCountingSystem* simulation = new SimulationCountingSystem();
  CountingSystem* scene = new CountingSystem();

  ECSManager::AddSystem(pre_scene, SystemPhase::kPreScene);
  ECSManager::AddSystem(simulation, SystemPhase::kSimulation);
  ECSManager::AddSystem(scene);

  ECSManager::Update(TestScene(), 0.f);

  EXPECT_EQ(pre_scene->updates, 1);
  EXPECT_EQ(simulation->steps, 0);
  EXPECT_EQ(scene->updates, 1);

  ECSManager::UpdateSimulation(TestScene(), 1.0f / 60.0f);

  EXPECT_EQ(simulation->steps, 1);
  EXPECT_EQ(pre_scene->updates, 1);
  EXPECT_EQ(scene->updates, 1);
}
}  // namespace
