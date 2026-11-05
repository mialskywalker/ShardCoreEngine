#pragma once

#include <windows.h>
#include <cstdint>
#include <functional>

class Window
{
public:
	Window();
	~Window();

	bool Init();
	bool ProcessEvents();

private:
	HWND				m_Hwnd = nullptr;
	uint32_t			m_Width = 1280;
	uint32_t			m_Height = 720;
	const wchar_t*		m_Title = L"ShardCore Engine";
	bool				m_Resized = false;

	std::function<bool(HWND, UINT, WPARAM, LPARAM)> m_MessageCallback;

public:
	HWND GetHandle() const { return m_Hwnd; }
	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }

	void SetSize(uint32_t width, uint32_t height);
	bool ConsumeResize();

	void SetMessageCallback(const std::function<bool(HWND, UINT, WPARAM, LPARAM)>& callback);
	bool ForwardMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};