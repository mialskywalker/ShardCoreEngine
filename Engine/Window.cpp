#include "Window.h"
#include "resource.h"
#include "Logger.h"
#include "Keyboard.h"
#include "Mouse.h"

Window::Window() {}

Window::~Window() {}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE)
	{
		auto* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		auto* window = static_cast<Window*>(createStruct->lpCreateParams);

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
	}

	auto* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (window && window->ForwardMessage(hwnd, msg, wParam, lParam))
	{
		return 0;
	}

	switch (msg)
	{
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
		DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
		break;
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
		this
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

void Window::SetMessageCallback(const std::function<bool(HWND, UINT, WPARAM, LPARAM)>& callback)
{
	m_MessageCallback = callback;
}

bool Window::ForwardMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_MessageCallback)
	{
		return m_MessageCallback(hwnd, msg, wParam, lParam);
	}

	return false;
}
