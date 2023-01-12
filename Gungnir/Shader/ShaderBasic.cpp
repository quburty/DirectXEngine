#include "Framework.h"
#include "ShaderBasic.h"

ShaderBasic::ShaderBasic()
	: Shader()
{
	//world cbuffer
	{
		D3DXMATRIX world;
		D3DXMatrixIdentity(&world);
		D3DXMATRIX s, r, t;

		D3DXMatrixScaling(&s, 1, 1, 1);
		D3DXMatrixIdentity(&r);
		D3DXMatrixIdentity(&t);

		world = s * r*t;
		D3DXMatrixTranspose(&world, &world);

		//CBuffer Desc
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(D3DXMATRIX);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		//Subresource Data
		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = &world;

		auto hr = D3D::Get()->GetDevice()->CreateBuffer(&desc, &data, &w_cbuffer);
		assert(SUCCEEDED(hr));
	}
}

ShaderBasic::ShaderBasic(std::wstring file)
	: Shader(file)
{
	{
		D3DXMATRIX world;
		D3DXMatrixIdentity(&world);
		D3DXMATRIX s, r, t;

		D3DXMatrixScaling(&s, 1, 1, 1);
		D3DXMatrixIdentity(&r);
		D3DXMatrixIdentity(&t);

		world = s * r*t;
		D3DXMatrixTranspose(&world, &world);

		//CBuffer Desc
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(D3DXMATRIX);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		//Subresource Data
		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = &world;

		auto hr = D3D::Get()->GetDevice()->CreateBuffer(&desc, &data, &w_cbuffer);
		assert(SUCCEEDED(hr));
	}
}

ShaderBasic::~ShaderBasic()
{
	SAFE_RELEASE(w_cbuffer);
}

void ShaderBasic::SetWorld(const D3DXMATRIX* world, Mesh* mesh)
{
	//Map, Unmap Cbuffer
	D3D11_MAPPED_SUBRESOURCE subresource;
	auto hr = DeviceContext->Map(w_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	assert(SUCCEEDED(hr));
	{
		memcpy_s(subresource.pData, subresource.RowPitch, world, sizeof(D3DXMATRIX));
	}
	DeviceContext->Unmap(w_cbuffer, 0);
}

void ShaderBasic::Render(Mesh* mesh)
{
	Shader::Render(mesh);

	DeviceContext->VSSetConstantBuffers(0, 1, &w_cbuffer);

	DeviceContext->DrawIndexed(indexCount, 0, 0);
}