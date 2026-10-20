#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <core/texture_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/scene_io.h>

/*
Scenes are files. These tests pin the file format and the load contract: the
header is validated before the open scene is touched, a failed load leaves an
empty scene rather than half of two, and loading the same file twice must not
duplicate the entities -- the old ECSManager::Load restored over the live
registry and did exactly that.
*/
namespace {

using namespace rse;

class SceneIOTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_scene_io_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);

    m_file = m_directory / "start.xml";
    m_scratch = std::filesystem::path{m_file}.concat(".tmp");

    SceneIO::Clear();
  }

  void TearDown() override {
    SceneIO::Clear();
    TextureManager::Clean();
    std::filesystem::remove_all(m_directory);
  }

  void WriteFile(const std::string& contents) const {
    std::ofstream file{m_file};
    file << contents;
  }

  static std::string ReadFile(const std::filesystem::path& file) {
    std::ifstream stream{file};
    return std::string{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
  }

  static uint32_t CountEntities() {
    uint32_t entities = 0;
    ECSManager::GetView<BasicComponent>().each([&entities](auto, BasicComponent&) { ++entities; });
    return entities;
  }

  static uint32_t CreateSprite(const char* name, const glm::vec3& position) {
    const uint32_t entity = ECSManager::CreateEntity();
    ECSManager::AddComponent<BasicComponent>(entity, name, false, 0, 0);
    ECSManager::AddComponent<TransformComponent>(entity, position, glm::vec3(0.f), glm::vec3(1.f));
    ECSManager::AddComponent<SpriteComponent>(entity, new Sprite(position, glm::vec2(8.f, 8.f), 0xff00ff00u));
    return entity;
  }

  // A sprite that names its texture the way a loaded scene does: the texture
  // comes from the manager and the sprite borrows it.
  static uint32_t CreateTexturedSprite(const char* name, const glm::vec3& position, const std::string& texture,
                                       unsigned int color) {
    const uint32_t entity = ECSManager::CreateEntity();
    ECSManager::AddComponent<BasicComponent>(entity, name, false, 0, 0);
    ECSManager::AddComponent<TransformComponent>(entity, position, glm::vec3(0.f), glm::vec3(1.f));

    Sprite* sprite = new Sprite(position, glm::vec2(8.f, 8.f), TextureManager::GetOrLoad(texture));
    sprite->SetColor(color);
    ECSManager::AddComponent<SpriteComponent>(entity, sprite);
    return entity;
  }

  std::filesystem::path m_directory;
  std::filesystem::path m_file;
  std::filesystem::path m_scratch;
};

TEST_F(SceneIOTest, SavingWritesTheSceneFile) {
  std::string error;

  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  EXPECT_TRUE(std::filesystem::exists(m_file));
  EXPECT_FALSE(std::filesystem::exists(m_scratch));

  const std::string contents = ReadFile(m_file);
  EXPECT_NE(contents.find("<formatVersion>2</formatVersion>"), std::string::npos);
  EXPECT_NE(contents.find("<entities>"), std::string::npos);
}

TEST_F(SceneIOTest, SavedEntitiesComeBack) {
  CreateSprite("player", glm::vec3(12.5f, 34.25f, 0.f));

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();
  ASSERT_EQ(CountEntities(), 0u);

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;
  ASSERT_EQ(CountEntities(), 1u);

  ECSManager::GetView<BasicComponent, TransformComponent, SpriteComponent>().each(
      [](auto, BasicComponent& basic, TransformComponent& transform, SpriteComponent& sprite) {
        EXPECT_EQ(basic.m_name, "player");
        EXPECT_FALSE(basic.m_is_static);
        EXPECT_FLOAT_EQ(transform.position.x, 12.5f);
        EXPECT_FLOAT_EQ(transform.position.y, 34.25f);
        EXPECT_FLOAT_EQ(transform.scale.x, 1.f);
        ASSERT_NE(sprite.m_sprite, nullptr);
        EXPECT_FLOAT_EQ(sprite.m_sprite->GetSize().x, 8.f);
        EXPECT_EQ(sprite.m_sprite->GetColor(), 0xff00ff00u);
      });
}

