#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"

void Input::Initialize(HWND hWnd)
{
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();

	mouse->SetWindow(hWnd);
}

bool Input::IsKeyDown(KeyCode key)
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
	case KeyCode::Space:
		return keyState.Space;
	case KeyCode::Escape:
		return keyState.Escape;
	default:
		return false;
	}
}

bool Input::IsMouseButtonDown(MouseButton button)
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

const int Input::GetMouseDeltaX()
{
	const DirectX::Mouse::State& mouseState = mouse.get()->GetState();
	return mouseState.x;
}

const int Input::GetMouseDeltaY()
{
	const DirectX::Mouse::State& mouseState = mouse.get()->GetState();
	return mouseState.y;
}

const int Input::GetMouseWheelValue()
{
	const DirectX::Mouse::State& mouseState = mouse.get()->GetState();
	return mouseState.scrollWheelValue;
}

std::unique_ptr<DirectX::Keyboard> Input::keyboard;
std::unique_ptr<DirectX::Mouse> Input::mouse;