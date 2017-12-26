#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
{
	this->position = position;
	this->worldUp = up;
	this->yaw = yaw;
	this->pitch = pitch;
	this->updateCameraVectors();
	
	//projection = glm::perspective(GetZoom(), 1280.0f / 720.0f, 0.1f, 100.0f);
	projection = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);
}

Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
{
	this->position = glm::vec3(posX, posY, posZ);
	this->worldUp = glm::vec3(upX, upY, upZ);
	this->yaw = yaw;
	this->pitch = pitch;
	this->updateCameraVectors();
	
	//projection = glm::perspective(GetZoom(), 1280.0f / 720.0f, 0.1f, 100.0f);
	projection = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);
}

void Camera::Update(float deltaTime)
{
	if (Keyboard::Key(SDL_SCANCODE_UP))
	{
		this->ProcessKeyboard(FORWARD, deltaTime);
	}

	if (Keyboard::Key(SDL_SCANCODE_DOWN))
	{
		this->ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (Keyboard::Key(SDL_SCANCODE_LEFT))
	{
		this->ProcessKeyboard(LEFT, deltaTime);
	}

	if (Keyboard::Key(SDL_SCANCODE_RIGHT))
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

	//GLfloat xOffset = xPos - lastX;
	//GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	//lastX = xPos;
	//lastY = yPos;

	this->ProcessMouseMovement(xPos, yPos);
}

void Camera::Render(Shader shader)
{
	glm::mat4 view = GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(this->position, this->position + this->front, this->up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
{
	GLfloat velocity = this->movementSpeed * deltaTime;

	if (direction == FORWARD)
	{
		this->position += this->front * velocity;
	}

	if (direction == BACKWARD)
	{
		this->position -= this->front * velocity;
	}

	if (direction == LEFT)
	{
		this->position -= this->right * velocity;
	}

	if (direction == RIGHT)
	{
		this->position += this->right * velocity;
	}
}

void Camera::ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch)
{
	xOffset *= this->mouseSensitivity;
	yOffset *= this->mouseSensitivity;

	this->yaw += xOffset;
	this->pitch += yOffset;

	if (constrainPitch)
	{
		if (this->pitch > 89.0f)
		{
			this->pitch = 89.0f;
		}

		if (this->pitch < -89.0f)
		{
			this->pitch = -89.0f;
		}
	}

	this->updateCameraVectors();
}

void Camera::ProcessMouseScroll(GLfloat yOffset)
{

}

GLfloat Camera::GetZoom()
{
	return this->zoom;
}

glm::vec3 Camera::GetPosition()
{
	return this->position;
}

glm::vec3 Camera::GetFront()
{
	return this->front;
}

void Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	front.y = sin(glm::radians(this->pitch));
	front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	this->front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	this->right = glm::normalize(glm::cross(this->front, this->worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	this->up = glm::normalize(glm::cross(this->right, this->front));
}