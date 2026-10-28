#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <texture_catalog.h>
#include <vector>

#include <gtest/gtest.h>

#include <core/asset_registry.h>

/*
The catalog is the inspector's texture list: registered texture assets plus the
image files the registry does not know yet. These tests pin what is offered,
what is ignored, and how a typed query narrows it -- without opening a window.
*/
namespace {

using namespace rse;

class TextureCatalogTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_texture_catalog_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory / "assets");
    std::filesystem::create_directories(m_directory / "scenes");

    AssetRegistry::Clean();
    AssetRegistry::Init(m_directory);
  }

  void TearDown() override {
    AssetRegistry::Clean();
    std::filesystem::remove_all(m_directory);
  }

  // A stand-in for an asset file: these tests are about the catalog, not pixels.
  void Touch(const std::string& relative) const {
    std::filesystem::create_directories((m_directory / relative).parent_path());
    std::ofstream file{m_directory / relative};
    file << "asset";
  }

  std::filesystem::path m_directory;
};

TEST_F(TextureCatalogTest, RegisteredTextureAssetsAreOfferedById) {
  Touch("assets/tex3.png");
  const std::string id = AssetRegistry::Add("tex3.png", "texture", "assets/tex3.png");

  const std::vector<TextureChoice> choices = BuildTextureCatalog(m_directory, "assets");

  ASSERT_EQ(1u, choices.size());
  EXPECT_EQ(id, choices.front().reference);
  EXPECT_EQ("tex3.png", choices.front().name);
  EXPECT_EQ("assets/tex3.png", choices.front().project_path);
  EXPECT_EQ("tex3.png", choices.front().asset_root_path);
  EXPECT_TRUE(choices.front().registered);
}

TEST_F(TextureCatalogTest, NonTextureAssetsAreIgnored) {
  Touch("assets/effect.wav");
  Touch("assets/warm.material");
  Touch("scenes/start.xml");
  AssetRegistry::Add("effect.wav", "sound", "assets/effect.wav");
  AssetRegistry::Add("warm.material", "material", "assets/warm.material");
  AssetRegistry::Add("start.xml", "prefab", "scenes/start.xml");

  EXPECT_TRUE(BuildTextureCatalog(m_directory, "assets").empty());
}

TEST_F(TextureCatalogTest, MaterialCatalogOffersRegisteredAndUnregisteredMaterials) {
  Touch("assets/warm.material");
  Touch("assets/cool.material");
  Touch("assets/crate.png");
  const std::string id = AssetRegistry::Add("warm.material", "material", "assets/warm.material");

  const std::vector<TextureChoice> choices = BuildMaterialCatalog(m_directory, "assets");

  ASSERT_EQ(2u, choices.size());
  EXPECT_EQ("cool.material", choices[0].name);
  EXPECT_EQ("warm.material", choices[1].name);
  EXPECT_FALSE(choices[0].registered);
  EXPECT_TRUE(choices[1].registered);
  EXPECT_EQ(id, choices[1].reference);
  EXPECT_EQ("assets/warm.material", choices[1].project_path);
}

TEST_F(TextureCatalogTest, AFolderOfMaterialsIsNotOfferedAsTextures) {
  Touch("assets/materials/warm.material");
  const std::vector<TextureChoice> textures = BuildTextureCatalog(m_directory, "assets");
  const std::vector<TextureChoice> materials = BuildMaterialCatalog(m_directory, "assets");

  EXPECT_TRUE(textures.empty());
  ASSERT_EQ(1u, materials.size());
  EXPECT_EQ("warm.material", materials.front().name);
}

TEST_F(TextureCatalogTest, RegisteredAssetIdWorksForMaterials) {
  Touch("assets/warm.material");
  const std::string id = AssetRegistry::Add("warm.material", "material", "assets/warm.material");

  EXPECT_EQ(id, RegisteredAssetIdForFile(m_directory / "assets" / "warm.material", m_directory, "material"));
  EXPECT_TRUE(RegisteredAssetIdForFile(m_directory / "assets" / "warm.material", m_directory, "texture").empty());
}

TEST_F(TextureCatalogTest, MaterialFileClassificationIsExact) {
  EXPECT_TRUE(IsMaterialFile("assets/warm.material"));
  EXPECT_TRUE(IsMaterialFile("assets/WARM.MATERIAL"));
  EXPECT_FALSE(IsMaterialFile("assets/warm.material.xml"));
  EXPECT_FALSE(IsMaterialFile("assets/crate.png"));
}

