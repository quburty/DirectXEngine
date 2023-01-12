#include "stdafx.h"
#include "Mesh/SkyCube.h"
#include "Player/Player.h"
#include "Shader/ShaderBasic.h"

void Initialize();
void Update();
void Render();
void Destroy();

using namespace std;

SkyCube* sky;
Shader* skyShader;

Player* player;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Window::Get()->Initialize(hInstance, hPrevInstance, pCmdLine, nCmdShow, L"Client");

	Initialize();

	// Run the message loop.

	MSG msg = { };
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
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
	Mouse::Get()->Initialize();
	Camera::Get()->Initialize();
	
	skyShader = new ShaderBasic(L"../_Shaders/SkyCube.hlsl");//80ms
	skyShader->SetDiffuseMap(L"../_Textures/CubeMap/Texture1.dds");//22ms
	sky = new SkyCube();
	sky->AddShader(skyShader);

	player = new Player();

	Time::Get()->Start();
}

void Update()
{
	D3D::Get()->Update();
	Keyboard::Get()->Update();
	Gui::Get()->Update();
	Time::Get()->Update();
	Mouse::Get()->Update();
	Camera::Get()->Update();

	sky->Update();
	
	//if (Keyboard::Get()->Press('W'))
	//	player->Position(pos + D3DXVECTOR3(0, 0, 100 * Time::Get()->Delta()));
	//if (Keyboard::Get()->Press('S'))
	//	player->Position(pos + D3DXVECTOR3(0, 0, -100 * Time::Get()->Delta()));
	//if (Keyboard::Get()->Press('A'))
	//	player->Position(pos + D3DXVECTOR3(-100 * Time::Get()->Delta(),0,0));
	//if (Keyboard::Get()->Press('D'))
	//	player->Position(pos + D3DXVECTOR3(100 * Time::Get()->Delta(),0,0));

	player->Update();

	static D3DXVECTOR3 dir{ 1,0,0 };
	ImGui::NewLine();
	ImGui::SliderFloat3("dir", dir, -1, 1);
	D3D::Get()->SetLight(&dir);
	
}

void Render()
{
	D3D::Get()->Render_Begin();
	Time::Get()->Render();

	D3D::Get()->SetRenderTarget(D3D::Get()->GetRTV(), nullptr);
	sky->Render();
	D3D::Get()->SetRenderTarget(D3D::Get()->GetRTV(), D3D::Get()->GetDSV());

	player->Render();

	Gui::Get()->Render();
	D3D::Get()->Render_End();
}

void Destroy()
{
	SAFE_DELETE(player);

	SAFE_DELETE(sky);
	

	Camera::Get()->Destroy();
	Mouse::Get()->Destroy();
	Time::Get()->Stop();
	Time::Get()->Destroy();
	Gui::Get()->Destroy();
	Keyboard::Get()->Destroy();
	D3D::Get()->Destroy();
	Window::Get()->Destroy();
}
