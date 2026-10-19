#include "test_clear_color.h"

#include <imgui.h>

#include <GL/glew.h>

namespace test {
TestClearColor::TestClearColor() : m_clear_color{1.0f, 0.0f, 1.0f, 1.0f} {}

TestClearColor::~TestClearColor() {}

void TestClearColor::OnUpdate(float delta_time) {}

void TestClearColor::OnRender() {
  glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TestClearColor::OnImguiRender() { ImGui::ColorEdit4("Clear Color", m_clear_color); }
}  // namespace test
