#include "Application.h"
#include "Logger.h"
#include "Camera.h"
#include "Input.h"

Application::Application() {}

Application::~Application() {}

bool Application::Init()
{
	bool ok = m_Window.Init();
	ok = ok && Input::Initialize(m_Window.GetHandle());
	ok = ok && m_Camera.Init(m_Window.GetWidth(), m_Window.GetHeight());
	ok = ok && m_Renderer.Init(m_Window.GetHandle(), m_Window.GetWidth(), m_Window.GetHeight());

	return ok;
}

void Application::Run()
{
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
	m_Renderer.PreRender();
	m_Renderer.Render(m_Camera);

	OnRenderOverlay();

	m_Renderer.PostRender();
}