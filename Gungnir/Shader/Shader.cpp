#include "Framework.h"
#include "Shader.h"
#include "Mesh/Mesh.h"

Shader::Shader()
{
	Device = D3D::Get()->GetDevice();
	DeviceContext = D3D::Get()->GetDeviceContext();

	//Sampler
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		auto hr = Device->CreateSamplerState(&desc, &SamplerState);
		Check(hr);
	}

	//ShaderResourceView
	{
		auto hr = D3DX11CreateShaderResourceViewFromFile(Device, L"../_Textures/White.png", nullptr, nullptr, &SRV_diffuse, nullptr);
		Check(hr);
	}

	//Rasterizer
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
		desc.CullMode = D3D11_CULL_BACK;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = TRUE;

		this->CreateRasterizer(&desc);
	}

	//DepthStencil
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;

		this->CreateDepthStencil(&desc);
	}
}

Shader::Shader(std::wstring file)
	: Shader()
{
	SetShader(file);
}

Shader::~Shader()
{
	for (auto& mesh : meshes)
	{
		if (mesh != nullptr)
			mesh->DelistShader(this);
	}

	meshes.clear();

	SAFE_RELEASE(DepthStencil);
	SAFE_RELEASE(Rasterizer);

	SAFE_RELEASE(DSV);
	SAFE_RELEASE(DepthTexture);

	SAFE_RELEASE(SRV_diffuse);
	SAFE_RELEASE(SRV_normal);
	SAFE_RELEASE(SRV_specular);
	SAFE_RELEASE(SamplerState);

	SAFE_RELEASE(PixelShader);
	SAFE_RELEASE(VertexShader);
	SAFE_RELEASE(PsBlob);
	SAFE_RELEASE(VsBlob);

	SAFE_RELEASE(InputLayout);
	SAFE_RELEASE(IndexBuffer);
	SAFE_RELEASE(VertexBuffer);
}

void Shader::SetShader(std::wstring file)
{
	SetVertexShader(file);
	SetPixelShader(file);
}

