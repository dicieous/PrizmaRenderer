#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "Platforms/OpenGL/OpenGLRenderer.h"
#include "Camera.h"

const float SCR_WIDTH = 1280.0f;
const float SCR_HEIGHT = 720.0f;

static void ProcessInput(GLFWwindow* window) {

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void SetupImGuiStyleWithRoundedBorders()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowRounding = 10.0f;
	style.ChildRounding = 10.0f;
	style.PopupRounding = 10.0f;

	style.FrameRounding = 5.0f;
	style.ScrollbarRounding = 10.0f;

	// Customize colors if needed
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);  // Window background
	colors[ImGuiCol_Border] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);    // Border color
}

int main()
{
	//TODO: Put all this in a  Window Class
	bool isInitialized = glfwInit();

	if (!isInitialized) {
		std::cout << " Glfw not initialized" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGLRenderer", NULL, NULL);
	if (window == NULL) {
		std::cout << "Window not initialized" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Glad Initialization Failed" << std::endl;
		return -1;
	}

	//glViewport(0, 0, 1280, 720);

	std::unique_ptr<PerspectiveCamera> Camera = std::make_unique<PerspectiveCamera>(SCR_WIDTH, SCR_HEIGHT);
	glfwSetWindowUserPointer(window, Camera.get());

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
			PerspectiveCamera* camera = static_cast<PerspectiveCamera*>(glfwGetWindowUserPointer(window));
			camera->SetProjectionMatrix(static_cast<float>(width), static_cast<float>(height));
		});

	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
		});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos) {

		PerspectiveCamera* camera = static_cast<PerspectiveCamera*>(glfwGetWindowUserPointer(window));
		camera->SetCameraAngles(window, xPos, yPos);

		});

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {

		PerspectiveCamera* camera = static_cast<PerspectiveCamera*>(glfwGetWindowUserPointer(window));
		float fov = camera->GetFOV();
		fov -= (float)yOffset;
		fov = glm::clamp(fov, 1.0f, 70.0f);
		camera->SetFOV(fov);

		});

	{
		////////Triangle Shader//////////////////
		float vertices[] = {
			// Positions
			// Front face
			-0.5f, -0.5f,  0.5f, // Bottom-left
			 0.5f, -0.5f,  0.5f, // Bottom-right
			 0.5f,  0.5f,  0.5f, // Top-right
			-0.5f,  0.5f,  0.5f, // Top-left

			// Back face
			-0.5f, -0.5f, -0.5f, // Bottom-left
			 0.5f, -0.5f, -0.5f, // Bottom-right
			 0.5f,  0.5f, -0.5f, // Top-right
			-0.5f,  0.5f, -0.5f, // Top-left

			// Left face
			-0.5f, -0.5f, -0.5f, // Bottom-left
			-0.5f, -0.5f,  0.5f, // Bottom-right
			-0.5f,  0.5f,  0.5f, // Top-right
			-0.5f,  0.5f, -0.5f, // Top-left

			// Right face
			 0.5f, -0.5f, -0.5f, // Bottom-left
			 0.5f, -0.5f,  0.5f, // Bottom-right
			 0.5f,  0.5f,  0.5f, // Top-right
			 0.5f,  0.5f, -0.5f, // Top-left

			 // Top face
			 -0.5f,  0.5f, -0.5f, // Bottom-left
			  0.5f,  0.5f, -0.5f, // Bottom-right
			  0.5f,  0.5f,  0.5f, // Top-right
			 -0.5f,  0.5f,  0.5f, // Top-left

			 // Bottom face
			 -0.5f, -0.5f, -0.5f, // Bottom-left
			  0.5f, -0.5f, -0.5f, // Bottom-right
			  0.5f, -0.5f,  0.5f, // Top-right
			 -0.5f, -0.5f,  0.5f, // Top-left
		};

		unsigned int indices[] = {
			// Front face
			0, 1, 2,
			2, 3, 0,

			// Back face
			4, 5, 6,
			6, 7, 4,

			// Left face
			8, 9, 10,
			10, 11, 8,

			// Right face
			12, 13, 14,
			14, 15, 12,

			// Top face
			16, 17, 18,
			18, 19, 16,

			// Bottom face
			20, 21, 22,
			22, 23, 20
		};

		GLCall(glEnable(GL_DEPTH_TEST));

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));


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
		va.AddBuffer(vb, layout);

		lightVA.AddBuffer(vb, layout);

		//Create Fragment Shader
		OpenGLShader lightingShader("res/Shaders/Lighting.shader");
		lightingShader.Bind();

		OpenGLShader lightSrcShader("res/Shaders/LightCube.shader");
		lightSrcShader.Bind();

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		va.UnBind();
		lightVA.UnBind();
		vb.UnBind();

		OpenGLRenderer renderer;

		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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


		glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
		glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

		while (!glfwWindowShouldClose(window)) {


			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ProcessInput(window);

			Camera->OnUpdate(window);

			renderer.Clear();

			lightingShader.Bind();
			
			lightingShader.SetUniformVec3f("u_objectColor", objectColor);
			lightingShader.SetUniformVec3f("u_lightColor", lightColor);

			lightingShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			lightingShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());

			glm::mat4 model = glm::mat4(1.0f);
			lightingShader.SetUniformMat4f("u_model", model);

			va.Bind();
			renderer.Draw(va, ib, lightingShader);
			
			va.UnBind();
			lightingShader.UnBind();

			lightSrcShader.Bind();
			
			lightSrcShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			lightSrcShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			
			model = glm::mat4(1.0f);
			glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
			model = glm::translate(model, lightPos) * glm::scale(model, glm::vec3(0.2f));
			lightSrcShader.SetUniformMat4f("u_model", model);
			
			lightVA.Bind();

			renderer.Draw(lightVA, ib, lightSrcShader);
			
			lightVA.UnBind();
			lightSrcShader.UnBind();

			ImGui::Begin("FPS");

			//ImGui::Text("This is some useful text.");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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

