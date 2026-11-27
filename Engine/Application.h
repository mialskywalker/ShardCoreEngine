#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Timer.h"
#include "Camera.h"
#include "FileSystem.h"
#include <memory>

class SceneModule;
class Scene;
class ResourceManager;
class AssetRegistry;

class Application
{
public:
	Application();
	virtual ~Application();

	bool Init();
	void Run();

protected:
	virtual void OnBeginFrame() {}
	virtual void OnRenderOverlay() {}

	virtual void OnUpdate(float deltaTime) {}
	virtual Camera& GetRenderCamera() { return m_Camera; }

public:
	Window& GetWindow() { return m_Window; }
	Renderer& GetRenderer() { return m_Renderer; }
	Scene* GetActiveScene() const;
	FileSystem& GetFileSystem() { return m_FileSystem; }
	SceneModule& GetSceneModule() const { return *m_SceneModule; }
	ResourceManager& GetResourceManager() const { return *m_ResourceManager; }
	AssetRegistry& GetAssetRegistry() const { return *m_AssetRegistry; }
	
private:
	void Update();
	void Render();

private:
	bool			m_Running = true;
	Window			m_Window;
	Timer			m_Timer;
	Renderer		m_Renderer;
	Camera			m_Camera;
	FileSystem		m_FileSystem;

	std::unique_ptr<SceneModule> m_SceneModule;
	std::unique_ptr<ResourceManager> m_ResourceManager;
	std::unique_ptr<AssetRegistry> m_AssetRegistry;

	float fov = 0.78f;
};