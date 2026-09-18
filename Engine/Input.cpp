#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"

void Input::Initialize(HWND hWnd)
{
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();

	mouse->SetWindow(hWnd);
}

std::unique_ptr<DirectX::Keyboard> Input::keyboard;
std::unique_ptr<DirectX::Mouse> Input::mouse;