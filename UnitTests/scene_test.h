#pragma once

#include <memory>

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include <components/sprite.h>
#include <ecs/components/audio.h>
#include <ecs/components/basic.h>
#include <ecs/components/disabled_components.h>
#include <ecs/components/entity_enabled.h>
#include <ecs/components/rigid_body.h>
#include <ecs/components/sprite.h>
#include <ecs/components/transform.h>
#include <ecs/components/uuid.h>
#include <ecs/entity.h>
#include <ecs/scene.h>
#include <physics/rigidbody.h>

/*
A scene owns its entities, so the only thing that has to be true is that its
handles keep naming what they were made for. The failure mode is silent: an id
is an index into a registry and is recycled, so a handle that survives a destroy
without noticing would reach whatever entity took the slot next.

The enabled flags are scene-level too: a missing component means enabled, and
the scene is what writes and reads the mask the systems consult.
*/
namespace {

using namespace rse;

// The copy's entt id is its own registry's, so a test finds the copy the way
// anything outside a registry has to: by the uuid it kept.
uint32_t FindByUuid(const Scene& scene, const Uuid& uuid) {
  uint32_t found = static_cast<uint32_t>(entt::null);

  scene.Registry().each([&scene, &uuid, &found](entt::DefaultRegistry::entity_type entity) {
    if (scene.Registry().has<UuidComponent>(entity) && scene.Registry().get<UuidComponent>(entity).m_uuid == uuid) {
      found = entity;
    }
  });

  return found;
}

TEST(SceneTest, ANewSceneIsEmpty) {
  const Scene scene;

  EXPECT_EQ(scene.EntityCount(), 0u);
}

TEST(SceneTest, CreateEntityMakesALiveEntity) {
  Scene scene;
  const Entity entity = scene.CreateEntity();

  EXPECT_TRUE(entity.IsValid());
  EXPECT_TRUE(scene.Contains(entity));
  EXPECT_EQ(entity.GetScene(), &scene);
  EXPECT_EQ(scene.EntityCount(), 1u);
}

TEST(SceneTest, DestroyingOneEntityLeavesTheOthersAlive) {
  Scene scene;
  const Entity first = scene.CreateEntity();
  const Entity second = scene.CreateEntity();

  scene.DestroyEntity(first);

  EXPECT_FALSE(first.IsValid());
  EXPECT_FALSE(scene.Contains(first));
  EXPECT_TRUE(second.IsValid());
  EXPECT_TRUE(scene.Contains(second));
  EXPECT_EQ(scene.EntityCount(), 1u);
}

TEST(SceneTest, DestroyingTwiceIsHarmless) {
  Scene scene;
  const Entity entity = scene.CreateEntity();

  scene.DestroyEntity(entity);
  scene.DestroyEntity(entity);

  EXPECT_FALSE(entity.IsValid());
  EXPECT_EQ(scene.EntityCount(), 0u);
}

TEST(SceneTest, AHandleFromAnotherSceneIsNotContained) {
  Scene first_scene;
  Scene second_scene;
  const Entity entity = first_scene.CreateEntity();

  // The same numeric id would be a different entity in the other scene, so the
  // other scene must not claim it -- and must not destroy it.
  EXPECT_FALSE(second_scene.Contains(entity));

  second_scene.DestroyEntity(entity);

  EXPECT_TRUE(entity.IsValid());
  EXPECT_EQ(first_scene.EntityCount(), 1u);
  EXPECT_EQ(second_scene.EntityCount(), 0u);
}

TEST(SceneTest, ADefaultHandleNamesNothing) {
  Scene scene;
  Entity entity;

  EXPECT_FALSE(entity.IsValid());
  EXPECT_FALSE(scene.Contains(entity));
  EXPECT_EQ(entity.GetScene(), nullptr);

  entity.Destroy();
  EXPECT_EQ(scene.EntityCount(), 0u);
}

// A destroyed entity's slot is recycled. The old handle must not come back to
// life with it: the recycled id carries a new version, so the two handles
// differ even though they name the same slot.
TEST(SceneTest, ARecycledSlotDoesNotReviveAnOldHandle) {
  Scene scene;
  const Entity first = scene.CreateEntity();
  const uint32_t first_handle = first.Handle();

  scene.DestroyEntity(first);
  const Entity second = scene.CreateEntity();

  EXPECT_TRUE(second.IsValid());
  EXPECT_NE(second.Handle(), first_handle);
  EXPECT_FALSE(first.IsValid());
  EXPECT_TRUE(scene.Contains(second));
}

// The physics world belongs to the scene, not to a system: two scenes are two
// simulations, and a body built in one must not be reachable from the other.
TEST(SceneTest, EachSceneHasItsOwnPhysicsWorld) {
  Scene first;
  Scene second;

  EXPECT_NE(first.PhysicsWorld(), nullptr);
  EXPECT_NE(first.PhysicsWorld(), second.PhysicsWorld());
}

// Clearing is what a scene load does before it fills the scene again. The world
// has to survive it empty, and every body has to be gone from it: a body is
// destroyed by its Rigidbody, which the registry destroys with the entity.
TEST(SceneTest, ClearingASceneDestroysItsEntitiesAndBodies) {
  Scene scene;
  Entity entity = scene.CreateEntity();
  entity.AddComponent<TransformComponent>(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));

