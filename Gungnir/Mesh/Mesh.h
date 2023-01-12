#pragma once

class Mesh
{
public:
	friend class ModelRender;

public:
	Mesh();
	virtual ~Mesh();

	virtual void Update() {}
	virtual void Render();

	class Shader* GetShader(uint idx) { return shaders.at(idx); }
	virtual void AddShader(class Shader* shader);
	void DelistShader(class Shader* shader);

	void SetInstancing(bool instancing) { this->instancing = instancing; }

protected:
	using Vector3 = D3DXVECTOR3;

public:
	//row-major
	void SetWorld(D3DXMATRIX world);

	void Position(float x, float y, float z);
	void Position(Vector3& vec);
	void Position(Vector3* vec);

	void Rotation(float x, float y, float z);
	void Rotation(Vector3& vec);
	void Rotation(Vector3* vec);

	void RotationDegree(float x, float y, float z);
	void RotationDegree(Vector3& vec);
	void RotationDegree(Vector3* vec);

	void Scale(Vector3& vec);

	//row-major
	const D3DXMATRIX& GetWorld() { return this->world; }
	const D3DXMATRIX& GetTransWorld() { return this->transWorld; }

private:
	//row-major
	void WorldUpdate();

public:
	const std::vector<Vertex>& GetVertices() { return vertices; }
	const std::vector<uint>& GetIndices() { return indices; }

private:
	Vector3 position;
	Vector3 rotation; //yaw pitch roll
	Vector3 scale;

	D3DXMATRIX world; //row-major
	D3DXMATRIX transWorld; //col-major

	bool instancing = false;

protected:
	std::vector<class Shader*> shaders;

	std::vector<Vertex> vertices;
	std::vector<uint> indices;

};