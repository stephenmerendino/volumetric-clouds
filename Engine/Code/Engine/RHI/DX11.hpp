#pragma once

#include "Engine/RHI/RHITypes.hpp"

#include <d3d11.h>
#include <DXGI.h>
#include <d3dcompiler.h> // needed for shader compilation/reflection

// DEBUG STUFF
#include <dxgidebug.h>
#pragma comment( lib, "dxguid.lib" )

// LIBS
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )
#pragma comment( lib, "d3dcompiler.lib" ) // needed for shader compilation/reflection

#define DX_SAFE_RELEASE(dxResource) { if(dxResource){dxResource->Release(); dxResource = nullptr; } }

D3D11_USAGE DXGetBufferUsage(const BufferUsage usage);
D3D11_PRIMITIVE_TOPOLOGY DXGetTopology(const PrimitiveType type);
D3D11_CULL_MODE DXGetCullMode(CullMode mode);
D3D11_FILL_MODE DXGetFillMode(FillMode mode);
D3D11_COMPARISON_FUNC DXGetDepthComparisonFunc(DepthTest test);
D3D11_COMPARISON_FUNC DXGetSamplerComparisonFunc(SamplerComparisonFunc func);