#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Math/Matrix4.hpp"

class RHIDeviceContext;
class RHIOutput;

class RHIInstance
{
public:
	static RHIInstance& GetInstance();

public:
	~RHIInstance() {};

	bool CreateOuput(RHIDeviceContext** out_rhiDeviceContext,
					 RHIOutput** out_rhiOutput,
					 const unsigned int width,
					 const unsigned int height,
					 RHIOutputMode initialOutputMode = RHI_OUTPUT_MODE_WINDOWED);

	Matrix4 CreateOrthoProjection(float nx, float fx, float ny, float fy, float nz = 0.0f, float fz = 1.0f);
	Matrix4 CreatePerspectiveProjection(float nz, float fz, float viewingAngleDegrees, float aspect);

private:
	RHIInstance() {};
};