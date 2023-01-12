#pragma once

#define MAX_FRAME_COUNT 1000

namespace Animation
{
	struct KeyFrame
	{
		D3DXVECTOR3 positionKey = D3DXVECTOR3(0.f,0.f,0.f);
		D3DXVECTOR3 scaleKey = D3DXVECTOR3(1.f, 1.f, 1.f);;
		D3DXQUATERNION rotationKey = D3DXQUATERNION(0.f, 0.f, 0.f, 1.0f);
	};

	typedef std::vector<KeyFrame> BonesFrame;

	struct ModelAnimation
	{
		std::string name;
		float length;
		std::vector<std::pair<float, BonesFrame>> frames;
	};

}

class ModelAnimator final
{
public:
	ModelAnimator(class ModelRender* model);
	~ModelAnimator();

	void ReadAnimation(std::wstring wfile);
	void ReadAnimation(std::vector<Animation::ModelAnimation*>* model_anims);

	void CreateAnimTransforms(uint idx);
	void CreateAnimationTexture(uint idx);
	void CreateAnimSRV();

	void AnimationUpdate(uint idx);

private:
	void SetOwner(class ModelRender* model) { ownerModel = model; }

private:
	std::vector<Animation::ModelAnimation> model_anims;

	ID3D11Texture2D* animTexture;
	ID3D11ShaderResourceView* animSRV;

	class ModelRender* ownerModel;
};