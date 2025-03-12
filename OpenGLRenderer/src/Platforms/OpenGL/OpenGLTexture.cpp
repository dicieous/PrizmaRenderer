#include <glad/glad.h>
#include <stb_image.h>

#include "OpenGLTexture.h"
#include "DebugUtils/GLUtils.h"



Texture2D::Texture2D(const std::string& filePath)
	:m_RendererID(0), m_FilePath(filePath), m_LocalBuffer(nullptr), m_width(0), m_height(0), m_BPP(0)
{
	stbi_set_flip_vertically_on_load(0);
	m_LocalBuffer = stbi_load(filePath.c_str(), &m_width, &m_height, &m_BPP, 0);

	if (m_LocalBuffer)
	{
		GLenum internalFormat = (m_BPP == 4) ? GL_SRGB_ALPHA : GL_SRGB;
		GLenum dataFormat = (m_BPP == 4) ? GL_RGBA : GL_RGB;

		GLCall(glGenTextures(1, &m_RendererID));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		GLint texWrappingMode = m_BPP == 4 ? GL_CLAMP_TO_EDGE : GL_REPEAT;

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texWrappingMode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texWrappingMode));

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, dataFormat, GL_UNSIGNED_BYTE, m_LocalBuffer));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	}
	else
	{
		ASSERT(false);
	}

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	if (m_LocalBuffer) {
		stbi_image_free(m_LocalBuffer);
	}
}

Texture2D::~Texture2D()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture2D::Bind(unsigned int slot) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture2D::UnBind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
