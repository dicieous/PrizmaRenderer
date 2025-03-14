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
	//GLCall(glEnable(GL_PROGRAM_POINT_SIZE));

	{

#if BOXES

		////////Triangle Shader//////////////////
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

		VertexBufferLayout lightSourceLayout;
		lightSourceLayout.Push<float>(3);//Postion Attributes
		lightSourceLayout.AddStride(5 * sizeof(float));
		lightVA.AddBuffer(vb, lightSourceLayout);

		//Create Fragment Shader
		OpenGLShader bloomShader("res/Shaders/bloom.shader");
		bloomShader.Bind();

		OpenGLShader lightSrcShader("res/Shaders/bloomLight.shader");
		lightSrcShader.Bind();

		OpenGLShader bloomFinalShader("res/Shaders/bloomOutput.shader");
		bloomFinalShader.Bind();

		OpenGLShader bloomBlurShader("res/Shaders/bloomBlur.shader");
		bloomBlurShader.Bind();


		//Bind Textures
		Texture2D diffuseMap("res/Textures/container2.png");
		diffuseMap.Bind();

		//Texture2D specularMap("res/Textures/container2_specular.png");
		//specularMap.Bind(1);

		//Texture2D emissionMap("res/Textures/matrix.jpg");
		//emissionMap.Bind(2);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		va.UnBind();
		lightVA.UnBind();
		vb.UnBind();

		float woodQuadVertices[] = {
			// position				//Normals		 //TexCoords
			 25.0f, -0.5f,  25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,// Top-right
			-25.0f, -0.5f,  25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,// Top-left
			-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,// Bottom-left
			 25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,// Bottom-right
		};

		uint32_t woodQuadIndices[] = {
			0, 1, 2,
			0, 2 ,3,
		};

		OpenGLVertexArray woodQuadVA;

		//Assign the Vertex Buffer on the GPU
		OpenGLVertexBuffer woodQuadVB(sizeof(woodQuadVertices), woodQuadVertices);

		//Assign the Index Buffer on the GPU
		OpenGLIndexBuffer woodQuadIB(sizeof(woodQuadIndices) / sizeof(woodQuadIndices[0]), woodQuadIndices);

		//Add Vertex Layouts
		VertexBufferLayout woodQuadLayout;
		woodQuadLayout.Push<float>(3); //Position Attribute
		woodQuadLayout.Push<float>(3); //Normal Attribute
		woodQuadLayout.Push<float>(2); //Texture Attribute
		woodQuadVA.AddBuffer(woodQuadVB, woodQuadLayout);

		//Create Texture
		Texture2D woodTexture("res/Textures/wood.png");
		woodTexture.Bind();
		woodQuadVB.UnBind();

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

		quadVAO.UnBind();

		glm::vec3 cubePositions[] = {
			glm::vec3(0.0f,  0.0f,  0.0f),
			glm::vec3(2.0f,  5.0f, -15.0f),
			glm::vec3(-1.5f, -2.2f, -2.5f),
			glm::vec3(-3.8f, -2.0f, -12.3f),
			glm::vec3(2.4f, -0.4f, -3.5f),
			glm::vec3(-1.7f,  3.0f, -7.5f),
			glm::vec3(1.3f, -2.0f, -2.5f),
			glm::vec3(1.5f,  2.0f, -2.5f),
			glm::vec3(1.5f,  0.2f, -1.5f),
			glm::vec3(-1.3f,  1.0f, -1.5f)
		};

		glm::vec3 pointLightPositions[] = {
			glm::vec3(0.4f,  1.5f,  -0.5f),
			glm::vec3(2.3f, -2.3f, -4.0f),
			glm::vec3(-4.0f,  2.0f, -12.0f),
			glm::vec3(0.0f,  0.0f, -3.0f),
			//glm::vec3(0.0f,  -6.0f, 0.0f),
		};

		glm::vec3 spotLightPositions[] = {
			glm::vec3(0.5f,  0.2f,  2.0f),
			glm::vec3(2.3f, 3.3f, -4.0f),
		};

		//glm::vec3 objectColor(1.0f, 0.7f, 0.19f);
		//glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		OpenGLRenderer renderer;
#endif
		//Deferred Rendering

		//Create FrameBuffers
		uint32_t gBufferFBO;
		glGenFramebuffers(1, &gBufferFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

		//Textures
		uint32_t gPosition, gNormal, gAlbedo;

		//Position Texture Buffer
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		//Normal Texture Buffer
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		//Albedo + specular Texture Buffer
		glGenTextures(1, &gAlbedo);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

		//RenderBuffer
		uint32_t depthRBO;
		glGenRenderbuffers(1, &depthRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

		uint32_t attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//ssao FrameBuffer
		uint32_t ssaoFBO;
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		//ssao Texture
		uint32_t ssaoColorBuffer;
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//blur FrameBuffer
		uint32_t ssaoBlurFBO;
		glGenFramebuffers(1, &ssaoBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

		//ssao blur Texture
		uint32_t ssaoBlurColorBuffer;
		glGenTextures(1, &ssaoBlurColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurColorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoKernel;
		
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = (float)i / 64.0f;
			scale = lerp(0.1f, 1.0f, scale * scale);
			ssaoKernel.push_back(sample);
		}

		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator) * 2.0f - 1.0f,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		//noise Texture
		uint32_t noiseTexture;
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
		glm::vec3 lightColor = glm::vec3(0.3, 0.3, 0.7);

		std::vector<glm::vec3> objectPositions;
		objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
		objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
		objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
		objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
		objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
		objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
		objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
		objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
		objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));

#if MODEL

		//OpenGLRenderer renderer;

		OpenGLShader ssaoGeometryPassShader("res/Shaders/ssaoGeometryPass.shader");
		ssaoGeometryPassShader.Bind();

		OpenGLShader ssaoBlurPassShader("res/Shaders/ssaoBlur.shader");
		ssaoBlurPassShader.Bind();

		OpenGLShader ssaoShader("res/Shaders/ssao.shader");
		ssaoShader.Bind();

		OpenGLShader ssaoLightingPassShader("res/Shaders/ssaoLighting.shader");
		ssaoLightingPassShader.Bind();


		Model model("res/Models/backpack/backpack.obj");

		ssaoBlurPassShader.Bind();
		ssaoBlurPassShader.SetUniform1i("u_ssaoInput", 0);

		ssaoShader.Bind();
		ssaoShader.SetUniform1i("u_gPosition", 0);
		ssaoShader.SetUniform1i("u_gNormal", 1);
		ssaoShader.SetUniform1i("u_texNoise", 2);

		ssaoLightingPassShader.Bind();
		ssaoLightingPassShader.SetUniform1i("u_gPosition", 0);
		ssaoLightingPassShader.SetUniform1i("u_gNormal", 1);
		ssaoLightingPassShader.SetUniform1i("u_gAlbedo", 2);
		ssaoLightingPassShader.SetUniform1i("u_ssao", 3);

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

		//glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

		while (!glfwWindowShouldClose(window))
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ProcessInput(window);

			renderer.Clear();

			Camera->OnUpdate(window);

#if MODEL
			//Geometry Pass
			glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			GLCall(glDisable(GL_BLEND));
			ssaoGeometryPassShader.Bind();
			ssaoGeometryPassShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			ssaoGeometryPassShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());

			glm::mat4 roomModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.3f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 7.5f, 10.0f));

			ssaoGeometryPassShader.SetUniformMat4f("u_model", roomModel);
			
			ssaoGeometryPassShader.SetUniform1i("u_invertedNormals", 1);
			renderer.Draw(va, ib, ssaoGeometryPassShader);
			ssaoGeometryPassShader.SetUniform1i("u_invertedNormals", 0);

			glm::mat4 matModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

			ssaoGeometryPassShader.SetUniformMat4f("u_model", matModel);
			model.Draw(ssaoGeometryPassShader);

			ssaoGeometryPassShader.UnBind();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//SSAO Pass
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT);
			ssaoShader.Bind();
			for (int i = 0; i < 64; i++)
			{
				ssaoShader.SetUniformVec3f("u_samples[" + std::to_string(i) + "]", ssaoKernel[i]);
			}
			ssaoShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);

			quadVAO.Bind();
			quadIBO.Bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//blur
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
			glClear(GL_COLOR_BUFFER_BIT);
			ssaoBlurPassShader.Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

			quadVAO.Bind();
			quadIBO.Bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//Lighting Pass
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ssaoLightingPassShader.Bind();
			ssaoLightingPassShader.SetUniformVec3f("u_light.Position", lightPos);
			ssaoLightingPassShader.SetUniformVec3f("u_light.Color", lightColor);
			// update attenuation parameters and calculate radius
			const float linear = 0.09f;
			const float quadratic = 0.032f;
			ssaoLightingPassShader.SetUniform1f("u_light.Linear", linear);
			ssaoLightingPassShader.SetUniform1f("u_light.Quadratic", quadratic);
		
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gAlbedo);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);

			quadVAO.Bind();
			quadIBO.Bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			GLCall(glEnable(GL_BLEND));  

			//glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO);
			//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			//
			//glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//render lightCube

			/*lightSrcShader.Bind();
			lightSrcShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			lightSrcShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			glm::mat4 lightCube = glm::translate(glm::mat4(1.0f), glm::vec3(lightPos))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.25f));

			lightSrcShader.SetUniformMat4f("u_model", lightCube);
			lightSrcShader.SetUniformVec3f("u_lightColor", lightColor);
			renderer.Draw(va, ib, lightSrcShader);

			lightSrcShader.UnBind();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);*/


