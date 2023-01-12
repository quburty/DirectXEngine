#pragma once

#define MODEL_MAX_WEIGHTS_NUM 4

using Vertex = struct VertexModel;

struct VertexNormalTangent
{
	VertexNormalTangent() {}
	VertexNormalTangent(float x, float y, float z) : position(x, y, z) {}
	VertexNormalTangent(float x, float y, float z, float u, float v)
		: position(x, y, z), uv(u, v) {}

	D3DXVECTOR3 position;

	D3DXVECTOR2 uv;

	D3DXVECTOR3 normal;

	D3DXVECTOR3 tangent;
};

struct VertexModel
{
	VertexModel() {}
	VertexModel(float x, float y, float z) : position(x, y, z) {}
	VertexModel(float x, float y, float z, float u, float v)
		: position(x, y, z), uv(u, v) {}

	D3DXVECTOR3 position{ 0.0f,0.0f,0.0f };

	D3DXVECTOR2 uv{ 0.0f,0.0f };

	D3DXVECTOR3 normal{ 0.0f,1.0f,0.0f };

	D3DXVECTOR3 tangent{ 1.0f,0.0f,0.0f };
	
	uint indices[MODEL_MAX_WEIGHTS_NUM] = { 0 };
	float weights[MODEL_MAX_WEIGHTS_NUM] = { 0.f };

public:
	void AddIndexAndWeight(uint index, float weight);
};