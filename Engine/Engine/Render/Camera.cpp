#include "Camera.hpp"

#include <algorithm>
#include <cmath>

#ifdef EMSCRIPTEN
	#define GL_GLEXT_PROTOTYPES
	#include <SDL3/SDL_opengles2.h>
#else
	#include <GL/glew.h>
#endif // EMSCRIPTEN

#include "OrthoView.hpp"
#include "../Components/Shader.hpp"
#include "../Input/Keyboard.hpp"
#include "../Input/Mouse.hpp"

namespace rse
{
	Camera::Camera(glm::vec3 position)
		: m_Position(position)
	{
		setup();
	}

	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
		: m_Position(position), m_WorldUp(up), m_Yaw(yaw), m_Pitch(pitch)
	{
		setup();
	}

	Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
		: m_Position(glm::vec3(posX, posY, posZ)), m_WorldUp(glm::vec3(upX, upY, upZ)), m_Yaw(yaw), m_Pitch(pitch)
	{
		setup();
	}

	void Camera::setup()
	{
		//TODO Need to fix the hard coded values for the camera
		this->m_Front = glm::normalize(m_Front);
		this->m_Right = glm::normalize(glm::cross(this->m_Front, this->m_WorldUp));
		this->m_Up = glm::normalize(glm::cross(this->m_Right, this->m_Front));
		//this->updateCameraVectors();

		//TODO Implement perspective support
		//TODO Add support for multiple resolutions
		updateOrthoProjection();
	}

	void Camera::updateOrthoProjection()
	{
		// Centred on the reference rectangle's middle, so the default framing is
		// exactly what it always was: (0,0)-(1280,720) at zoom 1.
		const OrthoView view = makeOrthoView(m_ReferenceSize * 0.5f, 1.0f, m_TargetSize, m_ReferenceSize);
		m_Projection = makeOrthoProjection(view);
	}

	void Camera::Update(float deltaTime)
	{
		using namespace rse;

		if (Keyboard::IsKeyPressed(SDL_SCANCODE_UP))
		{
			this->ProcessKeyboard(FORWARD, deltaTime);
		}

		if (Keyboard::IsKeyPressed(SDL_SCANCODE_DOWN))
		{
			this->ProcessKeyboard(BACKWARD, deltaTime);
		}

		if (Keyboard::IsKeyPressed(SDL_SCANCODE_LEFT))
		{
			this->ProcessKeyboard(LEFT, deltaTime);
		}

		if (Keyboard::IsKeyPressed(SDL_SCANCODE_RIGHT))
		{
			this->ProcessKeyboard(RIGHT, deltaTime);
		}

		double xPos = Mouse::GetMouseX();
		double yPos = Mouse::GetMouseY();

		//if (firstMouse)
		//{
		//	lastX = xPos;
		//	lastY = yPos;
		//	firstMouse = false;
		//}

		//float xOffset = xPos - lastX;
		//float yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

		//lastX = xPos;
		//lastY = yPos;

		this->ProcessMouseMovement(static_cast<float>(xPos), static_cast<float>(yPos));
	}

	void Camera::Render(Shader* shader)
	{
		// Size the projection to the target that is actually bound. The window is
		// resizable, and an ortho rectangle of the wrong shape is exactly what
		// stretches the scene when it is.
		int viewport[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GL_VIEWPORT, viewport);

		if (viewport[2] > 0 && viewport[3] > 0)
		{
			const glm::vec2 targetSize(static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
			if (targetSize != m_TargetSize)
			{
				m_TargetSize = targetSize;
				updateOrthoProjection();
			}
		}

		glm::mat4 view = GetViewMatrix();
		shader->setUniformMat4("view", glm::value_ptr(view));
		shader->setUniformMat4("projection", glm::value_ptr(m_Projection));
	}

	glm::mat4 Camera::GetViewMatrix()
	{
		return glm::lookAt(this->m_Position, this->m_Position + this->m_Front, this->m_Up);
	}

	void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = m_MovementSpeed * deltaTime;

		if (direction == FORWARD)
		{
			m_Position += m_Front * velocity;
		}

		if (direction == BACKWARD)
		{
			m_Position -= m_Front * velocity;
		}

		if (direction == LEFT)
		{
			m_Position -= m_Right * velocity;
		}

		if (direction == RIGHT)
		{
			m_Position += m_Right * velocity;
		}
	}

	void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
	{
		xOffset *= this->m_MouseSensitivity;
		yOffset *= this->m_MouseSensitivity;

		this->m_Yaw += xOffset;
		this->m_Pitch += yOffset;

		if (constrainPitch)
		{
			if (this->m_Pitch > 89.0f)
			{
				this->m_Pitch = 89.0f;
			}

			if (this->m_Pitch < -89.0f)
			{
				this->m_Pitch = -89.0f;
			}
		}

		//this->updateCameraVectors();
	}

	void Camera::ProcessMouseScroll(float yOffset)
	{

	}

	void Camera::updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(this->m_Yaw)) * cos(glm::radians(this->m_Pitch));
		front.y = sin(glm::radians(this->m_Pitch));
		front.z = sin(glm::radians(this->m_Yaw)) * cos(glm::radians(this->m_Pitch));
		this->m_Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		this->m_Right = glm::normalize(glm::cross(this->m_Front, this->m_WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->m_Up = glm::normalize(glm::cross(this->m_Right, this->m_Front));
	}
}