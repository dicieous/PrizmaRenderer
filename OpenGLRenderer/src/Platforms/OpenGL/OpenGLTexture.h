#pragma once

#include <iostream>

class Texture2D {

public:
	Texture2D(const std::string& filePath);
	~Texture2D();

	void Bind(uint32_t slot = 0) const;
	void UnBind(uint32_t slot = 0) const;

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }

	inline int GetRendererID() const { return m_RendererID; }

private:
	unsigned int m_RendererID;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_width, m_height, m_BPP;
};