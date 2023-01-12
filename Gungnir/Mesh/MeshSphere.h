#pragma once
#include "Mesh.h"

class MeshSphere : public Mesh
{
public:
	MeshSphere();
	virtual ~MeshSphere();

private:
	float radius = 0.5f;

	uint stackCount = 20;
	uint sliceCount = 20;

};