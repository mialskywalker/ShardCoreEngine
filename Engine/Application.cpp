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
	}
}

void Application::Update()
{
	std::cout << "Running..." << std::endl;
}