void Shader::SetVertexShader(std::wstring file)
{
	SAFE_RELEASE(VsBlob);
	SAFE_RELEASE(VertexShader);

	bool result = LoadShader(file, &VsBlob);
	assert(result);

	//CreateInputLayout
	{
		D3D11_INPUT_ELEMENT_DESC descs[] =
		{
			{
				"Position",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				0,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
			,{
				"Uv",
				0,
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				12,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"Normal",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				20,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"Tangent",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				32,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"Indices",
				0,
				DXGI_FORMAT_R32G32B32A32_UINT,
				0,
				44,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"Weights",
				0,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				0,
				60,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
		};

		auto hr = Device->CreateInputLayout(descs, ARRAYSIZE(descs), VsBlob->GetBufferPointer(), VsBlob->GetBufferSize(), &InputLayout);
		Check(hr);
	}

	auto hr = D3D::Get()->GetDevice()->CreateVertexShader(VsBlob->GetBufferPointer(), VsBlob->GetBufferSize(), nullptr, &VertexShader);
	assert(SUCCEEDED(hr));
}

void Shader::SetPixelShader(std::wstring file)
{
	SAFE_RELEASE(PsBlob);
	SAFE_RELEASE(PixelShader);

	LoadShader(file, &PsBlob);

	auto hr = D3D::Get()->GetDevice()->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &PixelShader);
	assert(SUCCEEDED(hr));
}

bool Shader::LoadShader(const std::wstring & file, ID3D10Blob** shaderBlob_ptr)
{
	//Reading Shader File.
	{
		std::wstring extension = Path::GetExtension(file);

		if (extension == L"cso")
		{
			HANDLE fileHandle = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			bool bChecked = fileHandle != INVALID_HANDLE_VALUE;
			CheckFalseResult(bChecked, false);

			DWORD dataSize = GetFileSize(fileHandle, NULL);
			CheckTrueResult(dataSize == 0xFFFFFFFF, false);

			void* data = malloc(dataSize);
			DWORD readSize;
			CheckFalseResult(ReadFile(fileHandle, data, dataSize, &readSize, NULL), false);

			CloseHandle(fileHandle);
			fileHandle = NULL;

			D3DCreateBlob(dataSize, shaderBlob_ptr);
			memcpy((*shaderBlob_ptr)->GetBufferPointer(), data, dataSize);
			
		}
		else if (extension == L"hlsl")
		{
			bool result = CompileShader(file, shaderBlob_ptr);
			CheckFalseResult(result, false);
		}
		else
		{
			//Unsupportable shader file extension.
			return false;
		}
	}

	return true;
}

bool Shader::CompileShader(const std::wstring & file, ID3D10Blob** shaderBlob_ptr)
{
	//Create Shader

	ID3D10Blob* errorBlob = nullptr;

	const char* functionName = nullptr;
	const char* shaderVersion = nullptr;

	if (*shaderBlob_ptr == VsBlob)
	{
		functionName = "VS"; shaderVersion = "vs_5_0";
	}
	else if (*shaderBlob_ptr == PsBlob)
	{
		functionName = "PS"; shaderVersion = "ps_5_0";
	}

	auto hr = D3DX11CompileFromFileW
	(
		file.c_str(),
		nullptr,
		nullptr,
		functionName,
		shaderVersion,
		NULL,
		NULL,
		nullptr,
		shaderBlob_ptr,
		&errorBlob,
		nullptr
	);

	if (errorBlob != nullptr)
	{
		MessageBoxA(Window::Get()->GetHWND(), (char*)errorBlob->GetBufferPointer(), "", 0);
		return false;
	}

	return true;
}

void Shader::CreateVertexBuffer(void* vertices, uint stride, uint count, bool bGpuWrite, bool bUseUav)
{
	SAFE_RELEASE(VertexBuffer);

	this->stride = stride;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	if (bUseUav)
	{
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	}

	desc.ByteWidth = stride * count;
	desc.CPUAccessFlags = 0;

	if (bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	else
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
	}

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = vertices;

	auto hr = D3D::Get()->GetDevice()->CreateBuffer(&desc, &data, &VertexBuffer);
	assert(SUCCEEDED(hr));
}

void Shader::CreateIndexBuffer(uint* indices, uint count)
{
	SAFE_RELEASE(IndexBuffer);

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof(uint) * count;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = indices;

	auto hr = D3D::Get()->GetDevice()->CreateBuffer(&desc, &data, &IndexBuffer);
	assert(SUCCEEDED(hr));

	indexCount = count;
}

void Shader::SetDiffuseMap(std::wstring file)
{
	if (SRV_diffuse != nullptr)
		SAFE_RELEASE(SRV_diffuse);

	auto hr = D3DX11CreateShaderResourceViewFromFile(Device, file.c_str(), nullptr, nullptr, &SRV_diffuse, nullptr);
	Check(hr);

	bDiffuseMap = true;
}

void Shader::SetNormalMap(std::wstring file)
{
	if (SRV_normal != nullptr)
		SAFE_RELEASE(SRV_normal);

	auto hr = D3DX11CreateShaderResourceViewFromFile(Device, file.c_str(), nullptr, nullptr, &SRV_normal, nullptr);
	Check(hr);

	bNormalMap = true;
}

void Shader::SetSpecularMap(std::wstring file)
{
	if (SRV_specular != nullptr)
		SAFE_RELEASE(SRV_specular);

	auto hr = D3DX11CreateShaderResourceViewFromFile(Device, file.c_str(), nullptr, nullptr, &SRV_specular, nullptr);
	Check(hr);	

	bSpecularMap = true;
}

void Shader::SetDiffuseMap(void* data, uint length)
{
	if (SRV_diffuse != nullptr)
		SAFE_RELEASE(SRV_diffuse);

	auto hr = D3DX11CreateShaderResourceViewFromMemory(Device, data, length, nullptr, nullptr, &SRV_diffuse, nullptr);
	Check(hr);

	bDiffuseMap = true;
}

void Shader::SetNormalMap(void * data, uint length)
{
	if (SRV_normal != nullptr)
		SAFE_RELEASE(SRV_normal);

	auto hr = D3DX11CreateShaderResourceViewFromMemory(Device, data, length, nullptr, nullptr, &SRV_normal, nullptr);
	Check(hr);

	bNormalMap = true;
}

void Shader::SetSpecularMap(void * data, uint length)
{
	if (SRV_specular != nullptr)
		SAFE_RELEASE(SRV_specular);

	auto hr = D3DX11CreateShaderResourceViewFromMemory(Device, data, length, nullptr, nullptr, &SRV_specular, nullptr);
	Check(hr);

	bSpecularMap = true;
}

void Shader::SetDiffuseMap(ID3D11ShaderResourceView * srv)
{
	SRV_diffuse = srv;

	if (srv == nullptr)
		bDiffuseMap = false;
	else
		bDiffuseMap = true;
}

void Shader::CreateRasterizer(D3D11_RASTERIZER_DESC * desc)
{
	SAFE_RELEASE(Rasterizer);

	auto hr = Device->CreateRasterizerState(desc, &Rasterizer);
	Check(hr);
}

void Shader::CreateDepthStencil(D3D11_DEPTH_STENCIL_DESC * desc)
{
	SAFE_RELEASE(DepthStencil);

	auto hr = Device->CreateDepthStencilState(desc, &DepthStencil);
	Check(hr);
}

void Shader::RegisterMesh(Mesh * mesh)
{
	auto& it = std::find(meshes.begin(), meshes.end(), mesh);

	if (it != meshes.end())
		return;

	meshes.emplace(mesh);
}

void Shader::DelistMesh(Mesh * mesh)
{
	auto& it = std::find(meshes.begin(), meshes.end(), mesh);

	if (it == meshes.end())
		return;

	meshes.erase(it);

}

void Shader::Render(class Mesh* mesh)
{
	const D3DXMATRIX* transWorld = &mesh->GetTransWorld();
	SetWorld(transWorld, mesh);

	uint offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
	DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetInputLayout(InputLayout);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->VSSetShader(VertexShader, nullptr, 0);
	
	DeviceContext->RSSetState(Rasterizer);
	DeviceContext->PSSetShader(PixelShader, nullptr, 0);;
	DeviceContext->PSSetSamplers(0, 1, &SamplerState);
	
	DeviceContext->PSSetShaderResources(0, 1, &SRV_diffuse);
	DeviceContext->PSSetShaderResources(1, 1, &SRV_normal);
	DeviceContext->PSSetShaderResources(2, 1, &SRV_specular);

	DeviceContext->OMSetDepthStencilState(DepthStencil, 0);
}
