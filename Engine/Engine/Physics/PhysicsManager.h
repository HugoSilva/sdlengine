#pragma once

#include <Box2D.h>
#include "Updatable.h"
#include "..\Components\Sprite.h"
#include "Rigidbody.h"

class PhysicsManager : public physics::Updatable
{
public:
	static void init();
	static void add(Rigidbody* rb);
	void Update(float deltaTime) override;
	static b2World* GetWorldObject();

private:
	PhysicsManager() { }
	static std::vector<Rigidbody*> m_Rigidbodies;
	static b2Vec2 m_gravity;
	static b2World m_world;

	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;
};