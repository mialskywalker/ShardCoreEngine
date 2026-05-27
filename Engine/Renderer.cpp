#include "Renderer.h"
#include "Logger.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	m_Hwnd = hwnd;
	m_Width = width;
	m_Height = height;

	Logger::Log("Rendered initialized successfully!");

	return true;
}

void Renderer::Render()
{
	
}