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

const uint32_t SCR_WIDTH = 1280;
const uint32_t SCR_HEIGHT = 720;

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
	//GLCall(glEnable(GL_CULL_FACE));
	//GLCall(glCullFace(GL_FRONT));
	//GLCall(glFrontFace(GL_CW));

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
		OpenGLShader pointShadowMapShader("res/Shaders/PointShadowMap.shader");
		pointShadowMapShader.Bind();

		OpenGLShader pointShadowDepthShader("res/Shaders/PointShadowDepthShader.shader");
		pointShadowDepthShader.Bind();

		OpenGLShader depthDebuggingQuadShader("res/Shaders/DepthDebugQuad.shader");
		depthDebuggingQuadShader.Bind();

		OpenGLShader lightSrcShader("res/Shaders/LightCube.shader");

		//Bind Textures
		Texture2D diffuseMap("res/Textures/container2.png");
		diffuseMap.Bind();

		Texture2D specularMap("res/Textures/container2_specular.png");
		specularMap.Bind(3);

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
		woodTexture.Bind(2);
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
			0, 1, 2,
			0, 2 ,3,
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
		glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		OpenGLRenderer renderer;
#endif
		//Normal Map stuff

		// positions
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);

		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);

		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		//First Triangle
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		glm::vec3 tangent1, tangent2;
		glm::vec3 biTangent1, biTangent2;

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		biTangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		biTangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		biTangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		//Second Triangle
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		biTangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		biTangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		biTangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


		glm::vec3 tan = glm::normalize((tangent1 + tangent2) * 0.5f);
		glm::vec3 biTan = glm::normalize((biTangent1 + biTangent2) * 0.5f);

		float normalQuadVertices[] = {
			//Position              //Normals         //TexCoords   //Tangent            //BiTangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tan.x, tan.y, tan.z, biTan.x, biTan.y, biTan.z,
			pos2.x, pos2.y, pos2.z,	nm.x, nm.y, nm.z, uv2.x, uv2.y, tan.x, tan.y, tan.z, biTan.x, biTan.y, biTan.z,
			pos3.x, pos3.y, pos3.z,	nm.x, nm.y, nm.z, uv3.x, uv3.y, tan.x, tan.y, tan.z, biTan.x, biTan.y, biTan.z,
			pos4.x, pos4.y, pos4.z,	nm.x, nm.y, nm.z, uv4.x, uv4.y, tan.x, tan.y, tan.z, biTan.x, biTan.y, biTan.z,
		};


		//Indices
		uint32_t normalQuadIndices[] = {
			0, 1, 2,
			0, 2 ,3,
		};

		OpenGLVertexArray parallaxVAO;

		OpenGLVertexBuffer normalVBO(sizeof(normalQuadVertices), normalQuadVertices);
		OpenGLIndexBuffer parallaxIBO(sizeof(normalQuadIndices) / sizeof(normalQuadIndices[0]), normalQuadIndices);

		VertexBufferLayout normalMapLayout;
		normalMapLayout.Push<float>(3); //Position
		normalMapLayout.Push<float>(3); //Normals
		normalMapLayout.Push<float>(2); //TexCoords
		normalMapLayout.Push<float>(3); //Tangents
		normalMapLayout.Push<float>(3); //BiTangents

		parallaxVAO.AddBuffer(normalVBO, normalMapLayout);

		OpenGLShader parallaxMappingShader("res/Shaders/ParallaxMapping.shader");

		Texture2D brick2DiffuseMap("res/Textures/bricks2.jpg");
		brick2DiffuseMap.Bind();

		Texture2D brick2NormalMap("res/Textures/bricks2_normal.jpg");
		brick2NormalMap.Bind(1);

		Texture2D brick2DepthMap("res/Textures/bricks2_disp.jpg");
		brick2DepthMap.Bind(2);

		Texture2D toyBoxTexture("res/Textures/wood.png");
		toyBoxTexture.Bind(3);

		Texture2D toyBoxNormalMap("res/Textures/toy_box_normal.png");
		toyBoxNormalMap.Bind(4);

		Texture2D toyBoxDisplacementMap("res/Textures/toy_box_disp.png");
		toyBoxDisplacementMap.Bind(5);

		parallaxMappingShader.Bind();
		parallaxMappingShader.SetUniform1i("u_diffuseMap", 0);
		parallaxMappingShader.SetUniform1i("u_NormalMap", 1);
		parallaxMappingShader.SetUniform1i("u_DepthMap", 2);



