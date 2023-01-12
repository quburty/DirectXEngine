#include "Framework.h"
#include "D3D.h"

D3D::D3D()
{
}

D3D::~D3D()
{
}

void D3D::Initialize(HWND hwnd, uint width, uint height)
{
	//DX Create SwapChain, Device, DeviceContext
	{
		DXGI_MODE_DESC bufferDesc; //�ĸ������ �Ӽ����� �����ϴ� ����ü
		ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));
		bufferDesc.Width = width;
		bufferDesc.Height = height;
		bufferDesc.RefreshRate.Numerator = 60;
		bufferDesc.RefreshRate.Denominator = 1;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //���÷��� ��ĵ ���� ���
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //�ؽ��� ��� ����

		DXGI_SWAP_CHAIN_DESC desc;
		ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
		desc.BufferCount = 1; //���� ���۸� or ���� ���۸�??
		desc.BufferDesc = bufferDesc;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.OutputWindow = hwnd;
		desc.SampleDesc.Count = 1; //�ȼ��� ������ ǥ���� ����, ��Ƽ���ø��� ������� ������ 1�� �����ϰ� ���� ǰ�� 0���� ����
		desc.SampleDesc.Quality = 0;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //���۸� ���� ����� �ɼ�, discard�� �ٷ� ���
		desc.Windowed = TRUE;

		D3D_FEATURE_LEVEL feature_levels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};

		HRESULT hr = D3D11CreateDeviceAndSwapChain
		(
			nullptr, //(����ī�� �������̽� ����)
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr, //(�����Ͷ������� �����Ǿ� �ִ� dll�� �ڵ��� ����)
			D3D11_CREATE_DEVICE_DEBUG, //(����� dx11�� api ���̾�)
			feature_levels,
			sizeof(feature_levels) / sizeof(D3D_FEATURE_LEVEL),
			D3D11_SDK_VERSION,
			&desc,
			&SwapChain,
			&Device,
			nullptr,
			&DeviceContext
		);

		assert(hr == S_OK);
	}

	//CreateRTV
	{
		ID3D11Texture2D* BackBuffer;
		auto hr = D3D::Get()->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
		assert(SUCCEEDED(hr));

		hr = D3D::Get()->GetDevice()->CreateRenderTargetView(BackBuffer, nullptr, &RTV); //backbuffer�� ������, ������ �� ���� ���� ����ü
		assert(SUCCEEDED(hr));
		SAFE_RELEASE(BackBuffer);//?
	}

	//Create DSV Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = Window::Get()->GetDisplayWidth();
		desc.Height = Window::Get()->GetDisplayHeight();
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

		auto hr = Device->CreateTexture2D(&desc, nullptr, &DepthTexture);
		Check(hr);
	}

	//CreateDSV
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		auto hr = Device->CreateDepthStencilView(DepthTexture, &desc, &DSV);
		Check(hr);
	}

	D3D::Get()->GetDeviceContext()->OMSetRenderTargets(1, &RTV, DSV); //��¿� rtv, dsv ����

	//Create Viewport
	{
		Viewport.Width = Window::Get()->GetDisplayWidthFloat();
		Viewport.Height = Window::Get()->GetDisplayHeightFloat();
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;
		Viewport.TopLeftX = 0.0f;
		Viewport.TopLeftY = 0.0f;
		DeviceContext->RSSetViewports(1, &Viewport);
	}

	//ViewProj CBuffer
	{
		//CBuffer Desc
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(ViewProj);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		//Subresource Data
		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = &viewProj;

		auto hr = D3D::Get()->GetDevice()->CreateBuffer(&desc, &data, &vp_cbuffer);
		assert(SUCCEEDED(hr));
	}

	//PerFrame CBuffer
	{
		//CBuffer Desc
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(PerFrame);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		//Subresource Data
		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = &perFrame;

		auto hr = D3D::Get()->GetDevice()->CreateBuffer(&desc, &data, &perFrame_cbuffer);
		assert(SUCCEEDED(hr));
	}
	
}

void D3D::Resize()
{
}

void D3D::Destroy()
{
	SAFE_RELEASE(perFrame_cbuffer);

	SAFE_RELEASE(DSV);
	SAFE_RELEASE(DepthTexture);

	SAFE_RELEASE(vp_cbuffer);

	SAFE_RELEASE(RTV);

	SAFE_RELEASE(SwapChain);
	SAFE_RELEASE(DeviceContext);


	////DXGI �޸� �� Ȯ�� �ڵ�
	//ID3D11Debug* dxgiDebug;

	//if (SUCCEEDED(Device->QueryInterface(IID_PPV_ARGS(&dxgiDebug))))
	//{
	//	dxgiDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	//	SAFE_RELEASE(dxgiDebug);
	//}

	SAFE_RELEASE(Device);
	
}

void D3D::Update()
{
}

void D3D::Render_Begin()
{
	Camera::Get()->GetMatrix(&viewProj.view);
	Camera::Get()->GetProjMatrix(&viewProj.projection);

	//Map, Unmap Cbuffer
	D3D11_MAPPED_SUBRESOURCE subresource;
	auto hr = GetDeviceContext()->Map(vp_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	assert(SUCCEEDED(hr));
	{
		//subresource.pData = &wvp;
		memcpy_s(subresource.pData, subresource.RowPitch, &viewProj, sizeof(ViewProj));
	}
	GetDeviceContext()->Unmap(vp_cbuffer, 0);

	Camera::Get()->Position(&perFrame.cameraPosition);
	hr = GetDeviceContext()->Map(perFrame_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	assert(SUCCEEDED(hr));
	{
		memcpy_s(subresource.pData, subresource.RowPitch, &perFrame, sizeof(perFrame));
	}
	GetDeviceContext()->Unmap(perFrame_cbuffer, 0);

	float color[4] = { 0.2f,0.2f,0.2f,0.2f };
	GetDeviceContext()->ClearRenderTargetView(RTV, color);
	GetDeviceContext()->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH, 1, 0);

	GetDeviceContext()->VSSetConstantBuffers(1, 1, &vp_cbuffer);
	GetDeviceContext()->PSSetConstantBuffers(2, 1, &perFrame_cbuffer);
}

void D3D::Render_End()
{
	GetSwapChain()->Present(0, 0);
}

void D3D::SetRenderTarget(ID3D11RenderTargetView * rtv, ID3D11DepthStencilView * dsv)
{
	D3D::Get()->GetDeviceContext()->OMSetRenderTargets(1, &rtv, dsv);
}

void D3D::SetRenderTargets(uint viewNum, ID3D11RenderTargetView * const * rtvs, ID3D11DepthStencilView * dsv)
{
	D3D::Get()->GetDeviceContext()->OMSetRenderTargets(viewNum, rtvs, dsv);
}
