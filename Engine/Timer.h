#pragma once
#include <chrono>

class Timer
{
public:
	Timer();
	~Timer();

	void Update();

private:
	float												m_DeltaTime = 0.0f;
	std::chrono::high_resolution_clock::time_point		m_LastTime;

public:
	float GetDeltaTime() const { return m_DeltaTime; }
};