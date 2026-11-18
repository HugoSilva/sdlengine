#pragma once

#include <gtest/gtest.h>

#include <ecs/components/basic.h>
#include <ecs/components/uuid.h>
#include <ecs/entity.h>
#include <ecs/scene.h>

/*
An Entity is a value: two copies name the same entity, and every component
access goes through the scene that made it. These tests pin that the handle is
really a handle -- not a snapshot of the components -- and that its uuid is the
same value the UuidComponent holds.
*/
namespace {

using namespace rse;

TEST(EntityTest, AComponentAddedThroughTheFacadeIsTheOneItReadsBack) {
  Scene scene;
  Entity entity = scene.CreateEntity();

  BasicComponent& basic = entity.AddComponent<BasicComponent>("crate", true, 2, 3);

  EXPECT_TRUE(entity.HasComponent<BasicComponent>());
  EXPECT_EQ(&basic, &entity.GetComponent<BasicComponent>());
  EXPECT_EQ(basic.m_name, "crate");
  EXPECT_TRUE(basic.m_is_static);
  EXPECT_EQ(basic.m_tag, 2);
  EXPECT_EQ(basic.m_layer, 3);
}

TEST(EntityTest, CopiesOfAHandleNameTheSameEntity) {
  Scene scene;
  Entity entity = scene.CreateEntity();
  Entity copy = entity;

  EXPECT_EQ(entity, copy);
  EXPECT_EQ(entity.Handle(), copy.Handle());

  copy.AddComponent<BasicComponent>("shared", false, 0, 0);
  entity.GetComponent<BasicComponent>().m_name = "renamed";

  EXPECT_TRUE(copy.HasComponent<BasicComponent>());
  EXPECT_EQ(copy.GetComponent<BasicComponent>().m_name, "renamed");

  copy.Destroy();

  EXPECT_FALSE(entity.IsValid());
  EXPECT_FALSE(copy.IsValid());
  EXPECT_EQ(scene.EntityCount(), 0u);
}

TEST(EntityTest, RemovingAComponentLeavesTheEntityAlive) {
  Scene scene;
  Entity entity = scene.CreateEntity();
  entity.AddComponent<BasicComponent>("crate", false, 0, 0);

  entity.RemoveComponent<BasicComponent>();

  EXPECT_FALSE(entity.HasComponent<BasicComponent>());
  EXPECT_TRUE(entity.IsValid());
  EXPECT_EQ(scene.EntityCount(), 1u);
}

TEST(EntityTest, EveryEntityHasItsOwnIdentity) {
  Scene scene;
  const Entity first = scene.CreateEntity();
  const Entity second = scene.CreateEntity();

  EXPECT_FALSE(first.GetUuid().IsNil());
  EXPECT_FALSE(second.GetUuid().IsNil());
  EXPECT_NE(first.GetUuid(), second.GetUuid());

  // Reading it twice is the same value, and it is the value the component holds.
  EXPECT_EQ(first.GetUuid(), first.GetUuid());
  EXPECT_EQ(first.GetUuid(), first.GetComponent<UuidComponent>().m_uuid);
}

TEST(EntityTest, DestroyingThroughTheFacadeInvalidatesTheHandle) {
  Scene scene;
  Entity entity = scene.CreateEntity();

  entity.Destroy();
  entity.Destroy();

  EXPECT_FALSE(entity.IsValid());
  EXPECT_EQ(scene.EntityCount(), 0u);
}

TEST(EntityTest, TwoEntitiesInOneSceneKeepTheirComponentsApart) {
  Scene scene;
  Entity first = scene.CreateEntity();
  Entity second = scene.CreateEntity();

  first.AddComponent<BasicComponent>("first", false, 0, 0);
  second.AddComponent<BasicComponent>("second", false, 0, 0);

  EXPECT_EQ(first.GetComponent<BasicComponent>().m_name, "first");
  EXPECT_EQ(second.GetComponent<BasicComponent>().m_name, "second");

  first.GetComponent<BasicComponent>().m_name = "changed";
  EXPECT_EQ(second.GetComponent<BasicComponent>().m_name, "second");
}
}  // namespace
