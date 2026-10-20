#pragma once

#include <algorithm>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <core/project.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
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

TEST(SampleProjectTest, ManifestLoads) {
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

TEST(SampleProjectTest, EveryListedSceneLoads) {
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

TEST(SampleProjectTest, AssetRootExists) {
  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(kSampleProject / "project.xml", project, &error)) << error;
  EXPECT_TRUE(std::filesystem::is_directory(project.Resolve(project.asset_root)));
}
}  // namespace
