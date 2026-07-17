#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Time.h"

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
	bool m_Running = true;
	Window m_Window;
	Time m_Time;
	Renderer m_Renderer;
};