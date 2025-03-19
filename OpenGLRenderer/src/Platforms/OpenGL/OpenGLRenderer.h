#pragma once

#include "OpenGLShader.h"
#include "OpenGLIndexBuffer.h"
#include "OpenGLVertexArray.h"
#include "OpenGLTexture.h"

class OpenGLRenderer {

public:
    void Clear();
    void Draw(const OpenGLVertexArray& va, const OpenGLIndexBuffer& ib, const OpenGLShader& shader) const;
    void DrawTriangleStrip(const OpenGLVertexArray& va, const OpenGLIndexBuffer& ib, const OpenGLShader& shader) const;

private: 
};