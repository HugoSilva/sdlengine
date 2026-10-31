#include "rigidbody.h"

Rigidbody::Rigidbody(glm::vec2 position, glm::vec2 size) : m_position(position), m_size(size) {}

Rigidbody* Rigidbody::Clone() const {
  // Init has not run on the copy, so m_world, m_body and m_init keep the
  // values a newly constructed body has: PhysicsSystem builds it later.
  Rigidbody* clone = new Rigidbody(m_position, m_size);
  clone->m_material = m_material;
  clone->m_material_values = m_material_values;
  clone->m_body_type = m_body_type;
  return clone;
}

Rigidbody::~Rigidbody() {
  // A loaded body has none until its first update; the world is the one
  // PhysicsSystem owns and it is still alive when the registry is emptied.
  if (m_body != nullptr) {
    m_world->DestroyBody(m_body);
  }
}

void Rigidbody::Init(b2World* world) { Init(world, rse::PhysicsMaterial{}); }

void Rigidbody::Init(b2World* world, const rse::PhysicsMaterial& material) {
  if (m_body != nullptr) {
    return;
  }

  m_world = world;
  m_material_values = material;

  b2BodyDef body_definition;

  body_definition.position.Set(m_position.x, m_position.y);
  body_definition.type = m_body_type;

  m_body = world->CreateBody(&body_definition);

  b2PolygonShape dynamic_box;
  dynamic_box.SetAsBox(m_size.x, m_size.y);

  b2FixtureDef fixture_def;
  fixture_def.shape = &dynamic_box;
  fixture_def.density = material.density;
  fixture_def.friction = material.friction;
  fixture_def.restitution = material.restitution;

  m_body->CreateFixture(&fixture_def);

  m_init = true;
}

void Rigidbody::SetEnabled(bool enabled) {
  if (m_body == nullptr) {
    return;
  }

  m_body->SetEnabled(enabled);
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
