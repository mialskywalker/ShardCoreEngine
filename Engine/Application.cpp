#include "Application.h"
#include <iostream>

Application::Application() {}

Application::~Application() {}

void Application::Run()
{
	int count = 0;
	while (m_Running)
	{
		// Update
		std::cout << "Engine running\n";
		if (count == 5)
			m_Running = false;
		count++;
		m_Time.Update();
		std::cout << m_Time.GetDeltaTime() << "\n";
		// Render
	}
}