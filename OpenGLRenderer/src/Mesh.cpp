#include "Mesh.h"

#include <string>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<Texture>& textures)
	:m_Vertices(vertices), m_Indices(indices), m_Textures(textures)
{
	SetupMesh();
}

void Mesh::Draw(OpenGLShader& shader)
{
	shader.Bind();

	uint32_t diffuseNr = 1;
	uint32_t specularNr = 1;

	for (uint32_t i = 0; i < m_Textures.size(); i++)
	{
		std::string number;
		std::string name = m_Textures[i].Type;

		if (name == "texture_diffuse")
		{
			number = std::to_string(diffuseNr++);
		}
		else if (name == "texture_specular")
		{
			number = std::to_string(specularNr++);
		}

		shader.SetUniform1i("material." + name + number, i);
		m_Textures[i].Texture->Bind(i);
	}

	m_VertexArray->Bind();
	OpenGLRenderer renderer;
	renderer.Draw(*m_VertexArray, *m_IndexBuffer, shader);
	m_VertexArray->UnBind();

}

void Mesh::SetupMesh()
{
	m_VertexArray = std::make_shared<OpenGLVertexArray>();
	m_VertexArray->Bind();

	m_VertexBuffer = std::make_shared<OpenGLVertexBuffer>(m_Vertices.size() * sizeof(Vertex), m_Vertices.data());
	m_VertexBuffer->Bind();

	m_IndexBuffer = std::make_shared<OpenGLIndexBuffer>( m_Indices.size(), m_Indices.data());
	m_IndexBuffer->Bind();

	VertexBufferLayout layout;
	layout.Push<float>(3); //Position
	layout.Push<float>(3); //Normal
	layout.Push<float>(2); //TexCoords

	m_VertexArray->AddBuffer(*m_VertexBuffer, layout);

	m_VertexArray->UnBind();

}
