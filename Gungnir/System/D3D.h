#pragma once
#include "Framework.h"

class D3D final
{
private:
	D3D();
	~D3D();

public:
	static D3D* Get()
	{
		static D3D object;
		return &object;
	}

	void Initialize(HWND hwnd, uint width, uint height);
	void Resize();
	void Destroy();

	void Update();
	void Render_Begin();
	void Render_End();

	IDXGISwapChain* GetSwapChain() { return SwapChain; }
	ID3D11Device* GetDevice() { return Device; }
	ID3D11DeviceContext* GetDeviceContext() { return DeviceContext; }

	D3D11_VIEWPORT* GetViewport() { return &Viewport; }

	ID3D11RenderTargetView* GetRTV() { return RTV; }
	ID3D11DepthStencilView* GetDSV() { return DSV; }
	ID3D11Texture2D* GetDepthTexture() { return DepthTexture; }

	void SetRenderTarget(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv);
	void SetRenderTargets(uint viewNum, ID3D11RenderTargetView* const * rtvs, ID3D11DepthStencilView* dsv);

	void SetLight(D3DXVECTOR3* direction) { perFrame.direction = *direction; }
	float& Roughness() { return perFrame.roughness; }
	float& Ambient() { return perFrame.ambient; }
	float& Diffuse() { return perFrame.diffuse; }
	float& Specular() { return perFrame.specular; }

private:
	IDXGISwapChain* SwapChain = nullptr;
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;

	ID3D11Debug* Debug;

	D3D11_VIEWPORT Viewport;

	ID3D11RenderTargetView* RTV;

	struct ViewProj
	{
		D3DXMATRIX view;
		D3DXMATRIX projection;
	} viewProj;

	ID3D11Buffer* vp_cbuffer;

	struct PerFrame
	{
		D3DXVECTOR3 direction{ 0.5f,0.5f,0.5f };
		float padding;
		D3DXVECTOR3 cameraPosition;
		float padding2;

		//temp
		float roughness = 8.0f;
		float ambient = 0.2f;
		float diffuse = 0.5f;
		float specular = 1.0f;

	} perFrame;
	ID3D11Buffer* perFrame_cbuffer;
	
	ID3D11Texture2D* DepthTexture;
	ID3D11DepthStencilView* DSV;
};