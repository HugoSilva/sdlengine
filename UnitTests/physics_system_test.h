#pragma once

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "scene_test_support.h"
#include <core/fixed_timestep.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/scene_io.h>
#include <ecs/systems/physics_system.h>

/*
Physics runs headless: the active scene owns the b2World, so a test can step it
without a renderer. The first update is what creates each b2Body, which is also
the path a loaded scene takes -- a component that came back from a file has no
body until then.
*/
namespace {
using namespace rse;

class PhysicsSystemTest : public ::testing::Test {
 protected:
  void TearDown() override {
    TestScene().Clear();
    PhysicsMaterialManager::Clean();
  }

  static uint32_t CreateBody(const char* name, float y, b2BodyType type) {
    const uint32_t entity = TestScene().CreateEntity().Handle();
    TestScene().Registry().assign<BasicComponent>(entity, name, false, 0, 0);
    TestScene().Registry().assign<TransformComponent>(entity, glm::vec3(0.f, y, 0.f), glm::vec3(0.f), glm::vec3(1.f));

    Rigidbody* body = new Rigidbody(glm::vec2(0.f, y), glm::vec2(8.f, 8.f));
    *body->GetBodyType() = type;
    TestScene().Registry().assign<RigidBodyComponent>(entity, body);
    return entity;
  }
};

TEST_F(PhysicsSystemTest, ABodyIsNotSimulatedUntilTheFirstUpdate) {
  CreateBody("faller", 100.f, b2_dynamicBody);

  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        ASSERT_NE(rigid.m_rigidbody, nullptr);
        EXPECT_FALSE(rigid.m_rigidbody->m_init);
        EXPECT_FLOAT_EQ(transform.position.y, 100.f);
      });
}

// The system builds each body in the active scene's world; that world is what
// it steps, and the scene outlives the entities so the bodies have a world to
// be destroyed from.
TEST_F(PhysicsSystemTest, BodiesAreBuiltInTheActiveSceneWorld) {
  CreateBody("faller", 100.f, b2_dynamicBody);

  b2World* world = TestScene().PhysicsWorld();
  ASSERT_EQ(world->GetBodyCount(), 0);

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  EXPECT_EQ(world->GetBodyCount(), 1);
  EXPECT_EQ(TestScene().PhysicsWorld()->GetBodyCount(), 1);
}

TEST_F(PhysicsSystemTest, ABodyWithNoMaterialUsesTheDefaults) {
  CreateBody("faller", 100.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  TestScene().Registry().view<RigidBodyComponent>().each([](auto, RigidBodyComponent& rigid) {
    ASSERT_NE(rigid.m_rigidbody, nullptr);
    EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().density, PhysicsMaterial::kDefaultDensity);
    EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().friction, PhysicsMaterial::kDefaultFriction);
    EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().restitution, PhysicsMaterial::kDefaultRestitution);
  });
}

TEST_F(PhysicsSystemTest, ABodyUsesTheMaterialItReferences) {
  auto material = std::make_unique<PhysicsMaterialAsset>();
  material->id = "0123456789abcdef";
  material->name = "Ice";
  material->material.density = 0.9f;
  material->material.friction = 0.05f;
  material->material.restitution = 0.1f;
  PhysicsMaterialManager::Add(std::move(material));

  const uint32_t entity = CreateBody("faller", 100.f, b2_dynamicBody);
  RigidBodyComponent& rigid = TestScene().Registry().get<RigidBodyComponent>(entity);
  ASSERT_NE(rigid.m_rigidbody, nullptr);
  rigid.m_rigidbody->SetMaterial("0123456789abcdef");

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().density, 0.9f);
  EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().friction, 0.05f);
  EXPECT_FLOAT_EQ(rigid.m_rigidbody->GetMaterialValues().restitution, 0.1f);
}

TEST_F(PhysicsSystemTest, ADynamicBodyFalls) {
  CreateBody("faller", 100.f, b2_dynamicBody);

  PhysicsSystem physics;

  for (int step = 0; step < 60; ++step) {
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_LT(transform.position.y, 100.f);
      });
}