#if MODEL
		OpenGLRenderer renderer;

		OpenGLShader ModelShader("res/Shaders/Model_Loading.shader");
		ModelShader.Bind();

		//OpenGLShader ModelNormalsVisualizerShader("res/Shaders/Model_Normals.shader");
		//ModelNormalsVisualizerShader.Bind();

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

		glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

		while (!glfwWindowShouldClose(window))
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ProcessInput(window);

			renderer.Clear();

			Camera->OnUpdate(window);

#if MODEL

			ModelShader.Bind();

			ModelShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			ModelShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());

			//ModelShader.SetUniform1f("u_time", glfwGetTime());
			//ModelShader.SetUniform1i("u_Skybox", 0);

			glm::mat4 matModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
			ModelShader.SetUniformMat4f("u_model", matModel);
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapID);

			model.Draw(ModelShader);
			ModelShader.UnBind();

			//ModelNormalsVisualizerShader.Bind();
			//ModelNormalsVisualizerShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			//ModelNormalsVisualizerShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			//ModelNormalsVisualizerShader.SetUniformMat4f("u_model", matModel);

			//model.Draw(ModelNormalsVisualizerShader);
			//ModelNormalsVisualizerShader.UnBind();

#endif

#if BOXES
			//Render Normal Mapped Quad
			parallaxMappingShader.Bind();
			parallaxMappingShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			parallaxMappingShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());

			parallaxMappingShader.SetUniformVec3f("u_lightPos", lightPos);
			parallaxMappingShader.SetUniformVec3f("u_viewPos", Camera->GetCameraPosition());

			parallaxMappingShader.SetUniform1f("u_HeightScale", 0.1f);

			glm::mat4 brickwallQuadModel = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			parallaxMappingShader.SetUniformMat4f("u_model", brickwallQuadModel);
			brick2DiffuseMap.Bind();
			brick2NormalMap.Bind(1);
			brick2DepthMap.Bind(2);

			parallaxMappingShader.SetUniform1i("u_diffuseMap", 0);
			parallaxMappingShader.SetUniform1i("u_NormalMap", 1);
			parallaxMappingShader.SetUniform1i("u_DepthMap", 2);

			renderer.Draw(parallaxVAO, parallaxIBO, parallaxMappingShader);

			brickwallQuadModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 2.0f))
				* glm::rotate(glm::mat4(1.0f), glm::radians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			parallaxMappingShader.SetUniformMat4f("u_model", brickwallQuadModel);

			toyBoxTexture.Bind(3);
			toyBoxNormalMap.Bind(4);
			toyBoxDisplacementMap.Bind(5);

			parallaxMappingShader.SetUniform1i("u_diffuseMap", 3);
			parallaxMappingShader.SetUniform1i("u_NormalMap", 4);
			parallaxMappingShader.SetUniform1i("u_DepthMap", 5);

			renderer.Draw(parallaxVAO, parallaxIBO, parallaxMappingShader);


			parallaxMappingShader.UnBind();


			//render lightCube
			glm::mat4 lightCube = glm::translate(glm::mat4(1.0f), glm::vec3(lightPos))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.25f));

			lightSrcShader.Bind();
			lightSrcShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			lightSrcShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			lightSrcShader.SetUniformMat4f("u_model", lightCube);
			lightSrcShader.SetUniformVec3f("u_lightColor", glm::vec3(1.0f));
			renderer.Draw(va, ib, lightSrcShader);

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