  Rigidbody* body = new Rigidbody(glm::vec2(0.f), glm::vec2(8.f, 8.f));
  body->Init(scene.PhysicsWorld());
  entity.AddComponent<RigidBodyComponent>(body);

  ASSERT_EQ(scene.PhysicsWorld()->GetBodyCount(), 1);

  scene.Clear();

  EXPECT_EQ(scene.EntityCount(), 0u);
  EXPECT_FALSE(entity.IsValid());
  EXPECT_EQ(scene.PhysicsWorld()->GetBodyCount(), 0);
  EXPECT_NE(scene.PhysicsWorld(), nullptr);
}

// Destroying a scene destroys its entities -- and through them their bodies --
// before the world itself. If the order were reversed, Rigidbody's destructor
// would call DestroyBody on a world that no longer exists.
TEST(SceneTest, DestroyingASceneDestroysItsBodiesFirst) {
  {
    Scene scene;
    Entity entity = scene.CreateEntity();
    entity.AddComponent<TransformComponent>(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));

    Rigidbody* body = new Rigidbody(glm::vec2(0.f), glm::vec2(8.f, 8.f));
    body->Init(scene.PhysicsWorld());
    entity.AddComponent<RigidBodyComponent>(body);

    ASSERT_EQ(scene.PhysicsWorld()->GetBodyCount(), 1);
  }

  SUCCEED();
}

// A play session is a film of the open scene: the copy has to start from the
// same framing and the same entities, or what plays is not what was authored.
TEST(SceneTest, ACopiedSceneStartsWithTheSourceCamera) {
  Scene source;
  source.GetCamera().position = glm::vec2(100.f, 200.f);
  source.GetCamera().zoom = 2.f;
  source.GetCamera().background = glm::vec3(0.1f, 0.2f, 0.3f);

  Scene copy;
  copy.CopyFrom(source);

  EXPECT_FLOAT_EQ(copy.GetCamera().position.x, 100.f);
  EXPECT_FLOAT_EQ(copy.GetCamera().position.y, 200.f);
  EXPECT_FLOAT_EQ(copy.GetCamera().zoom, 2.f);
  EXPECT_FLOAT_EQ(copy.GetCamera().background.z, 0.3f);

  // The camera is the copy's: panning during play must not move the authored
  // scene's framing.
  copy.GetCamera().zoom = 4.f;
  EXPECT_FLOAT_EQ(source.GetCamera().zoom, 2.f);
}

