#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <audio/sound_manager.h>
#include <core/asset_registry.h>
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

    AssetRegistry::Clean();
    SceneIO::Clear();
  }

  void TearDown() override {
    SceneIO::Clear();
    TextureManager::Clean();
    SoundManager::Clean();
    AssetRegistry::Clean();
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

  // An entity with a sound, the way a loaded scene has one.
  static uint32_t CreateAudioEntity(const char* name, const glm::vec3& position, const std::string& sound, int volume,
                                    bool loop, bool autoplay) {
    const uint32_t entity = CreateSprite(name, position);

    AudioComponent audio{SoundManager::GetOrLoad(sound)};
    audio.volume = volume;
    audio.loop = loop;
    audio.autoplay = autoplay;
    ECSManager::AddComponent<AudioComponent>(entity, audio);
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
  EXPECT_NE(contents.find("<formatVersion>10</formatVersion>"), std::string::npos);
  EXPECT_NE(contents.find("<camera>"), std::string::npos);
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

TEST_F(SceneIOTest, DisabledStateRoundTrips) {
  const uint32_t entity = CreateSprite("hidden", glm::vec3(1.f, 2.f, 0.f));
  ECSManager::SetEntityEnabled(entity, false);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();
  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  ASSERT_EQ(CountEntities(), 1u);

  ECSManager::GetView<BasicComponent>().each([](auto entity, BasicComponent& basic) {
    EXPECT_EQ(basic.m_name, "hidden");
    EXPECT_TRUE(ECSManager::HasComponent<EntityEnabledComponent>(entity));
    EXPECT_FALSE(ECSManager::GetComponent<EntityEnabledComponent>(entity).m_enabled);
    EXPECT_FALSE(ECSManager::IsEntityEnabled(entity));
  });
}

TEST_F(SceneIOTest, DisabledComponentStateRoundTrips) {
  const uint32_t entity = CreateAudioEntity("hidden", glm::vec3(1.f, 2.f, 0.f), "effect.wav", 64, false, false);
  ECSManager::AddComponent<RigidBodyComponent>(entity, new Rigidbody(glm::vec2(1.f, 2.f), glm::vec2(8.f, 8.f)));

  ECSManager::SetComponentEnabled<SpriteComponent>(entity, false);
  ECSManager::SetComponentEnabled<AudioComponent>(entity, false);
  ECSManager::SetComponentEnabled<RigidBodyComponent>(entity, false);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();
  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  ASSERT_EQ(CountEntities(), 1u);

  ECSManager::GetView<BasicComponent>().each([](auto entity, BasicComponent& basic) {
    EXPECT_EQ(basic.m_name, "hidden");
    ASSERT_TRUE(ECSManager::HasComponent<DisabledComponentsComponent>(entity));
    EXPECT_FALSE(ECSManager::IsComponentEnabled<SpriteComponent>(entity));
    EXPECT_FALSE(ECSManager::IsComponentEnabled<AudioComponent>(entity));
    EXPECT_FALSE(ECSManager::IsComponentEnabled<RigidBodyComponent>(entity));
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

TEST_F(SceneIOTest, AnAudioComponentRoundTrips) {
  CreateAudioEntity("speaker", glm::vec3(1.f, 2.f, 0.f), "effect.wav", 64, true, false);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();
  TextureManager::Clean();
  SoundManager::Clean();

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  EXPECT_EQ(CountEntities(), 1u);
  EXPECT_EQ(SoundManager::Count(), 1u);

  int speakers = 0;

  ECSManager::GetView<AudioComponent>().each([&speakers](auto, AudioComponent& audio) {
    ++speakers;
    ASSERT_NE(audio.m_sound, nullptr);
    EXPECT_EQ(audio.m_sound->GetName(), "effect.wav");
    EXPECT_EQ(audio.volume, 64);
    EXPECT_FALSE(audio.mute);
    EXPECT_TRUE(audio.loop);
    EXPECT_FALSE(audio.autoplay);

    // Loading is silent: the entity asks for no autoplay, and nothing starts a
    // sound before AudioSystem's first update anyway.
    EXPECT_FALSE(audio.m_autoplay_started);
    EXPECT_FALSE(audio.m_sound->IsPlaying());
  });

  EXPECT_EQ(speakers, 1);
}

TEST_F(SceneIOTest, EntitiesThatShareASoundShareOneSound) {
  CreateAudioEntity("left", glm::vec3(0.f), "shared.wav", 64, false, false);
  CreateAudioEntity("right", glm::vec3(16.f, 0.f, 0.f), "shared.wav", 32, false, false);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();
  SoundManager::Clean();

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  // One decoded sound for two entities, and the volume each entity authored is
  // still its own -- the reason the settings live on the component.
  EXPECT_EQ(SoundManager::Count(), 1u);

  int speakers = 0;

  ECSManager::GetView<BasicComponent, AudioComponent>().each(
      [&speakers](auto, BasicComponent& basic, AudioComponent& audio) {
        ++speakers;
        ASSERT_NE(audio.m_sound, nullptr);
        EXPECT_EQ(audio.m_sound->GetName(), "shared.wav");

        if (basic.m_name == "left") {
          EXPECT_EQ(audio.volume, 64);
        } else if (basic.m_name == "right") {
          EXPECT_EQ(audio.volume, 32);
        } else {
          ADD_FAILURE() << "unexpected entity " << basic.m_name;
        }
      });

  EXPECT_EQ(speakers, 2);
}

TEST_F(SceneIOTest, AMissingSoundKeepsItsName) {
  CreateAudioEntity("speaker", glm::vec3(1.f, 2.f, 0.f), "this_file_does_not_exist.wav", 128, false, true);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();
  SoundManager::Clean();

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  ECSManager::GetView<AudioComponent>().each([](auto, AudioComponent& audio) {
    ASSERT_NE(audio.m_sound, nullptr);
    EXPECT_EQ(audio.m_sound->GetName(), "this_file_does_not_exist.wav");
    EXPECT_TRUE(audio.autoplay);
    EXPECT_FALSE(audio.m_sound->IsPlaying());
  });
}

TEST_F(SceneIOTest, ARigidBodyComponentRoundTrips) {
  const uint32_t floor = CreateSprite("floor", glm::vec3(640.f, 80.f, 0.f));
  Rigidbody* floor_body = new Rigidbody(glm::vec2(640.f, 80.f), glm::vec2(640.f, 16.f));
  ECSManager::AddComponent<RigidBodyComponent>(floor, floor_body);

  const uint32_t crate = CreateSprite("crate", glm::vec3(640.f, 400.f, 0.f));
  Rigidbody* crate_body = new Rigidbody(glm::vec2(640.f, 400.f), glm::vec2(40.f, 40.f));
  crate_body->SetMaterial("0123456789abcdef");
  *crate_body->GetBodyType() = b2_dynamicBody;
  ECSManager::AddComponent<RigidBodyComponent>(crate, crate_body);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();

  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  EXPECT_EQ(CountEntities(), 2u);

  int bodies = 0;

  ECSManager::GetView<BasicComponent, RigidBodyComponent>().each(
      [&bodies](auto, BasicComponent& basic, RigidBodyComponent& rigid) {
        ++bodies;
        ASSERT_NE(rigid.m_rigidbody, nullptr);

        // Nothing was simulated while the scene was a file.
        EXPECT_FALSE(rigid.m_rigidbody->m_init);

        if (basic.m_name == "floor") {
          EXPECT_EQ(*rigid.m_rigidbody->GetBodyType(), b2_staticBody);
          EXPECT_TRUE(rigid.m_rigidbody->GetMaterial().empty());
        } else if (basic.m_name == "crate") {
          EXPECT_EQ(*rigid.m_rigidbody->GetBodyType(), b2_dynamicBody);
          EXPECT_EQ(rigid.m_rigidbody->GetMaterial(), "0123456789abcdef");
        } else {
          ADD_FAILURE() << "unexpected entity " << basic.m_name;
        }
      });

  EXPECT_EQ(bodies, 2);
}

// The camera is scene-root data: each scene stores its own, and loading one
// replaces the other.
TEST_F(SceneIOTest, ASpriteAppearanceRoundTrips) {
  const uint32_t entity = CreateSprite("mirrored", glm::vec3(4.f, 5.f, 0.f));

  SpriteAppearanceComponent appearance;
  appearance.flip_x = true;
  appearance.flip_y = true;
  appearance.material = "mat_warm";
  ECSManager::AddComponent<SpriteAppearanceComponent>(entity, appearance);

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  SceneIO::Clear();
  ASSERT_EQ(CountEntities(), 0u);
  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  ASSERT_TRUE(ECSManager::HasComponent<SpriteAppearanceComponent>(entity));
  const SpriteAppearanceComponent& loaded = ECSManager::GetComponent<SpriteAppearanceComponent>(entity);
  EXPECT_TRUE(loaded.flip_x);
  EXPECT_TRUE(loaded.flip_y);
  EXPECT_EQ(loaded.material, "mat_warm");
}

TEST_F(SceneIOTest, ASceneCameraRoundTrips) {
  const std::filesystem::path second_file = m_directory / "second.xml";

  SceneIO::GetCamera().position = glm::vec2(320.f, 180.f);
  SceneIO::GetCamera().zoom = 2.f;
  SceneIO::GetCamera().background = glm::vec3(0.2f, 0.4f, 0.8f);

  CreateSprite("hero", glm::vec3(320.f, 180.f, 0.f));

  std::string error;
  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  // A different scene, with a different camera.
  SceneIO::GetCamera().position = glm::vec2(1000.f, 100.f);
  SceneIO::GetCamera().zoom = 0.5f;
  SceneIO::GetCamera().background = glm::vec3(0.f, 0.f, 0.f);

  ASSERT_TRUE(SceneIO::Save(second_file, &error)) << error;

  SceneIO::Clear();
  ASSERT_TRUE(SceneIO::Load(m_file, &error)) << error;

  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.x, 320.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.y, 180.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().zoom, 2.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.r, 0.2f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.g, 0.4f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.b, 0.8f);

  ASSERT_TRUE(SceneIO::Load(second_file, &error)) << error;

  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.x, 1000.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.y, 100.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().zoom, 0.5f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.r, 0.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.g, 0.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.b, 0.f);
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

  // Version 3 appended the audio group; a version 2 file simply has none, and
  // reading it must not ask for a group that is not in the file.
  EXPECT_EQ(SoundManager::Count(), 0u);

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

          // No asset registry is open here, so the name is a bare path: the
          // fallback that keeps a scene written before the registry loading.
          EXPECT_TRUE(sprite.m_sprite->GetTextureAssetId().empty());
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

// The version 3 fixture: the same floor and crate as version 2, plus a speaker
// with no sprite whose sound and playback settings have to come back.
TEST_F(SceneIOTest, AV3SceneLoadsFromTheFixture) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v3.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  ASSERT_EQ(CountEntities(), 3u);
  EXPECT_EQ(TextureManager::Count(), 1u);
  EXPECT_EQ(SoundManager::Count(), 1u);

  int speakers = 0;

  ECSManager::GetView<BasicComponent, AudioComponent>().each(
      [&speakers](auto, BasicComponent& basic, AudioComponent& audio) {
        EXPECT_EQ(basic.m_name, "speaker");
        ++speakers;

        ASSERT_NE(audio.m_sound, nullptr);
        EXPECT_EQ(audio.m_sound->GetName(), "effect.wav");
        EXPECT_EQ(audio.volume, 64);
        EXPECT_TRUE(audio.loop);
        EXPECT_FALSE(audio.mute);
        EXPECT_TRUE(audio.autoplay);
      });

  EXPECT_EQ(speakers, 1);

  // An entity can carry audio without a sprite, and a sprite without audio.
  EXPECT_EQ(ECSManager::GetView<SpriteComponent>().size(), 2u);

  // Version 4 appended rigid bodies; a version 3 file simply has none.
  EXPECT_EQ(ECSManager::GetView<RigidBodyComponent>().size(), 0u);
}

// The version 4 fixture carries every group the writer knows: the version 3
// floor, crate and speaker, with a static body on the floor and a dynamic one
// on the crate.
TEST_F(SceneIOTest, AV4SceneLoadsFromTheFixture) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v4.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  ASSERT_EQ(CountEntities(), 3u);
  EXPECT_EQ(TextureManager::Count(), 1u);
  EXPECT_EQ(SoundManager::Count(), 1u);
  EXPECT_EQ(ECSManager::GetView<RigidBodyComponent>().size(), 2u);

  int bodies = 0;

  ECSManager::GetView<BasicComponent, RigidBodyComponent>().each(
      [&bodies](auto, BasicComponent& basic, RigidBodyComponent& rigid) {
        ++bodies;
        ASSERT_NE(rigid.m_rigidbody, nullptr);
        EXPECT_FALSE(rigid.m_rigidbody->m_init);

        // Version 4 stores density and friction on the body. The reader ignores
        // those removed nodes, so the body comes back with no material and the
        // defaults rather than the old values.
        EXPECT_TRUE(rigid.m_rigidbody->GetMaterial().empty());
        EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().density, PhysicsMaterial::kDefaultDensity);
        EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().friction, PhysicsMaterial::kDefaultFriction);

        if (basic.m_name == "floor") {
          EXPECT_EQ(*rigid.m_rigidbody->GetBodyType(), b2_staticBody);
        } else if (basic.m_name == "crate") {
          EXPECT_EQ(*rigid.m_rigidbody->GetBodyType(), b2_dynamicBody);
        } else {
          ADD_FAILURE() << "unexpected body " << basic.m_name;
        }
      });

  EXPECT_EQ(bodies, 2);
}

