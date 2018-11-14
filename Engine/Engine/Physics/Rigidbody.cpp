#include "Rigidbody.h"
#include "PhysicsManager.h"

Rigidbody::Rigidbody(glm::vec2 position, glm::vec2 size, bool useType, bool useFixture)
	:m_position(position), m_size(size), m_useType(useType), m_useFixture(useFixture)
{
}

void Rigidbody::init(b2World* world)
{
	b2BodyDef bodyDefinition;
	if (m_useType)
	{
		bodyDefinition.type = b2_dynamicBody;
	}
	bodyDefinition.position.Set(m_position.x, m_position.y);

	m_body = world->CreateBody(&bodyDefinition);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(m_size.x, m_size.y);

	if (m_useFixture)
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
	m_init = true;
}

void Rigidbody::Update(glm::vec3& position)
{
	b2Vec2 Position = m_body->GetPosition();
	position = glm::vec3(Position.x, Position.y, 0.f);
}