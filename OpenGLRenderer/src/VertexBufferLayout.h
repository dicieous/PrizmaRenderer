#pragma once

#include <vector>
#include <glad/glad.h>
#include <stdexcept>

#include "Platforms/OpenGL/DebugUtils/GLUtils.h"

struct VertexBufferElement {
	uint32_t type;
	uint32_t count;
	unsigned char normalized;

	static uint32_t GetTypeSize(uint32_t type) 
	{
		switch (type)
		{
			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4;
			case GL_UNSIGNED_BYTE: return 1;
		}

		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout {

public:
	VertexBufferLayout()
		: m_Stride(0) {}

	template<typename T>
	void Push(uint32_t count) 
	{
		std::runtime_error(false);
	}

	template<>
	void Push<float>(uint32_t count) 
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::GetTypeSize(GL_FLOAT);
	}

	template<>
	void Push<uint32_t>(uint32_t count)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::GetTypeSize(GL_UNSIGNED_INT);
	}

	template<>
	void Push<unsigned char>(uint32_t count)
	{
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += count * VertexBufferElement::GetTypeSize(GL_UNSIGNED_BYTE);
	}

	inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
	inline uint32_t GetStride() const { return m_Stride; }

	void AddStride(uint32_t stride) { m_Stride += stride;  }

private:
	uint32_t m_Stride;
	std::vector<VertexBufferElement> m_Elements;
};