TEST_F(PhysicsSystemTest, AStaticBodyStaysWhereItWasPut) {
  CreateBody("floor", 100.f, b2_staticBody);

  PhysicsSystem physics;

  for (int step = 0; step < 600; ++step) {
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_FLOAT_EQ(transform.position.y, 100.f);
      });
}

TEST_F(PhysicsSystemTest, ADisabledBodyIsNotSimulatedUntilItIsEnabledAgain) {
  const uint32_t entity = CreateBody("faller", 100.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  TestScene().SetEntityEnabled(entity, false);
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  float stopped_y = 0.f;
  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [&stopped_y](auto, TransformComponent& transform, RigidBodyComponent&) { stopped_y = transform.position.y; });

  for (int step = 0; step < 60; ++step) {
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [stopped_y](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_FLOAT_EQ(transform.position.y, stopped_y);
      });

  TestScene().SetEntityEnabled(entity, true);

  for (int step = 0; step < 60; ++step) {
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [stopped_y](auto, TransformComponent& transform, RigidBodyComponent&) {
        EXPECT_LT(transform.position.y, stopped_y);
      });
}

TEST_F(PhysicsSystemTest, ADisabledRigidBodyComponentIsNotSimulatedUntilItIsEnabledAgain) {
  const uint32_t entity = CreateBody("faller", 100.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  TestScene().SetComponentEnabled<RigidBodyComponent>(entity, false);
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  float stopped_y = 0.f;
  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [&stopped_y](auto, TransformComponent& transform, RigidBodyComponent&) { stopped_y = transform.position.y; });

  for (int step = 0; step < 60; ++step) {
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [stopped_y](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_FLOAT_EQ(transform.position.y, stopped_y);
      });

  TestScene().SetComponentEnabled<RigidBodyComponent>(entity, true);

  for (int step = 0; step < 60; ++step) {
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [stopped_y](auto, TransformComponent& transform, RigidBodyComponent&) {
        EXPECT_LT(transform.position.y, stopped_y);
      });
}

