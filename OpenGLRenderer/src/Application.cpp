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

#include "Platforms/OpenGL/OpenGLRenderer.h"
#include "Camera.h"

#include "Model.h"

#define BOXES 0
#define MODEL 1

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

		GLCall(glEnable(GL_DEPTH_TEST));

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	{

#if BOXES

		////////Triangle Shader//////////////////
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

		

		while (!glfwWindowShouldClose(window)) {

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ProcessInput(window);

			Camera->OnUpdate(window);

			renderer.Clear();
			
#if MODEL

			ModelShader.Bind();
			
			ModelShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			ModelShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			
			glm::mat4 matModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
			ModelShader.SetUniformMat4f("u_model", matModel);

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
			
			// directional light
			lightingShader.SetUniformVec3f("dirLight.direction", glm::vec3(- 0.2f, -1.0f, -0.3f));
			lightingShader.SetUniformVec3f("dirLight.ambient", glm::vec3(0.05f));
			lightingShader.SetUniformVec3f("dirLight.diffuse", glm::vec3(0.4f));
			lightingShader.SetUniformVec3f("dirLight.specular", glm::vec3(0.5f));
			// point light 1
			lightingShader.SetUniformVec3f("pointLightList[0].position", pointLightPositions[0]);
			lightingShader.SetUniformVec3f("pointLightList[0].ambient", glm::vec3(0.05f));
			lightingShader.SetUniformVec3f("pointLightList[0].diffuse", glm::vec3(0.8f));
			lightingShader.SetUniformVec3f("pointLightList[0].specular", glm::vec3(1.0f));
			lightingShader.SetUniform1f("pointLightList[0].constant", 1.0f);
			lightingShader.SetUniform1f("pointLightList[0].linear", 0.09f);
			lightingShader.SetUniform1f("pointLightList[0].quadratic", 0.032f);
			// point light 2
			lightingShader.SetUniformVec3f("pointLightList[1].position", pointLightPositions[1]);
			lightingShader.SetUniformVec3f("pointLightList[1].ambient", glm::vec3(0.05f));
			lightingShader.SetUniformVec3f("pointLightList[1].diffuse", glm::vec3(0.8f));
			lightingShader.SetUniformVec3f("pointLightList[1].specular", glm::vec3(1.0f));
			lightingShader.SetUniform1f("pointLightList[1].constant", 1.0f);
			lightingShader.SetUniform1f("pointLightList[1].linear", 0.09f);
			lightingShader.SetUniform1f("pointLightList[1].quadratic", 0.032f);
			// point light 3
			lightingShader.SetUniformVec3f("pointLightList[2].position", pointLightPositions[2]);
			lightingShader.SetUniformVec3f("pointLightList[2].ambient", glm::vec3(0.05f));
			lightingShader.SetUniformVec3f("pointLightList[2].diffuse", glm::vec3(0.8f));
			lightingShader.SetUniformVec3f("pointLightList[2].specular", glm::vec3(1.0f));
			lightingShader.SetUniform1f("pointLightList[2].constant", 1.0f);
			lightingShader.SetUniform1f("pointLightList[2].linear", 0.09f);
			lightingShader.SetUniform1f("pointLightList[2].quadratic", 0.032f);
			// point light 4
			lightingShader.SetUniformVec3f("pointLightList[3].position", pointLightPositions[3]);
			lightingShader.SetUniformVec3f("pointLightList[3].ambient", glm::vec3(0.05f));
			lightingShader.SetUniformVec3f("pointLightList[3].diffuse", glm::vec3(0.8f));
			lightingShader.SetUniformVec3f("pointLightList[3].specular", glm::vec3(1.0f));
			lightingShader.SetUniform1f("pointLightList[3].constant", 1.0f);
			lightingShader.SetUniform1f("pointLightList[3].linear", 0.09f);
			lightingShader.SetUniform1f("pointLightList[3].quadratic", 0.032f);
			// spotLight 1
			lightingShader.SetUniformVec3f("spotLightList[0].position", spotLightPositions[0]);
			lightingShader.SetUniformVec3f("spotLightList[0].direction", glm::normalize(-spotLightPositions[0]));
			lightingShader.SetUniformVec3f("spotLightList[0].ambient", glm::vec3(0.0f));
			lightingShader.SetUniformVec3f("spotLightList[0].diffuse", glm::vec3(1.0f));
			lightingShader.SetUniformVec3f("spotLightList[0].specular", glm::vec3(1.0f));
			lightingShader.SetUniform1f("spotLightList[0].constant", 1.0f);
			lightingShader.SetUniform1f("spotLightList[0].linear", 0.09f);
			lightingShader.SetUniform1f("spotLightList[0].quadratic", 0.032f);
			lightingShader.SetUniform1f("spotLightList[0].cutOff", glm::cos(glm::radians(12.5f)));
			lightingShader.SetUniform1f("spotLightList[0].outerCutOff", glm::cos(glm::radians(15.0f)));

			// spotLight 2
			lightingShader.SetUniformVec3f("spotLightList[1].position", spotLightPositions[1]);
			lightingShader.SetUniformVec3f("spotLightList[1].direction", glm::normalize(-spotLightPositions[1]));
			lightingShader.SetUniformVec3f("spotLightList[1].ambient", glm::vec3(0.0f));
			lightingShader.SetUniformVec3f("spotLightList[1].diffuse", glm::vec3(1.0f));
			lightingShader.SetUniformVec3f("spotLightList[1].specular", glm::vec3(1.0f));
			lightingShader.SetUniform1f("spotLightList[1].constant", 1.0f);
			lightingShader.SetUniform1f("spotLightList[1].linear", 0.09f);
			lightingShader.SetUniform1f("spotLightList[1].quadratic", 0.032f);
			lightingShader.SetUniform1f("spotLightList[1].cutOff", glm::cos(glm::radians(12.5f)));
			lightingShader.SetUniform1f("spotLightList[1].outerCutOff", glm::cos(glm::radians(15.0f)));

			lightingShader.SetUniformVec3f("u_viewPos", Camera->GetCameraPosition());

			lightingShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			lightingShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());

			glm::mat4 model = glm::mat4(1.0f);
			//lightingShader.SetUniformMat4f("u_model", model);

			va.Bind();
			for (int i = 0; i < sizeof(cubePositions) / sizeof(glm::vec3); i++)
			{
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, cubePositions[i]);
				float angle = 20.0f * (i + 1);
				model = glm::rotate(model, glm::radians(angle) * (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
				lightingShader.SetUniformMat4f("u_model", model);
				renderer.Draw(va, ib, lightingShader);
			}
			//renderer.Draw(va, ib, lightingShader);
			
			va.UnBind();
			lightingShader.UnBind();
			diffuseMap.UnBind();
			//////////////////

			//LightObject
			lightSrcShader.Bind();
			
			lightSrcShader.SetUniformMat4f("u_view", Camera->GetViewMatrix());
			lightSrcShader.SetUniformMat4f("u_projection", Camera->GetProjectionMatrix());
			lightSrcShader.SetUniformVec3f("u_lightColor", lightColor);

			lightVA.Bind();
			
			for (int i = 0; i < (sizeof(pointLightPositions) + sizeof(spotLightPositions)) / sizeof(glm::vec3); i++) 
			{
				model = glm::mat4(1.0f);

				glm::vec3 position(0.0f);
				if (i < 4) {
					position = pointLightPositions[i];
				}
				else {
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

