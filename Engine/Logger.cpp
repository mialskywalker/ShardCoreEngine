#include "Logger.h"

void Logger::Log(const std::string& msg, LogType type)
{
	Message m;
	m.logType = type;
	m.msg = msg;
	m.type = Logger::LoggerType::LOG_INFO;
	m_Messages.push_back(m);
}

void Logger::Warn(const std::string& msg, LogType type)
{
	Message m;
	m.logType = type;
	m.msg = msg;
	m.type = Logger::LoggerType::LOG_WARN;
	m_Messages.push_back(m);
}

void Logger::Error(const std::string& msg, LogType type)
{
	Message m;
	m.logType = type;
	m.msg = msg;
	m.type = Logger::LoggerType::LOG_ERROR;
	m_Messages.push_back(m);
}

void Logger::Clear()
{
	if (!m_Messages.empty())
	{
		m_Messages.clear();
	}
}

std::vector<Logger::Message> Logger::m_Messages;