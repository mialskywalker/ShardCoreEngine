#pragma once
#include <vector>
#include <string>

enum class LogType
{
	Engine,
	Gameplay
};

class Logger
{
public:

	enum LoggerType
	{
		LOG_INFO,
		LOG_WARN,
		LOG_ERROR
	};

	struct Message
	{
		std::string msg;
		LogType logType;
		LoggerType type;
	};

	static void Log(const std::string& msg, LogType type);
	static void Warn(const std::string& msg, LogType type);
	static void Error(const std::string& msg, LogType type);
	static void Clear();
	static const std::vector<Message> GetMessages() { return m_Messages; }

private:
	static std::vector<Message> m_Messages;
};