#include "Logger.h"
#include <iostream>

void Logger::Log(std::string msg)
{
	m_Messages.push_back(msg);
	std::cout << msg << std::endl;
}

void Logger::Clear()
{
	if (!m_Messages.empty())
	{
		m_Messages.clear();
	}
}

std::vector<std::string> Logger::m_Messages;