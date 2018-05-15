#include "Rigidbody.h"

Rigidbody::Rigidbody(glm::vec2 position, glm::vec2 size, bool useType, bool useFixture)
{
	b2BodyDef bodyDefinition;
	if (useType)
	{
		bodyDefinition.type = b2_dynamicBody;
	}
	bodyDefinition.position.Set(position.x, position.y);

	m_body = PhysicsManager::GetWorldObject()->CreateBody(&bodyDefinition);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(size.x, size.y);

	if (useFixture)
	{
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;
		fixtureDef.density = m_density;
		fixtureDef.friction = m_friction;

		m_body->CreateFixture(&fixtureDef);
	}
	else
	{
		m_body->CreateFixture(&dynamicBox, 0.0f);
	}
}

void Rigidbody::Update(float deltaTime, Sprite sprite)
{
	b2Vec2 position = m_body->GetPosition();
	sprite.position = glm::vec3(position.x, position.y, 0);
}