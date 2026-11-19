#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "scene_test_support.h"
#include <audio/sound_manager.h>
#include <core/texture_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/prefab_io.h>
#include <ecs/scene_io.h>

/*
A prefab is one entity saved from a scene and copied into a scene. These tests
pin the file format and the copy contract: the file is a version header and one
entity, instantiating never restores the file's ids over the open scene, and a
copy owns what an entity owns -- its sprite and its rigid body.
*/
namespace {

using namespace rse;

// The value entt uses for "no entity".
constexpr uint32_t kNullEntity{static_cast<uint32_t>(entt::null)};

class PrefabIOTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_prefab_io_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);

    m_file = m_directory / "crate.xml";
    m_scratch = std::filesystem::path{m_file}.concat(".tmp");

    TestScene().Clear();
  }

  void TearDown() override {
    TestScene().Clear();
    TextureManager::Clean();
    SoundManager::Clean();
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
    TestScene().Registry().view<BasicComponent>().each([&entities](auto, BasicComponent&) { ++entities; });
    return entities;
  }

  static uint32_t CreateSprite(const char* name, const glm::vec3& position) {
    const uint32_t entity = TestScene().CreateEntity().Handle();
    TestScene().Registry().assign<BasicComponent>(entity, name, false, 0, 0);
    TestScene().Registry().assign<TransformComponent>(entity, position, glm::vec3(0.f), glm::vec3(1.f));
    TestScene().Registry().assign<SpriteComponent>(entity, new Sprite(position, glm::vec2(8.f, 8.f), 0xff00ff00u));
    return entity;
  }

  static uint32_t CreateTexturedSprite(const char* name, const glm::vec3& position, const std::string& texture,
                                       unsigned int color) {
    const uint32_t entity = TestScene().CreateEntity().Handle();
    TestScene().Registry().assign<BasicComponent>(entity, name, false, 0, 0);
    TestScene().Registry().assign<TransformComponent>(entity, position, glm::vec3(0.f), glm::vec3(1.f));

    Sprite* sprite = new Sprite(position, glm::vec2(8.f, 8.f), TextureManager::GetOrLoad(texture));
    sprite->SetColor(color);
    TestScene().Registry().assign<SpriteComponent>(entity, sprite);
    return entity;
  }

  std::filesystem::path m_directory;
  std::filesystem::path m_file;
  std::filesystem::path m_scratch;
};

TEST_F(PrefabIOTest, SavingWritesThePrefabFile) {
  const uint32_t crate = CreateSprite("crate", glm::vec3(1.f, 2.f, 0.f));

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, crate, &error)) << error;

  EXPECT_TRUE(std::filesystem::exists(m_file));
  EXPECT_FALSE(std::filesystem::exists(m_scratch));

  const std::string contents = ReadFile(m_file);

  EXPECT_NE(contents.find("<formatVersion>5</formatVersion>"), std::string::npos);

  // One entity, not a scene: no camera, and the entity block has the same
  // component shape a scene's <entities> has.
  EXPECT_NE(contents.find("<entity>"), std::string::npos);
  EXPECT_EQ(contents.find("<camera>"), std::string::npos);
  EXPECT_NE(contents.find("<Name>crate</Name>"), std::string::npos);
}

// A prefab file names no identity: the format did not move for this, and every
// placement is a new entity, so it gets a new uuid rather than the source's.
TEST_F(PrefabIOTest, EveryPlacedCopyGetsItsOwnIdentity) {
  const uint32_t crate = CreateSprite("crate", glm::vec3(1.f, 2.f, 0.f));
  const std::string source_id = TestScene().Registry().get<UuidComponent>(crate).m_uuid.ToString();

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, crate, &error)) << error;

  const std::string contents = ReadFile(m_file);
  EXPECT_EQ(contents.find(source_id), std::string::npos);
  EXPECT_NE(contents.find("<formatVersion>5</formatVersion>"), std::string::npos);

  TestScene().Clear();

  const uint32_t first = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(first, kNullEntity) << error;

  const uint32_t second = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(second, kNullEntity) << error;

  const std::string first_id = TestScene().Registry().get<UuidComponent>(first).m_uuid.ToString();
  const std::string second_id = TestScene().Registry().get<UuidComponent>(second).m_uuid.ToString();

  EXPECT_NE(first_id, second_id);
  EXPECT_NE(first_id, source_id);
  EXPECT_NE(second_id, source_id);
}

