#include "Window.h"
#include "resource.h"
#include "Logger.h"

Window::Window() {}

Window::~Window() {}

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
	// Register Window Class

	HINSTANCE hInstance = GetModuleHandle(nullptr);

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"ShardCoreWindowClass";
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

	RegisterClass(&wc);

	// Create a Window
	
	m_Hwnd = CreateWindowEx(
		0,
		L"ShardCoreWindowClass",
		m_Title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		m_Width, m_Height,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);
	
	if (!m_Hwnd)
		return false;

	ShowWindow(m_Hwnd, SW_SHOW);

	Logger::Log("Window initialization successful!"); // Debug log

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