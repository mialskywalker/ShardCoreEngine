#include "Application.h"
#include <iostream>

Application::Application() {}

Application::~Application() {}

void Application::Run()
{
	m_Window.Init();

	while (m_Running)
	{
		m_Running = m_Window.ProcessEvents();

		Update();
		Render();
	}
}

void Application::Update()
{
	m_Time.Update();
	std::cout << m_Time.GetDeltaTime() << "\n";
}

void Application::Render()
{
	std::cout << "Rendering...\n";
}