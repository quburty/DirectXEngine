#include "Framework.h"
#include "MeshSphere.h"

MeshSphere::MeshSphere()
	:Mesh()
{
	vertices.reserve((stackCount - 1)*sliceCount + 2);
	vertices.push_back(Vertex(0.0f, radius, 0.0f, 0.0f, 0.0f));

	float phiStep = Math::PI / stackCount;
	float thetaStep = Math::PI * 2.0f / sliceCount;

	for (uint i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;

		for (uint k = 0; k <= sliceCount; k++)
		{
			float theta = k * thetaStep;

			D3DXVECTOR3 p = D3DXVECTOR3
			(
				(radius * sinf(phi) * cosf(theta)),
				(radius * cos(phi)),
				(radius * sinf(phi) * sinf(theta))
			);

			D3DXVECTOR3 n;
			D3DXVec3Normalize(&n, &p);

			D3DXVECTOR2 uv = D3DXVECTOR2(theta / (Math::PI * 2), phi / Math::PI);

			Vertex v(p.x, p.y, p.z, uv.x, uv.y);
			v.normal = p;

			vertices.push_back(v);
		}
	}
	vertices.push_back(Vertex(0, -radius, 0, 0, 0));

	for (uint i = 1; i <= sliceCount; i++)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	uint baseIndex = 1;
	uint ringVertexCount = sliceCount + 1;
	for (uint i = 0; i < stackCount - 2; i++)
	{
		for (uint j = 0; j < sliceCount; j++)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	uint southPoleIndex = vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint i = 0; i < sliceCount; i++)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}
}

MeshSphere::~MeshSphere()
{
}
