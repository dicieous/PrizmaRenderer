#include "OpenGLVertexArray.h"



OpenGLVertexArray::OpenGLVertexArray()
	: m_RendererID(0)
{
	GLCall(glGenVertexArrays(1, &m_RendererID));
}

OpenGLVertexArray::~OpenGLVertexArray()
{
	GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void OpenGLVertexArray::Bind() const
{
	glBindVertexArray(m_RendererID);
}

void OpenGLVertexArray::UnBind() const
{
	glBindVertexArray(0);
}

void OpenGLVertexArray::AddBuffer(const OpenGLVertexBuffer& vb, const VertexBufferLayout& layout)
{
	Bind();
	vb.Bind();

	const auto& elements = layout.GetElements();
	unsigned int offset = 0;

	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		GLCall(glEnableVertexAttribArray(m_AttributeIndex));
		GLCall(glVertexAttribPointer(m_AttributeIndex, element.count, element.type, element.normalized, layout.GetStride(), (const void*) offset));
		offset += element.count * VertexBufferElement::GetTypeSize(element.type);
		m_AttributeIndex++;
	}
}
