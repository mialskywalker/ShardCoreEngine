#pragma once

#include "imgui.h"

class WindowLogger
{
public:
	WindowLogger();
	~WindowLogger();

	void Draw();

private:
	ImGuiTextFilter     m_Filter;

	bool                m_AutoScroll = true;
	bool                m_ScrollToBottom = false;

	bool				m_ShowInfo = true;
	bool				m_ShowWarning = true;
	bool				m_ShowError = true;
};