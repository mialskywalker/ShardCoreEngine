#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"

void Input::Initialize(HWND hWnd)
{
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();

	mouse->SetWindow(hWnd);
}

bool Input::IsKeyDown(KeyCode key) // When a keyboard key is pressed
{
	const DirectX::Keyboard::State& keyState = keyboard.get()->GetState();
	
	switch (key)
	{
	case KeyCode::W:
		return keyState.W;
	case KeyCode::A:
		return keyState.A;
	case KeyCode::S:
		return keyState.S;
	case KeyCode::D:
		return keyState.D;
	case KeyCode::Q:
		return keyState.Q;
	case KeyCode::E:
		return keyState.E;
	case KeyCode::F:
		return keyState.F;
	case KeyCode::LeftShift:
		return keyState.LeftShift;
	default:
		return false;
	}
}

bool Input::IsMouseButtonDown(MouseButton button) // When a mouse button is pressed
{
	const DirectX::Mouse::State& mouseState = mouse.get()->GetState();

	switch (button)
	{
	case MouseButton::Left:
		return mouseState.leftButton;
	case MouseButton::Right:
		return mouseState.rightButton;
	case MouseButton::Middle:
		return mouseState.middleButton;
	default:
		return false;
	}
}

const int Input::GetMouseDeltaX() // Get mouse position x
{
	const DirectX::Mouse::State& mouseState = mouse.get()->GetState();
	return mouseState.x;
}

const int Input::GetMouseDeltaY() // Get mouse position y
{
	const DirectX::Mouse::State& mouseState = mouse.get()->GetState();
	return mouseState.y;
}

const int Input::GetMouseWheelValue() // Get mouse scroll wheel value
{
	const DirectX::Mouse::State& mouseState = mouse.get()->GetState();
	return mouseState.scrollWheelValue;
}

std::unique_ptr<DirectX::Keyboard> Input::keyboard;
std::unique_ptr<DirectX::Mouse> Input::mouse;