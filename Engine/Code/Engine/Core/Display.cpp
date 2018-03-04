#include "Engine/Core/Display.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/Window.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct DisplayInfo
{
	unsigned int m_id;
	AABB2 m_bounds;
	bool m_isPrimary;
	HMONITOR m_monitorHandle;
};

#include <vector>
static std::vector<DisplayInfo> s_displays;

static BOOL CALLBACK EnumToQueryDisplays(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM arg)
{
	UNUSED(hdc);

	unsigned int* displayIndex = (unsigned int*)arg;

	DisplayInfo newDisplayInfo;
	newDisplayInfo.m_id = *displayIndex;
	newDisplayInfo.m_monitorHandle = monitor;

	newDisplayInfo.m_bounds.mins = Vector2((float)rect->left, (float)rect->top);
	newDisplayInfo.m_bounds.maxs = Vector2((float)rect->right, (float)rect->bottom);

	MONITORINFO info;
	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(MONITORINFO);
	if(GetMonitorInfoA(monitor, &info)){
		if(info.dwFlags & MONITORINFOF_PRIMARY){
			newDisplayInfo.m_isPrimary = true;
		} else{
			newDisplayInfo.m_isPrimary = false;
		}
	}

	s_displays.push_back(newDisplayInfo);

	(*displayIndex)++;

	return true;
}

void DisplaySystemStartup()
{
	//SetProcessDPIAware();
	unsigned int startingDisplayIndex = 0;
	EnumDisplayMonitors(NULL, NULL, EnumToQueryDisplays, (LPARAM)&startingDisplayIndex);
}

void DisplaySystemUpdate()
{
	s_displays.clear();
	unsigned int startingDisplayIndex = 0;
	EnumDisplayMonitors(NULL, NULL, EnumToQueryDisplays, (LPARAM)&startingDisplayIndex);
}

void DisplaySystemShutdown()
{
	s_displays.clear();
}

unsigned int GetDisplayCount()
{
	return s_displays.size();
}

unsigned int GetPrimaryDisplayIndex()
{
	for(const DisplayInfo& displayInfo : s_displays){
		if(displayInfo.m_isPrimary){
			return displayInfo.m_id;
		}
	}

	return 0;
}

AABB2 GetDisplayBounds(const unsigned int displayIndex)
{
	return s_displays[displayIndex].m_bounds;
}

AABB2 GetMonitorBoundsForWindow(const Window* window){
	HMONITOR monitorHandle = MonitorFromWindow(window->GetHandle(), MONITOR_DEFAULTTONEAREST);
	for(const DisplayInfo& displayInfo : s_displays){
		if(displayInfo.m_monitorHandle == monitorHandle){
			return displayInfo.m_bounds;
		}
	}

	return AABB2();
}