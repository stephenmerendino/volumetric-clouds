#pragma once

#include "Engine/Math/AABB2.hpp"

class Window;

void DisplaySystemStartup();
void DisplaySystemUpdate();
void DisplaySystemShutdown();

unsigned int GetDisplayCount();
unsigned int GetPrimaryDisplayIndex();

AABB2 GetDisplayBounds(const unsigned int displayIndex);
AABB2 GetMonitorBoundsForWindow(const Window* window);