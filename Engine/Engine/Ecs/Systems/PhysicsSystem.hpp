#pragma once

#include "BaseSystem.hpp"
#include "../ECSManager.hpp"

class PhysicsSystem : public BaseSystem
{
public:
	PhysicsSystem() : BaseSystem() {}

	void update(const float alpha)
	{
		rse::ECSManager::getView<TransformComponent, RigidBodyComponent>().each([&, this](auto entity,
			TransformComponent &transform, RigidBodyComponent &rigid)
		{
			//TODO Should the old physics manager be replaced ?
			//TODO Remove the logic from the old Rigidbody update function and do it here
			//transform, rigid
		});
	}
};