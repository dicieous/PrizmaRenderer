#include <glad/glad.h>

#include "DebugUtils/GLUtils.h"
#include "OpenGLRenderer.h"

void OpenGLRenderer::Clear()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void OpenGLRenderer::Draw(const OpenGLVertexArray& va, const OpenGLIndexBuffer& ib, const OpenGLShader& shader) const
{
	va.Bind();
	ib.Bind();
	shader.Bind();

	GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void OpenGLRenderer::DrawTriangleStrip(const OpenGLVertexArray& va, const OpenGLIndexBuffer& ib, const OpenGLShader& shader) const
{
	va.Bind();
	ib.Bind();
	shader.Bind();

	GLCall(glDrawElements(GL_TRIANGLE_STRIP, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}
