#include <fstream>
#include <string>
#include <sstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "OpenGLShader.h"
#include "DebugUtils/GLUtils.h"


OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySource)
	:m_RendererID(0)
{
	m_RendererID = CreateShader(vertexSrc, fragmentSrc, geometrySource);
}

OpenGLShader::OpenGLShader(const std::string& filePath)
	:m_RendererID(0)
{
	ShaderSouce source = ParseShader(filePath);
	m_RendererID = CreateShader(source.VertexSource, source.FragmentSource, source.GeometrySource);
}

OpenGLShader::~OpenGLShader()
{
	GLCall(glDeleteProgram(m_RendererID));
}

unsigned int OpenGLShader::CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc)
{
	//Create the Shader Program
	unsigned int shaderProgram = glCreateProgram();

	unsigned int vS = CompileShader(GL_VERTEX_SHADER, vertexSrc);
	unsigned int fS = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

	GLCall(glAttachShader(shaderProgram, vS));
	GLCall(glAttachShader(shaderProgram, fS));
	
	unsigned int gS = 0;
	if (!geometrySrc.empty())
	{
		gS = CompileShader(GL_GEOMETRY_SHADER, geometrySrc);
		GLCall(glAttachShader(shaderProgram, gS));
	}

	GLCall(glLinkProgram(shaderProgram));
	GLCall(glValidateProgram(shaderProgram));

	GLCall(glDeleteShader(vS));
	GLCall(glDeleteShader(fS));
	if(gS)GLCall(glDeleteShader(gS));

	int success;
	GLCall(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success));
	if (success == GL_FALSE)
	{
		int length;
		GLCall(glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length));

		if (length > 0)
		{
			std::vector<char> message(length);
			glGetProgramInfoLog(shaderProgram, length, nullptr, message.data());

			std::cout << "Failed to Create Program \n" << std::endl;
			std::cout << message.data() << std::endl;
		}

		GLCall(glDeleteProgram(shaderProgram));

		return 0;
	}


	return shaderProgram;
}


void OpenGLShader::Bind() const
{
	glUseProgram(m_RendererID);
}

void OpenGLShader::UnBind() const
{
	glUseProgram(0);
}

unsigned int OpenGLShader::CompileShader(const unsigned int type, const std::string& shaderSource)
{
	unsigned int shader = glCreateShader(type);
	const char* src = shaderSource.c_str();
	GLCall(glShaderSource(shader, 1, &src, NULL));
	GLCall(glCompileShader(shader));

	int success;
	GLCall(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));

	if (!success)
	{
		int length;
		GLCall(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)_malloca(length * sizeof(char));

		GLCall(glGetShaderInfoLog(shader, length, &length, message));
		switch (type)
		{
		case GL_VERTEX_SHADER:
			std::cout << "Couldn't Compile Vertex Shader" << std::endl;
			break;
		case GL_FRAGMENT_SHADER:
			std::cout << "Couldn't Compile Fragment Shader" << std::endl;
			break;
		case GL_GEOMETRY_SHADER:
			std::cout << "Couldn't Compile Geometry Shader" << std::endl;
			break;
		default:
			std::cout << "Couldn't Compile unidentified Shader" << std::endl;
			break;
		}

		std::cout << message << std::endl;

		GLCall(glDeleteShader(shader));
		return 0;
	}

	return shader;
}

ShaderSouce OpenGLShader::ParseShader(const std::string& filePath)
{
	std::ifstream stream(filePath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2,
	};

	std::string line;
	std::stringstream ss[3];

	ShaderType type = ShaderType::NONE;

	while (std::getline(stream, line))
	{
		if (line.find("#type") != std::string::npos)
		{
			{
				if (line.find("vertex") != std::string::npos)
				{
					//set mode to vertex
					type = ShaderType::VERTEX;
				}
				else if (line.find("fragment") != std::string::npos || line.find("pixel") != std::string::npos)
				{
					//set mode to fragment
					type = ShaderType::FRAGMENT;

				}
				else if (line.find("geometry") != std::string::npos)
				{
					//set mode to geometry
					type = ShaderType::GEOMETRY;

				}
			}
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str(), ss[2].str() };
}

unsigned int OpenGLShader::GetUniformLocation(const std::string& name)
{
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[name];

	GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));

	if (location == -1)
		std::cout << "Warning: uniform In " << name.c_str() << " doesn't exist!" << std::endl;

	m_UniformLocationCache[name] = location;

	return location;
}

void OpenGLShader::SetUniform1i(const std::string& name, int value)
{
	GLCall(glUniform1i(GetUniformLocation(name), value));
}

void OpenGLShader::SetUniform1f(const std::string& name, float value)
{
	GLCall(glUniform1f(GetUniformLocation(name), value));
}

void OpenGLShader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
	GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)));
}

void OpenGLShader::SetUniformVec2f(const std::string& name, const glm::vec2& value)
{
	GLCall(glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value)));
}

void OpenGLShader::SetUniformVec3f(const std::string& name, const glm::vec3& value)
{
	GLCall(glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value)));
}

void OpenGLShader::SetUniformVec4f(const std::string& name, const glm::vec4& value)
{
	GLCall(glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value)));
}

