#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

#include "Platforms/OpenGL/OpenGLRenderer.h"
#include "Mesh.h"


class Model{

public:
	Model(const std::string& filePath);

	void Draw(OpenGLShader& shader);
	std::vector<Mesh> GetMeshesList() const { return m_Meshes; }
	std::vector<Texture> GetTextureList() const { return m_Texture_Loaded; }

private:
	void LoadModel(const std::string& filePath);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Texture> LoadMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string typeName);

private:
	std::vector<Mesh> m_Meshes;
	std::vector<Texture> m_Texture_Loaded;
	std::string m_Directory;
};