TEST_F(PrefabIOTest, APlacedEntityComesBackWithItsValues) {
  const uint32_t crate = CreateSprite("crate", glm::vec3(12.5f, 34.25f, 0.f));

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, crate, &error)) << error;

  TestScene().Clear();
  ASSERT_EQ(CountEntities(), 0u);

  const uint32_t placed = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(placed, kNullEntity) << error;
  ASSERT_EQ(CountEntities(), 1u);

  TestScene().Registry().view<BasicComponent, TransformComponent, SpriteComponent>().each(
      [](auto, BasicComponent& basic, TransformComponent& transform, SpriteComponent& sprite) {
        EXPECT_EQ(basic.m_name, "crate");
        EXPECT_FALSE(basic.m_is_static);
        EXPECT_FLOAT_EQ(transform.position.x, 12.5f);
        EXPECT_FLOAT_EQ(transform.position.y, 34.25f);
        EXPECT_FLOAT_EQ(transform.scale.x, 1.f);
        ASSERT_NE(sprite.m_sprite, nullptr);
        EXPECT_FLOAT_EQ(sprite.m_sprite->GetSize().x, 8.f);
        EXPECT_EQ(sprite.m_sprite->GetColor(), 0xff00ff00u);
      });
}

TEST_F(PrefabIOTest, DisabledStateIsCopiedWithThePrefab) {
  const uint32_t crate = CreateSprite("crate", glm::vec3(1.f, 2.f, 0.f));
  TestScene().SetEntityEnabled(crate, false);

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, crate, &error)) << error;

  TestScene().Clear();
  const uint32_t placed = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(placed, kNullEntity) << error;

  ASSERT_TRUE(TestScene().Registry().has<EntityEnabledComponent>(placed));
  EXPECT_FALSE(TestScene().Registry().get<EntityEnabledComponent>(placed).m_enabled);
  EXPECT_FALSE(TestScene().IsEntityEnabled(placed));
}

TEST_F(PrefabIOTest, DisabledComponentStateIsCopiedWithThePrefab) {
  const uint32_t crate = CreateSprite("crate", glm::vec3(1.f, 2.f, 0.f));
  TestScene().SetComponentEnabled<SpriteComponent>(crate, false);
  TestScene().SetComponentEnabled<AudioComponent>(crate, false);

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, crate, &error)) << error;

  TestScene().Clear();
  const uint32_t placed = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(placed, kNullEntity) << error;

  ASSERT_TRUE(TestScene().Registry().has<DisabledComponentsComponent>(placed));
  EXPECT_FALSE(TestScene().IsComponentEnabled<SpriteComponent>(placed));
  EXPECT_FALSE(TestScene().IsComponentEnabled<AudioComponent>(placed));
  EXPECT_TRUE(TestScene().IsComponentEnabled<RigidBodyComponent>(placed));
}

TEST_F(PrefabIOTest, ASpriteAppearanceIsCopiedWithThePrefab) {
  const uint32_t crate = CreateSprite("crate", glm::vec3(1.f, 2.f, 0.f));

  SpriteAppearanceComponent appearance;
  appearance.flip_x = true;
  appearance.material = "mat_warm";
  TestScene().Registry().assign<SpriteAppearanceComponent>(crate, appearance);

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, crate, &error)) << error;

  TestScene().Clear();
  ASSERT_EQ(CountEntities(), 0u);

  const uint32_t placed = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(placed, kNullEntity) << error;
  ASSERT_TRUE(TestScene().Registry().has<SpriteAppearanceComponent>(placed));

  const SpriteAppearanceComponent& copied = TestScene().Registry().get<SpriteAppearanceComponent>(placed);
  EXPECT_TRUE(copied.flip_x);
  EXPECT_FALSE(copied.flip_y);
  EXPECT_EQ(copied.material, "mat_warm");
}

