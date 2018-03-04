#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

enum WindowMode
{
   WINDOW_MODE_WINDOWED, 
   WINDOW_MODE_BORDERLESS,
   WINDOW_MODE_FULLSCREEN_WINDOWED,
   /*WINDOW_MODE_FULLSCREEN_DEDICATED,*/
   NUM_WINDOW_MODES
};

typedef bool (*WindowsMessageCallback)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Window
{
public:
	HWND					m_windowHandle;
	WindowsMessageCallback	m_customWindowsMessageCallback;

public:
	Window(int width, int height, WindowMode windowMode);
	~Window();

	bool Open();
	bool IsOpen();
	void Close();

	int ProcessMessages();

	void SetCustomWindowsMessageCallback(WindowsMessageCallback newCallback);

public:
	inline HWND			GetHandle()				const { return m_windowHandle; }

	inline const char*	GetTitle()				const { return m_title; }
	inline WindowMode	GetWindowMode()			const { return m_windowMode; }

	IntVector2			GetClientResolution()	const;
	int					GetWidth()				const;
	int					GetHeight()				const;
	float				GetAspectRatio()		const;

	IntVector2			GetPosition()			const;

	Vector2				ConvertScreenCoordsToNormalizedClientCoords(IntVector2 screen_coords) const;

public:
	void SetTitle(const char* windowTitle);
	void SetClientSize(int width, int height);
	void SetWindowMode(WindowMode mode);
	void Center();

public:
	std::string GetClipboard();
	void SetClipboard(const std::string& text);

private:
	const char*				m_title;
	WindowMode				m_windowMode;
	IntVector2				m_resolution;
	unsigned int			m_clipboardFormat;
};

bool RegisterGameWindowClass();
void UnregisterGameWindowClass();