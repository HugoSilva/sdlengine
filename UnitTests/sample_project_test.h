#pragma once

#include <algorithm>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <audio/sound_manager.h>
#include <core/asset_registry.h>
#include <core/project.h>
#include <core/texture_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/prefab_io.h>
#include <ecs/scene_io.h>

/*
The sample project is checked in and deployed with the binaries, so it is the
one project that has to keep opening: these tests fail when the format moves
and the sample is left behind.
*/
namespace {

using namespace rse;

const std::filesystem::path kSampleProject{RSE_SAMPLE_PROJECT_DIR};

uint32_t CountEntities() {
  uint32_t entities = 0;
  ECSManager::GetView<BasicComponent>().each([&entities](auto, BasicComponent&) { ++entities; });
  return entities;
}

class SampleProjectTest : public ::testing::Test {
 protected:
  // The sample ships an asset registry, so the tests open it the way a project
  // does: before any scene, and cleaned up after.
  void SetUp() override { AssetRegistry::Init(kSampleProject); }

  void TearDown() override {
    SceneIO::Clear();
    TextureManager::Clean();
    SoundManager::Clean();
    AssetRegistry::Clean();
  }
};

TEST_F(SampleProjectTest, ManifestLoads) {
  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(kSampleProject / "project.xml", project, &error)) << error;

  EXPECT_EQ(project.format_version, Project::kFormatVersion);
  EXPECT_FALSE(project.name.empty());
  EXPECT_FALSE(project.window.title.empty());
  EXPECT_GT(project.window.width, 0);
  EXPECT_GT(project.window.height, 0);
  EXPECT_FALSE(project.scenes.empty());

  const auto startup = std::find(project.scenes.begin(), project.scenes.end(), project.startup_scene);
  EXPECT_NE(startup, project.scenes.end()) << "the startup scene has to be one of the project's scenes";
}

TEST_F(SampleProjectTest, ManifestSavesInTheCurrentVersion) {
  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(kSampleProject / "project.xml", project, &error)) << error;

  // Saving the shipped manifest must produce a file this build reads back: the
  // sample is the one project that ships with every binary.
  const std::filesystem::path directory = std::filesystem::temp_directory_path() / "rse_sample_manifest";
  std::filesystem::remove_all(directory);
  std::filesystem::create_directories(directory);
  project.SetFile(directory / "project.xml");

  ASSERT_TRUE(project.Save(&error)) << error;

  Project reloaded;
  ASSERT_TRUE(Project::Load(project.File(), reloaded, &error)) << error;

  EXPECT_EQ(reloaded.format_version, Project::kFormatVersion);
  EXPECT_EQ(reloaded.name, project.name);
  EXPECT_EQ(reloaded.scenes, project.scenes);
  EXPECT_EQ(reloaded.audio.master, project.audio.master);

  std::filesystem::remove_all(directory);
}

TEST_F(SampleProjectTest, EveryListedSceneLoads) {
  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(kSampleProject / "project.xml", project, &error)) << error;

  for (const std::string& scene : project.scenes) {
    const std::filesystem::path file = project.Resolve(scene);
    ASSERT_TRUE(std::filesystem::exists(file)) << file;

    SceneIO::Clear();
    ASSERT_TRUE(SceneIO::Load(file, &error)) << file << ": " << error;
    EXPECT_GT(CountEntities(), 0u) << file;
  }

  SceneIO::Clear();
}

TEST_F(SampleProjectTest, AssetRootExists) {
  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(kSampleProject / "project.xml", project, &error)) << error;
  EXPECT_TRUE(std::filesystem::is_directory(project.Resolve(project.asset_root)));
}

