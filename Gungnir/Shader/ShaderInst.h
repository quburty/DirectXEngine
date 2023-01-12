#pragma once
#include "Shader.h"

#define SHADER_INST_MAX_TRANSFORM 1000

class ShaderInst final : public Shader
{
public:
	ShaderInst(std::wstring file);
	virtual ~ShaderInst();

	void SetWorld(const D3DXMATRIX* world, class Mesh* mesh) override;

private:
	void Render(class Mesh* mesh) override;

private:
	D3DXMATRIX worlds[SHADER_INST_MAX_TRANSFORM];

	ID3D11Buffer* inst_cbuffer;
};