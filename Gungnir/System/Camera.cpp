#include "Framework.h"
#include "Camera.h"

void Camera::Initialize()
{
	//View
	D3DXMatrixIdentity(&matRotation);
	D3DXMatrixIdentity(&matView);

	Position(0, 0, -100);
	Rotation();
	Move();

	//Projection
	D3DXMatrixIdentity(&matProj);

	D3DXMatrixPerspectiveFovLH
	(
		&matProj,
		(float)D3DX_PI / 4,
		Window::Get()->GetDisplayWidthFloat() / Window::Get()->GetDisplayHeightFloat(),
		nearZ,
		farZ
	);

	D3DXMatrixTranspose(&matProj, &matProj);
}

void Camera::Destroy()
{
}

void Camera::Update()
{
	if (Mouse::Get()->Press(1) == false) return;

	Vector3 f = Forward();
	Vector3 u = Up();
	Vector3 r = Right();

	//Move
	{
		Vector3 P;
		Position(&P);

		if (Keyboard::Get()->Press('W'))
			P = P + f * moveSpeed * Time::Get()->Delta();
		else if (Keyboard::Get()->Press('S'))
			P = P - f * moveSpeed * Time::Get()->Delta();

		if (Keyboard::Get()->Press('D'))
			P = P + r * moveSpeed * Time::Get()->Delta();
		else if (Keyboard::Get()->Press('A'))
			P = P - r * moveSpeed * Time::Get()->Delta();

		if (Keyboard::Get()->Press('E'))
			P = P + u * moveSpeed * Time::Get()->Delta();
		else if (Keyboard::Get()->Press('Q'))
			P = P - u * moveSpeed * Time::Get()->Delta();

		Position(P);
	}


	//Rotation
	{
		Vector3 R;
		Rotation(&R);

		Vector3 val = Mouse::Get()->GetMoveValue();
		R.x = R.x + val.y * rotationSpeed * Time::Get()->Delta();
		R.y = R.y + val.x * rotationSpeed * Time::Get()->Delta();
		R.z = 0.0f;

		Rotation(R);
	}
}

void Camera::Position(float x, float y, float z)
{
	Position(Vector3(x, y, z));
}

void Camera::Position(Vector3 & vec)
{
	position = vec;

	Move();
}

void Camera::Position(Vector3 * vec)
{
	*vec = position;
}

void Camera::Rotation(float x, float y, float z)
{
	Rotation(Vector3(x, y, z));
}

void Camera::Rotation(Vector3 & vec)
{
	rotation = vec;

	Rotation();
}

void Camera::Rotation(Vector3 * vec)
{
	*vec = rotation;
}

void Camera::RotationDegree(float x, float y, float z)
{
	RotationDegree(D3DXVECTOR3(x, y, z));
}

void Camera::RotationDegree(Vector3 & vec)
{
	//rotation = vec * Math::PI / 180.0f;
	rotation = vec * 0.01745328f;

	Rotation();
}

void Camera::RotationDegree(Vector3 * vec)
{
	//*vec = rotation * 180.0f / Math::PI;
	*vec = rotation * 57.29577957f;
}

void Camera::GetMatrix(Matrix * matrix)
{
	//*matrix = matView;
	memcpy(matrix, &matView, sizeof(Matrix));
}

void Camera::GetProjMatrix(Matrix * matrix)
{
	memcpy(matrix, &matProj, sizeof(Matrix));
}

void Camera::Rotation()
{
	Matrix X, Y, Z;
	D3DXMatrixRotationX(&X, rotation.x);
	D3DXMatrixRotationY(&Y, rotation.y);
	D3DXMatrixRotationZ(&Z, rotation.z);

	matRotation = X * Y * Z;


	D3DXVec3TransformNormal(&forward, &Vector3(0, 0, 1), &matRotation);
	D3DXVec3TransformNormal(&up, &Vector3(0, 1, 0), &matRotation);
	D3DXVec3TransformNormal(&right, &Vector3(1, 0, 0), &matRotation);
}

void Camera::Move()
{
	View();
}

void Camera::View()
{
	D3DXMatrixLookAtLH(&matView, &position, &(position + forward), &up);
	D3DXMatrixTranspose(&matView, &matView);
}

void Camera::Speed(float move, float rotation)
{
	this->moveSpeed = move;
	this->rotationSpeed = rotation;
}
