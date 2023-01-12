#include "Framework.h"
#include "ModelRender.h"
#include "Animation/ModelAnimator.h"
#include "Mesh/Mesh.h"
#include "Shader/Shader.h"

using namespace Model;

ModelRender::ModelRender()
{
	CreateBonesBuffer();
}

ModelRender::~ModelRender()
{
	SAFE_DELETE_ARRAY(meshes);
	meshCount = 0;

	SAFE_RELEASE(bones_buffer);
}

void ModelRender::ReadModel(std::wstring wfile)
{
	std::vector<Model::ModelMesh> model_meshes;

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
		//mesh.bone = r->UInt();
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

	ReadModel(&model_meshes);
}

void ModelRender::ReadModel(std::vector<Model::ModelMesh>* model_meshes, std::vector<Model::ModelBone>* model_bones)
{
	if (model_bones != nullptr)
		this->model_bones = *model_bones;

	if (meshes != nullptr)
		SAFE_DELETE_ARRAY(meshes);

	meshes = new Mesh[model_meshes->size()];
	meshCount = model_meshes->size();

	for (uint i = 0; i < model_meshes->size(); i++)
	{
		//setting mesh

		meshes[i].vertices = model_meshes->at(i).vertices;
		meshes[i].indices = model_meshes->at(i).indices;
	}

	UpdateBone(0);
	UpdateCBuffer();
}

void ModelRender::ReadMaterial(std::wstring wfile)
{
	std::vector<Model::ModelMaterial> materials;

	std::string file;
	file.assign(wfile.begin(), wfile.end());

	Xml::XMLDocument* document = new Xml::XMLDocument();
	auto error = document->LoadFile(file.c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement* root = document->FirstChildElement();
	Xml::XMLElement* materialNode = root->FirstChildElement();

	int i = 0;
	do
	{
		ModelMaterial material;

		Xml::XMLElement* node = materialNode->FirstChildElement("Name");
		material.name = node->GetText();

		//node = materialNode->FirstChildElement("TexturesPath");
		//material.texturesPath = node->GetText();

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

		i++;
	} while (materialNode != nullptr);

	delete document;

	ReadMaterial(&materials, wfile);
}

void ModelRender::ReadMaterial(std::vector<Model::ModelMaterial>* materials, const std::wstring& matTexturesDir)
{
	for (uint i = 0; i < materials->size(); i++)
	{
		const ModelMaterial& material = materials->at(i);

		if (i < meshCount)
		{
			Shader* meshShader = meshes[i].GetShader(0);
			if (meshShader != nullptr)
			{
				std::wstring wName;
				wName.assign(material.name.begin(), material.name.end());
				std::wstring wTexturePath = Path::GetDirectoryName(matTexturesDir) + wName + L"/";

				std::wstring wFileName;

				wFileName.assign(material.diffuseMap.begin(), material.diffuseMap.end());
				wFileName = wTexturePath + wFileName;
				if (material.diffuseMap != "" && Path::ExistFile(wFileName))
				{
					meshShader->SetDiffuseMap(wFileName);
				}

				wFileName.assign(material.normalMap.begin(), material.normalMap.end());
				wFileName = wTexturePath + wFileName;
				if (material.normalMap != "" && Path::ExistFile(wFileName))
				{
					meshShader->SetNormalMap(wFileName);
				}

				wFileName.assign(material.specularMap.begin(), material.specularMap.end());
				wFileName = wTexturePath + wFileName;
				if (material.specularMap != "" && Path::ExistFile(wFileName))
				{
					meshShader->SetSpecularMap(wFileName);
				}
			}
		}
	}
}

void ModelRender::ReadAnimation(std::wstring wfile)
{
	if (animator == nullptr)
	{
		animator = new ModelAnimator(this);
	}

	animator->ReadAnimation(wfile);
}

void ModelRender::ReadAnimation(std::vector<Animation::ModelAnimation*>* model_anims)
{
	if (animator == nullptr)
	{
		animator = new ModelAnimator(this);
	}

	animator->ReadAnimation(model_anims);
}

Model::ModelBone * ModelRender::GetBone(std::string name)
{
	for (auto& bone : model_bones)
	{
		if (bone.name == name)
			return &bone;
	}

	return nullptr;
}

Model::ModelBone * ModelRender::GetBone(uint boneIdx)
{
	if (boneIdx < model_bones.size())
		return &model_bones[boneIdx];

	return nullptr;
}

void ModelRender::UpdateBone(uint boneIdx, bool bOwnUpdate)
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

void ModelRender::Update()
{
	if (animator != nullptr)
		animator->AnimationUpdate(0);
}

void ModelRender::Render()
{
	D3D::Get()->GetDeviceContext()->VSSetConstantBuffers(4, 1, &bones_buffer);

	if (meshes != nullptr)
	{
		for (uint i = 0; i < meshCount; i++)
		{
			meshes[i].Render();
		}
	}
}

void ModelRender::UpdateCBuffer()
{
	D3D11_MAPPED_SUBRESOURCE subresource;
	auto hr = D3D::Get()->GetDeviceContext()->Map(bones_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	assert(SUCCEEDED(hr));
	{
		memcpy(subresource.pData, &worlds[0], sizeof(worlds));
	}
	D3D::Get()->GetDeviceContext()->Unmap(bones_buffer, 0);
}

void ModelRender::CreateBonesBuffer()
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
