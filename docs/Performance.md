# Performance

Optimisation work in this project is judged against numbers, not impressions.
This file describes how the numbers are taken and records them, so a change can
be compared with the state it replaced.

## Tooling

The editor's **Performance** panel (`View > Performance`) shows:

| Readout | Source | Meaning |
| --- | --- | --- |
| `Frame` | `Core::GetFrameStats().delta_ms` | Wall time of the last frame, in milliseconds. |
| `FPS` | `Core::GetFrameStats().fps` | Frames per second, averaged over the last whole second. The first second of a run says `measuring`. |
| `Sprites` | `Renderer2DStats::submitted` | Quads submitted this frame, before culling. |
| `Culled` | `Renderer2DStats::culled` | Quads rejected by the view bounds. |
| `Batches` | `Renderer2DStats::batches` | Batches the frame was split into. |
| `Draw calls` | `Renderer2DStats::draw_calls` | Draw calls issued. |
| `Vertices` / `Indices` | `Renderer2DStats::vertices` / `indices` | Buffer sizes built this frame. |
| `Renderer` | `Renderer2DStats::cpu_ms` | CPU time from `Begin()` to the end of `Flush()`: submit, sort, cull, vertex build, upload, draw issue. |

The panel also owns the **benchmark load**: a synthetic grid of untextured
quads submitted straight to the renderer, up to 100 000 of them. The grid is
built to fill the default view (1280x720 world units centred on 640,360), so
nothing is culled, and it never touches the scene -- a measurement cannot dirty,
save or undo anything.

## Taking a baseline

1. Build the preset you want to compare: `x64-release` for shipped behaviour,
   `x64-debug` when you need assertions. Never compare across presets.
2. Open `SampleProject` in the editor.
3. Open `View > Performance`. Leave the benchmark off first and let the run
   settle for a few seconds; note `Frame`.
4. Tick **Synthetic sprites** and set the count (10 000 is the default). Wait a
   few seconds for the FPS average to settle.
5. Record `Frame`, `FPS`, and every renderer counter in the table below.
6. Repeat at 50 000 to see how the curves diverge. A fix that helps small
   counts but not large ones is usually a draw-call or upload change; the
   reverse is usually CPU-side batching.

Close any other GPU application first. Frame time is what matters; `FPS` is the
same number inverted, and it is reported because it is what people quote.

## Baseline

Recorded before the performance work began. VSync is off in this build
(`SDL_GL_SetSwapInterval(0)`), so the frame rate is uncapped and the numbers
describe the engine's own cost rather than the display's refresh rate.

| Date | Commit | Preset | Sprites | Frame (ms) | FPS | Submitted | Culled | Batches | Draw calls | Vertices |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| _pending_ | `ad8a9ff` | `x64-release` | 10 000 | | | | | | | |

## Targets

The renderer work is done when, on the baseline scene above:

- frame time improves by at least 3x at 10 000 sprites,
- draw calls fall by at least 5x once sprites can share draw calls across
  textures,
- no frame exceeds 16.6 ms on the sample project with the benchmark off.

## Limits of the benchmark

- **One texture.** The quads are untextured, so the frame lands in a single
  batch and draw calls stay at 1. Multi-texture batching gains will not show
  here; they need a scene with many distinct, non-atlased textures.
- **No ECS.** The grid bypasses the registry on purpose. Per-entity iteration,
  component lookups and material resolution are not in these numbers.
- **Uncapped.** Without VSync a frame can finish far faster than a display
  could present it, so frame time is the comparable figure, not FPS.
- **Warm-up.** The first second after enabling the load includes the sprite
  grid rebuild; ignore it.
