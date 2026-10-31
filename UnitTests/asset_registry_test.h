#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <core/asset_registry.h>

/*
The asset registry is the table that gives assets stable ids. These tests pin
what it stores, that an id never moves when the file does, and that a table this
build cannot read is refused rather than half-read.
*/
namespace {

using namespace rse;

class AssetRegistryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_asset_registry_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory / "assets");

    AssetRegistry::Clean();
    AssetRegistry::Init(m_directory);
  }

  void TearDown() override {
    AssetRegistry::Clean();
    std::filesystem::remove_all(m_directory);
  }

  // A stand-in for an asset file: these tests are about the table, not pixels.
  void TouchFile(const std::string& relative) const {
    std::filesystem::create_directories((m_directory / relative).parent_path());
    std::ofstream file{m_directory / relative};
    file << "asset";
  }

  std::filesystem::path m_directory;
};

TEST_F(AssetRegistryTest, AnEmptyDirectoryOpensAnEmptyRegistry) {
  EXPECT_TRUE(AssetRegistry::All().empty());
  EXPECT_TRUE(AssetRegistry::Find("0123456789abcdef") == nullptr);

  // No table is written until something is added: a project with no assets has
  // nothing to say.
  EXPECT_FALSE(std::filesystem::exists(m_directory / AssetRegistry::kFileName));
}

TEST_F(AssetRegistryTest, AddAndSaveRoundTrips) {
  const std::string id = AssetRegistry::Add("tex3.png", "texture", "assets/tex3.png");

  EXPECT_TRUE(AssetRegistry::IsId(id));

  std::string error;
  ASSERT_TRUE(AssetRegistry::Save(&error)) << error;
  EXPECT_TRUE(std::filesystem::exists(m_directory / AssetRegistry::kFileName));
  EXPECT_FALSE(std::filesystem::exists(std::filesystem::path{m_directory / AssetRegistry::kFileName}.concat(".tmp")));

  // A fresh registry reading the file back sees the same entry.
  AssetRegistry::Clean();
  AssetRegistry::Init(m_directory);

  ASSERT_EQ(AssetRegistry::All().size(), 1u);
  const AssetRegistry::Asset& asset = AssetRegistry::All().front();
  EXPECT_EQ(asset.id, id);
  EXPECT_EQ(asset.name, "tex3.png");
  EXPECT_EQ(asset.type, "texture");
  EXPECT_EQ(asset.path, "assets/tex3.png");

  EXPECT_EQ(AssetRegistry::Resolve(id), "assets/tex3.png");
  EXPECT_EQ(AssetRegistry::Name(id), "tex3.png");
  EXPECT_EQ(AssetRegistry::Type(id), "texture");
}

TEST_F(AssetRegistryTest, GeneratedIdsAreSixteenHexCharacters) {
  for (int index = 0; index < 64; ++index) {
    const std::string id = AssetRegistry::GenerateId();
    EXPECT_EQ(id.size(), 16u);
    EXPECT_TRUE(AssetRegistry::IsId(id)) << id;
  }

  EXPECT_FALSE(AssetRegistry::IsId(""));
  EXPECT_FALSE(AssetRegistry::IsId("tooshort"));
  EXPECT_FALSE(AssetRegistry::IsId("0123456789abcdeZ"));
}

TEST_F(AssetRegistryTest, SameNameAssetsGetDistinctIds) {
  const std::string first = AssetRegistry::Add("crate.png", "texture", "assets/a/crate.png");
  const std::string second = AssetRegistry::Add("crate.png", "texture", "assets/b/crate.png");

  EXPECT_NE(first, second);
  EXPECT_EQ(AssetRegistry::All().size(), 2u);
}

TEST_F(AssetRegistryTest, FindByPathMatchesABareFileName) {
  const std::string id = AssetRegistry::Add("crate.png", "texture", "assets/sprites/crate.png");

  ASSERT_NE(AssetRegistry::Find(id), nullptr);
  EXPECT_EQ(AssetRegistry::FindByPath("assets/sprites/crate.png")->id, id);

  // A scene written before the registry stored a project-relative path may name
  // the file the way the asset root did.
  EXPECT_EQ(AssetRegistry::FindByPath("sprites/crate.png")->id, id);
  EXPECT_EQ(AssetRegistry::FindByPath("crate.png")->id, id);
  EXPECT_EQ(AssetRegistry::FindByPath("other.png"), nullptr);
}

TEST_F(AssetRegistryTest, FindReferencePrefersIdThenNameThenPath) {
  const std::string crate = AssetRegistry::Add("crate.png", "texture", "assets/crate.png");
  const std::string effect = AssetRegistry::Add("effect.wav", "sound", "assets/effect.wav");

  ASSERT_NE(AssetRegistry::FindReference(crate), nullptr);
  EXPECT_EQ(AssetRegistry::FindReference(crate)->id, crate);
  EXPECT_EQ(AssetRegistry::FindReference("crate.png")->id, crate);
  EXPECT_EQ(AssetRegistry::FindReference("assets/effect.wav")->id, effect);
  EXPECT_EQ(AssetRegistry::FindReference("nothing.png"), nullptr);
}

TEST_F(AssetRegistryTest, RenameKeepsTheIdAndMovesTheFile) {
  TouchFile("assets/crate.png");
  const std::string id = AssetRegistry::Add("crate.png", "texture", "assets/crate.png");

  std::string error;
  ASSERT_TRUE(AssetRegistry::Rename(id, "box.png", &error)) << error;

  // The id is what a scene stores, and it did not move: this is the property
  // the registry exists for.
  EXPECT_EQ(AssetRegistry::Find(id), AssetRegistry::FindByName("box.png"));
  EXPECT_EQ(AssetRegistry::Name(id), "box.png");
  EXPECT_EQ(AssetRegistry::Resolve(id), "assets/box.png");

  EXPECT_TRUE(std::filesystem::exists(m_directory / "assets" / "box.png"));
  EXPECT_FALSE(std::filesystem::exists(m_directory / "assets" / "crate.png"));
}

