#include "Application.h"
#include "Logger.h"
#include "Camera.h"
#include "Input.h"

Application::Application() {}

Application::~Application() {}

void Application::Run()
{
	m_Window.Init();
	Input::Initialize(m_Window.GetHandle());
	m_Camera.Init();
	m_Renderer.Init(m_Window.GetHandle(), m_Window.GetWidth(), m_Window.GetHeight());

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
	m_Camera.Update(m_Timer.GetDeltaTime());
}

void Application::Render()
{
	m_Renderer.Render(m_Camera);
}