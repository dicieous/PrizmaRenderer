#pragma once

#include "OpenGLVertexBuffer.h"
#include "VertexBufferLayout.h"

//class VertexBufferLayout;

class OpenGLVertexArray {

public:
	OpenGLVertexArray();
	~OpenGLVertexArray();

	 void Bind() const;
	 void UnBind() const;

	void AddBuffer(const OpenGLVertexBuffer& vb, const VertexBufferLayout& layout);
	uint32_t GetRendererID() const { return m_RendererID; }

private:
	uint32_t m_RendererID = 0;
	unsigned int m_AttributeIndex = 0;
};