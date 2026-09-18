#pragma once
#include <memory>
#include <windows.h>
#include "InputCodes.h"

namespace DirectX { class Keyboard; class Mouse; }

class Input
{
public:
	static void Initialize(HWND hWnd);
	static bool IsKeyDown(KeyCode key);
	static bool IsMouseButtonDown(MouseButton button);
	static const int GetMouseDeltaX();
	static const int GetMouseDeltaY();
	static const int GetMouseWheelValue();

	static std::unique_ptr<DirectX::Keyboard> keyboard;
	static std::unique_ptr<DirectX::Mouse> mouse;
};