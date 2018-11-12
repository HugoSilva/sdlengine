#pragma once

#include "BaseSystem.hpp"
#include "../ECSManager.hpp"

class PhysicsSystem : public BaseSystem
{
public:
	PhysicsSystem() : BaseSystem() {}

	void update(const float alpha)
	{
		m_world->Step(timeStep, velocityIterations, positionIterations);

		rse::ECSManager::getView<TransformComponent, RigidBodyComponent>().each([&, this](auto entity,
			TransformComponent &transform, RigidBodyComponent &rigid)
		{
			rigid.m_Rigidbody->Update();
		});
	}

private:
	b2World* m_world;
	float32 timeStep = 1.0f / 6000.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;
};