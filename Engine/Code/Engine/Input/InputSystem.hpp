#pragma once

#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/IntVector2.hpp"

enum KeyCode{
	KEYCODE_LMB			= 0x00,
	KEYCODE_RMB			= 0x01,
	KEYCODE_TAB			= 0x09,
	KEYCODE_SHIFT		= 0x10,
	KEYCODE_ENTER		= 0x0D,
	KEYCODE_PAUSE		= 0x13,
	KEYCODE_ESCAPE		= 0x1B,
	KEYCODE_SPACE		= 0x20,
	KEYCODE_LEFT		= 0x25,
	KEYCODE_UP			= 0x26,
	KEYCODE_RIGHT		= 0x27,
	KEYCODE_DOWN		= 0x28,
	KEYCODE_F1			= 0x70,
	KEYCODE_F2			= 0x71,
	KEYCODE_F3			= 0x72,
	KEYCODE_F4			= 0x73,
	KEYCODE_F5			= 0x74,
	KEYCODE_F6			= 0x75,
	KEYCODE_F7			= 0x76,
	KEYCODE_F8			= 0x77,
	KEYCODE_F9			= 0x78,
	KEYCODE_F10			= 0x79,
	KEYCODE_F11			= 0x7A,
	KEYCODE_F12			= 0x7B,
	KEYCODE_TILDE		= 0xC0,
	NUM_KEYBOARD_KEYS	= 256
};

struct KeyState{
	bool isDown;
	bool wasJustChanged;
};

const int MAX_NUM_XBOX_CONTROLLERS = 4;

class InputSystem{
public:
	InputSystem();
	~InputSystem();

	void						BeginFrame();
	void						Update(float deltaSeconds);
	void						EndFrame();

	void						RegisterKeyDown(unsigned char keyIndex);
	void						RegisterKeyUp(unsigned char keyIndex);

	void						UpdateMouseMoveDelta();

	bool						IsKeyDown(unsigned char keyIndex)										const;
	bool						WasKeyJustPressed(unsigned char keyIndex)								const;

	const XboxController*		GetXboxController(int controllerID) const;
	void						VibrateXboxController(int controllerID, float leftMotorSpeedNormalized, 
																		float rightMotorSpeedNormalized, 
																		float durationSeconds);

	void						SetCursorScreenPos(const IntVector2& newMouseCursorScreenPos);
	IntVector2					GetCursorScreenPos()													const;

	Vector2						GetCursorClientAreaNormalized()											const;

	IntVector2					GetScreenDimensions()													const;
	IntVector2					GetScreenCenter()														const;

	void						ShowMouseCursor(bool isNowVisible);

	IntVector2					GetMouseMoveDelta()														const;

	void						RegisterMouseWheelDelta(float newMouseWheelDelta);
	float						GetMouseWheelDelta()													const;

private:
	KeyState					m_keyStates[NUM_KEYBOARD_KEYS];
	XboxController*				m_xboxControllers[MAX_NUM_XBOX_CONTROLLERS];
	float						m_mouseWheelDelta;
	IntVector2					m_mouseMoveDelta;
};