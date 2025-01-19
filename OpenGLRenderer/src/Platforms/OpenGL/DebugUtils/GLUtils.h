#pragma once

#define ASSERT(x) if(!(x)) __debugbreak();

// Makes it's own scope so be aware
#ifdef _DEBUG
#define GLScopedCall(...) \
        do { \
            GLClearError(); \
            __VA_ARGS__; \
            ASSERT(GLLogCall(#__VA_ARGS__, __FILE__, __LINE__)); \
        } while(0)

#define GLCall(x) GLClearError(); \
x;\
ASSERT(GLLogCall(#x, __FILE__, __LINE__)) 

#else
#define GLCall(...) __VA_ARGS__
#endif

void GLClearError();
bool GLLogCall(const char* function, const char* filePath, int line) noexcept;