TEST_F(AssetRegistryTest, RenamingWithoutAnExtensionKeepsTheOldOne) {
  TouchFile("assets/crate.png");
  const std::string id = AssetRegistry::Add("crate.png", "texture", "assets/crate.png");

  std::string error;
  ASSERT_TRUE(AssetRegistry::Rename(id, "box", &error)) << error;

  EXPECT_EQ(AssetRegistry::Name(id), "box.png");
  EXPECT_EQ(AssetRegistry::Resolve(id), "assets/box.png");
  EXPECT_TRUE(std::filesystem::exists(m_directory / "assets" / "box.png"));
}

TEST_F(AssetRegistryTest, RenamingOntoAnExistingFileIsRefused) {
  TouchFile("assets/crate.png");
  TouchFile("assets/box.png");
  const std::string id = AssetRegistry::Add("crate.png", "texture", "assets/crate.png");

  std::string error;
  EXPECT_FALSE(AssetRegistry::Rename(id, "box.png", &error));
  EXPECT_FALSE(error.empty());

  // Nothing moved.
  EXPECT_EQ(AssetRegistry::Name(id), "crate.png");
  EXPECT_TRUE(std::filesystem::exists(m_directory / "assets" / "crate.png"));
}

TEST_F(AssetRegistryTest, RemoveDropsTheEntryAndKeepsTheFile) {
  TouchFile("assets/crate.png");
  const std::string id = AssetRegistry::Add("crate.png", "texture", "assets/crate.png");

  EXPECT_TRUE(AssetRegistry::Remove(id));
  EXPECT_EQ(AssetRegistry::Find(id), nullptr);
  EXPECT_TRUE(AssetRegistry::All().empty());

  // Removing an entry never deletes what a user authored.
  EXPECT_TRUE(std::filesystem::exists(m_directory / "assets" / "crate.png"));

  EXPECT_FALSE(AssetRegistry::Remove(id));
}

TEST_F(AssetRegistryTest, TypeForPathNamesTheFilesTheRegistryKnows) {
  EXPECT_EQ(AssetRegistry::TypeForPath("crate.png"), "texture");
  EXPECT_EQ(AssetRegistry::TypeForPath("CRATE.JPG"), "texture");
  EXPECT_EQ(AssetRegistry::TypeForPath("effect.wav"), "sound");
  EXPECT_EQ(AssetRegistry::TypeForPath("menu.ogg"), "sound");
  EXPECT_EQ(AssetRegistry::TypeForPath("main.ttf"), "font");
  EXPECT_EQ(AssetRegistry::TypeForPath("ice.physicsmaterial"), "physics_material");
  EXPECT_EQ(AssetRegistry::TypeForPath("ICE.PHYSICSMATERIAL"), "physics_material");
  EXPECT_EQ(AssetRegistry::TypeForPath("crate.xml"), "prefab");
  EXPECT_EQ(AssetRegistry::TypeForPath("readme.txt"), "");
}

TEST_F(AssetRegistryTest, AVersionZeroTableIsNotARegistry) {
  std::ofstream file{m_directory / AssetRegistry::kFileName};
  file << "<cereal><assets><formatVersion>0</formatVersion></assets></cereal>";
  file.close();

  AssetRegistry::Clean();
  AssetRegistry::Init(m_directory);

  // A bad table leaves an empty registry rather than a half-read one.
  EXPECT_TRUE(AssetRegistry::All().empty());
}

TEST_F(AssetRegistryTest, ANewerTableIsRefused) {
  std::ofstream file{m_directory / AssetRegistry::kFileName};
  file << "<cereal><assets><formatVersion>99</formatVersion></assets></cereal>";
  file.close();

  std::string error;
  EXPECT_FALSE(AssetRegistry::Load(m_directory / AssetRegistry::kFileName, &error));
  EXPECT_NE(error.find("99"), std::string::npos);
  EXPECT_NE(error.find("newer"), std::string::npos);
}

TEST_F(AssetRegistryTest, ASaveWithoutADirectoryReportsFailure) {
  AssetRegistry::Clean();

  std::string error;
  EXPECT_FALSE(AssetRegistry::Save(&error));
  EXPECT_FALSE(error.empty());
}

// The checked-in version 1 table: the file a later format change has to keep
// reading, and the pair the version 6 scene fixture resolves against.
TEST_F(AssetRegistryTest, AV1RegistryLoadsFromTheFixture) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "assets_v1.xml";

  std::string error;
  ASSERT_TRUE(AssetRegistry::Load(fixture, &error)) << error;

  ASSERT_EQ(AssetRegistry::All().size(), 2u);

  const AssetRegistry::Asset* texture = AssetRegistry::Find("0123456789abcdef");
  ASSERT_NE(texture, nullptr);
  EXPECT_EQ(texture->name, "tex3.png");
  EXPECT_EQ(texture->type, "texture");
  EXPECT_EQ(texture->path, "assets/tex3.png");

  const AssetRegistry::Asset* sound = AssetRegistry::FindByName("effect.wav");
  ASSERT_NE(sound, nullptr);
  EXPECT_EQ(sound->id, "fedcba9876543210");
  EXPECT_EQ(sound->type, "sound");
}
}  // namespace
