#pragma once

#include <gtest/gtest.h>

#include <ecs/entity.h>
#include <ecs/scene.h>

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
}  // namespace
