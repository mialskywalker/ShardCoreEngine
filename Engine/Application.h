#pragma once
#include "Time.h"
#include "Window.h"
#include "Renderer.h"

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
	Time m_Time;
	Window m_Window;
	Renderer m_Renderer;
};