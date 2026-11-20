#pragma once

#include <cstdint>

#include <gtest/gtest.h>

#include <ecs/components/include_list.h>
#include <ecs/entity.h>
#include <ecs/scene.h>
#include <ecs/systems/camera_system.h>
#include <render/ortho_view.h>

/*
The camera component is data plus a rule for choosing one. These tests drive a
scene directly: no window, no renderer, no mixer. What they pin down is the
selection -- priority, ties, disabled state -- and that a scene with no camera
entity keeps the framing it was saved with.
*/
namespace {

using namespace rse;

class CameraComponentTest : public ::testing::Test {
 protected:
  static uint32_t AddCamera(Scene& scene, glm::vec2 position, float zoom = 1.0f, int priority = 0) {
    const uint32_t entity = scene.CreateEntity().Handle();
    scene.Registry().assign<TransformComponent>(entity, glm::vec3(position, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    scene.Registry().assign<CameraComponent>(entity);
    CameraComponent& camera = scene.Registry().get<CameraComponent>(entity);
    camera.zoom = zoom;
    camera.priority = priority;
    return entity;
  }
};

TEST_F(CameraComponentTest, ACameraEntityIsTheView) {
  Scene scene;
  AddCamera(scene, glm::vec2(40.0f, 80.0f), 2.0f);

  CameraSystem::View view;
  ASSERT_TRUE(CameraSystem::Resolve(scene, &view));
  EXPECT_FLOAT_EQ(40.0f, view.position.x);
  EXPECT_FLOAT_EQ(80.0f, view.position.y);
  EXPECT_FLOAT_EQ(2.0f, view.zoom);
}

TEST_F(CameraComponentTest, HighestPriorityWins) {
  Scene scene;
  AddCamera(scene, glm::vec2(10.0f, 10.0f), 1.0f, 0);
  AddCamera(scene, glm::vec2(20.0f, 20.0f), 3.0f, 5);

  CameraSystem::View view;
  ASSERT_TRUE(CameraSystem::Resolve(scene, &view));
  EXPECT_FLOAT_EQ(20.0f, view.position.x);
  EXPECT_FLOAT_EQ(3.0f, view.zoom);
}

TEST_F(CameraComponentTest, TheLowerEntityIdBreaksAPriorityTie) {
  Scene scene;
  const uint32_t first = AddCamera(scene, glm::vec2(10.0f, 10.0f), 1.0f, 2);
  const uint32_t second = AddCamera(scene, glm::vec2(20.0f, 20.0f), 2.0f, 2);
  ASSERT_LT(first, second);

  CameraSystem::View view;
  ASSERT_TRUE(CameraSystem::Resolve(scene, &view));
  EXPECT_FLOAT_EQ(10.0f, view.position.x);
}

TEST_F(CameraComponentTest, ASceneWithoutACameraKeepsItsSavedFraming) {
  Scene scene;
  scene.GetCamera().position = glm::vec2(5.0f, 6.0f);
  scene.GetCamera().zoom = 4.0f;

  CameraSystem::View view;
  EXPECT_FALSE(CameraSystem::Resolve(scene, &view));

  CameraSystem system;
  system.UpdateSimulation(scene, 1.0f / 60.0f);

  EXPECT_FLOAT_EQ(5.0f, scene.GetCamera().position.x);
  EXPECT_FLOAT_EQ(6.0f, scene.GetCamera().position.y);
  EXPECT_FLOAT_EQ(4.0f, scene.GetCamera().zoom);
}

TEST_F(CameraComponentTest, UpdateSimulationWritesTheSceneCamera) {
  Scene scene;
  AddCamera(scene, glm::vec2(100.0f, 200.0f), 3.0f);
  scene.GetCamera().background = glm::vec3(0.2f, 0.3f, 0.4f);

  CameraSystem system;
  system.UpdateSimulation(scene, 1.0f / 60.0f);

  EXPECT_FLOAT_EQ(100.0f, scene.GetCamera().position.x);
  EXPECT_FLOAT_EQ(200.0f, scene.GetCamera().position.y);
  EXPECT_FLOAT_EQ(3.0f, scene.GetCamera().zoom);

  // The background belongs to the scene, not to a camera entity.
  EXPECT_FLOAT_EQ(0.2f, scene.GetCamera().background.r);
  EXPECT_FLOAT_EQ(0.4f, scene.GetCamera().background.b);
}

TEST_F(CameraComponentTest, DisabledEntitiesAndComponentsAreSkipped) {
  Scene scene;
  const uint32_t primary = AddCamera(scene, glm::vec2(10.0f, 10.0f), 1.0f, 10);
  AddCamera(scene, glm::vec2(20.0f, 20.0f), 1.0f, 0);

  CameraSystem::View view;

  scene.SetEntityEnabled(primary, false);
  ASSERT_TRUE(CameraSystem::Resolve(scene, &view));
  EXPECT_FLOAT_EQ(20.0f, view.position.x);

  scene.SetEntityEnabled(primary, true);
  scene.SetComponentEnabled<CameraComponent>(primary, false);
  ASSERT_TRUE(CameraSystem::Resolve(scene, &view));
  EXPECT_FLOAT_EQ(20.0f, view.position.x);

  scene.SetComponentEnabled<CameraComponent>(primary, true);
  ASSERT_TRUE(CameraSystem::Resolve(scene, &view));
  EXPECT_FLOAT_EQ(10.0f, view.position.x);
}

TEST_F(CameraComponentTest, AnEntityWithoutATransformIsNotACamera) {
  Scene scene;
  const uint32_t entity = scene.CreateEntity().Handle();
  scene.Registry().assign<CameraComponent>(entity);

  EXPECT_FALSE(CameraSystem::Resolve(scene, nullptr));
}

TEST_F(CameraComponentTest, ZoomIsClampedToTheProjectionRange) {
  Scene scene;
  const uint32_t entity = AddCamera(scene, glm::vec2(0.0f), 0.0f);

  CameraSystem::View view;
  ASSERT_TRUE(CameraSystem::Resolve(scene, &view));
  EXPECT_FLOAT_EQ(kOrthoMinZoom, view.zoom);

  scene.Registry().get<CameraComponent>(entity).zoom = 1000.0f;
  ASSERT_TRUE(CameraSystem::Resolve(scene, &view));
  EXPECT_FLOAT_EQ(kOrthoMaxZoom, view.zoom);
}

TEST_F(CameraComponentTest, ASceneCopyCarriesItsCameras) {
  Scene source;
  AddCamera(source, glm::vec2(77.0f, 88.0f), 2.5f);

  Scene copy;
  copy.CopyFrom(source);

  EXPECT_EQ(1u, copy.Registry().view<CameraComponent>().size());

  CameraSystem::View view;
  ASSERT_TRUE(CameraSystem::Resolve(copy, &view));
  EXPECT_FLOAT_EQ(77.0f, view.position.x);
  EXPECT_FLOAT_EQ(88.0f, view.position.y);
  EXPECT_FLOAT_EQ(2.5f, view.zoom);
}

}  // namespace
