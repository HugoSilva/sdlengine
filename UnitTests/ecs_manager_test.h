#pragma once

#include <gtest/gtest.h>

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
