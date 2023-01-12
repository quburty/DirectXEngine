#include "stdafx.h"
#include "Player.h"
#include "Model/ModelRender.h"
#include "Mesh/Mesh.h"
#include "Shader/ShaderBasic.h"

Player::Player()
{
	model = new ModelRender();
	model->ReadModel(L"../_Models/mutant.model");//1400ms
	for (uint i = 0; i < model->GetMeshCount(); i++)
	{
		Mesh* modelMesh = model->GetMeshes() + i;
		//TODO:
		//이거 new로 만들어놓고 지워지는지 누가 어떻게앎?
		Shader* shader = new ShaderBasic(); //47ms
		shader->SetVertexShader(L"../_Shaders/Model_VS.cso"); //7ms
		shader->SetPixelShader(L"../_Shaders/Model_PS.cso");
		modelMesh->AddShader(shader); //4000ms
	}
	//TODO:
	//그리고 그냥 액터 관리 시스템?같이 하나 만들것 맨날 뭐 추가할때마다
	//Update()에 넣어주고 Render()에 넣어주고 지워주고 못해먹겠음
	//뭐 맨날 DXGI WARNING뜨고

	//TODO: 그리고 diffuse는 그렇다쳐도 normalmap specularmap 없을수도 있는데
	//없을때 대처 어떻게할래?
	//일단 해당 srv가 들어왔는지 bool값으로 체크할수 있게 해놨음

	//그리고 속도 왜이렇게 느림?
	model->ReadMaterial(L"../_Models/mutant.material");
}

Player::~Player()
{
	SAFE_DELETE(model);
}

void Player::Update()
{
	model->Update();
}

void Player::Render()
{
	model->Render();
}