// A version 4 scene has no camera node, so it keeps the defaults -- which are
// exactly the framing it was drawn with.
TEST_F(SceneIOTest, AVersion4SceneKeepsTheDefaultCamera) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v4.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.x, 640.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.y, 360.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().zoom, 1.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.r, 1.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.g, 0.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.b, 1.f);
}

// The version 5 fixture is the first file with a camera node, and the shape a
// later change has to keep reading.
TEST_F(SceneIOTest, AV5SceneLoadsFromTheFixture) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v5.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  ASSERT_EQ(CountEntities(), 1u);

  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.x, 320.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.y, 180.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().zoom, 2.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.r, 0.2f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.g, 0.4f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.b, 0.8f);
}

// A version 6 scene stores an asset *id*, not a name: the registry the fixture
// pairs with resolves it back to a name for the inspector. The scene is the
// sample's physics scene, which is the rule worked through.
TEST_F(SceneIOTest, AV6SceneLoadsFromTheFixture) {
  const std::filesystem::path registry = std::filesystem::path{RSE_FIXTURES_DIR} / "assets_v1.xml";
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v6.xml";

  std::string error;
  ASSERT_TRUE(AssetRegistry::Load(registry, &error)) << error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  ASSERT_EQ(CountEntities(), 2u);

  // Version 6 has no appearance group, so every entity opens with the defaults
  // and no component is invented for it.
  uint32_t appearances = 0;
  ECSManager::GetView<SpriteAppearanceComponent>().each(
      [&appearances](auto, SpriteAppearanceComponent&) { ++appearances; });
  EXPECT_EQ(appearances, 0u);

  int crates = 0;

  ECSManager::GetView<BasicComponent, SpriteComponent>().each(
      [&crates](auto, BasicComponent& basic, SpriteComponent& sprite) {
        if (basic.m_name != "crate") {
          return;
        }

        ++crates;
        ASSERT_NE(sprite.m_sprite, nullptr);

        // The id is what the file stores; the name is what the registry says it
        // is, which is what the inspector shows.
        EXPECT_EQ(sprite.m_sprite->GetTextureAssetId(), "0123456789abcdef");
        EXPECT_EQ(sprite.m_sprite->GetTextureName(), "tex3.png");
      });

  EXPECT_EQ(crates, 1);
}

