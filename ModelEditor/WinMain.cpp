#include "stdafx.h"
#include "Mesh/SkyCube.h"
#include "Model/ModelRender.h"
#include "Shader/ShaderBasic.h"
#include "Shader/ShaderInst.h"
#include "Converter.h"

#include "Mesh/MeshSphere.h"

void Initialize();
void Update();
void Render();
void Destroy();

bool SelectFile(HWND hwnd, OUT std::wstring& wFile, LPWSTR filter, bool bSave = false)
{
	wchar_t file[256] = L"";

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = file;
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = filter;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = L"c:\\";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR;
	if (bSave == false)
		ofn.Flags |= OFN_FILEMUSTEXIST;

	ofn.lpstrTitle = TEXT("title");
	
	bool result;

	bSave ? result = GetSaveFileName(&ofn) : result = GetOpenFileName(&ofn);

	result == true ? wFile = file : wFile = L"";

	return result;
}

void RecursiveTree(ModelRender* model, uint idx, uint& select, std::valarray<bool>& bArray);

using namespace std;

SkyCube* sky;
Shader* skyShader;
Converter* converter;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Window::Get()->Initialize(hInstance, hPrevInstance, pCmdLine, nCmdShow, L"ModelEditor");

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

	converter = new Converter();

	Camera::Get()->Position(0, 130, -320);

	sky = new SkyCube();
	skyShader = new ShaderBasic(L"../_Shaders/SkyCube.hlsl");
	skyShader->RegisterMesh(sky);
	skyShader->SetDiffuseMap(L"../_Textures/CubeMap/Texture1.dds");

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
	
	static D3DXVECTOR3 dir{ 1,0,0 };
	ImGui::NewLine();
	ImGui::SliderFloat3("dir", dir, -1, 1);
	D3D::Get()->SetLight(&dir);
	
	converter->Update();
	
	if (ImGui::Button("Load FBX Model"))
	{
		std::wstring wFile;
		if (SelectFile(Window::Get()->GetHWND(), wFile, L"FBX Model File\0*.fbx\0"))
		{
			converter->EndModelRender();
			converter->ReadFBX(wFile);
			converter->StartModelRender();
		}
	}

	if (ImGui::Button("Load FBX Anim"))
	{
		std::wstring wFile;
		if (SelectFile(Window::Get()->GetHWND(), wFile, L"FBX Anim File\0*.fbx\0"))
		{
			converter->EndModelRender();
			converter->ReadFBXAnim(wFile);
			converter->StartModelRender();
		}
	}

	if (ImGui::Button("Convert to model"))
	{
		std::wstring wFile;
		if (converter->IsModelAvailable()
			&& SelectFile(Window::Get()->GetHWND(), wFile, L"Model File\0*.model\0", true))
		{
			converter->WriteModel(wFile);
		}
	}

	if (ImGui::Button("Save to material"))
	{
		std::wstring wFile;
		if (converter->IsModelAvailable()
			&& SelectFile(Window::Get()->GetHWND(), wFile, L"Material File\0*.material\0", true))
		{
			converter->WriteMaterialsFile(wFile);
		}
	}

	if (ImGui::Button("Convert to clip"))
	{
		std::wstring wFile;
		if (converter->IsAnimAvailable()
			&& SelectFile(Window::Get()->GetHWND(), wFile, L"Clip File\0*.clip\0", true))
		{
			converter->WriteAnimationFile(wFile, 0);
		}
	}

	ModelRender* model = converter->GetModel();

	if (model == nullptr)
		return;

	//Bone Select
	static uint idx = 0;
	static std::valarray<bool> bArray(false, model->GetBoneCount());
	RecursiveTree(model, 0, idx, bArray);

	Model::ModelBone* currentBone = model->GetBone(idx);
	ImGui::Text("Current Bone: %s", currentBone->name.c_str());

	//Gizmo
	{
		D3DXMATRIX view, proj;
		Camera::Get()->GetMatrix(&view);
		Camera::Get()->GetProjMatrix(&proj);
		D3DXMatrixTranspose(&view, &view);
		D3DXMatrixTranspose(&proj, &proj);

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
		if (ImGui::IsKeyPressed(90))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(69))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(82)) // r Key
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
			mCurrentGizmoOperation = ImGuizmo::SCALE;

		ImGuizmo::Manipulate(view, proj, mCurrentGizmoOperation, mCurrentGizmoMode,
			currentBone->w_transform);
	}

	//pos,rot,scale value
	{
		D3DXMATRIX local = currentBone->l_transform;
		D3DXVECTOR3 scale, rot, pos;
		
		Math::MatrixDecompose(local, scale, rot, pos);
	
		ImGui::SliderFloat3("Scale", scale, -100.f, +100.f);
		ImGui::SliderFloat3("Rotation", rot, -360.f, +360.f);
		ImGui::SliderFloat3("Position", pos, -500.f, +500.f);
	}
	
	
}

void Render()
{
	D3D::Get()->Render_Begin();
	Time::Get()->Render();

	D3D::Get()->SetRenderTarget(D3D::Get()->GetRTV(), nullptr);
	skyShader->Render(sky);
	D3D::Get()->SetRenderTarget(D3D::Get()->GetRTV(), D3D::Get()->GetDSV());

	converter->Render();

	Gui::Get()->Render();
	D3D::Get()->Render_End();
}

void Destroy()
{
	converter->EndModelRender();
	SAFE_DELETE(converter);

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

void RecursiveTree(ModelRender* model, uint idx, uint& select, std::valarray<bool>& bArray)
{
	ImGuiTreeNodeFlags flags = 0;

	Model::ModelBone* bone = model->GetBone(idx);
	
	if (select == idx)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (bone->childs.size() == 0)
		flags |= ImGuiTreeNodeFlags_Bullet;

	bool selected = ImGui::TreeNodeEx(bone->name.c_str(), flags);
	if (selected)
	{
		for (uint i = 0; i < bone->childs.size(); i++)
			RecursiveTree(model, bone->childs[i], select, bArray);

		ImGui::TreePop();
	}

	if (selected != bArray[idx])
	{
		bArray[idx] = selected;
		select = idx;
	}
}
