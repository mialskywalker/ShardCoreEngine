#include "Time.h"

Time::Time()
{
	m_LastTime = std::chrono::high_resolution_clock::now();
}

Time::~Time() {}

void Time::Update()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	m_DeltaTime = std::chrono::duration<float>(currentTime - m_LastTime).count();
	m_LastTime = currentTime;
}

float Time::GetDeltaTime() const
{
	return m_DeltaTime;
}