#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace rse {
class Shader;  // forward declaration for Shader

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific
// input methods
enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for
// use in OpenGL
class Camera {
 public:
  Camera(glm::vec3 position);
  Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);
  Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float yaw, float pitch);

  void Setup();
  void Update(float delta_time);
  void Render(Shader* shader);
  glm::mat4 GetViewMatrix();
  void ProcessKeyboard(CameraMovement direction, float delta_time);
  void ProcessMouseMovement(float x_offset, float y_offset, bool constrain_pitch = true);
  void ProcessMouseScroll(float y_offset);

  inline const float GetZoom() const { return m_zoom; }
  inline const glm::vec3 GetPosition() const { return m_position; }
  inline const glm::vec3 GetFront() const { return m_front; }

 private:
  void UpdateCameraVectors();
  void UpdateOrthoProjection();

  glm::vec3 m_position{glm::vec3(0.0f, 0.0f, 0.0f)};
  glm::vec3 m_world_up{glm::vec3(0.0f, 1.0f, 0.0f)};
  glm::vec3 m_front{glm::vec3(0.0f, 0.0f, -1.0f)};
  glm::vec3 m_up;
  glm::vec3 m_right;
  glm::mat4 m_projection;
  float m_yaw{-90.f}, m_pitch{0.f};
  float m_movement_speed{10.f}, m_mouse_sensitivity{0.25f}, m_zoom{45.f};

  // 2D ortho framing. referenceSize is what a zoom of 1 shows on a 16:9
  // target and is what the renderer has always framed; targetSize is
  // whatever is being drawn into this frame, refreshed in Render(). Going
  // through rse::makeOrthoView is what makes the engine camera and the
  // editor camera agree on the shape of the view -- and the reason a
  // resized window now shows more world instead of stretching it.
  glm::vec2 m_reference_size{1280.0f, 720.0f};
  glm::vec2 m_target_size{1280.0f, 720.0f};
};
}  // namespace rse