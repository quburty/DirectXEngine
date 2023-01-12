#include "stdafx.h"
#include "Converter.h"
#include "Model/ModelRender.h"
#include "Mesh/Mesh.h"
#include "Shader/ShaderBasic.h"
#include "Animation/ModelAnimator.h"

using namespace Model;
using namespace Animation;

using namespace std;

void Converter::ReadFBX(std::wstring wfile)
{
	materials.clear();
	model_bones.clear();
	model_meshes.clear();
	model_anims.clear();

	BinaryReader reader;
	reader.Open(wfile);
	void* fileData = malloc(reader.FileSize());
	reader.Byte(&fileData, reader.FileSize());

	Assimp::Importer* importer = new Assimp::Importer();
	const aiScene* scene = importer->ReadFileFromMemory
	(
		fileData,
		reader.FileSize(),
		aiProcess_ConvertToLeftHanded
		| aiProcess_Triangulate
		| aiProcess_GenUVCoords
		| aiProcess_GenNormals
		| aiProcess_CalcTangentSpace
	);
	assert(scene != NULL);

	free(fileData);

	reader.Close();

	ReadBone(scene->mRootNode, scene, -1);

	for (uint i = 0; i < scene->mNumMeshes; i++)
	{
		ReadMesh(i, scene);
	}

	ReadMaterial(scene);

	delete importer;
}

void Converter::ReadBone(aiNode * node, const aiScene * scene, int parent)
{
	ModelBone bone;

	bone.parent = parent;
	bone.name = node->mName.C_Str();
	bone.l_transform = D3DXMATRIX(node->mTransformation[0]);
	D3DXMatrixTranspose(&bone.l_transform, &bone.l_transform);

	if (parent >= 0)
		bone.w_transform = bone.l_transform*model_bones[parent].w_transform;
	else
		bone.w_transform = bone.l_transform;

	bone.w_transform_org = bone.w_transform;

	uint idx = model_bones.size();

	if (parent >= 0)
		model_bones[parent].childs.push_back(idx);

	model_bones.push_back(bone);

	for (uint i = 0; i < node->mNumChildren; i++)
		ReadBone(node->mChildren[i], scene, idx);
}

void Converter::ReadMesh(uint meshIdx, const aiScene* scene)
{
	ModelMesh modelMesh;

	aiMesh* mesh = scene->mMeshes[meshIdx];

	//Vertices
	for (uint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex v;

		memcpy(&v.position, &mesh->mVertices[i], sizeof(D3DXVECTOR3));

		memcpy(&v.uv, &mesh->mTextureCoords[0][i], sizeof(D3DXVECTOR2));

		memcpy(&v.normal, &mesh->mNormals[i], sizeof(D3DXVECTOR3));
		memcpy(&v.tangent, &mesh->mTangents[i], sizeof(D3DXVECTOR3));

		modelMesh.vertices.push_back(v);
	}

	//bones
	for (uint i = 0; i < mesh->mNumBones; i++)
	{
		aiBone* bone = mesh->mBones[i];
		uint boneIdx = UINT_MAX;

		for (uint j = 0; j < model_bones.size(); j++)
		{
			if (model_bones[j].name == bone->mName.C_Str())
			{
				boneIdx = j;
				break;
			}
		}
		assert(boneIdx != UINT_MAX);

		for (uint j = 0; j < bone->mNumWeights; j++)
		{
			aiVertexWeight& vw = bone->mWeights[j];

			modelMesh.vertices[vw.mVertexId].AddIndexAndWeight(boneIdx, vw.mWeight);
		}
	}

	//Indices
	for (uint i = 0; i < mesh->mNumFaces; i++)
	{
		auto& face = mesh->mFaces[i];

		for (uint j = 0; j < face.mNumIndices; j++)
			modelMesh.indices.push_back(face.mIndices[j]);
	}

	modelMesh.material = mesh->mMaterialIndex;

	model_meshes.push_back(modelMesh);
}

void Converter::ReadMaterial(const aiScene* scene)
{
	for (uint i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* srcMaterial = scene->mMaterials[i];
		ModelMaterial material;

		material.name = srcMaterial->GetName().C_Str();

		aiColor3D color;

		srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		memcpy(material.diffuseColor, &color, sizeof(float) * 3);
		material.diffuseColor.a = 1.0f;

		srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
		memcpy(material.specularColor, &color, sizeof(float) * 3);

		srcMaterial->Get(AI_MATKEY_SHININESS, material.specularColor.a);

		std::string temp;
		aiString file;

		srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
		material.diffuseMap = Path::GetFileName(file.C_Str());

		srcMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
		material.specularMap = Path::GetFileName(file.C_Str());

		srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
		material.normalMap = Path::GetFileName(file.C_Str());

		materials.push_back(material);

		ReadTextures(scene, i);
	}
}

