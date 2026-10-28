#pragma once

#include <core.h>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <components/material_shader.h>
#include <core/asset_registry.h>
#include <core/material_file.h>
#include <core/material_manager.h>

/*
The manager is the asset-to-program cache. These tests run without a GL context,
so they pin the part that does not need one: registry resolution, the one-entry
per material contract, and the failure object that keeps a missing material from
being retried every frame.
*/
namespace {

using namespace rse;

class MaterialManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_material_manager_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory / "assets");

    MaterialManager::Clean();
    AssetRegistry::Clean();
    AssetRegistry::Init(m_directory);

    // The manager resolves a registered asset path through the project. Attach
    // the static project to the temporary directory without loading a manifest:
    // SetFile is the whole of what ResolveProjectPath reads.
    m_previous_project = Core::GetProject().File();
    Core::GetProject().SetFile(m_directory / "project.xml");
  }

  void TearDown() override {
    MaterialManager::Clean();
    AssetRegistry::Clean();
    Core::GetProject().SetFile(m_previous_project);
    std::filesystem::remove_all(m_directory);
  }

  std::filesystem::path WriteMaterial(const std::string& name) {
    const std::filesystem::path path = m_directory / "assets" / (name + ".material");

    MaterialFile material;
    material.name = name;
    material.vertex_shader = "shaders/test.vert";
    material.fragment_shader = "shaders/test.frag";

    std::string error;
    EXPECT_TRUE(MaterialFile::Save(path, material, &error)) << error;
    return path;
  }

  std::filesystem::path m_directory;
  std::filesystem::path m_previous_project;
};

TEST_F(MaterialManagerTest, ARegisteredMaterialResolvesThroughTheRegistry) {
  WriteMaterial("warm");
  const std::string id = AssetRegistry::Add("warm.material", "material", "assets/warm.material");

  MaterialShader* material = MaterialManager::GetOrLoad(id);
  ASSERT_NE(material, nullptr);

  EXPECT_EQ(material->GetAssetId(), id);
  EXPECT_EQ(material->GetName(), "warm");
  EXPECT_EQ(material->GetAssetPath(), Core::ResolveProjectPath("assets/warm.material"));
  EXPECT_FALSE(material->IsLoaded());
  EXPECT_EQ(MaterialManager::Count(), 1u);

  // The id, the file's display name and a second request for the id are the
  // same cached material; nothing is parsed twice.
  EXPECT_EQ(MaterialManager::Get(id), material);
  EXPECT_EQ(MaterialManager::Get("warm"), material);
  EXPECT_EQ(MaterialManager::GetOrLoad(id), material);
  // The registry name and a scene's legacy path are the same cached material,
  // even though the file's display name is just "warm".
  EXPECT_EQ(MaterialManager::GetOrLoad("warm.material"), material);
  EXPECT_EQ(MaterialManager::GetOrLoad("assets/warm.material"), material);
  EXPECT_EQ(MaterialManager::Count(), 1u);
}

TEST_F(MaterialManagerTest, AMissingMaterialIsCachedWithNoProgram) {
  MaterialShader* material = MaterialManager::GetOrLoad("assets/missing.material");
  ASSERT_NE(material, nullptr);
  EXPECT_FALSE(material->IsLoaded());
  EXPECT_EQ(MaterialManager::Count(), 1u);

  MaterialShader* again = MaterialManager::GetOrLoad("assets/missing.material");
  EXPECT_EQ(again, material);
  EXPECT_EQ(MaterialManager::Count(), 1u);
}

TEST_F(MaterialManagerTest, AReferenceOfTheWrongTypeIsNotAMaterial) {
  const std::string id = AssetRegistry::Add("crate.png", "texture", "assets/crate.png");
  EXPECT_EQ(MaterialManager::GetOrLoad(id), nullptr);
}

TEST_F(MaterialManagerTest, CleanDropsEveryCachedMaterial) {
  WriteMaterial("warm");
  const std::string id = AssetRegistry::Add("warm.material", "material", "assets/warm.material");

  ASSERT_NE(MaterialManager::GetOrLoad(id), nullptr);
  ASSERT_EQ(MaterialManager::Count(), 1u);

  MaterialManager::Clean();
  EXPECT_EQ(MaterialManager::Count(), 0u);
  EXPECT_EQ(MaterialManager::Get(id), nullptr);
}

}  // namespace
