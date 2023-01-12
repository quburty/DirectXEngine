#pragma once
#include "Mesh.h"

class MeshTerrain final : public Mesh
{
public:
	MeshTerrain();
	virtual ~MeshTerrain();

	void SetHeightMap(std::wstring file);

public:
	void Edit(D3DXVECTOR3 editPos);

private:
	void Initialize();

public:
	void InitializeGPGPU();


private:
	int width = 100;
	int height = 100;
	float offset = 10.0f;

	int* depths = nullptr;

	ID3D11Buffer* UavBuffer;
	ID3D11UnorderedAccessView* uav;
	ID3D11UnorderedAccessView* vertexUav;

	struct Data
	{
		float cWidth;
		D3DXVECTOR3 center;
	} cbufferData;
	ID3D11Buffer* cbuffer;

	class ComputeShader* cs;
};