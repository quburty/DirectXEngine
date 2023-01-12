#pragma once
#include "Shader.h"

class ShaderBasic final : public Shader
{
public:
	ShaderBasic();
	ShaderBasic(std::wstring file);
	virtual ~ShaderBasic();

	void SetWorld(const D3DXMATRIX* world, class Mesh* mesh) override;

private:
	void Render(class Mesh* mesh) override;

private:
	ID3D11Buffer* w_cbuffer;
};