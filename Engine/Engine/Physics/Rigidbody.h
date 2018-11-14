#pragma once

#include <Box2D.h>
#include "Updatable.h"

class Rigidbody
{
public:
	Rigidbody(glm::vec2 position, glm::vec2 size, bool useType = false, bool useFixture = false);
	void init(b2World* world);
	void Update(glm::vec3& position);
	//TODO lazy public variable need to fix this in the future
	bool m_init{ false };

private:
	b2Body* m_body;
	float m_density = 1.0f;
	float m_friction = 0.3f;
	glm::vec2 m_position{ 0.f, 0.f };
	glm::vec2 m_size{ 0.f, 0.f };
	bool m_useType{ false };
	bool m_useFixture{ false };
};