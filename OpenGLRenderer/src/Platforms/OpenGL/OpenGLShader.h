#pragma once

#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>

struct ShaderSouce {
	std::string VertexSource;
	std::string FragmentSource;
	std::string GeometrySource;
};

class OpenGLShader {
public:
	OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySource = "");
	OpenGLShader(const std::string& filePath);

	~OpenGLShader();

	void Bind() const;
	void UnBind() const;

	uint32_t GetRendererID() const { return m_RendererID; }

	//SetUniforms
	void SetUniform1i(const std::string& name, int value);

	void SetUniform1f(const std::string& name, float value);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

	void SetUniformVec2f(const std::string& name, const glm::vec2& value);
	void SetUniformVec3f(const std::string& name, const glm::vec3& value);
	void SetUniformVec4f(const std::string& name, const glm::vec4& value);


private:
	unsigned int CompileShader(const unsigned int type, const std::string& shaderSource);
	ShaderSouce ParseShader(const std::string& filePath);

	unsigned int CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc = "");

	unsigned int GetUniformLocation(const std::string& name);
	
private:
	std::unordered_map<std::string, int> m_UniformLocationCache;
	unsigned int m_RendererID;
};