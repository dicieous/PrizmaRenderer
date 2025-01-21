#pragma once
//TODO: Remove GLFW form Camera Class
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class PerspectiveCamera {

public:
	PerspectiveCamera(float width, float height);

	inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	void SetProjectionMatrix(float width, float height);

	inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

	inline const float GetFOV() const { return m_FOV; }
	void SetFOV(float fov) { m_FOV = fov; SetProjectionMatrix(m_ScreenWidth, m_ScreenHeight); }

	void OnUpdate(GLFWwindow* window);

	void SetCameraAngles(GLFWwindow* window, double xPos, double yPos);

private:
	void LookAt();

private:
	float m_ScreenWidth;
	float m_ScreenHeight;

	float m_FOV;
	
	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ViewProjectionMatrix;

	float m_NearPlane;
	float m_FarPlane;

	float m_LastX;
	float m_LastY;
	
	bool m_FirstMouse = true;

	float m_deltaTime = 0.0f;
	float m_lastFrame = 0.0f;

	glm::vec3 m_CameraUp = glm::vec3{ 0.0f, 1.0f, 0.0f };
	glm::vec3 m_CameraFront = glm::vec3{ 0.0f, 0.0f,-1.0f };
	glm::vec3 m_CameraPos = glm::vec3{ 0.0f, 0.0f, 3.0f };
	
	float m_Yaw = -90.0f;
	float m_Pitch = 0.0f;
};
