
<h1 align="center">
Prizma Renderer
</h1>

## **Overview**
PrizmaRenderer is a rendering engine that I developed as a learning project for graphics programming. It is not intended for commercial use, but anyone interested can clone this repository, experiment with it, or make their own modifications. The engine abstracts most of the cumbersome processes involved in graphics programming, requiring mainly shader creation to define custom rendering behaviors.

<br>

## **Features**


| **Feature**   |  **Description**        |
| :-------- |  :------------------------- |
| OpenGL Integration |  *Utilizes OpenGL for rendering, ensuring cross-platform compatibility and access to a wide range of graphics functionalities.* |
| Blinn-Phong Shading |  *Implements the Blinn-Phong lighting model for better specular highlights.* |
| Deferred Rendering |  *Implements a deferred shading pipeline for optimized rendering performance.* |
| Bloom |  *Adds a glow effect to bright areas, enhancing realism.* |
| Deferred Rendering |  *Implements a deferred shading pipeline for optimized rendering performance.* |
| Physically Based Rendering (PBR) |  *Supports PBR techniques for realistic lighting and material rendering.* |
| Image Based Lighting (IBL) |  *Uses environment maps for realistic lighting.* |
| Screen Space Ambient Occlusion (SSAO) |  *Enhances visual depth and realism by simulating ambient occlusion.* |
| Shadow Mapping |  *Implements dynamic shadow casting techniques for more immersive scenes.* |
| Post-Processing Effects |  *Includes effects such as bloom, HDR, and tone mapping.* |
| Shader Support |  *Incorporates GLSL shaders, allowing for customizable and dynamic visual effects.* |
| Geometry Shader Support |  *Enhances texture depth perception using displacement techniques.* |
| Parallax Mapping |  *Incorporates GLSL shaders, allowing for customizable and dynamic visual effects.* |
| Modular Architecture |  *Designed with a modular approach to enable easy extension and integration of additional features.* |

<br>

## **Showcase**

**Blending**
![Blending](https://github.com/dicieous/PrizmaRenderer/blob/a926bb8417cccfa1820962339e11d4421049bde1/OpenGLRenderer/readme/Blend.png)

**HDRBloom**
![HDRBloom](https://github.com/dicieous/PrizmaRenderer/blob/985876e97dfe0c49059ba6893e4bd6928f81244b/OpenGLRenderer/readme/HDRBloom.png)

**NormalMapping**
![NormalMapping](https://github.com/dicieous/PrizmaRenderer/blob/985876e97dfe0c49059ba6893e4bd6928f81244b/OpenGLRenderer/readme/NormalMapping.png)

**ParallaxMapping**
![ParallaxMapping](https://github.com/dicieous/PrizmaRenderer/blob/985876e97dfe0c49059ba6893e4bd6928f81244b/OpenGLRenderer/readme/ParallaxMapping.png)

**Post Processing**
|           |                             |               
| :-------- |  :------------------------- |
|GreyScale ![GreyScale](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/PostProcess1.png)| Saturation ![Saturation](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/PostProcess3.png) |
|Blur ![Blur](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/PostProcess4.png) | Color Inversion ![Color Inversion](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/PostProcess2.png) |

**Shadow Mapping**

![Shadow Mapping](https://github.com/dicieous/PrizmaRenderer/blob/8b46a47a501d026705c869e9034b405f9a7fe28e/OpenGLRenderer/readme/shadowMapping.gif)

**Geometry Vertex Transformation**
![Geometry Vertex Transformation](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/GeometryShader.png)

**DeferredShading**
![DeferredShading](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/DeferredShading.png)

**AmbientOcculsion**
![AmbientOcculsion](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/AmbientOcculsion.png)

**Physically Based Rendering**
|           |                             |               
| :-------- |  :------------------------- |
|![PBR1](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/PBR1.png)| ![PBR2](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/PBR2.png) |
| ![PBR3](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/PBR3.png) | ![PBR4](https://github.com/dicieous/PrizmaRenderer/blob/75c9c3e998a93ba33781fd99b36fe0eb7fc99569/OpenGLRenderer/readme/PBR4.png) |

<br>

## **Getting Started**

### Dependencies
- **Development Environment**: Ensure you have a C++ development environment set up. This includes a compatible compiler and development tools(Favourably Visual Studios and with MSVC).

- [**OpenGL**](https://www.opengl.org/): Verify that your system supports OpenGL and that the necessary drivers are installed.

 - [**GLAD**](https://glad.dav1d.de/): Used for loading OpenGL function pointers.

- [**ImGui**](https://github.com/ocornut/imgui): Immediate Mode GUI library for debugging and UI development.

- [**GLM**](https://github.com/g-truc/glm): A math library for handling matrices and vectors in graphics programming.

### Building the Project

1. **Clone the Repository**:
```bash
  git clone https://github.com/dicieous/PrizmaRenderer.git
```

2. **Navigate to the Project Directory**:

```bash
  cd PrizmaRenderer
```

3.  **Open the Solution File**: Locate the ```OpenGLRenderer.sln``` file and open it with your preferred IDE that supports solution files.

4. **Configure Build Settings**: Ensure that the include directories and library paths for GLFW, GLAD, ImGui, and GLM are correctly set in your project settings.

5. **Build the Project**: Compile the solution using your IDE's build functionality.

### Running the Application

- After a successful build, execute the compiled binary. A window should appear, demonstrating the rendering capabilities of PrizmaRenderer.
-  ```Left-Arrow``` Key and ```Right-Arrow``` Key are used to change the HDRI Maps & ```Z``` and ```C``` Key to Navigate between the models.
- ```WASD``` and ```Q``` & ```E``` should be used for camera Navigation with ```SCROLL_WHEEL``` to change POV.

<br>

## **Project Structure**

```OpenGLRenderer/```: Contains the source code and resources for the renderer.

```OpenGLRenderer.sln```: Solution file for building the project.

```.gitignore```: Specifies files and directories to be ignored by Git.

```LICENSE```: The project's license information.

```README.md```: This readme file.

```images/```: Folder containing screenshots showcasing rendering capabilities.

<br>

## **License**

This project is licensed under the [Apache-2.0](https://choosealicense.com/licenses/apache-2.0/) License. See the ```LICENSE``` file for more details.

<br>

## **Acknowledgements**

- **GLFW**: For window creation and input handling.

- **GLAD**: For managing OpenGL function pointers.

- **ImGui**: For UI development.

- **GLM**: For mathematical computations in graphics.

- **LearnOpenG**L: For Tutorials and Engaging Learning Experience

- **OpenGL Community**: For continuous support and resources.


