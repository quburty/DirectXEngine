#pragma once

class Player final
{
public:
	Player();
	~Player();

	void Update();
	void Render();

	class ModelRender* GetModel() { return model; }

private:
	class ModelRender* model;
};