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
    {GL_NO_ERROR, "GL_NO_ERROR"}, // No error (useful for checking)
    {GL_INVALID_ENUM, "GL_INVALID_ENUM"}, // Invalid enumeration value
    {GL_INVALID_VALUE, "GL_INVALID_VALUE"}, // Invalid parameter value
    {GL_INVALID_OPERATION, "GL_INVALID_OPERATION"}, // Invalid operation in the current state
    {GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW"}, // Stack overflow error
    {GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW"}, // Stack underflow error
    {GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY"}, // Not enough memory for operation
    {GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION"}, // Framebuffer is not complete
    {GL_CONTEXT_LOST, "GL_CONTEXT_LOST"}, // Context lost due to a GPU reset (GL 4.5+)
    {GL_FRAMEBUFFER_UNDEFINED, "GL_FRAMEBUFFER_UNDEFINED"}, // Default framebuffer is not defined (GL 4.3+)
    {GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"}, // Incomplete framebuffer attachment
    {GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"}, // No attachments in framebuffer
    {GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"}, // Missing draw buffers
    {GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"}, // Missing read buffer
    {GL_FRAMEBUFFER_UNSUPPORTED, "GL_FRAMEBUFFER_UNSUPPORTED"}, // Unsupported framebuffer format
    {GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"}, // Mismatch in multisample settings
    {GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS, "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"} // Mismatch in layered framebuffer attachments (GL 4.5+)
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
