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

		OpenGLShader pbrShader("res/Shaders/pbr.shader");
		pbrShader.Bind();
		pbrShader.SetUniformVec3f("u_albedo", glm::vec3(0.5f, 0.0f, 0.0f));
		pbrShader.SetUniform1f("u_ao", 1.0f);
		pbrShader.SetUniform1i("u_irradianceMap", 0);

		OpenGLShader environmentCubeMapShader("res/Shaders/pbrCubemap.shader");
		environmentCubeMapShader.Bind();
		environmentCubeMapShader.SetUniform1i("u_environmentMap", 0);

		OpenGLRenderer renderer;
		
		//SkyBox Stuff

		uint32_t captureFBO, captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		glGenRenderbuffers(1, &captureRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);


		stbi_set_flip_vertically_on_load(1);
		int width, height, BPPComp;
		float* hdrImageData = stbi_loadf("res/Textures/Skybox/warm_restaurant_night_2k.hdr", &width, &height, &BPPComp, 0);
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
		uint32_t envCubeMap;
		glGenTextures(1, &envCubeMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

		OpenGLShader equirectangularToCubemap("res/Shaders/equirectangularToCubemap.shader");
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

			renderer.Draw(va, ib, equirectangularToCubemap);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//Generate irradiance Cubemap

		uint32_t irradianceMap;
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

		OpenGLShader irradianceShader("res/Shaders/irradianceConvolution.shader");
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

			renderer.Draw(va, ib, irradianceShader);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		int scrWidth, scrHeight;
		glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
		glViewport(0, 0, scrWidth, scrHeight);
#endif
		

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
		float spacing = 2.5;

		while (!glfwWindowShouldClose(window))
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ProcessInput(window);

			renderer.Clear();

			Camera->OnUpdate(window);

#if MODEL
			pbrShader.Bind();
			pbrShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			pbrShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());

			pbrShader.SetUniformVec3f("u_viewPos", Camera->GetCameraPosition());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

			for (int row = 0; row < nrRows; row++)
			{
				pbrShader.SetUniform1f("u_metallic", (float)row / (float)nrRows);
				for (int col = 0; col < nrColumns; col++)
				{
					pbrShader.SetUniform1f("u_roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

					glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(
						(col - (nrColumns * 0.5f))* spacing,
						(row - (nrRows * 0.5f))* spacing,
						0.0f
					));

					pbrShader.SetUniformMat4f("u_model", model);
					renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);
				}
			}

			//Render Lights
			for (int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); i++)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
				newPos = lightPositions[i];

				pbrShader.SetUniformVec3f("u_lightPositions[" + std::to_string(i) + "]", newPos);
				pbrShader.SetUniformVec3f("u_lightColors[" + std::to_string(i) + "]", lightColors[i]);

				glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), newPos)
					* glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

				pbrShader.SetUniformMat4f("u_model", lightModel);
				renderer.DrawTriangleStrip(sphereVAO, sphereIBO, pbrShader);
			}

			//Render BackGround
			glDepthFunc(GL_LEQUAL);
			environmentCubeMapShader.Bind();
			environmentCubeMapShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			environmentCubeMapShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
			renderer.Draw(va, ib, environmentCubeMapShader);
			glDepthFunc(GL_LESS);

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

