#pragma once
#include <chrono>

class Time
{
public:
	Time();
	~Time();

	void Update();

private:
	float m_DeltaTime = 0.0f;
	std::chrono::high_resolution_clock::time_point m_LastTime;

public:
	float GetDeltaTime() const { return m_DeltaTime; }
};