#include "Timer.h"

Timer::Timer()
{
	m_LastTime = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {}

void Timer::Update()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	m_DeltaTime = std::chrono::duration<float>(currentTime - m_LastTime).count();
	m_LastTime = currentTime;
}