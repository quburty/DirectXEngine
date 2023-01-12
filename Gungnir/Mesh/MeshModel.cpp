#include "Framework.h"
#include "MeshModel.h"
#include "Animation/ModelAnimator.h"

using namespace Model;

MeshModel::MeshModel()
	: Mesh()
{
	CreateBonesBuffer();
}

MeshModel::~MeshModel()
{
	SAFE_RELEASE(bones_buffer);
}

void MeshModel::ReadModel(std::wstring wfile)
{
	BinaryReader* r = new BinaryReader();
	r->Open(wfile);

	uint size;

	//Bone
	size = r->UInt();
	model_bones.resize(size);

	for (auto& bone : model_bones)
	{
		bone.parent = r->Int();
		bone.l_transform = r->Matrix();
		
		bone.name = r->String();
		size = r->UInt();
		bone.childs.resize(size);
		for (auto& child : bone.childs)
			child = r->UInt();

		if (bone.parent >= 0)
		{
			const D3DXMATRIX& parent = model_bones[bone.parent].w_transform;
			bone.w_transform = bone.l_transform * parent;
			bone.w_transform_org = bone.l_transform * parent;
		}
		else
		{
			bone.w_transform = bone.l_transform;
			bone.w_transform_org = bone.l_transform;
		}
	}

	//Mesh
	size = r->UInt();
	model_meshes.resize(size);

	for (auto& mesh : model_meshes)
	{
		mesh.bone = r->UInt();
		size = r->UInt();
		mesh.vertices.resize(size);
		for (auto& v : mesh.vertices)
		{
			v.position = r->Vector3();
			v.uv = r->Vector2();
			v.normal = r->Vector3();
			v.tangent = r->Vector3();
			
			size = r->UInt();
			for (uint i = 0; i < size; i++)
			{
				uint index = r->UInt();
				float weight = r->Float();

				if (MODEL_MAX_WEIGHTS_NUM > i)
				{
					v.indices[i] = index;
					v.weights[i] = weight;
				}
			}
		}
		size = r->UInt();
		mesh.indices.resize(size);
		for (auto& i : mesh.indices)
			i = r->UInt();
	}

	r->Close();
	delete r;

	uint count = 0;
	for (auto& mesh : model_meshes)
	{
		count = vertices.size();

		for (auto vertex : mesh.vertices)
		{
			D3DXVECTOR4 pos = D3DXVECTOR4(vertex.position, 1.0f);

			D3DXVec4Transform(&pos, &pos, &model_bones[mesh.bone].w_transform);

			memcpy(&vertex, &pos, sizeof(D3DXVECTOR3));

			vertices.push_back(vertex);
		}

		for (auto& index : mesh.indices)
		{
			indices.push_back(index + count);
		}
	}

	UpdateBone(0);
	UpdateCBuffer();
}

void MeshModel::ReadMaterial(std::wstring wfile)
{
	std::string file;
	file.assign(wfile.begin(), wfile.end());

	Xml::XMLDocument* document = new Xml::XMLDocument();
	auto error = document->LoadFile(file.c_str());
	assert(error == Xml::XML_SUCCESS);


	Xml::XMLElement* root = document->FirstChildElement();
	Xml::XMLElement* materialNode = root->FirstChildElement();

	do
	{
		ModelMaterial material;

		Xml::XMLElement* node = materialNode->FirstChildElement("Name");
		material.name = node->GetText();

		node = materialNode->FirstChildElement("DiffuseFile");
		material.diffuseMap = node->GetText();

		node = materialNode->FirstChildElement("NormalFile");
		material.normalMap = node->GetText();

		node = materialNode->FirstChildElement("SpecularFile");
		material.specularMap = node->GetText();

		node = materialNode->FirstChildElement("Diffuse");
		material.diffuseColor.r = node->FloatAttribute("R");
		material.diffuseColor.g = node->FloatAttribute("G");
		material.diffuseColor.b = node->FloatAttribute("B");
		material.diffuseColor.a = node->FloatAttribute("A");

		node = materialNode->FirstChildElement("Specular");
		material.specularColor.r = node->FloatAttribute("R");
		material.specularColor.g = node->FloatAttribute("G");
		material.specularColor.b = node->FloatAttribute("B");
		material.specularColor.a = node->FloatAttribute("A");

		materials.push_back(material);

		materialNode = materialNode->NextSiblingElement();
	} while (materialNode != nullptr);

	delete document;
}

void MeshModel::ReadAnimation(std::wstring wfile)
{
	if (animator == nullptr)
	{
		animator = new ModelAnimator(this);
	}

	animator->ReadAnimation(wfile);
}

Model::ModelBone * MeshModel::GetBone(std::string name)
{
	for (auto& bone : model_bones)
	{
		if (bone.name == name)
			return &bone;
	}

	return nullptr;
}

Model::ModelBone * MeshModel::GetBone(uint boneIdx)
{
	if (boneIdx < model_bones.size())
		return &model_bones[boneIdx];

	return nullptr;
}

void MeshModel::UpdateBone(uint boneIdx, bool bOwnUpdate)
{
	auto& bone = model_bones[boneIdx];

	if (bOwnUpdate)
	{
		if (bone.parent >= 0)
			bone.w_transform = bone.l_transform * model_bones[bone.parent].w_transform;
		else
			bone.w_transform = bone.l_transform;
	}

	D3DXMATRIX inv;
	D3DXMatrixInverse(&inv, nullptr, &bone.w_transform_org);
	worlds[boneIdx] = inv * bone.w_transform;

	D3DXMatrixTranspose(&worlds[boneIdx], &worlds[boneIdx]);

	for (uint i = 0; i < bone.childs.size(); i++)
		UpdateBone(bone.childs[i]);
}

void MeshModel::UpdateCBuffer()
{
	D3D11_MAPPED_SUBRESOURCE subresource;
	auto hr = D3D::Get()->GetDeviceContext()->Map(bones_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	assert(SUCCEEDED(hr));
	{
		memcpy(subresource.pData, &worlds[0], sizeof(worlds));
	}
	D3D::Get()->GetDeviceContext()->Unmap(bones_buffer, 0);
}

void MeshModel::Update()
{
	Super::Update();

	if (animator != nullptr)
		animator->AnimationUpdate(0);
}

void MeshModel::Render()
{
	D3D::Get()->GetDeviceContext()->VSSetConstantBuffers(4, 1, &bones_buffer);
}

void MeshModel::CreateBonesBuffer()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(worlds);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &worlds[0];
	data.SysMemPitch = sizeof(D3DXMATRIX);

	D3D::Get()->GetDevice()->CreateBuffer(&desc, &data, &bones_buffer);
}
