#include <glad/glad.h>
#include <map>
#include <filesystem>
#include <iostream>

#include "GLUtils.h"

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

std::map<GLenum, const char*> ERROR_MESSAGES = {
	{GL_INVALID_ENUM, "GL_INVALID_ENUM"},
	{GL_INVALID_VALUE, "GL_INVALID_VALUE"},
	{GL_INVALID_OPERATION, "GL_INVALID_OPERATION"},
	{GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW"},
	{GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW"},
	{GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY"},
};

bool GLLogCall(const char* function, const char* filePath, int line) noexcept {
	bool noError = true;
	GLenum error;

	while ((error = glGetError()) != GL_NO_ERROR) {

		std::filesystem::path path = filePath;
		const auto& filename = path.filename().string();

		auto it = ERROR_MESSAGES.find(error);
		const auto& errorMsg = (it != ERROR_MESSAGES.end()) ?
			it->second : "UNKNOWN_ERROR";

		std::cerr << "[OpenGL Error] (" << errorMsg << "): "
			<< function << ' ' << filename << "::" << line << '\n';
		noError = false;
	}

	return noError;
}
