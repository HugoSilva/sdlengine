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
	void Update() override;
	static void UpdateObjects();
	static void Clean();
	static b2World* GetWorldObject();

private:
	PhysicsManager() { }
	static std::vector<Rigidbody*> m_Rigidbodies;
	static b2Vec2 m_gravity;
	static b2World* m_world;

	static float32 timeStep;
	static int32 velocityIterations;
	static int32 positionIterations;
};