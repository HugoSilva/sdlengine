#pragma once

#include <string>

#include <box2d/box2d.h>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <glm/glm.hpp>

#include "physics_material.h"
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

  // Creates the b2Body. The material overload copies the referenced asset's
  // fixture values into the body; the plain overload uses the defaults.
  void Init(b2World* world);
  void Init(b2World* world, const rse::PhysicsMaterial& material);
  void Update(glm::vec3& position);

  // A disabled body stops colliding and simulating, rather than merely being
  // skipped by PhysicsSystem while it keeps pushing other bodies around. A
  // body that has not been initialised yet has nothing to toggle; enabling it
  // later lets PhysicsSystem create it on the next update.
  void SetEnabled(bool enabled);

  // TODO lazy public variable need to fix this in the future
  bool m_init{false};

  inline b2BodyType* GetBodyType() { return &m_body_type; }

  // The physics material asset this body refers to: an asset id, or a path for
  // a file the registry does not know. Empty means no material, which uses the
  // PhysicsMaterial defaults.
  inline const std::string& GetMaterial() const { return m_material; }
  inline void SetMaterial(const std::string& material) { m_material = material; }

  // The values the fixture was built with. Before Init they are the defaults;
  // after Init they are what the referenced material supplied.
  inline const rse::PhysicsMaterial& GetMaterialValues() const { return m_material_values; }

  // Only the inputs are saved: position, size, the body type and the physics
  // material reference. m_body points into a world that does not exist while
  // the scene is a file, and m_init only says Init has run, so both stay out of
  // the file; a loaded body starts uninitialised and PhysicsSystem builds it on
  // the first update, exactly as it does for a body added in the editor.
  template <typename Archive>
  void save(Archive& archive) const {
    const std::string body_type = BodyTypeName(m_body_type);
    archive(cereal::make_nvp("Position", m_position), cereal::make_nvp("Size", m_size),
            cereal::make_nvp("BodyType", body_type), cereal::make_nvp("Material", m_material));
  }

  // No default constructor: cereal builds the body through this and reads the
  // rest into it, which is what a shared_ptr<Rigidbody> needs.
  template <typename Archive>
  static void load_and_construct(Archive& archive, cereal::construct<Rigidbody>& construct) {
    glm::vec2 position{0.f, 0.f};
    glm::vec2 size{0.f, 0.f};

    archive(cereal::make_nvp("Position", position), cereal::make_nvp("Size", size));
    construct(position, size);

    std::string body_type;
    archive(cereal::make_nvp("BodyType", body_type));
    construct->m_body_type = BodyTypeFromName(body_type);

    // Scenes and prefabs are XML archives. A file written before physics
    // materials has no Material node: density and friction used to sit in this
    // position. Reading the field only when the next node names it keeps those
    // older files loading without a fallback to the removed values.
    if (NextNodeIs(archive, "Material")) {
      archive(cereal::make_nvp("Material", construct->m_material));
    }
  }

 private:
  // The file stores the type by name, so a renumbered or reordered b2BodyType
  // enum stays readable.
  static std::string BodyTypeName(b2BodyType type);
  static b2BodyType BodyTypeFromName(const std::string& name);

  // XML input archives expose the next child's name. The project only
  // serialises scenes and prefabs as XML, so this is enough to treat the
  // material reference as an optional tail field.
  template <typename Archive>
  static bool NextNodeIs(Archive& archive, const char* name) {
    const char* next = archive.getNodeName();
    return next != nullptr && std::string{next} == name;
  }

  b2World* m_world{nullptr};
  b2Body* m_body{nullptr};
  rse::PhysicsMaterial m_material_values;
  std::string m_material;
  glm::vec2 m_position{0.f, 0.f};
  glm::vec2 m_size{0.f, 0.f};

  b2BodyType m_body_type{b2_staticBody};
};
