#include "PhysicsManager.h"
#include "../Core.h"

b2Vec2 PhysicsManager::m_gravity{ 0.0f, -10.0f };
b2World* PhysicsManager::m_world;
std::vector<Rigidbody*> PhysicsManager::m_Rigidbodies;
// Needs to be dynamic to reflect the fps count
float32 PhysicsManager::timeStep = 1.0f / 6000.0f;
int32 PhysicsManager::velocityIterations = 6;
int32 PhysicsManager::positionIterations = 2;

void PhysicsManager::init()
{
	m_world = new b2World{ m_gravity };
}

void PhysicsManager::add(Rigidbody* rb)
{
	m_Rigidbodies.push_back(rb);
}

void PhysicsManager::Update()
{
	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	m_world->Step(timeStep, velocityIterations, positionIterations);
}

void PhysicsManager::UpdateObjects()
{
	while (Core::getRunning())
	{
		m_world->Step(timeStep, velocityIterations, positionIterations);

		//for (Rigidbody* rb : m_Rigidbodies)
			//rb->Update();
	}
}

void PhysicsManager::Clean()
{
	for (unsigned int i = 0; i < m_Rigidbodies.size(); i++)
		delete m_Rigidbodies[i];

	m_world = nullptr;
}

b2World* PhysicsManager::GetWorldObject()
{
	return m_world;
}