// The prefab was saved from this scene, so the live registry already has an
// entity at the id the file names. Restoring the file's snapshot would replace
// it; the placed copy has to get an id the scene hands out instead.
TEST_F(PrefabIOTest, InstantiatingIntoANonEmptySceneKeepsBothEntities) {
  const uint32_t original = CreateSprite("crate", glm::vec3(12.5f, 34.25f, 0.f));
  const uint32_t bystander = CreateSprite("bystander", glm::vec3(100.f, 200.f, 0.f));

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, original, &error)) << error;

  const uint32_t placed = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(placed, kNullEntity) << error;

  EXPECT_NE(placed, original);
  EXPECT_NE(placed, bystander);
  EXPECT_EQ(CountEntities(), 3u);

  // Both the original and the copy still carry the values the prefab stored,
  // and the bystander was not touched.
  int crates = 0;
  int bystanders = 0;

  TestScene().Registry().view<BasicComponent, TransformComponent>().each(
      [&crates, &bystanders](auto, BasicComponent& basic, TransformComponent& transform) {
        if (basic.m_name == "crate") {
          ++crates;
          EXPECT_FLOAT_EQ(transform.position.x, 12.5f);
          EXPECT_FLOAT_EQ(transform.position.y, 34.25f);
        } else if (basic.m_name == "bystander") {
          ++bystanders;
          EXPECT_FLOAT_EQ(transform.position.x, 100.f);
          EXPECT_FLOAT_EQ(transform.position.y, 200.f);
        } else {
          ADD_FAILURE() << "unexpected entity " << basic.m_name;
        }
      });

  EXPECT_EQ(crates, 2);
  EXPECT_EQ(bystanders, 1);
}

// Two placements are two entities, not one entity drawn twice: a change to one
// sprite must not reach the other, and both still share the texture by name.
TEST_F(PrefabIOTest, TwoInstantiationsAreIndependent) {
  const uint32_t source = CreateTexturedSprite("crate", glm::vec3(1.f, 2.f, 0.f), "shared.png", 0xff00ff00u);

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, source, &error)) << error;

  TestScene().Clear();
  TextureManager::Clean();

  const uint32_t first = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(first, kNullEntity) << error;
  const uint32_t second = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(second, kNullEntity) << error;

  ASSERT_NE(first, second);
  EXPECT_EQ(CountEntities(), 2u);
  EXPECT_EQ(TextureManager::Count(), 1u);

  std::shared_ptr<Sprite>& first_sprite = TestScene().Registry().get<SpriteComponent>(first).m_sprite;
  std::shared_ptr<Sprite>& second_sprite = TestScene().Registry().get<SpriteComponent>(second).m_sprite;

  ASSERT_NE(first_sprite, nullptr);
  ASSERT_NE(second_sprite, nullptr);
  EXPECT_NE(first_sprite.get(), second_sprite.get());

  first_sprite->SetColor(0xff0000ffu);

  EXPECT_EQ(first_sprite->GetColor(), 0xff0000ffu);
  EXPECT_EQ(second_sprite->GetColor(), 0xff00ff00u);

  // The texture is one manager-owned asset, so both copies still refer to it.
  EXPECT_EQ(first_sprite->GetTextureName(), "shared.png");
  EXPECT_EQ(first_sprite->GetTId(), second_sprite->GetTId());
}

// A body belongs to one entity. A placed copy gets a new, uninitialised body
// with the same inputs, so PhysicsSystem builds it like any body added in the
// editor -- and two placements do not share one b2Body.
TEST_F(PrefabIOTest, APrefabWithARigidBodyGetsAFreshBody) {
  const uint32_t crate = CreateSprite("crate", glm::vec3(640.f, 400.f, 0.f));

  Rigidbody* body = new Rigidbody(glm::vec2(640.f, 400.f), glm::vec2(40.f, 40.f));
  body->SetMaterial("0123456789abcdef");
  *body->GetBodyType() = b2_dynamicBody;
  TestScene().Registry().assign<RigidBodyComponent>(crate, body);

  std::string error;
  ASSERT_TRUE(PrefabIO::Save(TestScene(), m_file, crate, &error)) << error;

  TestScene().Clear();

  const uint32_t first = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(first, kNullEntity) << error;
  const uint32_t second = PrefabIO::Instantiate(TestScene(), m_file, &error);
  ASSERT_NE(second, kNullEntity) << error;

  std::shared_ptr<Rigidbody>& first_body = TestScene().Registry().get<RigidBodyComponent>(first).m_rigidbody;
  std::shared_ptr<Rigidbody>& second_body = TestScene().Registry().get<RigidBodyComponent>(second).m_rigidbody;

  ASSERT_NE(first_body, nullptr);
  ASSERT_NE(second_body, nullptr);
  EXPECT_NE(first_body.get(), second_body.get());

  EXPECT_FALSE(first_body->m_init);
  EXPECT_EQ(*first_body->GetBodyType(), b2_dynamicBody);
  EXPECT_EQ(first_body->GetMaterial(), "0123456789abcdef");
  EXPECT_EQ(second_body->GetMaterial(), "0123456789abcdef");
}

