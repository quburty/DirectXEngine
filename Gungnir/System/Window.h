#pragma once
#include "Framework.h"

#define INITIAL_WIDTH 1600
#define INITIAL_HEIGHT 900

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class Window final
{
private:
	Window();
	~Window();

public:
	static Window* Get()
	{
		static Window object;
		return &object;
	}

	void Initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow, LPCWSTR lpWindowName);
	void Destroy();

	void Resize(uint newWidth, uint newHeight, bool bIsFullScreen = false) {}

	const uint& GetDisplayWidth() { return displayWidth; }
	const uint& GetDisplayHeight() { return displayHeight; }

	float GetDisplayWidthFloat()  { return static_cast<float>(displayWidth); }
	float GetDisplayHeightFloat() { return static_cast<float>(displayHeight); }

	const uint& GetWindowWidth() { return windowWidth; }
	const uint& GetWindowHeight() { return windowHeight; }

	float GetWindowWidthFloat() { return static_cast<float>(windowWidth); }
	float GetWindowHeightFloat() { return static_cast<float>(windowHeight); }

	HWND GetHWND() { return hwnd; }

private:
	uint displayWidth = INITIAL_WIDTH;
	uint displayHeight = INITIAL_HEIGHT;

	uint windowWidth;
	uint windowHeight;

	HWND hwnd = NULL;
	HINSTANCE hInstance;
	const wchar_t* CLASS_NAME = L"DirectX Window Class";
};