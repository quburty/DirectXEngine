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
		//�̰� new�� �������� ���������� ���� ��Ծ�?
		Shader* shader = new ShaderBasic(); //47ms
		shader->SetVertexShader(L"../_Shaders/Model_VS.cso"); //7ms
		shader->SetPixelShader(L"../_Shaders/Model_PS.cso");
		modelMesh->AddShader(shader); //4000ms
	}
	//TODO:
	//�׸��� �׳� ���� ���� �ý���?���� �ϳ� ����� �ǳ� �� �߰��Ҷ�����
	//Update()�� �־��ְ� Render()�� �־��ְ� �����ְ� ���ظ԰���
	//�� �ǳ� DXGI WARNING�߰�

	//TODO: �׸��� diffuse�� �׷����ĵ� normalmap specularmap �������� �ִµ�
	//������ ��ó ����ҷ�?
	//�ϴ� �ش� srv�� ���Դ��� bool������ üũ�Ҽ� �ְ� �س���

	//�׸��� �ӵ� ���̷��� ����?
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
