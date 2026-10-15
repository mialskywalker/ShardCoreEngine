#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Timer.h"
#include "Camera.h"

class Application
{
public:
	Application();
	~Application();

	void Run();
	
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