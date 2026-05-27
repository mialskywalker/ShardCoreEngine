#include "Application.h"
#include <iostream>

Application::Application() {}

Application::~Application() {}

void Application::Run()
{
	m_Window.Init();
	m_Renderer.Init(m_Window.GetHandle(), m_Window.GetWidth(), m_Window.GetHeight());

	while (m_Running)
	{
		m_Running = m_Window.ProcessEvents();

		Update();
		Render();
	}
}

void Application::Update()
{
	std::cout << "Running..." << std::endl;
}

void Application::Render()
{
	m_Renderer.Render();
}