// An entity id names a slot in one registry, so a copy cannot share one. The
// uuid is the identity both worlds agree on, and the components are values the
// copy owns.
TEST(SceneTest, ACopyOwnsItsComponents) {
  Scene source;
  Entity original = source.CreateEntity();
  original.AddComponent<TransformComponent>(glm::vec3(1.f, 2.f, 0.f), glm::vec3(0.f), glm::vec3(1.f));
  original.AddComponent<SpriteComponent>(new Sprite(glm::vec3(1.f, 2.f, 0.f), glm::vec2(8.f, 8.f), 0xffffffffu));

  Scene copy;
  copy.CopyFrom(source);

  ASSERT_EQ(copy.EntityCount(), 1u);

  const uint32_t handle = FindByUuid(copy, original.GetUuid());
  ASSERT_NE(handle, static_cast<uint32_t>(entt::null));

  // Same identity, different world: the handle in the copy must be reached
  // through the copy's registry.
  EXPECT_EQ(copy.Registry().get<UuidComponent>(handle).m_uuid, original.GetUuid());
  EXPECT_FLOAT_EQ(copy.Registry().get<TransformComponent>(handle).position.x, 1.f);

  // Writing through the copy is not writing through the source -- neither the
  // transform nor the sprite the entity owns.
  copy.Registry().get<TransformComponent>(handle).position = glm::vec3(9.f, 9.f, 0.f);
  EXPECT_FLOAT_EQ(original.GetComponent<TransformComponent>().position.x, 1.f);

  EXPECT_NE(copy.Registry().get<SpriteComponent>(handle).m_sprite.get(),
            original.GetComponent<SpriteComponent>().m_sprite.get());
  copy.Registry().get<SpriteComponent>(handle).m_sprite->SetColor(0x00000000u);
  EXPECT_EQ(original.GetComponent<SpriteComponent>().m_sprite->GetColor(), 0xffffffffu);
}

// A Rigidbody owns a b2Body, and a body belongs to exactly one world. The copy
// gets a clone with the same inputs, uninitialised, so PhysicsSystem builds it
// in the copy's world rather than in the source's.
TEST(SceneTest, ACopiedSceneGivesTheCopyItsOwnBodies) {
  Scene source;
  Entity original = source.CreateEntity();
  original.AddComponent<TransformComponent>(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));

  Rigidbody* body = new Rigidbody(glm::vec2(0.f), glm::vec2(8.f, 8.f));
  body->Init(source.PhysicsWorld());
  original.AddComponent<RigidBodyComponent>(body);

  ASSERT_EQ(source.PhysicsWorld()->GetBodyCount(), 1);

  Scene copy;
  copy.CopyFrom(source);

  const uint32_t handle = FindByUuid(copy, original.GetUuid());
  ASSERT_NE(handle, static_cast<uint32_t>(entt::null));

  const std::shared_ptr<Rigidbody>& copied_body = copy.Registry().get<RigidBodyComponent>(handle).m_rigidbody;
  ASSERT_TRUE(copied_body != nullptr);
  EXPECT_NE(copied_body.get(), body);
  EXPECT_FALSE(copied_body->m_init);
  EXPECT_EQ(copy.PhysicsWorld()->GetBodyCount(), 0);

  // Built in the copy's world, it is the copy's body: clearing one scene leaves
  // the other's simulation whole.
  copied_body->Init(copy.PhysicsWorld());
  EXPECT_EQ(copy.PhysicsWorld()->GetBodyCount(), 1);
  EXPECT_EQ(source.PhysicsWorld()->GetBodyCount(), 1);

  copy.Clear();
  EXPECT_EQ(copy.PhysicsWorld()->GetBodyCount(), 0);
  EXPECT_EQ(source.PhysicsWorld()->GetBodyCount(), 1);
}

// Copying is a replacement, not an append: what the target held is gone, the
// way a scene load replaces the open scene.
TEST(SceneTest, CopyingOverASceneReplacesWhatItHeld) {
  Scene source;
  Entity source_entity = source.CreateEntity();
  source_entity.AddComponent<TransformComponent>(glm::vec3(1.f), glm::vec3(0.f), glm::vec3(1.f));

  Scene target;
  Entity stale = target.CreateEntity();
  stale.AddComponent<TransformComponent>(glm::vec3(9.f), glm::vec3(0.f), glm::vec3(1.f));
  const Uuid stale_uuid = stale.GetUuid();

  target.CopyFrom(source);

  EXPECT_EQ(target.EntityCount(), 1u);
  EXPECT_EQ(FindByUuid(target, stale_uuid), static_cast<uint32_t>(entt::null));
  EXPECT_NE(FindByUuid(target, source_entity.GetUuid()), static_cast<uint32_t>(entt::null));
}

