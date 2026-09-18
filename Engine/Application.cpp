#include "Application.h"
#include "Logger.h"
#include "Camera.h"

Application::Application() {}

Application::~Application() {}

void Application::Run()
{
	m_Window.Init();
	m_Renderer.Init(m_Window.GetHandle(), m_Window.GetWidth(), m_Window.GetHeight());
	m_Camera.Init();

	while (m_Running)
	{
		m_Running = m_Window.ProcessEvents();
		m_Timer.Update();

		Update();
		Render();
	}

	Logger::Log("Application exit!");
	Logger::Clear();
}

void Application::Update()
{
}

void Application::Render()
{
	m_Renderer.Render(m_Camera);
}