void Converter::ReadTextures(const aiScene* scene, uint idx)
{
	if (scene->HasTextures() == false)
		return;

	for (uint i = 0; i < scene->mNumTextures; i++)
	{
		aiTexture* texture = scene->mTextures[i];
		const std::string& str_file = Path::GetFileName(texture->mFilename.C_Str());

		if (materials[idx].diffuseMap == str_file)
		{
			if (texture->mHeight < 1 && texture->mWidth > 0)
			{
				diffuseMap[idx].data = malloc(texture->mWidth);
				memcpy(diffuseMap[idx].data, texture->pcData, texture->mWidth);
				diffuseMap[idx].mapWidth = texture->mWidth;
				diffuseMap[idx].fileName.assign(str_file.begin(), str_file.end());
			}
			else
				assert(false);
		}

		if (materials[idx].normalMap == str_file)
		{
			if (texture->mHeight < 1 && texture->mWidth > 0)
			{
				normalMap[idx].data = malloc(texture->mWidth);
				memcpy(normalMap[idx].data, texture->pcData, texture->mWidth);
				normalMap[idx].mapWidth = texture->mWidth;
				normalMap[idx].fileName.assign(str_file.begin(), str_file.end());
			}
			else
				assert(false);
		}

		if (materials[idx].specularMap == str_file)
		{
			if (texture->mHeight < 1 && texture->mWidth > 0)
			{
				specularMap[idx].data = malloc(texture->mWidth);
				memcpy(specularMap[idx].data, texture->pcData, texture->mWidth);
				specularMap[idx].mapWidth = texture->mWidth;
				specularMap[idx].fileName.assign(str_file.begin(), str_file.end());
			}
			else
				assert(false);
		}
	}
}

void Converter::ReadFBXAnim(std::wstring wfile)
{
	model_anims.clear();

	BinaryReader reader;
	reader.Open(wfile);
	void* fileData = malloc(reader.FileSize());
	reader.Byte(&fileData, reader.FileSize());

	Assimp::Importer* importer = new Assimp::Importer();
	const aiScene* scene = importer->ReadFileFromMemory
	(
		fileData,
		reader.FileSize(),
		aiProcess_ConvertToLeftHanded
		| aiProcess_Triangulate
		| aiProcess_GenUVCoords
		| aiProcess_GenNormals
		| aiProcess_CalcTangentSpace
	);
	assert(scene != NULL);

	free(fileData);

	reader.Close();

	//Reading Animation Clip.
	string nodeName = scene->mRootNode->mName.data;//루트본 이름
	uint num = scene->mNumAnimations;

	typedef std::pair<float, KeyFrame> KeyFrameTime;
	std::vector<std::vector<KeyFrameTime>> bones_animation; //일종의 임시배열

	bones_animation.resize(model_bones.size()); //본 개수만큼 할당

	float length = 0.0f;
	AssimpToTemp(bones_animation, scene, 0, length);
	TempToModelAnim(bones_animation, scene, 0, length);

	SAFE_DELETE(importer);
}

