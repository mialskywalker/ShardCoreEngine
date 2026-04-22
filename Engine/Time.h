#pragma once
#include <chrono>

class Time
{
public:
	Time();
	~Time();

public:
	void Update();
	float GetDeltaTime() const;

private:
	float m_DeltaTime = 0.0f;
	std::chrono::high_resolution_clock::time_point m_LastTime;
};