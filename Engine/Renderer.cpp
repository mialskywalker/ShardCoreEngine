#include "Renderer.h"
#include <iostream>


Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	m_Hwnd = hwnd;
	m_Width = width;
	m_Height = height;

	return true;
}

void Renderer::Render()
{
	std::cout << "Rendering" << std::endl;
}