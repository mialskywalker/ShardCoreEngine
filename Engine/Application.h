#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Timer.h"
#include "Camera.h"

class Application
{
public:
	Application();
	virtual ~Application();

	bool Init();
	void Run();

protected:
	virtual void OnRenderOverlay() {}

public:
	Window& GetWindow() { return m_Window; }
	Renderer& GetRenderer() { return m_Renderer; }
	
private:
	void Update();
	void Render();

private:
	bool			m_Running = true;
	Window			m_Window;
	Timer			m_Timer;
	Renderer		m_Renderer;
	Camera			m_Camera;

	float fov = 0.78f;
};