#include "Framework.h"
#include "Texture.h"

Texture::Texture()
	:Texture(Window::Get()->GetDisplayWidth(), Window::Get()->GetDisplayHeight())
{
	
}

Texture::Texture(uint inWidth, uint inHeight)
{
	device = D3D::Get()->GetDevice();
	deviceContext = D3D::Get()->GetDeviceContext();

	width = inWidth;
	height = inHeight;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = width;
	desc.Height = height;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	auto hr = device->CreateTexture2D
	(
		&desc,
		nullptr,
		&texture
	);
	Check(hr);

	CreateSRV();
	CreateRTV();
}

Texture::Texture(std::wstring wFile)
{
	device = D3D::Get()->GetDevice();
	deviceContext = D3D::Get()->GetDeviceContext();

	ID3D11Texture2D* srcTexture;

	auto hr = D3DX11CreateTextureFromFile(
		device, wFile.c_str(), nullptr, nullptr,
		reinterpret_cast<ID3D11Resource**>(&srcTexture),
		nullptr);
	Check(hr);

	D3D11_TEXTURE2D_DESC srcDesc;
	srcTexture->GetDesc(&srcDesc);
	width = srcDesc.Width;
	height = srcDesc.Height;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = width;
	desc.Height = height;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	hr = device->CreateTexture2D
	(
		&desc,
		nullptr,
		&texture
	);
	Check(hr);

	hr = D3DX11LoadTextureFromTexture(deviceContext, srcTexture, nullptr, texture);
	Check(hr);

	SAFE_RELEASE(srcTexture);

	CreateSRV();
	CreateRTV();
}

Texture::~Texture()
{
	SAFE_RELEASE(srv);
	SAFE_RELEASE(rtv);
	SAFE_RELEASE(texture);
}

void Texture::CreateSRV()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = 1;

	auto hr = device->CreateShaderResourceView(texture, &desc, &srv);
	Check(hr);
}

void Texture::CreateRTV()
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	auto hr = device->CreateRenderTargetView(texture, &desc, &rtv);
	Check(hr);
}

void Texture::GetPixels(int** pixels, uint* outWidth, uint* outHeight)
{
	SAFE_DELETE_ARRAY(*pixels);

	ID3D11Texture2D* cpuTexture;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = width;
	desc.Height = height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.ArraySize = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	auto hr = device->CreateTexture2D(&desc, nullptr, &cpuTexture);
	Check(hr);

	hr = D3DX11LoadTextureFromTexture(deviceContext, texture, nullptr, cpuTexture);
	Check(hr);

	if (texture == nullptr)
	{
		SAFE_RELEASE(cpuTexture);
		return;
	}

	D3D11_MAPPED_SUBRESOURCE subresource;
	ZeroMemory(&subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	hr = deviceContext->Map(cpuTexture, 0, D3D11_MAP_READ, 0, &subresource);
	Check(hr);
	{
		*pixels = new int[width*height];
		memcpy(*pixels, subresource.pData, sizeof(int)*width*height);
	}
	deviceContext->Unmap(cpuTexture, 0);

	*outWidth = width;
	*outHeight = height;

	SAFE_RELEASE(cpuTexture);
}

bool Texture::SaveTexture(std::wstring wFile)
{
	auto hr = D3DX11SaveTextureToFileW(
		deviceContext, texture, D3DX11_IFF_PNG, wFile.c_str());

	Check(hr);

	return true;
}
