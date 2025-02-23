#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


PerspectiveCamera::PerspectiveCamera(float width, float height)
	:m_ScreenWidth(width), m_ScreenHeight(height), 
	m_FOV(45.0f), m_ProjectionMatrix(glm::perspective(glm::radians(m_FOV), width / height, 0.1f, 100.0f)), m_ViewMatrix(1.0f),
	m_NearPlane(0.1f), m_FarPlane(1000.0f), m_LastX(width * 0.5f), m_LastY(height * 0.5f)
{
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void PerspectiveCamera::SetProjectionMatrix(float width, float height)
{
	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), width / height, m_NearPlane, m_FarPlane);
}

void PerspectiveCamera::OnUpdate(GLFWwindow* window)
{
	float currentFrame = static_cast<float>(glfwGetTime());
	m_deltaTime = currentFrame - m_lastFrame;
	m_lastFrame = currentFrame;

	//TODO: Make a Input Abstraction Class
	const float CameraSpeed = 2.5f * m_deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		m_CameraPos += m_CameraFront * CameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		m_CameraPos -= m_CameraFront * CameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		m_CameraPos -= glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * CameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		m_CameraPos += glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * CameraSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		m_CameraPos += m_CameraUp * CameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		m_CameraPos -= m_CameraUp * CameraSpeed;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		m_FirstMouse = true;
	}

	LookAt();
}

void PerspectiveCamera::SetCameraAngles(GLFWwindow* window, double xPos, double yPos)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
		return;
	}

	float xpos = static_cast<float>(xPos);
	float ypos = static_cast<float>(yPos);

	if (m_FirstMouse)
	{
		m_LastX = xpos;
		m_LastY = ypos;
		m_FirstMouse = false;
		return;
	}

	float xOffset = xpos - m_LastX;
	float yOffset = m_LastY - ypos;
	m_LastX = xpos;
	m_LastY = ypos;

	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	m_Yaw += xOffset;
	m_Pitch += yOffset;

	// Constrain pitch
	m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

	// Calculate new front vector
	glm::vec3 direction;
	direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	direction.y = sin(glm::radians(m_Pitch));
	direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_CameraFront = glm::normalize(direction);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void PerspectiveCamera::LookAt()
{
	m_ViewMatrix = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
}
