#include "Framework.h"
#include "ModelAnimator.h"
#include "Model/ModelRender.h"

using namespace Animation;

ModelAnimator::ModelAnimator(ModelRender * model)
{
	SetOwner(model);
}

ModelAnimator::~ModelAnimator()
{
	SAFE_RELEASE(animTexture);
	SAFE_RELEASE(animSRV);
}

void ModelAnimator::ReadAnimation(std::wstring wfile)
{
	BinaryReader r;
	r.Open(wfile);

	ModelAnimation anim;
	anim.name = r.String();
	anim.length = r.Float();
	anim.frames.resize(r.UInt());
	for (auto& timeFrame : anim.frames)
	{
		timeFrame.first = r.Float();

		timeFrame.second.resize(r.UInt());
		for (auto& frame : timeFrame.second)
		{
			frame.positionKey = r.Vector3();
			{
				D3DXVECTOR4 vec4 = r.Vector4();
				frame.rotationKey.w = vec4.w;
				frame.rotationKey.x = vec4.x;
				frame.rotationKey.y = vec4.y;
				frame.rotationKey.z = vec4.z;
			}
			frame.scaleKey = r.Vector3();
		}
	}

	model_anims.emplace_back(anim);

	r.Close();
}

void ModelAnimator::ReadAnimation(std::vector<Animation::ModelAnimation*>* model_anims)
{
	for (uint i = 0; i < model_anims->size(); i++)
	{
		this->model_anims.push_back(*model_anims->at(i));
	}
}

void ModelAnimator::CreateAnimTransforms(uint idx)
{
}

void ModelAnimator::CreateAnimationTexture(uint idx)
{
	assert(false);

	const auto& anim = model_anims[idx];

	ID3D11Device* device = D3D::Get()->GetDevice();

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.SampleDesc.Count = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Width = MAX_BONE_COUNT;
	desc.Height = MAX_FRAME_COUNT;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));

	void* animData = VirtualAlloc(
		nullptr,
		sizeof(D3DXMATRIX)*MAX_BONE_COUNT*MAX_FRAME_COUNT,
		MEM_COMMIT, PAGE_READWRITE
	);

	device->CreateTexture2D(&desc, &data, &animTexture);

	VirtualFree(animData, sizeof(D3DXMATRIX)*MAX_BONE_COUNT*MAX_FRAME_COUNT, MEM_DECOMMIT);
}

void ModelAnimator::CreateAnimSRV()
{
	ID3D11Device* device = D3D::Get()->GetDevice();
	
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	auto hr = device->CreateShaderResourceView(animTexture, &desc, &animSRV);
	Check(hr);
}

void ModelAnimator::AnimationUpdate(uint idx)
{
	if (ownerModel == nullptr)
		assert(false);

	if (!(idx < model_anims.size()))
		assert(false);

	const auto& model_anim = model_anims[idx];
	static uint frameIdx = 0;
	static float current = 0.0f;
	current += Time::Get()->Delta() * 10;
	float delta = Time::Get()->Delta() * 10;

	ImGui::Text("%f", current);
	ImGui::Text("%d", frameIdx);

	if (current > model_anim.length)
	{
		frameIdx = 0;
		current = 0.0f;
	}
	else if (frameIdx + 1 < model_anim.frames.size()
		&& current > model_anim.frames[frameIdx + 1].first)
		frameIdx++;

	const auto& currentFrame = model_anim.frames[frameIdx].second;
	const auto& nextFrame = model_anim.frames[(frameIdx + 1) % model_anim.frames.size()].second;
	
	for (uint i = 0; i < ownerModel->model_bones.size(); i++)
	{
		D3DXMATRIX animation;

		if (currentFrame.size() <= i)
		{
			D3DXMatrixIdentity(&animation);
			assert(false);
		}

		const auto& currentBoneFrame = currentFrame[i];
		
		{
			D3DXMATRIX s, r, t;
			D3DXMatrixScaling(&s, currentBoneFrame.scaleKey.x, currentBoneFrame.scaleKey.y, currentBoneFrame.scaleKey.z);
			D3DXMatrixRotationQuaternion(&r, &currentBoneFrame.rotationKey);
			D3DXMatrixTranslation(&t, currentBoneFrame.positionKey.x, currentBoneFrame.positionKey.y, currentBoneFrame.positionKey.z);
			animation = s * r*t;
		}
		
		//temp
		D3DXMATRIX next;

		if (nextFrame.size() <= i)
		{
			D3DXMatrixIdentity(&next);
			assert(false);
		}

		const auto& nextBoneFrame = nextFrame[i];
		
		{
			D3DXMATRIX s, r, t;
			D3DXMatrixScaling(&s, nextBoneFrame.scaleKey.x, nextBoneFrame.scaleKey.y, nextBoneFrame.scaleKey.z);
			D3DXMatrixRotationQuaternion(&r, &nextBoneFrame.rotationKey);
			D3DXMatrixTranslation(&t, nextBoneFrame.positionKey.x, nextBoneFrame.positionKey.y, nextBoneFrame.positionKey.z);
			next = s * r*t;
		}

		float rate = (current - model_anim.frames[frameIdx].first)
			/ (model_anim.frames[(frameIdx + 1) % model_anim.frames.size()].first - model_anim.frames[frameIdx].first);

		Math::LerpMatrix(animation, animation, next, rate);
		ownerModel->model_bones[i].l_transform = animation;
	}

	ownerModel->UpdateBone(0);
	ownerModel->UpdateCBuffer();

	
}

