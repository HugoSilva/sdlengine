#pragma once

// The project commands live next to the editor, not in the engine, so this
// test is only built when the editor's command translation unit is compiled in
// (see UnitTests/CMakeLists.txt).
#ifdef RSE_EDITOR_COMMANDS

#include <core.h>
#include <filesystem>
#include <fstream>
#include <project_commands.h>
#include <string>

#include <gtest/gtest.h>

#include <core/project.h>
#include <core/scene_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/scene_io.h>

/*
The project commands are the part of the editor that writes files: a new
project, a scene saved under a second name, a project saved whole. They run
headless here -- no dialog opens, so no window is needed.
*/
namespace {

using namespace rse;

class ProjectCommandsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_root = std::filesystem::temp_directory_path() / "rse_project_commands_test";
    std::filesystem::remove_all(m_root);
    std::filesystem::create_directories(m_root);

    Core::GetProject() = Project{};
    SceneManager::Clean();
    SceneIO::Clear();
  }

  void TearDown() override {
    Core::GetProject() = Project{};
    SceneManager::Clean();
    SceneIO::Clear();
    std::filesystem::remove_all(m_root);
  }

  static uint32_t CountEntities() {
    uint32_t entities = 0;
    ECSManager::GetView<BasicComponent>().each([&entities](auto, BasicComponent&) { ++entities; });
    return entities;
  }

  static uint32_t AddSprite(const glm::vec3& position) {
    const uint32_t entity = ECSManager::CreateEntity();
    ECSManager::AddComponent<BasicComponent>(entity, "sprite", false, 0, 0);
    ECSManager::AddComponent<TransformComponent>(entity, position, glm::vec3(0.0f), glm::vec3(1.0f));
    ECSManager::AddComponent<SpriteComponent>(entity, new Sprite(position, glm::vec2(8.0f), 0xffffffffu));
    return entity;
  }

  std::filesystem::path m_root;
};

TEST_F(ProjectCommandsTest, NewProjectWritesTheFolderStructure) {
  const std::filesystem::path directory = m_root / "MyGame";

  NewProjectAt(directory);

  ASSERT_TRUE(std::filesystem::exists(directory / "project.xml"));
  ASSERT_TRUE(std::filesystem::exists(directory / "scenes" / "start.xml"));
  ASSERT_TRUE(std::filesystem::is_directory(directory / "assets"));
  ASSERT_TRUE(std::filesystem::is_directory(directory / "prefabs"));

  Project project;
  std::string error;
  ASSERT_TRUE(Project::Load(directory / "project.xml", project, &error)) << error;

  EXPECT_EQ(project.name, "MyGame");
  EXPECT_EQ(project.window.title, "MyGame");
  EXPECT_EQ(project.startup_scene, "scenes/start.xml");
  ASSERT_EQ(project.scenes.size(), 1u);
  EXPECT_EQ(project.scenes.front(), "scenes/start.xml");

  // Creating a project opens it; the first scene is the one it starts with.
  EXPECT_TRUE(Core::GetProject().IsAttached());
  EXPECT_EQ(SceneManager::GetActiveSceneName(), "start");
}

TEST_F(ProjectCommandsTest, SaveProjectWritesTheOpenScene) {
  NewProjectAt(m_root / "MyGame");
  AddSprite(glm::vec3(10.0f, 20.0f, 0.0f));
  SceneManager::MarkDirty();

  std::string error;
  ASSERT_TRUE(Core::SaveProject(&error)) << error;
  EXPECT_FALSE(SceneManager::IsDirty());

  SceneIO::Clear();
  ASSERT_TRUE(SceneIO::Load(SceneManager::GetActiveScene(), &error)) << error;
  EXPECT_EQ(CountEntities(), 1u);
}

TEST_F(ProjectCommandsTest, SaveProjectWithoutAProjectReportsFailure) {
  std::string error;

  EXPECT_FALSE(Core::SaveProject(&error));
  EXPECT_FALSE(error.empty());
}

