#include "Framework.h"
#include "MeshTerrain.h"
#include "Shader/Texture.h"
#include "Shader/Shader.h"
#include "Compute/ComputeShader.h"

MeshTerrain::MeshTerrain()
	:Mesh()
{
	Initialize();
}

MeshTerrain::~MeshTerrain()
{
	SAFE_DELETE(cs);

	SAFE_RELEASE(UavBuffer);

	SAFE_RELEASE(uav);
	SAFE_RELEASE(vertexUav);
	SAFE_RELEASE(cbuffer);
}

void MeshTerrain::SetHeightMap(std::wstring file)
{
	ID3D11Device* device = D3D::Get()->GetDevice();
	ID3D11DeviceContext* deviceContext = D3D::Get()->GetDeviceContext();

	Texture* texture = new Texture(file);

	uint x, y;
	texture->GetPixels(&depths, &x, &y);
	width = (uint)x;
	height = (uint)y;

	Initialize(); //490ms

	SAFE_DELETE(texture);
	SAFE_DELETE_ARRAY(depths);
}

void MeshTerrain::Edit(D3DXVECTOR3 editPos)
{
	ID3D11DeviceContext* deviceContext = D3D::Get()->GetDeviceContext();

	cbufferData.center = editPos;

	D3D11_MAPPED_SUBRESOURCE subresource;
	auto hr = deviceContext->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	Check(hr);
	{
		memcpy(subresource.pData, &cbufferData, sizeof(cbufferData));
	}
	deviceContext->Unmap(cbuffer, 0);

	ID3D11UnorderedAccessView* uavs[2] = { uav,vertexUav };
	cs->SetComputeSize(500, 1, 1);
	cs->SetCBuffer(cbuffer, 6);
	cs->SetUAVs(uavs, 2);
	cs->Compute();

	uavs[0] = nullptr;
	uavs[1] = nullptr;

	deviceContext->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);
	
	{
		////확인용 코드

		//ID3D11Buffer* tempReadBuffer;
		//D3D11_BUFFER_DESC tempDesc;
		//ZeroMemory(&tempDesc, sizeof(tempDesc));
		//tempDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		//tempDesc.Usage = D3D11_USAGE_STAGING;
		//tempDesc.ByteWidth = sizeof(Vertex)*width*height;
		//hr = D3D::Get()->GetDevice()->CreateBuffer(&tempDesc, nullptr, &tempReadBuffer);
		//Check(hr);

		//deviceContext->CopyResource(tempReadBuffer, shaders[0]->GetVertexBuffer());

		////D3D11_MAPPED_SUBRESOURCE subresource;
		//hr = deviceContext->Map(tempReadBuffer, 0, D3D11_MAP_READ, 0, &subresource);
		//{
		//	memcpy(vertices.data(), subresource.pData, sizeof(Vertex)*width*height);
		//}
		//deviceContext->Unmap(tempReadBuffer, 0);
	}
}

void MeshTerrain::Initialize()
{
	vertices.clear();
	indices.clear();

	//Vertices
	vertices.resize(width*height);

	int i = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			i = y * width + x;

			Vertex v;
			sizeof(Vertex);

			if (depths == nullptr)
				v.position = Vector3(offset*(float)x, 0.0f, offset*(float)y);
			else
			{
				float a = (float)((0xFF000000 & depths[i]) >> 24);
				float b = (float)((0x00FF0000 & depths[i]) >> 16);
				float g = (float)((0x0000FF00 & depths[i]) >> 8);
				float r = (float)((0x000000FF & depths[i]) >> 0);

				v.position = Vector3(offset*(float)x, r, offset*(float)y);
			}

			v.normal = Vector3(0, 0, 0);
			v.tangent = Vector3(1, 0, 0);
			v.uv = D3DXVECTOR2((float)(x) / (float)width, (float)(y) / (float)height);

			vertices[i] = v;

			//i++;
		}
	}

	//Indices
	for (int y = 0; y < height - 1; y++)
	{
		for (int x = 0; x < width - 1; x++)
		{
			indices.push_back(y*width + x);
			indices.push_back((y + 1)*width + x);
			indices.push_back(y*width + x + 1);
			indices.push_back(y*width + x + 1);
			indices.push_back((y + 1)*width + x);
			indices.push_back((y + 1)*width + x + 1);
		}
	}

	//Vertex-Normals
	for (uint i = 0; i < indices.size() / 3; i++)
	{
		auto& v0 = vertices[indices[i * 3 + 0]];
		auto& v1 = vertices[indices[i * 3 + 1]];
		auto& v2 = vertices[indices[i * 3 + 2]];

		Vector3 delta;
		D3DXVec3Cross(&delta, &(v0.position - v2.position), &(v0.position - v1.position));
		v0.normal += delta;
		D3DXVec3Cross(&delta, &(v1.position - v0.position), &(v1.position - v2.position));
		v1.normal += delta;
		D3DXVec3Cross(&delta, &(v2.position - v1.position), &(v2.position - v0.position));
		v2.normal += delta;
	}
}

void MeshTerrain::InitializeGPGPU()
{
	cs = new ComputeShader(L"../_Shaders/MapEdit.hlsl");
	

	HRESULT hr;
	ID3D11Device* device = D3D::Get()->GetDevice();
	ID3D11DeviceContext* deviceContext = D3D::Get()->GetDeviceContext();

	{
		SAFE_RELEASE(UavBuffer);

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.ByteWidth = sizeof(Vertex)*width*height;
		bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		bufferDesc.StructureByteStride = sizeof(Vertex);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = vertices.data();

		auto hr = device->CreateBuffer(&bufferDesc, &data, &UavBuffer); //10ms
		Check(hr);
	}

	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.NumElements = width * height;

		hr = device->CreateUnorderedAccessView(UavBuffer, &uavDesc, &uav);
		Check(hr);
	}

	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.NumElements = width * height * sizeof(Vertex) / 4;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

		hr = device->CreateUnorderedAccessView(shaders[0]->GetVertexBuffer(), &uavDesc, &vertexUav);
		Check(hr);
	}

	{
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.ByteWidth = 16;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		cbufferData.cWidth = (float)width;

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = &cbufferData;

		hr = device->CreateBuffer(&bufferDesc, &data, &cbuffer);
		Check(hr);
	}
}
