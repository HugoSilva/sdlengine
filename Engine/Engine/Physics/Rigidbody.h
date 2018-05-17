#pragma once

#include <Box2D.h>
#include "Updatable.h"
#include "..\Components\Sprite.h"

class Rigidbody : public physics::Updatable
{
public:
	Rigidbody(Sprite* sprite, glm::vec2 position, glm::vec2 size, bool useType = false, bool useFixture = false);
	void Update(float deltaTime) override;

private:
	b2Body* m_body;
	float m_density = 1.0f;
	float m_friction = 0.3f;
	Sprite* m_Sprite;
};