// The sample is what a reader copies: one image, many sprites. Loading the
// startup scene has to leave the manager with one texture and every sprite
// pointing at it, or the file demonstrates the opposite of the format's point.
TEST_F(SampleProjectTest, TheStartupSceneSharesOneTexture) {
  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(kSampleProject / "project.xml", project, &error)) << error;
  ASSERT_TRUE(SceneIO::Load(project.Resolve(project.startup_scene), &error)) << error;

  EXPECT_EQ(TextureManager::Count(), 1u);

  std::string shared;
  int sprites = 0;

  ECSManager::GetView<SpriteComponent>().each([&shared, &sprites](auto, SpriteComponent& component) {
    ++sprites;
    ASSERT_NE(component.m_sprite, nullptr);

    const std::string name = component.m_sprite->GetTextureName();
    EXPECT_FALSE(name.empty());

    if (shared.empty()) {
      shared = name;
    }

    EXPECT_EQ(name, shared);
  });

  EXPECT_GT(sprites, 1);
}
// The sample's second scene is the physics worked example: one static floor and
// one dynamic crate, both still uninitialised -- a loaded body starts like one
// added in the editor, and PhysicsSystem builds it on the first update.
TEST_F(SampleProjectTest, ThePhysicsSceneHasAFloorAndAFallingBody) {
  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(kSampleProject / "project.xml", project, &error)) << error;
  ASSERT_TRUE(SceneIO::Load(project.Resolve("scenes/physics.xml"), &error)) << error;

  EXPECT_EQ(ECSManager::GetView<RigidBodyComponent>().size(), 2u);

  int static_bodies = 0;
  int dynamic_bodies = 0;

  ECSManager::GetView<RigidBodyComponent>().each([&static_bodies, &dynamic_bodies](auto, RigidBodyComponent& rigid) {
    ASSERT_NE(rigid.m_rigidbody, nullptr);
    EXPECT_FALSE(rigid.m_rigidbody->m_init);

    if (*rigid.m_rigidbody->GetBodyType() == b2_staticBody) {
      ++static_bodies;
    } else if (*rigid.m_rigidbody->GetBodyType() == b2_dynamicBody) {
      ++dynamic_bodies;
    }
  });

  EXPECT_EQ(static_bodies, 1);
  EXPECT_EQ(dynamic_bodies, 1);
}

// The sample's prefab is the physics crate saved on its own: instantiating it
// places the same dynamic crate, uninitialised, at the position the file stores.
TEST_F(SampleProjectTest, ThePrefabPlacesTheCrate) {
  Project project;
  std::string error;
  ASSERT_TRUE(Project::Load(kSampleProject / "project.xml", project, &error)) << error;

  const uint32_t crate = rse::PrefabIO::Instantiate(kSampleProject / "prefabs" / "crate.xml", &error);
  ASSERT_NE(crate, static_cast<uint32_t>(entt::null)) << error;
  ASSERT_EQ(CountEntities(), 1u);

  ASSERT_TRUE(ECSManager::HasComponent<SpriteComponent>(crate));
  ASSERT_TRUE(ECSManager::HasComponent<RigidBodyComponent>(crate));

  EXPECT_EQ(ECSManager::GetComponent<BasicComponent>(crate).m_name, "crate");

  const TransformComponent& transform = ECSManager::GetComponent<TransformComponent>(crate);
  EXPECT_FLOAT_EQ(transform.position.x, 640.f);
  EXPECT_FLOAT_EQ(transform.position.y, 400.f);

  const SpriteComponent& sprite = ECSManager::GetComponent<SpriteComponent>(crate);
  ASSERT_NE(sprite.m_sprite, nullptr);
  EXPECT_EQ(sprite.m_sprite->GetTextureName(), "tex3.png");

  const RigidBodyComponent& rigidbody = ECSManager::GetComponent<RigidBodyComponent>(crate);
  ASSERT_NE(rigidbody.m_rigidbody, nullptr);
  EXPECT_FALSE(rigidbody.m_rigidbody->m_init);
  EXPECT_EQ(*rigidbody.m_rigidbody->GetBodyType(), b2_dynamicBody);
}
}  // namespace
