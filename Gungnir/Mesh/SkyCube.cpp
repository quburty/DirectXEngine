#include "Framework.h"
#include "SkyCube.h"
#include "Shader/Shader.h"

SkyCube::SkyCube()
	: MeshCube()
{
	
}

SkyCube::~SkyCube()
{
}

void SkyCube::AddShader(Shader * shader)
{
	Super::AddShader(shader);

	if (shader == nullptr)
		return;

	ID3D11RasterizerState* rasterizer = shader->GetRasterizer();	
	D3D11_RASTERIZER_DESC rasterizer_desc;
	rasterizer->GetDesc(&rasterizer_desc);
	
	rasterizer_desc.CullMode = D3D11_CULL_FRONT;
	
	shader->CreateRasterizer(&rasterizer_desc);
	
	ID3D11DepthStencilState* depth = shader->GetDepthStencil();
	D3D11_DEPTH_STENCIL_DESC depth_desc;
	depth->GetDesc(&depth_desc);
	
	depth_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	
	shader->CreateDepthStencil(&depth_desc);

	this->Scale(D3DXVECTOR3(500,500,500));
}

void SkyCube::Update()
{
	D3DXVECTOR3 pos;
	Camera::Get()->Position(&pos);
	
	this->Position(pos);
}