#endif

#if 0
			glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			bloomShader.Bind();
			bloomShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			bloomShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());

			woodTexture.Bind(0);

			for (int i = 0; i < lightPositions.size(); i++)
			{
				bloomShader.SetUniformVec3f("u_lights[" + std::to_string(i) + "].position", lightPositions[i]);
				bloomShader.SetUniformVec3f("u_lights[" + std::to_string(i) + "].color", lightColors[i]);
			}

			//bloomShader.SetUniformVec3f("u_viewPos", Camera->GetCameraPosition());

			glm::mat4 floorCube = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(12.5f, 0.5f, 12.5f));

			bloomShader.SetUniformMat4f("u_model", floorCube);
			renderer.Draw(va, ib, bloomShader);

			woodTexture.UnBind();

			diffuseMap.Bind();

			glm::mat4 cube1 = glm::translate(glm::mat4(), glm::vec3(0.0f, 1.5f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			bloomShader.SetUniformMat4f("u_model", cube1);
			renderer.Draw(va, ib, bloomShader);

			glm::mat4 cube2 = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 1.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			bloomShader.SetUniformMat4f("u_model", cube2);
			renderer.Draw(va, ib, bloomShader);

			glm::mat4 cube3 = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			bloomShader.SetUniformMat4f("u_model", cube3);
			renderer.Draw(va, ib, bloomShader);

			glm::mat4 cube4 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 2.0f))
				* glm::rotate(glm::mat4(1.0f), glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
			bloomShader.SetUniformMat4f("u_model", cube4);
			renderer.Draw(va, ib, bloomShader);

			glm::mat4 cube5 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)))
				* glm::scale(glm::mat4(1.0f), glm::vec3(1.25f));
			bloomShader.SetUniformMat4f("u_model", cube5);
			renderer.Draw(va, ib, bloomShader);

			glm::mat4 cube6 = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.0f, -3.0f))
				* glm::rotate(glm::mat4(1.0f), glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
			bloomShader.SetUniformMat4f("u_model", cube6);
			renderer.Draw(va, ib, bloomShader);


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
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//blur with 2 pass gaussian blur
			bool horizontal = true, first_iteration = true;
			int amount = 10;
			bloomBlurShader.Bind();
			for (unsigned int i = 0; i < amount; i++)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
				bloomBlurShader.SetUniform1i("u_horizontal", horizontal);
				glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingPongColorBuffer[!horizontal]);

				quadVAO.Bind();
				quadIBO.Bind();
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				horizontal = !horizontal;
				if (first_iteration)
					first_iteration = false;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//render to the quad with HDR
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			bloomFinalShader.Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pingPongColorBuffer[!horizontal]);
			bloomFinalShader.SetUniform1f("u_exposure", 1.3f);

			quadVAO.Bind();
			quadIBO.Bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


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

