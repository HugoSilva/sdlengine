#include "rigidbody.h"

Rigidbody::Rigidbody(glm::vec2 position, glm::vec2 size) : m_position(position), m_size(size) {}

Rigidbody::~Rigidbody() {
  // A loaded body has none until its first update; the world is the one
  // PhysicsSystem owns and it is still alive when the registry is emptied.
  if (m_body != nullptr) {
    m_world->DestroyBody(m_body);
  }
}

void Rigidbody::Init(b2World* world) {
  if (m_body != nullptr) {
    return;
  }

  m_world = world;

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

std::string Rigidbody::BodyTypeName(b2BodyType type) {
  switch (type) {
    case b2_staticBody:
      return "static";
    case b2_kinematicBody:
      return "kinematic";
    case b2_dynamicBody:
      return "dynamic";
    default:
      return "static";
  }
}

b2BodyType Rigidbody::BodyTypeFromName(const std::string& name) {
  if (name == "kinematic") {
    return b2_kinematicBody;
  }

  if (name == "dynamic") {
    return b2_dynamicBody;
  }

  return b2_staticBody;
}