TEST_F(ProjectCommandsTest, SaveSceneAsAddsTheSceneToTheManifest) {
  const std::filesystem::path directory = m_root / "MyGame";
  NewProjectAt(directory);
  AddSprite(glm::vec3(5.0f, 5.0f, 0.0f));

  const std::filesystem::path scene = directory / "scenes" / "level_02.xml";
  SaveSceneAsAt(scene);

  EXPECT_TRUE(std::filesystem::exists(scene));
  EXPECT_EQ(SceneManager::GetActiveSceneName(), "level_02");

  Project project;
  std::string error;
  ASSERT_TRUE(Project::Load(directory / "project.xml", project, &error)) << error;
  ASSERT_EQ(project.scenes.size(), 2u);
  EXPECT_EQ(project.scenes.at(0), "scenes/start.xml");
  EXPECT_EQ(project.scenes.at(1), "scenes/level_02.xml");
}

TEST_F(ProjectCommandsTest, SaveSceneAsOutsideTheProjectIsRefused) {
  const std::filesystem::path directory = m_root / "MyGame";
  NewProjectAt(directory);

  const std::filesystem::path outside = m_root / "elsewhere.xml";
  SaveSceneAsAt(outside);

  EXPECT_FALSE(std::filesystem::exists(outside));
  EXPECT_EQ(Core::GetProject().scenes.size(), 1u);
  EXPECT_EQ(SceneManager::GetActiveSceneName(), "start");
}

TEST_F(ProjectCommandsTest, CreatePrefabWritesTheSelectedEntityAndPlacesIt) {
  const std::filesystem::path directory = m_root / "MyGame";
  NewProjectAt(directory);

  const uint32_t entity = AddSprite(glm::vec3(10.0f, 20.0f, 0.0f));
  const std::filesystem::path prefab = directory / "prefabs" / "sprite.xml";

  CreatePrefabAt(entity, prefab);

  ASSERT_TRUE(std::filesystem::exists(prefab));
  EXPECT_FALSE(std::filesystem::exists(std::filesystem::path{prefab}.concat(".tmp")));
  EXPECT_EQ(CountEntities(), 1u);

  // Placing it copies the entity into the open scene and marks it unsaved; the
  // original is still there under its own id.
  InstantiatePrefabAt(prefab);

  EXPECT_EQ(CountEntities(), 2u);
  EXPECT_TRUE(SceneManager::IsDirty());
}

TEST_F(ProjectCommandsTest, CreatePrefabOutsideTheProjectIsRefused) {
  const std::filesystem::path directory = m_root / "MyGame";
  NewProjectAt(directory);

  const uint32_t entity = AddSprite(glm::vec3(10.0f, 20.0f, 0.0f));
  const std::filesystem::path outside = m_root / "elsewhere.xml";

  CreatePrefabAt(entity, outside);

  EXPECT_FALSE(std::filesystem::exists(outside));
}

TEST_F(ProjectCommandsTest, OpeningAProjectSwitchesTheProjectAndTheScene) {
  NewProjectAt(m_root / "First");
  ASSERT_EQ(Core::GetProject().name, "First");

  NewProjectAt(m_root / "Second");

  EXPECT_EQ(Core::GetProject().name, "Second");
  EXPECT_EQ(SceneManager::GetActiveSceneName(), "start");
  EXPECT_EQ(SceneManager::GetActiveScene(), m_root / "Second" / "scenes" / "start.xml");
}

TEST_F(ProjectCommandsTest, OpeningAMissingProjectChangesNothing) {
  NewProjectAt(m_root / "First");
  const std::filesystem::path before = Core::GetProject().File();

  std::string error;
  EXPECT_FALSE(Core::OpenProject(m_root / "Missing" / "project.xml", &error));
  EXPECT_FALSE(error.empty());
  EXPECT_EQ(Core::GetProject().File(), before);
}

TEST_F(ProjectCommandsTest, ResolveResourcePrefersTheProjectAssetRoot) {
  const std::filesystem::path directory = m_root / "MyGame";
  NewProjectAt(directory);

  const std::filesystem::path asset = directory / "assets" / "probe.png";
  std::ofstream file(asset, std::ios::binary);
  file << "probe";
  file.close();

  EXPECT_EQ(std::filesystem::path{Core::ResolveResource("probe.png")}, asset);
}

TEST_F(ProjectCommandsTest, ResolveResourceFallsBackToTheEngineResources) {
  const std::string engine_copy = (std::filesystem::path("Resources") / "tex3.png").string();

  EXPECT_EQ(Core::ResolveResource("tex3.png"), engine_copy);

  // A project that does not ship the file still gets the engine's copy.
  NewProjectAt(m_root / "MyGame");
  EXPECT_EQ(Core::ResolveResource("tex3.png"), engine_copy);
}
}  // namespace

#endif  // RSE_EDITOR_COMMANDS
