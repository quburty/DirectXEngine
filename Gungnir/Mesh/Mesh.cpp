#include "Framework.h"
#include "Mesh.h"
#include "Mesh/Vertex.h"
#include "Shader/Shader.h"

Mesh::Mesh()
{
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	SetWorld(identity);
}

Mesh::~Mesh()
{
	for (Shader* shader : shaders)
	{
		if (shader != nullptr)
		{
			shader->DelistMesh(this);

			if (shader->GetMeshRefNum() == 0)
				SAFE_DELETE(shader);
		}
	}

	shaders.clear();
}

void Mesh::Render()
{
	for (Shader* shader : shaders)
		shader->Render(this);
}

void Mesh::AddShader(Shader * shader)
{
	if (shader == nullptr)
		return;

	shaders.push_back(shader);

	shader->RegisterMesh(this);

	if (shader->IsVertexBufferValid() == false)
		shader->CreateVertexBuffer(vertices.data(), sizeof(Vertex), vertices.size());
	if (shader->IsIndexBufferValid() == false)
		shader->CreateIndexBuffer(indices.data(), indices.size());
}

void Mesh::DelistShader(Shader * shader)
{
	auto it = std::find(shaders.begin(), shaders.end(), shader);
	
	if (it != shaders.end())
		shaders.erase(it);
}


void Mesh::SetWorld(D3DXMATRIX world)
{
	D3DXQUATERNION quat;

	D3DXMatrixDecompose(&scale, &quat, &position, &world);
	
	float sqw = quat.w*quat.w;
	float sqx = quat.x*quat.x;
	float sqy = quat.y*quat.y;
	float sqz = quat.z*quat.z;

	rotation.x = asinf(2.0f*(quat.w*quat.x - quat.y*quat.z));
	rotation.y = atan2f(2.0f*(quat.x*quat.z + quat.w*quat.y), (-sqx - sqy + sqz + sqw));
	rotation.z = atan2f(2.0f*(quat.x*quat.y + quat.w*quat.z), (-sqx + sqy -	 sqz + sqw));
	
	WorldUpdate();
}

void Mesh::Position(float x, float y, float z)
{
	Position(Vector3(x, y, z));
}

void Mesh::Position(Vector3 & vec)
{
	position = vec;

	WorldUpdate();
}

void Mesh::Position(Vector3 * vec)
{
	*vec = position;
}

void Mesh::Rotation(float x, float y, float z)
{
	Rotation(Vector3(x, y, z));
}

void Mesh::Rotation(Vector3 & vec)
{
	rotation = vec;

	WorldUpdate();
}

void Mesh::Rotation(Vector3 * vec)
{
	*vec = rotation;
}

void Mesh::RotationDegree(float x, float y, float z)
{
	RotationDegree(D3DXVECTOR3(x, y, z));
}

void Mesh::RotationDegree(Vector3 & vec)
{
	//rotation = vec * Math::PI / 180.0f;
	rotation = vec * 0.01745328f;

	WorldUpdate();
}

void Mesh::RotationDegree(Vector3 * vec)
{
	//*vec = rotation * 180.0f / Math::PI;
	*vec = rotation * 57.29577957f;
}

void Mesh::Scale(Vector3 & vec)
{
	scale = vec;

	WorldUpdate();
}

void Mesh::WorldUpdate()
{
	D3DXMATRIX S, R, T;

	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationYawPitchRoll(&R, rotation.x, rotation.y, rotation.z);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	this->world = S * R*T;

	D3DXMatrixTranspose(&transWorld, &this->world);
}