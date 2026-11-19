#pragma once

#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "scene_test_support.h"
#include <audio/sound_manager.h>
#include <core/scene_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/scene_io.h>

/*
The play session is the editor's "run the open scene" loop: Play copies the open
scene and simulates the copy, Pause holds it, and Stop throws it away and makes
the open scene active again. The open scene is never simulated, which is what
makes Stop exact. These tests drive the state machine headless; no window,
renderer or mixer is needed.
*/
namespace {

using namespace rse;

class SceneManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_scene_manager_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);

    SoundManager::Clean();
    SceneManager::Init(m_directory);
    SceneManager::OpenScene().Clear();
  }

  void TearDown() override {
    SceneManager::Clean();
    SceneManager::OpenScene().Clear();
    SoundManager::Clean();
    std::filesystem::remove_all(m_directory);
  }

  static uint32_t CreateEntity(const char* name, const glm::vec3& position) {
    const uint32_t entity = TestScene().CreateEntity().Handle();
    TestScene().Registry().assign<BasicComponent>(entity, name, false, 0, 0);
    TestScene().Registry().assign<TransformComponent>(entity, position, glm::vec3(0.f), glm::vec3(1.f));
    return entity;
  }

  static uint32_t CountEntities() {
    uint32_t entities = 0;
    TestScene().Registry().view<BasicComponent>().each([&entities](auto, BasicComponent&) { ++entities; });
    return entities;
  }

  // The copy's entt ids are its own registry's, so the copy is found by the
  // uuid it kept -- the same way anything outside a registry names an entity.
  static uint32_t FindByUuid(const Uuid& uuid) {
    uint32_t found = static_cast<uint32_t>(entt::null);

    TestScene().Registry().each([&uuid, &found](entt::DefaultRegistry::entity_type entity) {
      if (TestScene().Registry().has<UuidComponent>(entity) &&
          TestScene().Registry().get<UuidComponent>(entity).m_uuid == uuid) {
        found = entity;
      }
    });

    return found;
  }

  std::filesystem::path m_directory;
};

TEST_F(SceneManagerTest, StartsStopped) {
  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kStopped);
  EXPECT_FALSE(SceneManager::IsPlaying());
  EXPECT_FALSE(SceneManager::IsPaused());
  EXPECT_FALSE(SceneManager::IsPlayActive());
  EXPECT_EQ(&TestScene(), &SceneManager::OpenScene());
}

TEST_F(SceneManagerTest, PlayRunsACopyOfTheOpenScene) {
  const uint32_t player = CreateEntity("player", glm::vec3(1.f, 2.f, 0.f));
  const Uuid uuid = TestScene().Registry().get<UuidComponent>(player).m_uuid;

  SceneManager::Play();

  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kPlaying);
  EXPECT_NE(&TestScene(), &SceneManager::OpenScene());

  // The copy starts as the open scene did, with the same identity.
  EXPECT_EQ(CountEntities(), 1u);
  const uint32_t copy = FindByUuid(uuid);
  ASSERT_NE(copy, static_cast<uint32_t>(entt::null));

  // The scene runs forward in the copy: an entity moved and one appeared.
  TestScene().Registry().get<TransformComponent>(copy).position = glm::vec3(9.f, 10.f, 0.f);
  const uint32_t transient = CreateEntity("transient", glm::vec3(5.f, 5.f, 0.f));
  TestScene().GetCamera().zoom = 3.0f;

  EXPECT_EQ(CountEntities(), 2u);

  // None of that reached the scene being edited.
  EXPECT_EQ(SceneManager::OpenScene().EntityCount(), 1u);
  EXPECT_FLOAT_EQ(SceneManager::OpenScene().Registry().get<TransformComponent>(player).position.x, 1.f);
  EXPECT_FLOAT_EQ(SceneManager::OpenScene().GetCamera().zoom, 1.f);

  SceneManager::Stop();

  // The copy is gone with the session; the open scene's entities are all that
  // is left.
  EXPECT_TRUE(TestScene().Registry().valid(player));
  EXPECT_FALSE(TestScene().Registry().valid(transient));
}

TEST_F(SceneManagerTest, StopReturnsToTheOpenScene) {
  const uint32_t player = CreateEntity("player", glm::vec3(1.f, 2.f, 0.f));
  const Uuid uuid = TestScene().Registry().get<UuidComponent>(player).m_uuid;

  SceneManager::Play();
  TestScene().Registry().get<TransformComponent>(FindByUuid(uuid)).position = glm::vec3(9.f, 10.f, 0.f);

  SceneManager::Stop();

  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kStopped);
  EXPECT_FALSE(SceneManager::IsPlayActive());
  EXPECT_EQ(&TestScene(), &SceneManager::OpenScene());

  // The open scene is exactly what Play found: same entity, same values.
  EXPECT_EQ(CountEntities(), 1u);
  EXPECT_TRUE(TestScene().Registry().valid(player));
  EXPECT_FLOAT_EQ(TestScene().Registry().get<TransformComponent>(player).position.x, 1.f);
}

