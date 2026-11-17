#pragma once

#include <glm/glm.hpp>
#include <gtest/gtest.h>

#include <render/camera.h>

/*
The engine camera frames the scene with an ortho projection. What a scene
stores is where that frame sits and how far in it is zoomed, so these tests pin
the two things a scene camera needs from it: moving the position moves the
world point in the middle of the frame, and the zoom changes how much of the
reference rectangle is shown.
*/
namespace {
using namespace rse;

class CameraTest : public ::testing::Test {
 protected:
  // The view-projection a world point ends up with, in clip space.
  static glm::vec4 Project(Camera& camera, const glm::vec2& world) {
    return camera.GetProjection() * camera.GetViewMatrix() * glm::vec4(world.x, world.y, 0.0f, 1.0f);
  }
};

TEST_F(CameraTest, TheDefaultCameraFramesTheReferenceRectangle) {
  Camera camera{glm::vec3(640.0f, 360.0f, 0.0f)};

  EXPECT_FLOAT_EQ(camera.GetPosition().x, 640.0f);
  EXPECT_FLOAT_EQ(camera.GetPosition().y, 360.0f);
  EXPECT_FLOAT_EQ(camera.GetZoom(), 1.0f);

  // A 1280x720 world rectangle fills a 1280x720 frame at zoom 1.
  const glm::vec4 bottom_left = Project(camera, glm::vec2(0.0f, 0.0f));
  const glm::vec4 top_right = Project(camera, glm::vec2(1280.0f, 720.0f));

  EXPECT_NEAR(bottom_left.x, -1.0f, 1e-4f);
  EXPECT_NEAR(bottom_left.y, -1.0f, 1e-4f);
  EXPECT_NEAR(top_right.x, 1.0f, 1e-4f);
  EXPECT_NEAR(top_right.y, 1.0f, 1e-4f);
}

TEST_F(CameraTest, SetPositionMovesTheWorldPointInTheMiddle) {
  Camera camera{glm::vec3(640.0f, 360.0f, 0.0f)};
  camera.SetPosition(glm::vec3(100.0f, 200.0f, 0.0f));

  EXPECT_FLOAT_EQ(camera.GetPosition().x, 100.0f);
  EXPECT_FLOAT_EQ(camera.GetPosition().y, 200.0f);

  const glm::vec4 centre = Project(camera, glm::vec2(100.0f, 200.0f));
  EXPECT_NEAR(centre.x, 0.0f, 1e-4f);
  EXPECT_NEAR(centre.y, 0.0f, 1e-4f);

  // Zoom 1 still spans the whole reference rectangle around the new centre.
  const glm::vec4 bottom_left = Project(camera, glm::vec2(-540.0f, -160.0f));
  EXPECT_NEAR(bottom_left.x, -1.0f, 1e-4f);
  EXPECT_NEAR(bottom_left.y, -1.0f, 1e-4f);
}

TEST_F(CameraTest, SetZoomShowsLessWorld) {
  Camera camera{glm::vec3(640.0f, 360.0f, 0.0f)};
  camera.SetZoom(2.0f);

  EXPECT_FLOAT_EQ(camera.GetZoom(), 2.0f);

  // Half the reference rectangle now fills the frame.
  const glm::vec4 bottom_left = Project(camera, glm::vec2(320.0f, 180.0f));
  const glm::vec4 top_right = Project(camera, glm::vec2(960.0f, 540.0f));

  EXPECT_NEAR(bottom_left.x, -1.0f, 1e-4f);
  EXPECT_NEAR(bottom_left.y, -1.0f, 1e-4f);
  EXPECT_NEAR(top_right.x, 1.0f, 1e-4f);
  EXPECT_NEAR(top_right.y, 1.0f, 1e-4f);
}

TEST_F(CameraTest, TheProjectionTakesTheShapeOfTheTarget) {
  Camera camera{glm::vec3(640.0f, 360.0f, 0.0f)};

  // At zoom 1 the frame is 720 world units tall whatever the target, so a 4:3
  // target is 960 wide: 480 units either side of the centre, not 640. This is
  // the resize behaviour: more world, not stretched world.
  camera.SetTargetSize(glm::ivec2{800, 600});

  const glm::vec4 right_edge = Project(camera, glm::vec2(640.0f + 480.0f, 360.0f));
  EXPECT_NEAR(right_edge.x, 1.0f, 1e-4f);

  const glm::vec4 top_edge = Project(camera, glm::vec2(640.0f, 360.0f + 360.0f));
  EXPECT_NEAR(top_edge.y, 1.0f, 1e-4f);
}

TEST_F(CameraTest, ATargetWithNoSizeKeepsTheFraming) {
  Camera camera{glm::vec3(640.0f, 360.0f, 0.0f)};

  // Nothing has been bound yet, so the camera has no shape to adopt. Collapsing
  // the projection to a zero rectangle would draw nothing at all.
  camera.SetTargetSize(glm::ivec2{0, 0});
  camera.SetTargetSize(glm::ivec2{-4, 600});

  const glm::vec4 top_right = Project(camera, glm::vec2(1280.0f, 720.0f));
  EXPECT_NEAR(top_right.x, 1.0f, 1e-4f);
  EXPECT_NEAR(top_right.y, 1.0f, 1e-4f);
}

TEST_F(CameraTest, MovingTheCameraLeavesTheProjectionAlone) {
  Camera camera{glm::vec3(640.0f, 360.0f, 0.0f)};
  const glm::mat4 before = camera.GetProjection();

  camera.SetPosition(glm::vec3(100.0f, 200.0f, 0.0f));

  // The projection is centred at the origin and the view carries the position,
  // so walking used to re-derive this matrix for nothing, every frame.
  EXPECT_TRUE(camera.GetProjection() == before);
}

TEST_F(CameraTest, MovingWithTheKeyboardMovesTheView) {
  Camera camera{glm::vec3(640.0f, 360.0f, 0.0f)};
  const glm::mat4 at_start = camera.GetViewMatrix();

  camera.ProcessKeyboard(kRight, 1.0f);

  EXPECT_FALSE(camera.GetViewMatrix() == at_start);

  // The camera moved +x, so what used to be the middle of the frame is now to
  // its left.
  const glm::vec4 centre = Project(camera, glm::vec2(640.0f, 360.0f));
  EXPECT_LT(centre.x, 0.0f);
}
}  // namespace
