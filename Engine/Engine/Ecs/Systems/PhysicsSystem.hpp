#pragma once

#include "BaseSystem.hpp"
#include "../ECSManager.hpp"

class PhysicsSystem : public BaseSystem
{
public:
	PhysicsSystem() : BaseSystem()
	{
		m_world = new b2World{ m_gravity };
	}

	void update(const float alpha)
	{
		m_world->Step(timeStep, velocityIterations, positionIterations);

		rse::ECSManager::getView<TransformComponent, RigidBodyComponent>().each([&, this](auto entity,
			TransformComponent &transform, RigidBodyComponent &rigid)
		{
			if (!rigid.m_Rigidbody->m_init)
			{
				rigid.m_Rigidbody->init(m_world);
			}
			rigid.m_Rigidbody->Update(transform.position);
		});
	}

private:
	b2Vec2 m_gravity{ 0.0f, -10.0f };
	b2World* m_world;
	float32 timeStep = 1.0f / 6000.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;
};