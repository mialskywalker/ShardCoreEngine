#pragma once
#include "Window.h"

class Application
{
public:
	Application();
	~Application();

	void Run();
	
private:
	void Update();

private:
	bool m_Running = true;
	Window m_Window;
};