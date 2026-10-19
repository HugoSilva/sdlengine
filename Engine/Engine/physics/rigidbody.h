#pragma once

#include <box2d/box2d.h>

#include "updatable.h"

class Rigidbody {
 public:
  Rigidbody(glm::vec2 position, glm::vec2 size);
  void Init(b2World* world);
  void Update(glm::vec3& position);
  // TODO lazy public variable need to fix this in the future
  bool m_init{false};

  inline float* GetDensity() { return &m_density; }
  inline float* GetFriction() { return &m_friction; }
  inline b2BodyType* GetBodyType() { return &m_body_type; }

 private:
  b2Body* m_body;
  float m_density = 1.0f;
  float m_friction = 0.3f;
  glm::vec2 m_position{0.f, 0.f};
  glm::vec2 m_size{0.f, 0.f};

  b2BodyType m_body_type{b2_staticBody};
};