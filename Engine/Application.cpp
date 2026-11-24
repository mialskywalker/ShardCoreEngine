#include "Application.h"
#include "Logger.h"
#include "Camera.h"
#include "Input.h"
#include "D3D12Module.h"
#include "SceneModule.h"
#include "Scene.h"
#include "SceneSerializer.h"

#include "GameObject.h"
#include "TransformComponent.h"

Application::Application() {}

Application::~Application() {}

bool Application::Init()
{
	bool ok = true;

	// Initialize FileSystem
	std::filesystem::path projectRoot =	std::filesystem::current_path() / "SandboxProject";

	if (!m_FileSystem.Init(projectRoot))
	{
		Logger::Error("FileSystem initialization failed", LogType::Engine);
		return false;
	}

	// Create project directories

	ok = ok && m_FileSystem.CreateDirectories(".");
	ok = ok && m_FileSystem.CreateDirectories("Assets");
	ok = ok && m_FileSystem.CreateDirectories("Scenes");
	ok = ok && m_FileSystem.CreateDirectories("Library");
	ok = ok && m_FileSystem.CreateDirectories("ProjectSettings");

	if (!ok)
	{
		Logger::Error("Failed to create project directories", LogType::Engine);
		return ok;
	}

	// Systems init

	ok = ok && m_Window.Init();
	ok = ok && Input::Initialize(m_Window.GetHandle());
	ok = ok && m_Renderer.Init(m_Window.GetHandle(), m_Window.GetWidth(), m_Window.GetHeight());

	// Create Scene
	m_SceneModule = std::make_unique<SceneModule>();

	Scene* scene = m_SceneModule->CreateScene("NewScene");

	if (!scene || !scene->Init())
	{
		return false;
	}

	std::string root = m_FileSystem.GetProjectRoot().string();
	Logger::Log(root.c_str(), LogType::Engine);

	// test
	GameObject* go1 = scene->CreateGameObject("first");
	GameObject* go2 = scene->CreateGameObject("second");
	GameObject* go3 = scene->CreateGameObject("third");

	go2->SetActive(false);
	go3->SetParent(go1);
	go1->GetTransform()->SetPosition(Vector3(2.0f, 1.0f, 0.0f));
	go1->GetTransform()->SetRotation(Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
	go1->GetTransform()->SetScale(Vector3(1.0f, 2.0f, 2.0f));
	// test end

	// scene serializer test
	SceneSerializer ss(m_FileSystem);
	ss.SaveScene(*scene, "Scenes/" + scene->GetSceneName() + ".json");

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
	float deltaTime = m_Timer.GetDeltaTime();

	OnUpdate(deltaTime);

	if (m_SceneModule)
	{
		m_SceneModule->Update(deltaTime);
	}
}

void Application::Render()
{
	m_Renderer.PreRender();	
	m_Renderer.Render(GetRenderCamera(), GetActiveScene());

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