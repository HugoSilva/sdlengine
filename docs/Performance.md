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

## Baseline and result

`ad8a9ff` is the state before the renderer work; `c799535` is the same load after
it. VSync was hardcoded off in the baseline build, and the after rows were taken
with VSync off and the frame cap at 0 in **Edit > Project Settings > Frame
pacing**, so both describe the engine rather than the display.

Each row is the average of a three-second window after a two-second warm-up, on
the same machine and the same `x64-release` preset. The load was driven at
startup from a temporary build rather than clicked in the panel: it enables the
same grid, submits through the same renderer and reads the same counters, but
leaves the editor UI out of the frame time. The submitted count includes the
sample scene's twelve sprites.

| Date | Commit | Preset | Sprites | Frame (ms) | FPS | Submitted | Culled | Batches | Draw calls | Vertices |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2026-11-16 | `ad8a9ff` | `x64-release` | 10 000 | 1.44 | 694 | 10 012 | 0 | 2 | 2 | 40 048 |
| 2026-11-20 | `c799535` | `x64-release` | 10 000 | 0.45 | 2 233 | 10 012 | 0 | 1 | 1 | 40 048 |
| 2026-11-16 | `ad8a9ff` | `x64-release` | 50 000 | 6.43 | 156 | 50 012 | 0 | 5 | 5 | 200 048 |
| 2026-11-20 | `c799535` | `x64-release` | 50 000 | 2.00 | 500 | 50 012 | 0 | 4 | 4 | 200 048 |

## Result

- **Frame time: 3.2x faster at both counts.** 1.44 -> 0.45 ms at 10 000 (694 ->
  2 233 FPS) and 6.43 -> 2.00 ms at 50 000 (156 -> 500 FPS).
- **Draw calls: the texture win is not visible in this grid.** At 10 000 the
  untextured load went from 2 batches to 1, and at 50 000 from 5 to 4; both are
  bounded by the 16-bit vertex limit, not by textures. The five-fold target was
  measured on the sixteen-texture load described in PR #109: sixteen distinct
  texture pages fell from sixteen draw calls to one. PR #111 grows the atlas to
  as many pages as a project needs, so a project that outgrows one page keeps
  batching until it needs a seventeenth.
- **16.6 ms: met with room.** With the load off, the sample project is paced by
  the display; with it on, the worst case recorded here is 2.00 ms at 50 000
  sprites.
- **The last commit before this table** (`c799535`) checks whether the draw list
  is already ordered before sorting it. On the grid it always is, and skipping
  the sort moved the 10 000 frame from 0.53 to 0.45 ms -- the difference between
  2.7x and the 3.2x above.

## Targets

The renderer work is done when, on the baseline scene above:

- frame time improves by at least 3x at 10 000 sprites: **3.2x**;
- draw calls fall by at least 5x once sprites can share draw calls across
  textures: **16 -> 1** on the sixteen-texture load (the grid has no texture
  changes to remove);
- no frame exceeds 16.6 ms on the sample project with the benchmark off:
  **met**.

## Limits of the benchmark

- **One texture.** The quads are untextured, so texture changes never split a
  batch: draw calls are bounded by the batch vertex limit (1 at 10 000, 4 at
  50 000) and stay at that count before and after multi-texture batching. The
  16 -> 1 draw-call result needs a scene with many distinct, non-atlased
  textures.
- **No ECS.** The grid bypasses the registry on purpose. Per-entity iteration,
  component lookups and material resolution are not in these numbers.
- **Pacing.** With VSync on, a frame cannot finish faster than the display shows
  it, so frame time measures the display rather than the engine. Turn both VSync
  and the frame cap off in the project settings before recording numbers.
- **Warm-up.** The first second after enabling the load includes the sprite
  grid rebuild; ignore it. The recorded rows used two seconds.
