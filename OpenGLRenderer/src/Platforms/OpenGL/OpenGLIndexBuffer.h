#pragma once


class OpenGLIndexBuffer {

public:
	OpenGLIndexBuffer(unsigned int count, const unsigned int* data);
	~OpenGLIndexBuffer();

	void Bind() const;
	void UnBind() const;

	inline unsigned int GetCount() const { return m_count; }

private:
	unsigned int m_RendererID;
	unsigned int m_count;
};