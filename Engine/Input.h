#pragma once
#include <memory>
#include <windows.h>

namespace DirectX { class Keyboard; class Mouse; }

class Input
{
public:
	static void Initialize(HWND hWnd);

	static std::unique_ptr<DirectX::Keyboard> keyboard;
	static std::unique_ptr<DirectX::Mouse> mouse;
};