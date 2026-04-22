#pragma once
#include "Time.h"
#include "Window.h"

class Application
{
public:	
	Application();
	~Application();

public:
	void Run();

private:
	void Update();
	void Render();

private:
	bool m_Running = true;
	Time m_Time;
	Window m_Window;
};