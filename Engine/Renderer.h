#pragma once
#include <windows.h>
#include <cstdint>

class Renderer
{

public:
	Renderer();
	~Renderer();

	bool Init(HWND hwnd, uint32_t width, uint32_t height);
	void Render();

private:
	HWND m_Hwnd = nullptr;
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
};