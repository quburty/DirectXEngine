#include "Framework.h"
#include "MeshCube.h"

MeshCube::MeshCube()
	: Mesh()
{
	vertices.resize(24);
	vertices[0] = { -0.5f,-0.5f,-0.5f,0.0f,1.0f }; vertices[0].normal = D3DXVECTOR3(0, 0, -1); vertices[0].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[1] = { -0.5f,+0.5f,-0.5f,0.0f,0.0f }; vertices[1].normal = D3DXVECTOR3(0, 0, -1); vertices[1].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[2] = { +0.5f,-0.5f,-0.5f,1.0f,1.0f }; vertices[2].normal = D3DXVECTOR3(0, 0, -1); vertices[2].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[3] = { +0.5f,+0.5f,-0.5f,1.0f,0.0f }; vertices[3].normal = D3DXVECTOR3(0, 0, -1); vertices[3].tangent = D3DXVECTOR3(1, 0, 0);

	vertices[4] = { -0.5f,+0.5f,-0.5f,0.0f,1.0f }; vertices[4].normal = D3DXVECTOR3(0, +1, 0); vertices[4].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[5] = { -0.5f,+0.5f,+0.5f,0.0f,0.0f }; vertices[5].normal = D3DXVECTOR3(0, +1, 0); vertices[5].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[6] = { +0.5f,+0.5f,-0.5f,1.0f,1.0f }; vertices[6].normal = D3DXVECTOR3(0, +1, 0); vertices[6].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[7] = { +0.5f,+0.5f,+0.5f,1.0f,0.0f }; vertices[7].normal = D3DXVECTOR3(0, +1, 0); vertices[7].tangent = D3DXVECTOR3(1, 0, 0);

	vertices[8] = { +0.5f,-0.5f,+0.5f,0.0f,1.0f }; vertices[8].normal = D3DXVECTOR3(0, 0, +1); vertices[8].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[9] = { +0.5f,+0.5f,+0.5f,0.0f,0.0f }; vertices[9].normal = D3DXVECTOR3(0, 0, +1); vertices[9].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[10] = { -0.5f,-0.5f,+0.5f,1.0f,1.0f }; vertices[10].normal = D3DXVECTOR3(0, 0, +1); vertices[10].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[11] = { -0.5f,+0.5f,+0.5f,1.0f,0.0f }; vertices[11].normal = D3DXVECTOR3(0, 0, +1); vertices[11].tangent = D3DXVECTOR3(1, 0, 0);

	vertices[12] = { -0.5f,-0.5f,+0.5f,0.0f,1.0f }; vertices[12].normal = D3DXVECTOR3(0, -1, 0); vertices[12].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[13] = { -0.5f,-0.5f,-0.5f,0.0f,0.0f }; vertices[13].normal = D3DXVECTOR3(0, -1, 0); vertices[13].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[14] = { +0.5f,-0.5f,+0.5f,1.0f,1.0f }; vertices[14].normal = D3DXVECTOR3(0, -1, 0); vertices[14].tangent = D3DXVECTOR3(1, 0, 0);
	vertices[15] = { +0.5f,-0.5f,-0.5f,1.0f,0.0f }; vertices[15].normal = D3DXVECTOR3(0, -1, 0); vertices[15].tangent = D3DXVECTOR3(1, 0, 0);

	vertices[16] = { +0.5f,-0.5f,-0.5f,0.0f,1.0f }; vertices[16].normal = D3DXVECTOR3(+1, 0, 0); vertices[16].tangent = D3DXVECTOR3(0, 0, 1);
	vertices[17] = { +0.5f,+0.5f,-0.5f,0.0f,0.0f }; vertices[17].normal = D3DXVECTOR3(+1, 0, 0); vertices[17].tangent = D3DXVECTOR3(0, 0, 1);
	vertices[18] = { +0.5f,-0.5f,+0.5f,1.0f,1.0f }; vertices[18].normal = D3DXVECTOR3(+1, 0, 0); vertices[18].tangent = D3DXVECTOR3(0, 0, 1);
	vertices[19] = { +0.5f,+0.5f,+0.5f,1.0f,0.0f }; vertices[19].normal = D3DXVECTOR3(+1, 0, 0); vertices[19].tangent = D3DXVECTOR3(0, 0, 1);

	vertices[20] = { -0.5f,-0.5f,+0.5f,0.0f,1.0f }; vertices[20].normal = D3DXVECTOR3(-1, 0, 0); vertices[20].tangent = D3DXVECTOR3(0, 0, -1);
	vertices[21] = { -0.5f,+0.5f,+0.5f,0.0f,0.0f }; vertices[21].normal = D3DXVECTOR3(-1, 0, 0); vertices[21].tangent = D3DXVECTOR3(0, 0, -1);
	vertices[22] = { -0.5f,-0.5f,-0.5f,1.0f,1.0f }; vertices[22].normal = D3DXVECTOR3(-1, 0, 0); vertices[22].tangent = D3DXVECTOR3(0, 0, -1);
	vertices[23] = { -0.5f,+0.5f,-0.5f,1.0f,0.0f }; vertices[23].normal = D3DXVECTOR3(-1, 0, 0); vertices[23].tangent = D3DXVECTOR3(0, 0, -1);

	indices = {
	0,1,2,2,1,3,
	4,5,6,6,5,7,
	8,9,10,10,9,11,
	12,13,14,14,13,15,
	16,17,18,18,17,19,
	20,21,22,22,21,23
	};
}

MeshCube::~MeshCube()
{
}
