#include "imgui_layer.h"

#include <filesystem>
#include <imgui.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include <utils/logger.h>

namespace rse {
namespace {
SDL_Window* s_window{nullptr};

// Dear ImGui's built-in font is 13 px, which is small on a modern display. The
// UI uses Arial at a larger base size instead, resolved the way the rest of the
// engine resolves resources. The built-in font is the fallback, so a binary
// without the resource still shows readable text.
constexpr const char* kUiFont = "Resources/arial.ttf";
constexpr float kUiFontSize = 18.0f;
constexpr float kBuiltInFontSize = 13.0f;
}  // namespace

void ImGuiLayer::Init(SDL_Window* window, SDL_GLContext context) {
  s_window = window;

  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  // The vendored ImGui is the docking branch (v1.93.0-docking), which is
  // what gives the Editor its DockSpace based layout.
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplSDL3_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init("#version 130");
  ImGui::StyleColorsDark();

  // ImGui scales the glyphs with FontSizeBase but not the padding, spacing or
  // scrollbars, so the whole style is scaled with the font; otherwise every row
  // stays cramped around the larger text.
  ImGuiStyle& style = ImGui::GetStyle();
  style.FontSizeBase = kUiFontSize;
  style.ScaleAllSizes(kUiFontSize / kBuiltInFontSize);

  if (std::filesystem::exists(kUiFont)) {
    io.Fonts->AddFontFromFileTTF(kUiFont, kUiFontSize);
  } else {
    Logger::Warning(std::string("UI font ") + kUiFont + " is missing; falling back to the built-in font.");
    io.Fonts->AddFontDefault();
  }
}

void ImGuiLayer::Shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  s_window = nullptr;
}

void ImGuiLayer::BeginFrame() {
  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::EndFrame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_SwapWindow(s_window);
}

void ImGuiLayer::ProcessEvent(const SDL_Event& event) { ImGui_ImplSDL3_ProcessEvent(&event); }
}  // namespace rse
