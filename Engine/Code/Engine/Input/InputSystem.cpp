#include "Engine/Input/InputSystem.hpp"
#include "Engine/Engine.hpp"
#include "Engine/RHI/RHIOutput.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

InputSystem::InputSystem()
	:m_mouseWheelDelta(0),
	 m_mouseMoveDelta(0, 0)
{
	for(int keyIndex = 0; keyIndex < NUM_KEYBOARD_KEYS; ++keyIndex){
		m_keyStates[keyIndex].isDown = false;
		m_keyStates[keyIndex].wasJustChanged = false;
	}

	for(int xboxControllerID = 0; xboxControllerID < MAX_NUM_XBOX_CONTROLLERS; ++xboxControllerID){
		m_xboxControllers[xboxControllerID] = new XboxController(xboxControllerID);
	}
}

InputSystem::~InputSystem(){
	for(int xboxControllerID = 0; xboxControllerID < MAX_NUM_XBOX_CONTROLLERS; ++xboxControllerID){
		delete m_xboxControllers[xboxControllerID];
		m_xboxControllers[xboxControllerID] = nullptr;
	}
}

void InputSystem::BeginFrame(){
}

void InputSystem::Update(float deltaSeconds){
	for(int xboxControllerID = 0; xboxControllerID < MAX_NUM_XBOX_CONTROLLERS; ++xboxControllerID){
		m_xboxControllers[xboxControllerID]->Update(deltaSeconds);
	}
}

void InputSystem::EndFrame(){
	for(int keyIndex = 0; keyIndex < NUM_KEYBOARD_KEYS; ++keyIndex){
		m_keyStates[keyIndex].wasJustChanged = false;
	}

	for(int xboxControllerID = 0; xboxControllerID < MAX_NUM_XBOX_CONTROLLERS; ++xboxControllerID){
		m_xboxControllers[xboxControllerID]->ExpireButtons();
	}

	m_mouseWheelDelta = 0.f;
	m_mouseMoveDelta = IntVector2::ZERO;
}

void InputSystem::RegisterKeyDown(unsigned char keyIndex){
	KeyState& keyState = m_keyStates[keyIndex];
	bool wasDown = keyState.isDown;

	keyState.isDown = true;
	keyState.wasJustChanged = (wasDown != keyState.isDown);
}

void InputSystem::RegisterKeyUp(unsigned char keyIndex){
	KeyState& keyState = m_keyStates[keyIndex];
	bool wasDown = keyState.isDown;

	keyState.isDown = false;
	keyState.wasJustChanged = (wasDown != keyState.isDown);
}

void InputSystem::UpdateMouseMoveDelta(){
	IntVector2 mouseScreenPos = GetCursorScreenPos();
	IntVector2 screenCenterPos = GetScreenCenter();
	m_mouseMoveDelta = mouseScreenPos - screenCenterPos;
	SetCursorScreenPos(screenCenterPos);
}

bool InputSystem::IsKeyDown(unsigned char keyIndex) const{
	return m_keyStates[keyIndex].isDown;
}

bool InputSystem::WasKeyJustPressed(unsigned char keyIndex) const{
	return IsKeyDown(keyIndex) & m_keyStates[keyIndex].wasJustChanged;
}

const XboxController* InputSystem::GetXboxController(int controllerID) const{
	if(controllerID < 0 || controllerID >= MAX_NUM_XBOX_CONTROLLERS)
		return nullptr;

	return m_xboxControllers[controllerID];
}

void InputSystem::VibrateXboxController(int controllerID, float leftMotorSpeedNormalized, float rightMotorSpeedNormalized, float durationSeconds){
	if(controllerID < 0 || controllerID > 3)
		return;

	m_xboxControllers[controllerID]->StartVibrate(leftMotorSpeedNormalized, rightMotorSpeedNormalized, durationSeconds);
}

void InputSystem::SetCursorScreenPos(const IntVector2& newMouseCursorScreenPos){
	SetCursorPos(newMouseCursorScreenPos.x, newMouseCursorScreenPos.y);
}

IntVector2 InputSystem::GetCursorScreenPos() const{
	POINT rawCursorPos;
	GetCursorPos(&rawCursorPos);
	return IntVector2(rawCursorPos.x, rawCursorPos.y);
}

Vector2 InputSystem::GetCursorClientAreaNormalized() const
{
	return g_theRenderer->m_output->m_window->ConvertScreenCoordsToNormalizedClientCoords(GetCursorScreenPos());
}

IntVector2 InputSystem::GetScreenDimensions() const{
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);

	IntVector2 screenDims;
	screenDims.x = desktopRect.right - desktopRect.left;
	screenDims.y = desktopRect.bottom - desktopRect.top;
	return screenDims;
}

IntVector2 InputSystem::GetScreenCenter() const{
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);

	IntVector2 screenCenter;
	screenCenter.x = (desktopRect.right - desktopRect.left) / 2;
	screenCenter.y = (desktopRect.bottom - desktopRect.top) / 2;
	return screenCenter;
}

void InputSystem::ShowMouseCursor(bool isNowVisible){
	if(isNowVisible){
		int currentShowCount = ShowCursor(TRUE);
		while(currentShowCount < 0){
			currentShowCount = ShowCursor(TRUE);
		}
	} else{
		int currentShowCount = ShowCursor(FALSE);
		while(currentShowCount >= 0){
			currentShowCount = ShowCursor(FALSE);
		}
	}
}

IntVector2 InputSystem::GetMouseMoveDelta() const{
	return m_mouseMoveDelta;
}

void InputSystem::RegisterMouseWheelDelta(float newMouseWheelDelta){
	m_mouseWheelDelta = newMouseWheelDelta;
}

float InputSystem::GetMouseWheelDelta() const{
	return m_mouseWheelDelta;
}