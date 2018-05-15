#include "PhysicsManager.h"

b2Vec2 PhysicsManager::m_gravity{ 0.0f, -10.0f };
b2World PhysicsManager::m_world{ m_gravity };
std::vector<Rigidbody*> PhysicsManager::m_Rigidbodies;

void PhysicsManager::init()
{
	m_world = b2World{ m_gravity };
}

void PhysicsManager::add(Rigidbody* rb)
{
	m_Rigidbodies.push_back(rb);
}

void PhysicsManager::Update(float deltaTime)
{
	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	m_world.Step(timeStep, velocityIterations, positionIterations);
	
	for (Rigidbody* rb : m_Rigidbodies)
		rb->Update(deltaTime);
}

b2World* PhysicsManager::GetWorldObject()
{
	return &m_world;
}