// Play after Stop has to start from the open scene again, not from what the
// discarded copy had become.
TEST_F(SceneManagerTest, PlayingAgainStartsFromTheOpenScene) {
  const uint32_t player = CreateEntity("player", glm::vec3(1.f, 2.f, 0.f));
  const Uuid uuid = TestScene().Registry().get<UuidComponent>(player).m_uuid;

  SceneManager::Play();
  TestScene().Registry().get<TransformComponent>(FindByUuid(uuid)).position = glm::vec3(9.f, 10.f, 0.f);
  SceneManager::Stop();

  SceneManager::Play();

  const uint32_t copy = FindByUuid(uuid);
  ASSERT_NE(copy, static_cast<uint32_t>(entt::null));
  EXPECT_FLOAT_EQ(TestScene().Registry().get<TransformComponent>(copy).position.x, 1.f);
}

TEST_F(SceneManagerTest, PauseHoldsAndResumeContinues) {
  CreateEntity("player", glm::vec3(0.f));

  SceneManager::Play();
  ASSERT_TRUE(SceneManager::IsPlaying());

  SceneManager::Pause();
  EXPECT_TRUE(SceneManager::IsPaused());
  EXPECT_FALSE(SceneManager::IsPlaying());
  EXPECT_TRUE(SceneManager::IsPlayActive());

  // Play doubles as Resume from the paused state.
  SceneManager::Play();
  EXPECT_TRUE(SceneManager::IsPlaying());

  SceneManager::Stop();
  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kStopped);
}

TEST_F(SceneManagerTest, PauseAndStopWhenStoppedAreNoOps) {
  SceneManager::Pause();
  EXPECT_FALSE(SceneManager::IsPlayActive());

  SceneManager::Stop();
  EXPECT_FALSE(SceneManager::IsPlayActive());

  // Resuming without a pause does nothing.
  SceneManager::Resume();
  EXPECT_FALSE(SceneManager::IsPlayActive());
}

TEST_F(SceneManagerTest, PlayWhenPlayingKeepsTheSameCopy) {
  const uint32_t player = CreateEntity("player", glm::vec3(1.f, 2.f, 0.f));
  const Uuid uuid = TestScene().Registry().get<UuidComponent>(player).m_uuid;

  SceneManager::Play();
  TestScene().Registry().get<TransformComponent>(FindByUuid(uuid)).position = glm::vec3(7.f, 8.f, 0.f);

  // A second Play is a no-op: the session is already running, and the copy it
  // started is the one that keeps running.
  SceneManager::Play();
  EXPECT_TRUE(SceneManager::IsPlaying());

  SceneManager::Stop();
  EXPECT_FLOAT_EQ(TestScene().Registry().get<TransformComponent>(player).position.x, 1.f);
}

TEST_F(SceneManagerTest, StopReturnsToTheDirtyFlagFromBeforePlay) {
  // A scene that was already dirty stays dirty: its unsaved edits are part of
  // what Play copied, so stopping does not make them saved.
  SceneManager::MarkDirty();
  SceneManager::Play();
  SceneManager::Stop();
  EXPECT_TRUE(SceneManager::IsDirty());
}

TEST_F(SceneManagerTest, EditsMadeWhilePlayingDoNotLeaveTheSceneDirty) {
  ASSERT_FALSE(SceneManager::IsDirty());
  SceneManager::Play();

  SceneManager::MarkDirty();
  ASSERT_TRUE(SceneManager::IsDirty());

  SceneManager::Stop();
  EXPECT_FALSE(SceneManager::IsDirty());
}

TEST_F(SceneManagerTest, SaveIsRefusedWhilePlaying) {
  const std::filesystem::path scene = m_directory / "scene.xml";
  std::string error;

  ASSERT_TRUE(SceneManager::Create(scene, &error)) << error;
  SceneManager::Play();

  EXPECT_FALSE(SceneManager::Save(&error));
  EXPECT_NE(error.find("stop the playing scene"), std::string::npos);

  SceneManager::Stop();
  ASSERT_TRUE(SceneManager::Save(&error)) << error;
  EXPECT_TRUE(std::filesystem::exists(scene));
}

TEST_F(SceneManagerTest, OpeningASceneDiscardsTheSession) {
  const std::filesystem::path first = m_directory / "first.xml";
  const std::filesystem::path second = m_directory / "second.xml";
  std::string error;

  ASSERT_TRUE(SceneManager::Create(first, &error)) << error;
  ASSERT_TRUE(SceneManager::Create(second, &error)) << error;
  CreateEntity("only in the open scene", glm::vec3(0.f));

  SceneManager::Play();
  ASSERT_TRUE(SceneManager::IsPlaying());

  // A scene switch leaves the session behind rather than restoring it, so the
  // copy is dropped before the new file is read.
  ASSERT_TRUE(SceneManager::Open(second, &error)) << error;

  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kStopped);
  EXPECT_FALSE(SceneManager::IsPlayActive());
  EXPECT_EQ(&TestScene(), &SceneManager::OpenScene());
  EXPECT_EQ(SceneManager::OpenScene().EntityCount(), 0u);
}
}  // namespace