void Converter::AssimpToTemp(std::vector<std::vector<KeyFrameTime>>& bones_animation, const aiScene* scene, uint idx, float& length)
{
	auto anim = scene->mAnimations[idx];
	uint numC = anim->mNumChannels;

	for (uint j = 0; j < numC; j++)
	{
		auto channel = anim->mChannels[j]; //채널: 본마다
		string cNodeName = channel->mNodeName.data; //여기서의 nodename이 본 이름이 된다

		uint boneIdx = UINT_MAX;
		for (uint k = 0; k < model_bones.size(); k++)
		{
			if (cNodeName == model_bones[k].name)
			{
				boneIdx = k;
				break;
			}
		}
		if (boneIdx == UINT_MAX)//본을 못찾았을 경우
		{
			assert(false);
		}

		auto& bone_anim = bones_animation[boneIdx];

		uint p = 0, r = 0, s = 0;
		while (true)
		{
			KeyFrame frame;
			float time;

			double minValue = DBL_MAX;

			if (p < channel->mNumPositionKeys)
				minValue = min(minValue, channel->mPositionKeys[p].mTime);

			if (r < channel->mNumRotationKeys)
				minValue = min(minValue, channel->mRotationKeys[r].mTime);

			if (s < channel->mNumScalingKeys)
				minValue = min(minValue, channel->mScalingKeys[s].mTime);

			if (minValue == DBL_MAX) //모든 배열이 끝났을 경우 루프문 종료
				break;

			const auto& pKey = channel->mPositionKeys[p]; //key에서 시간마다의 값이 들어있음
			const auto& rKey = channel->mRotationKeys[r];
			const auto& sKey = channel->mScalingKeys[s];

			if (Math::IsNearlySame(minValue, pKey.mTime))
			{
				p++;
				memcpy(&frame.positionKey, &pKey.mValue, sizeof(D3DXVECTOR3));
			}
			if (minValue == rKey.mTime)
			{
				r++;
				frame.rotationKey.w = rKey.mValue.w;
				frame.rotationKey.x = rKey.mValue.x;
				frame.rotationKey.y = rKey.mValue.y;
				frame.rotationKey.z = rKey.mValue.z;
			}
			if (minValue == sKey.mTime)
			{
				s++;
				memcpy(&frame.scaleKey, &sKey.mValue, sizeof(D3DXVECTOR3));
			}

			time = static_cast<float>(minValue);

			length = length < time ? time : length;

			bone_anim.push_back(KeyFrameTime(time, frame));
		}
	}
}

void Converter::TempToModelAnim(const std::vector<std::vector<KeyFrameTime>> & bones_animation, const aiScene* scene, uint idx, const float& length)
{
	ModelAnimation* model_anim = new ModelAnimation;
	model_anim->name = scene->mAnimations[idx]->mName.C_Str();


	//Reorganize bones_animation.

	std::vector<uint> bonesIdx;
	bonesIdx.resize(bones_animation.size(), 0);

	while (true)
	{
		//만약 모든 본 애니메이션을 다 조사했다면
		bool completed = true;
		for (uint j = 0; j < bones_animation.size(); j++)
		{
			if (bonesIdx[j] < bones_animation[j].size())
			{
				completed = false;
				break;
			}
		}
		if (completed)
			break;

		float minValue = FLT_MAX;

		//일단 가장 빠른 프레임을 구한뒤
		for (uint j = 0; j < bones_animation.size(); j++)
		{
			const auto& bone_anim = bones_animation[j];

			if (bone_anim.size() == 0)
				continue; //보통 root본은 애니메이션이 아예 없어서 0번인덱스도 에러남

			if (bone_anim.size() <= bonesIdx[j])
				continue;

			minValue = minValue < bone_anim[bonesIdx[j]].first ?
				minValue : bone_anim[bonesIdx[j]].first;
		}
		assert(minValue != FLT_MAX);

		std::pair<float, BonesFrame> timeFrame;
		timeFrame.first = minValue;

		//해당 프레임을 가지고 있는 본들은 idx하나씩 올리고, 해당 프레임값을 ModelAnimation에 추가
		//가지고 있지 않은 본들은 그냥 보간값 프레임을 ModelAnimation에 추가
		for (uint j = 0; j < bones_animation.size(); j++)
		{
			const auto& bone_anim = bones_animation[j];

			KeyFrame frame;

			if (bonesIdx[j] >= bone_anim.size())
			{
				if (bonesIdx[j] != 0)
					frame = bone_anim[bonesIdx[j] - 1].second;
			}
			else if (Math::IsNearlySame(bone_anim[bonesIdx[j]].first, minValue))
			{
				frame = bone_anim[bonesIdx[j]].second;
				bonesIdx[j]++;
			}
			else
			{
				KeyFrameTime curr = bone_anim[bonesIdx[j]];
				KeyFrameTime prev;

				if (bonesIdx[j] - 1 < 0)
					prev = curr;
				else
					prev = bone_anim[bonesIdx[j] - 1];

				float rate = (minValue - prev.first) / (curr.first - prev.first);

				D3DXVec3Lerp(&frame.positionKey, &prev.second.positionKey, &curr.second.positionKey, rate);
				D3DXQuaternionSlerp(&frame.rotationKey,
					&prev.second.rotationKey, &curr.second.rotationKey, rate);
				D3DXVec3Lerp(&frame.scaleKey, &prev.second.scaleKey, &curr.second.scaleKey, rate);
			}

			timeFrame.second.push_back(frame);
		}

		model_anim->frames.push_back(timeFrame);
	}

	model_anim->length = length;
	model_anims.push_back(model_anim);
}

