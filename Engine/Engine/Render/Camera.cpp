#include "Camera.hpp"

#include <algorithm>
#include <cmath>

#ifdef EMSCRIPTEN
#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

#include "../Components/Shader.hpp"
#include "../Input/Keyboard.hpp"
#include "../Input/Mouse.hpp"
#include "OrthoView.hpp"

namespace rse {
Camera::Camera(glm::vec3 position) : m_position(position) { Setup(); }

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_position(position), m_world_up(up), m_yaw(yaw), m_pitch(pitch) {
  Setup();
}

Camera::Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float yaw, float pitch)
    : m_position(glm::vec3(pos_x, pos_y, pos_z)), m_world_up(glm::vec3(up_x, up_y, up_z)), m_yaw(yaw), m_pitch(pitch) {
  Setup();
}

void Camera::Setup() {
  // TODO Need to fix the hard coded values for the camera
  this->m_front = glm::normalize(m_front);
  this->m_right = glm::normalize(glm::cross(this->m_front, this->m_world_up));
  this->m_up = glm::normalize(glm::cross(this->m_right, this->m_front));
  // this->updateCameraVectors();

  // TODO Implement perspective support
  // TODO Add support for multiple resolutions
  UpdateOrthoProjection();
}

void Camera::UpdateOrthoProjection() {
  // Centred on the reference rectangle's middle, so the default framing is
  // exactly what it always was: (0,0)-(1280,720) at zoom 1.
  const OrthoView view = MakeOrthoView(m_reference_size * 0.5f, 1.0f, m_target_size, m_reference_size);
  m_projection = MakeOrthoProjection(view);
}

void Camera::Update(float delta_time) {
  using namespace rse;

  if (Keyboard::IsKeyPressed(SDL_SCANCODE_UP)) {
    this->ProcessKeyboard(FORWARD, delta_time);
  }

  if (Keyboard::IsKeyPressed(SDL_SCANCODE_DOWN)) {
    this->ProcessKeyboard(BACKWARD, delta_time);
  }

  if (Keyboard::IsKeyPressed(SDL_SCANCODE_LEFT)) {
    this->ProcessKeyboard(LEFT, delta_time);
  }

  if (Keyboard::IsKeyPressed(SDL_SCANCODE_RIGHT)) {
    this->ProcessKeyboard(RIGHT, delta_time);
  }

  double x_pos = Mouse::GetMouseX();
  double y_pos = Mouse::GetMouseY();

  // if (firstMouse)
  //{
  //	lastX = xPos;
  //	lastY = yPos;
  //	firstMouse = false;
  // }

  // float xOffset = xPos - lastX;
  // float yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

  // lastX = xPos;
  // lastY = yPos;

  this->ProcessMouseMovement(static_cast<float>(x_pos), static_cast<float>(y_pos));
}

void Camera::Render(Shader* shader) {
  // Size the projection to the target that is actually bound. The window is
  // resizable, and an ortho rectangle of the wrong shape is exactly what
  // stretches the scene when it is.
  int viewport[4] = {0, 0, 0, 0};
  glGetIntegerv(GL_VIEWPORT, viewport);

  if (viewport[2] > 0 && viewport[3] > 0) {
    const glm::vec2 target_size(static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
    if (target_size != m_target_size) {
      m_target_size = target_size;
      UpdateOrthoProjection();
    }
  }

  glm::mat4 view = GetViewMatrix();
  shader->SetUniformMat4("view", glm::value_ptr(view));
  shader->SetUniformMat4("projection", glm::value_ptr(m_projection));
}

glm::mat4 Camera::GetViewMatrix() {
  return glm::lookAt(this->m_position, this->m_position + this->m_front, this->m_up);
}

void Camera::ProcessKeyboard(CameraMovement direction, float delta_time) {
  float velocity = m_movement_speed * delta_time;

  if (direction == FORWARD) {
    m_position += m_front * velocity;
  }

  if (direction == BACKWARD) {
    m_position -= m_front * velocity;
  }

  if (direction == LEFT) {
    m_position -= m_right * velocity;
  }

  if (direction == RIGHT) {
    m_position += m_right * velocity;
  }
}

void Camera::ProcessMouseMovement(float x_offset, float y_offset, bool constrain_pitch) {
  x_offset *= this->m_mouse_sensitivity;
  y_offset *= this->m_mouse_sensitivity;

  this->m_yaw += x_offset;
  this->m_pitch += y_offset;

  if (constrain_pitch) {
    if (this->m_pitch > 89.0f) {
      this->m_pitch = 89.0f;
    }

    if (this->m_pitch < -89.0f) {
      this->m_pitch = -89.0f;
    }
  }

  // this->updateCameraVectors();
}

void Camera::ProcessMouseScroll(float y_offset) {}

void Camera::UpdateCameraVectors() {
  // Calculate the new Front vector
  glm::vec3 front;
  front.x = cos(glm::radians(this->m_yaw)) * cos(glm::radians(this->m_pitch));
  front.y = sin(glm::radians(this->m_pitch));
  front.z = sin(glm::radians(this->m_yaw)) * cos(glm::radians(this->m_pitch));
  this->m_front = glm::normalize(front);
  // Also re-calculate the Right and Up vector
  this->m_right = glm::normalize(
      glm::cross(this->m_front, this->m_world_up));  // Normalize the vectors, because their length gets closer to 0 the
                                                     // more you look up or down which results in slower movement.
  this->m_up = glm::normalize(glm::cross(this->m_right, this->m_front));
}
}  // namespace rse