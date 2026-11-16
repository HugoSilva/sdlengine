#pragma once

#include <algorithm>
#include <cmath>

namespace rse {
// Turns wall-clock frame time into whole simulation steps.
//
// A simulation stepped by the frame's own delta time changes with the frame
// rate: the same jump covers more ground at 30 frames per second than at 144,
// and one long frame can push a body straight through a wall. Stepping by a
// constant amount of time instead makes the result depend on elapsed time
// alone; this class is what decides how many of those steps a frame owes.
//
// Whatever time is left over at the end of a frame is exposed as Alpha(), which
// render systems use to draw between the last two steps. Without it, a 60 Hz
// simulation on a 144 Hz display would visibly move in 60 bursts a second.
class FixedTimestep {
 public:
  // 60 Hz is the rate Box2D is tuned for, and it makes one step exactly one
  // sixtieth of a second of simulated time whatever the frame rate is.
  static constexpr float kDefaultStep{1.0f / 60.0f};

  // A frame that took longer than this many steps is treated as a stall: the
  // extra time is dropped rather than simulated. Without the cap a hitch makes
  // the next frame simulate more, which makes it hitch longer -- the death
  // spiral this cap exists to break.
  static constexpr int kDefaultMaxSteps{5};

  explicit FixedTimestep(float step = kDefaultStep, int max_steps = kDefaultMaxSteps)
      : m_step(step > 0.0f ? step : kDefaultStep), m_max_steps(max_steps > 0 ? max_steps : 1) {}

  // Adds one frame's elapsed time and returns how many whole steps to run now.
  //
  // A delta that is zero, negative or not finite contributes nothing: a clock
  // that went backwards, or a frame that never happened, is not a step.
  int Advance(float delta_time) {
    if (!std::isfinite(delta_time) || delta_time <= 0.0f) {
      return 0;
    }

    // Clamped on the way in as well as on the way out, so a stall cannot leave
    // a backlog that takes several frames to chew through afterwards.
    m_accumulator += std::min(delta_time, m_step * static_cast<float>(m_max_steps));

    int steps = static_cast<int>(m_accumulator / m_step);

    if (steps > m_max_steps) {
      steps = m_max_steps;
    }

    m_accumulator -= static_cast<float>(steps) * m_step;

    // The subtraction can land a hair below zero when the division rounded up.
    if (m_accumulator < 0.0f) {
      m_accumulator = 0.0f;
    }

    return steps;
  }

  // The simulated time one step advances, in seconds.
  float Step() const { return m_step; }

  // How far the frame is into the next step, in [0, 1]. This is what a render
  // system mixes the previous step's transform into the current one by.
  float Alpha() const { return std::clamp(m_accumulator / m_step, 0.0f, 1.0f); }

  // Drops the time banked so far. A scene that is reloaded or restarted has
  // nothing pending: replaying the old remainder would step the new scene.
  void Reset() { m_accumulator = 0.0f; }

 private:
  float m_step;
  int m_max_steps;
  float m_accumulator{0.0f};
};
}  // namespace rse