// An entity with no EntityEnabledComponent is enabled: that is what keeps a
// scene written before the component existed fully playable. Setting the state
// adds the component; setting it back leaves it present and true.
TEST(SceneTest, AnEntityIsEnabledUntilItIsDisabled) {
  Scene scene;
  Entity entity = scene.CreateEntity();

  EXPECT_TRUE(scene.IsEntityEnabled(entity.Handle()));
  EXPECT_FALSE(entity.HasComponent<EntityEnabledComponent>());

  scene.SetEntityEnabled(entity.Handle(), false);
  ASSERT_TRUE(entity.HasComponent<EntityEnabledComponent>());
  EXPECT_FALSE(entity.GetComponent<EntityEnabledComponent>().m_enabled);
  EXPECT_FALSE(scene.IsEntityEnabled(entity.Handle()));

  scene.SetEntityEnabled(entity.Handle(), true);
  EXPECT_TRUE(scene.IsEntityEnabled(entity.Handle()));
}

// A component with no disabled bit is enabled. Disabling writes the mask group
// the systems read; enabling clears the bit and drops the group once nothing is
// off.
TEST(SceneTest, AComponentIsEnabledUntilItIsDisabled) {
  Scene scene;
  Entity entity = scene.CreateEntity();
  entity.AddComponent<SpriteComponent>(static_cast<Sprite*>(nullptr));

  EXPECT_TRUE(scene.IsComponentEnabled<SpriteComponent>(entity.Handle()));
  EXPECT_FALSE(entity.HasComponent<DisabledComponentsComponent>());

  scene.SetComponentEnabled<SpriteComponent>(entity.Handle(), false);
  ASSERT_TRUE(entity.HasComponent<DisabledComponentsComponent>());
  EXPECT_FALSE(scene.IsComponentEnabled<SpriteComponent>(entity.Handle()));

  // One component's bit does not switch the others off.
  EXPECT_TRUE(scene.IsComponentEnabled<AudioComponent>(entity.Handle()));
  EXPECT_TRUE(scene.IsComponentEnabled<RigidBodyComponent>(entity.Handle()));

  scene.SetComponentEnabled<SpriteComponent>(entity.Handle(), true);
  EXPECT_TRUE(scene.IsComponentEnabled<SpriteComponent>(entity.Handle()));
  EXPECT_FALSE(entity.HasComponent<DisabledComponentsComponent>());
}

TEST(SceneTest, DisablingTwoComponentsKeepsTheMaskUntilBothAreBack) {
  Scene scene;
  Entity entity = scene.CreateEntity();

  scene.SetComponentEnabled<SpriteComponent>(entity.Handle(), false);
  scene.SetComponentEnabled<AudioComponent>(entity.Handle(), false);

  ASSERT_TRUE(entity.HasComponent<DisabledComponentsComponent>());
  EXPECT_FALSE(scene.IsComponentEnabled<SpriteComponent>(entity.Handle()));
  EXPECT_FALSE(scene.IsComponentEnabled<AudioComponent>(entity.Handle()));

  scene.SetComponentEnabled<SpriteComponent>(entity.Handle(), true);
  EXPECT_TRUE(entity.HasComponent<DisabledComponentsComponent>());
  EXPECT_TRUE(scene.IsComponentEnabled<SpriteComponent>(entity.Handle()));
  EXPECT_FALSE(scene.IsComponentEnabled<AudioComponent>(entity.Handle()));

  scene.SetComponentEnabled<AudioComponent>(entity.Handle(), true);
  EXPECT_FALSE(entity.HasComponent<DisabledComponentsComponent>());
}
}  // namespace
