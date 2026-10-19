#pragma once

#include "Test.h"

namespace test {
class TestClearColor : public Test {
 public:
  TestClearColor();
  ~TestClearColor();

  void OnUpdate(float deltaTime) override;
  void OnRender() override;
  void OnImguiRender() override;

  // The renderer clears with this before the scene pass.
  float* getClearColor() { return m_clearColor; }

 private:
  float m_clearColor[4];
};
}  // namespace test