TEST_F(PrefabIOTest, AMissingPrefabLeavesTheSceneAlone) {
  CreateSprite("keep", glm::vec3(1.f, 2.f, 0.f));

  std::string error;
  EXPECT_EQ(PrefabIO::Instantiate(TestScene(), m_directory / "missing.xml", &error), kNullEntity);
  EXPECT_NE(error.find("not found"), std::string::npos);
  EXPECT_EQ(CountEntities(), 1u);
}

TEST_F(PrefabIOTest, ANewerPrefabIsRefusedWithoutTouchingTheScene) {
  CreateSprite("keep", glm::vec3(1.f, 2.f, 0.f));

  // One past what this build writes. The body is deliberately incomplete: the
  // version check has to reject the file before any of it is parsed.
  const std::string newer = std::to_string(PrefabIO::kFormatVersion + 1);
  WriteFile("<cereal><formatVersion>" + newer + "</formatVersion><entity><value0>0</value0></entity></cereal>");

  std::string error;
  EXPECT_EQ(PrefabIO::Instantiate(TestScene(), m_file, &error), kNullEntity);
  EXPECT_NE(error.find(newer), std::string::npos);
  EXPECT_NE(error.find("newer"), std::string::npos);
  EXPECT_EQ(CountEntities(), 1u);
}

TEST_F(PrefabIOTest, AVersionZeroPrefabIsNotAPrefabFile) {
  WriteFile("<cereal><formatVersion>0</formatVersion><entity><value0>0</value0></entity></cereal>");

  std::string error;
  EXPECT_EQ(PrefabIO::Instantiate(TestScene(), m_file, &error), kNullEntity);
  EXPECT_NE(error.find("not a prefab file"), std::string::npos);
}

TEST_F(PrefabIOTest, APrefabWithNoEntityIsRefused) {
  WriteFile(
      "<cereal><formatVersion>1</formatVersion><entity><value0>0</value0><value1>0</value1><value2>0</value2>"
      "<value3>0</value3><value4>0</value4><value5>0</value5></entity></cereal>");

  std::string error;
  EXPECT_EQ(PrefabIO::Instantiate(TestScene(), m_file, &error), kNullEntity);
  EXPECT_NE(error.find("no entity"), std::string::npos);
}

TEST_F(PrefabIOTest, SavingAnEntityThatIsNotThereFails) {
  CreateSprite("keep", glm::vec3(1.f, 2.f, 0.f));

  const uint32_t destroyed = TestScene().CreateEntity().Handle();
  TestScene().Registry().destroy(destroyed);

  std::string error;
  EXPECT_FALSE(PrefabIO::Save(TestScene(), m_file, destroyed, &error));
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(std::filesystem::exists(m_file));
  EXPECT_FALSE(std::filesystem::exists(m_scratch));
}

TEST_F(PrefabIOTest, AnUnwritablePathReportsFailureWithoutLeavingAScratchFile) {
  const uint32_t crate = CreateSprite("crate", glm::vec3(1.f, 2.f, 0.f));
  const std::filesystem::path missing = m_directory / "missing" / "crate.xml";
  const std::filesystem::path missing_scratch = std::filesystem::path{missing}.concat(".tmp");

  std::string error;
  EXPECT_FALSE(PrefabIO::Save(TestScene(), missing, crate, &error));
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(std::filesystem::exists(missing_scratch));
}

