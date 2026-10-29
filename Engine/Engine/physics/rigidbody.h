#pragma once

#include <string>

#include <box2d/box2d.h>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <glm/glm.hpp>

#include "updatable.h"

class Rigidbody {
 public:
  Rigidbody(glm::vec2 position, glm::vec2 size);
  ~Rigidbody();

  // A body is a simulation object, not data: it belongs to one world and is
  // created by Init. Nothing copies a body around; the component owns the one
  // it has and the registry destroys it with the entity.
  Rigidbody(const Rigidbody&) = delete;
  Rigidbody& operator=(const Rigidbody&) = delete;

  // A second body with the same inputs, still uninitialised: a prefab placed
  // twice has to own two bodies, and a b2Body belongs to a world and cannot be
  // copied. The caller owns the result.
  Rigidbody* Clone() const;

  void Init(b2World* world);
  void Update(glm::vec3& position);

  // A disabled body stops colliding and simulating, rather than merely being
  // skipped by PhysicsSystem while it keeps pushing other bodies around. A
  // body that has not been initialised yet has nothing to toggle; enabling it
  // later lets PhysicsSystem create it on the next update.
  void SetEnabled(bool enabled);

  // TODO lazy public variable need to fix this in the future
  bool m_init{false};

  inline float* GetDensity() { return &m_density; }
  inline float* GetFriction() { return &m_friction; }
  inline b2BodyType* GetBodyType() { return &m_body_type; }

  // Only the inputs are saved: position, size, density, friction and the body
  // type. m_body points into a world that does not exist while the scene is a
  // file, and m_init only says Init has run, so both stay out of the file; a
  // loaded body starts uninitialised and PhysicsSystem builds it on the first
  // update, exactly as it does for a body added in the editor.
  template <typename Archive>
  void save(Archive& archive) const {
    const std::string body_type = BodyTypeName(m_body_type);
    archive(cereal::make_nvp("Position", m_position), cereal::make_nvp("Size", m_size),
            cereal::make_nvp("Density", m_density), cereal::make_nvp("Friction", m_friction),
            cereal::make_nvp("BodyType", body_type));
  }

  // No default constructor: cereal builds the body through this and reads the
  // rest into it, which is what a shared_ptr<Rigidbody> needs.
  template <typename Archive>
  static void load_and_construct(Archive& archive, cereal::construct<Rigidbody>& construct) {
    glm::vec2 position{0.f, 0.f};
    glm::vec2 size{0.f, 0.f};

    archive(cereal::make_nvp("Position", position), cereal::make_nvp("Size", size));
    construct(position, size);

    archive(cereal::make_nvp("Density", construct->m_density), cereal::make_nvp("Friction", construct->m_friction));

    std::string body_type;
    archive(cereal::make_nvp("BodyType", body_type));
    construct->m_body_type = BodyTypeFromName(body_type);
  }

 private:
  // The file stores the type by name, so a renumbered or reordered b2BodyType
  // enum stays readable.
  static std::string BodyTypeName(b2BodyType type);
  static b2BodyType BodyTypeFromName(const std::string& name);

  b2World* m_world{nullptr};
  b2Body* m_body{nullptr};
  float m_density = 1.0f;
  float m_friction = 0.3f;
  glm::vec2 m_position{0.f, 0.f};
  glm::vec2 m_size{0.f, 0.f};

  b2BodyType m_body_type{b2_staticBody};
};
