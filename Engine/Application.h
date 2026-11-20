#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Timer.h"
#include "Camera.h"
#include <memory>

class SceneModule;
class Scene;

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
	
private:
	void Update();
	void Render();

private:
	bool			m_Running = true;
	Window			m_Window;
	Timer			m_Timer;
	Renderer		m_Renderer;
	Camera			m_Camera;

	std::unique_ptr<SceneModule> m_SceneModule;

	float fov = 0.78f;
};