#pragma once

#include "../Components/Shader.hpp"

class Scene {
 public:
  Scene() {};

  virtual void Update(float delta_time) = 0;

  virtual void Render() = 0;

  inline const std::string& GetName() const { return m_name; }

 protected:
  std::string m_name;
  rse::Shader* m_shader;
};