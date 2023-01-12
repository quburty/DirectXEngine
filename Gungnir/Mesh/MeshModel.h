#pragma once
#include "Mesh.h"

#define MAX_BONE_COUNT 1000

class MeshModel : public Mesh
{
public:
	friend class ModelAnimator;

public:
	MeshModel();
	virtual ~MeshModel();

	void ReadModel(std::wstring wfile);
	void ReadMaterial(std::wstring wfile);
	void ReadAnimation(std::wstring wfile);

	Model::ModelBone* GetBone(std::string name);
	Model::ModelBone* GetBone(uint boneIdx);
	inline uint GetBoneCount() { return model_bones.size(); }

	void UpdateBone(uint boneIdx = 0, bool bOwnUpdate = true);

	void Update() override;
	void Render() override;

private:
	void UpdateCBuffer();
	void CreateBonesBuffer();

private:
	std::vector<Model::ModelMaterial> materials;
	std::vector<Model::ModelBone> model_bones;
	std::vector<Model::ModelMesh> model_meshes;

	D3DXMATRIX worlds[MAX_BONE_COUNT]; //col-major
	ID3D11Buffer* bones_buffer;

	class ModelAnimator* animator;
};