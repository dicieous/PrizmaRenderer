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

#define BOXES 1
#define MODEL 0
#define OUTLINE 0

const float SCR_WIDTH = 1280.0f;
const float SCR_HEIGHT = 720.0f;

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
	//GLCall(glEnable(GL_CULL_FACE));
	//GLCall(glCullFace(GL_FRONT));
	//GLCall(glFrontFace(GL_CW));

	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glEnable(GL_PROGRAM_POINT_SIZE));

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
		OpenGLShader lightingShader("res/Shaders/Multiple_Lights.shader");
		lightingShader.Bind();

		OpenGLShader lightSrcShader("res/Shaders/LightCube.shader");
		lightSrcShader.Bind();

		//Bind Textures
		Texture2D diffuseMap("res/Textures/container2.png");
		diffuseMap.Bind();

		Texture2D specularMap("res/Textures/container2_specular.png");
		specularMap.Bind(1);

		//Texture2D emissionMap("res/Textures/matrix.jpg");
		//emissionMap.Bind(2);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		va.UnBind();
		lightVA.UnBind();
		vb.UnBind();

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
			glm::vec3(0.7f,  0.2f,  2.0f),
			glm::vec3(2.3f, -3.3f, -4.0f),
			glm::vec3(-4.0f,  2.0f, -12.0f),
			glm::vec3(0.0f,  0.0f, -3.0f)
		};

		glm::vec3 spotLightPositions[] = {
			glm::vec3(0.5f,  0.2f,  2.0f),
			glm::vec3(2.3f, 3.3f, -4.0f),
		};

		//glm::vec3 objectColor(1.0f, 0.7f, 0.19f);
		glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		OpenGLRenderer renderer;
