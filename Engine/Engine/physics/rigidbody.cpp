#include "rigidbody.h"

#include "physics_manager.h"

Rigidbody::Rigidbody(glm::vec2 position, glm::vec2 size) : m_position(position), m_size(size) {}

void Rigidbody::Init(b2World* world) {
  b2BodyDef body_definition;

  body_definition.position.Set(m_position.x, m_position.y);
  body_definition.type = m_body_type;

  m_body = world->CreateBody(&body_definition);

  b2PolygonShape dynamic_box;
  dynamic_box.SetAsBox(m_size.x, m_size.y);

  b2FixtureDef fixture_def;
  fixture_def.shape = &dynamic_box;
  fixture_def.density = m_density;
  fixture_def.friction = m_friction;

  m_body->CreateFixture(&fixture_def);

  m_init = true;
}

void Rigidbody::Update(glm::vec3& position) {
  // TODO remove this from the update there should be a editor mode which should reset init to false
  if (m_body->GetType() != m_body_type) {
    m_body->SetType(m_body_type);
  }
  b2Vec2 body_position = m_body->GetPosition();
  position = glm::vec3(body_position.x, body_position.y, 0.f);
}