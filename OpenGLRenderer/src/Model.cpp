#include "Model.h"


std::vector<aiTextureType> GetAvailableTextureTypes(aiMaterial* material)
{
	std::vector<aiTextureType> textureTypes;

	for (int i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
	{
		aiTextureType type = static_cast<aiTextureType>(i);
		if (material->GetTextureCount(type) > 0)
		{
			textureTypes.push_back(type); // Store the int value of the texture type
		}
	}

	return textureTypes;
}

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
	const aiScene * scene = import.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

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
		glm::vec3 normals = glm::vec3(0.0f);
		if(mesh->HasNormals()) normals = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

		glm::vec3 tangents(0.0f);
		glm::vec3 biTangents(0.0f);
		glm::vec2 TexCoords(0.0f);

		if (mesh->mTextureCoords[0])
		{
			TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			tangents = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
			biTangents = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
		}

		vertices.push_back(Vertex{ positions, normals, TexCoords, tangents, biTangents });
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

		std::vector<aiTextureType> availableTypes = GetAvailableTextureTypes(material);

		//For Debugging
		/*std::cout << "Available Texture Types for Material " << mesh->mMaterialIndex << ": ";
		for (auto type : availableTypes)
		{
			std::cout << type << " ";
		}
		std::cout << std::endl;*/

		std::vector<Texture> albedoMap = LoadMaterialTexture(material, aiTextureType_BASE_COLOR, "u_albedoMap");
		textures.insert(textures.end(), albedoMap.begin(), albedoMap.end());

		std::vector<Texture> normalMaps = LoadMaterialTexture(material, aiTextureType_NORMAL_CAMERA, "u_normalMap");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> roughnessMaps = LoadMaterialTexture(material, aiTextureType_DIFFUSE_ROUGHNESS, "u_roughnessMap");
		textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

		std::vector<Texture> metalnessMaps = LoadMaterialTexture(material, aiTextureType_METALNESS, "u_metallicMap");
		textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());

		std::vector<Texture> aoMaps = LoadMaterialTexture(material, aiTextureType_DISPLACEMENT, "u_aoMap");
		textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string typeName)
{
	std::vector<Texture> textures;
	uint32_t count = mat->GetTextureCount(type);
	for (uint32_t i = 0; i < count; i++)
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
