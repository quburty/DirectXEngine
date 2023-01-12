#pragma once

#define MAX_BONE_COUNT 1000

namespace Model
{
	struct ModelBone
	{
		int parent;
		D3DXMATRIX l_transform; //local, row-major
		D3DXMATRIX w_transform; //world, row-major
		D3DXMATRIX w_transform_org; //world, row-major
		std::string name;
		std::vector<uint> childs;
	};

	struct ModelMesh
	{
		std::vector<Vertex> vertices; //local
		std::vector<uint> indices; //local
		uint material;
	};

	struct ModelMaterial
	{
		std::string name;

		std::string diffuseMap;
		D3DXCOLOR diffuseColor;

		std::string normalMap;

		std::string specularMap;
		D3DXCOLOR specularColor;
	};
}

namespace Animation
{
	struct ModelAnimation;
}

class ModelRender final
{
public:
	friend class ModelAnimator;

public:
	ModelRender();
	virtual ~ModelRender();

public:
	void ReadModel(std::wstring wfile);
	void ReadModel(std::vector<Model::ModelMesh>* model_meshes,
		std::vector<Model::ModelBone>* model_bones = nullptr);

public:
	void ReadMaterial(std::wstring wfile);
	void ReadMaterial(std::vector<Model::ModelMaterial>* materials,
		const std::wstring& matTexturesDir);

public:
	void ReadAnimation(std::wstring wfile);
	void ReadAnimation(std::vector<Animation::ModelAnimation*> * model_anims);

	Model::ModelBone* GetBone(std::string name);
	Model::ModelBone* GetBone(uint boneIdx);
	inline uint GetBoneCount() { return model_bones.size(); }

	class Mesh* GetMeshes() { return meshes; }
	uint GetMeshCount() { return meshCount; }

	void UpdateBone(uint boneIdx = 0, bool bOwnUpdate = true);

	void Update();
	void Render();

private:
	void UpdateCBuffer();
	void CreateBonesBuffer();

private:
	std::vector<Model::ModelBone> model_bones;

	D3DXMATRIX worlds[MAX_BONE_COUNT]; //col-major
	ID3D11Buffer* bones_buffer;

	class Mesh* meshes;
	uint meshCount = 0;

	class ModelAnimator* animator;
};