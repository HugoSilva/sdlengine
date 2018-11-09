#pragma once

#include <Box2D.h>
#include "Updatable.h"
#include "../Components/Sprite.hpp"

class Rigidbody : public physics::Updatable
{
public:
	Rigidbody(rse::Sprite* sprite, glm::vec2 position, glm::vec2 size, bool useType = false, bool useFixture = false);
	void Update() override;

private:
	b2Body* m_body;
	float m_density = 1.0f;
	float m_friction = 0.3f;
	rse::Sprite* m_Sprite;
};