// The checked-in version 1 file: one crate with every component a prefab can
// carry. It is the file a later format change has to keep reading.
TEST_F(PrefabIOTest, APrefabV1FixtureLoads) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "prefab_v1.xml";

  std::string error;
  const uint32_t crate = PrefabIO::Instantiate(TestScene(), fixture, &error);
  ASSERT_NE(crate, kNullEntity) << error;
  ASSERT_EQ(CountEntities(), 1u);

  ASSERT_TRUE(TestScene().Registry().has<BasicComponent>(crate));
  ASSERT_TRUE(TestScene().Registry().has<TransformComponent>(crate));
  ASSERT_TRUE(TestScene().Registry().has<SpriteComponent>(crate));
  ASSERT_TRUE(TestScene().Registry().has<AudioComponent>(crate));
  ASSERT_TRUE(TestScene().Registry().has<RigidBodyComponent>(crate));
  EXPECT_FALSE(TestScene().Registry().has<SpriteAppearanceComponent>(crate));

  const BasicComponent& basic = TestScene().Registry().get<BasicComponent>(crate);
  EXPECT_EQ(basic.m_name, "crate");

  const TransformComponent& transform = TestScene().Registry().get<TransformComponent>(crate);
  EXPECT_FLOAT_EQ(transform.position.x, 640.f);
  EXPECT_FLOAT_EQ(transform.position.y, 400.f);

  const SpriteComponent& sprite = TestScene().Registry().get<SpriteComponent>(crate);
  ASSERT_NE(sprite.m_sprite, nullptr);
  EXPECT_FLOAT_EQ(sprite.m_sprite->GetSize().x, 40.f);
  EXPECT_EQ(sprite.m_sprite->GetColor(), 0xffff0000u);
  EXPECT_EQ(sprite.m_sprite->GetTextureName(), "crate.png");

  const AudioComponent& audio = TestScene().Registry().get<AudioComponent>(crate);
  ASSERT_NE(audio.m_sound, nullptr);
  EXPECT_EQ(audio.m_sound->GetName(), "effect.wav");
  EXPECT_EQ(audio.volume, 64);
  EXPECT_TRUE(audio.loop);
  EXPECT_FALSE(audio.autoplay);

  const RigidBodyComponent& rigidbody = TestScene().Registry().get<RigidBodyComponent>(crate);
  ASSERT_NE(rigidbody.m_rigidbody, nullptr);
  EXPECT_FALSE(rigidbody.m_rigidbody->m_init);
  EXPECT_EQ(*rigidbody.m_rigidbody->GetBodyType(), b2_dynamicBody);

  // Version 1 stored density and friction on the body. Those nodes are ignored
  // now, so the body loads with no material and the defaults.
  EXPECT_TRUE(rigidbody.m_rigidbody->GetMaterial().empty());
  EXPECT_FLOAT_EQ(rigidbody.m_rigidbody->GetMaterialValues().density, PhysicsMaterial::kDefaultDensity);
  EXPECT_FLOAT_EQ(rigidbody.m_rigidbody->GetMaterialValues().friction, PhysicsMaterial::kDefaultFriction);
}
// The checked-in version 2 file: the first prefab format with the appearance
// group. It is what a later format change has to keep reading.
TEST_F(PrefabIOTest, APrefabV2FixtureLoads) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "prefab_v2.xml";

  std::string error;
  const uint32_t crate = PrefabIO::Instantiate(TestScene(), fixture, &error);
  ASSERT_NE(crate, kNullEntity) << error;
  ASSERT_EQ(CountEntities(), 1u);

  ASSERT_TRUE(TestScene().Registry().has<SpriteAppearanceComponent>(crate));
  const SpriteAppearanceComponent& appearance = TestScene().Registry().get<SpriteAppearanceComponent>(crate);
  EXPECT_TRUE(appearance.flip_x);
  EXPECT_FALSE(appearance.flip_y);
  EXPECT_EQ(appearance.material, "mat_warm");

  // Version 2 predates the enabled group: a prefab that has none is enabled.
  EXPECT_FALSE(TestScene().Registry().has<EntityEnabledComponent>(crate));
  EXPECT_TRUE(TestScene().IsEntityEnabled(crate));

  const BasicComponent& basic = TestScene().Registry().get<BasicComponent>(crate);
  EXPECT_EQ(basic.m_name, "crate");
}

// The checked-in version 3 file: the last prefab format before the disabled
// component mask. It has no mask group, so every component loads enabled.
TEST_F(PrefabIOTest, APrefabV3FixtureLoadsWithEveryComponentEnabled) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "prefab_v3.xml";

  std::string error;
  const uint32_t crate = PrefabIO::Instantiate(TestScene(), fixture, &error);
  ASSERT_NE(crate, kNullEntity) << error;
  ASSERT_EQ(CountEntities(), 1u);

  EXPECT_FALSE(TestScene().Registry().has<DisabledComponentsComponent>(crate));
  EXPECT_TRUE(TestScene().IsComponentEnabled<SpriteComponent>(crate));
  EXPECT_TRUE(TestScene().IsComponentEnabled<AudioComponent>(crate));
  EXPECT_TRUE(TestScene().IsComponentEnabled<RigidBodyComponent>(crate));
}

}  // namespace
