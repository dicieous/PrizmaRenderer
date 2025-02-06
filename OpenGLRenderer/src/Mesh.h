#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Platforms/OpenGL/OpenGLRenderer.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

//enum class TextureType { Diffuse, Specular, Normal, Height };

struct Texture
{
	std::shared_ptr<Texture2D> Texture;
	std::string Type;
	std::string Path;
};

class Mesh
{

public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<Texture>& textures);

	Mesh(const Mesh&) = default;

	// Deep copy assignment operator
	Mesh& operator=(const Mesh& other)
	{
		if (this != &other)
		{
			m_Vertices = other.m_Vertices;
			m_Indices = other.m_Indices;
			m_Textures = other.m_Textures;

			// Re-create the OpenGL buffers
			SetupMesh();
		}
		return *this;
	}

	// Add move constructor and move assignment operator
	Mesh(Mesh&& other) noexcept = default;
	Mesh& operator=(Mesh&& other) noexcept = default;

	~Mesh() = default;

	void Draw(OpenGLShader& shader);

private:
	void SetupMesh();

public:
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<Texture> m_Textures;

private:
	std::unique_ptr<OpenGLVertexArray> m_VertexArray;
	std::unique_ptr<OpenGLVertexBuffer> m_VertexBuffer;
	std::unique_ptr<OpenGLIndexBuffer> m_IndexBuffer;
};