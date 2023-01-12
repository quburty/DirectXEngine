#pragma once

struct GuiText
{
	D3DXVECTOR2 Position;
	D3DXCOLOR Color;
	std::string Content;

	GuiText()
	{

	}
};

class Gui
{
public:
	void Initialize();
	void Destroy();

	static Gui* Get();

	LRESULT MsgProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
	void Resize();

	void Update();
	void Render();

	void AddWidget(class Widget* widget);

	void RenderText(GuiText& text);
	void RenderText(float x, float y, std::string content);
	void RenderText(float x, float y, float r, float g, float b, std::string content);
	void RenderText(D3DXVECTOR2 position, D3DXCOLOR color, std::string content);

private:
	Gui();
	~Gui();

private:
	void ApplyStyle();

private:
	std::vector<GuiText> texts;
};