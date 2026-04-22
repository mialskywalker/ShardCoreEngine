#include "Window.h"

Window::Window() {}

Window::~Window() {}

// windows calls this functions for window messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Window::Init()
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"ShardCoreWindowClass";
	
	RegisterClass(&wc);

	m_Hwnd = CreateWindowEx(
		0,
		L"ShardCoreWindowClass",
		L"ShardCore Engine",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1280, 720,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (!m_Hwnd)
		return false;

	ShowWindow(m_Hwnd, SW_SHOW);
	return true;
}

bool Window::ProcessEvents()
{
	MSG msg = {};

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		
		if (msg.message == WM_QUIT)
			return false;
	}

	return true;
}