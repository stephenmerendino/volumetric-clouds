#include "Engine/Core/Window.hpp"
#include "Engine/Core/Display.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

LPCWSTR GAME_WINDOW_CLASS_NAME = L"GameWindowClass";

static HINSTANCE GetCurrentHINSTANCE()
{
	return GetModuleHandle(NULL);
}

Window* GetWindowFromHandle(HWND windowHandle)
{
	return (Window*)GetWindowLongPtr(windowHandle, GWLP_USERDATA);
}

static LRESULT CALLBACK GameWindowMessageHandlingProc(HWND		hwnd,
													  UINT		msg,
													  WPARAM	wParam,
													  LPARAM	lParam)
{
	// Custom Handling
	Window* window = GetWindowFromHandle(hwnd);
	if(window && window->m_customWindowsMessageCallback){
		bool allowDefaultProcessing = window->m_customWindowsMessageCallback(hwnd, msg, wParam, lParam);
		if(!allowDefaultProcessing){
			return 0;
		}
	}

	// Engine Handling
	switch(msg){
		// Store the Window pointer itself within the userdata for future use
		case WM_CREATE:
		{
			CREATESTRUCT* cp = (CREATESTRUCT*)lParam;
			window = (Window*)cp->lpCreateParams;
			window->m_windowHandle = hwnd;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
			if(window->m_customWindowsMessageCallback){
				bool allowDefaultProcessing = window->m_customWindowsMessageCallback(hwnd, msg, wParam, lParam);
				if(!allowDefaultProcessing){
					return 0;
				}
			}
		} break;

		case WM_DISPLAYCHANGE:
		{
			DisplaySystemUpdate();
		} break;
	}

	// Default Handling
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static RECT WindowGetRect(IntVector2 clientAreaSize, DWORD style)
{
	RECT windowRect;
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.right = clientAreaSize.x;
	windowRect.bottom = clientAreaSize.y;

	AdjustWindowRect(&windowRect, style, FALSE);

	return windowRect;
}

static DWORD GetWindowStyle(WindowMode mode)
{
	DWORD style = 0;
	if(mode == WINDOW_MODE_WINDOWED){
		style |= (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER);
	} else{
		style |= WS_POPUP;
	}

	style |= WS_VISIBLE;
	return style;
}

// Jank function that makes sure a windowed window (has a title bar) is positioned in
// the top left with the title bar visible. I figured this out through trial and error.
// The adjusted rect returned from WindowGetRect() doesn't work very well when the window
// has the title bar (WS_OVERLAPPED breaks the Rect function from Windows).
static void PositionWindowedWindowInTopLeft(Window* window){
	if(!window || window->GetWindowMode() != WINDOW_MODE_WINDOWED){
		return;
	}

	DWORD style = GetWindowStyle(WINDOW_MODE_WINDOWED);

	RECT windowRect = WindowGetRect(window->GetClientResolution(), style);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	AABB2 bounds = GetMonitorBoundsForWindow(window);

	SetWindowPos(window->GetHandle(), NULL, (int)bounds.mins.x + windowRect.left, (int)bounds.mins.y, windowWidth, windowHeight, NULL);
}

// Window Class Definitions

Window::Window(int width, int height, WindowMode windowMode)
	:m_title(nullptr)
	,m_resolution(width, height)
	,m_windowMode(windowMode)
	,m_customWindowsMessageCallback(nullptr)
{
}

Window::~Window()
{
	Close();
}

bool Window::Open()
{
	DWORD extendedStyle = 0;
	DWORD style = GetWindowStyle(m_windowMode);

	RECT windowRect = WindowGetRect(m_resolution, style);

	int windowX = windowRect.left;
	int windowY = windowRect.top;
	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	windowX = windowY = 100;

	HWND windowHandle = CreateWindowEx(
		extendedStyle,
		GAME_WINDOW_CLASS_NAME,
		NULL,
		style,
		windowX, windowY,
		windowWidth, windowHeight,
		NULL,
		NULL,
		GetCurrentHINSTANCE(),
		this);

	if(m_windowMode == WINDOW_MODE_WINDOWED){
		PositionWindowedWindowInTopLeft(this);
	}

	if(windowHandle == NULL){
		DebuggerPrintf("Failed to create window [Error: %u]\n", GetLastError());
		return false;
	}

	m_windowHandle = windowHandle;

	return true;
}

bool Window::IsOpen()
{
	return IsWindow(m_windowHandle) != 0;
}

void Window::Close()
{
	if(IsOpen()){
		DestroyWindow(m_windowHandle);
	}
}

int Window::ProcessMessages()
{
	int messagesProcessed = 0;
	
	if(m_windowHandle != NULL){
		MSG message;
		while(PeekMessage(&message, m_windowHandle, 0, 0, PM_REMOVE)){
			TranslateMessage(&message);
			DispatchMessage(&message);

			++messagesProcessed;
		}
	}

	return messagesProcessed;
}

void Window::SetCustomWindowsMessageCallback(WindowsMessageCallback newCallback)
{
	m_customWindowsMessageCallback = newCallback;
}

IntVector2 Window::GetPosition() const
{
	RECT windowRect;
	GetWindowRect(m_windowHandle, &windowRect);
	return IntVector2(windowRect.left, windowRect.top);
}

Vector2 Window::ConvertScreenCoordsToNormalizedClientCoords(IntVector2 screen_coords) const
{
	POINT screen_point;
	screen_point.x = screen_coords.x;
	screen_point.y = screen_coords.y;

	ScreenToClient(m_windowHandle, &screen_point);

	Vector2 normalized_client_coords;
	normalized_client_coords.x = (float)screen_point.x / GetWidth();
	normalized_client_coords.y = 1.0f - ((float)screen_point.y / GetHeight());
	return normalized_client_coords;
}

IntVector2 Window::GetClientResolution() const 
{ 
	RECT clientRect;
	GetClientRect(m_windowHandle, &clientRect);
	//GetWindowRect(m_windowHandle, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	return IntVector2(width, height);
}

int Window::GetWidth() const
{
	return GetClientResolution().x;
}

int Window::GetHeight() const 
{ 
	return GetClientResolution().y;
}

float Window::GetAspectRatio() const 
{ 
	IntVector2 clientResolution = GetClientResolution();
	return (float)clientResolution.x / (float)clientResolution.y;
}

void Window::SetTitle(const char* windowTitle)
{
	m_title = windowTitle;
	SetWindowTextA(m_windowHandle, windowTitle);
}

void Window::SetClientSize(int width, int height)
{
	if(m_windowMode == WINDOW_MODE_FULLSCREEN_WINDOWED /*|| m_windowMode == WINDOW_MODE_FULLSCREEN_DEDICATED*/){
		return;
	}

	m_resolution = IntVector2(width, height);
	IntVector2 position = GetPosition();

	SetWindowPos(m_windowHandle, NULL, position.x, position.y, width, height, SWP_FRAMECHANGED);
}

void Window::SetWindowMode(WindowMode mode)
{
	if(mode == m_windowMode){
		return;
	}

	// If changing to a windowed mode then do everything the current way
	IntVector2 currentPosition = GetPosition();
	RECT prevRect = WindowGetRect(m_resolution, GetWindowStyle(m_windowMode));

	// Set new style
	m_windowMode = mode;
	DWORD newStyle = GetWindowStyle(mode);
	SetWindowLongPtr(m_windowHandle, GWL_STYLE, newStyle);

	// Get new rect
	RECT newWindowRect = WindowGetRect(m_resolution, newStyle);

	// Get the bound for the windows monitor
	AABB2 monitorBounds = GetMonitorBoundsForWindow(this);

	int windowX;
	int windowY;
	int windowWidth;
	int windowHeight;

	if(mode == WINDOW_MODE_WINDOWED){
		windowX = currentPosition.x + newWindowRect.left;
		windowY = currentPosition.y + newWindowRect.top;
		windowWidth = newWindowRect.right - newWindowRect.left;
		windowHeight = newWindowRect.bottom - newWindowRect.top;
	} else if(mode == WINDOW_MODE_FULLSCREEN_WINDOWED){
		windowX = (int)monitorBounds.mins.x;
		windowY = (int)monitorBounds.mins.y;
		windowWidth = (int)monitorBounds.maxs.x - (int)monitorBounds.mins.x;
		windowHeight = (int)monitorBounds.maxs.y - (int)monitorBounds.mins.y;
	} else{
		// BORDERLESS
		// Use the adjusted rect of the windowed window to know how offset the client area was from the title bar
		windowX = currentPosition.x - prevRect.left;
		windowY = currentPosition.y - prevRect.top;
		windowWidth = newWindowRect.right - newWindowRect.left;
		windowHeight = newWindowRect.bottom - newWindowRect.top;
	}

	SetWindowPos(m_windowHandle, NULL, windowX, windowY, windowWidth, windowHeight, SWP_FRAMECHANGED);

	// Make sure the title bar is shown
	if(currentPosition.x == monitorBounds.mins.x && currentPosition.y == monitorBounds.mins.y && m_windowMode == WINDOW_MODE_WINDOWED){
		PositionWindowedWindowInTopLeft(this);
	}
}

void Window::Center()
{
	// if fullscreen, ignore
	if(m_windowMode == WINDOW_MODE_FULLSCREEN_WINDOWED){
		return;
	}

	// Get width and height of window
	RECT windowRect;
	GetWindowRect(m_windowHandle, &windowRect);

	// Get bounds of current monitor
	AABB2 monitorBounds = GetMonitorBoundsForWindow(this);

	int monitorWidth = (int)(monitorBounds.maxs.x - monitorBounds.mins.x);
	int monitorHeight = (int)(monitorBounds.maxs.y - monitorBounds.mins.y);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	int leftOverWidth = monitorWidth - windowWidth;
	int leftOverHeight = monitorHeight - windowHeight;

	int centeredWindowX = (int)monitorBounds.mins.x + (int)((float)leftOverWidth / 2.0f);
	int centeredWindowY = (int)monitorBounds.mins.y + (int)((float)leftOverHeight / 2.0f);

	SetWindowPos(m_windowHandle, NULL, centeredWindowX, centeredWindowY, windowWidth, windowHeight, 0);
}

std::string Window::GetClipboard()
{
	ASSERT_OR_DIE(OpenClipboard(m_windowHandle), "GetClipboard OpenClipboard failed");

	HANDLE data = GetClipboardData(CF_TEXT);
	ASSERT_OR_DIE(data, "GetClipboardData failed");

	char* text = (char*)GlobalLock(data);
	std::string outputText(text);
	GlobalUnlock(data);

	CloseClipboard();

	return outputText;
}

void Window::SetClipboard(const std::string& text)
{
	HGLOBAL winMemory = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
	ASSERT_OR_DIE(winMemory, "SetClipboard GlobalAlloc Failed");
	
	memcpy(GlobalLock(winMemory), text.data(), text.size() + 1);
	GlobalUnlock(winMemory);
	ASSERT_OR_DIE(OpenClipboard(m_windowHandle), "SetClipboard OpenClipboard failed");
	EmptyClipboard();
	SetClipboardData(CF_TEXT, winMemory);
	CloseClipboard();
}

bool RegisterGameWindowClass()
{
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = (CS_HREDRAW | CS_VREDRAW | CS_OWNDC);

	wc.lpfnWndProc = GameWindowMessageHandlingProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetCurrentHINSTANCE();

	wc.hIcon = NULL;
	wc.hIconSm = NULL;

	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)0;
	wc.lpszMenuName = NULL;

	wc.lpszClassName = GAME_WINDOW_CLASS_NAME;

	ATOM result = RegisterClassEx(&wc);
	if(result == NULL){
		DebuggerPrintf("Failed to register window class [Error: %u]\n", GetLastError());
		return false;
	}

	return true;
}

void UnregisterGameWindowClass()
{
	UnregisterClass(GAME_WINDOW_CLASS_NAME, GetCurrentHINSTANCE());
}
