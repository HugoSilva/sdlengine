#include "ImGuiLayer.hpp"

#include <imgui.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

namespace rse {
namespace {
SDL_Window* s_Window{nullptr};
}

void ImGuiLayer::Init(SDL_Window* window, SDL_GLContext context) {
  s_Window = window;

  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  // The vendored ImGui is the docking branch (v1.93.0-docking), which is
  // what gives the Editor its DockSpace based layout.
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplSDL3_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init("#version 130");
  ImGui::StyleColorsDark();
}

void ImGuiLayer::Shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  s_Window = nullptr;
}

void ImGuiLayer::BeginFrame() {
  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::EndFrame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_SwapWindow(s_Window);
}

void ImGuiLayer::ProcessEvent(const SDL_Event& event) { ImGui_ImplSDL3_ProcessEvent(&event); }
}  // namespace rse
