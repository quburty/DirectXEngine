#include "stdafx.h"
#include "Mesh/MeshTerrain.h"
#include "Mesh/MeshSphere.h"
#include "Mesh/SkyCube.h"
#include "Shader/ShaderBasic.h"
#include "Shader/Texture.h"

void Initialize();
void Update();
void Render();
void Destroy();

using namespace std;

MeshTerrain* terrain;
MeshSphere* sphere;

Shader* terrainShader;
Shader* sphereShader;

ID3D11Texture2D* dstTexture;
uint* uintArr;

D3D11_MAPPED_SUBRESOURCE subresource;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Window::Get()->Initialize(hInstance, hPrevInstance, pCmdLine, nCmdShow, L"MapEditor");

	Initialize();

	// Run the message loop.

	MSG msg = { };
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT
				|| Keyboard::Get()->Down(VK_ESCAPE))
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Update();
		Render();

	}

	Destroy();

	return 0;
}

void Initialize()
{
	D3D::Get()->Initialize(Window::Get()->GetHWND(), Window::Get()->GetDisplayWidth(), Window::Get()->GetDisplayHeight());
	Keyboard::Get()->Initialize();
	Gui::Get()->Initialize();
	Time::Get()->Initialize();
	Time::Get()->Start();
	Mouse::Get()->Initialize();
	Camera::Get()->Initialize();

	Camera::Get()->Position(0,500,0);

	terrainShader = new ShaderBasic(L"../_Shaders/Normal.hlsl");
	terrainShader->SetDiffuseMap(L"../_Textures/Terrain/Forest_Floor.jpg");
	terrain = new MeshTerrain();
	terrain->SetHeightMap(L"../_Textures/Terrain/Gray256.png");
	terrain->AddShader(terrainShader);

	terrainShader->CreateVertexBuffer((void*)terrain->GetVertices().data(), sizeof(Vertex), terrain->GetVertices().size(), true, true);
	terrain->InitializeGPGPU();

	sphereShader = new ShaderBasic(L"../_Shaders/Specular.hlsl");
	sphereShader->SetDiffuseMap(L"../_Textures/Red.png");
	sphere = new MeshSphere();
	sphere->AddShader(sphereShader);
	sphere->Scale(D3DXVECTOR3(50, 50, 50));

	uintArr = new uint[Window::Get()->GetDisplayHeight() * Window::Get()->GetDisplayWidth()];

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.Height = Window::Get()->GetDisplayHeight();
	desc.Width = Window::Get()->GetDisplayWidth();
	desc.MipLevels = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_STAGING;

	auto hr = D3D::Get()->GetDevice()->CreateTexture2D(&desc, nullptr, &dstTexture);
	Check(hr);
	
	ZeroMemory(&subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
}

void Update()
{
	D3D::Get()->Update();
	Keyboard::Get()->Update();
	Gui::Get()->Update();
	Time::Get()->Update();
	Mouse::Get()->Update();
	Camera::Get()->Update();
	
	{
		D3DXVECTOR3 mousePos = Mouse::Get()->GetPosition();

		D3D::Get()->GetDeviceContext()->CopySubresourceRegion(dstTexture, 0, 0, 0, 0,
			D3D::Get()->GetDepthTexture(), 0, nullptr);

		auto hr = D3D::Get()->GetDeviceContext()->Map(dstTexture, 0, D3D11_MAP_READ, 0, &subresource);
		Check(hr);
		{
			memcpy(uintArr, subresource.pData, Window::Get()->GetDisplayHeight() * Window::Get()->GetDisplayWidth()*4);
		}
		D3D::Get()->GetDeviceContext()->Unmap(dstTexture, 0);

		uint idx = ((uint)mousePos.y)*Window::Get()->GetDisplayWidth() + (uint)mousePos.x;
		D3DXVECTOR3 worldPos;
		uint value = uintArr[idx];

		float depth = (float)((0x00FFFFFF & value) >> 0) / (float)(0x00FFFFFF);

		float x = mousePos.x / Window::Get()->GetDisplayWidth() * 2 - 1;
		float y = (1 - mousePos.y / Window::Get()->GetDisplayHeight()) * 2 - 1;
		
		D3DXVECTOR4 pos4 = D3DXVECTOR4(x, y, depth, 1.0f);

		D3DXMATRIX invProj, proj;
		Camera::Get()->GetProjMatrix(&proj);
		D3DXMatrixTranspose(&proj, &proj);
		D3DXMatrixInverse(&invProj, nullptr, &proj);

		D3DXMATRIX invView, view;
		Camera::Get()->GetMatrix(&view);
		D3DXMatrixTranspose(&view, &view);
		D3DXMatrixInverse(&invView, nullptr, &view);
		
		D3DXVec4Transform(&pos4, &pos4, &invProj);
		pos4 /= pos4.w;

		D3DXVec4Transform(&pos4, &pos4, &invView);
		pos4 /= pos4.w;

		worldPos = D3DXVECTOR3(pos4.x, pos4.y, pos4.z);

		ImGui::Text("World x: %f y: %f z: %f", worldPos.x, worldPos.y, worldPos.z);

		sphere->Position(worldPos);

		if (Keyboard::Get()->Press('A'))
			terrain->Edit(worldPos);
	}

	static D3DXVECTOR3 dir{ 1,0,0 };
	ImGui::SliderFloat3("dir", dir, -1, 1);
	D3D::Get()->SetLight(&dir);
}

void Render()
{
	D3D::Get()->Render_Begin();
	Time::Get()->Render();

	D3D::Get()->SetRenderTarget(D3D::Get()->GetRTV(), nullptr);
	D3D::Get()->SetRenderTarget(D3D::Get()->GetRTV(), D3D::Get()->GetDSV());

	terrainShader->Render(terrain);

	Gui::Get()->Render();

	D3D::Get()->SetRenderTarget(D3D::Get()->GetRTV(), nullptr);
	sphereShader->Render(sphere);
	D3D::Get()->SetRenderTarget(D3D::Get()->GetRTV(), D3D::Get()->GetDSV());
	
	D3D::Get()->Render_End();
}

void Destroy()
{
	SAFE_DELETE_ARRAY(uintArr);

	SAFE_RELEASE(dstTexture);

	SAFE_DELETE(sphere);
	SAFE_DELETE(terrain);

	Camera::Get()->Destroy();
	Mouse::Get()->Destroy();
	Time::Get()->Stop();
	Time::Get()->Destroy();
	Gui::Get()->Destroy();
	Keyboard::Get()->Destroy();
	D3D::Get()->Destroy();
	Window::Get()->Destroy();
}