void Converter::WriteModel(std::wstring wfileName)
{
	BinaryWriter w;
	w.Open(wfileName);

	WriteBoneFile(&w);
	WriteMeshFile(&w);

	w.Close();
}

void Converter::WriteBoneFile(BinaryWriter * w)
{
	w->UInt(model_bones.size());

	for (auto& bone : model_bones)
	{
		w->Int(bone.parent);
		w->Matrix(bone.l_transform);
		w->String(bone.name);
		w->UInt(bone.childs.size());
		for (auto& child : bone.childs)
			w->UInt(child);
	}
}

void Converter::WriteMeshFile(BinaryWriter * w)
{
	w->UInt(model_meshes.size());

	for (auto& mesh : model_meshes)
	{
		w->UInt(mesh.vertices.size());
		for (auto& v : mesh.vertices)
		{
			w->Vector3(v.position);
			w->Vector2(v.uv);
			w->Vector3(v.normal);
			w->Vector3(v.tangent);

			w->UInt(MODEL_MAX_WEIGHTS_NUM);
			for (uint i = 0; i < MODEL_MAX_WEIGHTS_NUM; i++)
			{
				w->UInt(v.indices[i]);
				w->Float(v.weights[i]);
			}
		}

		w->UInt(mesh.indices.size());
		for (auto& i : mesh.indices)
			w->UInt(i);
	}
}

