#pragma once

#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <audio/sound_manager.h>
#include <core/scene_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/scene_io.h>

/*
The play session is the editor's "run the open scene" loop: Play snapshots the
scene, Pause holds it, and Stop puts it back exactly as Play found it. These
tests drive the state machine headless; no window, renderer or mixer is needed.
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
    SceneIO::Clear();
    SceneManager::Init(m_directory);
  }

  void TearDown() override {
    SceneManager::Clean();
    SceneIO::Clear();
    SoundManager::Clean();
    std::filesystem::remove_all(m_directory);
  }

  static uint32_t CreateEntity(const char* name, const glm::vec3& position) {
    const uint32_t entity = ECSManager::CreateEntity();
    ECSManager::AddComponent<BasicComponent>(entity, name, false, 0, 0);
    ECSManager::AddComponent<TransformComponent>(entity, position, glm::vec3(0.f), glm::vec3(1.f));
    return entity;
  }

  static uint32_t CountEntities() {
    uint32_t entities = 0;
    ECSManager::GetView<BasicComponent>().each([&entities](auto, BasicComponent&) { ++entities; });
    return entities;
  }

  std::filesystem::path m_directory;
};

TEST_F(SceneManagerTest, StartsStopped) {
  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kStopped);
  EXPECT_FALSE(SceneManager::IsPlaying());
  EXPECT_FALSE(SceneManager::IsPaused());
  EXPECT_FALSE(SceneManager::IsPlayActive());
}

TEST_F(SceneManagerTest, PlayCapturesAndStopRestores) {
  const uint32_t player = CreateEntity("player", glm::vec3(1.f, 2.f, 0.f));

  std::string error;
  ASSERT_TRUE(SceneManager::Play(&error)) << error;
  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kPlaying);

  // The scene runs forward: the player moved and an entity appeared.
  ECSManager::GetComponent<TransformComponent>(player).position = glm::vec3(9.f, 10.f, 0.f);
  const uint32_t transient = CreateEntity("transient", glm::vec3(5.f, 5.f, 0.f));
  SceneIO::GetCamera().zoom = 3.0f;

  ASSERT_TRUE(SceneManager::Stop(&error)) << error;
  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kStopped);

  EXPECT_EQ(CountEntities(), 1u);
  EXPECT_TRUE(ECSManager::IsValidEntity(player));
  EXPECT_FALSE(ECSManager::IsValidEntity(transient));

  const TransformComponent& transform = ECSManager::GetComponent<TransformComponent>(player);
  EXPECT_FLOAT_EQ(transform.position.x, 1.f);
  EXPECT_FLOAT_EQ(transform.position.y, 2.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().zoom, 1.0f);
}

TEST_F(SceneManagerTest, PauseHoldsAndResumeContinues) {
  CreateEntity("player", glm::vec3(0.f));

  std::string error;
  ASSERT_TRUE(SceneManager::Play(&error)) << error;
  ASSERT_TRUE(SceneManager::IsPlaying());

  SceneManager::Pause();
  EXPECT_TRUE(SceneManager::IsPaused());
  EXPECT_FALSE(SceneManager::IsPlaying());
  EXPECT_TRUE(SceneManager::IsPlayActive());

  // Play doubles as Resume from the paused state.
  ASSERT_TRUE(SceneManager::Play(&error)) << error;
  EXPECT_TRUE(SceneManager::IsPlaying());

  ASSERT_TRUE(SceneManager::Stop(&error)) << error;
  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kStopped);
}

TEST_F(SceneManagerTest, PauseAndStopWhenStoppedAreNoOps) {
  std::string error;

  SceneManager::Pause();
  EXPECT_FALSE(SceneManager::IsPlayActive());

  EXPECT_TRUE(SceneManager::Stop(&error));
  EXPECT_TRUE(error.empty());
  EXPECT_FALSE(SceneManager::IsPlayActive());

  // Resuming without a pause does nothing.
  SceneManager::Resume();
  EXPECT_FALSE(SceneManager::IsPlayActive());
}

TEST_F(SceneManagerTest, PlayWhenPlayingKeepsTheOriginalSnapshot) {
  const uint32_t player = CreateEntity("player", glm::vec3(1.f, 2.f, 0.f));

  std::string error;
  ASSERT_TRUE(SceneManager::Play(&error)) << error;

  ECSManager::GetComponent<TransformComponent>(player).position = glm::vec3(7.f, 8.f, 0.f);

  ASSERT_TRUE(SceneManager::Play(&error)) << error;
  EXPECT_TRUE(SceneManager::IsPlaying());

  ASSERT_TRUE(SceneManager::Stop(&error)) << error;
  EXPECT_FLOAT_EQ(ECSManager::GetComponent<TransformComponent>(player).position.x, 1.f);
}

TEST_F(SceneManagerTest, StopReturnsToTheDirtyFlagFromBeforePlay) {
  std::string error;

  // A scene that was already dirty stays dirty: its unsaved edits are part of
  // the snapshot, so stopping does not make them saved.
  SceneManager::MarkDirty();
  ASSERT_TRUE(SceneManager::Play(&error)) << error;
  ASSERT_TRUE(SceneManager::Stop(&error)) << error;
  EXPECT_TRUE(SceneManager::IsDirty());
}

TEST_F(SceneManagerTest, EditsMadeWhilePlayingDoNotLeaveTheSceneDirty) {
  std::string error;

  ASSERT_FALSE(SceneManager::IsDirty());
  ASSERT_TRUE(SceneManager::Play(&error)) << error;

  SceneManager::MarkDirty();
  ASSERT_TRUE(SceneManager::IsDirty());

  ASSERT_TRUE(SceneManager::Stop(&error)) << error;
  EXPECT_FALSE(SceneManager::IsDirty());
}

TEST_F(SceneManagerTest, SaveIsRefusedWhilePlaying) {
  const std::filesystem::path scene = m_directory / "scene.xml";
  std::string error;

  ASSERT_TRUE(SceneManager::Create(scene, &error)) << error;
  ASSERT_TRUE(SceneManager::Play(&error)) << error;

  EXPECT_FALSE(SceneManager::Save(&error));
  EXPECT_NE(error.find("stop the playing scene"), std::string::npos);

  ASSERT_TRUE(SceneManager::Stop(&error)) << error;
  ASSERT_TRUE(SceneManager::Save(&error)) << error;
  EXPECT_TRUE(std::filesystem::exists(scene));
}

TEST_F(SceneManagerTest, OpeningASceneDiscardsTheSession) {
  const std::filesystem::path first = m_directory / "first.xml";
  const std::filesystem::path second = m_directory / "second.xml";
  std::string error;

  ASSERT_TRUE(SceneManager::Create(first, &error)) << error;
  CreateEntity("player", glm::vec3(0.f));

  ASSERT_TRUE(SceneManager::Play(&error)) << error;
  ASSERT_TRUE(SceneManager::IsPlaying());

  // A scene switch leaves the old scene behind rather than restoring it, so the
  // session is dropped before the new file is read.
  ASSERT_TRUE(SceneIO::Save(second, &error)) << error;
  ASSERT_TRUE(SceneManager::Open(second, &error)) << error;

  EXPECT_EQ(SceneManager::GetPlayState(), SceneManager::PlayState::kStopped);
  EXPECT_FALSE(SceneManager::IsPlayActive());
  EXPECT_TRUE(SceneManager::Stop(&error));
}
}  // namespace
