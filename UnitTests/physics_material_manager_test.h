#pragma once

#include <core.h>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <core/asset_registry.h>
#include <core/physics_material_file.h>
#include <core/physics_material_manager.h>

/*
The manager is the asset-to-values cache. These tests run without a physics
world, so they pin registry resolution, the one-entry-per-material contract, and
the defaults a missing material still answers with.
*/
namespace {

using namespace rse;

class PhysicsMaterialManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_physics_material_manager_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory / "assets");

    PhysicsMaterialManager::Clean();
    AssetRegistry::Clean();
    AssetRegistry::Init(m_directory);

    // The manager resolves a registered asset path through the project. Attach
    // the static project to the temporary directory without loading a manifest:
    // SetFile is the whole of what ResolveProjectPath reads.
    m_previous_project = Core::GetProject().File();
    Core::GetProject().SetFile(m_directory / "project.xml");
  }

  void TearDown() override {
    PhysicsMaterialManager::Clean();
    AssetRegistry::Clean();
    Core::GetProject().SetFile(m_previous_project);
    std::filesystem::remove_all(m_directory);
  }

  std::filesystem::path WriteMaterial(const std::string& name, float density, float friction, float restitution) {
    const std::filesystem::path path = m_directory / "assets" / (name + ".physicsmaterial");

    PhysicsMaterialFile material;
    material.name = name;
    material.material.density = density;
    material.material.friction = friction;
    material.material.restitution = restitution;

    std::string error;
    EXPECT_TRUE(PhysicsMaterialFile::Save(path, material, &error)) << error;
    return path;
  }

  std::filesystem::path m_directory;
  std::filesystem::path m_previous_project;
};

TEST_F(PhysicsMaterialManagerTest, ARegisteredMaterialResolvesThroughTheRegistry) {
  WriteMaterial("ice", 0.9f, 0.05f, 0.1f);
  const std::string id = AssetRegistry::Add("ice.physicsmaterial", "physics_material", "assets/ice.physicsmaterial");

  const PhysicsMaterialAsset* material = PhysicsMaterialManager::GetOrLoad(id);
  ASSERT_NE(material, nullptr);

  EXPECT_EQ(material->id, id);
  EXPECT_EQ(material->name, "ice");
  EXPECT_EQ(material->path, Core::ResolveProjectPath("assets/ice.physicsmaterial"));
  EXPECT_FLOAT_EQ(material->material.density, 0.9f);
  EXPECT_FLOAT_EQ(material->material.friction, 0.05f);
  EXPECT_FLOAT_EQ(material->material.restitution, 0.1f);
  EXPECT_EQ(PhysicsMaterialManager::Count(), 1u);

  // The id, the file's display name and a second request for the id are the
  // same cached material; nothing is parsed twice.
  EXPECT_EQ(PhysicsMaterialManager::Get(id), material);
  EXPECT_EQ(PhysicsMaterialManager::Get("ice"), material);
  EXPECT_EQ(PhysicsMaterialManager::GetOrLoad(id), material);
  // The registry name and a scene's path are the same cached material, even
  // though the file's display name is just "ice".
  EXPECT_EQ(PhysicsMaterialManager::GetOrLoad("ice.physicsmaterial"), material);
  EXPECT_EQ(PhysicsMaterialManager::GetOrLoad("assets/ice.physicsmaterial"), material);
  EXPECT_EQ(PhysicsMaterialManager::Count(), 1u);
}

TEST_F(PhysicsMaterialManagerTest, AnUnregisteredPathLoadsAndIsCached) {
  WriteMaterial("ice", 0.9f, 0.05f, 0.1f);

  const PhysicsMaterialAsset* material = PhysicsMaterialManager::GetOrLoad("assets/ice.physicsmaterial");
  ASSERT_NE(material, nullptr);
  EXPECT_TRUE(material->id.empty());
  EXPECT_EQ(material->name, "ice");
  EXPECT_FLOAT_EQ(material->material.density, 0.9f);

  EXPECT_EQ(PhysicsMaterialManager::GetOrLoad("assets/ice.physicsmaterial"), material);
  EXPECT_EQ(PhysicsMaterialManager::Count(), 1u);
}

TEST_F(PhysicsMaterialManagerTest, AMissingMaterialIsCachedWithTheDefaults) {
  const PhysicsMaterialAsset* material = PhysicsMaterialManager::GetOrLoad("assets/missing.physicsmaterial");
  ASSERT_NE(material, nullptr);
  EXPECT_FLOAT_EQ(material->material.density, PhysicsMaterial::kDefaultDensity);
  EXPECT_FLOAT_EQ(material->material.friction, PhysicsMaterial::kDefaultFriction);
  EXPECT_FLOAT_EQ(material->material.restitution, PhysicsMaterial::kDefaultRestitution);
  EXPECT_EQ(PhysicsMaterialManager::Count(), 1u);

  EXPECT_EQ(PhysicsMaterialManager::GetOrLoad("assets/missing.physicsmaterial"), material);
  EXPECT_EQ(PhysicsMaterialManager::Count(), 1u);
}

TEST_F(PhysicsMaterialManagerTest, AReferenceOfTheWrongTypeIsNotAPhysicsMaterial) {
  const std::string id = AssetRegistry::Add("crate.png", "texture", "assets/crate.png");
  EXPECT_EQ(PhysicsMaterialManager::GetOrLoad(id), nullptr);
}

TEST_F(PhysicsMaterialManagerTest, InitPreloadsTheRegistry) {
  WriteMaterial("ice", 0.9f, 0.05f, 0.1f);
  const std::string id = AssetRegistry::Add("ice.physicsmaterial", "physics_material", "assets/ice.physicsmaterial");

  PhysicsMaterialManager::Init();

  const PhysicsMaterialAsset* material = PhysicsMaterialManager::Get(id);
  ASSERT_NE(material, nullptr);
  EXPECT_FLOAT_EQ(material->material.friction, 0.05f);
  EXPECT_EQ(PhysicsMaterialManager::Count(), 1u);
}

TEST_F(PhysicsMaterialManagerTest, CleanDropsEveryCachedMaterial) {
  WriteMaterial("ice", 0.9f, 0.05f, 0.1f);
  const std::string id = AssetRegistry::Add("ice.physicsmaterial", "physics_material", "assets/ice.physicsmaterial");

  ASSERT_NE(PhysicsMaterialManager::GetOrLoad(id), nullptr);
  ASSERT_EQ(PhysicsMaterialManager::Count(), 1u);

  PhysicsMaterialManager::Clean();
  EXPECT_EQ(PhysicsMaterialManager::Count(), 0u);
  EXPECT_EQ(PhysicsMaterialManager::Get(id), nullptr);
}

}  // namespace
