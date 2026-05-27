#pragma once
#include <vector>
#include <string>

class Logger
{
public:
	static void Log(std::string msg);
	static void Clear();

private:
	static std::vector<std::string> m_Messages;
};