TEST_F(SceneIOTest, AnEmptySceneRoundTrips) {
  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;
  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  EXPECT_EQ(CountEntities(), 0u);
}

TEST_F(SceneIOTest, LoadingTwiceDoesNotDuplicateEntities) {
  CreateSprite("one", glm::vec3(1.f, 2.f, 0.f));
  CreateSprite("two", glm::vec3(3.f, 4.f, 0.f));

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;
  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  EXPECT_EQ(CountEntities(), 2u);
}

TEST_F(SceneIOTest, SavingAfterLoadingProducesTheSameFile) {
  CreateSprite("one", glm::vec3(1.f, 2.f, 0.f));
  CreateTexturedSprite("two", glm::vec3(3.f, 4.f, 0.f), "shared.png", 0xff00ff00u);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;
  const std::string first_write = ReadFile(m_file);

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  EXPECT_EQ(ReadFile(m_file), first_write);
}

TEST_F(SceneIOTest, LoadWithoutAFileReportsFailure) {
  std::string error;

  EXPECT_NO_THROW({ EXPECT_FALSE(SceneIO::Load(m_file, &error)); });
  EXPECT_NE(error.find("not found"), std::string::npos);
}

TEST_F(SceneIOTest, LoadOfAnEmptyFileReportsFailure) {
  WriteFile("");

  std::string error;

  EXPECT_NO_THROW({ EXPECT_FALSE(SceneIO::Load(m_file, &error)); });
  EXPECT_FALSE(error.empty());
}

TEST_F(SceneIOTest, LoadOfAMalformedFileReportsFailure) {
  WriteFile("<cereal>this is not a scene file");

  std::string error;

  EXPECT_NO_THROW({ EXPECT_FALSE(SceneIO::Load(m_file, &error)); });
  EXPECT_FALSE(error.empty());
}

TEST_F(SceneIOTest, AnUnknownFormatVersionIsRefusedWithoutTouchingTheScene) {
  CreateSprite("keep", glm::vec3(1.f, 2.f, 0.f));
  WriteFile("<cereal><formatVersion>99</formatVersion><entities><value0>0</value0></entities></cereal>");

  std::string error;

  EXPECT_FALSE(SceneIO::Load(m_file, &error));
  EXPECT_NE(error.find("99"), std::string::npos);
  EXPECT_NE(error.find("newer"), std::string::npos);

  // Refusing a file must not cost the author the scene that is already open.
  EXPECT_EQ(CountEntities(), 1u);
}

TEST_F(SceneIOTest, ASceneStoresTheTextureByName) {
  CreateTexturedSprite("crate", glm::vec3(1.f, 2.f, 0.f), "crate.png", 0xffff0000u);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  const std::string contents = ReadFile(m_file);

  // The name, as text -- not the texture's pixels and not a pointer record.
  EXPECT_NE(contents.find("<Texture>crate.png</Texture>"), std::string::npos);
}

TEST_F(SceneIOTest, SpritesThatShareATextureNameShareOneTexture) {
  CreateTexturedSprite("left", glm::vec3(0.f), "shared.png", 0xff00ff00u);
  CreateTexturedSprite("right", glm::vec3(16.f, 0.f, 0.f), "shared.png", 0xffff0000u);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  // Empty the manager as well as the registry, so what the load creates is
  // visible: two sprites naming one image have to end up with one texture.
  SceneIO::Clear();
  TextureManager::Clean();
  ASSERT_EQ(TextureManager::Count(), 0u);

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  EXPECT_EQ(TextureManager::Count(), 1u);
  EXPECT_NE(TextureManager::Get("shared.png"), nullptr);

  int sprites = 0;

  ECSManager::GetView<SpriteComponent>().each([&sprites](auto, SpriteComponent& component) {
    ++sprites;
    ASSERT_NE(component.m_sprite, nullptr);
    EXPECT_EQ(component.m_sprite->GetTextureName(), "shared.png");
  });

  EXPECT_EQ(sprites, 2);
}

