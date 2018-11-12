#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace rse
{
	class Shader; //forward declaration for Shader

	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum Camera_Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
	class Camera
	{
	public:

		Camera(glm::vec3 position);
		Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

		void setup();
		void Update(float deltaTime);
		void Render(Shader* shader);
		glm::mat4 GetViewMatrix();
		void ProcessKeyboard(Camera_Movement direction, float deltaTime);
		void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
		void ProcessMouseScroll(float yOffset);

		inline const float getZoom() const { return m_Zoom; }
		inline const glm::vec3 getPosition() const { return m_Position; }
		inline const glm::vec3 getFront() const { return m_Front; }

	private:
		void updateCameraVectors();

		glm::vec3 m_Position{ glm::vec3(0.0f, 0.0f, 0.0f) };
		glm::vec3 m_WorldUp{ glm::vec3(0.0f, 1.0f, 0.0f) };
		glm::vec3 m_Front{ glm::vec3(0.0f, 0.0f, -1.0f) };
		glm::vec3 m_Up;
		glm::vec3 m_Right;
		glm::mat4 m_Projection;
		float m_Yaw{ -90.f }, m_Pitch{ 0.f };
		float m_MovementSpeed{ 10.f }, m_MouseSensitivity{ 0.25f }, m_Zoom{ 45.f };
	};
}