// A body outlives the scene it was simulated in: emptying the registry has to
// destroy the b2Body (the world is still alive), and the next scene has to be
// able to create its own bodies without either one being freed twice.
TEST_F(PhysicsSystemTest, ClearingTheRegistryFreesTheBodyItSimulated) {
  CreateBody("first", 100.f, b2_dynamicBody);

  {
    PhysicsSystem physics;
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  TestScene().Clear();
  EXPECT_EQ(TestScene().Registry().view<RigidBodyComponent>().size(), 0u);

  CreateBody("second", 200.f, b2_dynamicBody);

  PhysicsSystem physics;

  for (int step = 0; step < 60; ++step) {
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  TestScene().Registry().view<TransformComponent, RigidBodyComponent>().each(
      [](auto, TransformComponent& transform, RigidBodyComponent& rigid) {
        EXPECT_TRUE(rigid.m_rigidbody->m_init);
        EXPECT_LT(transform.position.y, 200.f);
      });
}

// The transform's rotation is in degrees and a body's angle in radians: it is
// easy to write one and forget the other, which is what left a toppling crate's
// sprite standing upright while its collider turned.
TEST_F(PhysicsSystemTest, TheBodyAngleReachesTheTransform) {
  const uint32_t entity = CreateBody("crate", 100.f, b2_dynamicBody);
  RigidBodyComponent& rigid = TestScene().Registry().get<RigidBodyComponent>(entity);
  ASSERT_NE(rigid.m_rigidbody, nullptr);

  // A world of the test's own, so the body's angle can be set without a
  // simulation that has to produce a rotation on its own.
  b2World world{b2Vec2{0.f, -10.f}};
  rigid.m_rigidbody->Init(&world);

  b2Body* body = world.GetBodyList();
  ASSERT_NE(body, nullptr);
  body->SetTransform(body->GetPosition(), 0.5f);

  TransformComponent& transform = TestScene().Registry().get<TransformComponent>(entity);
  rigid.m_rigidbody->Update(transform.position, transform.rotation);

  EXPECT_NEAR(transform.rotation.z, glm::degrees(0.5f), 0.001f);

  // The body belongs to the world on this stack frame, so it has to go before
  // the world does.
  TestScene().Registry().destroy(entity);
}

// The gate for the fixed timestep: one second of wall time drops a body the
// same distance whatever the frame rate is. Before the accumulator, the same
// second dropped it once per frame, so a 144 Hz display fell 2.4 times as far
// as a 60 Hz one.
TEST_F(PhysicsSystemTest, OneSecondOfFallIsTheSameAtEveryFrameRate) {
  const float start_y = 1000.f;
  constexpr float kDropInOneSecond = 5.0f;  // gravity is 10 units per second squared

  for (float rate : {60.0f, 144.0f, 240.0f}) {
    TestScene().Clear();

    const uint32_t entity = CreateBody("faller", start_y, b2_dynamicBody);

    {
      PhysicsSystem physics;
      FixedTimestep timestep;

      for (int frame = 0; frame < static_cast<int>(rate); ++frame) {
        const int steps = timestep.Advance(1.0f / rate);

        for (int step = 0; step < steps; ++step) {
          physics.UpdateSimulation(TestScene(), timestep.Step());
        }
      }
    }

    const float dropped = start_y - TestScene().Registry().get<TransformComponent>(entity).position.y;

    // A quarter of a unit is well inside one step of the fall, and outside the
    // difference a per-frame step would produce (2.4x).
    EXPECT_NEAR(dropped, kDropInOneSecond, 0.25f) << "at " << rate << " Hz";
  }

  TestScene().Clear();
}
// A contact is reported as a pair of entity ids, whichever fixture Box2D made
// first, so the comparison here ignores order.
bool HasEvent(const std::vector<CollisionEvent>& events, uint32_t a, uint32_t b) {
  for (const CollisionEvent& event : events) {
    if ((event.entity_a == a && event.entity_b == b) || (event.entity_a == b && event.entity_b == a)) {
      return true;
    }
  }

  return false;
}

// The first update builds the bodies after the step; the next step is the one
// that sees them and produces the contact.
TEST_F(PhysicsSystemTest, OverlappingBodiesProduceABeginEvent) {
  const uint32_t floor = CreateBody("floor", 0.f, b2_staticBody);
  const uint32_t crate = CreateBody("crate", 0.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  const CollisionEvents& events = TestScene().GetCollisionEvents();
  ASSERT_EQ(1u, events.begun.size());
  EXPECT_TRUE(HasEvent(events.begun, floor, crate));
  EXPECT_FALSE(events.begun[0].sensor);

  // The body names the entity it belongs to, which is what the event carries.
  EXPECT_EQ(crate, TestScene().Registry().get<RigidBodyComponent>(crate).m_rigidbody->GetEntity());
}

TEST_F(PhysicsSystemTest, AContactIsReportedOnceWhileItLasts) {
  CreateBody("floor", 0.f, b2_staticBody);
  CreateBody("crate", 0.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);

  for (int step = 0; step < 30; ++step) {
    physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  }

  EXPECT_EQ(1u, TestScene().GetCollisionEvents().begun.size());
  EXPECT_TRUE(TestScene().GetCollisionEvents().ended.empty());
}

TEST_F(PhysicsSystemTest, DestroyingABodyEndsItsContacts) {
  const uint32_t floor = CreateBody("floor", 0.f, b2_staticBody);
  const uint32_t crate = CreateBody("crate", 0.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  ASSERT_EQ(1u, TestScene().GetCollisionEvents().begun.size());

  // DestroyBody reports EndContact while the contact still exists.
  TestScene().Registry().destroy(crate);

  const CollisionEvents& events = TestScene().GetCollisionEvents();
  ASSERT_EQ(1u, events.ended.size());
  EXPECT_TRUE(HasEvent(events.ended, floor, crate));
}

TEST_F(PhysicsSystemTest, ClearingTheQueueForgetsTheFrame) {
  CreateBody("floor", 0.f, b2_staticBody);
  CreateBody("crate", 0.f, b2_dynamicBody);

  PhysicsSystem physics;
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  physics.UpdateSimulation(TestScene(), 1.0f / 60.0f);
  ASSERT_FALSE(TestScene().GetCollisionEvents().Empty());

  TestScene().ClearCollisionEvents();

  EXPECT_TRUE(TestScene().GetCollisionEvents().Empty());
}

}  // namespace
