#pragma once

#include <gtest/gtest.h>

#include "scene_test_support.h"
#include <ecs/ecs_manager.h>
#include <ecs/entity.h>
#include <ecs/scene.h>
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

// A system is written against whichever scene it is given: that is what lets a
// play session run a copy while the editor keeps the saved one.
TEST_F(ECSManagerTest, ASystemRunsAgainstTheSceneItIsGiven) {
  Scene other;
  other.CreateEntity();
  other.CreateEntity();

  CountingEntitiesSystem system;
  system.Update(other, 1.0f);

  EXPECT_EQ(system.count, 2u);
  EXPECT_EQ(TestScene().EntityCount(), 0u);
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
