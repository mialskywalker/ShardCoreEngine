#include "Application.h"
#include "Logger.h"
#include "Camera.h"
#include "Input.h"
#include "D3D12Module.h"
#include "SceneModule.h"
#include "Scene.h"
#include "GameObject.h"

Application::Application() {}

Application::~Application() {}

bool Application::Init()
{
	bool ok = m_Window.Init();
	ok = ok && Input::Initialize(m_Window.GetHandle());
	ok = ok && m_Camera.Init(m_Window.GetWidth(), m_Window.GetHeight());
	ok = ok && m_Renderer.Init(m_Window.GetHandle(), m_Window.GetWidth(), m_Window.GetHeight());

	m_SceneModule = std::make_unique<SceneModule>();
	Scene* scene = m_SceneModule->CreateScene("Default Scene");

	if (!scene || !scene->Init())
	{
		return false;
	}

	return ok;
}

void Application::Run()
{
	while (m_Running)
	{
		m_Running = m_Window.ProcessEvents();
		m_Timer.Update();

		if (m_Window.ConsumeResize())
		{
			m_Renderer.GetD3D12()->Resize();
		}

		OnBeginFrame();

		Update();
		Render();
	}

	Logger::Log("Application exit!", LogType::Engine);
	Logger::Clear();
}

void Application::Update()
{
	if (CanUpdateCamera())
	{
		m_Camera.Update(m_Timer.GetDeltaTime());
	}

	if (m_SceneModule)
	{
		m_SceneModule->Update(m_Timer.GetDeltaTime());
	}
}

void Application::Render()
{
	m_Renderer.PreRender();	
	m_Renderer.Render(m_Camera, GetActiveScene());

	OnRenderOverlay();

	m_Renderer.PostRender();
}

Scene* Application::GetActiveScene() const
{
	if (!m_SceneModule)
	{
		return nullptr;
	}

	return m_SceneModule->GetActiveScene();
}