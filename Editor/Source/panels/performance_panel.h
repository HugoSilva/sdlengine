#pragma once

#include <core.h>
#include <cstddef>
#include <imgui.h>

#include <layout/editor_panel.h>
#include <render/renderer_2d.h>
#include <systems/benchmark_render_system.h>

// ---------------------------------------------------------------------------
// Frame timing and renderer statistics, plus the switch that loads the renderer
// with a synthetic grid of sprites.
//
// The panel exists so every performance change in this project is measured the
// same way: one frame time, one set of renderer counters, one reproducible
// load. The numbers come from Core and Renderer2D -- never from ImGui's own
// framerate -- so the readout describes the engine rather than the UI on top
// of it.
// ---------------------------------------------------------------------------
class PerformancePanel : public EditorPanel {
 public:
  explicit PerformancePanel(BenchmarkRenderSystem* benchmark) : m_benchmark(benchmark) {}

  const char* Id() const override { return "Performance"; }
  DockSlot Slot() const override { return DockSlot::kRight; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    DrawFrame();
    ImGui::Separator();
    DrawRenderer();
    ImGui::Separator();
    DrawBenchmark();

    ImGui::End();
  }

 private:
  void DrawFrame() const {
    const Core::FrameStats frame = Core::GetFrameStats();

    ImGui::Text("Frame %.3f ms", static_cast<double>(frame.delta_ms));

    // The counter is averaged over a whole second, so the first second of a run
    // has no number to show yet rather than a misleading one.
    if (frame.fps > 0.0f) {
      ImGui::Text("FPS   %.1f", static_cast<double>(frame.fps));
    } else {
      ImGui::TextDisabled("FPS   measuring");
    }
  }

  void DrawRenderer() const {
    rse::Renderer2D* renderer = Core::GetRenderer();

    if (renderer == nullptr) {
      ImGui::TextDisabled("No renderer");
      return;
    }

    const rse::Renderer2DStats stats = renderer->GetStats();

    ImGui::Text("Sprites    %zu", stats.submitted);
    ImGui::Text("Culled     %zu", stats.culled);
    ImGui::Text("Batches    %zu", stats.batches);
    ImGui::Text("Draw calls %zu", stats.draw_calls);
    ImGui::Text("Vertices   %zu", stats.vertices);
    ImGui::Text("Indices    %zu", stats.indices);
    ImGui::Text("Renderer   %.3f ms", stats.cpu_ms);
  }

  void DrawBenchmark() {
    if (m_benchmark == nullptr) {
      ImGui::TextDisabled("No benchmark system");
      return;
    }

    ImGui::TextUnformatted("Benchmark load");

    bool enabled = m_benchmark->IsEnabled();
    if (ImGui::Checkbox("Synthetic sprites", &enabled)) {
      m_benchmark->SetEnabled(enabled);
    }

    if (ImGui::SliderInt("##benchmark_count", &m_benchmark_count, 100, 100000, "%d sprites",
                         ImGuiSliderFlags_Logarithmic)) {
      m_benchmark->SetCount(static_cast<std::size_t>(m_benchmark_count));
    }

    ImGui::TextDisabled("%zu submitted by the benchmark", m_benchmark->GetCount());
  }

  BenchmarkRenderSystem* m_benchmark{nullptr};
  int m_benchmark_count{10000};
};
