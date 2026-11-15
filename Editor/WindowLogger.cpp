#include "WindowLogger.h"
#include "Logger.h"

WindowLogger::WindowLogger() {}

WindowLogger::~WindowLogger() {}

void WindowLogger::Draw()
{
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Log");

	// Filter
	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
	ImGui::SetNextItemWidth(-FLT_MIN);
	m_Filter.DrawWithHint("##Filter", "Filter (incl -excl)");
	ImGui::Separator();

	// Text Field
	ImGuiStyle& style = ImGui::GetStyle();
	const float footer_height_to_reserve = style.SeparatorSize + style.ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
	{
		for (auto i : Logger::GetMessages())
		{
			ImVec4 color;
			bool has_color = false;

			std::string res;
			if (i.logType == LogType::Engine)
			{
				res = "[Engine] " + i.msg;			;
			}
			else if (i.logType == LogType::Gameplay)
			{
				res = "[Script] " + i.msg;
			}
			else
			{
				res = "[???] " + i.msg;
			}

			const char* item = res.c_str();

			if (i.type == Logger::LOG_ERROR) 
			{ 
				color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
				has_color = true; 
			}
			else if (i.type == Logger::LOG_WARN) 
			{ 
				color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
				has_color = true;
			}

			if (has_color)
				ImGui::PushStyleColor(ImGuiCol_Text, color);

			if (!m_Filter.PassFilter(item))
				continue;

			if (i.type == Logger::LOG_INFO && !m_ShowInfo || i.type == Logger::LOG_WARN && !m_ShowWarning || i.type == Logger::LOG_ERROR && !m_ShowError)
			{
				continue;
			}

			ImGui::TextUnformatted(item);
			if (has_color)
				ImGui::PopStyleColor();
		}

		if (m_ScrollToBottom || (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			ImGui::SetScrollHereY(1.0f);
		m_ScrollToBottom = false;

	}
	ImGui::EndChild();
	ImGui::Separator();

	// Clear Button
	if (ImGui::Button("Clear"))
	{
		Logger::Clear();
	}

	ImGui::SameLine();

	// Options menu
	if (ImGui::BeginPopup("Options"))
	{
		ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
		ImGui::EndPopup();
	}

	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Options"))
		ImGui::OpenPopup("Options");

	ImGui::SameLine();
	ImGui::Checkbox("Logs", &m_ShowInfo);
	ImGui::SameLine();
	ImGui::Checkbox("Warnings", &m_ShowWarning);
	ImGui::SameLine();
	ImGui::Checkbox("Errors", &m_ShowError);

	ImGui::End();
}