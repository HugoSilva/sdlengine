#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

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
  EXPECT_NE(contents.find("<formatVersion>1</formatVersion>"), std::string::npos);
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
  CreateSprite("two", glm::vec3(3.f, 4.f, 0.f));

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

// A scene saved by the first published format stays loadable. This is the
// regression the version policy exists for: the fixture is never rewritten,
// so a later change that stops reading it is caught here rather than by a user
// opening an old project.
TEST_F(SceneIOTest, AV1SceneLoadsFromTheFixture) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v1.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  ASSERT_EQ(CountEntities(), 2u);

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
          EXPECT_EQ(sprite.m_sprite->GetColor(), 0xff00ff00u);
        } else if (basic.m_name == "floor") {
          EXPECT_TRUE(basic.m_is_static);
          EXPECT_FLOAT_EQ(transform.position.y, 80.f);
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
