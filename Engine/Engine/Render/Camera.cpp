#include "Camera.hpp"

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
		//projection = glm::perspective(GetZoom(), 1280.0f / 720.0f, 0.1f, 100.0f);
		m_Projection = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);
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