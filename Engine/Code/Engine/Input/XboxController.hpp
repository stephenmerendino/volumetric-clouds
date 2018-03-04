#pragma once

#include "Engine/Math/Vector2.hpp"

enum XboxButton{
	XBOX_BUTTON_DPAD_UP,
	XBOX_BUTTON_DPAD_DOWN,
	XBOX_BUTTON_DPAD_LEFT,
	XBOX_BUTTON_DPAD_RIGHT,
	XBOX_BUTTON_START,
	XBOX_BUTTON_BACK,
	XBOX_BUTTON_LEFT_THUMB,
	XBOX_BUTTON_RIGHT_THUMB,
	XBOX_BUTTON_LEFT_SHOULDER,
	XBOX_BUTTON_RIGHT_SHOULDER,
	XBOX_BUTTON_A,
	XBOX_BUTTON_B,
	XBOX_BUTTON_X,
	XBOX_BUTTON_Y,
	NUM_XBOX_BUTTONS = 14
};

struct ButtonState{
	bool isDown;
	bool wasJustChanged;
};

struct JoystickState{
	Vector2 raw;
	Vector2 corrected;

	JoystickState();
	JoystickState(const JoystickState& copy);

	static const float JOYSTICK_LOWER_MAGNITUDE_BOUNDS;
	static const float JOYSTICK_UPPER_MAGNITUDE_BOUNDS;
};

class XboxController{
	friend class InputSystem;

public:
	explicit XboxController(int controllerID);
	~XboxController();

	bool					IsConnected()								const;

	const JoystickState&	GetLeftJoystickState()						const;
	const JoystickState&	GetRightJoystickState()						const;

	float   				GetLeftTriggerMagnitude()					const;
	float   				GetRightTriggerMagnitude()					const;

	bool					IsButtonDown(XboxButton buttonCode)			const;
	bool					WasJustPressed(XboxButton buttonCode)		const;


private:
	int				m_controllerID;
	bool			m_isConnected;

	ButtonState		m_buttonStates[NUM_XBOX_BUTTONS];

	JoystickState	m_leftJoystickState;
	JoystickState	m_rightJoystickState;

	float			m_leftTriggerMagnitude;
	float			m_rightTriggerMagnitude;

	float			m_currentLeftMotorSpeed;
	float			m_currentRightMotorSpeed;
	float			m_currentVibrateDurationSeconds;

	void			Update(float deltaSeconds);
	void			CalcJoystick(JoystickState& joystickState, float joystickX, float joystickY);
	void			UpdateButtonState(XboxButton buttonCode, unsigned short gamepadState, unsigned short buttonBitmask);
	void			ExpireButtons();

	void			StartVibrate(float leftMotorSpeedNormalized, float rightMotorSpeedNormalized, float durationSeconds);
	void			UpdateVibrate(float deltaSeconds);

	static const float MAX_VIBRATE_DURATION;
};