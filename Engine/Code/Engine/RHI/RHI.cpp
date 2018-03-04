#include "Engine/RHI/RHI.hpp"
#include "Engine/Core/Display.hpp"

void RHISystemStartup()
{
	DisplaySystemStartup();
}

void RHISystemShutdown()
{
	DisplaySystemShutdown();
}
