#pragma once
#include <windows.h>

class Window
{
public:
	Window();
	~Window();

public:
	bool Init();
	bool ProcessEvents();

private:
	HWND m_Hwnd = nullptr;
};