#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <windows.h>
#include <xinput.h>
#pragma comment(lib, "xinput9_1_0")

JoystickState::JoystickState()
	:raw(0.f, 0.f),
	 corrected(0.f, 0.f)
{
}

JoystickState::JoystickState(const JoystickState& copy)
	:raw(copy.raw),
	 corrected(copy.corrected)
{
}

const float JoystickState::JOYSTICK_LOWER_MAGNITUDE_BOUNDS = (float)XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE / (float)SHRT_MAX;
const float JoystickState::JOYSTICK_UPPER_MAGNITUDE_BOUNDS = 0.95f;

XboxController::XboxController(int controllerID)
	:m_controllerID(controllerID),
	 m_isConnected(false),
	 m_leftTriggerMagnitude(0.f),
	 m_rightTriggerMagnitude(0.f),
	 m_currentLeftMotorSpeed(0.f),
	 m_currentRightMotorSpeed(0.f),
	 m_currentVibrateDurationSeconds(0.f)
{
	for(int buttonStateIndex = 0; buttonStateIndex < NUM_XBOX_BUTTONS; ++buttonStateIndex){
		m_buttonStates[buttonStateIndex].isDown			= false;
		m_buttonStates[buttonStateIndex].wasJustChanged = false;
	}
}

XboxController::~XboxController(){
}

bool XboxController::IsConnected() const{
	return m_isConnected;
}

const JoystickState& XboxController::GetLeftJoystickState() const{
	return m_leftJoystickState;
}

const JoystickState& XboxController::GetRightJoystickState() const{
	return m_rightJoystickState;
}

float XboxController::GetLeftTriggerMagnitude() const{
	return m_leftTriggerMagnitude;
}

float XboxController::GetRightTriggerMagnitude() const{
	return m_rightTriggerMagnitude;
}

bool XboxController::IsButtonDown(XboxButton buttonCode) const{
	return m_buttonStates[buttonCode].isDown;
}

bool XboxController::WasJustPressed(XboxButton buttonCode) const{
	return m_buttonStates[buttonCode].isDown && m_buttonStates[buttonCode].wasJustChanged;
}