// The version 7 fixture is the first file with the sprite appearance group. It
// predates the enabled group, so every entity in it has to open enabled.
TEST_F(SceneIOTest, AV7SceneLoadsWithEveryEntityEnabled) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v7.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  ASSERT_EQ(CountEntities(), 2u);
  EXPECT_EQ(ECSManager::GetView<EntityEnabledComponent>().size(), 0u);

  ECSManager::GetView<BasicComponent>().each(
      [](auto entity, BasicComponent&) { EXPECT_TRUE(ECSManager::IsEntityEnabled(entity)); });
}

TEST_F(SceneIOTest, AV8SceneLoadsWithEveryComponentEnabled) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v8.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  ASSERT_EQ(CountEntities(), 2u);
  EXPECT_EQ(ECSManager::GetView<DisabledComponentsComponent>().size(), 0u);

  ECSManager::GetView<BasicComponent>().each([](auto entity, BasicComponent&) {
    EXPECT_TRUE(ECSManager::IsComponentEnabled<SpriteComponent>(entity));
    EXPECT_TRUE(ECSManager::IsComponentEnabled<AudioComponent>(entity));
    EXPECT_TRUE(ECSManager::IsComponentEnabled<RigidBodyComponent>(entity));
  });
}

// A version 5 scene names its texture, and the registry maps that name onto the
// asset's id. Saving then writes the id, which is the migration: the file moves
// from names to ids without the author doing anything.
TEST_F(SceneIOTest, AV5SceneMigratesNamesToIds) {
  AssetRegistry::Init(m_directory);
  const std::string id = AssetRegistry::Add("shared.png", "texture", "assets/shared.png");

  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "scene_v2.xml";

  std::string error;
  ASSERT_TRUE(SceneIO::Load(fixture, &error)) << error;

  int textured = 0;

  ECSManager::GetView<SpriteComponent>().each([&textured, &id](auto, SpriteComponent& sprite) {
    if (sprite.m_sprite == nullptr || sprite.m_sprite->GetTextureName() != "shared.png") {
      return;
    }

    ++textured;
    EXPECT_EQ(sprite.m_sprite->GetTextureAssetId(), id);
  });

  EXPECT_EQ(textured, 1);

  ASSERT_TRUE(SceneIO::Save(m_file, &error)) << error;

  const std::string contents = ReadFile(m_file);
  EXPECT_EQ(contents.find("<Texture>shared.png</Texture>"), std::string::npos);
  EXPECT_NE(contents.find("<Texture>" + id + "</Texture>"), std::string::npos);
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

TEST_F(SceneIOTest, AStringSnapshotRoundTrips) {
  CreateSprite("snapshot", glm::vec3(4.5f, 6.25f, 0.f));

  SceneIO::GetCamera().position = glm::vec2(10.f, 20.f);
  SceneIO::GetCamera().zoom = 2.0f;
  SceneIO::GetCamera().background = glm::vec3(0.25f, 0.5f, 0.75f);

  std::string snapshot;
  std::string error;

  ASSERT_TRUE(SceneIO::SaveToString(&snapshot, &error)) << error;
  EXPECT_FALSE(snapshot.empty());
  EXPECT_FALSE(std::filesystem::exists(m_file));

  // Run the live scene forward the way a play session would.
  ECSManager::GetView<TransformComponent>().each(
      [](auto, TransformComponent& transform) { transform.position = glm::vec3(99.f, 99.f, 0.f); });
  CreateSprite("made-during-play", glm::vec3(0.f));
  SceneIO::GetCamera().zoom = 0.5f;

  ASSERT_TRUE(SceneIO::LoadFromString(snapshot, &error)) << error;

  ASSERT_EQ(CountEntities(), 1u);

  ECSManager::GetView<BasicComponent, TransformComponent>().each(
      [](auto, BasicComponent& basic, TransformComponent& transform) {
        EXPECT_EQ(basic.m_name, "snapshot");
        EXPECT_FLOAT_EQ(transform.position.x, 4.5f);
        EXPECT_FLOAT_EQ(transform.position.y, 6.25f);
      });

  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.x, 10.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().position.y, 20.f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().zoom, 2.0f);
  EXPECT_FLOAT_EQ(SceneIO::GetCamera().background.z, 0.75f);
}

TEST_F(SceneIOTest, AnInvalidStringSnapshotLeavesTheSceneAlone) {
  CreateSprite("player", glm::vec3(1.f, 2.f, 0.f));

  std::string error;

  EXPECT_FALSE(SceneIO::LoadFromString("<cereal><formatVersion>0</formatVersion></cereal>", &error));
  EXPECT_NE(error.find("not a scene file"), std::string::npos);
  EXPECT_NE(error.find("<snapshot>"), std::string::npos);

  EXPECT_EQ(CountEntities(), 1u);

  ECSManager::GetView<BasicComponent>().each([](auto, BasicComponent& basic) { EXPECT_EQ(basic.m_name, "player"); });
}

TEST_F(SceneIOTest, ANewerStringSnapshotIsRefused) {
  std::string error;

  EXPECT_FALSE(SceneIO::LoadFromString("<cereal><formatVersion>99</formatVersion></cereal>", &error));
  EXPECT_NE(error.find("newer than this build supports"), std::string::npos);
  EXPECT_EQ(CountEntities(), 0u);
}
}  // namespace