TEST_F(SceneIOTest, AnUntexturedSpriteComesBackWithoutATexture) {
  CreateSprite("plain", glm::vec3(1.f, 2.f, 0.f));

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();
  TextureManager::Clean();

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  EXPECT_EQ(TextureManager::Count(), 0u);

  ECSManager::GetView<SpriteComponent>().each([](auto, SpriteComponent& component) {
    ASSERT_NE(component.m_sprite, nullptr);
    EXPECT_EQ(component.m_sprite->GetTextureName(), "");
    EXPECT_EQ(component.m_sprite->GetTId(), 0u);
  });
}

// Version 2 changed what a sprite stores for its texture: a name instead of
// the texture itself. A version 1 file cannot be read as a version 2 one -- the
// field changed shape rather than gaining a tail -- so it is refused as too
// old. The fixture is kept because it pins that refusal, and because the day
// somebody writes the migration this test changes back to loading it.
TEST_F(SceneIOTest, AV1SceneIsRefusedAsOlderThanThisBuild) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v1.xml";

  CreateSprite("keep", glm::vec3(1.f, 2.f, 0.f));

  std::string error;

  EXPECT_FALSE(SceneIO::Load(fixture, &error));
  EXPECT_NE(error.find("older"), std::string::npos) << error;
  EXPECT_NE(error.find("version 1"), std::string::npos) << error;

  // Refusing a file must not cost the author the scene that is already open.
  EXPECT_EQ(CountEntities(), 1u);
}

// The version 2 fixture, written by the version 2 writer, is the file every
// later change has to keep reading: one untextured static floor and one crate
// that names its texture.
TEST_F(SceneIOTest, AV2SceneLoadsFromTheFixture) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v2.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  ASSERT_EQ(CountEntities(), 2u);
  EXPECT_EQ(TextureManager::Count(), 1u);

  int sprites = 0;

  ECSManager::GetView<BasicComponent, TransformComponent, SpriteComponent>().each(
      [&sprites](auto, BasicComponent& basic, TransformComponent& transform, SpriteComponent& sprite) {
        ++sprites;

        if (basic.m_name == "crate") {
          EXPECT_FALSE(basic.m_is_static);
          EXPECT_FLOAT_EQ(transform.position.x, 320.f);
          EXPECT_FLOAT_EQ(transform.position.y, 240.f);
          EXPECT_FLOAT_EQ(transform.scale.x, 1.f);
          ASSERT_NE(sprite.m_sprite, nullptr);
          EXPECT_FLOAT_EQ(sprite.m_sprite->GetSize().x, 64.f);
          EXPECT_EQ(sprite.m_sprite->GetColor(), 0xffff0000u);
          EXPECT_EQ(sprite.m_sprite->GetTextureName(), "shared.png");
        } else if (basic.m_name == "floor") {
          EXPECT_TRUE(basic.m_is_static);
          EXPECT_FLOAT_EQ(transform.position.y, 80.f);
          ASSERT_NE(sprite.m_sprite, nullptr);
          EXPECT_EQ(sprite.m_sprite->GetTextureName(), "");
        } else {
          ADD_FAILURE() << "unexpected entity " << basic.m_name;
        }
      });

  EXPECT_EQ(sprites, 2);
}

TEST_F(SceneIOTest, AVersionZeroSceneIsNotASceneFile) {
  WriteFile("<cereal><formatVersion>0</formatVersion><entities><value0>0</value0></entities></cereal>");

  std::string error;

  EXPECT_FALSE(SceneIO::Load(m_file, &error));
  EXPECT_NE(error.find("not a scene file"), std::string::npos);
}

TEST_F(SceneIOTest, AnUnwritablePathReportsFailureWithoutLeavingAScratchFile) {
  const std::filesystem::path missing = m_directory / "missing" / "scene.xml";
  const std::filesystem::path missing_scratch = std::filesystem::path{missing}.concat(".tmp");

  std::string error;

  EXPECT_FALSE(SceneIO::Save(missing, &error));
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(std::filesystem::exists(missing_scratch));
}
}  // namespace
