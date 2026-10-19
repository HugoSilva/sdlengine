#include "physics_manager.h"

#include <core.h>

b2Vec2 PhysicsManager::m_gravity{0.0f, -10.0f};
b2World* PhysicsManager::m_world;
std::vector<Rigidbody*> PhysicsManager::m_rigidbodies;
// Needs to be dynamic to reflect the fps count
float PhysicsManager::time_step = 1.0f / 6000.0f;
int32 PhysicsManager::velocity_iterations = 6;
int32 PhysicsManager::position_iterations = 2;

void PhysicsManager::Init() { m_world = new b2World{m_gravity}; }

void PhysicsManager::Add(Rigidbody* rb) { m_rigidbodies.push_back(rb); }

void PhysicsManager::Update() {
  // Prepare for simulation. Typically we use a time step of 1/60 of a
  // second (60Hz) and 10 iterations. This provides a high quality simulation
  // in most game scenarios.
  m_world->Step(time_step, velocity_iterations, position_iterations);
}

void PhysicsManager::UpdateObjects() {
  while (Core::GetRunning()) {
    m_world->Step(time_step, velocity_iterations, position_iterations);

    // for (Rigidbody* rb : m_Rigidbodies)
    // rb->Update();
  }
}

void PhysicsManager::Clean() {
  for (unsigned int i = 0; i < m_rigidbodies.size(); i++) delete m_rigidbodies[i];

  m_world = nullptr;
}

b2World* PhysicsManager::GetWorldObject() { return m_world; }