#endif

		//FrameBuffer Creation 
		//Quad Stuff
		float QuadVertices[] = {
			//Positions   //TexCoords
			-1.0f,  1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 1.0f, 0.0f,
		};

		uint32_t QuadIndices[] = {
			0, 1, 2,
			1, 3 ,2,
		};

		//Create Framebuffers
		uint32_t fbo;
		GLCall(glGenFramebuffers(1, &fbo));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

		uint32_t colorAttachment;
		GLCall(glGenTextures(1, &colorAttachment));
		GLCall(glBindTexture(GL_TEXTURE_2D, colorAttachment));

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment, 0));
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));

		//Create RenderBuffer
		uint32_t rbo;
		GLCall(glGenRenderbuffers(1, &rbo));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
		GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));

		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo));
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			ASSERT((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));
		}
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		OpenGLVertexArray quadVA;

		//Assign the Vertex Buffer on the GPU
		OpenGLVertexBuffer quadVB(sizeof(QuadVertices), QuadVertices);

		//Assign the Index Buffer on the GPU
		OpenGLIndexBuffer quadIB(sizeof(QuadIndices) / sizeof(QuadIndices[0]), QuadIndices);

		//Add Vertex Layouts
		VertexBufferLayout quadLayout;
		quadLayout.Push<float>(2); //Position Attribute
		quadLayout.Push<float>(2); //Texture Attribute
		quadVA.AddBuffer(quadVB, quadLayout);

		//Create Fragment Shader
		OpenGLShader QuadScreenShader("res/Shaders/FrameBufferScreen.shader");
		QuadScreenShader.Bind();
		QuadScreenShader.SetUniform1i("screenTexture", 0);

		quadVB.UnBind();

		//Cubemap Stuff

		float cm_vertices[] = {
			// Positions
			// Front face
			-1.0f, -1.0f,  1.0f,// Bottom-left
			 1.0f, -1.0f,  1.0f,// Bottom-right
			 1.0f,  1.0f,  1.0f,// Top-right
			-1.0f,  1.0f,  1.0f,// Top-left

			// Back face
			-1.0, -1.0, -1.0f,// Bottom-left
			 1.0, -1.0, -1.0f,// Bottom-right
			 1.0,  1.0, -1.0f,// Top-right
			-1.0,  1.0, -1.0f,// Top-left

			// Left face
			-1.0f, -1.0f, -1.0f,// Bottom-left
			-1.0f, -1.0f,  1.0f,// Bottom-right
			-1.0f,  1.0f,  1.0f,// Top-right
			-1.0f,  1.0f, -1.0f,// Top-left

			// Right face
			 1.0f, -1.0f, -1.0f,// Bottom-left
			 1.0f, -1.0f,  1.0f,// Bottom-right
			 1.0f,  1.0f,  1.0f,// Top-right
			 1.0f,  1.0f, -1.0f,// Top-left

			 // Top face
			 -1.0f,  1.0f, -1.0f,// Bottom-left
			  1.0f,  1.0f, -1.0f,// Bottom-right
			  1.0f,  1.0f,  1.0f,// Top-right
			 -1.0f,  1.0f,  1.0f,// Top-left

			 // Bottom face
			 -1.0f, -1.0f, -1.0f,// Bottom-left
			  1.0f, -1.0f, -1.0f,// Bottom-right
			  1.0f, -1.0f,  1.0f,// Top-right
			 -1.0f, -1.0f,  1.0f,// Top-left
		};

		unsigned int cm_indices[] = {
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

		OpenGLVertexArray CubeMapVA;

		//Assign the Vertex Buffer on the GPU
		OpenGLVertexBuffer CubeMapVB(sizeof(cm_vertices), cm_vertices);

		//Assign the Index Buffer on the GPU
		OpenGLIndexBuffer CubeMapIB(sizeof(cm_indices) / sizeof(cm_indices[0]), cm_indices);

		//Add Vertex Layouts
		VertexBufferLayout CubeMapLayout;
		CubeMapLayout.Push<float>(3); //Position Attribute
		CubeMapVA.AddBuffer(CubeMapVB, CubeMapLayout);

		//Create Fragment Shader
		OpenGLShader CubeMapShader("res/Shaders/Skybox.shader");
		CubeMapShader.Bind();
		CubeMapShader.SetUniform1i("u_skybox", 0);

		CubeMapVB.UnBind();

		std::vector<std::string> faces
		{
			"res/Textures/Skybox/Lake/posx.jpg",
			"res/Textures/Skybox/Lake/negx.jpg",
			"res/Textures/Skybox/Lake/posy.jpg",
			"res/Textures/Skybox/Lake/negy.jpg",
			"res/Textures/Skybox/Lake/posz.jpg",
			"res/Textures/Skybox/Lake/negz.jpg",
		};

		uint32_t CubeMapID;
		glGenTextures(1, &CubeMapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapID);

		int cm_Width, cm_Height, cm_BPP;

		for (int i = 0; i < faces.size(); i++)
		{
			stbi_set_flip_vertically_on_load(0);
			unsigned char* LocalBuffer = stbi_load(faces[i].c_str(), &cm_Width, &cm_Height, &cm_BPP, 0);

			if (LocalBuffer)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, cm_Width, cm_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, LocalBuffer);
				stbi_image_free(LocalBuffer);
			}
			else
			{
				ASSERT(false);
				std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(LocalBuffer);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		//Uniform Buffer Objects
		uint32_t ubo;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, 2 * sizeof(glm::mat4));

		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(Camera->GetViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(Camera->GetProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//Light's UBOs
		uint32_t lightUBO;
		glGenBuffers(1, &lightUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
		glBufferData(GL_UNIFORM_BUFFER, 640, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 1, lightUBO, 0, 640);

#if MODEL
		OpenGLRenderer renderer;

		OpenGLShader ModelShader("res/Shaders/Model_Loading.shader");
		ModelShader.Bind();

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
		ImGui_ImplOpenGL3_Init("#version 410");

		SetupImGuiStyleWithRoundedBorders();



		while (!glfwWindowShouldClose(window))
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ProcessInput(window);

			//GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			//renderer.Clear();

			//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			//glEnable(GL_DEPTH_TEST);

			renderer.Clear();

			Camera->OnUpdate(window);

#if MODEL

			ModelShader.Bind();

			ModelShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			ModelShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			//ModelShader.SetUniform1i("u_Skybox", 0);

			glm::mat4 matModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
			ModelShader.SetUniformMat4f("u_model", matModel);
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapID);

			model.Draw(ModelShader);

			ModelShader.UnBind();
#endif

#if BOXES

			const float position = 2.0f;
			float posY = sin(glfwGetTime()) * position;
			float posZ = cos(glfwGetTime()) * position;
			glm::vec4 lightPos(1.2f, 1.0f, 2.0f, 1.0f);
			glm::vec4 lightPosDirec(-0.2f, -1.0f, -0.3f, 0.0f);
			//glm::vec3 lightPos(posY, 1.0f, posZ);

			//RecieverObject
			lightingShader.Bind();
			diffuseMap.Bind();
			lightingShader.SetUniform1i("material.diffuse", 0);


			specularMap.Bind(1);
			lightingShader.SetUniform1i("material.specular", 1);

			/*emissionMap.Bind(2);
			lightingShader.SetUniform1i("material.emission", 2);*/

			lightingShader.SetUniform1f("material.shininess", 64.0f);

			// Offsets
			const uint32_t PointLightOffset = 64;
			const uint32_t SpotLightOffset = 384;

			const uint32_t PointLightStride = 80;
			const uint32_t SpotLightStride = 112;

			const uint32_t PointLightCount = 4;
			const uint32_t SpotLightCount = 2;

			// Directional Light
			glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(glm::vec3(-0.2f, -1.0f, -0.3f))); // Direction
			glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(glm::vec3), glm::value_ptr(glm::vec3(0.05f))); // Ambient
			glBufferSubData(GL_UNIFORM_BUFFER, 32, sizeof(glm::vec3), glm::value_ptr(glm::vec3(0.5f))); // Diffuse
			glBufferSubData(GL_UNIFORM_BUFFER, 48, sizeof(glm::vec3), glm::value_ptr(glm::vec3(0.5f))); // Specular

			// Point Lights
			float constant = 1.0f;
			float linear = 0.09f;
			float quadratic = 0.032f;

			for (uint32_t i = 0; i < PointLightCount; i++)
			{
				uint32_t baseOffset = PointLightOffset + i * PointLightStride;

				glBufferSubData(GL_UNIFORM_BUFFER, baseOffset + 0, sizeof(glm::vec3), glm::value_ptr(pointLightPositions[i])); // Position
				glBufferSubData(GL_UNIFORM_BUFFER, baseOffset + 16, sizeof(glm::vec3), glm::value_ptr(glm::vec3(0.05f))); // Ambient
				glBufferSubData(GL_UNIFORM_BUFFER, baseOffset + 32, sizeof(glm::vec3), glm::value_ptr(glm::vec3(1.0f))); // Diffuse
				glBufferSubData(GL_UNIFORM_BUFFER, baseOffset + 48, sizeof(glm::vec3), glm::value_ptr(glm::vec3(1.2f))); // Specular

				glBufferSubData(GL_UNIFORM_BUFFER, baseOffset + 64, sizeof(float), &constant);
				glBufferSubData(GL_UNIFORM_BUFFER, baseOffset + 68, sizeof(float), &linear);
				glBufferSubData(GL_UNIFORM_BUFFER, baseOffset + 72, sizeof(float), &quadratic);
			}

			// Spot Lights
			float cutOff = glm::cos(glm::radians(12.5f));
			float outerCutOff = glm::cos(glm::radians(15.0f));

			for (int i = 0; i < SpotLightCount; i++)
			{
				size_t offset = SpotLightOffset + i * SpotLightStride;

				glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec3), glm::value_ptr(spotLightPositions[i])); // position
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 16, sizeof(glm::vec3), glm::value_ptr(glm::vec3(0.0f))); // ambient
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 48, sizeof(glm::vec3), glm::value_ptr(glm::vec3(1.0f))); // diffuse
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 64, sizeof(glm::vec3), glm::value_ptr(glm::vec3(1.2f))); // specular

				glBufferSubData(GL_UNIFORM_BUFFER, offset + 80, sizeof(float), &constant);
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 84, sizeof(float), &linear);
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 88, sizeof(float), &quadratic);

				glBufferSubData(GL_UNIFORM_BUFFER, offset + 96, sizeof(float), &cutOff);
				glBufferSubData(GL_UNIFORM_BUFFER, offset + 100, sizeof(float), &outerCutOff);
			}

			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			lightingShader.Bind();
			lightingShader.SetUniformVec3f("u_viewPos", Camera->GetCameraPosition());

			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(Camera->GetViewMatrix()));
		
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(Camera->GetProjectionMatrix()));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			//lightingShader.SetUniform1i("u_Skybox", 0);

			glm::mat4 model = glm::mat4(1.0f);
			//lightingShader.SetUniformMat4f("u_model", model);

			va.Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapID);
			for (int i = 0; i < sizeof(cubePositions) / sizeof(glm::vec3); i++)
			{
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, cubePositions[i]);
				float angle = 10.0f * (i + 1);
				model = glm::rotate(model, glm::radians(angle) * (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
				lightingShader.SetUniformMat4f("u_model", model);
				renderer.Draw(va, ib, lightingShader);
			}

			lightingShader.UnBind();
			va.UnBind();
			diffuseMap.UnBind();
			specularMap.UnBind();
			//////////////////

			//LightObject
			lightSrcShader.Bind();

			//lightSrcShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			//lightSrcShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			lightSrcShader.SetUniformVec3f("u_lightColor", lightColor);

			lightVA.Bind();

			for (int i = 0; i < (sizeof(pointLightPositions) + sizeof(spotLightPositions)) / sizeof(glm::vec3); i++)
			{
				model = glm::mat4(1.0f);

				glm::vec3 position(0.0f);
				if (i < 4)
				{
					position = pointLightPositions[i];
				}
				else
				{
					position = spotLightPositions[i - 4];
				}

				model = glm::translate(model, position)
					* glm::scale(model, glm::vec3(0.2f));

				lightSrcShader.SetUniformMat4f("u_model", model);
				renderer.Draw(lightVA, ib, lightSrcShader);
			}

			lightVA.UnBind();
			lightSrcShader.UnBind();
			/////////////////////
#endif
			//FrameBuffer Stuff
			/*glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			QuadScreenShader.Bind();
			quadVA.Bind();
			GLCall(glActiveTexture(GL_TEXTURE0));
			glBindTexture(GL_TEXTURE_2D, colorAttachment);
			QuadScreenShader.SetUniform1i("screenTexture", 0);
			renderer.Draw(quadVA, quadIB, QuadScreenShader);
			QuadScreenShader.UnBind();
			quadVA.UnBind();*/

			//CubeMap
			/*glDepthFunc(GL_LEQUAL);
			CubeMapShader.Bind();

			glm::mat4 view = glm::mat4(glm::mat3(Camera->GetViewMatrix()));

			CubeMapShader.SetUniformMat4f("u_view", view);
			CubeMapShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());

			CubeMapVA.Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapID);
			renderer.Draw(CubeMapVA, CubeMapIB, CubeMapShader);

			CubeMapShader.UnBind();
			CubeMapVA.UnBind();
			glDepthFunc(GL_LESS);*/

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

		glDeleteRenderbuffers(1, &rbo);
		glDeleteFramebuffers(1, &fbo);

	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

