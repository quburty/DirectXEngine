#pragma once
#include "MeshCube.h"

class SkyCube : public MeshCube
{
public:
	SkyCube();
	virtual ~SkyCube();

	void AddShader(class Shader* shader) override;

	void Update() override;
};