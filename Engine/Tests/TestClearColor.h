#pragma once

#include "Test.h"

namespace test {
class TestClearColor : public Test {
 public:
  TestClearColor();
  ~TestClearColor();

  void OnUpdate(float delta_time) override;
  void OnRender() override;
  void OnImguiRender() override;

  // The renderer clears with this before the scene pass.
  float* GetClearColor() { return m_clear_color; }

 private:
  float m_clear_color[4];
};
}  // namespace test
