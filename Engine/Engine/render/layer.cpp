#include "layer.h"

#include "renderable_2d.h"
#include "renderer_2d.h"
#include <components/shader.h>

namespace rse {
Layer::Layer(Renderer2D* renderer, Shader* shader) : m_renderer(renderer), m_shader(shader) {
  m_shader->Enable();

#ifdef EMSCRIPTEN
  m_Shader->setUniform1i("texture_0", 0);
  m_Shader->setUniform1i("texture_1", 1);
  m_Shader->setUniform1i("texture_2", 2);
  m_Shader->setUniform1i("texture_3", 3);
  m_Shader->setUniform1i("texture_4", 4);
  m_Shader->setUniform1i("texture_5", 5);
  m_Shader->setUniform1i("texture_6", 6);
  m_Shader->setUniform1i("texture_7", 7);
#else
  int tex_i_ds[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

  m_shader->SetUniform1iv("textures", tex_i_ds, 32);
#endif
  m_shader->Disable();
}

Layer::~Layer() {
  delete m_shader;
  delete m_renderer;

  for (unsigned int i = 0; i < m_renderables.size(); i++) delete m_renderables[i];
}

void Layer::Add(Renderable2D* renderable) { m_renderables.push_back(renderable); }

void Layer::Remove(Renderable2D* renderable) {
  for (std::vector<Renderable2D*>::iterator it = m_renderables.begin(); it != m_renderables.end(); ++it) {
    if (*it == renderable) {
      m_renderables.erase(it);
    }
  }
}

void Layer::Render() {
  m_shader->Enable();
  m_renderer->Begin();

  for (const Renderable2D* renderable : m_renderables) renderable->Submit(m_renderer);

  m_renderer->End();
  m_renderer->Flush();
}
}  // namespace rse