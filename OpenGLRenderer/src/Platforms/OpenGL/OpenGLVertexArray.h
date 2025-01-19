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

private:
	unsigned int m_RendererID;
	unsigned int m_AttributeIndex = 0;
};