void Converter::WriteMaterialsFile(std::wstring wfileName)
{
	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (uint i = 0; i < materials.size(); i++)
	{
		const auto& material = materials[i];

		std::wstring wName;
		wName.assign(material.name.begin(), material.name.end());

		std::wstring texturePath = Path::GetDirectoryName(wfileName)
			+ wName + L"/";

		WriteTexture(texturePath, i);

		Xml::XMLElement* node = document->NewElement("Material");
		document->LinkEndChild(node);

		Xml::XMLElement* element = NULL;

		element = document->NewElement("Name");
		element->SetText(material.name.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("DiffuseFile");
		element->SetText(material.diffuseMap.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		element->SetText(material.specularMap.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		element->SetText(material.normalMap.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		element->SetAttribute("R", material.diffuseColor.r);
		element->SetAttribute("G", material.diffuseColor.g);
		element->SetAttribute("B", material.diffuseColor.b);
		element->SetAttribute("A", material.diffuseColor.a);
		node->LinkEndChild(element);

		element = document->NewElement("Specular");
		element->SetAttribute("R", material.specularColor.r);
		element->SetAttribute("G", material.specularColor.g);
		element->SetAttribute("B", material.specularColor.b);
		element->SetAttribute("A", material.specularColor.a);
		node->LinkEndChild(element);

		root->LinkEndChild(node);
	}

	FILE *pFile = NULL;

	_wfopen_s(&pFile, wfileName.c_str(), L"w");
	assert(pFile != NULL);

	Xml::XMLError error = document->SaveFile(pFile);
	assert(error == Xml::XML_SUCCESS);

	::fclose(pFile);
}

void Converter::WriteMaterialFile(std::wstring wfileName, uint idx)
{
	if (idx >= materials.size() || idx < 0)
		assert(false);

	const auto& material = materials[idx];

	std::wstring wName;
	wName.assign(material.name.begin(), material.name.end());

	std::wstring texturePath = Path::GetDirectoryName(wfileName)
		+ wName + L"/";

	WriteTexture(texturePath, idx);

	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* node = document->NewElement("Material");
	node->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	node->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(node);

	Xml::XMLElement* element = NULL;

	element = document->NewElement("Name");
	element->SetText(material.name.c_str());
	node->LinkEndChild(element);

	element = document->NewElement("DiffuseFile");
	element->SetText(material.diffuseMap.c_str());
	node->LinkEndChild(element);

	element = document->NewElement("SpecularFile");
	element->SetText(material.specularMap.c_str());
	node->LinkEndChild(element);

	element = document->NewElement("NormalFile");
	element->SetText(material.normalMap.c_str());
	node->LinkEndChild(element);

	element = document->NewElement("Diffuse");
	element->SetAttribute("R", material.diffuseColor.r);
	element->SetAttribute("G", material.diffuseColor.g);
	element->SetAttribute("B", material.diffuseColor.b);
	element->SetAttribute("A", material.diffuseColor.a);
	node->LinkEndChild(element);

	element = document->NewElement("Specular");
	element->SetAttribute("R", material.specularColor.r);
	element->SetAttribute("G", material.specularColor.g);
	element->SetAttribute("B", material.specularColor.b);
	element->SetAttribute("A", material.specularColor.a);
	node->LinkEndChild(element);

	FILE *pFile = NULL;

	_wfopen_s(&pFile, wfileName.c_str(), L"w");
	assert(pFile != NULL);

	Xml::XMLError error = document->SaveFile(pFile);
	assert(error == Xml::XML_SUCCESS);

	::fclose(pFile);
}

void Converter::WriteAnimationFile(std::wstring wfileName, uint idx)
{
	BinaryWriter w;
	w.Open(wfileName);

	const auto& anim = model_anims[idx];

	w.String(anim->name);
	w.Float(anim->length);
	w.UInt(anim->frames.size());
	for (const auto& frame : anim->frames)
	{
		w.Float(frame.first);
		
		w.UInt(frame.second.size());
		for (const auto& boneFrame : frame.second)
		{
			w.Vector3(boneFrame.positionKey);
			w.Vector4(D3DXVECTOR4(
				boneFrame.rotationKey.x,
				boneFrame.rotationKey.y,
				boneFrame.rotationKey.z,
				boneFrame.rotationKey.w
			));
			w.Vector3(boneFrame.scaleKey);
		}
	}

	w.Close();
}

void Converter::WriteTexture(std::wstring wPathName, uint idx)
{
	BinaryWriter w;

	if (Path::ExistDirectory(wPathName) == false)
		Path::CreateFolder(wPathName);

	std::wstring fileName;

	if (diffuseMap[idx].data != nullptr)
	{
		fileName = wPathName + diffuseMap[idx].fileName;
		w.Open(fileName);
		w.Byte(diffuseMap[idx].data, diffuseMap[idx].mapWidth);
		w.Close();
	}

	if (normalMap[idx].data != nullptr)
	{
		fileName = wPathName + normalMap[idx].fileName;
		w.Open(fileName);
		w.Byte(normalMap[idx].data, normalMap[idx].mapWidth);
		w.Close();
	}

	if (specularMap[idx].data != nullptr)
	{
		fileName = wPathName + specularMap[idx].fileName;
		w.Open(fileName);
		w.Byte(specularMap[idx].data, specularMap[idx].mapWidth);
		w.Close();
	}
}

void Converter::Clear()
{
	materials.clear();
	model_bones.clear();
	model_meshes.clear();
	model_anims.clear();
}

void Converter::StartModelRender()
{
	modelRender = new ModelRender();
	modelRender->ReadModel(&model_meshes, &model_bones);

	for (uint i = 0; i < modelRender->GetMeshCount(); i++)
	{
		Mesh& mesh = (modelRender->GetMeshes())[i];
		
		Shader* shader = new ShaderBasic();

		if (model_anims.empty())
			shader->SetVertexShader(L"../_Shaders/Basic.hlsl");
		else
			shader->SetVertexShader(L"../_Shaders/Model_VS.hlsl");

		shader->SetPixelShader(L"../_Shaders/Model_PS.hlsl");

		if (diffuseMap[i].data != nullptr)
			shader->SetDiffuseMap(diffuseMap[i].data, diffuseMap[i].mapWidth);
		if (normalMap[i].data != nullptr)
			shader->SetNormalMap(normalMap[i].data, normalMap[i].mapWidth);
		if (specularMap[i].data != nullptr)
			shader->SetSpecularMap(specularMap[i].data, specularMap[i].mapWidth);

		mesh.AddShader(shader);
	}

	modelRender->ReadMaterial(&materials);

	if (model_anims.empty() == false)
		modelRender->ReadAnimation(&model_anims);
}

void Converter::EndModelRender()
{
	SAFE_DELETE(modelRender);
}

void Converter::Update()
{
	if (modelRender != nullptr)
		modelRender->Update();
}

void Converter::Render()
{
	if (modelRender != nullptr)
		modelRender->Render();
}

bool Converter::IsModelAvailable()
{
	return model_bones.empty() ? false : true;
}

bool Converter::IsAnimAvailable()
{
	if (IsModelAvailable() == false)
		return false;

	return model_anims.empty() ? false : true;
}
