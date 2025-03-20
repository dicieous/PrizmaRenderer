#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

#include "Platforms/OpenGL/OpenGLRenderer.h"
#include "Camera.h"

#include "Model.h"
#include <random>

#define BOXES 1
#define MODEL 1

const uint32_t SCR_WIDTH = 1600;
const uint32_t SCR_HEIGHT = 900;

static glm::vec2 s_ViewportSize{ SCR_WIDTH, SCR_HEIGHT };

static void ProcessInput(GLFWwindow* window)
{

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void SetupImGuiStyleWithRoundedBorders()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowRounding = 7.0f;
	style.ChildRounding = 7.0f;
	style.PopupRounding = 7.0f;

	style.FrameRounding = 5.0f;
	style.ScrollbarRounding = 10.0f;

	// Customize colors if needed
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);  // Window background
	colors[ImGuiCol_Border] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);    // Border color
}

void ResizeFramebuffer(uint32_t& colorAttachment, uint32_t& rbo, int width, int height)
{
	// Resize color attachment
	GLCall(glBindTexture(GL_TEXTURE_2D, colorAttachment));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	// Resize renderbuffer
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
	GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void renderCube(OpenGLShader& shader, bool striped = false)
{
	//Cube stuff
	float vertices[] = {
		// Positions         //Normals          //Textures
		// Front face
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,// Bottom-left
		 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,// Bottom-right
		 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,// Top-right
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,// Top-left

		// Back face
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,// Bottom-left
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,// Bottom-right
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,// Top-right
		-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// Top-left

		// Left face
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,// Bottom-left
		-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,// Bottom-right
		-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,// Top-right
		-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,// Top-left

		// Right face
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,// Bottom-left
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,// Bottom-right
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,// Top-right
		 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,// Top-left

		 // Top face
		 -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,// Bottom-left
		  0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,// Bottom-right
		  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,// Top-right
		 -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// Top-left

		 // Bottom face
		 -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,// Bottom-left
		  0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// Bottom-right
		  0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,// Top-right
		 -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,// Top-left
	};

	unsigned int indices[] = {
		// Front face
		0, 2, 1,
		2, 0, 3,

		// Back face
		4, 6, 5,
		6, 4, 7,

		// Left face
		8, 10, 9,
		10, 8, 11,

		// Right face
		12, 14, 13,
		14, 12, 15,

		// Top face
		16, 18, 17,
		18, 16, 19,

		// Bottom face
		20, 22, 21,
		22, 20, 23
	};

	//Assign VertexArrayObject
	OpenGLVertexArray va;

	OpenGLVertexArray lightVA;

	//Assign the Vertex Buffer on the GPU
	OpenGLVertexBuffer vb(sizeof(vertices), vertices);

	//Assign the Index Buffer on the GPU
	OpenGLIndexBuffer ib(sizeof(indices) / sizeof(indices[0]), indices);

	//Add Vertex Layouts
	VertexBufferLayout layout;
	layout.Push<float>(3); //Position Attributes
	layout.Push<float>(3); //Normal Attributes
	layout.Push<float>(2); //Texture Attributes
	va.AddBuffer(vb, layout);

	OpenGLRenderer renderer;
	if (striped)
		renderer.DrawTriangleStrip(va, ib, shader);
	else
		renderer.Draw(va, ib, shader);
}

void renderQuad(OpenGLShader& shader, bool striped = false)
{
	//Quad
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	uint32_t quadIndices[] = {
		0, 1, 2, 3
	};

	OpenGLVertexArray quadVAO;
	OpenGLVertexBuffer quadVBO(sizeof(quadVertices), quadVertices);
	OpenGLIndexBuffer quadIBO(sizeof(quadIndices) / sizeof(quadIndices[0]), quadIndices);

	VertexBufferLayout quadLayout;
	quadLayout.Push<float>(3);
	quadLayout.Push<float>(2);

	quadVAO.Bind();
	quadVAO.AddBuffer(quadVBO, quadLayout);


	OpenGLRenderer renderer;
	if (striped)
		renderer.DrawTriangleStrip(quadVAO, quadIBO, shader);
	else
		renderer.Draw(quadVAO, quadIBO, shader);
}

void LoadEnvironment(const std::string& hdrPath, uint32_t& envCubeMap, uint32_t& brdfLUTTexture, uint32_t& prefilterMap, uint32_t& irradianceMap)
{
	OpenGLRenderer renderer;

	OpenGLShader equirectangularToCubemap("res/Shaders/equirectangularToCubemap.shader");

	OpenGLShader irradianceShader("res/Shaders/irradianceConvolution.shader");

	OpenGLShader preFilterShader("res/Shaders/pbrPreFilter.shader");

	OpenGLShader brdfShader("res/Shaders/brdf.shader");

	uint32_t captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	glGenRenderbuffers(1, &captureRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);


	stbi_set_flip_vertically_on_load(1);
	int width, height, BPPComp;
	float* hdrImageData = stbi_loadf(hdrPath.c_str(), &width, &height, &BPPComp, 0);
	uint32_t hdrTexture;

	if (hdrImageData)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, hdrImageData);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	if (hdrImageData)
	{
		stbi_image_free(hdrImageData);
	}

	//Generate background Cubemap
	//uint32_t envCubeMap;
	glGenTextures(1, &envCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};


	equirectangularToCubemap.Bind();
	equirectangularToCubemap.SetUniformMat4f("u_projection", captureProjection);
	equirectangularToCubemap.SetUniform1i("u_equirectangularMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 1024, 1024);
	for (int i = 0; i < 6; i++)
	{
		equirectangularToCubemap.SetUniformMat4f("u_view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube(equirectangularToCubemap);
		//renderer.Draw(va, ib, equirectangularToCubemap);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//To remove visual dots
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	//Generate irradiance Cubemap
	//uint32_t irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 64, 64, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 64, 64);

	irradianceShader.Bind();
	irradianceShader.SetUniformMat4f("u_projection", captureProjection);
	irradianceShader.SetUniform1i("u_equirectangularMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

	glViewport(0, 0, 64, 64);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (int i = 0; i < 6; i++)
	{
		irradianceShader.SetUniformMat4f("u_view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//renderer.Draw(va, ib, irradianceShader);
		renderCube(irradianceShader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Generate PreFilter Map
	//uint32_t prefilterMap;
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 256, 256, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	preFilterShader.Bind();
	preFilterShader.SetUniformMat4f("u_projection", captureProjection);
	preFilterShader.SetUniform1i("u_environmentMap", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	uint32_t maxMipLevels = 5;

	for (int mip = 0; mip < maxMipLevels; mip++)
	{
		uint32_t mipWidth = 256 * glm::pow(0.5, mip);
		uint32_t mipHeight = 256 * glm::pow(0.5, mip);

		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		preFilterShader.SetUniform1f("u_roughness", roughness);

		for (int i = 0; i < 6; i++)
		{
			preFilterShader.SetUniformMat4f("u_view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//renderer.Draw(va, ib, preFilterShader);
			renderCube(preFilterShader);
		}
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//generate 2D LUT from BRDF equation
	//uint32_t brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 1024, 1024, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);
	glViewport(0, 0, 1024, 1024);

	brdfShader.Bind();
	renderer.Clear();

	//renderer.Draw(quadVAO, quadIBO, brdfShader);
	renderQuad(brdfShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

const std::vector<std::string> environmentPaths = {
	"res/Textures/Skybox/warmRestaurant.hdr",
	"res/Textures/Skybox/warm_restaurant_night_2k.hdr",
	"res/Textures/Skybox/metro_noord_2k.hdr",
	"res/Textures/Skybox/peppermint_powerplant_2_2k.hdr",
	"res/Textures/Skybox/rogland_clear_night_2k.hdr",
	"res/Textures/Skybox/shanghai_bund_2k.hdr",
};
static int currentEnvIndex = 0;

void ChangeEnvironmentInput(GLFWwindow* window, uint32_t& envCubeMap, uint32_t& brdfLUTTexture, uint32_t& prefilterMap, uint32_t& irradianceMap)
{
	static double lastPressTime = 0.0;
	double currentTime = glfwGetTime();

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && currentTime - lastPressTime > 2.0)
	{
		lastPressTime = currentTime;
		if (currentEnvIndex > 0)
			currentEnvIndex--;
		else
			return;

		LoadEnvironment(environmentPaths[currentEnvIndex], envCubeMap, brdfLUTTexture, prefilterMap, irradianceMap);
	}


	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && currentTime - lastPressTime > 2.0)
	{
		lastPressTime = currentTime;
		if (currentEnvIndex < environmentPaths.size() - 1)
			currentEnvIndex++;
		else
			return;
		LoadEnvironment(environmentPaths[currentEnvIndex], envCubeMap, brdfLUTTexture, prefilterMap, irradianceMap);
	}
}

int main()
{
	//TODO: Put all this in a  Window Class
	bool isInitialized = glfwInit();

	if (!isInitialized)
	{
		std::cout << " Glfw not initialized" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGLRenderer", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Window not initialized" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Glad Initialization Failed" << std::endl;
		return -1;
	}

	//glViewport(0, 0, 1280, 720);

	std::unique_ptr<PerspectiveCamera> Camera = std::make_unique<PerspectiveCamera>(SCR_WIDTH, SCR_HEIGHT);
	glfwSetWindowUserPointer(window, Camera.get());

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);

			s_ViewportSize.x = width;
			s_ViewportSize.y = height;

			PerspectiveCamera* camera = static_cast<PerspectiveCamera*>(glfwGetWindowUserPointer(window));
			camera->SetProjectionMatrix(static_cast<float>(width), static_cast<float>(height));
		});

	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			s_ViewportSize.x = width;
			s_ViewportSize.y = height;

			glViewport(0, 0, width, height);
		});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
		{

			PerspectiveCamera* camera = static_cast<PerspectiveCamera*>(glfwGetWindowUserPointer(window));
			camera->SetCameraAngles(window, xPos, yPos);

		});

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
		{

			PerspectiveCamera* camera = static_cast<PerspectiveCamera*>(glfwGetWindowUserPointer(window));
			float fov = camera->GetFOV();
			fov -= (float)yOffset;
			fov = glm::clamp(fov, 1.0f, 70.0f);
			camera->SetFOV(fov);

		});

	GLCall(glEnable(GL_DEPTH_TEST));
	GLCall(glEnable(GL_STENCIL_TEST));

	GLCall(glEnable(GL_BLEND));
	GLCall(glEnable(GL_MULTISAMPLE));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
	GLCall(glDepthFunc(GL_LEQUAL));
	//GLCall(glEnable(GL_PROGRAM_POINT_SIZE));

	{
		//Sphere
		std::vector<glm::vec3> spherePositions;
		std::vector<glm::vec2> sphereUV;
		std::vector<glm::vec3> sphereNormals;
		std::vector<uint32_t> sphereIndices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				spherePositions.push_back(glm::vec3(xPos, yPos, zPos));
				sphereUV.push_back(glm::vec2(xSegment, ySegment));
				sphereNormals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (uint32_t y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
				{
					sphereIndices.push_back(y * (X_SEGMENTS + 1) + x);
					sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					sphereIndices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		uint32_t indexCount = static_cast<uint32_t>(sphereIndices.size());

		std::vector<float> data;
		for (uint32_t i = 0; i < spherePositions.size(); ++i)
		{
			data.push_back(spherePositions[i].x);
			data.push_back(spherePositions[i].y);
			data.push_back(spherePositions[i].z);
			if (sphereNormals.size() > 0)
			{
				data.push_back(sphereNormals[i].x);
				data.push_back(sphereNormals[i].y);
				data.push_back(sphereNormals[i].z);
			}
			if (sphereUV.size() > 0)
			{
				data.push_back(sphereUV[i].x);
				data.push_back(sphereUV[i].y);
			}
		}

		OpenGLVertexArray sphereVAO;

		OpenGLVertexBuffer sphereVBO(data.size() * sizeof(float), data.data());
		OpenGLIndexBuffer sphereIBO(indexCount, sphereIndices.data());

		VertexBufferLayout sphereLayout;
		sphereLayout.Push<float>(3); //Position
		sphereLayout.Push<float>(3); //Normal
		sphereLayout.Push<float>(2); //TexCoords

		sphereVAO.AddBuffer(sphereVBO, sphereLayout);

		//LoadTextures
		//BrickWall
		Texture2D brickWallAlbedo("res/Textures/brickWall/alley-brick-wall_albedo.png");
		Texture2D brickWallNormal("res/Textures/brickWall/alley-brick-wall_normal-ogl.png");
		Texture2D brickWallMetallic("res/Textures/brickWall/alley-brick-wall_metallic.png");
		Texture2D brickWallRoughness("res/Textures/brickWall/alley-brick-wall_roughness.png");
		Texture2D brickWallAO("res/Textures/brickWall/alley-brick-wall_ao.png");

		//CopperScuffed
		Texture2D CopperScuffedAlbedo("res/Textures/CopperScuffed/Copper-scuffed_basecolor-boosted.png");
		Texture2D CopperScuffedNormal("res/Textures/CopperScuffed/Copper-scuffed_normal.png");
		Texture2D CopperScuffedMetallic("res/Textures/CopperScuffed/Copper-scuffed_metallic.png");
		Texture2D CopperScuffedRoughness("res/Textures/CopperScuffed/Copper-scuffed_roughness.png");
		Texture2D CopperScuffedAO("res/Textures/CopperScuffed/Copper-scuffed_ao.png");

		//darkTiles
		Texture2D darkTilesAlbedo("res/Textures/darkTiles/dark-grey-tiles_albedo.png");
		Texture2D darkTilesNormal("res/Textures/darkTiles/dark-grey-tiles_normal-ogl.png");
		Texture2D darkTilesMetallic("res/Textures/darkTiles/dark-grey-tiles_metallic.png");
		Texture2D darkTilesRoughness("res/Textures/darkTiles/dark-grey-tiles_roughness.png");
		Texture2D darkTilesAO("res/Textures/darkTiles/dark-grey-tiles_ao.png");

		//darkWood
		Texture2D darkWoodAlbedo("res/Textures/darkWood/dark-wood-stain_albedo.png");
		Texture2D darkWoodNormal("res/Textures/darkWood/dark-wood-stain_normal-ogl.png");
		Texture2D darkWoodMetallic("res/Textures/darkWood/dark-wood-stain_metallic.png");
		Texture2D darkWoodRoughness("res/Textures/darkWood/dark-wood-stain_roughness.png");
		Texture2D darkWoodAO("res/Textures/darkWood/dark-wood-stain_ao.png");

		//hammeredGold
		Texture2D hammeredGoldAlbedo("res/Textures/hammeredGold/hammered-gold_albedo.png");
		Texture2D hammeredGoldNormal("res/Textures/hammeredGold/hammered-gold_normal-ogl.png");
		Texture2D hammeredGoldMetallic("res/Textures/hammeredGold/hammered-gold_metallic.png");
		Texture2D hammeredGoldRoughness("res/Textures/hammeredGold/hammered-gold_roughness.png");
		Texture2D hammeredGoldAO("res/Textures/hammeredGold/hammered-gold_ao.png");

		//rustedTex
		Texture2D rustedTexAlbedo("res/Textures/rustedTex/rustediron2_basecolor.png");
		Texture2D rustedTexNormal("res/Textures/rustedTex/rustediron2_normal.png");
		Texture2D rustedTexMetallic("res/Textures/rustedTex/rustediron2_metallic.png");
		Texture2D rustedTexRoughness("res/Textures/rustedTex/rustediron2_roughness.png");
		Texture2D rustedTexAO("res/Textures/rustedTex/rustediron2_ao.png");

		//WhispyGrass
		Texture2D WhispyGrassAlbedo("res/Textures/WhispyGrass/wispy-grass-meadow_albedo.png");
		Texture2D WhispyGrassNormal("res/Textures/WhispyGrass/wispy-grass-meadow_normal-ogl.png");
		Texture2D WhispyGrassMetallic("res/Textures/WhispyGrass/wispy-grass-meadow_metallic.png");
		Texture2D WhispyGrassRoughness("res/Textures/WhispyGrass/wispy-grass-meadow_roughness.png");
		Texture2D WhispyGrassAO("res/Textures/WhispyGrass/wispy-grass-meadow_ao.png");



		OpenGLShader pbrShader("res/Shaders/pbr.shader");
		pbrShader.Bind();
		pbrShader.SetUniform1i("u_irradianceMap", 0);
		pbrShader.SetUniform1i("u_prefilterMap", 1);
		pbrShader.SetUniform1i("u_brdfLUT", 2);

		pbrShader.SetUniform1i("u_albedoMap", 3);
		pbrShader.SetUniform1i("u_normalMap", 4);
		pbrShader.SetUniform1i("u_metallicMap", 5);
		pbrShader.SetUniform1i("u_roughnessMap", 6);
		pbrShader.SetUniform1i("u_aoMap", 7);

		OpenGLShader environmentCubeMapShader("res/Shaders/pbrCubemap.shader");
		environmentCubeMapShader.Bind();
		environmentCubeMapShader.SetUniform1i("u_environmentMap", 0);


		OpenGLRenderer renderer;

		//SkyBox Stuff

		uint32_t envCubeMap, brdfLUTTexture, prefilterMap, irradianceMap;
		//std::string hdrPath = "res/Textures/Skybox/warmRestaurant.hdr";

		LoadEnvironment(environmentPaths[currentEnvIndex], envCubeMap, brdfLUTTexture, prefilterMap, irradianceMap);

		int scrWidth, scrHeight;
		glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
		glViewport(0, 0, scrWidth, scrHeight);


#if MODEL

		//OpenGLRenderer renderer;

		Model model("res/Models/backpack/backpack.obj");

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 450");

		SetupImGuiStyleWithRoundedBorders();


		glm::vec3 lightPositions[] = {
			glm::vec3(-10.0f,  10.0f, 10.0f),
			glm::vec3(10.0f,  10.0f, 10.0f),
			glm::vec3(-10.0f, -10.0f, 10.0f),
			glm::vec3(10.0f, -10.0f, 10.0f),
		};

		glm::vec3 lightColors[] = {
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f)
		};

		int nrRows = 7;
		int nrColumns = 7;
		float spacing = 5;

		while (!glfwWindowShouldClose(window))
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ProcessInput(window);

			renderer.Clear();

			Camera->OnUpdate(window);
			ChangeEnvironmentInput(window, envCubeMap, brdfLUTTexture, prefilterMap, irradianceMap);

			glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
			glViewport(0, 0, scrWidth, scrHeight);

#if MODEL
			pbrShader.Bind();
			pbrShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			pbrShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());

			pbrShader.SetUniformVec3f("u_viewPos", Camera->GetCameraPosition());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

			//brickwall
			brickWallAlbedo.Bind(3);
			brickWallNormal.Bind(4);
			brickWallMetallic.Bind(5);
			brickWallRoughness.Bind(6);
			brickWallAO.Bind(7);
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, 0.0f, 2.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

			pbrShader.SetUniformMat4f("u_model", model);
			renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);

			//copperScuffed
			CopperScuffedAlbedo.Bind(3);
			CopperScuffedNormal.Bind(4);
			CopperScuffedMetallic.Bind(5);
			CopperScuffedRoughness.Bind(6);
			CopperScuffedAO.Bind(7);
			model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 0.0f, 2.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

			pbrShader.SetUniformMat4f("u_model", model);
			renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);

			//darkTiles
			darkTilesAlbedo.Bind(3);
			darkTilesNormal.Bind(4);
			darkTilesMetallic.Bind(5);
			darkTilesRoughness.Bind(6);
			darkTilesAO.Bind(7);
			model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 2.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

			pbrShader.SetUniformMat4f("u_model", model);
			renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);

			//darkWood
			darkWoodAlbedo.Bind(3);
			darkWoodNormal.Bind(4);
			darkWoodMetallic.Bind(5);
			darkWoodRoughness.Bind(6);
			darkWoodAO.Bind(7);
			model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 2.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

			pbrShader.SetUniformMat4f("u_model", model);
			renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);

			//hammeredGold
			hammeredGoldAlbedo.Bind(3);
			hammeredGoldNormal.Bind(4);
			hammeredGoldMetallic.Bind(5);
			hammeredGoldRoughness.Bind(6);
			hammeredGoldAO.Bind(7);
			model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 2.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

			pbrShader.SetUniformMat4f("u_model", model);
			renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);

			//rustedTex
			rustedTexAlbedo.Bind(3);
			rustedTexNormal.Bind(4);
			rustedTexMetallic.Bind(5);
			rustedTexRoughness.Bind(6);
			rustedTexAO.Bind(7);
			model = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 2.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

			pbrShader.SetUniformMat4f("u_model", model);
			renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);

			//WhispyGrass
			WhispyGrassAlbedo.Bind(3);
			WhispyGrassNormal.Bind(4);
			WhispyGrassMetallic.Bind(5);
			WhispyGrassRoughness.Bind(6);
			WhispyGrassAO.Bind(7);
			model = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 0.0f, 2.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

			pbrShader.SetUniformMat4f("u_model", model);
			renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);

			//Render Lights
			/*for (int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); i++)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
				newPos = lightPositions[i];

				pbrShader.SetUniformVec3f("u_lightPositions[" + std::to_string(i) + "]", newPos);
				pbrShader.SetUniformVec3f("u_lightColors[" + std::to_string(i) + "]", lightColors[i]);

				glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), newPos)
					* glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

				pbrShader.SetUniformMat4f("u_model", lightModel);
				renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);
			}*/

			//Render BackGround
			//glDepthFunc(GL_LEQUAL);
			environmentCubeMapShader.Bind();
			environmentCubeMapShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			environmentCubeMapShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
			renderCube(environmentCubeMapShader);
			//renderer.Draw(va, ib, environmentCubeMapShader);
			//glDepthFunc(GL_LESS);

#endif

#if 0

			//render lightCube
			lightSrcShader.Bind();
			lightSrcShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			lightSrcShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			for (int i = 0; i < lightPositions.size(); i++)
			{
				glm::mat4 lightCube = glm::translate(glm::mat4(1.0f), glm::vec3(lightPositions[i]))
					* glm::scale(glm::mat4(1.0f), glm::vec3(0.25f));

				lightSrcShader.SetUniformMat4f("u_model", lightCube);
				lightSrcShader.SetUniformVec3f("u_lightColor", lightColors[i]);
				renderer.Draw(va, ib, lightSrcShader);
			}
			lightSrcShader.UnBind();

			/////////////////////
#endif

			ImGui::Begin("FPS");

			//ImGui::Text("This is some useful text.");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::Separator();

			ImGui::Separator();
			ImGui::End();

			glfwPollEvents();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
		}
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

