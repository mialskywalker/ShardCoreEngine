#pragma once
#include "Time.h"

class Application
{
public:	
	Application();
	~Application();

public:
	void Run();

private:
	bool m_Running = true;
	Time m_Time;
};