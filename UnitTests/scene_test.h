#pragma once

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include <ecs/components/basic.h>
#include <ecs/components/rigid_body.h>
#include <ecs/components/transform.h>
#include <ecs/entity.h>
#include <ecs/scene.h>
#include <physics/rigidbody.h>

/*
A scene owns its entities, so the only thing that has to be true is that its
handles keep naming what they were made for. The failure mode is silent: an id
is an index into a registry and is recycled, so a handle that survives a destroy
without noticing would reach whatever entity took the slot next.
*/
namespace {

using namespace rse;

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
}  // namespace
