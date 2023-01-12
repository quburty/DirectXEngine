#pragma once

namespace Model
{
	struct ModelMaterial;
	struct ModelBone;
	struct ModelMesh;
}

namespace Animation
{
	struct KeyFrame;
	struct ModelAnimation;
}

struct MaterialMap
{
	void* data = nullptr;
	uint mapWidth = 0;
	std::wstring fileName = L"";

public:
	~MaterialMap()
	{
		SAFE_FREE(data);
	}
};

class Converter final
{
public:
	void ReadFBX(std::wstring wfile);

private:
	void ReadBone(aiNode* node, const aiScene* scene, int parent);
	void ReadMesh(uint meshIdx, const aiScene* scene);

	void ReadMaterial(const aiScene* scene);
	void ReadTextures(const aiScene* scene, uint idx);

public:
	void ReadFBXAnim(std::wstring wfile);

private:
	typedef std::pair<float, Animation::KeyFrame> KeyFrameTime;

	void AssimpToTemp(
		std::vector<std::vector<KeyFrameTime>>& bones_animation,
		const aiScene* scene,
		uint idx,
		float& length);
	void TempToModelAnim(
		const std::vector<std::vector<KeyFrameTime>>& bones_animation,
		const aiScene* scene,
		uint idx,
		const float& length);

public:
	void WriteModel(std::wstring wfileName);

private:
	void WriteBoneFile(BinaryWriter * w);
	void WriteMeshFile(BinaryWriter * w);

public:
	void WriteMaterialsFile(std::wstring wfileName);
	void WriteMaterialFile(std::wstring wfileName, uint idx);
	void WriteAnimationFile(std::wstring wfileName, uint idx);

private:
	void WriteTexture(std::wstring wPathName, uint idx);

public:
	void Clear();

public:
	void StartModelRender();
	void EndModelRender();

	void Update();
	void Render();

public:
	bool IsModelAvailable();
	bool IsAnimAvailable();

public:
	class ModelRender* GetModel() { return modelRender; }

private:
	std::vector<Model::ModelMaterial> materials;
	std::vector<Model::ModelBone> model_bones;
	std::vector<Model::ModelMesh> model_meshes;
	std::vector<Animation::ModelAnimation*> model_anims;

	MaterialMap diffuseMap[10];
	MaterialMap normalMap[10];
	MaterialMap specularMap[10];

	class ModelRender* modelRender;
};