#pragma once

#include <vector>

namespace rse {
class Shader;        // forward declaration for Shader
class Renderable2D;  // forward declaration for Renderable2D
class Renderer2D;    // forward declaration for Renderer2D

class Layer {
 public:
  Layer(Renderer2D* renderer, Shader* shader);
  virtual ~Layer();
  virtual void Add(Renderable2D* renderable);
  virtual void Remove(Renderable2D* renderable);
  virtual void Render();

  inline const std::vector<Renderable2D*>& GetRenderables() const { return m_Renderables; }

 protected:
  Renderer2D* m_Renderer;
  std::vector<Renderable2D*> m_Renderables;
  Shader* m_Shader;
};
}  // namespace rse