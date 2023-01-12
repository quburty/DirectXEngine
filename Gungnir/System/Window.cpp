#include "Framework.h"
#include "Window.h"

Window::Window()
{
}

Window::~Window()
{
	Destroy();
}

void Window::Initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow, LPCWSTR lpWindowName)
{
	this->hInstance = hInstance;

	// Register the window class.
	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	RECT rect = { 0,0,INITIAL_WIDTH,INITIAL_HEIGHT };
	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	windowWidth = rect.right - rect.left;
	windowHeight = rect.bottom - rect.top;

	// Create the window.
	hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		lpWindowName,    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	assert(hwnd != NULL);

	ShowWindow(hwnd, nCmdShow);
}

void Window::Destroy()
{
	UnregisterClass(CLASS_NAME, hInstance);
	DestroyWindow(hwnd);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Mouse::Get()->InputProc(uMsg, wParam, lParam);

	if (Gui::Get()->MsgProc(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);



		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
	}
	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}