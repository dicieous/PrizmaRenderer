#include "Model.h"


Model::Model(const std::string& filePath)
{
	LoadModel(filePath);
}

void Model::Draw(OpenGLShader& shader)
{
	for (uint32_t i = 0; i < m_Meshes.size(); i++)
	{
		m_Meshes[i].Draw(shader);
	}
}

void Model::LoadModel(const std::string& filePath)
{
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}

	m_Directory = filePath.substr(0, filePath.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;

	//vertices
	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 positions = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		glm::vec3 normals = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

		glm::vec2 TexCoords(0.0f);
		if (mesh->mTextureCoords[0])
		{
			TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}

		vertices.push_back(Vertex{ positions,normals,TexCoords });
	}

	//Indices
	for (uint32_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	//textures
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMap = LoadMaterialTexture(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMap.begin(), diffuseMap.end());

		std::vector<Texture> specularMaps = LoadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string typeName)
{
	std::vector<Texture> textures;

	for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		bool skip_Texture = false;
		for (uint32_t j = 0; j < m_Texture_Loaded.size(); j++)
		{
			if (std::strcmp(m_Texture_Loaded[j].Path.data(), str.C_Str()) == 0)
			{
				textures.push_back(m_Texture_Loaded[j]);
				skip_Texture = true;
				break;
			}
		}

		if (!skip_Texture)
		{
			Texture texture;
			std::string filepath = m_Directory + '/' + str.C_Str();
			texture.Texture = std::make_shared<Texture2D>(filepath);
			texture.Type = typeName;
			texture.Path = str.C_Str();

			textures.push_back(texture);
			m_Texture_Loaded.push_back(texture);
		}
	}

	return textures;
}
