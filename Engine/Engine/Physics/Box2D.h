#pragma once

#include <Box2D.h>
#include "Updatable.h"

class Box2D : public physics::Updatable
{
public:
	Box2D();
	void Update(float deltaTime) override;
private:
	b2Vec2 m_gravity{ 0.0f, -10.0f };
	b2World m_world{ m_gravity };

	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;
};