TEST_F(TextureCatalogTest, UnregisteredImagesInTheProjectAreOffered) {
  Touch("assets/unregistered.png");
  Touch("assets/readme.txt");
  Touch("scenes/start.xml");

  const std::vector<TextureChoice> choices = BuildTextureCatalog(m_directory, "assets");

  ASSERT_EQ(1u, choices.size());
  EXPECT_TRUE(choices.front().reference.empty());
  EXPECT_EQ("unregistered.png", choices.front().name);
  EXPECT_EQ("assets/unregistered.png", choices.front().project_path);
  EXPECT_EQ("unregistered.png", choices.front().asset_root_path);
  EXPECT_FALSE(choices.front().registered);
}

TEST_F(TextureCatalogTest, AnUnregisteredImageOutsideTheAssetRootIsStillOffered) {
  Touch("textures/hero.png");

  const std::vector<TextureChoice> choices = BuildTextureCatalog(m_directory, "assets");

  ASSERT_EQ(1u, choices.size());
  EXPECT_EQ("textures/hero.png", choices.front().project_path);
  EXPECT_TRUE(choices.front().asset_root_path.empty());
}

TEST_F(TextureCatalogTest, ARegisteredFileIsNotListedTwice) {
  Touch("assets/crate.png");
  AssetRegistry::Add("crate.png", "texture", "assets/crate.png");

  const std::vector<TextureChoice> choices = BuildTextureCatalog(m_directory, "assets");

  ASSERT_EQ(1u, choices.size());
  EXPECT_TRUE(choices.front().registered);
}

TEST_F(TextureCatalogTest, ChoicesAreSortedByName) {
  Touch("assets/zeta.png");
  Touch("assets/alpha.png");
  AssetRegistry::Add("zeta.png", "texture", "assets/zeta.png");
  AssetRegistry::Add("alpha.png", "texture", "assets/alpha.png");

  const std::vector<TextureChoice> choices = BuildTextureCatalog(m_directory, "assets");

  ASSERT_EQ(2u, choices.size());
  EXPECT_EQ("alpha.png", choices.front().name);
  EXPECT_EQ("zeta.png", choices.back().name);
}

TEST_F(TextureCatalogTest, HiddenDirectoriesAreSkipped) {
  Touch(".git/secret.png");

  EXPECT_TRUE(BuildTextureCatalog(m_directory, "assets").empty());
}

TEST_F(TextureCatalogTest, FilteringIsCaseInsensitiveAcrossNameIdAndPath) {
  TextureChoice choice;
  choice.name = "Crate.png";
  choice.reference = "0123456789abcdef";
  choice.project_path = "assets/sprites/crate.png";

  EXPECT_TRUE(MatchesTextureQuery(choice, ""));
  EXPECT_TRUE(MatchesTextureQuery(choice, "CRATE"));
  EXPECT_TRUE(MatchesTextureQuery(choice, "01234567"));
  EXPECT_TRUE(MatchesTextureQuery(choice, "sprites"));
  EXPECT_FALSE(MatchesTextureQuery(choice, "missing"));
}

TEST_F(TextureCatalogTest, RegisteredTextureIdForFileFindsTheExactPath) {
  Touch("assets/tex3.png");
  const std::string id = AssetRegistry::Add("tex3.png", "texture", "assets/tex3.png");

  EXPECT_EQ(id, RegisteredTextureIdForFile(m_directory / "assets" / "tex3.png", m_directory));
  EXPECT_TRUE(RegisteredTextureIdForFile(m_directory / "assets" / "other.png", m_directory).empty());
  EXPECT_TRUE(RegisteredTextureIdForFile(m_directory.parent_path() / "outside.png", m_directory).empty());
}

TEST_F(TextureCatalogTest, RegisteredTextureIdForFileIgnoresASameNamedFileElsewhere) {
  Touch("assets/tex3.png");
  Touch("textures/tex3.png");
  const std::string id = AssetRegistry::Add("tex3.png", "texture", "assets/tex3.png");

  EXPECT_EQ(id, RegisteredTextureIdForFile(m_directory / "assets" / "tex3.png", m_directory));
  EXPECT_TRUE(RegisteredTextureIdForFile(m_directory / "textures" / "tex3.png", m_directory).empty());
}

}  // namespace