void XboxController::Update(float deltaSeconds){
	XINPUT_STATE xboxControllerState;
	memset(&xboxControllerState, 0, sizeof(xboxControllerState));
	DWORD errorStatus = XInputGetState(m_controllerID, &xboxControllerState);

	if(errorStatus == ERROR_DEVICE_NOT_CONNECTED){
		m_isConnected = false;
		return;
	}

	if(errorStatus != ERROR_SUCCESS){
		m_isConnected = false;
		DebuggerPrintf("Xbox controller #%d: Error Updating. Status Code: %u (0x%08x).\n", m_controllerID, errorStatus, errorStatus);
		return;
	}

	m_isConnected = true;

	{ // Joysticks
		short leftStickX = xboxControllerState.Gamepad.sThumbLX;
		short leftStickY = xboxControllerState.Gamepad.sThumbLY;
		short rightStickX = xboxControllerState.Gamepad.sThumbRX;
		short rightStickY = xboxControllerState.Gamepad.sThumbRY;

		CalcJoystick(m_leftJoystickState, leftStickX, leftStickY);
		CalcJoystick(m_rightJoystickState, rightStickX, rightStickY);
	}

	{ // Triggers
		unsigned char leftTrigger = xboxControllerState.Gamepad.bLeftTrigger;
		unsigned char rightTrigger = xboxControllerState.Gamepad.bRightTrigger;

		m_leftTriggerMagnitude = MapFloatZeroToOne(leftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255.f);
		m_rightTriggerMagnitude = MapFloatZeroToOne(rightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255.f);
	}

	{ // Gamepad buttons states
		unsigned short gamepadButtons = xboxControllerState.Gamepad.wButtons;

		UpdateButtonState(XBOX_BUTTON_DPAD_UP, gamepadButtons, XINPUT_GAMEPAD_DPAD_UP);
		UpdateButtonState(XBOX_BUTTON_DPAD_DOWN, gamepadButtons, XINPUT_GAMEPAD_DPAD_DOWN);
		UpdateButtonState(XBOX_BUTTON_DPAD_LEFT, gamepadButtons, XINPUT_GAMEPAD_DPAD_LEFT);
		UpdateButtonState(XBOX_BUTTON_DPAD_RIGHT, gamepadButtons, XINPUT_GAMEPAD_DPAD_RIGHT);

		UpdateButtonState(XBOX_BUTTON_START, gamepadButtons, XINPUT_GAMEPAD_START);
		UpdateButtonState(XBOX_BUTTON_BACK, gamepadButtons, XINPUT_GAMEPAD_BACK);

		UpdateButtonState(XBOX_BUTTON_LEFT_THUMB, gamepadButtons, XINPUT_GAMEPAD_LEFT_THUMB);
		UpdateButtonState(XBOX_BUTTON_RIGHT_THUMB, gamepadButtons, XINPUT_GAMEPAD_RIGHT_THUMB);

		UpdateButtonState(XBOX_BUTTON_LEFT_SHOULDER, gamepadButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
		UpdateButtonState(XBOX_BUTTON_RIGHT_SHOULDER, gamepadButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);

		UpdateButtonState(XBOX_BUTTON_A, gamepadButtons, XINPUT_GAMEPAD_A);
		UpdateButtonState(XBOX_BUTTON_B, gamepadButtons, XINPUT_GAMEPAD_B);
		UpdateButtonState(XBOX_BUTTON_X, gamepadButtons, XINPUT_GAMEPAD_X);
		UpdateButtonState(XBOX_BUTTON_Y, gamepadButtons, XINPUT_GAMEPAD_Y);
	}

	UpdateVibrate(deltaSeconds);
}

void XboxController::CalcJoystick(JoystickState& joystickState, float joystickX, float joystickY){
	joystickState.raw.x = MapFloatToRange(joystickX, SHRT_MIN, SHRT_MAX, -1.f, 1.f);
	joystickState.raw.y = MapFloatToRange(joystickY, SHRT_MIN, SHRT_MAX, -1.f, 1.f);
	float rawRadius = joystickState.raw.CalcLength();

	float orientationDegrees = joystickState.raw.CalcHeadingDegrees();
	float correctedRadius = MapFloatToRange(rawRadius, JoystickState::JOYSTICK_LOWER_MAGNITUDE_BOUNDS, JoystickState::JOYSTICK_UPPER_MAGNITUDE_BOUNDS, 0.f, 1.f);
	correctedRadius = Clamp(correctedRadius, 0.f, 1.f);

	joystickState.corrected.SetLengthAndHeadingDegrees(correctedRadius, orientationDegrees);
}

void XboxController::UpdateButtonState(XboxButton buttonCode, unsigned short gamepadState, unsigned short buttonBitmask){
	ButtonState& buttonState = m_buttonStates[buttonCode];
	bool wasDown = buttonState.isDown;

	if(gamepadState & buttonBitmask){
		buttonState.isDown = true;
	} else{
		buttonState.isDown = false;
	}

	buttonState.wasJustChanged = (wasDown != buttonState.isDown);
}

void XboxController::ExpireButtons(){
	for(int buttonStateIndex = 0; buttonStateIndex < NUM_XBOX_BUTTONS; ++buttonStateIndex){
		m_buttonStates[buttonStateIndex].wasJustChanged = false;
	}
}

void XboxController::StartVibrate(float leftMotorSpeedNormalized, float rightMotorSpeedNormalized, float durationSeconds){
	m_currentLeftMotorSpeed = leftMotorSpeedNormalized;
	m_currentRightMotorSpeed = rightMotorSpeedNormalized;
	m_currentVibrateDurationSeconds = Clamp(durationSeconds, 0.f, MAX_VIBRATE_DURATION);
}

void XboxController::UpdateVibrate(float deltaSeconds){
	if(!IsConnected())
		return;

	if(m_currentVibrateDurationSeconds <= 0.f){
		m_currentLeftMotorSpeed = 0.f;
		m_currentRightMotorSpeed = 0.f;
		m_currentVibrateDurationSeconds = 0.f;
	} else{
		m_currentVibrateDurationSeconds -= deltaSeconds;
	}

	XINPUT_VIBRATION vibration;
	memset(&vibration, 0, sizeof(vibration));

	vibration.wLeftMotorSpeed = static_cast<unsigned short>(MapFloatToRange(m_currentLeftMotorSpeed, 0.f, 1.f, 0.f, 65535.f));
	vibration.wRightMotorSpeed = static_cast<unsigned short>(MapFloatToRange(m_currentRightMotorSpeed, 0.f, 1.f, 0.f, 65535.f));

	XInputSetState(m_controllerID, &vibration);
}

const float XboxController::MAX_VIBRATE_DURATION = 2.f;