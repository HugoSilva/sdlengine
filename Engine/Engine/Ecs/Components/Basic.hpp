#pragma once

#include <string>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

struct BasicComponent {
  BasicComponent() {};

  BasicComponent(const char* name, bool is_static, int tag, int layer)
      : m_name(name), m_is_static(is_static), m_tag(tag), m_layer(layer) {};

  std::string m_name{"New Entity"};
  bool m_is_static{false};
  int m_tag{0};
  int m_layer{0};

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Name", m_name), cereal::make_nvp("Static", m_is_static), cereal::make_nvp("Tag", m_tag),
            cereal::make_nvp("Layer", m_layer));
  }
};
