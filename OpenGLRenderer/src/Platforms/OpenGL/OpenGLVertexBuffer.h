#pragma once


class OpenGLVertexBuffer {

public:
	OpenGLVertexBuffer(unsigned int size, const void* data);
	~OpenGLVertexBuffer();

	void Bind() const;
	void UnBind() const;

private:
	unsigned int m_RendererID;
};