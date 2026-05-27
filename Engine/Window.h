#pragma once
#include <windows.h>
#include <cstdint>

class Window
{
public:
	Window();
	~Window();

	bool Init();
	bool ProcessEvents();

private:
	HWND m_Hwnd = nullptr;
	uint32_t m_Width = 1280;
	uint32_t m_Height = 720;
	const wchar_t* m_Title = L"ShardCore Engine";

public:
	HWND getHandle() const { return m_Hwnd; }
	uint32_t getWidth() const { return m_Width; }
	uint32_t getHeight() const { return m_Height; }
};