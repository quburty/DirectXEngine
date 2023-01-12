#pragma once

class Texture final
{
public:
	Texture();
	Texture(uint inWidth, uint inHeight);
	Texture(std::wstring wFile);
	~Texture();

private:
	void CreateSRV();
	void CreateRTV();

public:
	ID3D11ShaderResourceView* SRV() { return srv; }
	ID3D11RenderTargetView* RTV() { return rtv; }
	ID3D11Texture2D* Texture2D() { return texture; }

	void GetPixels(int** pixels, uint* outWidth, uint* outHeight);

public:
	bool SaveTexture(std::wstring wFile);

private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	ID3D11ShaderResourceView* srv;
	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* texture;

	uint width;
	uint height;
};