#pragma once

class Camera
{
private:
	using Vector3 = D3DXVECTOR3;
	using Matrix = D3DXMATRIX;

	Camera() {}
	~Camera() {}

public:
	static Camera* Get()
	{
		static Camera object;
		return &object;
	}

	void Initialize();
	void Destroy();

	void Update();

public:
	void Position(float x, float y, float z);
	void Position(Vector3& vec);
	void Position(Vector3* vec);

	void Rotation(float x, float y, float z);
	void Rotation(Vector3& vec);
	void Rotation(Vector3* vec);

	void RotationDegree(float x, float y, float z);
	void RotationDegree(Vector3& vec);
	void RotationDegree(Vector3* vec);

	void GetMatrix(Matrix* matrix);

	Vector3 Forward() { return forward; }
	Vector3 Up() { return up; }
	Vector3 Right() { return right; }

	void GetProjMatrix(Matrix* matrix);

	float Near() { return nearZ; }
	float Far() { return farZ; }

protected:
	virtual void Rotation();
	virtual void Move();

protected:
	void View();

public:
	void Speed(float move, float rotation = 2.0f);

private:
	float moveSpeed = 100.0f;
	float rotationSpeed = 2.0f;

protected:
	Matrix matView; //뷰 행렬, col-major
	Matrix matProj; //프로젝션 행렬, col-major

private:
	Vector3 position = Vector3(0, 0, 0);
	Vector3 rotation = Vector3(0, 0, 0);

	Vector3 forward = Vector3(0, 0, 1);
	Vector3 up = Vector3(0, 1, 0);
	Vector3 right = Vector3(1, 0, 0);

	Matrix matRotation; //회전 행렬

	float nearZ = 1.0f;
	float farZ = 2000.f;
};