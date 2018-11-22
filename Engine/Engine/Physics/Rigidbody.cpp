#include "Rigidbody.h"
#include "PhysicsManager.h"

Rigidbody::Rigidbody(glm::vec2 position, glm::vec2 size)
	:m_position(position), m_size(size)
{
}

void Rigidbody::init(b2World* world)
{
	b2BodyDef bodyDefinition;

	bodyDefinition.position.Set(m_position.x, m_position.y);
	bodyDefinition.type = m_bodyType;

	m_body = world->CreateBody(&bodyDefinition);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(m_size.x, m_size.y);
	
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = m_density;
	fixtureDef.friction = m_friction;

	m_body->CreateFixture(&fixtureDef);

	m_init = true;
}

void Rigidbody::Update(glm::vec3& position)
{
	//TODO remove this from the update there should be a editor mode which should reset init to false
	if (m_body->GetType() != m_bodyType)
	{
		m_body->SetType(m_bodyType);
	}
	b2Vec2 Position = m_body->GetPosition();
	position = glm::vec3(Position.x, Position.y, 0.f);
}