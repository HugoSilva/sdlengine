#pragma once

#include <gtest/gtest.h>

#include <Render/OrthoView.hpp>

// ---------------------------------------------------------------------------
// The 2D view maths behind both cameras.
//
// The first two tests are the regression test for the bug this header was
// extracted to fix: a fixed 1280x720 ortho rectangle stretched onto a target of
// a different shape. They pin both the old behaviour (so the test would catch a
// revert) and the new one.
// ---------------------------------------------------------------------------
namespace {
using namespace rse;

constexpr glm::vec2 kReference{1280.0f, 720.0f};

// The docked viewport's real size on the dev machine, measured on screen.
constexpr glm::vec2 kDockedTarget{660.0f, 484.0f};

TEST(OrthoView, FixedRectangleDistortsASquareTarget) {
  // The behaviour before the extraction: half extent (640,360) whatever the
  // target shape. On a 660x484 target a 100x100 world square comes out
  // 1.35x taller than it is wide -- the distortion to keep caught.
  OrthoView legacy;
  legacy.center = kReference * 0.5f;
  legacy.halfExtent = kReference * 0.5f;

  const glm::vec2 min = worldToViewport(legacy, glm::vec2(-50.0f, -50.0f), kDockedTarget);
  const glm::vec2 max = worldToViewport(legacy, glm::vec2(50.0f, 50.0f), kDockedTarget);

  const float width = max.x - min.x;
  const float height = min.y - max.y;

  EXPECT_NEAR(width, 51.6f, 0.1f);
  EXPECT_NEAR(height, 67.2f, 0.1f);
  EXPECT_GT(height / width, 1.3f);
}

TEST(OrthoView, SquareWorldStaysSquareOnANonSixteenNineTarget) {
  const OrthoView view = makeOrthoView(glm::vec2(0.0f), 1.0f, kDockedTarget, kReference);

  const glm::vec2 min = worldToViewport(view, glm::vec2(-50.0f, -50.0f), kDockedTarget);
  const glm::vec2 max = worldToViewport(view, glm::vec2(50.0f, 50.0f), kDockedTarget);

  const float width = max.x - min.x;
  const float height = min.y - max.y;

  EXPECT_NEAR(width, height, 0.01f);
}

TEST(OrthoView, HeightIsTheReferenceHeightOverZoom) {
  const OrthoView view = makeOrthoView(glm::vec2(0.0f), 2.0f, kDockedTarget, kReference);

  EXPECT_FLOAT_EQ(view.size().y, 360.0f);
  EXPECT_NEAR(view.aspect(), kDockedTarget.x / kDockedTarget.y, 0.0001f);
}

TEST(OrthoView, SixteenNineTargetFramesExactlyWhatItAlwaysDid) {
  const OrthoView view = makeOrthoView(kReference * 0.5f, 1.0f, glm::vec2(1920.0f, 1080.0f), kReference);

  EXPECT_FLOAT_EQ(view.min().x, 0.0f);
  EXPECT_FLOAT_EQ(view.min().y, 0.0f);
  EXPECT_FLOAT_EQ(view.max().x, 1280.0f);
  EXPECT_FLOAT_EQ(view.max().y, 720.0f);
}

TEST(OrthoView, ZoomIsClamped) {
  EXPECT_FLOAT_EQ(makeOrthoView(glm::vec2(0.0f), 0.001f, kDockedTarget, kReference).size().y,
                  kReference.y / kOrthoMinZoom);
  EXPECT_FLOAT_EQ(makeOrthoView(glm::vec2(0.0f), 1000.0f, kDockedTarget, kReference).size().y,
                  kReference.y / kOrthoMaxZoom);
}

TEST(OrthoView, ProjectionMatchesTheViewRectangle) {
  const OrthoView view = makeOrthoView(glm::vec2(100.0f, 50.0f), 1.0f, kDockedTarget, kReference);
  const glm::mat4 projection = makeOrthoProjection(view);

  // glm::ortho puts the rectangle's edges at -1/+1 in clip space.
  const glm::vec4 corner = projection * glm::vec4(view.min(), 0.0f, 1.0f);
  const glm::vec4 opposite = projection * glm::vec4(view.max(), 0.0f, 1.0f);

  EXPECT_NEAR(corner.x, -1.0f, 0.0001f);
  EXPECT_NEAR(corner.y, -1.0f, 0.0001f);
  EXPECT_NEAR(opposite.x, 1.0f, 0.0001f);
  EXPECT_NEAR(opposite.y, 1.0f, 0.0001f);
}

TEST(OrthoView, ScreenAndWorldRoundTrip) {
  const OrthoView view = makeOrthoView(glm::vec2(100.0f, -20.0f), 3.0f, kDockedTarget, kReference);

  const glm::vec2 world{123.4f, -45.6f};
  const glm::vec2 pixel = worldToViewport(view, world, kDockedTarget);
  const glm::vec2 back = viewportToWorld(view, pixel, kDockedTarget);

  EXPECT_NEAR(back.x, world.x, 0.0001f);
  EXPECT_NEAR(back.y, world.y, 0.0001f);
}

TEST(OrthoView, ViewportCoordinatesAreYDownFromTheTopLeft) {
  const OrthoView view = makeOrthoView(glm::vec2(0.0f), 1.0f, kDockedTarget, kReference);

  const glm::vec2 bottomLeft = worldToViewport(view, view.min(), kDockedTarget);
  const glm::vec2 topRight = worldToViewport(view, view.max(), kDockedTarget);

  EXPECT_NEAR(bottomLeft.x, 0.0f, 0.0001f);
  EXPECT_NEAR(bottomLeft.y, kDockedTarget.y, 0.0001f);
  EXPECT_NEAR(topRight.x, kDockedTarget.x, 0.0001f);
  EXPECT_NEAR(topRight.y, 0.0f, 0.0001f);
}

TEST(OrthoView, FitToBoundsCentresAndLeavesPadding) {
  const OrthoView view = makeOrthoView(glm::vec2(0.0f), 1.0f, kDockedTarget, kReference);

  Bounds2D bounds;
  bounds.min = glm::vec2(-40.0f, 10.0f);
  bounds.max = glm::vec2(40.0f, 90.0f);

  const OrthoView fitted = fitToBounds(view, bounds, 0.5f);

  EXPECT_FLOAT_EQ(fitted.center.x, 0.0f);
  EXPECT_FLOAT_EQ(fitted.center.y, 50.0f);

  // The bounds are 80 tall, so 40 half extent plus half again as padding.
  EXPECT_NEAR(fitted.halfExtent.y, 60.0f, 0.0001f);
  EXPECT_NEAR(fitted.aspect(), view.aspect(), 0.0001f);
  EXPECT_GE(fitted.halfExtent.x, 40.0f);
}

TEST(OrthoView, FitToBoundsSurvivesADegenerateSelection) {
  const OrthoView view = makeOrthoView(glm::vec2(0.0f), 1.0f, kDockedTarget, kReference);

  Bounds2D point;
  point.min = glm::vec2(7.0f, -3.0f);
  point.max = point.min;

  const OrthoView fitted = fitToBounds(view, point, 0.1f);

  EXPECT_TRUE(std::isfinite(fitted.halfExtent.x));
  EXPECT_TRUE(std::isfinite(fitted.halfExtent.y));
  EXPECT_GT(fitted.halfExtent.x, 0.0f);
  EXPECT_FLOAT_EQ(fitted.center.x, 7.0f);
  EXPECT_FLOAT_EQ(fitted.center.y, -3.0f);
}

TEST(OrthoView, NiceStepKeepsTheSpacingAboveTheMinimum) {
  float previous = 0.0f;

  for (float worldPerPixel = 0.01f; worldPerPixel < 100.0f; worldPerPixel *= 1.1f) {
    const float step = niceStep(worldPerPixel, 12.0f);

    EXPECT_GE(step / worldPerPixel, 12.0f);
    EXPECT_GE(step, previous);
    previous = step;

    // Never more than 10x coarser than it had to be.
    EXPECT_LE(step / worldPerPixel, 120.0f);
  }
}

TEST(OrthoView, NiceStepUsesOneTwoFiveDecades) {
  EXPECT_NEAR(niceStep(1.0f, 12.0f), 20.0f, 0.001f);
  EXPECT_NEAR(niceStep(10.0f, 12.0f), 200.0f, 0.001f);
  EXPECT_NEAR(niceStep(0.1f, 12.0f), 2.0f, 0.001f);
}

TEST(OrthoView, GridStepStaysOnTheBaseLattice) {
  const float base = 32.0f;

  for (float worldPerPixel = 0.01f; worldPerPixel < 100.0f; worldPerPixel *= 1.37f) {
    const float step = gridStep(base, worldPerPixel, 12.0f);
    const float doublings = std::log2(step / base);

    // Never finer than one grid square, never anything but a whole
    // number of them, and always coarse enough to leave the requested
    // spacing between lines.
    EXPECT_GE(step, base);
    EXPECT_NEAR(doublings, std::round(doublings), 1e-4f);
    EXPECT_GE(step / worldPerPixel, 12.0f - 1e-3f);
  }
}

TEST(OrthoView, GridStepSurvivesANonsenseBase) {
  EXPECT_GT(gridStep(0.0f, 1.0f, 12.0f), 0.0f);
  EXPECT_GT(gridStep(-5.0f, 1.0f, 12.0f), 0.0f);
  EXPECT_GT(gridStep(32.0f, 0.0f, 12.0f), 0.0f);
}

TEST(OrthoView, PointInOrientedBoxIsAxisAlignedAtZeroRotation) {
  const glm::vec2 half{40.0f, 10.0f};

  EXPECT_TRUE(pointInOrientedBox(glm::vec2(0.0f, 0.0f), glm::vec2(0.0f), 0.0f, half));
  EXPECT_TRUE(pointInOrientedBox(glm::vec2(39.0f, 9.0f), glm::vec2(0.0f), 0.0f, half));
  EXPECT_TRUE(pointInOrientedBox(glm::vec2(40.0f, 10.0f), glm::vec2(0.0f), 0.0f, half));
  EXPECT_FALSE(pointInOrientedBox(glm::vec2(41.0f, 0.0f), glm::vec2(0.0f), 0.0f, half));
  EXPECT_FALSE(pointInOrientedBox(glm::vec2(0.0f, 11.0f), glm::vec2(0.0f), 0.0f, half));
}

TEST(OrthoView, PointInOrientedBoxTurnsWithTheBox) {
  // A 40x10 box rotated 90 degrees is a 10x40 box: the point that was
  // inside along x is now outside, and vice versa.
  const glm::vec2 half{40.0f, 10.0f};

  EXPECT_FALSE(pointInOrientedBox(glm::vec2(39.0f, 0.0f), glm::vec2(0.0f), 90.0f, half));
  EXPECT_TRUE(pointInOrientedBox(glm::vec2(0.0f, 39.0f), glm::vec2(0.0f), 90.0f, half));
  EXPECT_TRUE(pointInOrientedBox(glm::vec2(0.0f, -9.0f), glm::vec2(0.0f), -90.0f, half));
}

TEST(OrthoView, OrientedBoxCornersAgreeWithTheHitTest) {
  const glm::vec2 center{12.0f, -7.0f};
  const glm::vec2 half{30.0f, 8.0f};
  const float rotation = 37.0f;

  const std::array<glm::vec2, 4> corners = orientedBoxCorners(center, rotation, half);

  // Corners sit on the boundary, so a hair inside the corner is inside and
  // a hair outside it is not. (The two exactly-on-the-boundary directions
  // are not compared: rotating a point out and back is not bit exact.)
  for (const glm::vec2& corner : corners) {
    const glm::vec2 inward = center + (corner - center) * 0.995f;
    EXPECT_TRUE(pointInOrientedBox(inward, center, rotation, half));

    const glm::vec2 outward = glm::normalize(corner - center);
    EXPECT_FALSE(pointInOrientedBox(corner + outward * 0.75f, center, rotation, half));
  }

  // And the box's own middle is always inside.
  EXPECT_TRUE(pointInOrientedBox(center, center, rotation, half));
}

TEST(OrthoView, BoundsOfCornersBoxesARotatedRectangle) {
  // A 20x10 rectangle (half extents 10 and 5) turned 90 degrees on screen
  // is 10 wide and 20 tall, so the box around it has to swap too.
  const Bounds2D bounds = boundsOfCorners(orientedBoxCorners(glm::vec2(0.0f), 90.0f, glm::vec2(10.0f, 5.0f)));

  EXPECT_NEAR(bounds.min.x, -5.0f, 1e-4f);
  EXPECT_NEAR(bounds.max.x, 5.0f, 1e-4f);
  EXPECT_NEAR(bounds.min.y, -10.0f, 1e-4f);
  EXPECT_NEAR(bounds.max.y, 10.0f, 1e-4f);
}

TEST(OrthoView, BoundsOfCornersKeepsTheCentre) {
  const glm::vec2 center{100.0f, -50.0f};
  const Bounds2D bounds = boundsOfCorners(orientedBoxCorners(center, 0.0f, glm::vec2(3.0f, 7.0f)));

  EXPECT_NEAR(bounds.center().x, center.x, 1e-4f);
  EXPECT_NEAR(bounds.center().y, center.y, 1e-4f);
  EXPECT_NEAR(bounds.size().x, 6.0f, 1e-4f);
  EXPECT_NEAR(bounds.size().y, 14.0f, 1e-4f);
}

TEST(OrthoView, UnionOfContainsBothInputs) {
  const Bounds2D a{glm::vec2(-10.0f, -5.0f), glm::vec2(10.0f, 5.0f)};
  const Bounds2D b{glm::vec2(2.0f, -20.0f), glm::vec2(30.0f, -1.0f)};

  const Bounds2D both = unionOf(a, b);

  EXPECT_NEAR(both.min.x, -10.0f, 1e-4f);
  EXPECT_NEAR(both.min.y, -20.0f, 1e-4f);
  EXPECT_NEAR(both.max.x, 30.0f, 1e-4f);
  EXPECT_NEAR(both.max.y, 5.0f, 1e-4f);
}

TEST(OrthoView, PointInBoxIncludesTheEdges) {
  const glm::vec2 min{10.0f, 20.0f};
  const glm::vec2 max{30.0f, 40.0f};

  EXPECT_TRUE(pointInBox(glm::vec2(20.0f, 30.0f), min, max));
  EXPECT_TRUE(pointInBox(min, min, max));
  EXPECT_TRUE(pointInBox(max, min, max));
  EXPECT_FALSE(pointInBox(glm::vec2(9.0f, 30.0f), min, max));
  EXPECT_FALSE(pointInBox(glm::vec2(20.0f, 41.0f), min, max));
}

TEST(OrthoView, SnapToStepRoundsToTheNearestMultiple) {
  // A translate: the grid step is both what is drawn and what Shift snaps
  // to, so a position either lands on a lattice point or barely moves.
  EXPECT_NEAR(snapToStep(37.0f, 32.0f), 32.0f, 1e-4f);
  EXPECT_NEAR(snapToStep(49.0f, 32.0f), 64.0f, 1e-4f);
  EXPECT_NEAR(snapToStep(-20.0f, 32.0f), -32.0f, 1e-4f);
  EXPECT_NEAR(snapToStep(64.0f, 32.0f), 64.0f, 1e-4f);

  // A rotate: 7 degrees is closer to 0 than to 15, 8 is closer to 15.
  EXPECT_NEAR(snapToStep(7.0f, 15.0f), 0.0f, 1e-4f);
  EXPECT_NEAR(snapToStep(8.0f, 15.0f), 15.0f, 1e-4f);
}

TEST(OrthoView, SnapToStepLeavesAValueAloneWithoutAStep) {
  // A zero (or nonsense) step is how a caller passes "no snapping" down,
  // so it must not turn a position into a NaN or a zero.
  EXPECT_FLOAT_EQ(snapToStep(37.0f, 0.0f), 37.0f);
  EXPECT_FLOAT_EQ(snapToStep(-1.5f, -2.0f), -1.5f);
}
}  // namespace
