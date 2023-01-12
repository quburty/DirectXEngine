#include "Framework.h"
#include "ShaderInst.h"

ShaderInst::ShaderInst(std::wstring file)
	: Shader(file)
{
	//Create Instance Cbuffer
	{
		//CBuffer Desc
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(worlds);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		//Subresource Data
		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = &worlds[0];

		auto hr = D3D::Get()->GetDevice()->CreateBuffer(&desc, &data, &inst_cbuffer);
		assert(SUCCEEDED(hr));
	}
}

ShaderInst::~ShaderInst()
{
	SAFE_RELEASE(inst_cbuffer);
}

void ShaderInst::SetWorld(const D3DXMATRIX* world, Mesh* mesh)
{
	auto& it = std::find(meshes.begin(), meshes.end(), mesh);
	uint idx = std::distance(meshes.begin(), it);

	if (idx >= meshes.size() || idx < 0)
		assert(false);

	worlds[idx] = *world;

	//Map, Unmap Cbuffer
	D3D11_MAPPED_SUBRESOURCE subresource;
	auto hr = DeviceContext->Map(inst_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	assert(SUCCEEDED(hr));
	{
		memcpy(subresource.pData, worlds, sizeof(worlds));
	}
	DeviceContext->Unmap(inst_cbuffer, 0);
}

void ShaderInst::Render(Mesh* mesh)
{
	Shader::Render(mesh);

	DeviceContext->VSSetConstantBuffers(3, 1, &inst_cbuffer);

	DeviceContext->DrawIndexedInstanced(indexCount, meshes.size(), 0, 0, 0);
}
